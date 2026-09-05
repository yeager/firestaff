#!/usr/bin/env python3
"""Verify the retail Nexus MAPD plane sequence and exact VDP2 transfer."""
from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path

MAPD_OFFSET = 0xE278
MAP_HEADER = 0x40
MAP_STRIDE = 0x1C04
MAP_BYTES = 0x1C00
MAP_WORDS = MAP_BYTES // 2
MAP_COUNT = 5
VDP2_BASE = 0x5C000
EXPECTED_FRAMES = (13294, 13334, 13375, 13415, 13455)
EXPECTED_STATES = tuple(0xFF10 + index for index in range(MAP_COUNT))
READ_MAGIC = "FIRESTAFF_NEXUS_SH2_CACHED_RAM_READ_TRACE_V2"
WRITE_MAGIC = "FIRESTAFF_NEXUS_VDP2_WRITE_TRACE_V1"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def metadata(path: Path) -> dict[str, str]:
    return dict(row.split("=", 1) for row in path.read_text().splitlines()
                if "=" in row)


def title_bin(track: Path) -> bytes:
    image = track.read_bytes()
    sector = 2352 if len(image) % 2352 == 0 else 2048
    user = 16 if sector == 2352 else 0

    def payload(lba: int) -> bytes:
        start = lba * sector + user
        return image[start:start + 2048]

    pvd = payload(16)
    if len(pvd) != 2048 or pvd[1:6] != b"CD001":
        raise ValueError("Track 1 has no ISO9660 PVD")
    root = pvd[156:190]
    root_lba = int.from_bytes(root[2:6], "little")
    root_size = int.from_bytes(root[10:14], "little")
    directory = b"".join(payload(root_lba + offset)
                         for offset in range((root_size + 2047) // 2048))[:root_size]
    pos = 0
    while pos < len(directory):
        length = directory[pos]
        if not length:
            pos = (pos // 2048 + 1) * 2048
            continue
        record = directory[pos:pos + length]
        name_len = record[32]
        name = record[33:33 + name_len].decode("ascii", "replace").split(";", 1)[0]
        if name.upper() == "TITLE.BIN":
            lba = int.from_bytes(record[2:6], "little")
            size = int.from_bytes(record[10:14], "little")
            return b"".join(payload(lba + offset)
                            for offset in range((size + 2047) // 2048))[:size]
        pos += length
    raise ValueError("TITLE.BIN is absent from Track 1")


def parse_fields(row: str) -> dict[str, str]:
    return dict(field.split("=", 1) for field in row.split() if "=" in field)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("track1", type=Path)
    args = parser.parse_args()
    try:
        cap = args.capture
        manifest_path = cap / "manifest.txt"
        reads_path = cap / "mapd-cached-reads.trace"
        writes_path = cap / "vdp2-writes.trace"
        raw_path = cap / "runtime-vdp12.raw"
        meta = metadata(manifest_path)
        if meta.get("capture_exit_status") != "0" or meta.get("frame_limit") != "1":
            raise ValueError("capture status is not successful")
        for path, key in (
            (reads_path, "FIRESTAFF_NEXUS_TRACE_SH2_CACHED_RAM_READS_sha256"),
            (writes_path, "FIRESTAFF_NEXUS_TRACE_VDP2_WRITES_sha256"),
            (raw_path, "raw_sha256"),
        ):
            if meta.get(key) != digest(path):
                raise ValueError(f"manifest hash mismatch: {path.name}")

        asset = title_bin(args.track1)
        record = asset[MAPD_OFFSET:]
        if record[:4] != b"MAPD" or record[8:12] != b"TIBG":
            raise ValueError("retail MAPD/TIBG signature missing")
        maps = []
        for index in range(MAP_COUNT):
            start = MAP_HEADER + index * MAP_STRIDE
            if record[start:start + 4] != b"\x00\x40\x00\x1c":
                raise ValueError(f"MAPD plane {index} is not 64x28")
            plane = record[start + 4:start + 4 + MAP_BYTES]
            if len(plane) != MAP_BYTES:
                raise ValueError(f"MAPD plane {index} is truncated")
            maps.append(plane)

        read_rows = reads_path.read_text().splitlines()
        if not read_rows or read_rows[0] != READ_MAGIC:
            raise ValueError("cached-read trace header mismatch")
        selections: list[tuple[int, int, int, int]] = []
        for row in read_rows[1:]:
            fields = parse_fields(row)
            if fields.get("pc") == "0x0603f61a":
                frame = int(fields["frame"])
                state = int(fields["r4"], 16)
                address = int(fields["addr"], 16)
                width = int(fields["value"], 16)
                selections.append((frame, state, address, width))
        expected_addresses = tuple(0x060B8038 + index * MAP_STRIDE
                                   for index in range(MAP_COUNT))
        expected = list(zip(EXPECTED_FRAMES, EXPECTED_STATES,
                            expected_addresses, (64,) * MAP_COUNT))
        if selections != expected:
            raise ValueError("MAPD plane selection order/timing mismatch")
        for frame, address in zip(EXPECTED_FRAMES, expected_addresses):
            if not any(f"frame={frame} " in row and
                       f"addr=0x{address + 2:08x} " in row and
                       "value=0x0000001c pc=0x0603f620" in row
                       for row in read_rows[1:]):
                raise ValueError(f"MAPD height receipt missing at frame {frame}")

        write_rows = writes_path.read_text().splitlines()
        if not write_rows or write_rows[0] != WRITE_MAGIC:
            raise ValueError("VDP2 write trace header mismatch")
        writes: list[tuple[int, int]] = []
        pattern = re.compile(r"^area=vram addr=0x([0-9a-f]+) size=2 "
                             r"value=0x([0-9a-f]+) pc0=0x0608b56[48] ")
        for row in write_rows[1:]:
            match = pattern.match(row)
            if match:
                address = int(match.group(1), 16)
                if VDP2_BASE <= address < VDP2_BASE + MAP_BYTES:
                    writes.append((address, int(match.group(2), 16)))
        if len(writes) != MAP_COUNT * MAP_WORDS:
            raise ValueError(f"expected {MAP_COUNT * MAP_WORDS} MAPD writes, got {len(writes)}")
        for index, plane in enumerate(maps):
            chunk = writes[index * MAP_WORDS:(index + 1) * MAP_WORDS]
            expected_words = [(VDP2_BASE + word * 2,
                               int.from_bytes(plane[word * 2:word * 2 + 2], "big"))
                              for word in range(MAP_WORDS)]
            if chunk != expected_words:
                raise ValueError(f"MAPD plane {index} differs from VDP2 transfer")
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"NEXUS_TITLE_MAPD_VDP2_TRANSFER_INVALID: {error}")
        return 1
    print("title_mapd_plane_order=N,E,X,U,S")
    print("title_mapd_frames=13294,13334,13375,13415,13455")
    print("title_mapd_vdp2_vram=0x05c000-0x05dbff")
    print("title_mapd_words_verified=17920")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
