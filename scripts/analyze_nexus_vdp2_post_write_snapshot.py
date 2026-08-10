#!/usr/bin/env python3
"""Verify a VDP2 post-write snapshot sequence against a retail Nexus asset."""

from __future__ import annotations

import argparse
import re
import struct
from pathlib import Path


MAGIC = b"FIRESTAFF_NEXUS_VDP2_POST_SNAPSHOT_V1\n"
HEADER = re.compile(r"^frame=(\d+) area=(\w+) addr=0x([0-9a-f]+)$")
PAYLOAD_SIZE = 0x200 + 0x80000 + 0x1000


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("snapshot", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--asset", default="TM.BIN")
    parser.add_argument("--source-file-offset", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--destination-start", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--minimum-writes", type=int, default=8)
    return parser.parse_args()


def cram_byte_offset(address: int) -> int:
    cri = (address & 0xFFF) >> 1
    return (((cri >> 1) & 0x3FF) | ((cri & 1) << 10)) * 2


def main() -> int:
    args = parse_args()
    raw = args.snapshot.read_bytes()
    if not raw.startswith(MAGIC):
        raise SystemExit("snapshot_magic=invalid")
    pos = len(MAGIC)
    rows: list[tuple[int, int, int]] = []
    while pos < len(raw):
        end = raw.find(b"\n", pos)
        if end < 0:
            raise SystemExit("snapshot_header=truncated")
        match = HEADER.fullmatch(raw[pos:end].decode("ascii"))
        if not match:
            raise SystemExit("snapshot_header=invalid")
        frame, area, address = int(match.group(1)), match.group(2), int(match.group(3), 16)
        pos = end + 1
        payload = raw[pos : pos + PAYLOAD_SIZE]
        if len(payload) != PAYLOAD_SIZE:
            raise SystemExit("snapshot_payload=truncated")
        pos += PAYLOAD_SIZE
        if area == "cram":
            cram = payload[0x200 + 0x80000 :]
            value = struct.unpack_from("<H", cram, cram_byte_offset(address))[0]
            rows.append((frame, address, value))

    asset_path = args.data_dir / args.asset
    asset = asset_path.read_bytes()
    selected = [
        row for row in rows
        if args.destination_start <= row[1] < args.destination_start + (len(asset) - args.source_file_offset)
    ]
    selected.sort(key=lambda row: row[1])
    verified = 0
    for index, (_, address, value) in enumerate(selected):
        expected_address = args.destination_start + index * 2
        if address != expected_address:
            break
        source_offset = args.source_file_offset + index * 2
        if source_offset + 2 > len(asset):
            break
        expected = int.from_bytes(asset[source_offset : source_offset + 2], "big")
        if value != expected:
            break
        verified += 1

    print(f"snapshot_records={len(rows)}")
    print(f"destination_start=0x{args.destination_start:06x}")
    print(f"source_file={asset_path}")
    print(f"source_file_offset_start=0x{args.source_file_offset:x}")
    print(f"source_value_join=verified" if verified >= args.minimum_writes else "source_value_join=blocked")
    print(f"verified_post_write_writes={verified}")
    print(f"minimum_writes={args.minimum_writes}")
    print("asset_identity=verified")
    return 0 if verified >= args.minimum_writes else 1


if __name__ == "__main__":
    raise SystemExit(main())
