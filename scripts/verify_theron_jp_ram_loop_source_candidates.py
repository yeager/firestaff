#!/usr/bin/env python3
"""Classify bounded writer and source candidates for the observed JP RAM loop.

The report joins instruction-fetch snapshots at physical $1000D2-$1000E3 to
the latest prior all-PC target write at every address in that range. For
$EA9C writes it reports matching source-valid $EA99 CD-read candidates within
16 shared events. The available writer traces do not establish exclusive RAM
provenance or gameplay handoff.
"""

from __future__ import annotations

import argparse
import hashlib
import sys
from bisect import bisect_left
from collections import defaultdict
from pathlib import Path

from verify_theron_indirect_jump_trace import verify as verify_indirect_jump
from verify_theron_jp_cd_ram_writer_pc_trace import (
    READ_SUFFIX,
    TRACK02_MD5,
    TRACK02_SIZE,
    WRITE_SUFFIX,
    verify_code_fetch_rows,
    verify_read_rows,
    verify_writer_rows,
)
from verify_theron_jp_code_follow_trace import verify as verify_code_follow
from verify_theron_jp_indirect_jump_follow_trace import (
    FOLLOW_ROW,
    verify as verify_indirect_jump_follow,
)
from verify_theron_jp_fetch_target_writes import (
    TARGET_SUFFIX,
    read_target_writes,
)


FOLLOW_SUFFIX = ".indirect-jump-follow"
DISPATCH_SUFFIX = ".indirect-jump"
CODE_FOLLOW_SUFFIX = ".system-card-code-follow"
LOOP_PHYSICAL_RANGE = (0x1000D2, 0x1000E3)
LOOP_LOGICAL_RANGE = (0x40D2, 0x40E3)
HELPER_INSTRUCTIONS = {
    0x6109: bytes.fromhex("a9ea"),
    0x610B: bytes.fromhex("8dd940"),
    0x610E: bytes.fromhex("8dda40"),
    0x6111: bytes.fromhex("8ddb40"),
    0x6114: bytes.fromhex("60"),
}
HELPER_RETURN_PC = 0x432B


def ensure_capture(paths: list[Path]) -> None:
    if len({path.parent.resolve() for path in paths}) != 1:
        raise ValueError("trace files are not from one capture directory")
    suffixes = (
        DISPATCH_SUFFIX,
        CODE_FOLLOW_SUFFIX,
        FOLLOW_SUFFIX,
        ".system-card-dispatch",
        TARGET_SUFFIX,
        READ_SUFFIX,
        WRITE_SUFFIX,
    )
    bases = []
    for path, suffix in zip(paths, suffixes, strict=True):
        if not path.name.endswith(suffix):
            raise ValueError(f"unexpected trace filename suffix: {path.name}")
        bases.append(path.name[: -len(suffix)])
    if len(set(bases)) != 1:
        raise ValueError("trace files do not share one capture name")


def parse_follow_rows(path: Path) -> list[dict[str, str]]:
    lines = path.read_text(encoding="ascii").splitlines()
    rows: list[dict[str, str]] = []
    for line_number, line in enumerate(lines[1:], 2):
        match = FOLLOW_ROW.fullmatch(line)
        if match is None:
            if line.startswith(("indirect_jump_follow_complete ",
                                "indirect_jump_follow_truncated ",
                                "indirect_jump_follow_skipped ")):
                continue
            raise ValueError(f"malformed follow row at line {line_number}")
        fields = match.groups()
        rows.append({
            "dispatch_event": fields[0],
            "index": fields[3],
            "event_sequence": fields[2],
            "logical_pc": fields[4],
            "bytes": fields[6],
            "physical_bytes": ",".join(fields[7:10]),
            "valid_mask": fields[10],
        })
    return rows


