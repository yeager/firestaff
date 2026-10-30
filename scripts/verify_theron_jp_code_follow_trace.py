#!/usr/bin/env python3
"""Validate the bounded executed-instruction sidecar for Theron's JP System Card trace."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


HEADER = (
    "source=mednafen-pce-fast-system-card-code-follow-v1 "
    "start_pc=6148 limit=256 terminal=next-instruction-fetch "
    "event_sequence_source=TheronTraceCDSharedEventSequence"
)
ROW = re.compile(
    r"^code_follow event_sequence=(\d+) index=(\d+) logical_pc=([0-9a-f]{4}) "
    r"physical_pc=([0-9a-f]{6}) bytes=([0-9a-f]{6}) "
    r"physical_bytes=([0-9a-f]{6}),([0-9a-f]{6}),([0-9a-f]{6}) "
    r"valid_mask=([0-7]) mpr0=([0-9a-f]{2}) mpr1=([0-9a-f]{2}) "
    r"mpr2=([0-9a-f]{2}) mpr3=([0-9a-f]{2}) mpr4=([0-9a-f]{2}) "
    r"mpr5=([0-9a-f]{2}) mpr6=([0-9a-f]{2}) mpr7=([0-9a-f]{2})$"
)
COMPLETE = re.compile(r"^code_follow_complete event_sequence=(\d+) count=(\d+)$")


def verify(path: Path, require_complete: bool) -> tuple[str, int]:
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != HEADER:
        raise ValueError("unexpected or missing code-follow header")

    rows = 0
    previous_sequence = -1
    complete_sequence: int | None = None
    for line_number, line in enumerate(lines[1:], 2):
        if "\\n" in line:
            raise ValueError(f"line {line_number}: literal backslash-n in trace")
        if complete_sequence is not None:
            raise ValueError(f"line {line_number}: data follows the terminal marker")

        match = ROW.fullmatch(line)
        if match:
            values = match.groups()
            event, index, logical_pc, physical_pc = (
                int(values[0]), int(values[1]), int(values[2], 16), int(values[3], 16)
            )
            opcodes = [int(values[4][offset:offset + 2], 16) for offset in (0, 2, 4)]
            physical_bytes = [int(values[i], 16) for i in (5, 6, 7)]
            valid_mask = int(values[8])
            mprs = [int(value, 16) for value in values[9:17]]
            if event <= previous_sequence:
                raise ValueError(f"line {line_number}: event sequence is not increasing")
            if index != rows:
                raise ValueError(f"line {line_number}: expected follow index {rows}, got {index}")
            if rows == 0 and logical_pc != 0x6148:
                raise ValueError(f"line {line_number}: first executed instruction is not $6148")
            expected_pc = (mprs[logical_pc >> 13] << 13) | (logical_pc & 0x1FFF)
            if physical_pc != expected_pc:
                raise ValueError(f"line {line_number}: physical PC does not match its MPR")
            for byte_index in range(3):
                logical = (logical_pc + byte_index) & 0xFFFF
                slot = logical >> 13
                expected_physical = (mprs[slot] << 13) | (logical & 0x1FFF)
                is_valid = bool(valid_mask & (1 << byte_index))
                if is_valid and physical_bytes[byte_index] != expected_physical:
                    raise ValueError(f"line {line_number}: opcode byte MPR mapping mismatch")
                if not is_valid and (physical_bytes[byte_index] != 0 or opcodes[byte_index] != 0):
                    raise ValueError(f"line {line_number}: invalid opcode byte is not zeroed")
            if rows >= 256:
                raise ValueError(f"line {line_number}: instruction cap exceeded")
            previous_sequence = event
            rows += 1
            continue

        terminal = COMPLETE.fullmatch(line)
        if terminal:
            complete_sequence, count = map(int, terminal.groups())
            if complete_sequence <= previous_sequence:
                raise ValueError(f"line {line_number}: terminal event sequence is not increasing")
            if rows != 256 or count != 256:
                raise ValueError(f"line {line_number}: completion marker requires exactly 256 rows")
            continue
        raise ValueError(f"line {line_number}: malformed or unknown record")

    if complete_sequence is not None:
        status = "complete"
    elif rows:
        status = "incomplete"
    else:
        status = "target_not_reached"
    if require_complete and status != "complete":
        raise ValueError(f"code-follow status is {status}, not complete")
    return status, rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--require-complete", action="store_true")
    args = parser.parse_args()
    try:
        status, rows = verify(args.trace, args.require_complete)
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    print(f"status={status} rows={rows}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
