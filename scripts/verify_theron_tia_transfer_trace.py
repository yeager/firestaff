#!/usr/bin/env python3
"""Validate bounded TIA transfers and $02/$03 accesses from an authentic reference trace."""

from __future__ import annotations

import argparse
import sys
from collections import defaultdict
from pathlib import Path

HEX_FIELDS = {
    "instruction_pc", "physical_pc", "logical", "mpr", "physical",
    "bmt_src", "bmt_dest", "bmt_length", "source", "destination",
    "encoded_length", "source_mpr", "source_physical",
    "destination_mpr", "destination_physical", "value",
    "mpr0", "mpr1", "mpr2", "mpr3", "mpr4", "mpr5", "mpr6", "mpr7",
    "remaining_length",
}


def fields(line: str) -> tuple[str, dict[str, str]]:
    parts = line.split()
    if not parts:
        raise ValueError("empty trace record")
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
        text = record[name]
    except KeyError as exc:
        raise ValueError(f"missing field {name!r}") from exc
    try:
        return int(text, 16 if name in HEX_FIELDS else 10)
    except ValueError as exc:
        raise ValueError(f"invalid numeric field {name!r}: {text!r}") from exc


def bounded(record: dict[str, str], name: str, maximum: int) -> int:
    value = number(record, name)
    if not 0 <= value <= maximum:
        raise ValueError(f"field {name!r} out of range: {value}")
    return value


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="utf-8").splitlines()
        expected_header = (
            "source=mednafen-pce-fast-theron-tia-byte-trace-v1 "
            "event_sequence_source=TheronTraceCDSharedEventSequence "
            "target_pc_range=4600-46ff"
        )
        legacy_header = (
            "source=mednafen-pce-fast-theron-tia-byte-trace-v1 "
            "event_sequence_source=TheronTraceCDSharedEventSequence target_pc=468c"
        )
        if not lines or lines[0] not in {expected_header, legacy_header}:
            raise ValueError("unexpected or missing TIA trace header")
        has_zero_page = lines[0] == expected_header

        starts: dict[int, dict[str, str]] = {}
        transfers: dict[int, list[dict[str, str]]] = defaultdict(list)
        zero_page: list[dict[str, str]] = []
        previous_sequence = -1
        expected_transfer_row = 0
        expected_zero_page_row = 0
        expected_move = 1
        active_move: int | None = None

        for line_number, line in enumerate(lines[1:], 2):
            if "\\n" in line:
                raise ValueError(f"line {line_number}: literal backslash-n in trace")
            if line.startswith(("tia_truncated ", "tia_zero_page_truncated ")):
                raise ValueError(f"line {line_number}: trace reports bounded-output truncation")

            kind, record = fields(line)
            if kind not in {"tia_start", "tia_byte", "tia_zero_page_access"}:
                raise ValueError(f"line {line_number}: unknown record {kind!r}")
            sequence = bounded(record, "event_sequence", 0xFFFFFFFF)
            if sequence <= previous_sequence:
                raise ValueError(f"line {line_number}: event sequence is not increasing")
            previous_sequence = sequence

            pc = bounded(record, "instruction_pc", 0xFFFF)
            if kind == "tia_zero_page_access":
                if not has_zero_page:
                    raise ValueError(f"line {line_number}: zero-page record uses legacy header")
                if not 0x4600 <= pc <= 0x46FF:
                    raise ValueError(f"line {line_number}: zero-page access outside PC range")
                logical = bounded(record, "logical", 0xFFFF)
                if logical not in {0x0002, 0x0003}:
                    raise ValueError(f"line {line_number}: unexpected logical address")
                if record.get("access") not in {"read", "write"}:
                    raise ValueError(f"line {line_number}: invalid access kind")
                slot = bounded(record, "mpr_slot", 7)
                mpr = bounded(record, "mpr", 0xFF)
                physical = bounded(record, "physical", 0x1FFFFF)
                if slot != 0 or physical != (mpr << 13) | (logical & 0x1FFF):
                    raise ValueError(f"line {line_number}: zero-page MPR mapping mismatch")
                bounded(record, "value", 0xFF)
                if bounded(record, "row", 0xFFFFFFFF) != expected_zero_page_row:
                    raise ValueError(f"line {line_number}: non-contiguous zero-page row index")
                expected_zero_page_row += 1
                block_move = bounded(record, "block_move", 1)
                if block_move and pc != 0x468C:
                    raise ValueError(f"line {line_number}: block move outside TIA PC")
                bounded(record, "bmt_src", 0xFFFF)
                bounded(record, "bmt_dest", 0xFFFF)
                bounded(record, "bmt_length", 0xFFFF)
                bounded(record, "alternate", 1)
                zero_page.append(record)
                continue

            if pc != 0x468C:
                raise ValueError(f"line {line_number}: TIA record outside PC $468C")
            move = bounded(record, "move", 0xFFFFFFFF)
            if kind == "tia_start":
                if move != expected_move:
                    raise ValueError(
                        f"line {line_number}: expected move {expected_move}, got {move}"
                    )
                bounded(record, "physical_pc", 0x1FFFFF)
                for slot in range(8):
                    bounded(record, f"mpr{slot}", 0xFF)
                bounded(record, "source", 0xFFFF)
                bounded(record, "destination", 0xFFFF)
                bounded(record, "encoded_length", 0xFFFF)
                starts[move] = record
                expected_move += 1
                active_move = move
                continue

            if active_move is None or move != active_move or move not in starts:
                raise ValueError(f"line {line_number}: byte row does not match active move")
            if bounded(record, "row", 0xFFFFFFFF) != expected_transfer_row:
                raise ValueError(f"line {line_number}: non-contiguous TIA row index")
            expected_transfer_row += 1
            bounded(record, "source", 0xFFFF)
            bounded(record, "destination", 0xFFFF)
            bounded(record, "value", 0xFF)
            bounded(record, "alternate", 1)
            bounded(record, "remaining_length", 0xFFFF)
            bounded(record, "source_mpr_slot", 7)
            bounded(record, "destination_mpr_slot", 7)
            bounded(record, "source_mpr", 0xFF)
            bounded(record, "destination_mpr", 0xFF)
            bounded(record, "source_physical", 0x1FFFFF)
            bounded(record, "destination_physical", 0x1FFFFF)
            transfers[move].append(record)

        if not starts:
            raise ValueError("trace contains no TIA instruction at $468C")
        if has_zero_page and not zero_page:
            raise ValueError("trace contains no $02/$03 accesses in the target PC range")

        for move, start in starts.items():
            source_start = bounded(start, "source", 0xFFFF)
            destination_start = bounded(start, "destination", 0xFFFF)
            encoded_length = bounded(start, "encoded_length", 0xFFFF)
            length = encoded_length or 0x10000
            move_rows = transfers[move]
            if len(move_rows) != length:
                raise ValueError(f"move {move}: expected {length} bytes, got {len(move_rows)}")
            for index, row in enumerate(move_rows):
                source = bounded(row, "source", 0xFFFF)
                destination = bounded(row, "destination", 0xFFFF)
                alternate = bounded(row, "alternate", 1)
                remaining = bounded(row, "remaining_length", 0xFFFF)
                source_mpr = bounded(row, "source_mpr", 0xFF)
                destination_mpr = bounded(row, "destination_mpr", 0xFF)
                source_slot = bounded(row, "source_mpr_slot", 7)
                destination_slot = bounded(row, "destination_mpr_slot", 7)
                if source != ((source_start + index) & 0xFFFF):
                    raise ValueError(f"move {move} row {index}: source sequence mismatch")
                if alternate != (index & 1) or destination != destination_start + alternate:
                    raise ValueError(f"move {move} row {index}: destination alternation mismatch")
                if remaining != ((length - index) & 0xFFFF):
                    raise ValueError(f"move {move} row {index}: remaining length mismatch")
                if source_slot != source >> 13 or destination_slot != destination >> 13:
                    raise ValueError(f"move {move} row {index}: MPR slot mismatch")
                if bounded(row, "source_physical", 0x1FFFFF) != (
                    source_mpr << 13
                ) | (source & 0x1FFF):
                    raise ValueError(f"move {move} row {index}: source mapping mismatch")
                if bounded(row, "destination_physical", 0x1FFFFF) != (
                    destination_mpr << 13
                ) | (destination & 0x1FFF):
                    raise ValueError(f"move {move} row {index}: destination mapping mismatch")

        if has_zero_page:
            required_reads = {(0x466F, 0x0002), (0x4674, 0x0003)}
            observed_reads = {
                (bounded(row, "instruction_pc", 0xFFFF), bounded(row, "logical", 0xFFFF))
                for row in zero_page
                if row["access"] == "read"
            }
            missing_reads = required_reads - observed_reads
            if missing_reads:
                raise ValueError(f"missing direct operand reads: {sorted(missing_reads)}")

        print(
            f"PASS: moves={len(starts)} byte_rows={expected_transfer_row} "
            f"zero_page_rows={expected_zero_page_row} event_sequence={previous_sequence} "
            "all_moves_complete=1 truncated=0"
        )
        return 0
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
