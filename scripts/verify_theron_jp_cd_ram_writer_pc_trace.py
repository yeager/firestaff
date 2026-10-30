#!/usr/bin/env python3
"""Validate source-bound System Card CD-RAM writes and executed code fetches.

The verifier authenticates disc-read bytes and reports ordered/value candidate
joins to writes. Repeated byte values can make a candidate join non-unique; this
does not prove transfer causality, full game loading, or gameplay.
"""

from __future__ import annotations

import argparse
from bisect import bisect_left
from collections import defaultdict, deque
import hashlib
import sys
from pathlib import Path

TRACK02_MD5 = "b7afb338ad31be1025b53f9aff12d73a"
TRACK02_SIZE = 8_102_640
TRACK02_START_LBA = 3_590
TRACK02_INDEX1_OFFSET = 526_848
TRACK02_SECTOR_BYTES = 2_352
TRACK02_USER_OFFSET = 16
READ_SUFFIX = ".cd-data-port-read"
WRITE_SUFFIX = ".cd-ram-writer-pc"
DISPATCH_SUFFIX = ".system-card-dispatch"
READ_HEADER = "source=mednafen-pce-fast-cd-data-port-read-with-reader-mpr-context"


def parse_fields(line: str, prefix: str, line_number: int) -> dict[str, str]:
    if not line.startswith(prefix + " "):
        raise ValueError(f"expected {prefix} at line {line_number}")
    fields: dict[str, str] = {}
    for item in line[len(prefix) + 1 :].split():
        if "=" not in item:
            raise ValueError(f"malformed field at line {line_number}")
        key, value = item.split("=", 1)
        if key in fields and fields[key] != value:
            raise ValueError(f"conflicting {key} at line {line_number}")
        fields[key] = value
    return fields


def ensure_capture_pair(paths: list[Path]) -> None:
    suffixes = [WRITE_SUFFIX, READ_SUFFIX, DISPATCH_SUFFIX]
    bases = []
    for path, suffix in zip(paths, suffixes, strict=True):
        if path.parent.resolve() != paths[0].parent.resolve() or not path.name.endswith(suffix):
            raise ValueError("trace files are not from one capture directory")
        bases.append(path.name[: -len(suffix)])
    if len(set(bases)) != 1:
        raise ValueError("trace files do not share one capture name")


def verify_read_rows(path: Path, track02: bytes) -> tuple[list[dict[str, str]], int]:
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != READ_HEADER:
        raise ValueError("unexpected CD data-port trace header")
    rows: list[dict[str, str]] = []
    previous_event = -1
    unbound = 0
    for line_no, line in enumerate(lines[1:], 2):
        row = parse_fields(line, "cd_data_port_read", line_no)
        event = int(row["event_sequence"])
        if event <= previous_event:
            raise ValueError(f"non-increasing CD read event sequence at line {line_no}")
        previous_event = event
        if row.get("address") != "1808":
            raise ValueError(f"unexpected CD data-port address at line {line_no}")
        pc = int(row["reader_pc"], 16)
        cpu_pc = int(row["cpu_pc"], 16)
        slot = int(row["reader_mpr_slot"])
        if pc != cpu_pc or slot != (pc >> 13) & 7:
            raise ValueError(f"invalid CD reader mapping at line {line_no}")
        mpr = int(row[f"reader_mpr{slot}"], 16)
        if int(row["reader_physical_pc"], 16) != (mpr << 13) | (pc & 0x1FFF):
            raise ValueError(f"invalid CD reader physical PC at line {line_no}")
        if row.get("source_valid") != "1":
            unbound += 1
            rows.append(row)
            continue
        lba = int(row["source_lba"])
        user_offset = int(row["source_user_offset"])
        raw_offset = int(row["source_raw_offset"])
        if (
            lba < TRACK02_START_LBA
            or
            int(row["track02_start_lba"]) != TRACK02_START_LBA
            or int(row["track02_index1_file_offset"]) != TRACK02_INDEX1_OFFSET
            or int(row["track02_sector_bytes"]) != TRACK02_SECTOR_BYTES
            or int(row["track02_end_lba"]) < lba + 1
            or raw_offset < TRACK02_USER_OFFSET
            or raw_offset >= TRACK02_SECTOR_BYTES
        ):
            raise ValueError(f"unexpected authentic-disc source metadata at line {line_no}")
        expected_file_offset = (
            TRACK02_INDEX1_OFFSET
            + (lba - TRACK02_START_LBA) * TRACK02_SECTOR_BYTES
            + raw_offset
        )
        if int(row["track02_raw_file_offset"]) != expected_file_offset:
            raise ValueError(f"incorrect raw disc offset at line {line_no}")
        if expected_file_offset >= len(track02) or track02[expected_file_offset] != int(row["value"], 16):
            raise ValueError(f"CD read differs from authentic Track 02 at line {line_no}")
        if user_offset != raw_offset - TRACK02_USER_OFFSET:
            raise ValueError(f"incorrect user-data offset at line {line_no}")
        rows.append(row)
    return rows, unbound


