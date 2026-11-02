#!/usr/bin/env python3
"""Join authentic Ymir SH-2 face/staging observations to VDP1 commands and retail DGN.

Raw game data and emulator captures are inputs only and stay outside Git.
"""
from __future__ import annotations
import argparse
import csv
import hashlib
import importlib.util
import json
import struct
from collections import Counter, defaultdict
from pathlib import Path

WRITE_PCS = {"0x608f366": 0, "0x608f36a": 1, "0x608f36e": 2, "0x608f372": 3}

def be16(data: bytes, off: int) -> int:
    return int.from_bytes(data[off:off + 2], "big")

def be32(data: bytes, off: int) -> int:
    return int.from_bytes(data[off:off + 4], "big")

def load_hashes(repo: Path) -> dict[str, str]:
    path = repo / "scripts/fixtures/nexus_v1_disc_file_hashes.py"
    spec = importlib.util.spec_from_file_location("nexus_disc_hashes", path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load retail hash fixture: {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.DISC_HASH

def parse_retail_dgn(data_dir: Path, expected: dict[str, str]):
    models = []
    for level in range(16):
        name = f"LEV{level:02d}.DGN"
        raw = (data_dir / name).read_bytes()
        digest = hashlib.sha256(raw).hexdigest()
        if expected.get(name) != digest:
            raise ValueError(f"{name} hash mismatch: {digest}")
        block = be16(raw, 0x1C)
        blocks = be16(raw, 0x1E)
        base, size = block * 2048, blocks * 2048
        if not block or not blocks or base + size > len(raw):
            raise ValueError(f"{name}: invalid Structure3 envelope")
        payload = raw[base:base + size]
        count = be32(payload, 0)
        if count == 0 or count > 4096 or 4 + count * 4 > size:
            raise ValueError(f"{name}: invalid Structure3 directory")
        for entry_index in range(count):
            entry_offset = be32(payload, 4 + entry_index * 4)
            entry_end = be32(payload, 8 + entry_index * 4) if entry_index + 1 < count else size
            if entry_offset + 40 > entry_end or entry_end > size:
                raise ValueError(f"{name} entry {entry_index}: invalid bounds")
            header = payload[entry_offset:entry_offset + 40]
            tag = be32(header, 0)
            vertex_count, face_count = be16(header, 4), be16(header, 6)
            vertex_offset, face_offset, normal_offset = (be32(header, x) for x in (8, 16, 20))
            if (vertex_offset + vertex_count * 12 > size or
                    face_offset + face_count * 12 > size or
                    normal_offset + face_count * 12 > size):
                raise ValueError(f"{name} entry {entry_index}: region exceeds payload")
            faces = []
            for ordinal in range(face_count):
                row = payload[face_offset + ordinal * 12:face_offset + (ordinal + 1) * 12]
                indexes = tuple(be16(row, slot * 2) for slot in range(4))
                if any(index >= vertex_count for index in indexes):
                    raise ValueError(f"{name} entry {entry_index} face {ordinal}: bad index")
                faces.append(indexes)
            models.append((level, entry_index, tag, faces))
    return models

def analyze(evidence: Path, retail: Path, repo: Path, frame: int):
    dump = evidence / "f364-correlation/dumps"
    expected = load_hashes(repo)
    models = parse_retail_dgn(retail, expected)
    regs = [row for row in csv.DictReader((dump / "sh2-memory-read-register-snapshots.csv").open())
            if row["vdp1_frame"] == str(frame) and row["pc"] == "0x608f364"]
    writes = list(csv.DictReader((dump / "sh2-memory-write-trace.csv").open()))
    staged: dict[int, dict[int, int]] = defaultdict(dict)
    for row in writes:
        if row["vdp1_frame"] != str(frame) or row["instruction_pc"] not in WRITE_PCS:
            continue
        address = int(row["address"], 16)
        base = address & ~0xF
        slot = WRITE_PCS[row["instruction_pc"]]
        if address != base + slot * 4 or slot in staged[base]:
            raise ValueError(f"unexpected/duplicate staging write: {row}")
        staged[base][slot] = int(row["value"], 16)
    cmds = []
    for row in csv.DictReader((dump / "vdp1-command-trace.csv").open()):
        if row["frame"] != str(frame):
            continue
        words = tuple(int(row[f"word{i}"], 16) for i in range(6, 14))
        corners = tuple((words[i], words[i + 1]) for i in (0, 2, 4, 6))
        cmds.append((int(row["index"]), int(row["address"], 16), corners))
    memory = (dump / "wram-hi.bin").read_bytes()
    rows = []
    for row in regs:
        cpu = row["sh2"]
        face_addr = int(row["r9"], 16)
        stage_addr = int(row["r2"], 16)
        stage = staged.get(stage_addr, {})
        if set(stage) != {0, 1, 2, 3}:
            raise ValueError(f"missing four staging writes at {stage_addr:#x}")
        mem_off = face_addr - 0x06000000
        if mem_off < 0 or mem_off + 8 > len(memory):
            raise ValueError(f"face pointer outside WRAM-H: {face_addr:#x}")
        face = struct.unpack_from(">4H", memory, mem_off)
        corners = tuple(((stage[i] >> 16) & 0xFFFF, stage[i] & 0xFFFF) for i in range(4))
        exact = []
        cyclic = []
        unordered = []
        for index, address, command_corners in cmds:
            if corners == command_corners:
                exact.append((index, address))
            if any(corners == command_corners[k:] + command_corners[:k] for k in range(4)):
                cyclic.append((index, address))
            if Counter(corners) == Counter(command_corners):
                unordered.append((index, address))
        candidates = []
        for level, entry, tag, faces in models:
            candidates.extend((level, entry, ordinal) for ordinal, source_face in enumerate(faces) if source_face == face)
        rows.append({"cpu": cpu, "face_ptr": face_addr, "stage_ptr": stage_addr, "face": face,
                     "corners": corners, "dgn_candidates": candidates, "command_exact": exact,
                     "command_cyclic": cyclic, "command_unordered": unordered})
    # Resolve duplicate single-face topology matches using contiguous face-row runs.
    resolved: dict[tuple[str, int], tuple[int, int, int]] = {}
    direct_unique = 0
    for row in rows:
        if len(row["dgn_candidates"]) == 1:
            direct_unique += 1
            resolved[(row["cpu"], row["face_ptr"])] = row["dgn_candidates"][0]
    runs = []
    for cpu in ("master", "slave"):
        cpu_rows = [row for row in rows if row["cpu"] == cpu]
        current = []
        for row in cpu_rows:
            if current and row["face_ptr"] - current[-1]["face_ptr"] != 12:
                if len(current) >= 2:
                    runs.append(current)
                current = []
            current.append(row)
        if len(current) >= 2:
            runs.append(current)
    unique_runs = []
    for run in runs:
        sequence = [row["face"] for row in run]
        matches = []
        for level, entry, tag, faces in models:
            for start in range(len(faces) - len(sequence) + 1):
                if faces[start:start + len(sequence)] == sequence:
                    matches.append((level, entry, start))
        if len(matches) == 1:
            level, entry, start = matches[0]
            unique_runs.append((run, level, entry, start))
            for offset, row in enumerate(run):
                key = (row["cpu"], row["face_ptr"])
                mapped = (level, entry, start + offset)
                old = resolved.get(key)
                if old is not None and old != mapped:
                    raise ValueError(f"DGN single and sequence mappings disagree at {key}: {old} vs {mapped}")
                resolved[key] = mapped
    # Enforce one-to-one unordered geometry ownership for this captured command set.
    if len(rows) != len(regs):
        raise ValueError("duplicate register/staging association")
    if any(len(row["command_unordered"]) != 1 for row in rows):
        raise ValueError("staging geometry did not map to exactly one captured VDP1 command")
    reverse = defaultdict(list)
    for row in rows:
        reverse[row["command_unordered"][0]].append(row)
    if any(len(value) != 1 for value in reverse.values()):
        raise ValueError("a captured VDP1 command geometry maps to multiple staging records")
    return rows, resolved, direct_unique, runs, unique_runs, len(cmds)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--evidence", type=Path, required=True)
    parser.add_argument("--retail-data", type=Path, required=True)
    parser.add_argument("--firestaff-repo", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=1605)
    parser.add_argument("--csv-out", type=Path, required=True)
    parser.add_argument("--json-out", type=Path, required=True)
    args = parser.parse_args()
    rows, resolved, direct, runs, unique_runs, command_count = analyze(args.evidence, args.retail_data, args.firestaff_repo, args.frame)
    with args.csv_out.open("w", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["cpu", "face_wram_address", "vertex_indices", "source_dgn", "source_entry", "source_face_ordinal", "mapping_basis", "staging_wram_address", "staged_xy_signed", "command_index", "command_vdp1_vram_address", "command_geometry_match"])
        for row in rows:
            key = (row["cpu"], row["face_ptr"])
            mapping = resolved.get(key)
            basis = "unique-face-row" if len(row["dgn_candidates"]) == 1 else ("unique-contiguous-face-sequence" if mapping else "unresolved")
            exact = row["command_exact"]
            cyclic = row["command_cyclic"]
            geometry_match = "ordered" if exact else ("cyclic" if cyclic else "unordered")
            command_index, command_address = row["command_unordered"][0]
            signed = tuple((x if x < 0x8000 else x - 0x10000) for pair in row["corners"] for x in pair)
            writer.writerow([row["cpu"], f"0x{row['face_ptr']:08x}", " ".join(f"{v:04x}" for v in row["face"]),
                             f"LEV{mapping[0]:02d}.DGN" if mapping else "", mapping[1] if mapping else "", mapping[2] if mapping else "", basis,
                             f"0x{row['stage_ptr']:08x}", " ".join(map(str, signed)), command_index, f"0x{command_address:04x}", geometry_match])
    summary = {
        "capture_frame": args.frame,
        "authentic_structure3_entries": 1144,
        "authentic_structure3_faces": 18478,
        "sh2_f364_register_rows": len(rows),
        "master_records": sum(row["cpu"] == "master" for row in rows),
        "slave_records": sum(row["cpu"] == "slave" for row in rows),
        "records_with_four_staging_writes": len(rows),
        "unique_unordered_command_geometry_matches": len(rows),
        "unique_reverse_command_to_staging_matches": len(rows),
        "ordered_coordinate_matches": sum(bool(row["command_exact"]) for row in rows),
        "cyclic_coordinate_matches": sum(bool(row["command_cyclic"]) for row in rows),
        "unique_single_face_retail_matches": direct,
        "contiguous_face_runs": len(runs),
        "unique_contiguous_face_run_matches": len(unique_runs),
        "unique_runtime_face_row_pointers": len({(row["cpu"], row["face_ptr"]) for row in rows}),
        "distinct_retail_face_rows_resolved": len(resolved),
        "distinct_runtime_face_row_pointers_unresolved": len({(row["cpu"], row["face_ptr"]) for row in rows}) - len(resolved),
        "capture_records_resolved_to_retail_face": sum((row["cpu"], row["face_ptr"]) in resolved for row in rows),
        "capture_records_unresolved_to_retail_face": sum((row["cpu"], row["face_ptr"]) not in resolved for row in rows),
        "duplicate_capture_records_sharing_face_row": len(rows) - len({(row["cpu"], row["face_ptr"]) for row in rows}),
        "sequence_matches": [{"cpu": run[0]["cpu"], "run_length": len(run), "first_runtime_r9": f"0x{run[0]['face_ptr']:08x}", "level": level, "entry": entry, "first_face_ordinal": first} for run, level, entry, first in unique_runs],
        "limits": ["Only authentic Ymir frame 1605 is covered.", "Command geometry ownership is an exact four-corner coordinate multiset join; rows marked cyclic/unordered do not prove winding or corner correspondence.", "Unresolved face rows remain unresolved; this does not imply they are absent from retail data.", "No pixel/material ownership, complete frame parity, original-Saturn parity, or production renderer admission is claimed."],
    }
    args.json_out.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(summary, indent=2))

if __name__ == "__main__":
    main()
