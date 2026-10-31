#!/usr/bin/env python3
"""Bind the captured JP indexed jump and destination fetch to authentic Track 02 bytes."""

from __future__ import annotations

import argparse
import hashlib
import sys
from collections import defaultdict
from pathlib import Path

from theron_python_compat import zip_strict
from verify_theron_indirect_jump_trace import (
    FETCH,
    parse_fetch_rows,
    verify as verify_indirect_jump,
)
from verify_theron_jp_cd_ram_writer_pc_trace import (
    TRACK02_MD5,
    TRACK02_SIZE,
    parse_fields as parse_trace_fields,
    verify_read_rows,
    verify_writer_rows,
)
from verify_theron_jp_code_follow_trace import verify as verify_code_follow


DISPATCH_SUFFIX = ".indirect-jump"
FOLLOW_SUFFIX = ".system-card-code-follow"
READ_SUFFIX = ".cd-data-port-read"
WRITE_SUFFIX = ".cd-ram-writer-pc"


def require_capture_pair(paths: list[Path]) -> None:
    if len({path.parent.resolve() for path in paths}) != 1:
        raise ValueError("trace files are not from one capture directory")
    suffixes = (DISPATCH_SUFFIX, FOLLOW_SUFFIX, READ_SUFFIX, WRITE_SUFFIX)
    bases = []
    for path, suffix in zip_strict(paths, suffixes):
        if not path.name.endswith(suffix):
            raise ValueError(f"unexpected trace filename suffix: {path.name}")
        bases.append(path.name[: -len(suffix)])
    if len(set(bases)) != 1:
        raise ValueError("trace files do not share one capture name")


def parse_dispatch_rows(path: Path) -> list[dict[str, str]]:
    lines = path.read_text(encoding="ascii").splitlines()
    rows = []
    for line_number, line in enumerate(lines[1:], 2):
        if not line.startswith("indirect_jump "):
            continue
        rows.append(parse_trace_fields(line, "indirect_jump", line_number))
    return rows


def parse_follow_rows(path: Path) -> tuple[list[dict[str, str]], int, int]:
    lines = path.read_text(encoding="ascii").splitlines()
    raw_rows, first_sequence, completion_sequence = parse_fetch_rows(lines)
    keys = (
        "event_sequence", "index", "logical_pc", "physical_pc", "bytes",
        "physical_bytes_0", "physical_bytes_1", "physical_bytes_2", "valid_mask",
        *(f"mpr{i}" for i in range(8)),
    )
    result = []
    for fields in raw_rows:
        result.append(dict(zip_strict(keys, fields)))
    return result, first_sequence, completion_sequence


