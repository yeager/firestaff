#!/usr/bin/env python3
"""Verify a System Card CD read-to-buffer trace against authentic JP media."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

from verify_theron_jp_cd_data_port_trace import verify as verify_data_port_trace


TRACE_HEADER = "source=mednafen-pce-fast-syscard-buffer-write"
OBSERVED_BYTES = 4_096
FIRST_LOGICAL_DESTINATION = 0x2800
FIRST_PHYSICAL_DESTINATION = 0x1F0800
WRITER_PC = 0xEA9C
WRITER_PHYSICAL_PC = 0x0A9C
DESTINATION_MPR = 0xF8
ROW_PATTERN = re.compile(
    r"syscard_buffer_write sequence=(\d+) logical_destination=([0-9a-f]{4}) "
    r"physical_destination=([0-9a-f]{6}) value=([0-9a-f]{2}) "
    r"writer_pc=([0-9a-f]{4}) writer_physical_pc=([0-9a-f]{6}) "
    r"destination_mpr=([0-9a-f]{2}) a=([0-9a-f]{2}) "
    r"x=([0-9a-f]{2}) y=([0-9a-f]{2})"
)


def verify(write_trace: Path, read_trace: Path, track02: Path) -> None:
    _, reads = verify_data_port_trace(read_trace, track02)
    lines = write_trace.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != TRACE_HEADER:
        raise ValueError("unexpected System Card buffer-write trace header")
    rows = lines[1:]
    if len(rows) != OBSERVED_BYTES:
        raise ValueError(f"expected {OBSERVED_BYTES} writes, found {len(rows)}")

    for sequence, (line, read) in enumerate(zip(rows, reads)):
        match = ROW_PATTERN.fullmatch(line)
        if not match:
            raise ValueError(f"malformed buffer-write row at line {sequence + 2}")
        values = [
            int(value, 16) if index else int(value)
            for index, value in enumerate(match.groups())
        ]
        (
            row_sequence,
            logical_destination,
            physical_destination,
            value,
            writer_pc,
            writer_physical_pc,
            destination_mpr,
            accumulator,
            _x,
            _y,
        ) = values
        if row_sequence != sequence:
            raise ValueError(f"unexpected write sequence at row {sequence}")
        if logical_destination != FIRST_LOGICAL_DESTINATION + sequence:
            raise ValueError(f"unexpected logical destination at row {sequence}")
        expected_physical = FIRST_PHYSICAL_DESTINATION + sequence
        mapped_physical = (destination_mpr << 13) | (logical_destination & 0x1FFF)
        if (
            destination_mpr != DESTINATION_MPR
            or physical_destination != expected_physical
            or mapped_physical != physical_destination
        ):
            raise ValueError(f"unexpected physical destination or MPR at row {sequence}")
        if writer_pc != WRITER_PC or writer_physical_pc != WRITER_PHYSICAL_PC:
            raise ValueError(f"unexpected writer PC at row {sequence}")
        if value != accumulator or value != read["value"]:
            raise ValueError(f"write differs from accumulator or source read at row {sequence}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("write_trace", type=Path)
    parser.add_argument("read_trace", type=Path)
    parser.add_argument("track02", type=Path)
    args = parser.parse_args()
    try:
        verify(args.write_trace, args.read_trace, args.track02)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    print("PASS: 5 verification loops; 4096 authentic CD-port bytes match CPU buffer writes")
    print("reader_pc=0xea99 writer_pc=0xea9c destination=$2800-$37ff physical=$1f0800-$1f17ff")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