def classify(
    fetch_rows: list[dict[str, str]],
    reads: list[dict[str, str]],
    writes: list[dict[str, str]],
    target_writes: dict[int, list[tuple[int, int, int]]],
) -> dict[str, int]:
    read_events_by_value: dict[int, list[int]] = defaultdict(list)
    for row in reads:
        if row.get("source_valid") == "1" and row.get("reader_pc") == "ea99":
            read_events_by_value[int(row["value"], 16)].append(int(row["event_sequence"]))
    writes_by_address: dict[int, list[dict[str, str]]] = defaultdict(list)
    for row in writes:
        writes_by_address[int(row["physical_address"], 16)].append(row)

    observed_addresses: set[int] = set()
    observed_bytes = matching_writes = source_read_matches = 0
    immediate_source_read_matches = unique_source_read_matches = 0
    ambiguous_source_read_matches = source_read_candidates_total = 0
    max_source_read_candidate_gap = 0
    mismatching_writes = missing_writes = missing_source_reads = 0
    non_ea9c_latest_writer_bytes = 0
    fully_backed_fetch_rows = 0
    fully_source_candidate_backed_fetch_rows = 0
    loop_fetch_rows = 0

    for row in fetch_rows:
        logical_pc = int(row["logical_pc"], 16)
        values = bytes.fromhex(row["bytes"])
        addresses = [int(value, 16) for value in row["physical_bytes"].split(",")]
        valid_mask = int(row["valid_mask"])
        event = int(row["event_sequence"])
        byte_results: list[bool] = []
        source_results: list[bool] = []
        for offset, (address, value) in enumerate(zip(addresses, values, strict=True)):
            if not LOOP_PHYSICAL_RANGE[0] <= address <= LOOP_PHYSICAL_RANGE[1]:
                continue
            if not LOOP_LOGICAL_RANGE[0] <= logical_pc + offset <= LOOP_LOGICAL_RANGE[1]:
                continue
            if not valid_mask & (1 << offset):
                continue
            observed_bytes += 1
            observed_addresses.add(address)
            if address in target_writes:
                prior = [item for item in target_writes[address] if item[0] < event]
            else:
                prior = [
                    (int(item["event_sequence"]), int(item["value"], 16), 0xEA9C)
                    for item in writes_by_address.get(address, ())
                    if int(item["event_sequence"]) < event
                ]
            if not prior:
                missing_writes += 1
                byte_results.append(False)
                source_results.append(False)
                continue
            write_event, write_value, writer_pc = max(prior, key=lambda item: item[0])
            if write_value != value:
                mismatching_writes += 1
                byte_results.append(False)
                source_results.append(False)
                continue
            matching_writes += 1
            if writer_pc != 0xEA9C:
                non_ea9c_latest_writer_bytes += 1
                byte_results.append(True)
                source_results.append(False)
            else:
                candidate_events = read_events_by_value.get(value, [])
                candidate_end = bisect_left(candidate_events, write_event)
                candidate_start = bisect_left(candidate_events, write_event - 16)
                candidates = candidate_events[candidate_start:candidate_end]
                if not candidates:
                    missing_source_reads += 1
                    byte_results.append(False)
                    source_results.append(False)
                    continue
                source_read_matches += 1
                source_read_candidates_total += len(candidates)
                if len(candidates) == 1:
                    unique_source_read_matches += 1
                else:
                    ambiguous_source_read_matches += 1
                nearest_gap = write_event - candidates[-1]
                max_source_read_candidate_gap = max(max_source_read_candidate_gap, nearest_gap)
                if nearest_gap == 1:
                    immediate_source_read_matches += 1
                byte_results.append(True)
                source_results.append(True)
        if byte_results:
            loop_fetch_rows += 1
            if all(byte_results):
                fully_backed_fetch_rows += 1
            if all(source_results):
                fully_source_candidate_backed_fetch_rows += 1

    return {
        "loop_fetch_rows": loop_fetch_rows,
        "observed_fetch_bytes": observed_bytes,
        "observed_unique_physical_bytes": len(observed_addresses),
        "expected_unique_physical_bytes": LOOP_PHYSICAL_RANGE[1] - LOOP_PHYSICAL_RANGE[0] + 1,
        "matching_latest_address_scoped_writes": matching_writes,
        "mismatching_latest_address_scoped_writes": mismatching_writes,
        "fetch_bytes_without_prior_address_scoped_write": missing_writes,
        "latest_non_ea9c_writer_byte_slots": non_ea9c_latest_writer_bytes,
        "matching_writes_with_source_valid_ea99_read_candidate_within_16_events": source_read_matches,
        "matching_writes_without_source_read_candidate_within_16_events": missing_source_reads,
        "unique_source_read_candidate_byte_slots": unique_source_read_matches,
        "ambiguous_source_read_candidate_byte_slots": ambiguous_source_read_matches,
        "source_read_candidate_matches_across_fetch_byte_slots": source_read_candidates_total,
        "nearest_source_read_candidate_gap_1": immediate_source_read_matches,
        "max_source_read_candidate_gap": max_source_read_candidate_gap,
        "fetch_rows_fully_candidate_backed": fully_backed_fetch_rows,
        "fetch_rows_fully_source_candidate_backed": fully_source_candidate_backed_fetch_rows,
    }