def verify_source_chain(
    dispatch_path: Path,
    follow_path: Path,
    read_path: Path,
    write_path: Path,
    track02_path: Path,
) -> tuple[list[tuple[int, int, int, int, int]], int]:
    paths = [dispatch_path, follow_path, read_path, write_path]
    require_capture_pair(paths)
    track02 = track02_path.read_bytes()
    if len(track02) != TRACK02_SIZE or hashlib.md5(track02).hexdigest() != TRACK02_MD5:
        raise ValueError("Track 02 is not authenticated Theron's Quest JP Rev. 1 media")

    if verify_code_follow(follow_path, require_complete=True) != ("complete", 256):
        raise ValueError("code-follow verifier did not confirm the complete window")
    _, matched_rows, out_of_window_rows = verify_indirect_jump(dispatch_path, follow_path)
    if matched_rows != 1:
        raise ValueError("expected exactly one in-window indirect dispatch")

    reads, unbound_reads = verify_read_rows(read_path, track02)
    if unbound_reads:
        raise ValueError(f"CD trace contains {unbound_reads} source-unbound rows")
    writes = verify_writer_rows(write_path)
    reads_by_event = {int(row["event_sequence"]): row for row in reads}
    writes_by_address: dict[int, list[dict[str, str]]] = defaultdict(list)
    for row in writes:
        writes_by_address[int(row["physical_address"], 16)].append(row)

    dispatch_rows = parse_dispatch_rows(dispatch_path)
    follow_rows, follow_start, follow_complete = parse_follow_rows(follow_path)
    dispatched = [
        row for row in dispatch_rows
        if follow_start < int(row["event_sequence"]) < follow_complete
    ]
    if len(dispatched) != 1:
        raise ValueError("expected exactly one dispatch inside the code-follow window")
    dispatch = dispatched[0]
    dispatch_event = int(dispatch["event_sequence"])
    target = int(dispatch["target"], 16)
    follow = next(
        (row for row in follow_rows if int(row["event_sequence"]) > dispatch_event),
        None,
    )
    if follow is None or int(follow["logical_pc"], 16) != target:
        raise ValueError("next code-follow row does not execute the resolved target")
    fetch_event = int(follow["event_sequence"])

    def source_backed_write(address: int, value: int, consumer_event: int) -> tuple[int, int, int, int, int]:
        prior = [row for row in writes_by_address.get(address, [])
                 if int(row["event_sequence"]) < consumer_event]
        if not prior:
            raise ValueError(f"no prior System Card writer-PC store to physical ${address:06X}")
        latest = max(prior, key=lambda row: int(row["event_sequence"]))
        write_event = int(latest["event_sequence"])
        if int(latest["value"], 16) != value:
            raise ValueError(f"latest logged store to physical ${address:06X} differs from observed byte")
        read = reads_by_event.get(write_event - 1)
        if read is None:
            raise ValueError(f"store at event {write_event} lacks an immediately preceding CD read")
        if (
            read.get("source_valid") != "1"
            or read.get("reader_pc") != "ea99"
            or int(read["value"], 16) != value
        ):
            raise ValueError(
                f"CD read at event {write_event - 1} is not a source-valid $EA99 read of the store value"
            )
        return (
            write_event - 1,
            write_event,
            int(read["track02_raw_file_offset"]),
            address,
            value,
        )

    joins: list[tuple[int, int, int, int, int]] = []
    table_address = int(dispatch["table_address"], 16)
    for offset, value, physical in (
        (0, int(dispatch["low"], 16), int(dispatch["low_physical"], 16)),
        (1, int(dispatch["high"], 16), int(dispatch["high_physical"], 16)),
    ):
        if physical != int(dispatch["low_physical" if offset == 0 else "high_physical"], 16):
            raise ValueError("dispatch table-byte physical mapping changed")
        joins.append(source_backed_write(physical, value, dispatch_event))
        if table_address + offset > 0xFFFF:
            raise ValueError("indirect table crosses the logical address-space boundary")

    code_values = bytes.fromhex(follow["bytes"])
    code_addresses = [int(follow[f"physical_bytes_{index}"], 16) for index in range(3)]
    if len(code_values) != len(code_addresses) or len(code_values) != 3:
        raise ValueError("unexpected next-fetch byte count")
    for value, address in zip_strict(code_values, code_addresses):
        joins.append(source_backed_write(address, value, fetch_event))

    return joins, out_of_window_rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dispatch_trace", type=Path)
    parser.add_argument("code_follow_trace", type=Path)
    parser.add_argument("cd_data_port_read_trace", type=Path)
    parser.add_argument("cd_ram_writer_pc_trace", type=Path)
    parser.add_argument("track02", type=Path)
    args = parser.parse_args()
    try:
        joins, out_of_window = verify_source_chain(
            args.dispatch_trace,
            args.code_follow_trace,
            args.cd_data_port_read_trace,
            args.cd_ram_writer_pc_trace,
            args.track02,
        )
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(f"PASS: {len(joins)} observed bytes match authentic Track 02 reads and latest traced $EA9C writes")
    print(f"out_of_window_dispatch_rows={out_of_window}")
    for read_event, write_event, file_offset, address, value in joins:
        print(
            f"source_read_event={read_event} ram_write_event={write_event} "
            f"track02_raw_file_offset={file_offset} physical_address={address:06x} value={value:02x}"
        )
    print("LIMIT: other guest writers are not covered by the $EA9C trace; exclusive provenance, loader completion, and gameplay remain unproven")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
