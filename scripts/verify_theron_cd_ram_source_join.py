#!/usr/bin/env python3
"""Join authenticated Track 02 CD reads to immediate PCE Fast RAM writes."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from verify_theron_jp_cd_data_port_trace import (
    TRACK02_END_LBA,
    TRACK02_INDEX1_FILE_OFFSET,
    TRACK02_START_LBA,
    SECTOR_BYTES,
    USER_DATA_OFFSET,
    verify as verify_data_port_trace,
)


READ_SUFFIX = ".cd-data-port-read"
WRITE_SUFFIX = ".cd-ram-target-write"
READ_HEADER = "source=mednafen-pce-fast-cd-data-port-read-with-reader-mpr-context"
WRITE_HEADER = "source=mednafen-pce-fast-cd-ram-target-write"


def parse_fields(line: str, prefix: str, line_number: int) -> dict[str, str]:
    if not line.startswith(prefix + " "):
        raise ValueError(f"malformed {prefix} row at line {line_number}")
    fields: dict[str, str] = {}
    for item in line[len(prefix) + 1 :].split():
        if "=" not in item:
            raise ValueError(f"malformed field at line {line_number}")
        key, value = item.split("=", 1)
        if key in fields and fields[key] != value:
            raise ValueError(f"conflicting duplicate field {key} at line {line_number}")
        fields[key] = value
    return fields


def join_rows(
    read_rows: list[dict[str, int]],
    write_rows: list[dict[str, str]],
    physical_first: int,
    physical_last: int,
    expected_mpr2: int | None,
    expected_writer_pc: int,
) -> list[tuple[dict[str, int], dict[str, str]]]:
    """Require each selected store to immediately follow an equal authentic read."""
    reads_by_event: dict[int, dict[str, int]] = {}
    for row in read_rows:
        if "event_sequence" not in row:
            raise ValueError("CD data-port rows lack shared event sequences")
        event = row["event_sequence"]
        if event in reads_by_event:
            raise ValueError("duplicate CD data-port event sequence")
        reads_by_event[event] = row

    joined: list[tuple[dict[str, int], dict[str, str]]] = []
    previous_event = -1
    for line_number, write in enumerate(write_rows, 2):
        try:
            event = int(write["event_sequence"])
            physical = int(write["physical_address"], 16)
        except (KeyError, ValueError) as error:
            raise ValueError(f"missing or malformed write coordinates at row {line_number - 1}") from error
        if event <= previous_event:
            raise ValueError(f"non-increasing write event sequence at row {line_number - 1}")
        previous_event = event
        if not physical_first <= physical <= physical_last:
            continue

        required = ("value", "writer_pc", "writer_physical_pc", "mpr2")
        if any(key not in write for key in required):
            raise ValueError(f"incomplete target-write context at row {line_number - 1}")
        if int(write["writer_pc"], 16) != expected_writer_pc:
            continue
        if (
            int(write["writer_physical_pc"], 16) != 0x0A9C
            or (expected_mpr2 is not None and int(write["mpr2"], 16) != expected_mpr2)
        ):
            raise ValueError(f"target write is not the expected System Card store at row {line_number - 1}")

        read = reads_by_event.get(event - 1)
        if read is None:
            raise ValueError(f"target write has no immediately preceding CD read at row {line_number - 1}")
        source_lba = read.get("source_lba", -1)
        source_offset = read.get("source_user_offset", -1)
        expected_raw_offset = (
            TRACK02_INDEX1_FILE_OFFSET
            + (source_lba - TRACK02_START_LBA) * SECTOR_BYTES
            + USER_DATA_OFFSET
            + source_offset
        )
        if (
            read.get("source_valid") != 1
            or not TRACK02_START_LBA <= source_lba < TRACK02_END_LBA
            or read.get("track02_start_lba") != TRACK02_START_LBA
            or read.get("track02_end_lba") != TRACK02_END_LBA
            or read.get("track02_index1_file_offset") != TRACK02_INDEX1_FILE_OFFSET
            or read.get("track02_sector_bytes") != SECTOR_BYTES
            or read.get("track02_raw_file_offset") != expected_raw_offset
            or read.get("address") != 0x1808
            or read.get("reader_pc") != 0xEA99
            or read.get("reader_physical_pc") != 0x0A99
        ):
            raise ValueError(f"preceding read lacks authenticated System Card source context at row {line_number - 1}")
        if read["value"] != int(write["value"], 16):
            raise ValueError(f"CD read and RAM write values differ at row {line_number - 1}")
        joined.append((read, write))

    if not joined:
        raise ValueError("no target writes matched the selected physical range")
    return joined


def verify(
    write_path: Path,
    read_path: Path,
    track02_path: Path,
    physical_first: int,
    physical_last: int,
    expected_mpr2: int | None = None,
    expected_writer_pc: int = 0xEA9C,
) -> list[tuple[dict[str, int], dict[str, str]]]:
    if physical_first < 0 or physical_last < physical_first:
        raise ValueError("invalid physical target range")
    if (
        write_path.parent.resolve() != read_path.parent.resolve()
        or not write_path.name.endswith(WRITE_SUFFIX)
        or write_path.name[: -len(WRITE_SUFFIX)]
        != read_path.name[: -len(READ_SUFFIX)]
        or not read_path.name.endswith(READ_SUFFIX)
    ):
        raise ValueError("CD read and RAM write traces are not paired from one capture")

    _, read_rows = verify_data_port_trace(
        read_path, track02_path, allow_other_sources=True
    )
    read_header = read_path.read_text(encoding="ascii").splitlines()[0]
    if read_header != READ_HEADER:
        raise ValueError("reader MPR context is required for a source-bound join")

    lines = write_path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != WRITE_HEADER:
        raise ValueError("unexpected CD RAM target-write trace header")
    write_rows = [
        parse_fields(line, "cd_ram_target_write", number)
        for number, line in enumerate(lines[1:], 2)
    ]
    for sequence, row in enumerate(write_rows):
        if row.get("sequence") != str(sequence):
            raise ValueError(f"non-contiguous RAM target-write sequence at row {sequence}")
    return join_rows(
        read_rows,
        write_rows,
        physical_first,
        physical_last,
        expected_mpr2,
        expected_writer_pc,
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("write_trace", type=Path)
    parser.add_argument("read_trace", type=Path)
    parser.add_argument("track02", type=Path)
    parser.add_argument("--physical-first", required=True, type=lambda value: int(value, 0))
    parser.add_argument("--physical-last", required=True, type=lambda value: int(value, 0))
    parser.add_argument("--mpr2", type=lambda value: int(value, 0))
    parser.add_argument("--writer-pc", type=lambda value: int(value, 0), default=0xEA9C)
    args = parser.parse_args()
    try:
        joined = verify(
            args.write_trace,
            args.read_trace,
            args.track02,
            args.physical_first,
            args.physical_last,
            args.mpr2,
            args.writer_pc,
        )
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    lbas = sorted({read["source_lba"] for read, _ in joined})
    offsets = sorted({int(write["physical_address"], 16) for _, write in joined})
    print(f"PASS: {len(joined)} immediate CD-read-to-RAM-write joins match authentic JP Track 02")
    print(f"source_lbas={lbas[0]}-{lbas[-1]} distinct_lbas={len(lbas)}")
    print(f"target_physical_addresses={len(offsets)} span=0x{offsets[0]:06x}-0x{offsets[-1]:06x}")
    print("reader_pc=0xea99 writer_pc=0xea9c; source byte values omitted")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
