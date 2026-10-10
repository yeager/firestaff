#!/usr/bin/env python3
"""Verify bounded block-transfer disassembly from a PCE Fast RAM-read trace."""

from __future__ import annotations

import argparse
import collections
import sys
from pathlib import Path


SOURCE_MARKER = "mednafen-pce-fast-main-ram-consumer-read"
EXPECTED_ROWS = 3452
EXPECTED_REPEATS = 16

# logical PC: physical PC, opcode, source, destination, byte count
TRANSFER_WINDOWS = {
    "b9fc": ("0e19fc", 0xE3, 0x2062, 0x0002, 0x20),
    "b985": ("0e1985", 0x73, 0x2F2C, 0x20A7, 0x15),
    "a1b7": ("0e01b7", 0x73, 0x287F, 0x2883, 0x14),
    "c1e7": ("0d21e7", 0x73, 0x2090, 0x2897, 0x04),
}


def parse_fields(line: str) -> dict[str, str]:
    fields: dict[str, str] = {}
    for token in line.split():
        key, separator, value = token.partition("=")
        if separator:
            fields[key] = value
    return fields


def verify(lines: list[str]) -> None:
    if not lines or parse_fields(lines[0]).get("source") != SOURCE_MARKER:
        raise ValueError("unexpected or missing trace source marker")

    rows: list[dict[str, str]] = []
    for line in lines[1:]:
        if not line.startswith("main_ram_consumer_read "):
            continue
        fields = parse_fields(line)
        rows.append(fields)

    if len(rows) != EXPECTED_ROWS:
        raise ValueError(f"expected {EXPECTED_ROWS} rows, found {len(rows)}")
    for expected_sequence, row in enumerate(rows):
        if row.get("sequence") != str(expected_sequence):
            raise ValueError(f"non-contiguous sequence at row {expected_sequence}")

    grouped: dict[str, list[dict[str, str]]] = collections.defaultdict(list)
    for row in rows:
        grouped[row.get("reader_pc", "").lower()].append(row)

    for logical_pc, (physical_pc, opcode, source, destination, length) in TRANSFER_WINDOWS.items():
        window_rows = grouped.get(logical_pc, [])
        if len(window_rows) != length * EXPECTED_REPEATS:
            raise ValueError(
                f"PC ${logical_pc}: expected {length * EXPECTED_REPEATS} reads, "
                f"found {len(window_rows)}"
            )

        source_counts: collections.Counter[int] = collections.Counter()
        code_windows: set[bytes] = set()
        for row in window_rows:
            if row.get("reader_physical_pc", "").lower() != physical_pc:
                raise ValueError(f"PC ${logical_pc}: unexpected physical mapping")
            address = int(row["logical_address"], 16)
            source_counts[address] += 1
            code_windows.add(bytes.fromhex(row["reader_code_bytes"]))

        expected_addresses = set(range(source, source + length))
        if set(source_counts) != expected_addresses:
            raise ValueError(f"PC ${logical_pc}: source interval is incomplete or unexpected")
        if set(source_counts.values()) != {EXPECTED_REPEATS}:
            raise ValueError(f"PC ${logical_pc}: source-byte read counts differ")
        if len(code_windows) != 1:
            raise ValueError(f"PC ${logical_pc}: instruction bytes changed within the trace")

        code = next(iter(code_windows))
        decoded = (
            code[0],
            code[1] | code[2] << 8,
            code[3] | code[4] << 8,
            code[5] | code[6] << 8,
        )
        if decoded != (opcode, source, destination, length):
            raise ValueError(f"PC ${logical_pc}: opcode operands do not match trace reads")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "trace",
        help="PCE Fast main-RAM consumer sidecar path, or - to read standard input",
    )
    args = parser.parse_args()

    try:
        if args.trace == "-":
            lines = sys.stdin.read().splitlines()
        else:
            lines = Path(args.trace).read_text(encoding="utf-8").splitlines()
        verify(lines)
    except (OSError, ValueError, KeyError, IndexError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    print(
        "PASS: rows={} transfer_windows={} reads_per_source_byte={}".format(
            EXPECTED_ROWS, len(TRANSFER_WINDOWS), EXPECTED_REPEATS
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
