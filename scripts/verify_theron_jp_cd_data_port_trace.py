#!/usr/bin/env python3
"""Verify a cold-start CD data-port trace against authentic JP Track 02."""

from __future__ import annotations

import argparse
import hashlib
import re
import sys
from pathlib import Path


EXPECTED_TRACK02_MD5 = "b7afb338ad31be1025b53f9aff12d73a"
EXPECTED_TRACK02_BYTES = 8_102_640
TRACK02_START_LBA = 3_590
TRACK02_END_LBA = 6_961
TRACK02_INDEX1_FILE_OFFSET = 526_848
SECTOR_BYTES = 2_352
USER_DATA_OFFSET = 16
USER_DATA_BYTES = 2_048
OBSERVED_BYTES = 4_096
TRACE_HEADER = "source=mednafen-pce-fast-cd-data-port-read"
TRACE_HEADER_WITH_READER_PC = "source=mednafen-pce-fast-cd-data-port-read-with-reader-pc"


def parse_row(line: str, line_number: int) -> dict[str, int]:
    match = re.fullmatch(
        r"cd_data_port_read sequence=(\d+) address=([0-9a-f]{4}) "
        r"value=([0-9a-f]{2}) "
        r"(?:reader_pc=([0-9a-f]{4}) reader_physical_pc=([0-9a-f]{6}) )?"
        r"source_valid=([01]) source_lba=(\d+) "
        r"source_user_offset=(\d+) source_raw_offset=(\d+) "
        r"track02_start_lba=(\d+) track02_end_lba=(\d+) "
        r"track02_index1_file_offset=(\d+) track02_sector_bytes=(\d+) "
        r"track02_raw_file_offset=(\d+)",
        line,
    )
    if not match:
        raise ValueError(f"malformed data-port row at line {line_number}")
    groups = match.groups()
    source_index = 5
    row = {
        "sequence": int(groups[0]),
        "address": int(groups[1], 16),
        "value": int(groups[2], 16),
        "source_valid": int(groups[source_index]),
        "source_lba": int(groups[source_index + 1]),
        "source_user_offset": int(groups[source_index + 2]),
        "source_raw_offset": int(groups[source_index + 3]),
        "track02_start_lba": int(groups[source_index + 4]),
        "track02_end_lba": int(groups[source_index + 5]),
        "track02_index1_file_offset": int(groups[source_index + 6]),
        "track02_sector_bytes": int(groups[source_index + 7]),
        "track02_raw_file_offset": int(groups[source_index + 8]),
    }
    if groups[3] is not None:
        row["reader_pc"] = int(groups[3], 16)
        row["reader_physical_pc"] = int(groups[4], 16)
    return row


def verify(trace_path: Path, track02_path: Path) -> tuple[bytes, list[dict[str, int]]]:
    raw = track02_path.read_bytes()
    if len(raw) != EXPECTED_TRACK02_BYTES:
        raise ValueError("Track 02 has the wrong authenticated JP Rev. 1 size")
    if hashlib.md5(raw).hexdigest() != EXPECTED_TRACK02_MD5:
        raise ValueError("Track 02 is not the authenticated JP Rev. 1 BIN")

    lines = trace_path.read_text(encoding="ascii").splitlines()
    reader_pc_trace = bool(lines and lines[0] == TRACE_HEADER_WITH_READER_PC)
    if not lines or lines[0] not in (TRACE_HEADER, TRACE_HEADER_WITH_READER_PC):
        raise ValueError("unexpected CD data-port trace header")
    rows = [parse_row(line, index + 2) for index, line in enumerate(lines[1:])]
    if len(rows) != OBSERVED_BYTES:
        raise ValueError(f"expected {OBSERVED_BYTES} data-port rows, found {len(rows)}")

    # Pass 1 checks the trace's ordered sequence and source metadata.
    for sequence, row in enumerate(rows):
        if row["sequence"] != sequence or row["address"] != 0x1808:
            raise ValueError(f"unexpected sequence or data-port address at row {sequence}")
        if row["source_valid"] != 1:
            raise ValueError(f"unbound source at row {sequence}")

    # The optional pass checks the traced CPU that performed each data-port read.
    if reader_pc_trace:
        for sequence, row in enumerate(rows):
            if row.get("reader_pc") != 0xEA99 or row.get("reader_physical_pc") != 0x0A99:
                raise ValueError(f"unexpected CD data-port reader PC at row {sequence}")
    else:
        for sequence, row in enumerate(rows):
            if "reader_pc" in row:
                raise ValueError(
                    f"reader-PC fields require the reader-PC trace header at row {sequence}"
                )

    # Pass 2 independently recomputes the two sector and raw-file offsets.
    for sequence, row in enumerate(rows):
        sector_index, user_offset = divmod(sequence, USER_DATA_BYTES)
        expected_offset = (
            TRACK02_INDEX1_FILE_OFFSET
            + sector_index * SECTOR_BYTES
            + USER_DATA_OFFSET
            + user_offset
        )
        if (
            row["track02_start_lba"] != TRACK02_START_LBA
            or row["track02_end_lba"] != TRACK02_END_LBA
            or row["track02_index1_file_offset"] != TRACK02_INDEX1_FILE_OFFSET
            or row["track02_sector_bytes"] != SECTOR_BYTES
            or row["source_lba"] != TRACK02_START_LBA + sector_index
            or row["source_user_offset"] != user_offset
            or row["source_raw_offset"] != USER_DATA_OFFSET + user_offset
            or row["track02_raw_file_offset"] != expected_offset
        ):
            raise ValueError(f"unexpected source span at row {sequence}")

    # Pass 3 compares every observed byte with the hash-authenticated raw BIN.
    observed = bytearray()
    for sequence, row in enumerate(rows):
        source_offset = row["track02_raw_file_offset"]
        if source_offset >= len(raw) or raw[source_offset] != row["value"]:
            raise ValueError(f"data-port byte differs from authentic media at row {sequence}")
        observed.append(row["value"])
    return bytes(observed), rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("track02", type=Path)
    args = parser.parse_args()
    try:
        observed, rows = verify(args.trace, args.track02)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    loop_count = 4 if "reader_pc" in rows[0] else 3
    print(f"PASS: {loop_count} verification loops; 4096 bytes match authentic JP Track 02")
    print(f"source_lbas={rows[0]['source_lba']}-{rows[-1]['source_lba']}")
    print(f"observed_sha256={hashlib.sha256(observed).hexdigest()}")
    if "reader_pc" in rows[0]:
        print(f"reader_pc=0x{rows[0]['reader_pc']:04x} physical_pc=0x{rows[0]['reader_physical_pc']:06x}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