def classify_patch_helper(
    fetch_rows: list[dict[str, str]],
    reads: list[dict[str, str]],
    writes: list[dict[str, str]],
    target_writes: dict[int, list[tuple[int, int, int]]],
) -> dict[str, int]:
    rows_by_dispatch: dict[int, list[dict[str, str]]] = defaultdict(list)
    for row in fetch_rows:
        rows_by_dispatch[int(row["dispatch_event"])].append(row)

    read_events_by_value: dict[int, list[int]] = defaultdict(list)
    for row in reads:
        if row.get("source_valid") == "1" and row.get("reader_pc") == "ea99":
            read_events_by_value[int(row["value"], 16)].append(int(row["event_sequence"]))
    writes_by_address: dict[int, list[dict[str, str]]] = defaultdict(list)
    for row in writes:
        writes_by_address[int(row["physical_address"], 16)].append(row)

    calls = 0
    fetch_byte_slots = writer_matches = source_read_matches = 0
    unique_source_reads = ambiguous_source_reads = source_read_records = 0
    target_store_matches = 0
    for rows in rows_by_dispatch.values():
        rows.sort(key=lambda row: int(row["index"]))
        for start in range(len(rows) - len(HELPER_INSTRUCTIONS)):
            window = rows[start : start + len(HELPER_INSTRUCTIONS) + 1]
            expected_pcs = [*HELPER_INSTRUCTIONS, HELPER_RETURN_PC]
            if [int(row["logical_pc"], 16) for row in window] != expected_pcs:
                continue
            if [int(row["index"]) for row in window] != list(
                range(int(window[0]["index"]), int(window[0]["index"]) + len(window))
            ):
                continue
            calls += 1
            call_stores: list[tuple[int, int, int]] = []
            for row, (pc, expected) in zip(window[:-1], HELPER_INSTRUCTIONS.items(), strict=True):
                values = bytes.fromhex(row["bytes"])
                addresses = [int(value, 16) for value in row["physical_bytes"].split(",")]
                if values[:len(expected)] != expected:
                    raise ValueError(f"unexpected patch-helper instruction bytes at ${pc:04X}")
                event = int(row["event_sequence"])
                for address, value in zip(addresses[:len(expected)], values[:len(expected)], strict=True):
                    fetch_byte_slots += 1
                    prior = [item for item in writes_by_address.get(address, ())
                             if int(item["event_sequence"]) < event]
                    if not prior:
                        continue
                    latest = max(prior, key=lambda item: int(item["event_sequence"]))
                    write_event = int(latest["event_sequence"])
                    if int(latest["value"], 16) != value:
                        continue
                    writer_matches += 1
                    candidates = read_events_by_value.get(value, [])
                    end = bisect_left(candidates, write_event)
                    begin = bisect_left(candidates, write_event - 16)
                    matching_candidates = candidates[begin:end]
                    if matching_candidates:
                        source_read_matches += 1
                        source_read_records += len(matching_candidates)
                        if len(matching_candidates) == 1:
                            unique_source_reads += 1
                        else:
                            ambiguous_source_reads += 1

                if pc in (0x610B, 0x610E, 0x6111):
                    next_event = int(window[list(HELPER_INSTRUCTIONS).index(pc) + 1]["event_sequence"])
                    destination = 0x1000D9 + (pc - 0x610B) // 3
                    store_rows = [
                        (target_event, value, writer_pc)
                        for target_event, value, writer_pc in target_writes.get(destination, ())
                        if event < target_event < next_event
                        and writer_pc == pc
                        and value == 0xEA
                    ]
                    if len(store_rows) != 1:
                        raise ValueError(f"patch-helper STA at ${pc:04X} lacks one matching target write")
                    call_stores.extend(store_rows)
            target_store_matches += len(call_stores)

    if not calls:
        raise ValueError("follow trace contains no complete $6109-$6114 patch-helper call")
    return {
        "complete_source_patch_helper_calls": calls,
        "patch_helper_executed_fetch_bytes": fetch_byte_slots,
        "patch_helper_bytes_matching_latest_ea9c_write": writer_matches,
        "patch_helper_bytes_with_source_read_candidate_within_16_events": source_read_matches,
        "patch_helper_unique_source_read_candidate_bytes": unique_source_reads,
        "patch_helper_ambiguous_source_read_candidate_bytes": ambiguous_source_reads,
        "patch_helper_source_read_candidate_records": source_read_records,
        "patch_helper_target_writes_verified": target_store_matches,
    }


