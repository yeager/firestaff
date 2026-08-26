#!/usr/bin/env python3
"""Join an authentic Saturn CDB LBA trace to ISO9660 retail members.

This is provenance evidence only. It does not decode a member, infer its
runtime consumer, or promote any host playback/rendering route.
"""

from __future__ import annotations

import argparse
import collections
from pathlib import Path


HEADER = "FIRESTAFF_NEXUS_CD_READ_TRACE_V1"
SECTOR_SIZE = 2048


def read_iso_files(iso: Path) -> list[tuple[int, int, str]]:
    """Read ISO9660 directory records from cooked or raw Track 1 in memory."""
    image = iso.read_bytes()
    sector_bytes = 2352 if len(image) % 2352 == 0 else SECTOR_SIZE
    user_offset = 16 if sector_bytes == 2352 else 0

    def sector(lba: int) -> bytes:
        start = lba * sector_bytes + user_offset
        end = start + SECTOR_SIZE
        if start < 0 or end > len(image):
            raise ValueError("ISO sector lies outside Track 1")
        return image[start:end]

    def extent(lba: int, size: int) -> bytes:
        return b"".join(sector(lba + index)
                        for index in range((size + SECTOR_SIZE - 1) // SECTOR_SIZE))[:size]

    pvd = sector(16)
    if pvd[1:6] != b"CD001":
        raise ValueError("Track 1 has no ISO9660 primary volume descriptor")
    root = pvd[156:]
    if len(root) < 34:
        raise ValueError("ISO9660 root record is truncated")
    files: list[tuple[int, int, str]] = []

    def walk(lba: int, size: int, prefix: str) -> None:
        data = extent(lba, size)
        offset = 0
        while offset < len(data):
            record_length = data[offset]
            if record_length == 0:
                offset = ((offset // SECTOR_SIZE) + 1) * SECTOR_SIZE
                continue
            record = data[offset : offset + record_length]
            if len(record) < 34:
                raise ValueError("ISO9660 directory record is truncated")
            child_lba = int.from_bytes(record[2:6], "little")
            child_size = int.from_bytes(record[10:14], "little")
            flags = record[25]
            name_length = record[32]
            name = record[33 : 33 + name_length].decode("ascii", "replace")
            if name not in ("\x00", "\x01"):
                name = name.split(";", 1)[0].upper()
                child_path = f"{prefix}/{name}".strip("/")
                if flags & 2:
                    walk(child_lba, child_size, child_path)
                else:
                    files.append((child_lba, child_size, child_path))
            offset += record_length

    walk(int.from_bytes(root[2:6], "little"), int.from_bytes(root[10:14], "little"), "")
    return files


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("iso", type=Path)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--require-member", action="append", default=[])
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
        if not lines or lines[0] != HEADER:
            raise ValueError("bad CDB trace header")
        lbas = []
        for number, line in enumerate(lines[1:], 2):
            key, separator, value = line.partition("=")
            if key != "lba" or not separator:
                raise ValueError(f"malformed trace line {number}")
            lbas.append(int(value, 10))
        files = read_iso_files(args.iso)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_CDB_TRACE_INVALID: {error}")
        return 1

    counts: collections.Counter[str] = collections.Counter()
    for lba in lbas:
        owner = "UNMAPPED"
        for start, size, name in files:
            end = start + (size + SECTOR_SIZE - 1) // SECTOR_SIZE
            if start <= lba < end:
                owner = name
                break
        counts[owner] += 1

    print(f"iso_files={len(files)}")
    print(f"trace_reads={len(lbas)}")
    if lbas:
        print(f"lba_range={min(lbas)}-{max(lbas)}")
    for name, count in counts.most_common():
        print(f"member={name} reads={count}")

    required = {name.upper() for name in args.require_member}
    present = {name.rsplit("/", 1)[-1].upper() for name in counts}
    missing = sorted(required - present)
    if missing:
        print("required_members_missing=" + ",".join(missing))
        return 1
    print("retail_lba_join=verified")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
