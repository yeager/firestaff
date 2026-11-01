#!/usr/bin/env python3
"""Derive physical Track 02 trace addresses for authentic JP Akutuba maps."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path


TRACK02_MD5 = "b7afb338ad31be1025b53f9aff12d73a"
TRACK02_BYTES = 8_102_640
RAW_SECTOR_BYTES = 2352
MODE1_USER_OFFSET = 16
MODE1_USER_BYTES = 2048
JP_INDEX01_FILE_SECTOR = 224
JP_TRACK02_INDEX01_DISC_LBA = 3590
LOADER_USER_PREFIX_SECTORS = 225
AKUTUBA_MAP_COUNT = 4
AKUTUBA_DIMS_OFFSET = 0x2991D
AKUTUBA_MAP_DATA_OFFSET = 0x29B03


def fail(message: str) -> None:
    raise SystemExit(f"FAIL: {message}")


def read_user_range(raw: bytes, start: int, size: int) -> bytes:
    """Read a bounded range in the raw BIN's MODE1/2352 user-data stream."""
    sectors = len(raw) // RAW_SECTOR_BYTES
    if start < 0 or size < 0 or start + size > sectors * MODE1_USER_BYTES:
        fail("requested user-data range is outside authenticated Track 02")
    chunks: list[bytes] = []
    cursor = start
    remaining = size
    while remaining:
        sector, in_sector = divmod(cursor, MODE1_USER_BYTES)
        take = min(remaining, MODE1_USER_BYTES - in_sector)
        raw_start = sector * RAW_SECTOR_BYTES + MODE1_USER_OFFSET + in_sector
        chunks.append(raw[raw_start : raw_start + take])
        cursor += take
        remaining -= take
    return b"".join(chunks)


def raw_segments(user_start: int, size: int) -> list[tuple[int, int, int]]:
    """Return (track sector, raw start, raw end-exclusive) for each span."""
    segments = []
    cursor = user_start
    end = user_start + size
    while cursor < end:
        sector, in_sector = divmod(cursor, MODE1_USER_BYTES)
        take = min(end - cursor, MODE1_USER_BYTES - in_sector)
        raw_start = sector * RAW_SECTOR_BYTES + MODE1_USER_OFFSET + in_sector
        segments.append((sector, raw_start, raw_start + take))
        cursor += take
    return segments


