#!/usr/bin/env python3
"""Validate Ymir's VDP1 command trace and expose per-draw state.

The command trace is execution evidence, not a texture or scene-owner proof.
This auditor checks command dispatch order against Saturn VDP1 CMDLINK/END
semantics and reports the LOCAL state observed by Ymir for each draw. It does
not assign DGN, MENU.BPK, FONT256, palette, camera, or production ownership.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import struct
import sys
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path
from typing import TextIO

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file
from analyze_nexus_vdp1_command_sequence import (
    Record,
    find_chain,
    follow,
    parse_copr,
    records,
    signed_11,
)


REQUIRED_COLUMNS = {
    "frame", "index", "address", "frame_root", "local_x", "local_y",
    *(f"word{i}" for i in range(16)),
}
COMMAND_BYTES = 32
VDP1_VRAM_BYTES = 0x80000
DRAW_TYPES = set(range(8))


class TraceError(ValueError):
    """Raised when a Ymir trace is incomplete or internally inconsistent."""


def _sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


@dataclass
class FrameAudit:
    frame: int
    records: int = 0
    draws: int = 0
    types: Counter[int] = field(default_factory=Counter)
    roots: set[int] = field(default_factory=set)
    locals_seen: set[tuple[int, int]] = field(default_factory=set)
    addresses: list[int] = field(default_factory=list)
    draw_state: list[dict[str, object]] = field(default_factory=list)
    user_clip: tuple[int, ...] | None = None
    system_clip: tuple[int, ...] | None = None

    def summary(self) -> dict[str, object]:
        return {
            "frame": self.frame,
            "root": f"0x{self.addresses[0]:05x}" if self.addresses else None,
            "records": self.records,
            "draws": self.draws,
            "command_types": {
                f"0x{kind:x}": self.types[kind]
                for kind in sorted(self.types)
            },
            "frame_roots": [f"0x{root:x}" for root in sorted(self.roots)],
            "local_coordinates": [list(point) for point in sorted(self.locals_seen)],
            "first_address": f"0x{self.addresses[0]:05x}" if self.addresses else None,
            "last_address": f"0x{self.addresses[-1]:05x}" if self.addresses else None,
        }


def _integer(row: dict[str, str], key: str, line_number: int) -> int:
    raw = row.get(key)
    if raw is None or raw == "":
        raise TraceError(f"line {line_number}: missing {key}")
    try:
        if key.startswith("word") or key in {"address", "frame_root"}:
            return int(raw, 0)
        return int(raw, 10)
    except ValueError as error:
        raise TraceError(f"line {line_number}: invalid {key}={raw!r}") from error


def _parse_row(row: dict[str, str], line_number: int) -> dict[str, int]:
    values = {key: _integer(row, key, line_number) for key in REQUIRED_COLUMNS}
    if values["address"] < 0 or values["address"] >= VDP1_VRAM_BYTES:
        raise TraceError(f"line {line_number}: VDP1 address outside VRAM")
    if values["address"] % COMMAND_BYTES:
        raise TraceError(f"line {line_number}: unaligned command address")
    if any(not 0 <= values[f"word{i}"] <= 0xFFFF for i in range(16)):
        raise TraceError(f"line {line_number}: command word outside uint16")
    if values["frame"] < 0 or values["index"] < 0:
        raise TraceError(f"line {line_number}: negative frame/index")
    return values


def _command_type(row: dict[str, int]) -> int:
    return row["word0"] & 0x000F


def _expected_next(row: dict[str, int], returns: list[int]) -> int | None:
    control = row["word0"]
    if control & 0x8000:
        return None
    mode = (control >> 12) & 0x3
    if mode == 0:
        return row["address"] + COMMAND_BYTES
    if mode == 1:
        return (row["word1"] << 3) & (VDP1_VRAM_BYTES - 1)
    if mode == 2:
        returns.append(row["address"] + COMMAND_BYTES)
        return (row["word1"] << 3) & (VDP1_VRAM_BYTES - 1)
    if not returns:
        raise TraceError(
            f"return command at 0x{row['address']:05x} has no matching call"
        )
    return returns.pop()


def _audit_frame(
    frame_rows: list[dict[str, int]], include_draw_state: bool = False
) -> FrameAudit:
    if not frame_rows:
        raise TraceError("empty frame")
    frame = frame_rows[0]["frame"]
    audit = FrameAudit(frame=frame)
    returns: list[int] = []
    seen_addresses: set[int] = set()

    for expected_index, row in enumerate(frame_rows):
        if row["frame"] != frame:
            raise TraceError("frame group contains mixed frame numbers")
        if row["index"] != expected_index:
            raise TraceError(
                f"frame {frame}: expected command index {expected_index}, "
                f"got {row['index']}"
            )
        address = row["address"]
        if address in seen_addresses:
            raise TraceError(
                f"frame {frame}: command loop at 0x{address:05x}"
            )
        seen_addresses.add(address)
        audit.addresses.append(address)
        audit.roots.add(row["frame_root"])
        audit.records += 1
        kind = _command_type(row)
        audit.types[kind] += 1
        audit.locals_seen.add((row["local_x"], row["local_y"]))

        if kind == 8 and not row["word0"] & 0x8000:
            audit.user_clip = (
                row["word6"] & 0x1FFF,
                row["word7"] & 0x1FFF,
                row["word10"] & 0x1FFF,
                row["word11"] & 0x1FFF,
            )
        elif kind == 9 and not row["word0"] & 0x8000:
            audit.system_clip = (row["word10"], row["word11"])

        if kind in DRAW_TYPES and not row["word0"] & 0x8000:
            audit.draws += 1
            if include_draw_state:
                audit.draw_state.append({
                    "index": row["index"],
                    "address": f"0x{address:05x}",
                    "type": f"0x{kind:x}",
                    "control": f"0x{row['word0']:04x}",
                    "link": f"0x{row['word1']:04x}",
                    "local": [row["local_x"], row["local_y"]],
                    "user_clip": list(audit.user_clip) if audit.user_clip else None,
                    "system_clip_words": (
                        list(audit.system_clip) if audit.system_clip else None
                    ),
                })

        expected = _expected_next(row, returns)
        if expected is None:
            if expected_index != len(frame_rows) - 1:
                raise TraceError(f"frame {frame}: records follow END")
            if returns:
                raise TraceError(f"frame {frame}: END leaves calls unreturned")
        elif expected_index + 1 >= len(frame_rows):
            raise TraceError(f"frame {frame}: trace ends before next command")
        elif frame_rows[expected_index + 1]["address"] != expected:
            following = frame_rows[expected_index + 1]["address"]
            raise TraceError(
                f"frame {frame} index {expected_index}: expected next "
                f"address 0x{expected:05x}, got 0x{following:05x}"
            )

    if audit.addresses[0] != 0:
        raise TraceError(
            f"frame {frame}: dispatch root is 0x{audit.addresses[0]:05x}, "
            "expected VDP1 command address zero"
        )
    if audit.roots != {0}:
        raise TraceError(
            f"frame {frame}: unexpected frame_root values "
            f"{sorted(audit.roots)}"
        )
    if not frame_rows[-1]["word0"] & 0x8000:
        raise TraceError(f"frame {frame}: trace ends without an END command")
    return audit


def audit_trace(path: Path, selected_frame: int | None = None) -> dict[str, object]:
    try:
        byte_count = path.stat().st_size
        source_hash = _sha256_file(path)
    except OSError as error:
        raise TraceError(f"cannot read trace {path}: {error}") from error

    selected: FrameAudit | None = None
    frame_count = 0
    first_frame: int | None = None
    last_frame: int | None = None
    total_records = 0
    total_draws = 0
    all_dispatch_roots_zero = True
    all_frame_roots_zero = True
    current_rows: list[dict[str, int]] = []
    current_frame: int | None = None
    try:
        with path.open("r", encoding="utf-8", newline="") as stream:
            reader = csv.DictReader(stream)
            columns = set(reader.fieldnames or ())
            missing = sorted(REQUIRED_COLUMNS - columns)
            if missing:
                raise TraceError("missing CSV columns: " + ", ".join(missing))
            for line_number, source_row in enumerate(reader, start=2):
                row = _parse_row(source_row, line_number)
                frame = row["frame"]
                if current_frame is None:
                    current_frame = frame
                elif frame != current_frame:
                    if frame < current_frame:
                        raise TraceError("frame numbers are not increasing")
                    audit = _audit_frame(
                        current_rows, include_draw_state=selected_frame == current_frame
                    )
                    frame_count += 1
                    if first_frame is None:
                        first_frame = audit.frame
                    last_frame = audit.frame
                    total_records += audit.records
                    total_draws += audit.draws
                    all_dispatch_roots_zero &= audit.addresses[0] == 0
                    all_frame_roots_zero &= audit.roots == {0}
                    if selected_frame == audit.frame:
                        selected = audit
                    current_rows = []
                    current_frame = frame
                current_rows.append(row)
            if current_rows:
                audit = _audit_frame(
                    current_rows, include_draw_state=selected_frame == current_frame
                )
                frame_count += 1
                if first_frame is None:
                    first_frame = audit.frame
                last_frame = audit.frame
                total_records += audit.records
                total_draws += audit.draws
                all_dispatch_roots_zero &= audit.addresses[0] == 0
                all_frame_roots_zero &= audit.roots == {0}
                if selected_frame == audit.frame:
                    selected = audit
    except OSError as error:
        raise TraceError(f"cannot read trace {path}: {error}") from error

    if frame_count == 0:
        raise TraceError("trace contains no command rows")
    if selected_frame is not None and selected is None:
        raise TraceError(f"requested frame {selected_frame} is absent")
    return {
        "schema": "firestaff.nexus.ymir-vdp1-trace-audit.v1",
        "trace": {
            "path": str(path),
            "sha256": source_hash,
            "bytes": byte_count,
        },
        "frames": {
            "count": frame_count,
            "first": first_frame,
            "last": last_frame,
            "records": total_records,
            "draws": total_draws,
            "all_dispatch_roots_zero": all_dispatch_roots_zero,
            "all_frame_roots_zero": all_frame_roots_zero,
        },
        "selected_frame": (
            {**selected.summary(), "draw_state": selected.draw_state}
            if selected else None
        ),
        "semantic_admission": "blocked",
        "limitations": [
            "The CSV proves dispatched command order and observed local/clip state only.",
            "It does not bind command textures or palettes to retail assets.",
            "It does not establish camera transforms, full-scene ownership, or Firestaff output.",
        ],
    }


def _chain_summary(chain: list[Record]) -> dict[str, object]:
    user_clip: tuple[int, ...] | None = None
    system_clip: tuple[int, ...] | None = None
    origin = (0, 0)
    origin_verified = False
    draw_state: list[dict[str, object]] = []
    type_counts: Counter[int] = Counter()
    for record in chain:
        kind = record.command_type
        type_counts[kind] += 1
        if record.end:
            continue
        if kind == 8:
            user_clip = (
                record.words[6] & 0x1FFF,
                record.words[7] & 0x1FFF,
                record.words[10] & 0x1FFF,
                record.words[11] & 0x1FFF,
            )
        elif kind == 9:
            system_clip = (record.words[10], record.words[11])
        elif kind == 10:
            origin = (signed_11(record.words[6]), signed_11(record.words[7]))
            origin_verified = True
        elif kind in DRAW_TYPES:
            draw_state.append({
                "offset": f"0x{record.offset:05x}",
                "type": f"0x{kind:x}",
                "origin": list(origin),
                "origin_verified": origin_verified,
                "user_clip": list(user_clip) if user_clip else None,
                "system_clip_words": list(system_clip) if system_clip else None,
            })
    return {
        "start": f"0x{chain[0].offset:05x}" if chain else None,
        "end": f"0x{chain[-1].offset:05x}" if chain else None,
        "records": len(chain),
        "draws": len(draw_state),
        "command_types": {
            f"0x{kind:x}": type_counts[kind] for kind in sorted(type_counts)
        },
        "offsets": [f"0x{record.offset:05x}" for record in chain],
        "draw_state": draw_state,
    }


def audit_runtime_capture(
    path: Path, required_frames: int, selected_frame: int
) -> dict[str, object]:
    """Compare the VDP1 command list rooted at zero to the VRAM scan heuristic."""
    if required_frames <= 0 or selected_frame < 0 or selected_frame >= required_frames:
        raise TraceError("runtime capture frame is outside the declared frame range")
    selected_vram: bytes | None = None
    selected_state: str | None = None
    try:
        for frame_index, regions in iter_frame_regions_file(path, required_frames):
            if frame_index == selected_frame:
                selected_vram = regions["vdp1-vram"]
                selected_state = regions["vdp1-state"]
    except (OSError, ValueError) as error:
        raise TraceError(f"invalid runtime capture: {error}") from error
    if selected_vram is None or selected_state is None:
        raise TraceError(f"runtime capture frame {selected_frame} is absent")

    try:
        copr_word = parse_copr(selected_state)
    except ValueError as error:
        raise TraceError(f"raw capture frame has no valid COPR: {error}") from error
    copr_offset = copr_word << 3
    command_index = {record.offset: record for record in records(selected_vram)}
    try:
        root_chain = follow(command_index, 0)
    except ValueError as error:
        raise TraceError(f"invalid VDP1 root-zero chain: {error}") from error
    try:
        scan_chain = find_chain(selected_vram, copr_word)
    except (ValueError, struct.error) as error:
        raise TraceError(f"VRAM candidate scan failed: {error}") from error

    root = _chain_summary(root_chain)
    scanned = _chain_summary(scan_chain)
    same_offsets = root["offsets"] == scanned["offsets"]
    same_draw_state = root["draw_state"] == scanned["draw_state"]
    first_offset_difference = next((
        index for index, (left, right) in enumerate(
            zip(root["offsets"], scanned["offsets"])
        ) if left != right
    ), min(len(root["offsets"]), len(scanned["offsets"])))
    try:
        capture_hash = _sha256_file(path)
    except OSError as error:
        raise TraceError(f"cannot hash runtime capture: {error}") from error
    return {
        "frame_index": selected_frame,
        "capture_frames_validated": required_frames,
        "capture_bytes": path.stat().st_size,
        "capture_sha256": capture_hash,
        "copr_byte_offset": f"0x{copr_offset:05x}",
        "copr_is_in_root_zero_chain": any(
            record.offset == copr_offset for record in root_chain
        ),
        "vdp1_state": selected_state,
        "hardware_root_zero_chain": root,
        "candidate_scan_selection": scanned,
        "comparison": {
            "same_command_offsets": same_offsets,
            "same_per_draw_local_clip_state": same_draw_state,
            "candidate_scan_starts_at_zero": bool(
                scan_chain and scan_chain[0].offset == 0
            ),
            "first_offset_difference_index": first_offset_difference,
            "classification": (
                "agreement" if same_offsets and same_draw_state
                else "candidate-scan-selects-a-different-chain"
            ),
        },
        "capture_semantics": "blocked",
    }


def main(argv: list[str] | None = None, stdout: TextIO = sys.stdout) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path, help="Ymir VDP1 trace CSV")
    parser.add_argument("--frame", type=int, help="include per-draw state for one frame")
    parser.add_argument("--runtime-capture", type=Path,
                        help="optional independent Firestaff/Mednafen raw capture")
    parser.add_argument("--runtime-capture-frames", type=int,
                        help="declared complete raw-capture frame count")
    parser.add_argument("--runtime-frame", type=int,
                        help="zero-based frame index in the raw capture")
    args = parser.parse_args(argv)
    runtime_args = (
        args.runtime_capture, args.runtime_capture_frames, args.runtime_frame
    )
    if any(value is not None for value in runtime_args) and not all(
        value is not None for value in runtime_args
    ):
        parser.error("--runtime-capture, --runtime-capture-frames, and "
                     "--runtime-frame must be provided together")
    try:
        report = audit_trace(args.trace, args.frame)
        if args.runtime_capture is not None:
            report["independent_runtime_capture"] = audit_runtime_capture(
                args.runtime_capture,
                args.runtime_capture_frames,
                args.runtime_frame,
            )
    except (OSError, TraceError) as error:
        print(f"NEXUS_YMIR_VDP1_TRACE_INVALID: {error}", file=stdout)
        return 1
    print(json.dumps(report, indent=2, sort_keys=True), file=stdout)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
