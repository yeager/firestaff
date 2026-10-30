#!/usr/bin/env python3
"""Reconcile observed writes at five JP code-fetch addresses with execution.

This checks event ordering and byte values only for addresses identified by the
2026-10-30 authentic JP cold capture. It does not authenticate disc reads,
prove a causal transfer, or establish gameplay or parity.
"""

from __future__ import annotations

import argparse
from bisect import bisect_left
from collections import defaultdict
import sys
from pathlib import Path

TARGET_ADDRESSES = (0x1000D9, 0x1000DA, 0x1000DB, 0x10468D, 0x10468E)
CAPTURED_ADDRESS_RANGES = (
    (0x0D04D2, 0x0D04E6),
    (0x0D089F, 0x0D08AB),
    (0x100000, 0x100010),
    (0x1000D9, 0x1000DB),
    (0x10468D, 0x10468E),
)
TARGET_SUFFIX = ".cd-ram-target-write"
FETCH_SUFFIX = ".system-card-dispatch"
TARGET_HEADER = "source=mednafen-pce-fast-cd-ram-target-write"
FETCH_HEADERS = {
    "source=mednafen-pce-fast-system-card-controller-dispatch-v2",
    "source=mednafen-pce-fast-system-card-controller-dispatch-v3 code_fetch_scope=targeted",
    "source=mednafen-pce-fast-system-card-controller-dispatch-v4 code_fetch_scope=targeted-consumer-boundaries",
}


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


def ensure_capture_pair(target_path: Path, fetch_path: Path) -> None:
    if target_path.parent.resolve() != fetch_path.parent.resolve():
        raise ValueError("trace files are not from one capture directory")
    if not target_path.name.endswith(TARGET_SUFFIX) or not fetch_path.name.endswith(FETCH_SUFFIX):
        raise ValueError("unexpected trace filename suffix")
    if target_path.name[:-len(TARGET_SUFFIX)] != fetch_path.name[:-len(FETCH_SUFFIX)]:
        raise ValueError("trace files do not share one capture name")


def read_target_writes(path: Path) -> dict[int, list[tuple[int, int, int]]]:
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != TARGET_HEADER:
        raise ValueError("unexpected CD-RAM target-write trace header")
    writes: dict[int, list[tuple[int, int, int]]] = defaultdict(list)
    previous_event = -1
    for line_number, line in enumerate(lines[1:], 2):
        if line.startswith("cd_ram_target_write_truncated "):
            raise ValueError(f"target-write trace exhausted at line {line_number}")
        row = parse_fields(line, "cd_ram_target_write", line_number)
        if int(row["sequence"]) != line_number - 2:
            raise ValueError(f"non-contiguous target-write sequence at line {line_number}")
        event = int(row["event_sequence"])
        if event <= previous_event:
            raise ValueError(f"non-increasing target-write event sequence at line {line_number}")
        previous_event = event
        address = int(row["physical_address"], 16)
        pc = int(row["writer_pc"], 16)
        mprs = [int(row[f"mpr{i}"], 16) for i in range(8)]
        if int(row["writer_physical_pc"], 16) != (mprs[(pc >> 13) & 7] << 13) | (pc & 0x1FFF):
            raise ValueError(f"invalid writer physical PC at line {line_number}")
        if not any(first <= address <= last for first, last in CAPTURED_ADDRESS_RANGES):
            raise ValueError(f"unexpected target-write address at line {line_number}")
        if not 0 <= int(row["value"], 16) <= 0xFF:
            raise ValueError(f"invalid target-write byte value at line {line_number}")
        if address in TARGET_ADDRESSES:
            writes[address].append((event, int(row["value"], 16), pc))
    return writes


