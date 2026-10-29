#!/usr/bin/env python3
"""Join captured $23AD destination writes to later reads of those bytes.

This reports byte-equal RAM read candidates only. It does not prove code
ownership, semantic use, disc ancestry, or a gameplay transition.
"""

from __future__ import annotations

import argparse
import hashlib
import re
import sys
from collections import Counter
from pathlib import Path


DESTINATION_FIRST = 0x0D0000
DESTINATION_LENGTH = 5064
EXPECTED_OUTPUT_SHA256 = "9adf13522e818ee49de16ac1020a7697c3416877e6bc6baa918840d7e963f8ee"
FIELD = re.compile(r"([a-zA-Z0-9_]+)=([^ ]+)")


def fields(line: str) -> dict[str, str]:
    return dict(FIELD.findall(line.rstrip("\r\n")))


def parse_hex(values: dict[str, str], name: str, width: int) -> int:
    value = values.get(name)
    if value is None or not re.fullmatch(rf"[0-9a-fA-F]{{1,{width}}}", value):
        raise ValueError(f"missing or malformed {name}")
    return int(value, 16)


def decoder_output(path: Path, expected_sha256: str) -> tuple[bytes, int]:
    writes: list[tuple[int, int, int]] = []
    return_events: list[int] = []
    with path.open(encoding="utf-8", errors="strict") as trace:
        for line_number, line in enumerate(trace, 1):
            if "phase=decoder" not in line:
                continue
            item = fields(line)
            if item.get("phase") == "decoder" and item.get("kind") == "return_rts":
                try:
                    return_events.append(int(item["event_sequence"], 10))
                except (KeyError, ValueError) as exc:
                    raise ValueError(f"{path}:{line_number}: malformed return event") from exc
            if item.get("phase") != "decoder" or item.get("kind") != "mem_write":
                continue
            try:
                address = parse_hex(item, "physical", 6)
                value = parse_hex(item, "value", 2)
                event_sequence = int(item["event_sequence"], 10)
            except (KeyError, ValueError) as exc:
                raise ValueError(f"{path}:{line_number}: malformed decoder write: {exc}") from exc
            if DESTINATION_FIRST <= address < DESTINATION_FIRST + DESTINATION_LENGTH:
                writes.append((address, value, event_sequence))

    if len(return_events) != 1:
        raise ValueError(f"expected one decoder return_rts event; found {len(return_events)}")
    if len(writes) != DESTINATION_LENGTH:
        raise ValueError(
            f"expected {DESTINATION_LENGTH} destination writes; found {len(writes)}"
        )
    expected_addresses = list(range(DESTINATION_FIRST, DESTINATION_FIRST + DESTINATION_LENGTH))
    actual_addresses = [address for address, _, _ in writes]
    if actual_addresses != expected_addresses:
        raise ValueError("destination writes are duplicated, missing, or out of order")
    sequences = [sequence for _, _, sequence in writes]
    if any(right <= left for left, right in zip(sequences, sequences[1:])):
        raise ValueError("destination write event sequences are not strictly increasing")
    output = bytes(value for _, value, _ in writes)
    digest = hashlib.sha256(output).hexdigest()
    if sequences[-1] >= return_events[0]:
        raise ValueError("decoder destination writes do not precede the return event")
    if digest != expected_sha256:
        raise ValueError(f"unexpected destination SHA-256: {digest}")
    return output, sequences[-1]


def read_candidates(
    path: Path, output: bytes, final_write_event_sequence: int
) -> tuple[int, set[int], Counter]:
    seen = 0
    matching_addresses: set[int] = set()
    pc_groups: Counter = Counter()
    with path.open(encoding="utf-8", errors="strict") as trace:
        header = fields(trace.readline())
        if header.get("event_sequence_source") != "TheronTraceCDSharedEventSequence":
            raise ValueError(
                f"{path}: consumer trace lacks the shared event-sequence header; "
                "capture with the PCE Fast consumer-range patch"
            )
        for line_number, line in enumerate(trace, 1):
            if not line.startswith("main_ram_consumer_read "):
                continue
            item = fields(line)
            try:
                address = parse_hex(item, "physical_address", 6)
                value = parse_hex(item, "value", 2)
            except (KeyError, ValueError) as exc:
                raise ValueError(f"{path}:{line_number}: malformed consumer read: {exc}") from exc
            if not DESTINATION_FIRST <= address < DESTINATION_FIRST + len(output):
                continue
            try:
                pc = parse_hex(item, "reader_physical_pc", 6)
                code = item.get("reader_code_bytes", "")
                event_sequence = int(item["event_sequence"], 10)
            except (KeyError, ValueError) as exc:
                raise ValueError(f"{path}:{line_number}: malformed output-range read: {exc}") from exc
            if event_sequence <= final_write_event_sequence:
                continue
            seen += 1
            offset = address - DESTINATION_FIRST
            if output[offset] != value:
                continue
            matching_addresses.add(address)
            pc_groups[(pc, code)] += 1
    return seen, matching_addresses, pc_groups


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("decoder_trace", type=Path)
    parser.add_argument("consumer_trace", type=Path)
    parser.add_argument(
        "--require-candidate",
        action="store_true",
        help="return failure when no post-decode byte-equal output read is found",
    )
    parser.add_argument(
        "--expected-output-sha256",
        default=EXPECTED_OUTPUT_SHA256,
        help="expected decoded-output digest (defaults to the captured US resource)",
    )
    args = parser.parse_args()
    if not re.fullmatch(r"[0-9a-fA-F]{64}", args.expected_output_sha256):
        parser.error("--expected-output-sha256 must be 64 hexadecimal characters")
    try:
        output, final_write_sequence = decoder_output(
            args.decoder_trace, args.expected_output_sha256.lower()
        )
        reads, addresses, groups = read_candidates(
            args.consumer_trace, output, final_write_sequence
        )
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1

    print(f"decoder_output_bytes={len(output)}")
    print(f"decoder_output_sha256={hashlib.sha256(output).hexdigest()}")
    print(f"post_decode_reads_in_output_range={reads}")
    print(f"byte_equal_read_count={sum(groups.values())}")
    print(f"unique_output_addresses_read={len(addresses)}")
    for (pc, code), count in sorted(groups.items()):
        print(f"read_candidate physical_pc={pc:06x} code_bytes={code or 'unavailable'} reads={count}")
    print("classification=byte-equal read candidates; ownership and gameplay use unproven")
    if args.require_candidate and not addresses:
        print("FAIL: no post-decode byte-equal output read candidate", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