def describe_span(label: str, user_start: int, size: int) -> None:
    for sector, raw_start, raw_end in raw_segments(user_start, size):
        relative_lba = sector - JP_INDEX01_FILE_SECTOR
        disc_lba = JP_TRACK02_INDEX01_DISC_LBA + relative_lba
        print(
            f"{label}: user=[0x{user_start:x},0x{user_start + size:x}) "
            f"raw=[0x{raw_start:x},0x{raw_end:x}) "
            f"track-sector={sector} track-lba={relative_lba} disc-lba={disc_lba}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("track02", type=Path, help="authentic JP Rev. 1 TQJP02.bin")
    parser.add_argument("--x", type=int, default=2, help="tile column (default: 2)")
    parser.add_argument("--y", type=int, default=3, help="tile row (default: 3)")
    parser.add_argument(
        "--dump-grids",
        action="store_true",
        help="print each authenticated map's raw tile bytes by row (y) and column (x)",
    )
    args = parser.parse_args()

    raw = args.track02.read_bytes()
    if len(raw) != TRACK02_BYTES or hashlib.md5(raw).hexdigest() != TRACK02_MD5:
        fail("input is not the authenticated Japanese Rev. 1 Track 02 BIN")
    if len(raw) % RAW_SECTOR_BYTES:
        fail("Track 02 length is not aligned to MODE1/2352 sectors")

    sectors = len(raw) // RAW_SECTOR_BYTES
    for sector in (JP_INDEX01_FILE_SECTOR, LOADER_USER_PREFIX_SECTORS + 83):
        offset = sector * RAW_SECTOR_BYTES
        header = raw[offset : offset + MODE1_USER_OFFSET]
        if (len(header) != MODE1_USER_OFFSET or header[0] != 0 or
                header[1:11] != b"\xff" * 10 or header[11] != 0 or
                header[15] != 1):
            fail(f"raw sector {sector} is not a MODE1 sector")

    ud_base = LOADER_USER_PREFIX_SECTORS * MODE1_USER_BYTES
    descriptor_start = ud_base + AKUTUBA_DIMS_OFFSET
    descriptor_size = 9 * AKUTUBA_MAP_COUNT + 32
    descriptor = read_user_range(raw, descriptor_start, descriptor_size)
    x_dims = descriptor[:AKUTUBA_MAP_COUNT]
    y_dims = descriptor[AKUTUBA_MAP_COUNT : 2 * AKUTUBA_MAP_COUNT]
    descriptor_fields = [
        descriptor[index * AKUTUBA_MAP_COUNT : (index + 1) * AKUTUBA_MAP_COUNT]
        for index in range(9)
    ]
    map_cursor = ud_base + AKUTUBA_MAP_DATA_OFFSET

    print(f"track02_md5={TRACK02_MD5}")
    print(f"track02_sha256={hashlib.sha256(raw).hexdigest()}")
    print(f"raw_sectors={sectors}")
    print(f"jp_index01_file_sector={JP_INDEX01_FILE_SECTOR}")
    print(f"jp_index01_disc_lba={JP_TRACK02_INDEX01_DISC_LBA}")
    print(f"loader_user_prefix_sectors={LOADER_USER_PREFIX_SECTORS}")
    print(f"akutuba_dims_offset=0x{AKUTUBA_DIMS_OFFSET:x}")
    print(f"akutuba_map_data_offset=0x{AKUTUBA_MAP_DATA_OFFSET:x}")
    print(f"akutuba_descriptor_sha256={hashlib.sha256(descriptor).hexdigest()}")
    describe_span("descriptor", descriptor_start, descriptor_size)
    if args.dump_grids:
        descriptor_names = (
            "x_dim", "y_dim", "x_offset", "y_offset", "map_id",
            "unknown_1", "unknown_2", "creature_count", "xp_modifier",
        )
        print("map descriptor bytes (unknown fields remain uninterpreted):")
        for map_index in range(AKUTUBA_MAP_COUNT):
            fields = " ".join(
                f"{name}=0x{descriptor_fields[field_index][map_index]:02x}"
                for field_index, name in enumerate(descriptor_names)
            )
            print(f"  map[{map_index}]: {fields}")

    total_map_bytes = sum((x_dims[i] + 1) * (y_dims[i] + 1)
                          for i in range(AKUTUBA_MAP_COUNT))
    map_bytes = read_user_range(raw, map_cursor, total_map_bytes)
    print(f"akutuba_map_arrays_sha256={hashlib.sha256(map_bytes).hexdigest()}")
    tile_cursor = map_cursor
    for map_index in range(AKUTUBA_MAP_COUNT):
        width = x_dims[map_index] + 1
        height = y_dims[map_index] + 1
        tile_count = width * height
        if args.x < 0 or args.x >= width or args.y < 0 or args.y >= height:
            tile = "out-of-bounds"
            tile_user_offset = tile_cursor
        else:
            # The source tile arrays are column-major: x is the outer index.
            tile_index = args.x * height + args.y
            tile_user_offset = tile_cursor + tile_index
            tile = f"0x{read_user_range(raw, tile_user_offset, 1)[0]:02x}"
        print(
            f"map[{map_index}]={width}x{height} "
            f"tiles_user=[0x{tile_cursor:x},0x{tile_cursor + tile_count:x}) "
            f"tile({args.x},{args.y})={tile} "
            f"tile_user_offset=0x{tile_user_offset:x}"
        )
        if args.dump_grids:
            grid_bytes = read_user_range(raw, tile_cursor, tile_count)
            print(f"map[{map_index}].raw_tile_bytes rows=y columns=x:")
            for y in range(height):
                row = [grid_bytes[x * height + y] for x in range(width)]
                print(f"  y={y:02d}: " + " ".join(f"{value:02x}" for value in row))
        describe_span(f"map[{map_index}]", tile_cursor, tile_count)
        if tile != "out-of-bounds":
            describe_span(f"map[{map_index}].tile({args.x},{args.y})",
                          tile_user_offset, 1)
        tile_cursor += tile_count
    describe_span("all_map_arrays", map_cursor, total_map_bytes)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
