#!/usr/bin/env python3
"""Verify the authentic JP first-stage CD bootstrap and sampled handoff."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

from verify_theron_jp_cd_data_port_trace import verify as verify_data_port_trace
from verify_theron_jp_syscard_buffer_write_trace import (
    ROW_PATTERN as WRITE_ROW_PATTERN,
    verify as verify_buffer_writes,
)


CONSUMER_HEADER = "source=mednafen-pce-fast-main-ram-consumer-read"
EXPECTED_SIGNATURE = (
    b"PC Engine CD-ROM SYSTEM\x00"
    b"Copyright HUDSON SOFT / NEC Home Electronics,Ltd.\x00"
)
BUFFER_BASE = 0x2800
BUFFER_PHYSICAL_BASE = 0x1F0800
BOOT_SIGNATURE_PC = 0x2B39
BOOT_ENTRY_PC = 0x2B44
STAGE_ENTRY_PC = 0x4002
CONSUMER_ROW = re.compile(
    r"main_ram_consumer_read sequence=(\d+) logical_address=([0-9a-f]{4}) "
    r"physical_address=([0-9a-f]{6}) value=([0-9a-f]{2}) "
    r"reader_pc=([0-9a-f]{4}) reader_physical_pc=([0-9a-f]{6}) "
    r"reader_code_bytes=([0-9a-f]{16}) a=([0-9a-f]{2}) "
    r"x=([0-9a-f]{2}) y=([0-9a-f]{2}) sp=([0-9a-f]{2}) p=([0-9a-f]{2})"
)


def read_consumer_rows(path: Path) -> list[dict[str, int | bytes]]:
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != CONSUMER_HEADER:
        raise ValueError("unexpected main-RAM consumer trace header")
    rows = []
    for line_number, line in enumerate(lines[1:], start=2):
        match = CONSUMER_ROW.fullmatch(line)
        if not match:
            raise ValueError(f"malformed consumer row at line {line_number}")
        values = match.groups()
        rows.append(
            {
                "sequence": int(values[0]),
                "logical_address": int(values[1], 16),
                "physical_address": int(values[2], 16),
                "value": int(values[3], 16),
                "reader_pc": int(values[4], 16),
                "reader_physical_pc": int(values[5], 16),
                "reader_code_bytes": bytes.fromhex(values[6]),
            }
        )
    return rows


def verify_bootstrap(
    write_trace: Path,
    read_trace: Path,
    consumer_trace: Path,
    track02: Path,
) -> None:
    verify_data_port_trace(read_trace, track02)
    verify_buffer_writes(write_trace, read_trace, track02)
    write_lines = write_trace.read_text(encoding="ascii").splitlines()[1:]
    writes = [WRITE_ROW_PATTERN.fullmatch(line) for line in write_lines]
    if any(row is None for row in writes):
        raise ValueError("malformed System Card buffer-write row")
    buffer = bytes(int(row.group(4), 16) for row in writes if row is not None)
    if len(buffer) != 4_096:
        raise ValueError(f"expected a 4096-byte first-stage buffer, found {len(buffer)}")

    signature_offset = 0x3020 - BUFFER_BASE
    signature = buffer[signature_offset : signature_offset + len(EXPECTED_SIGNATURE)]
    if signature != EXPECTED_SIGNATURE:
        raise ValueError("copied boot signature differs from authentic JP Track 02")

    rows = read_consumer_rows(consumer_trace)
    signature_reads = [row for row in rows if row["reader_pc"] == BOOT_SIGNATURE_PC]
    if len(signature_reads) != len(EXPECTED_SIGNATURE):
        raise ValueError(
            f"expected {len(EXPECTED_SIGNATURE)} signature reads, found {len(signature_reads)}"
        )
    signature_pc_offset = BOOT_SIGNATURE_PC - BUFFER_BASE
    expected_code = buffer[signature_pc_offset : signature_pc_offset + 8]
    for index, row in enumerate(signature_reads):
        logical = 0x3020 + index
        physical = 0x1F1020 + index
        if (
            row["logical_address"] != logical
            or row["physical_address"] != physical
            or row["value"] != EXPECTED_SIGNATURE[index]
            or row["reader_physical_pc"]
            != BUFFER_PHYSICAL_BASE + signature_pc_offset
            or row["reader_code_bytes"] != expected_code
        ):
            raise ValueError(f"signature consumer differs from source-bound bytes at index {index}")
        if index and row["sequence"] != signature_reads[index - 1]["sequence"] + 1:
            raise ValueError("signature reads are not a contiguous runtime sequence")

    by_pc_and_address = {
        (row["reader_pc"], row["logical_address"]): row for row in rows
    }
    continuation = by_pc_and_address.get((BOOT_ENTRY_PC, 0x300D))
    if continuation is None or continuation["value"] != 0x40:
        raise ValueError("the signature routine's $2B44 continuation was not observed")

    metadata = [0x00, 0x01, 0x02, 0x03, 0x04]
    metadata_pcs = [0x2C54, 0x2C5D, 0x2C66, 0x2C6F, 0x2C78]
    for index, (pc, value) in enumerate(zip(metadata_pcs, metadata)):
        row = by_pc_and_address.get((pc, 0x3008 + index))
        if row is None or row["value"] != value:
            raise ValueError(f"unexpected first-stage MPR metadata at index {index}")

    header = [0x00, 0x03, 0xA3, 0x03, 0x00, 0x40, 0x00, 0x40]
    for index, value in enumerate(header):
        row = by_pc_and_address.get((0x2C81, 0x3000 + index))
        if row is None or row["value"] != value:
            raise ValueError(f"unexpected first-stage loader header byte {index}")

    indirect_entry = [
        by_pc_and_address.get((0x2CD1, 0x2007)),
        by_pc_and_address.get((0x2CD1, 0x2008)),
    ]
    if any(row is None for row in indirect_entry) or [row["value"] for row in indirect_entry] != [0, 0x40]:
        raise ValueError("the sampled indirect handoff pointer is not $4000")
    stage_entry = next((row for row in rows if row["reader_pc"] == STAGE_ENTRY_PC), None)
    if (
        stage_entry is None
        or stage_entry["reader_physical_pc"] != 0x100002
        or stage_entry["reader_code_bytes"] != bytes.fromhex("73002001200f0073")
    ):
        raise ValueError("the sampled execution at the $4000 second-stage entry is missing")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("write_trace", type=Path)
    parser.add_argument("read_trace", type=Path)
    parser.add_argument("consumer_trace", type=Path)
    parser.add_argument("track02", type=Path)
    args = parser.parse_args()
    try:
        verify_bootstrap(args.write_trace, args.read_trace, args.consumer_trace, args.track02)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    print("PASS: authentic Track 02 signature accepted by executed first-stage code")
    print("handoff=$4000 physical_pc=0x100002; second-stage source binding remains open")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
