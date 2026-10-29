#!/usr/bin/env python3
"""Join authentic $23AD output writes to PCE Fast code-window RAM writes."""

from __future__ import annotations

import argparse
import re
import sys
from collections import Counter
from pathlib import Path

from verify_theron_decoder_output_read_candidates import (
    DESTINATION_FIRST,
    DESTINATION_LENGTH,
    EXPECTED_OUTPUT_SHA256,
    decoder_output,
    fields,
    parse_hex,
)


DESTINATION_END = DESTINATION_FIRST + DESTINATION_LENGTH


def read_decoder_writes(path: Path) -> tuple[dict[int, tuple[int, int]], int]:
    writes: dict[int, tuple[int, int]] = {}
    returns: list[int] = []
    with path.open(encoding="utf-8", errors="strict") as trace:
        for line_number, line in enumerate(trace, 1):
            item = fields(line)
            if item.get("phase") != "decoder":
                continue
            if item.get("kind") == "return_rts":
                try:
                    returns.append(int(item["event_sequence"], 10))
                except (KeyError, ValueError) as exc:
                    raise ValueError(f"{path}:{line_number}: malformed decoder return") from exc
            if item.get("kind") != "mem_write":
                continue
            try:
                address = parse_hex(item, "physical", 6)
                value = parse_hex(item, "value", 2)
                sequence = int(item["event_sequence"], 10)
            except (KeyError, ValueError) as exc:
                raise ValueError(f"{path}:{line_number}: malformed decoder write") from exc
            if DESTINATION_FIRST <= address < DESTINATION_END:
                if sequence in writes:
                    raise ValueError(f"{path}:{line_number}: duplicate decoder event_sequence")
                writes[sequence] = (address, value)
    if len(returns) != 1:
        raise ValueError(f"expected one decoder return; found {len(returns)}")
    return writes, returns[0]


def read_code_writes(path: Path) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    prior_sequence = -1
    with path.open(encoding="utf-8", errors="strict") as trace:
        header = fields(trace.readline())
        if header.get("event_sequence_source") != "TheronTraceCDSharedEventSequence":
            raise ValueError(f"{path}: code-write trace lacks the shared event-sequence header")
        for line_number, line in enumerate(trace, 2):
            if not line.startswith("pce_fast_code_ram_write "):
                continue
            item = fields(line)
            try:
                sequence = int(item["event_sequence"], 10)
                address = parse_hex(item, "physical_address", 6)
                parse_hex(item, "value", 2)
                pc = parse_hex(item, "writer_physical_pc", 6)
            except (KeyError, ValueError) as exc:
                raise ValueError(f"{path}:{line_number}: malformed code-write row") from exc
            if sequence <= prior_sequence:
                raise ValueError(f"{path}:{line_number}: code-write event sequence is not increasing")
            if not 0x1F1C00 <= pc < 0x1F1E00:
                raise ValueError(f"{path}:{line_number}: code-write PC is outside the configured window")
            if not DESTINATION_FIRST <= address < DESTINATION_END:
                raise ValueError(f"{path}:{line_number}: code-write destination is outside decoder output")
            item["_sequence"] = str(sequence)
            item["_address"] = f"{address:06x}"
            item["_value"] = str(parse_hex(item, "value", 2))
            item["_pc"] = f"{pc:06x}"
            rows.append(item)
            prior_sequence = sequence
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("decoder_trace", type=Path)
    parser.add_argument("code_write_trace", type=Path)
    parser.add_argument("--expected-output-sha256", default=EXPECTED_OUTPUT_SHA256)
    parser.add_argument("--require-complete", action="store_true")
    args = parser.parse_args()
    if not re.fullmatch(r"[0-9a-fA-F]{64}", args.expected_output_sha256):
        parser.error("--expected-output-sha256 must be 64 hexadecimal characters")
    try:
        _, final_write_sequence = decoder_output(
            args.decoder_trace, args.expected_output_sha256.lower()
        )
        decoder_writes, return_sequence = read_decoder_writes(args.decoder_trace)
        code_writes = read_code_writes(args.code_write_trace)
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1

    matches: Counter[str] = Counter()
    matched_sequences: set[int] = set()
    unmatched_after_return = 0
    for item in code_writes:
        sequence = int(item["_sequence"])
        expected = decoder_writes.get(sequence - 1)
        observed = (int(item["_address"], 16), int(item["_value"]))
        if expected == observed:
            matches[item["_pc"]] += 1
            matched_sequences.add(sequence - 1)
        elif sequence > return_sequence:
            unmatched_after_return += 1

    unmatched_decoder_writes = len(decoder_writes) - len(matched_sequences)
    print(f"decoder_output_sha256={args.expected_output_sha256.lower()}")
    print(f"decoder_output_bytes={len(decoder_writes)}")
    print(f"decoder_last_output_write_event_sequence={final_write_sequence}")
    print(f"decoder_return_event_sequence={return_sequence}")
    print(f"code_window_write_rows={len(code_writes)}")
    print(f"exact_decoder_write_matches={len(matched_sequences)}")
    print(f"decoder_output_writes_without_exact_code_trace_match={unmatched_decoder_writes}")
    print(f"unmatched_code_window_writes_after_decoder_return={unmatched_after_return}")
    for pc, count in sorted(matches.items()):
        print(f"decoder_writer_physical_pc={pc} exact_matches={count}")
    print("classification=address/value/shared-sequence join; unmatched writes are not attributed")
    if args.require_complete and unmatched_decoder_writes:
        print("FAIL: not every decoder output write has an exact code-window trace match", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
