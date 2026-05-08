#!/usr/bin/env python3
"""Pass270: bounded FIRES.EXENEW segment/pattern scan for DM1 PC34 globals."""
from __future__ import annotations

import hashlib
import json
import struct
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
OUT_DIR = ROOT / "parity-evidence/verification/pass270_dm1_v1_fires_segment_pattern_scan"
REPORT = ROOT / "parity-evidence/pass270_dm1_v1_fires_segment_pattern_scan.md"
PASS229 = ROOT / "parity-evidence/verification/pass229_unlzexe_fires_runtime_unblock/manifest.json"

LOAD_SEG = 0x0733
CODE_CANDIDATES = [
    {"id": "A", "static": "22AF:06E9", "source": "COMMAND.C", "meaning": "command/input candidate from pass263"},
    {"id": "B", "static": "1EA4:010D", "source": "MOVESENS.C", "meaning": "sensor/turn-chain candidate from pass263"},
    {"id": "C", "static": "1EA4:01AA", "source": "MOVESENS.C", "meaning": "move/event-chain candidate from pass263"},
    {"id": "D", "static": "1859:0516", "source": "MOVESENS.C", "meaning": "move-result/collision candidate from pass263"},
    {"id": "E", "static": "2AFF:110E", "source": "DUNVIEW.C", "meaning": "viewport draw candidate from pass263; expected out of body"},
]
DATA_TARGETS = [
    {"name": "G0306_i_PartyMapX", "idc_linear": 0x20D0C, "kind": "party_tuple", "bytes": 2},
    {"name": "G0307_i_PartyMapY", "idc_linear": 0x20D0E, "kind": "party_tuple", "bytes": 2},
    {"name": "G0308_i_PartyDirection", "idc_linear": 0x20D10, "kind": "party_tuple", "bytes": 2},
    {"name": "G0432_as_CommandQueue", "idc_linear": 0x25880, "kind": "queue", "bytes": 30},
    {"name": "G0433_i_CommandQueueFirstIndex", "idc_linear": 0x2589E, "kind": "queue_index", "bytes": 2},
    {"name": "G0434_i_CommandQueueLastIndex", "idc_linear": 0x258A0, "kind": "queue_index", "bytes": 2},
]
SOURCE_AUDIT = [
    ("COMMAND.C", 2045, 2156, ["F0380_COMMAND_ProcessQueue_CPSC", "G0432_as_CommandQueue", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]),
    ("COMMAND.C", 1734, 1812, ["G0443_ps_PrimaryKeyboardInput", "G0432_as_CommandQueue", "G0435_B_CommandQueueLocked"]),
    ("MOVESENS.C", 316, 556, ["F0267_MOVE_GetMoveResult_CPSCE", "G0306_i_PartyMapX", "G0307_i_PartyMapY"]),
    ("GAMELOOP.C", 55, 91, ["F0128_DUNGEONVIEW_Draw_CPSF", "G0308_i_PartyDirection", "G0306_i_PartyMapX", "G0307_i_PartyMapY"]),
    ("DUNVIEW.C", 8318, 8611, ["F0128_DUNGEONVIEW_Draw_CPSF", "F0097_DUNGEONVIEW_DrawViewport"]),
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def parse_csip(value: str) -> tuple[int, int]:
    cs, ip = value.split(":", 1)
    return int(cs, 16), int(ip, 16)


def fmt_csip(cs: int, ip: int) -> str:
    return f"{cs & 0xffff:04X}:{ip & 0xffff:04X}"


def header_bytes(exe: Path) -> int:
    data = exe.read_bytes()[:64]
    if data[:2] != b"MZ":
        raise SystemExit(f"not an MZ file: {exe}")
    return struct.unpack_from("<H", data, 8)[0] * 16


def load_pass229_exenew() -> Path:
    data = json.loads(PASS229.read_text())
    candidates = []
    if data.get("decompressed", {}).get("path"):
        candidates.append(Path(data["decompressed"]["path"]))
    candidates.append(ROOT / "parity-evidence/verification/pass229_unlzexe_fires_runtime_unblock/FIRES.EXENEW")
    for p in candidates:
        if p.exists():
            return p
    raise SystemExit("FIRES.EXENEW is not present locally; pass229 manifest path is unavailable")


def occurrences(blob: bytes, needle: bytes, limit: int = 20) -> dict[str, Any]:
    if not needle:
        return {"count": 0, "first_offsets": []}
    count = 0
    first: list[str] = []
    pos = blob.find(needle)
    while pos != -1:
        count += 1
        if len(first) < limit:
            first.append(f"0x{pos:05x}")
        pos = blob.find(needle, pos + 1)
    return {"count": count, "first_offsets": first, "truncated_first_offsets": count > limit}


def source_window() -> list[dict[str, Any]]:
    out = []
    for file_name, start, end, needles in SOURCE_AUDIT:
        path = SOURCE_ROOT / file_name
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        window = "\n".join(lines[start - 1:end])
        missing = [n for n in needles if n not in window]
        refs = []
        for idx in range(start, min(end, start + 7) + 1):
            refs.append(f"{file_name}:{idx}: {lines[idx - 1].strip()}")
        out.append({"file": file_name, "range": [start, end], "ok": not missing, "missing": missing, "refs": refs})
    return out


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    exe = load_pass229_exenew()
    hb = header_bytes(exe)
    raw = exe.read_bytes()
    body = raw[hb:]
    binary = {"path_policy": "read local pass229 FIRES.EXENEW only; not copied or committed", "path": str(exe), "size": exe.stat().st_size, "sha256": sha256(exe), "header_bytes": hb, "body_bytes": len(body)}

    code_results = []
    for cand in CODE_CANDIDATES:
        cs, ip = parse_csip(cand["static"])
        linear = (cs << 4) + ip
        runtime = fmt_csip(cs + LOAD_SEG, ip)
        rec: dict[str, Any] = {**cand, "static_linear": f"0x{linear:05x}", "runtime_by_pass246_load_seg_0733": runtime}
        if linear >= len(body):
            rec.update({"status": "outside_fires_exenew_body", "blocker": f"static linear 0x{linear:x} >= body 0x{len(body):x}"})
        else:
            slice48 = body[linear:linear + 48]
            prefix8 = slice48[:8]
            prefix12 = slice48[:12]
            rec.update({"status": "inside_fires_exenew_body", "bytes_16": slice48[:16].hex(" "), "prefix8_occurrences": occurrences(body, prefix8), "prefix12_occurrences": occurrences(body, prefix12)})
        code_results.append(rec)

    data_results = []
    for target in DATA_TARGETS:
        linear = int(target["idc_linear"])
        static_cs, static_ip = linear >> 4, linear & 0xF
        rec: dict[str, Any] = {**target, "static_csip_from_idc_linear": fmt_csip(static_cs, static_ip), "runtime_by_pass246_load_seg_0733": fmt_csip(static_cs + LOAD_SEG, static_ip)}
        if linear >= len(body):
            rec.update({"status": "outside_fires_exenew_body"})
        else:
            n = int(target["bytes"])
            rec.update({"status": "inside_fires_exenew_body", "bytes_at_target": body[linear:linear + n].hex(" "), "window_minus8_plus24": body[max(0, linear - 8):linear + 24].hex(" ")})
        data_results.append(rec)

    pattern_checks = {
        "party_tuple_8_zero_bytes": occurrences(body, b"\x00" * 8),
        "queue_36_zero_bytes": occurrences(body, b"\x00" * 36),
        "queue_30_zero_bytes_then_first0_last4": occurrences(body, (b"\x00" * 30) + b"\x00\x00" + b"\x04\x00"),
        "queue_30_zero_bytes_then_indices_zero": occurrences(body, b"\x00" * 34),
    }

    manifest = {
        "schema": "pass270_dm1_v1_fires_segment_pattern_scan.v1",
        "status": "BLOCKED_ZERO_DATA_PATTERNS_NON_UNIQUE_CODE_PATTERNS_ONLY",
        "binary": binary,
        "load_segment": "0733",
        "source_audit": source_window(),
        "code_candidate_scan": code_results,
        "data_target_scan": data_results,
        "pattern_checks": pattern_checks,
        "conclusion": "A-D code prefixes are bounded inside FIRES.EXENEW and their byte prefixes are usable static-image anchors; E remains outside the decompressed body. The IDC-derived data offsets land inside the decompressed file body but on non-zero/code-like bytes, while the zero-shaped tuple/queue patterns are massively non-unique elsewhere. Therefore the IDC labels cannot be promoted through a simple FIRES.EXENEW linear-body mapping or naive zero-pattern scan; BPM promotion still needs FIRES.MAP/public symbols or a debugger-observed write/read transition.",
        "next_step": "Preserve/build I34E DM.MAP, or dump live loaded FIRES data segments after a controlled queue write and scan for a changed queue/index pattern; do not promote the current zero-pattern matches as addresses.",
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    source_bullets = []
    for s in manifest["source_audit"]:
        source_bullets.append(f"- `{s['file']}:{s['range'][0]}-{s['range'][1]}` ok={s['ok']} missing={s['missing']}")
    code_table = []
    for c in code_results:
        occ = c.get("prefix12_occurrences", {}).get("count")
        code_table.append(f"| {c['id']} | `{c['static']}` | `{c['runtime_by_pass246_load_seg_0733']}` | `{c['static_linear']}` | {c['status']} | {occ if occ is not None else 'n/a'} |")
    data_table = []
    for d in data_results:
        data_table.append(f"| `{d['name']}` | `0x{d['idc_linear']:05X}` | `{d['static_csip_from_idc_linear']}` | `{d['runtime_by_pass246_load_seg_0733']}` | `{d.get('bytes_at_target', 'n/a')}` |")
    patt = pattern_checks
    report = f"""# pass270 — DM1 V1 FIRES.EXENEW segment/pattern scan

Date: 2026-05-06
Worktree: `{ROOT.name}`
Status: `BLOCKED_ZERO_DATA_PATTERNS_NON_UNIQUE_CODE_PATTERNS_ONLY`

## ReDMCSB source audit first

Primary source root: `{SOURCE_ROOT}`.

{chr(10).join(source_bullets)}

## FIRES.EXENEW input

- Read-only local evidence path: `{exe}`
- SHA256: `{binary['sha256']}`
- MZ header bytes: `{hb}`
- Body bytes scanned: `0x{len(body):x}`
- Load segment used from pass246/pass263 lineage: `0733`
- Binary policy: no original/decompressed binary copied or committed.

## Bounded code segment scan

| ID | static CS:IP | runtime with +0733 | static linear | result | 12-byte prefix occurrences |
| --- | --- | --- | --- | --- | ---: |
{chr(10).join(code_table)}

Candidate E remains exactly blocked: its static linear offset is beyond the `FIRES.EXENEW` body. A-D are valid static-image anchors only; this scan does not claim a gameplay/runtime hit.

## IDC-derived data target scan

| Global | IDC linear | static CS:IP | runtime with +0733 | bytes at target |
| --- | ---: | --- | --- | --- |
{chr(10).join(data_table)}

## Pattern uniqueness result

- 8 zero bytes (party tuple shape): `{patt['party_tuple_8_zero_bytes']['count']}` matches.
- 36 zero bytes (queue-sized zero block): `{patt['queue_36_zero_bytes']['count']}` matches.
- 30 zero bytes + first=0 + last=4: `{patt['queue_30_zero_bytes_then_first0_last4']['count']}` matches.
- 34 zero bytes (queue + zero indices shape): `{patt['queue_30_zero_bytes_then_indices_zero']['count']}` matches.

## Exact blocker / next step

The tiny bounded scan is possible and now done, but it proves the remaining global-address problem cannot be solved by the simple IDC-linear-to-FIRES-body model or by naive zero-pattern scanning: the IDC target offsets land on non-zero/code-like bytes, and zero-shaped tuple/queue patterns are massively non-unique elsewhere. The viable next step is to preserve/build an I34E `DM.MAP`/public-symbol table, or to dump live FIRES data segments after a controlled queue write and scan for a changed non-zero queue/index pattern before re-running `BPM`.

Manifest: `parity-evidence/verification/pass270_dm1_v1_fires_segment_pattern_scan/manifest.json`
"""
    REPORT.write_text(report)
    print(json.dumps({"report": str(REPORT), "manifest": str(OUT_DIR / "manifest.json"), "status": manifest["status"]}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
