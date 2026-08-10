#!/usr/bin/env python3
"""Join SH-2 source-read/source-write chunks to authenticated Nexus ISO files.

The trace is a byte-provenance receipt for runtime loading.  It requires a
complete contiguous destination chunk to occur verbatim in the ISO; prefixes,
inferred offsets and runtime PC names are not admitted as file identity.  The
result still does not authorize a VDP1/VDP2 consumer or gameplay semantics.
"""

from __future__ import annotations

import argparse
import collections
import re
from pathlib import Path


HEADER = "FIRESTAFF_NEXUS_SH2_RAM_SOURCE_TRACE_V1"
LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) value=0x(?P<value>[0-9a-fA-F]+) "
    r"source=0x(?P<source>[0-9a-fA-F]+) source_value=0x(?P<source_value>[0-9a-fA-F]+) "
    r"pc0=0x(?P<pc0>[0-9a-fA-F]+) pc1=0x(?P<pc1>[0-9a-fA-F]+)$"
)
SECTOR_SIZE = 2048


def read_iso_files(iso: Path) -> list[tuple[int, int, str]]:
    with iso.open("rb") as stream:
        stream.seek(16 * SECTOR_SIZE)
        pvd = stream.read(SECTOR_SIZE)
        root = pvd[156:]
        if len(root) < 34:
            raise ValueError("ISO9660 root record is truncated")
        root_lba = int.from_bytes(root[2:6], "little")
        root_size = int.from_bytes(root[10:14], "little")
        files: list[tuple[int, int, str]] = []

        def walk(lba: int, size: int, prefix: str) -> None:
            stream.seek(lba * SECTOR_SIZE)
            data = stream.read(size)
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
                        files.append((child_lba * SECTOR_SIZE, child_size, child_path))
                offset += record_length

        walk(root_lba, root_size, "")
        return files


def read_rows(trace: Path) -> list[tuple[int, int, int, int, int, int]]:
    lines = trace.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != HEADER:
        raise ValueError("bad source-trace header")
    rows = []
    for line_number, line in enumerate(lines[1:], 2):
        match = LINE.fullmatch(line)
        if not match:
            raise ValueError(f"malformed source-trace line {line_number}")
        rows.append(tuple(int(match[name], 16) for name in
                          ("addr", "value", "source", "source_value", "pc0", "pc1")))
    return rows


def chunks(rows: list[tuple[int, int, int, int, int, int]]) -> list[list[tuple[int, int, int, int, int, int]]]:
    result: list[list[tuple[int, int, int, int, int, int]]] = []
    current: list[tuple[int, int, int, int, int, int]] = []
    previous_end: int | None = None
    for row in rows:
        if previous_end is None or row[0] == previous_end:
            current.append(row)
        else:
            result.append(current)
            current = [row]
        previous_end = row[0] + 4
    if current:
        result.append(current)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("iso", type=Path)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--require-member", action="append", default=[])
    parser.add_argument(
        "--require-destination-range",
        type=lambda value: tuple(int(part, 0) for part in value.split(":", 1)),
        help="require one exact source chunk covering START:END (exclusive)",
    )
    parser.add_argument(
        "--require-pc",
        type=lambda value: int(value, 0),
        help="require the source writer PC on the destination-range chunk",
    )
    args = parser.parse_args()
    try:
        rows = read_rows(args.trace)
        files = read_iso_files(args.iso)
        iso_bytes = args.iso.read_bytes()
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_SH2_SOURCE_TRACE_INVALID: {error}")
        return 1

    matched: collections.Counter[str] = collections.Counter()
    exact_matches = 0
    required_chunk_verified = args.require_destination_range is None
    equal_values = sum(row[1] == row[3] for row in rows)
    for index, chunk in enumerate(chunks(rows)):
        blob = b"".join(row[1].to_bytes(4, "big") for row in chunk)
        # Zero-filled RAM and disc padding are not a source-owner witness.
        # They can produce arbitrarily long byte matches at the end of an ISO
        # image, so reject them before searching the retail image.
        if len(blob) < 32 or not any(blob):
            continue
        offset = iso_bytes.find(blob)
        if offset < 0:
            continue
        owner = "UNMAPPED"
        relative = 0
        for start, size, name in files:
            if start <= offset < start + size:
                owner = name
                relative = offset - start
                break
        # System-area/padding matches are not file identity. Keep the trace
        # observation, but do not count an unmapped offset as provenance.
        if owner == "UNMAPPED":
            continue
        if args.require_destination_range is not None:
            required_start, required_end = args.require_destination_range
            chunk_start = chunk[0][0]
            chunk_end = chunk[-1][0] + 4
            pc_matches = args.require_pc is None or all(
                row[4] == args.require_pc for row in chunk
            )
            required_names = {name.upper() for name in args.require_member}
            owner_matches = (not required_names or
                             owner.rsplit("/", 1)[-1].upper() in required_names)
            if (chunk_start <= required_start and chunk_end >= required_end and
                    pc_matches and owner_matches):
                required_chunk_verified = True
        matched[owner] += 1
        exact_matches += 1
        print(
            f"chunk={index} bytes={len(blob)} dest=0x{chunk[0][0]:08x} "
            f"pc0=0x{chunk[0][4]:08x} member={owner} "
            f"iso_offset=0x{offset:x} member_offset=0x{relative:x}"
        )

    print(f"trace_rows={len(rows)}")
    print(f"contiguous_chunks={len(chunks(rows))}")
    print(f"exact_iso_chunk_matches={exact_matches}")
    print(f"source_value_equals_destination={equal_values}/{len(rows)}")
    for name, count in matched.most_common():
        print(f"member={name} exact_chunks={count}")
    required = {name.upper() for name in args.require_member}
    present = {name.rsplit("/", 1)[-1].upper() for name in matched}
    missing = sorted(required - present)
    if missing:
        print("required_members_missing=" + ",".join(missing))
        return 1
    if not required_chunk_verified:
        print("required_destination_source_chunk=missing")
        return 1
    if args.require_destination_range is not None:
        start, end = args.require_destination_range
        print(f"required_destination_source_chunk=verified:0x{start:08x}-0x{end:08x}")
    if not matched:
        print("retail_runtime_source_join=missing")
        return 1
    print("retail_runtime_source_join=verified")
    print("consumer_semantics=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
