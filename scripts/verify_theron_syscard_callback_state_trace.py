#!/usr/bin/env python3
"""Validate mapped, ordered reads and writes of System Card callback state."""

from __future__ import annotations

import re
import sys
from pathlib import Path


HEADER = (
    "source=mednafen-pce-fast-system-card-callback-state-v1 "
    "event_sequence_source=TheronTraceCDSharedEventSequence "
    "limit=131072 logical_ranges=f8-fb,2256-226b"
)
ROW = re.compile(r"^callback_state (.+)$")
FIELDS = (
    "access", "index", "event_sequence", "logical_address", "physical_address",
    "mpr", "value", "logical_pc", "physical_pc", *(f"mpr{i}" for i in range(8)),
)
HEX_FIELDS = {
    "logical_address": 4, "physical_address": 6, "mpr": 2, "value": 2,
    "logical_pc": 4, "physical_pc": 6,
    **{f"mpr{i}": 2 for i in range(8)},
}


def verify_lines(lines: list[str]) -> dict[str, int]:
    if not lines or lines[0] != HEADER:
        raise ValueError("unexpected callback-state trace header")
    reads = writes = 0
    previous_event = -1
    rows = 0
    truncated = False
    for line_number, line in enumerate(lines[1:], 2):
        if line == "callback_state_truncated cap=131072":
            if truncated or rows != 131072:
                raise ValueError("callback-state truncation marker disagrees with row count")
            truncated = True
            continue
        match = ROW.fullmatch(line)
        if not match or truncated:
            raise ValueError(f"malformed or out-of-order callback-state row at line {line_number}")
        fields: dict[str, str] = {}
        for token in match.group(1).split():
            key, separator, value = token.partition("=")
            if not separator or key in fields:
                raise ValueError(f"malformed callback-state field at line {line_number}")
            fields[key] = value
        if tuple(fields) != FIELDS:
            raise ValueError(f"unexpected callback-state fields at line {line_number}")
        if fields["access"] not in ("read", "write"):
            raise ValueError(f"invalid memory access kind at line {line_number}")
        for key, width in HEX_FIELDS.items():
            if not re.fullmatch(rf"[0-9a-f]{{{width}}}", fields[key]):
                raise ValueError(f"invalid {key} encoding at line {line_number}")
        if not re.fullmatch(r"(?:0|[1-9][0-9]*)", fields["index"]):
            raise ValueError(f"invalid row index at line {line_number}")
        if int(fields["index"]) != rows:
            raise ValueError(f"non-contiguous row index at line {line_number}")
        if not re.fullmatch(r"(?:0|[1-9][0-9]*)", fields["event_sequence"]):
            raise ValueError(f"invalid event sequence at line {line_number}")
        event = int(fields["event_sequence"])
        if event <= previous_event:
            raise ValueError(f"non-increasing shared event sequence at line {line_number}")
        previous_event = event
        logical = int(fields["logical_address"], 16)
        if not (0x00F8 <= logical <= 0x00FB or 0x2256 <= logical <= 0x226B):
            raise ValueError(f"out-of-scope logical address at line {line_number}")
        slot = logical >> 13
        mprs = [int(fields[f"mpr{i}"], 16) for i in range(8)]
        mpr = int(fields["mpr"], 16)
        if mpr != mprs[slot]:
            raise ValueError(f"access MPR disagrees with logical address at line {line_number}")
        physical = (mpr << 13) | (logical & 0x1FFF)
        if physical != int(fields["physical_address"], 16):
            raise ValueError(f"logical/physical data mapping mismatch at line {line_number}")
        pc = int(fields["logical_pc"], 16)
        pc_slot = pc >> 13
        expected_pc = (mprs[pc_slot] << 13) | (pc & 0x1FFF)
        if expected_pc != int(fields["physical_pc"], 16):
            raise ValueError(f"logical/physical PC mapping mismatch at line {line_number}")
        if fields["access"] == "read":
            reads += 1
        else:
            writes += 1
        rows += 1
        if rows > 131072:
            raise ValueError("callback-state trace exceeds its configured cap")
    if not rows:
        raise ValueError("callback-state trace contains no memory accesses")
    if truncated:
        raise ValueError("callback-state trace reached its row cap")
    return {"rows": rows, "reads": reads, "writes": writes}


def main() -> int:
    try:
        result = verify_lines(Path(sys.argv[1]).read_text(encoding="ascii").splitlines())
    except (OSError, UnicodeError, ValueError, IndexError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(
        "PASS: System Card callback-state rows="
        f"{result['rows']} reads={result['reads']} writes={result['writes']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
