#!/usr/bin/env python3
"""Validate bounded Mednafen PCE Fast TIA transfer receipts."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path


def fields(line: str) -> tuple[str, dict[str, str]]:
    parts = line.split()
    values: dict[str, str] = {}
    for item in parts[1:]:
        if "=" not in item:
            raise ValueError(f"malformed field {item!r}")
        key, value = item.split("=", 1)
        if key in values:
            raise ValueError(f"duplicate field {key!r}")
        values[key] = value
    return parts[0], values


def number(record: dict[str, str], name: str) -> int:
    try:
        return int(record[name], 16 if name in {
            "instruction_pc", "physical_pc", "source",
            "destination", "encoded_length", "source_mpr", "source_physical",
            "destination_mpr", "destination_physical", "value",
            "remaining_length",
        } else 10)
    except KeyError as exc:
        raise ValueError(f"missing field {name!r}") from exc


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="utf-8").splitlines()
        if not lines or lines[0] != (
            "source=mednafen-pce-fast-theron-tia-byte-trace-v1 "
            "event_sequence_source=TheronTraceCDSharedEventSequence target_pc=468c"
        ):
            raise ValueError("unexpected or missing TIA trace header")

        starts: dict[int, dict[str, str]] = {}
        rows: dict[int, list[dict[str, str]]] = {}
        previous_sequence = -1
        global_row = 0
        truncated = False
        for line_number, line in enumerate(lines[1:], 2):
            if line.startswith("tia_truncated "):
                truncated = True
                continue
            kind, record = fields(line)
            if kind not in {"tia_start", "tia_byte"}:
                raise ValueError(f"line {line_number}: unknown record {kind!r}")
            sequence = number(record, "event_sequence")
            if sequence <= previous_sequence:
                raise ValueError(f"line {line_number}: event sequence is not increasing")
            previous_sequence = sequence
            if number(record, "instruction_pc") != 0x468C:
                raise ValueError(f"line {line_number}: record is outside target PC $468C")
            move = number(record, "move")
            if kind == "tia_start":
                if move in starts:
                    raise ValueError(f"line {line_number}: duplicate move {move}")
                starts[move] = record
                rows[move] = []
                continue

            if move not in starts:
                raise ValueError(f"line {line_number}: byte row has no move header")
            if number(record, "row") != global_row:
                raise ValueError(f"line {line_number}: non-contiguous global row index")
            global_row += 1
            rows[move].append(record)

        if not starts:
            raise ValueError("trace contains no TIA instruction at $468C")
        if truncated:
            raise ValueError("trace reports bounded-output truncation")

        for move, start in starts.items():
            start_source = number(start, "source")
            start_destination = number(start, "destination")
            encoded_length = number(start, "encoded_length")
            expected_rows = encoded_length or 0x10000
            move_rows = rows[move]
            if len(move_rows) != expected_rows:
                raise ValueError(
                    f"move {move}: expected {expected_rows} bytes, got {len(move_rows)}"
                )
            for index, row in enumerate(move_rows):
                source = number(row, "source")
                destination = number(row, "destination")
                alternate = number(row, "alternate")
                remaining = number(row, "remaining_length")
                source_mpr = number(row, "source_mpr")
                destination_mpr = number(row, "destination_mpr")
                source_slot = number(row, "source_mpr_slot")
                destination_slot = number(row, "destination_mpr_slot")
                if source != ((start_source + index) & 0xFFFF):
                    raise ValueError(f"move {move} row {index}: source sequence mismatch")
                if alternate != (index & 1) or destination != start_destination + alternate:
                    raise ValueError(f"move {move} row {index}: TIA destination alternation mismatch")
                if remaining != ((encoded_length - index) & 0xFFFF):
                    raise ValueError(f"move {move} row {index}: remaining length mismatch")
                if source_slot != (source >> 13) or destination_slot != (destination >> 13):
                    raise ValueError(f"move {move} row {index}: MPR slot mismatch")
                source_physical = (source_mpr << 13) | (source & 0x1FFF)
                destination_physical = (destination_mpr << 13) | (destination & 0x1FFF)
                if number(row, "source_physical") != source_physical:
                    raise ValueError(f"move {move} row {index}: source physical mapping mismatch")
                if number(row, "destination_physical") != destination_physical:
                    raise ValueError(f"move {move} row {index}: destination physical mapping mismatch")

        print(
            f"PASS: moves={len(starts)} byte_rows={global_row} "
            f"event_sequence={number(fields(lines[-1])[1], 'event_sequence')} "
            "all_moves_complete=1 truncated=0"
        )
        return 0
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
