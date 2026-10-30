#!/usr/bin/env python3
"""Classify source-bound $EA9C write candidates for the observed JP RAM loop.

The report joins bounded instruction-fetch snapshots at physical $1000D2-$1000E3
to the latest prior write in the $EA9C-only trace, then to the immediately
preceding authentic $EA99 CD read. It does not observe writes from every guest
PC and therefore cannot establish exclusive RAM provenance or gameplay handoff.
"""

from __future__ import annotations

import argparse
import hashlib
import sys
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


FOLLOW_SUFFIX = ".indirect-jump-follow"
DISPATCH_SUFFIX = ".indirect-jump"
CODE_FOLLOW_SUFFIX = ".system-card-code-follow"
LOOP_PHYSICAL_RANGE = (0x1000D2, 0x1000E3)
LOOP_LOGICAL_RANGE = (0x40D2, 0x40E3)


def ensure_capture(paths: list[Path]) -> None:
    if len({path.parent.resolve() for path in paths}) != 1:
        raise ValueError("trace files are not from one capture directory")
    suffixes = (
        DISPATCH_SUFFIX,
        CODE_FOLLOW_SUFFIX,
        FOLLOW_SUFFIX,
        ".system-card-dispatch",
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
) -> dict[str, int]:
    reads_by_event = {int(row["event_sequence"]): row for row in reads}
    writes_by_address: dict[int, list[dict[str, str]]] = defaultdict(list)
    for row in writes:
        writes_by_address[int(row["physical_address"], 16)].append(row)

    observed_addresses: set[int] = set()
    observed_bytes = matching_writes = source_read_matches = 0
    mismatching_writes = missing_writes = missing_source_reads = 0
    fully_backed_fetch_rows = 0
    loop_fetch_rows = 0

    for row in fetch_rows:
        logical_pc = int(row["logical_pc"], 16)
        values = bytes.fromhex(row["bytes"])
        addresses = [int(value, 16) for value in row["physical_bytes"].split(",")]
        valid_mask = int(row["valid_mask"])
        event = int(row["event_sequence"])
        byte_results: list[bool] = []
        for offset, (address, value) in enumerate(zip(addresses, values, strict=True)):
            if not LOOP_PHYSICAL_RANGE[0] <= address <= LOOP_PHYSICAL_RANGE[1]:
                continue
            if not LOOP_LOGICAL_RANGE[0] <= logical_pc + offset <= LOOP_LOGICAL_RANGE[1]:
                continue
            if not valid_mask & (1 << offset):
                continue
            observed_bytes += 1
            observed_addresses.add(address)
            prior = [item for item in writes_by_address.get(address, ())
                     if int(item["event_sequence"]) < event]
            if not prior:
                missing_writes += 1
                byte_results.append(False)
                continue
            latest = prior[-1]
            write_event = int(latest["event_sequence"])
            if int(latest["value"], 16) != value:
                mismatching_writes += 1
                byte_results.append(False)
                continue
            matching_writes += 1
            read = reads_by_event.get(write_event - 1)
            if (
                read is None
                or read.get("source_valid") != "1"
                or read.get("reader_pc") != "ea99"
                or int(read["value"], 16) != value
            ):
                missing_source_reads += 1
                byte_results.append(False)
                continue
            source_read_matches += 1
            byte_results.append(True)
        if byte_results:
            loop_fetch_rows += 1
            if all(byte_results):
                fully_backed_fetch_rows += 1

    return {
        "loop_fetch_rows": loop_fetch_rows,
        "observed_fetch_bytes": observed_bytes,
        "observed_unique_physical_bytes": len(observed_addresses),
        "expected_unique_physical_bytes": LOOP_PHYSICAL_RANGE[1] - LOOP_PHYSICAL_RANGE[0] + 1,
        "matching_latest_prior_ea9c_writes": matching_writes,
        "mismatching_latest_prior_ea9c_writes": mismatching_writes,
        "fetch_bytes_without_prior_ea9c_write": missing_writes,
        "matches_with_immediately_prior_source_valid_ea99_read": source_read_matches,
        "matching_writes_without_immediately_prior_source_valid_ea99_read": missing_source_reads,
        "fetch_rows_fully_candidate_backed": fully_backed_fetch_rows,
    }


def verify(
    dispatch_path: Path,
    code_follow_path: Path,
    follow_path: Path,
    system_card_dispatch_path: Path,
    read_path: Path,
    write_path: Path,
    track02_path: Path,
) -> dict[str, int]:
    ensure_capture([
        dispatch_path,
        code_follow_path,
        follow_path,
        system_card_dispatch_path,
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
    fetch_rows = parse_follow_rows(follow_path)
    report = classify(fetch_rows, reads, writes)
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
            root / "trace.log.cd-data-port-read",
            root / "trace.log.cd-ram-writer-pc",
            args.track02,
        )
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    for key, value in report.items():
        print(f"{key}={value}")
    print("LIMIT: $EA9C is a bounded writer-PC trace; these candidate joins do not prove exclusive RAM provenance or gameplay handoff")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
