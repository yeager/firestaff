#!/usr/bin/env python3
"""Validate bounded instruction-follow receipts around the authentic BIOS handoff."""

from __future__ import annotations

import re
import sys
from pathlib import Path


HEADER = (
    "source=mednafen-pce-fast-system-card-bootstrap-follow-v1 "
    "arm_pc=e11b bootstrap_limit=512 target_pc=4000 target_limit=128"
)
ROW = re.compile(r"^(bootstrap_follow|target_follow) (.+)$")
def fields(text: str, line_number: int) -> tuple[str, dict[str, str]]:
    match = ROW.fullmatch(text)
    if not match:
        raise ValueError(f"malformed row at line {line_number}")
    row_kind, payload = match.groups()
    values: dict[str, str] = {}
    for token in payload.split():
        key, separator, value = token.partition("=")
        if not separator or key in values:
            raise ValueError(f"malformed field at line {line_number}")
        values[key] = value
    return row_kind, values


def verify_lines(lines: list[str]) -> dict[str, int]:
    if not lines or lines[0] != HEADER:
        raise ValueError("unexpected bootstrap-follow trace header")
    counts = {"bootstrap_follow": 0, "target_follow": 0}
    first_pc: dict[str, int] = {}
    truncation_seen = {"bootstrap_follow": False, "target_follow": False}
    for line_number, line in enumerate(lines[1:], start=2):
        if line == "bootstrap_follow_truncated cap=512":
            if truncation_seen["bootstrap_follow"] or counts["bootstrap_follow"] != 512:
                raise ValueError("bootstrap truncation marker disagrees with row count")
            truncation_seen["bootstrap_follow"] = True
            continue
        if line == "target_follow_truncated cap=128":
            if truncation_seen["target_follow"] or counts["target_follow"] != 128:
                raise ValueError("target truncation marker disagrees with row count")
            truncation_seen["target_follow"] = True
            continue
        kind, row = fields(line, line_number)
        if truncation_seen[kind]:
            raise ValueError(f"{kind} row appears after its truncation marker")
        expected_index = counts[kind]
        if int(row.get("index", "-1"), 10) != expected_index:
            raise ValueError(f"non-contiguous {kind} index at line {line_number}")
        if expected_index >= (512 if kind == "bootstrap_follow" else 128):
            raise ValueError(f"{kind} exceeds its configured cap")
        pc = int(row["logical_pc"], 16)
        if not 0 <= pc <= 0xFFFF:
            raise ValueError(f"logical PC is out of range at line {line_number}")
        physical_pc = int(row["physical_pc"], 16)
        mprs = [int(row[f"mpr{i}"], 16) for i in range(8)]
        if not 0 <= physical_pc <= 0xFFFFFF or any(not 0 <= value <= 0xFF for value in mprs):
            raise ValueError(f"physical address or MPR is out of range at line {line_number}")
        mapped_pc = (mprs[(pc >> 13) & 7] << 13) | (pc & 0x1FFF)
        if mapped_pc != physical_pc:
            raise ValueError(f"logical/physical PC mapping mismatch at line {line_number}")
        bytes_value = row["bytes"]
        int(bytes_value, 16)
        physical_bytes = [int(value, 16) for value in row["physical_bytes"].split(",")]
        valid_mask = int(row["valid_mask"], 16)
        if (
            len(bytes_value) != 8
            or len(physical_bytes) != 4
            or not 0 <= valid_mask <= 0xF
            or not valid_mask & 1
            or any(not 0 <= value <= 0xFFFFFF for value in physical_bytes)
        ):
            raise ValueError(f"instruction snapshot width mismatch at line {line_number}")
        for index, physical_address in enumerate(physical_bytes):
            if valid_mask & (1 << index):
                logical = (pc + index) & 0xFFFF
                expected_physical = (mprs[(logical >> 13) & 7] << 13) | (logical & 0x1FFF)
                if physical_address != expected_physical:
                    raise ValueError(f"mapped instruction byte mismatch at line {line_number}")
        if expected_index == 0:
            first_pc[kind] = pc
        counts[kind] += 1
    if counts["bootstrap_follow"] and first_pc.get("bootstrap_follow") != 0xE11B:
        raise ValueError("bootstrap follow did not begin at $E11B")
    if counts["target_follow"] and first_pc.get("target_follow") != 0x4000:
        raise ValueError("target follow did not begin at $4000")
    if not counts["bootstrap_follow"] or not counts["target_follow"]:
        raise ValueError("capture did not observe both the BIOS bootstrap and $4000 target")
    return counts


def verify(path: Path) -> dict[str, int]:
    return verify_lines(path.read_text(encoding="ascii").splitlines())


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} TRACE", file=sys.stderr)
        return 2
    try:
        counts = verify(Path(sys.argv[1]))
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(
        "PASS: "
        f"status=bounded bootstrap_rows={counts['bootstrap_follow']} "
        f"target_rows={counts['target_follow']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