def reconcile(target_path: Path, fetch_path: Path) -> tuple[dict[int, int], dict[int, int], dict[int, int], int, int, int, int]:
    writes = read_target_writes(target_path)
    events = {address: [item[0] for item in rows] for address, rows in writes.items()}
    lines = fetch_path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] not in FETCH_HEADERS:
        raise ValueError("unexpected System Card dispatch trace header")
    is_v4 = lines[0].endswith("code_fetch_scope=targeted-consumer-boundaries")
    previous_event = -1
    fetch_bytes = defaultdict(int)
    write_counts = {address: len(writes.get(address, ())) for address in TARGET_ADDRESSES}
    non_ea9c_writes = {
        address: sum(pc != 0xEA9C for _, _, pc in writes.get(address, ()))
        for address in TARGET_ADDRESSES
    }
    matched = mismatched = missing = 0
    target_rows: dict[int, list[bool]] = {}
    for line_number, line in enumerate(lines[1:], 2):
        if line.startswith("code_fetch_truncated "):
            raise ValueError(f"incomplete code-fetch trace at line {line_number}")
        if line.startswith("code_fetch_extended "):
            prefix = "code_fetch_extended"
        elif line.startswith("code_fetch "):
            prefix = "code_fetch"
        else:
            if line.startswith(("system_card_controller ", "system_card_path ")):
                continue
            raise ValueError(f"unexpected dispatch trace row at line {line_number}")
        row = parse_fields(line, prefix, line_number)
        event = int(row["event_sequence"])
        if event <= previous_event:
            raise ValueError(f"non-increasing code-fetch event sequence at line {line_number}")
        previous_event = event
        pc = int(row["logical_pc"], 16)
        if is_v4:
            allowed_pcs = (
                {0x40D9, 0x42AB, 0x42B2, 0x44C1, 0x44D2}
                if prefix == "code_fetch"
                else {0x6138, 0x613A, 0x613D, 0x613F, 0x6142, 0x6144, 0x6148}
            )
            if pc not in allowed_pcs:
                raise ValueError(f"code-fetch PC outside v4 target set at line {line_number}")
        mprs = [int(row[f"mpr{i}"], 16) for i in range(8)]
        pc_range = (0x6000, 0x7FFF) if prefix == "code_fetch_extended" else (0x4000, 0x5FFF)
        if not pc_range[0] <= pc <= pc_range[1]:
            raise ValueError(f"code-fetch PC outside traced window at line {line_number}")
        expected_pc = (mprs[(pc >> 13) & 7] << 13) | (pc & 0x1FFF)
        if int(row["physical_pc"], 16) != expected_pc:
            raise ValueError(f"invalid code-fetch physical PC at line {line_number}")
        if int(row["valid_mask"], 16) != 7 or len(row["bytes"]) != 6:
            raise ValueError(f"incomplete code-fetch snapshot at line {line_number}")
        addresses = [int(value, 16) for value in row["physical_bytes"].split(",")]
        values = bytes.fromhex(row["bytes"])
        if len(addresses) != 3:
            raise ValueError(f"invalid physical-byte list at line {line_number}")
        for offset, address in enumerate(addresses):
            logical = pc + offset
            expected_address = (mprs[(logical >> 13) & 7] << 13) | (logical & 0x1FFF)
            if address != expected_address:
                raise ValueError(f"invalid code-fetch physical byte address at line {line_number}")
        row_checks: list[bool] = []
        for address, value in zip(addresses, values):
            if address not in TARGET_ADDRESSES:
                continue
            fetch_bytes[address] += 1
            address_writes = writes.get(address, [])
            latest_index = bisect_left(events.get(address, []), event) - 1
            if latest_index < 0:
                missing += 1
                row_checks.append(False)
            elif address_writes[latest_index][1] != value:
                mismatched += 1
                row_checks.append(False)
            else:
                matched += 1
                row_checks.append(True)
        if row_checks:
            target_rows[event] = row_checks
    if not any(fetch_bytes.values()):
        raise ValueError("no code fetches reached the five target addresses")
    for address in TARGET_ADDRESSES:
        if not writes.get(address):
            raise ValueError(f"no target writes captured for {address:06x}")
    fully_backed_rows = sum(all(checks) for checks in target_rows.values())
    return write_counts, dict(fetch_bytes), non_ea9c_writes, matched, mismatched, missing, fully_backed_rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("target_write_trace", type=Path)
    parser.add_argument("dispatch_trace", type=Path)
    args = parser.parse_args()
    try:
        ensure_capture_pair(args.target_write_trace, args.dispatch_trace)
        write_counts, fetch_bytes, other_pc_writes, matched, mismatched, missing, backed_rows = reconcile(
            args.target_write_trace, args.dispatch_trace
        )
        for address in TARGET_ADDRESSES:
            print(f"address_{address:06x}_writes={write_counts[address]}")
            print(f"address_{address:06x}_non_ea9c_writes={other_pc_writes[address]}")
            print(f"address_{address:06x}_fetch_bytes={fetch_bytes.get(address, 0)}")
        print(f"target_fetch_bytes_with_prior_matching_write={matched}")
        print(f"target_fetch_bytes_mismatching_latest_prior_write={mismatched}")
        print(f"target_fetch_bytes_without_prior_observed_write={missing}")
        print(f"target_fetch_rows_fully_backed={backed_rows}")
        if mismatched or missing:
            return 1
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
