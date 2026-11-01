#!/usr/bin/env python3
"""Join authentic MPR2=$68 instruction checkpoints to prior physical RAM writes."""

from __future__ import annotations

import argparse
import bisect
import re
import sys
from collections import Counter
from pathlib import Path

from analyze_theron_runtime_mpr_page_source import (
    TRACE_HEADER_V2,
    read_dispatches,
)


WRITER_HEADER = (
    "source=mednafen-pce-fast-main-ram-target-write "
    "event_sequence_source=TheronTraceCDSharedEventSequence"
)
WRITE_RE = re.compile(
    r"^main_ram_target_write sequence=(\d+) event_sequence=(\d+) "
    r"logical_address=([0-9a-f]{4}) physical_address=([0-9a-f]{6}) "
    r"value=([0-9a-f]{2}) writer_pc=([0-9a-f]{4}) "
    r"writer_physical_pc=([0-9a-f]{6}) "
    r"writer_code_bytes=([0-9a-f]{16}|unavailable) "
    r"mpr0=([0-9a-f]{2}) mpr1=([0-9a-f]{2}) mpr2=([0-9a-f]{2}) "
    r"mpr3=([0-9a-f]{2}) mpr4=([0-9a-f]{2}) mpr5=([0-9a-f]{2}) "
    r"mpr6=([0-9a-f]{2}) mpr7=([0-9a-f]{2})$"
)
PAGE_FIRST = 0x0D0000
PAGE_LAST = 0x0D1FFF


def fetch_slots(dispatches: list[dict[str, int]]) -> dict[int, list[tuple[int, int, int, int]]]:
    """Map each needed physical byte to its event, expected byte, PC and step."""
    slots: dict[int, list[tuple[int, int, int, int]]] = {}
    for row in dispatches:
        logical_pc = row["logical_pc"]
        code = (row["opcode"], row["operand1"], row["operand2"])
        for offset, value in enumerate(code):
            logical = (logical_pc + offset) & 0xFFFF
            slot = logical >> 13
            physical = (row[f"mpr{slot}"] << 13) | (logical & 0x1FFF)
            if PAGE_FIRST <= physical <= PAGE_LAST:
                slots.setdefault(physical, []).append(
                    (row["event_sequence"], value, logical_pc, row["step"])
                )
    return slots


def verify(runtime_path: Path, writer_path: Path) -> tuple[dict[str, int], Counter[int]]:
    runtime_lines = runtime_path.read_text(encoding="ascii").splitlines()
    if not runtime_lines or runtime_lines[0] != TRACE_HEADER_V2:
        raise ValueError("runtime checkpoint trace must use shared-sequence schema v2")
    dispatches, _ = read_dispatches(runtime_path)
    slots = fetch_slots(dispatches)
    if not slots:
        raise ValueError("runtime trace contains no instruction bytes in physical $0D0000-$0D1FFF")

    writes: dict[int, list[tuple[int, int, int]]] = {}
    previous_row_sequence = -1
    previous_event_sequence = -1
    with writer_path.open(encoding="ascii") as writer_stream:
        if writer_stream.readline().rstrip("\n") != WRITER_HEADER:
            raise ValueError("unexpected all-writer RAM trace header")
        for line_number, raw_line in enumerate(writer_stream, 2):
            line = raw_line.rstrip("\n")
            if line.startswith("main_ram_target_write_truncated="):
                raise ValueError("all-writer RAM trace is truncated")
            match = WRITE_RE.fullmatch(line)
            if not match:
                raise ValueError(f"malformed all-writer RAM record at line {line_number}")
            fields = match.groups()
            row_sequence, event_sequence = int(fields[0]), int(fields[1])
            logical = int(fields[2], 16)
            physical = int(fields[3], 16)
            value = int(fields[4], 16)
            writer_pc = int(fields[5], 16)
            mprs = [int(value, 16) for value in fields[8:16]]
            if row_sequence != previous_row_sequence + 1:
                raise ValueError(f"non-contiguous writer row sequence at line {line_number}")
            if event_sequence <= previous_event_sequence:
                raise ValueError(f"non-increasing writer event sequence at line {line_number}")
            if not PAGE_FIRST <= physical <= PAGE_LAST:
                raise ValueError(f"writer row outside requested MPR2 page at line {line_number}")
            if physical != ((mprs[logical >> 13] << 13) | (logical & 0x1FFF)):
                raise ValueError(f"writer logical/physical mapping mismatch at line {line_number}")
            previous_row_sequence, previous_event_sequence = row_sequence, event_sequence
            if physical in slots:
                writes.setdefault(physical, []).append((event_sequence, value, writer_pc))
    if previous_row_sequence < 0:
        raise ValueError("all-writer RAM trace contains no page writes")

    stats = {"matched_latest_prior_write": 0, "mismatched_latest_prior_write": 0,
             "fetch_without_prior_write": 0}
    writer_pcs: Counter[int] = Counter()
    for physical, fetches in slots.items():
        prior_writes = writes.get(physical, [])
        write_events = [item[0] for item in prior_writes]
        for fetch_event, expected_value, _logical_pc, _step in fetches:
            index = bisect.bisect_left(write_events, fetch_event) - 1
            if index < 0:
                stats["fetch_without_prior_write"] += 1
                continue
            _write_event, actual_value, writer_pc = prior_writes[index]
            writer_pcs[writer_pc] += 1
            if actual_value == expected_value:
                stats["matched_latest_prior_write"] += 1
            else:
                stats["mismatched_latest_prior_write"] += 1

    if sum(stats.values()) == 0:
        raise ValueError("no runtime instruction bytes mapped inside the target page")
    return stats, writer_pcs


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("runtime_window", type=Path)
    parser.add_argument("all_writer_trace", type=Path)
    args = parser.parse_args()
    try:
        stats, writer_pcs = verify(args.runtime_window, args.all_writer_trace)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    total = sum(stats.values())
    print(f"runtime_fetch_byte_slots={total}")
    for name, count in stats.items():
        print(f"{name}={count}")
    common_writers = ",".join(
        f"{pc:04x}:{count}" for pc, count in writer_pcs.most_common(12)
    ) or "none"
    print(f"latest_prior_writer_pcs={common_writers}")
    if stats["mismatched_latest_prior_write"] or stats["fetch_without_prior_write"]:
        print("classification=runtime page bytes are not fully backed by the captured writes")
        return 1
    print("classification=every observed runtime byte matches its latest preceding captured write")
    print("source ownership and gameplay meaning remain unresolved")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