def verify_writer_rows(path: Path) -> list[dict[str, str]]:
    lines = path.read_text(encoding="ascii").splitlines()
    expected_header = (
        "source=mednafen-pce-fast-cd-ram-writer-pc-write-v1 "
        "event_sequence_source=TheronTraceCDSharedEventSequence"
    )
    if not lines or lines[0] != expected_header:
        raise ValueError("unexpected CD RAM writer-PC trace header")
    rows: list[dict[str, str]] = []
    previous_event = -1
    for line_no, line in enumerate(lines[1:], 2):
        row = parse_fields(line, "cd_ram_writer_pc_write", line_no)
        if int(row["sequence"]) != len(rows):
            raise ValueError(f"non-contiguous writer sequence at line {line_no}")
        event = int(row["event_sequence"])
        if event <= previous_event:
            raise ValueError(f"non-increasing writer event sequence at line {line_no}")
        previous_event = event
        pc = int(row["writer_pc"], 16)
        if pc != 0xEA9C:
            raise ValueError(f"unexpected writer PC at line {line_no}")
        mpr7 = int(row["mpr7"], 16)
        if int(row["writer_physical_pc"], 16) != (mpr7 << 13) | (pc & 0x1FFF):
            raise ValueError(f"invalid writer physical PC at line {line_no}")
        address = int(row["physical_address"], 16)
        if int(row["target_bank"], 16) != address >> 13 or int(row["target_offset"], 16) != address & 0x1FFF:
            raise ValueError(f"invalid target address mapping at line {line_no}")
        rows.append(row)
    return rows


def verify_code_fetch_rows(path: Path) -> list[dict[str, str]]:
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != "source=mednafen-pce-fast-system-card-controller-dispatch-v2":
        raise ValueError("unexpected System Card dispatch trace header")
    rows: list[dict[str, str]] = []
    previous_event = -1
    for line_no, line in enumerate(lines[1:], 2):
        if not line.startswith("code_fetch "):
            if line.startswith(("system_card_controller ", "system_card_path ")):
                continue
            raise ValueError(f"unexpected System Card dispatch row at line {line_no}")
        row = parse_fields(line, "code_fetch", line_no)
        event = int(row["event_sequence"])
        if event <= previous_event:
            raise ValueError(f"non-increasing code-fetch event sequence at line {line_no}")
        previous_event = event
        pc = int(row["logical_pc"], 16)
        if not 0x4000 <= pc <= 0x5FFF:
            raise ValueError(f"code-fetch PC outside traced window at line {line_no}")
        mpr = [int(row[f"mpr{i}"], 16) for i in range(8)]
        slot = (pc >> 13) & 7
        expected_pc = (mpr[slot] << 13) | (pc & 0x1FFF)
        if int(row["physical_pc"], 16) != expected_pc:
            raise ValueError(f"invalid code-fetch physical PC at line {line_no}")
        if int(row["valid_mask"], 16) != 7:
            raise ValueError(f"incomplete opcode-byte snapshot at line {line_no}")
        if len(row["bytes"]) != 6 or len(row["physical_bytes"].split(",")) != 3:
            raise ValueError(f"invalid opcode-byte encoding at line {line_no}")
        for offset, physical_byte in enumerate(row["physical_bytes"].split(",")):
            logical = pc + offset
            expected_byte = (mpr[(logical >> 13) & 7] << 13) | (logical & 0x1FFF)
            if int(physical_byte, 16) != expected_byte:
                raise ValueError(f"invalid code-fetch physical byte address at line {line_no}")
        rows.append(row)
    return rows


