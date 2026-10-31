#!/usr/bin/env python3
"""Bind instruction-start snapshots at `$4002/$4009` to authentic JP bytes."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import verify_theron_cd_ram_source_join as cd_ram_join
import verify_theron_jp_cd_data_port_trace as cd_read_verifier
import verify_theron_jp_syscard_bootstrap_follow as follow_verifier


FOLLOW_SUFFIX = ".system-card-bootstrap-follow"
WRITE_SUFFIX = ".cd-ram-target-write"
READ_SUFFIX = ".cd-data-port-read"
SNAPSHOT_SITES = {0x4002: 0x100002, 0x4009: 0x100009}
FETCH_PHYSICAL_FIRST = 0x100002
FETCH_PHYSICAL_LAST = 0x10000C
FETCH_MPR2 = 0x80
SOURCE_SPAN_FILE_OFFSET = 0x2973A2


def require_shared_sequence_headers(write_path: Path, read_path: Path) -> None:
    write_lines = write_path.read_text(encoding="ascii").splitlines()
    read_lines = read_path.read_text(encoding="ascii").splitlines()
    if not write_lines or write_lines[0] != cd_ram_join.WRITE_HEADER_SHARED_SEQUENCE:
        raise ValueError("CD-RAM target-write trace lacks the shared event-sequence source marker")
    if not read_lines or read_lines[0] != cd_read_verifier.TRACE_HEADER_WITH_READER_MPR_SHARED_SEQUENCE:
        raise ValueError("CD read trace lacks the shared event-sequence source marker")


def paired_capture_paths(follow_path: Path, write_path: Path, read_path: Path) -> None:
    paths = ((follow_path, FOLLOW_SUFFIX), (write_path, WRITE_SUFFIX), (read_path, READ_SUFFIX))
    roots = []
    for path, suffix in paths:
        if not path.name.endswith(suffix):
            raise ValueError(f"trace filename must end with {suffix}")
        roots.append(path.name[: -len(suffix)])
    if len({path.parent.resolve() for path, _ in paths}) != 1 or len(set(roots)) != 1:
        raise ValueError("bootstrap, CD read and CD write traces are not paired from one capture")


def verify_fetch_binding(
    fetch: dict[str, str],
    target_writes: list[dict[str, str]],
    source_joined_writes: list[dict[str, str]],
    expected_pc: int = 0x4002,
    expected_physical: int = 0x100002,
) -> dict[str, int]:
    try:
        fetch_event = int(fetch["event_sequence"])
        logical_pc = int(fetch["logical_pc"], 16)
        physical_pc = int(fetch["physical_pc"], 16)
        mpr2 = int(fetch["mpr2"], 16)
        valid_mask = int(fetch["valid_mask"], 16)
        fetch_bytes = bytes.fromhex(fetch["bytes"])
        physical_bytes = [int(value, 16) for value in fetch["physical_bytes"].split(",")]
    except (KeyError, ValueError) as error:
        raise ValueError("malformed instruction-start snapshot coordinates") from error
    if (
        logical_pc != expected_pc
        or physical_pc != expected_physical
        or mpr2 != FETCH_MPR2
        or len(fetch_bytes) != 4
        or len(physical_bytes) != 4
        or valid_mask != 0xF
        or physical_bytes != list(range(expected_physical, expected_physical + 4))
    ):
        raise ValueError(f"target is not a complete mapped `${expected_pc:04x}` instruction-start snapshot")

    latest_by_address: dict[int, tuple[int, int]] = {}
    for row in target_writes:
        try:
            event = int(row["event_sequence"])
            address = int(row["physical_address"], 16)
            value = int(row["value"], 16)
        except (KeyError, ValueError) as error:
            raise ValueError("malformed CD-RAM target-write row") from error
        if event < fetch_event and expected_physical <= address < expected_physical + 4:
            if address not in latest_by_address or event > latest_by_address[address][0]:
                latest_by_address[address] = (event, value)

    source_bound_events = {int(row["event_sequence"]) for row in source_joined_writes}
    result: dict[str, int] = {f"fetch_{expected_pc:04x}_event": fetch_event}
    for offset, expected in enumerate(fetch_bytes):
        address = expected_physical + offset
        latest = latest_by_address.get(address)
        if latest is None:
            raise ValueError(f"no recorded pre-snapshot write for physical byte 0x{address:06x}")
        event, value = latest
        if event not in source_bound_events:
            raise ValueError(f"latest pre-snapshot write at 0x{address:06x} is not source-validated")
        if value != expected:
            raise ValueError(f"latest source-validated write differs from snapshot at 0x{address:06x}")
        result[f"fetch_{expected_pc:04x}_byte_{offset}_write_event"] = event
    return result


def verify_source_span_before_fetch(
    target_writes: list[dict[str, str]],
    joined_rows: list[tuple[dict[str, int], dict[str, str]]],
    before_event: int,
) -> None:
    latest_by_address: dict[int, tuple[int, int]] = {}
    for row in target_writes:
        event = int(row["event_sequence"])
        address = int(row["physical_address"], 16)
        if event < before_event and FETCH_PHYSICAL_FIRST <= address <= FETCH_PHYSICAL_LAST:
            latest_by_address[address] = (event, int(row["value"], 16))

    joined_by_write_event = {
        int(write["event_sequence"]): (read, write) for read, write in joined_rows
    }
    for address in range(FETCH_PHYSICAL_FIRST, FETCH_PHYSICAL_LAST + 1):
        latest = latest_by_address.get(address)
        if latest is None:
            raise ValueError(f"source span lacks a pre-snapshot write at physical 0x{address:06x}")
        event, value = latest
        pair = joined_by_write_event.get(event)
        if pair is None:
            raise ValueError(f"latest pre-snapshot write at 0x{address:06x} is not source-validated")
        read, write = pair
        if int(write["event_sequence"]) != int(read["event_sequence"]) + 1:
            raise ValueError(f"pre-snapshot source read is not immediately followed by its RAM write at 0x{address:06x}")
        expected_offset = SOURCE_SPAN_FILE_OFFSET + address - FETCH_PHYSICAL_FIRST
        if (
            int(write["physical_address"], 16) != address
            or int(write["value"], 16) != value
            or read["track02_raw_file_offset"] != expected_offset
            or read["value"] != value
        ):
            raise ValueError(f"latest pre-snapshot write at 0x{address:06x} does not match the contiguous authentic source span")


def verify(
    follow_path: Path,
    write_path: Path,
    read_path: Path,
    track02_path: Path,
) -> dict[str, int]:
    paired_capture_paths(follow_path, write_path, read_path)
    require_shared_sequence_headers(write_path, read_path)
    follow_verifier.verify(follow_path)
    _, read_rows = cd_read_verifier.verify(read_path, track02_path)
    joined = cd_ram_join.verify(
        write_path,
        read_path,
        track02_path,
        FETCH_PHYSICAL_FIRST,
        FETCH_PHYSICAL_LAST,
        FETCH_MPR2,
    )

    follow_lines = follow_path.read_text(encoding="ascii").splitlines()
    fetch_rows: dict[int, list[dict[str, str]]] = {pc: [] for pc in SNAPSHOT_SITES}
    for number, line in enumerate(follow_lines[1:], 2):
        if line.startswith("target_follow "):
            _, row = follow_verifier.fields(line, number)
            for pc in SNAPSHOT_SITES:
                if row.get("logical_pc") == f"{pc:04x}":
                    fetch_rows[pc].append(row)
    if any(not rows for rows in fetch_rows.values()):
        missing = ", ".join(f"${pc:04x}" for pc, rows in fetch_rows.items() if not rows)
        raise ValueError(f"bootstrap-follow trace lacks required instruction-start snapshots: {missing}")

    write_lines = write_path.read_text(encoding="ascii").splitlines()
    if not write_lines or write_lines[0] != cd_ram_join.WRITE_HEADER_SHARED_SEQUENCE:
        raise ValueError("unexpected CD-RAM target-write trace header")
    if any(line.startswith("cd_ram_target_write_truncated ") for line in write_lines[1:]):
        raise ValueError("CD-RAM target-write trace is truncated")
    target_writes = [
        cd_ram_join.parse_fields(line, "cd_ram_target_write", number)
        for number, line in enumerate(write_lines[1:], 2)
    ]

    shared_event_groups = [
        [row["event_sequence"] for row in read_rows],
        [row["event_sequence"] for row in target_writes],
        [int(row["event_sequence"]) for row in follow_rows(follow_lines)],
    ]
    all_events = [int(event) for group in shared_event_groups for event in group]
    if len(all_events) != len(set(all_events)):
        raise ValueError("shared event sequence collides across paired trace sidecars")

    first_snapshot_by_pc = {
        pc: min(rows, key=lambda row: int(row["event_sequence"]))
        for pc, rows in fetch_rows.items()
    }
    first_stage_snapshot_event = int(first_snapshot_by_pc[0x4002]["event_sequence"])
    verify_source_span_before_fetch(target_writes, joined, first_stage_snapshot_event)
    source_joined_writes = [write for _, write in joined]
    result: dict[str, int] = {}
    for pc, physical in SNAPSHOT_SITES.items():
        result.update(
            verify_fetch_binding(
                first_snapshot_by_pc[pc], target_writes, source_joined_writes, pc, physical
            )
        )
    return result


def follow_rows(lines: list[str]) -> list[dict[str, str]]:
    result = []
    for number, line in enumerate(lines[1:], 2):
        if line.startswith(("bootstrap_follow ", "target_follow ")):
            _, row = follow_verifier.fields(line, number)
            result.append(row)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bootstrap_follow_trace", type=Path)
    parser.add_argument("cd_ram_target_write_trace", type=Path)
    parser.add_argument("cd_data_port_read_trace", type=Path)
    parser.add_argument("track02", type=Path)
    args = parser.parse_args()
    try:
        result = verify(
            args.bootstrap_follow_trace,
            args.cd_ram_target_write_trace,
            args.cd_data_port_read_trace,
            args.track02,
        )
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print("PASS: `$4002` and `$4009` instruction-start snapshots match their latest pre-snapshot writes")
    print("all 11 bytes at physical `$100002-$10000c` join to a contiguous authentic JP Track 02 span")
    print(
        "snapshot_events="
        + ",".join(str(result[f"fetch_{pc:04x}_event"]) for pc in SNAPSHOT_SITES)
        + "; $4002_latest_write_events="
        + ",".join(str(result[f"fetch_4002_byte_{index}_write_event"]) for index in range(4))
        + "; $4009_latest_write_events="
        + ",".join(str(result[f"fetch_4009_byte_{index}_write_event"]) for index in range(4))
    )
    print("scope=observed instruction-start snapshots in bounded sidecars; no gameplay handoff or exclusive physical-memory provenance")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
