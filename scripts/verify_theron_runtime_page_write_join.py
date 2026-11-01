#!/usr/bin/env python3
"""Join authentic MPR2=$68 instruction checkpoints to prior physical RAM writes."""

from __future__ import annotations

import argparse
import bisect
import hashlib
import re
import sys
from collections import Counter
from pathlib import Path

from analyze_theron_runtime_mpr_page_source import (
    read_dispatches,
)


CAPTURE_ID_RE = re.compile(r"^[0-9a-f]{32}$")
RUNTIME_HEADER_RE = re.compile(
    r"^source=mednafen-pce-fast-theron-c852-runtime-window-v3 "
    r"event_sequence_source=TheronTraceCDSharedEventSequence "
    r"capture_id=([0-9a-f]{32})$"
)
WRITER_HEADER_RE = re.compile(
    r"^source=mednafen-pce-fast-main-ram-target-write-v2 "
    r"event_sequence_source=TheronTraceCDSharedEventSequence "
    r"capture_id=([0-9a-f]{32}) range=([0-9a-f]{6}-[0-9a-f]{6}) "
    r"sample_limit=([0-9]+)$"
)
EXPECTED_RANGE = "0d0000-0d1fff"
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


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def read_receipt(path: Path) -> dict[str, str]:
    values: dict[str, str] = {}
    for line_number, line in enumerate(path.read_text(encoding="ascii").splitlines(), 1):
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        if key in values:
            raise ValueError(f"duplicate transition receipt field {key!r} on line {line_number}")
        values[key] = value
    return values


def require_capture_binding(runtime_path: Path, writer_path: Path,
                            receipt_path: Path) -> tuple[int, int, str]:
    with runtime_path.open(encoding="ascii") as runtime_stream:
        runtime_header = runtime_stream.readline().rstrip("\n")
    runtime_match = RUNTIME_HEADER_RE.fullmatch(runtime_header)
    if not runtime_match:
        raise ValueError("runtime checkpoint trace must use capture-bound schema v3")
    runtime_capture_id = runtime_match.group(1)

    with writer_path.open(encoding="ascii") as writer_stream:
        writer_header = writer_stream.readline().rstrip("\n")
    writer_match = WRITER_HEADER_RE.fullmatch(writer_header)
    if not writer_match:
        raise ValueError("all-writer RAM trace must use capture-bound schema v2")
    writer_capture_id, writer_range, writer_limit_text = writer_match.groups()
    if runtime_capture_id != writer_capture_id:
        raise ValueError("runtime and all-writer traces have different capture IDs")
    if writer_range != EXPECTED_RANGE:
        raise ValueError("all-writer trace does not cover the complete MPR2 $68 physical page")
    writer_limit = int(writer_limit_text)
    if not 4096 <= writer_limit <= 1048576:
        raise ValueError("all-writer trace has an invalid sample limit")

    receipt = read_receipt(receipt_path)
    if receipt.get("source") != "authentic-mednafen-transition-receipt":
        raise ValueError("unrecognized wrapper transition receipt")
    if not CAPTURE_ID_RE.fullmatch(receipt.get("capture_id", "")):
        raise ValueError("wrapper receipt lacks a valid capture ID")
    if receipt["capture_id"] != runtime_capture_id:
        raise ValueError("wrapper receipt and trace sidecars have different capture IDs")
    if receipt.get("pce_fast_main_ram_target_write_range") != EXPECTED_RANGE:
        raise ValueError("wrapper receipt does not record the complete target-write range")
    try:
        receipt_limit = int(receipt["pce_fast_main_ram_target_write_sample_limit"])
        receipt_rows = int(receipt["pce_fast_main_ram_target_write_rows"])
        runtime_rows = int(receipt["c852_runtime_window_rows"])
        runtime_complete = int(receipt["c852_runtime_window_complete"])
    except (KeyError, ValueError) as exc:
        raise ValueError("wrapper receipt lacks numeric trace counts") from exc
    if receipt_limit != writer_limit:
        raise ValueError("writer header and wrapper receipt sample limits differ")
    if receipt.get("pce_fast_main_ram_target_write_truncated") != "0":
        raise ValueError("wrapper receipt does not certify an uncapped writer trace")
    if receipt.get("c852_runtime_window_status") != "complete" or runtime_complete != 1:
        raise ValueError("wrapper receipt does not certify a complete C852 trace")
    if receipt.get("pce_fast_main_ram_target_write_sha256") != file_sha256(writer_path):
        raise ValueError("all-writer trace hash does not match the wrapper receipt")
    if receipt.get("c852_runtime_window_sha256") != file_sha256(runtime_path):
        raise ValueError("C852 trace hash does not match the wrapper receipt")
    return receipt_rows, runtime_rows, runtime_capture_id


def verify(runtime_path: Path, writer_path: Path, receipt_path: Path) -> tuple[dict[str, int], Counter[int]]:
    expected_writer_rows, expected_runtime_rows, _capture_id = require_capture_binding(
        runtime_path, writer_path, receipt_path
    )
    runtime_lines = runtime_path.read_text(encoding="ascii").splitlines()
    if not runtime_lines or not RUNTIME_HEADER_RE.fullmatch(runtime_lines[0]):
        raise ValueError("runtime checkpoint trace must use capture-bound schema v3")
    dispatches, _ = read_dispatches(runtime_path)
    if len(dispatches) != expected_runtime_rows:
        raise ValueError("wrapper receipt C852 row count does not match the sidecar")
    slots = fetch_slots(dispatches)
    if not slots:
        raise ValueError("runtime trace contains no instruction bytes in physical $0D0000-$0D1FFF")

    writes: dict[int, list[tuple[int, int, int]]] = {}
    previous_row_sequence = -1
    previous_event_sequence = -1
    with writer_path.open(encoding="ascii") as writer_stream:
        writer_stream.readline()
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
    if previous_row_sequence + 1 != expected_writer_rows:
        raise ValueError("wrapper receipt writer-row count does not match the sidecar")

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
    parser.add_argument("transition_receipt", type=Path)
    args = parser.parse_args()
    try:
        stats, writer_pcs = verify(
            args.runtime_window, args.all_writer_trace, args.transition_receipt
        )
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
