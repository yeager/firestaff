#!/usr/bin/env python3
"""Verify TIA reads against the latest preceding write to the same RAM byte."""

from __future__ import annotations

import argparse
import bisect
import re
import sys
from pathlib import Path


TIA_HEADER = (
    "source=mednafen-pce-fast-theron-tia-byte-trace-v1 "
    "event_sequence_source=TheronTraceCDSharedEventSequence "
    "target_pc_range=4600-46ff"
)
WRITER_HEADER = (
    "source=mednafen-pce-fast-main-ram-target-write "
    "event_sequence_source=TheronTraceCDSharedEventSequence"
)
TIA_BYTE = re.compile(
    r"^tia_byte event_sequence=(\d+) move=(\d+) row=(\d+) "
    r"instruction_pc=([0-9a-f]{4}) source=([0-9a-f]{4}) "
    r"source_mpr_slot=(\d+) source_mpr=([0-9a-f]{2}) "
    r"source_physical=([0-9a-f]{6}) destination=([0-9a-f]{4}) "
    r"destination_mpr_slot=(\d+) destination_mpr=([0-9a-f]{2}) "
    r"destination_physical=([0-9a-f]{6}) value=([0-9a-f]{2}) "
    r"remaining_length=([0-9a-f]{4}) alternate=([01])$"
)
RAM_WRITE = re.compile(
    r"^main_ram_target_write sequence=(\d+) event_sequence=(\d+) "
    r"logical_address=([0-9a-f]{4}) physical_address=([0-9a-f]{6}) "
    r"value=([0-9a-f]{2}) writer_pc=([0-9a-f]{4}) "
    r"writer_physical_pc=([0-9a-f]{6}) writer_code_bytes=([0-9a-f]{16}) "
    r"mpr0=([0-9a-f]{2}) mpr1=([0-9a-f]{2}) mpr2=([0-9a-f]{2}) "
    r"mpr3=([0-9a-f]{2}) mpr4=([0-9a-f]{2}) mpr5=([0-9a-f]{2}) "
    r"mpr6=([0-9a-f]{2}) mpr7=([0-9a-f]{2})$"
)


def read_lines(path: Path) -> list[str]:
    return path.read_text(encoding="ascii").splitlines()


def verify(tia_path: Path, writes_path: Path) -> tuple[int, dict[str, int]]:
    tia_lines = read_lines(tia_path)
    write_lines = read_lines(writes_path)
    if not tia_lines or tia_lines[0] != TIA_HEADER:
        raise ValueError("unexpected TIA trace header")
    if not write_lines or write_lines[0] != WRITER_HEADER:
        raise ValueError("unexpected RAM writer trace header")
    if any(line.startswith(("tia_truncated ", "tia_zero_page_truncated ")) for line in tia_lines):
        raise ValueError("TIA trace is truncated")
    if any(line.startswith("main_ram_target_write_truncated=") for line in write_lines):
        raise ValueError("RAM writer trace is truncated")

    writes: dict[int, list[tuple[int, int, int]]] = {}
    previous_write_sequence = -1
    previous_write_event = -1
    for line_number, line in enumerate(write_lines[1:], 2):
        match = RAM_WRITE.fullmatch(line)
        if not match:
            raise ValueError(f"malformed RAM writer row at line {line_number}")
        fields = match.groups()
        sequence, event = int(fields[0]), int(fields[1])
        logical, physical = int(fields[2], 16), int(fields[3], 16)
        value, writer_pc = int(fields[4], 16), int(fields[5], 16)
        mprs = [int(value, 16) for value in fields[8:16]]
        expected_slot = logical >> 13
        if sequence != previous_write_sequence + 1 or event <= previous_write_event:
            raise ValueError(f"RAM writer sequence is not strictly ordered at line {line_number}")
        if physical != ((mprs[expected_slot] << 13) | (logical & 0x1FFF)):
            raise ValueError(f"RAM writer logical-to-physical mapping mismatch at line {line_number}")
        previous_write_sequence, previous_write_event = sequence, event
        writes.setdefault(physical, []).append((event, value, writer_pc))

    stats = {"matched": 0, "missing_predecessor": 0, "value_mismatch": 0}
    previous_tia_event = -1
    for line_number, line in enumerate(tia_lines[1:], 2):
        if line.startswith(("tia_start ", "tia_zero_page_access ")):
            continue
        if not line.startswith("tia_byte "):
            raise ValueError(f"unknown TIA record at line {line_number}")
        match = TIA_BYTE.fullmatch(line)
        if not match:
            raise ValueError(f"malformed TIA row at line {line_number}")
        fields = match.groups()
        event = int(fields[0])
        source = int(fields[4], 16)
        slot, mpr = int(fields[5]), int(fields[6], 16)
        physical, value = int(fields[7], 16), int(fields[12], 16)
        if event <= previous_tia_event:
            raise ValueError(f"TIA event sequence is not strictly ordered at line {line_number}")
        if slot != (source >> 13) or physical != ((mpr << 13) | (source & 0x1FFF)):
            raise ValueError(f"TIA source logical-to-physical mapping mismatch at line {line_number}")
        previous_tia_event = event
        prior_writes = writes.get(physical, [])
        prior_events = [row[0] for row in prior_writes]
        index = bisect.bisect_left(prior_events, event) - 1
        if index < 0:
            stats["missing_predecessor"] += 1
            continue
        stats["matched"] += 1
        if prior_writes[index][1] != value:
            stats["value_mismatch"] += 1

    total = sum(stats.values())
    if not total:
        raise ValueError("TIA trace contains no byte rows")
    if stats["missing_predecessor"] or stats["value_mismatch"]:
        raise ValueError(f"RAM-to-TIA join is incomplete or inconsistent: {stats}")
    return total, stats


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("tia_trace", type=Path)
    parser.add_argument("ram_writer_trace", type=Path)
    args = parser.parse_args()
    try:
        total, stats = verify(args.tia_trace, args.ram_writer_trace)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(
        f"PASS: latest preceding RAM writes match all {total} TIA byte reads; "
        f"missing={stats['missing_predecessor']} mismatches={stats['value_mismatch']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
