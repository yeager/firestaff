#!/usr/bin/env python3
"""Verify authentic-media CD reads and matching bank-$80 writes in one capture.

This verifies same-capture byte correlation only. The separate traces do not
prove that a particular CD-port read caused a particular bank-$80 write.
"""

from __future__ import annotations

import argparse
import hashlib
import sys
from pathlib import Path


EXPECTED_TRACK02_MD5 = "b7afb338ad31be1025b53f9aff12d73a"
EXPECTED_TRACK02_BYTES = 8_102_640
TRACK02_START_LBA = 3_590
TRACK02_INDEX1_FILE_OFFSET = 526_848
SECTOR_BYTES = 2_352
USER_DATA_OFFSET = 16
CANDIDATE_LBAS = (4_521, 4_589)
WINDOW_BYTES = 15


def fields(line: str, prefix: str, line_number: int) -> dict[str, str]:
    if not line.startswith(prefix + " "):
        raise ValueError(f"malformed {prefix} row at line {line_number}")
    result: dict[str, str] = {}
    for item in line[len(prefix) + 1 :].split():
        if "=" not in item:
            raise ValueError(f"malformed field at line {line_number}")
        key, value = item.split("=", 1)
        if key in result and result[key] != value:
            raise ValueError(f"conflicting duplicate field at line {line_number}")
        result[key] = value
    return result


def read_trace(path: Path) -> dict[int, list[dict[str, str]]]:
    lines = path.read_text(encoding="ascii").splitlines()
    mpr_context = "source=mednafen-pce-fast-cd-data-port-read-with-reader-mpr-context"
    if not lines or lines[0] not in (
        "source=mednafen-pce-fast-cd-data-port-read",
        "source=mednafen-pce-fast-cd-data-port-read-with-reader-pc",
        mpr_context,
    ):
        raise ValueError("unexpected CD data-port trace header")
    rows: dict[int, list[dict[str, str]]] = {lba: [] for lba in CANDIDATE_LBAS}
    expected_sequence = 0
    for line_number, line in enumerate(lines[1:], 2):
        row = fields(line, "cd_data_port_read", line_number)
        sequence = int(row["sequence"])
        if sequence != expected_sequence:
            raise ValueError(f"non-contiguous CD read sequence at line {line_number}")
        expected_sequence += 1
        if lines[0] == mpr_context:
            try:
                reader_pc = int(row["reader_pc"], 16)
                cpu_pc = int(row["cpu_pc"], 16)
                reader_physical_pc = int(row["reader_physical_pc"], 16)
                reader_mpr_slot = int(row["reader_mpr_slot"])
                expected_slot = (reader_pc >> 13) & 7
                reader_mpr = int(row[f"reader_mpr{expected_slot}"], 16)
            except (KeyError, ValueError) as error:
                raise ValueError(f"missing reader MPR context at line {line_number}") from error
            if (
                cpu_pc != reader_pc
                or reader_mpr_slot != expected_slot
                or reader_physical_pc != ((reader_mpr << 13) | (reader_pc & 0x1fff))
            ):
                raise ValueError(f"invalid reader PC to physical PC mapping at line {line_number}")
        elif "reader_mpr_slot" in row:
            raise ValueError(f"reader MPR fields require the MPR-context trace header at line {line_number}")
        if row.get("address") != "1808" or row.get("source_valid") != "1":
            continue
        lba = int(row["source_lba"])
        if lba in rows:
            rows[lba].append(row)
    return rows


def verify(write_path: Path, read_path: Path, track02_path: Path) -> dict[int, bytes]:
    if write_path.parent.resolve() != read_path.parent.resolve() or \
       write_path.name.removesuffix(".cd-ram-target-write") != \
       read_path.name.removesuffix(".cd-data-port-read"):
        raise ValueError("CD read and bank-$80 write traces are not paired from one capture")

    raw = track02_path.read_bytes()
    if len(raw) != EXPECTED_TRACK02_BYTES or hashlib.md5(raw).hexdigest() != EXPECTED_TRACK02_MD5:
        raise ValueError("Track 02 is not the authenticated JP Rev. 1 BIN")

    reads = read_trace(read_path)
    candidate_bytes: dict[int, bytes] = {}
    for lba in CANDIDATE_LBAS:
        rows = [row for row in reads[lba] if int(row["source_user_offset"]) < WINDOW_BYTES]
        if len(rows) != WINDOW_BYTES:
            raise ValueError(f"expected {WINDOW_BYTES} CD reads at LBA {lba}, found {len(rows)}")
        values = bytearray()
        for offset, row in enumerate(rows):
            raw_offset = TRACK02_INDEX1_FILE_OFFSET + (lba - TRACK02_START_LBA) * SECTOR_BYTES + USER_DATA_OFFSET + offset
            if int(row["source_user_offset"]) != offset or int(row["source_raw_offset"]) != USER_DATA_OFFSET + offset:
                raise ValueError(f"unexpected source offset at LBA {lba}, user offset {offset}")
            if (int(row["track02_start_lba"]) != TRACK02_START_LBA or
                int(row["track02_end_lba"]) != 6_961 or
                int(row["track02_index1_file_offset"]) != TRACK02_INDEX1_FILE_OFFSET or
                int(row["track02_sector_bytes"]) != SECTOR_BYTES or
                int(row["source_lba"]) != lba):
                raise ValueError(f"unexpected source metadata at LBA {lba}, user offset {offset}")
            if int(row["track02_raw_file_offset"]) != raw_offset or raw[raw_offset] != int(row["value"], 16):
                raise ValueError(f"CD read differs from authentic media at LBA {lba}, user offset {offset}")
            values.append(int(row["value"], 16))
        candidate_bytes[lba] = bytes(values)

    lines = write_path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != "source=mednafen-pce-fast-cd-ram-target-write":
        raise ValueError("unexpected bank-$80 target-write trace header")
    groups: list[list[dict[str, str]]] = []
    current: list[dict[str, str]] = []
    expected_sequence = 0
    for line_number, line in enumerate(lines[1:], 2):
        row = fields(line, "cd_ram_target_write", line_number)
        sequence = int(row["sequence"])
        if sequence != expected_sequence:
            raise ValueError(f"non-contiguous target-write sequence at line {line_number}")
        expected_sequence += 1
        if row.get("writer_pc") == "ea9c" and row.get("mpr2") == "80" and row.get("block_move") == "0":
            if int(row["physical_address"], 16) == 0x100000 + len(current):
                current.append(row)
                if len(current) == WINDOW_BYTES:
                    groups.append(current)
                    current = []
                continue
        current = []

    observed_writes: list[bytes] = []
    for group in groups:
        data = bytes(int(row["value"], 16) for row in group)
        if all(int(row["physical_address"], 16) == 0x100000 + offset for offset, row in enumerate(group)):
            observed_writes.append(data)
    for lba, source in candidate_bytes.items():
        if source not in observed_writes:
            raise ValueError(f"authentic CD-read bytes at LBA {lba} have no matching direct bank-$80 write window")
    return candidate_bytes


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("write_trace", type=Path)
    parser.add_argument("read_trace", type=Path)
    parser.add_argument("track02", type=Path)
    args = parser.parse_args()
    try:
        candidates = verify(args.write_trace, args.read_trace, args.track02)
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print("PASS: authentic Track 02 bytes read at candidate LBAs match direct bank-$80 writes in the same capture")
    for lba, data in candidates.items():
        print(f"candidate_lba={lba} bytes={len(data)} sha256={hashlib.sha256(data).hexdigest()}")
    print("LIMIT: same-capture correlation only; no source-to-destination causal lineage is established")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
