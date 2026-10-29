#!/usr/bin/env python3
"""Verify shared-sequence byte copies from Theron's decoded buffer.

This joins authentic decoder writes, later reads at the disassembled
``$1042AB`` loop, and the following destination write at ``$1042AD``. A
matching byte and event sequence proves the observed copy operation only; it
does not establish the copied data's gameplay meaning or a rendered effect.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

from verify_theron_decoder_output_read_candidates import (
    DESTINATION_FIRST,
    EXPECTED_OUTPUT_SHA256,
    decoder_output,
    fields,
    parse_hex,
)


SOURCE_PC = 0x1042AB
SOURCE_CODE = "b1009de047c8e8b1"
WRITER_PC = 0x1042AD
WRITER_CODE = "9de047c8e8b1009d"


def parse_sequence(item: dict[str, str], path: Path, line_number: int) -> int:
    try:
        return int(item["event_sequence"], 10)
    except (KeyError, ValueError) as exc:
        raise ValueError(f"{path}:{line_number}: malformed event_sequence") from exc


def decoded_reads(
    path: Path, output: bytes, final_write_sequence: int
) -> tuple[list[tuple[int, int, int]], set[int]]:
    rows: list[tuple[int, int, int]] = []
    addresses: set[int] = set()
    with path.open(encoding="utf-8", errors="strict") as trace:
        header = fields(trace.readline())
        if header.get("event_sequence_source") != "TheronTraceCDSharedEventSequence":
            raise ValueError(f"{path}: consumer trace lacks the shared event-sequence header")
        for line_number, line in enumerate(trace, 2):
            if not line.startswith("main_ram_consumer_read "):
                continue
            item = fields(line)
            try:
                pc = parse_hex(item, "reader_physical_pc", 6)
                address = parse_hex(item, "physical_address", 6)
                value = parse_hex(item, "value", 2)
            except ValueError as exc:
                raise ValueError(f"{path}:{line_number}: {exc}") from exc
            if pc != SOURCE_PC or not DESTINATION_FIRST <= address < DESTINATION_FIRST + len(output):
                continue
            if item.get("reader_code_bytes") != SOURCE_CODE:
                raise ValueError(f"{path}:{line_number}: unexpected source instruction bytes")
            sequence = parse_sequence(item, path, line_number)
            if sequence <= final_write_sequence:
                continue
            offset = address - DESTINATION_FIRST
            addresses.add(address)
            if value == output[offset]:
                rows.append((sequence, value, address))
    return rows, addresses


def target_writes(path: Path, final_write_sequence: int) -> dict[int, tuple[int, int, int, int]]:
    writes: dict[int, tuple[int, int, int, int]] = {}
    with path.open(encoding="utf-8", errors="strict") as trace:
        header = fields(trace.readline())
        if header.get("event_sequence_source") != "TheronTraceCDSharedEventSequence":
            raise ValueError(f"{path}: target-write trace lacks the shared event-sequence header")
        for line_number, line in enumerate(trace, 2):
            if line.startswith("main_ram_target_write_truncated="):
                raise ValueError(f"{path}:{line_number}: target-write trace is truncated")
            if not line.startswith("main_ram_target_write "):
                continue
            item = fields(line)
            try:
                pc = parse_hex(item, "writer_physical_pc", 6)
                value = parse_hex(item, "value", 2)
                logical = parse_hex(item, "logical_address", 4)
                physical = parse_hex(item, "physical_address", 6)
                mpr2 = parse_hex(item, "mpr2", 2)
                sequence = parse_sequence(item, path, line_number)
            except ValueError as exc:
                raise ValueError(f"{path}:{line_number}: {exc}") from exc
            if pc != WRITER_PC or sequence <= final_write_sequence:
                continue
            if item.get("writer_code_bytes") != WRITER_CODE:
                raise ValueError(f"{path}:{line_number}: unexpected destination instruction bytes")
            if ((mpr2 << 13) | (logical & 0x1FFF)) != physical:
                raise ValueError(f"{path}:{line_number}: logical/physical address disagrees with MPR2")
            if sequence in writes:
                raise ValueError(f"{path}:{line_number}: duplicate target-write event_sequence")
            writes[sequence] = (value, logical, physical, mpr2)
    return writes


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("decoder_trace", type=Path)
    parser.add_argument("consumer_trace", type=Path)
    parser.add_argument("target_write_trace", type=Path)
    parser.add_argument("--expected-output-sha256", default=EXPECTED_OUTPUT_SHA256)
    parser.add_argument("--require-copy", action="store_true")
    args = parser.parse_args()
    if not re.fullmatch(r"[0-9a-fA-F]{64}", args.expected_output_sha256):
        parser.error("--expected-output-sha256 must be 64 hexadecimal characters")
    try:
        output, final_write_sequence = decoder_output(
            args.decoder_trace, args.expected_output_sha256.lower()
        )
        reads, read_addresses = decoded_reads(
            args.consumer_trace, output, final_write_sequence
        )
        writes = target_writes(args.target_write_trace, final_write_sequence)
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1

    pairs = [
        (sequence, value, address, writes[sequence + 1])
        for sequence, value, address in reads
        if sequence + 1 in writes and writes[sequence + 1][0] == value
    ]
    write_addresses = {target[2] for _, _, _, target in pairs}
    print(f"decoder_output_sha256={args.expected_output_sha256.lower()}")
    print(f"decoder_last_output_write_event_sequence={final_write_sequence}")
    print(f"post_decode_source_reads_at_1042ab={len(read_addresses)}")
    print(f"byte_equal_post_decode_source_reads_at_1042ab={len(reads)}")
    print(f"post_decode_target_writes_at_1042ad={len(writes)}")
    print(f"immediate_shared_sequence_byte_equal_copy_pairs={len(pairs)}")
    print(f"unique_source_addresses_copied={len({address for _, _, address, _ in pairs})}")
    print(f"unique_destination_addresses_written={len(write_addresses)}")
    print(
        "destination_physical_range="
        f"{min(write_addresses, default=0):06x}-{max(write_addresses, default=0):06x}"
    )
    print(f"destination_mpr2_values={','.join(f'{mpr:02x}' for mpr in sorted({target[3] for *_, target in pairs}))}")
    print("classification=verified byte-copy operation; gameplay meaning and rendered effect unproven")
    if args.require_copy and not pairs:
        print("FAIL: no immediate shared-sequence byte-copy pair", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