def verify(
    dispatch_path: Path,
    code_follow_path: Path,
    follow_path: Path,
    system_card_dispatch_path: Path,
    target_write_path: Path,
    read_path: Path,
    write_path: Path,
    track02_path: Path,
) -> dict[str, int]:
    ensure_capture([
        dispatch_path,
        code_follow_path,
        follow_path,
        system_card_dispatch_path,
        target_write_path,
        read_path,
        write_path,
    ])
    track02 = track02_path.read_bytes()
    if len(track02) != TRACK02_SIZE or hashlib.md5(track02).hexdigest() != TRACK02_MD5:
        raise ValueError("Track 02 is not authenticated Theron's Quest JP Rev. 1 media")

    if verify_code_follow(code_follow_path, require_complete=True) != ("complete", 256):
        raise ValueError("System Card code-follow verifier did not confirm 256 complete rows")
    verify_indirect_jump(dispatch_path, code_follow_path)
    verify_indirect_jump_follow(dispatch_path, code_follow_path, follow_path)
    verify_code_fetch_rows(system_card_dispatch_path)
    reads, unbound_reads = verify_read_rows(read_path, track02)
    if unbound_reads:
        raise ValueError(f"CD trace contains {unbound_reads} source-unbound reads")
    writes = verify_writer_rows(write_path)
    target_writes = read_target_writes(target_write_path)
    if any(not target_writes.get(address)
           for address in range(LOOP_PHYSICAL_RANGE[0], LOOP_PHYSICAL_RANGE[1] + 1)):
        raise ValueError("all-PC target-write trace does not cover $1000D2-$1000E3")
    fetch_rows = parse_follow_rows(follow_path)
    report = classify(fetch_rows, reads, writes, target_writes)
    report.update(classify_patch_helper(fetch_rows, reads, writes, target_writes))
    if report["observed_unique_physical_bytes"] != report["expected_unique_physical_bytes"]:
        raise ValueError("follow trace does not cover all 18 physical RAM-loop bytes")
    if not report["loop_fetch_rows"]:
        raise ValueError("follow trace contains no fetches in the bounded RAM-loop range")
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture_dir", type=Path)
    parser.add_argument("track02", type=Path)
    args = parser.parse_args()
    root = args.capture_dir
    try:
        report = verify(
            root / "trace.log.indirect-jump",
            root / "trace.log.system-card-code-follow",
            root / "trace.log.indirect-jump-follow",
            root / "trace.log.system-card-dispatch",
            root / "trace.log.cd-ram-target-write",
            root / "trace.log.cd-data-port-read",
            root / "trace.log.cd-ram-writer-pc",
            args.track02,
        )
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    for key, value in report.items():
        print(f"{key}={value}")
    print("LIMIT: all-PC writes cover the observed $1000D2-$1000E3 loop bytes; CD read matches are candidates and do not prove exclusive provenance or gameplay handoff")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