def link_reads_to_writes(reads: list[dict[str, str]], writes: list[dict[str, str]]) -> tuple[int, int]:
    source_reads = [
        row
        for row in reads
        if row.get("source_valid") == "1"
        and row.get("reader_pc") == "ea99"
    ]
    reads_by_value: dict[str, deque[int]] = defaultdict(deque)
    next_read = 0
    matched = 0
    for write in writes:
        event = int(write["event_sequence"])
        while next_read < len(source_reads) and int(source_reads[next_read]["event_sequence"]) < event:
            row = source_reads[next_read]
            reads_by_value[row["value"]].append(next_read)
            next_read += 1
        candidates = reads_by_value[write["value"]]
        while candidates and event - int(source_reads[candidates[0]]["event_sequence"]) > 16:
            candidates.popleft()
        if candidates:
            candidates.pop()
            matched += 1
    return matched, len(writes) - matched

def link_code_to_writes(code_rows: list[dict[str, str]], writes: list[dict[str, str]]) -> tuple[int, int, int]:
    writes_by_address: dict[int, list[tuple[int, int]]] = defaultdict(list)
    for row in writes:
        writes_by_address[int(row["physical_address"], 16)].append(
            (int(row["event_sequence"]), int(row["value"], 16))
        )
    event_indexes = {
        address: [event for event, _ in address_writes]
        for address, address_writes in writes_by_address.items()
    }
    byte_matches = 0
    byte_mismatches = 0
    fully_backed = 0
    for row in code_rows:
        event = int(row["event_sequence"])
        values = bytes.fromhex(row["bytes"])
        addresses = [int(value, 16) for value in row["physical_bytes"].split(",")]
        row_backed = True
        for address, value in zip(addresses, values, strict=True):
            address_writes = writes_by_address.get(address)
            if not address_writes:
                row_backed = False
                continue
            latest_index = bisect_left(event_indexes[address], event) - 1
            if latest_index < 0:
                row_backed = False
                continue
            if address_writes[latest_index][1] == value:
                byte_matches += 1
            else:
                byte_mismatches += 1
                row_backed = False
        if row_backed:
            fully_backed += 1
    return byte_matches, byte_mismatches, fully_backed

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("writer_trace", type=Path)
    parser.add_argument("read_trace", type=Path)
    parser.add_argument("dispatch_trace", type=Path)
    parser.add_argument("track02", type=Path)
    args = parser.parse_args()
    try:
        ensure_capture_pair([args.writer_trace, args.read_trace, args.dispatch_trace])
        track02 = args.track02.read_bytes()
        if len(track02) != TRACK02_SIZE or hashlib.md5(track02).hexdigest() != TRACK02_MD5:
            raise ValueError("Track 02 is not authenticated Theron's Quest JP Rev. 1 media")
        reads, unbound = verify_read_rows(args.read_trace, track02)
        writes = verify_writer_rows(args.writer_trace)
        code_rows = verify_code_fetch_rows(args.dispatch_trace)
        read_write_matches, unmatched_writes = link_reads_to_writes(reads, writes)
        code_matches, code_mismatches, fully_backed = link_code_to_writes(code_rows, writes)
        print(f"authentic_track02={TRACK02_MD5}")
        print(f"source_bound_cd_reads={sum(row.get('source_valid') == '1' for row in reads)}")
        print(f"source_unbound_cd_reads={unbound}")
        print(f"system_card_ea99_writer_pc_writes={len(writes)}")
        print(f"ordered_value_read_write_candidate_matches={read_write_matches}")
        print(f"writer_pc_writes_without_ordered_value_candidate={unmatched_writes}")
        print(f"runtime_code_fetch_rows={len(code_rows)}")
        print(f"runtime_code_fetch_bytes_matching_prior_writer={code_matches}")
        print(f"runtime_code_fetch_bytes_mismatching_latest_writer={code_mismatches}")
        print(f"runtime_code_fetch_rows_fully_backed_by_prior_writes={fully_backed}")
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
