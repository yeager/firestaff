#!/usr/bin/env python3
"""Verify the authentic TITLE.BIN MAPD/palette CD-to-Work-RAM receipt."""
from __future__ import annotations

import argparse
import re
from pathlib import Path

FIFO_HEADER = "FIRESTAFF_NEXUS_CD_FIFO_WORD_TRACE_V1"
SOURCE_HEADER = "FIRESTAFF_NEXUS_SH2_RAM_SOURCE_TRACE_V4"
FIFO_LINE = re.compile(r"^lba=([0-9]+) word=([0-9]+) value=0x([0-9a-fA-F]+)$")
SOURCE_LINE = re.compile(
    r"^addr=0x([0-9a-fA-F]+) size=([0-9]+) value=0x[0-9a-fA-F]+ "
    r"source=0x([0-9a-fA-F]+) source_value=0x[0-9a-fA-F]+ "
    r"source_lba=0x([0-9a-fA-F]+) source_word=0x([0-9a-fA-F]+) "
    r"pc0=0x([0-9a-fA-F]+) ")

FIRST_LBA, LAST_LBA = 6063, 6089
CONTIGUOUS_LAST_LBA = 6088
WORDS_PER_SECTOR = 1024
RAW_SECTOR_BYTES = 2352
USER_OFFSET = 16
RAM_BASE = 0x060B7D80
LOADER_PC = 0x06090D04
CDB_DATA_PORT = 0x05818000
TITLE_BIN_LBA = 6035
MAPD_FILE_OFFSET = 0xE278
PALETTE_FILE_OFFSET = MAPD_FILE_OFFSET + 0x8C54


def track_bytes(track: bytes, lba: int, offset: int, size: int) -> bytes:
    start = lba * RAW_SECTOR_BYTES + USER_OFFSET + offset
    return track[start:start + size]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("fifo", type=Path)
    parser.add_argument("source_writes", type=Path)
    parser.add_argument("track1", type=Path)
    args = parser.parse_args()
    try:
        track = args.track1.read_bytes()
        fifo = args.fifo.read_text(encoding="ascii").splitlines()
        source = args.source_writes.read_text(encoding="ascii").splitlines()
        if not fifo or fifo[0] != FIFO_HEADER:
            raise ValueError("bad FIFO trace header")
        if not source or source[0] != SOURCE_HEADER:
            raise ValueError("bad source-write trace header")
        expected_fifo = [
            (lba, word)
            for lba in range(FIRST_LBA, CONTIGUOUS_LAST_LBA + 1)
            for word in range(8, 8 + WORDS_PER_SECTOR)
        ] + [(LAST_LBA, word) for word in range(8, 825)]
        if len(fifo) != len(expected_fifo) + 1:
            raise ValueError("unexpected MAPD FIFO-word count")
        for line, (lba, word) in zip(fifo[1:], expected_fifo):
            match = FIFO_LINE.fullmatch(line)
            if not match:
                raise ValueError("malformed FIFO row")
            if (int(match[1]), int(match[2])) != (lba, word):
                raise ValueError("MAPD FIFO sequence is discontinuous")
            expected = int.from_bytes(
                track[lba * RAW_SECTOR_BYTES + word * 2:
                      lba * RAW_SECTOR_BYTES + word * 2 + 2], "big")
            if int(match[3], 16) != expected:
                raise ValueError("MAPD FIFO word differs from Track 1")

        rows: dict[int, list[tuple[int, int]]] = {}
        for number, line in enumerate(source[1:], 2):
            match = SOURCE_LINE.match(line)
            if not match:
                raise ValueError(f"malformed source row {number}")
            address, size, port, lba, word, pc = (
                int(field, 16) for field in match.groups())
            if size != 4 or port != CDB_DATA_PORT or pc != LOADER_PC:
                raise ValueError("source row is not the bounded retail loader")
            rows.setdefault(lba, []).append((address, word))
        for lba in range(FIRST_LBA, CONTIGUOUS_LAST_LBA + 1):
            sector_rows = rows.get(lba, [])
            if len(sector_rows) != 512:
                raise ValueError(f"LBA {lba} does not have 512 loader writes")
            expected_address = RAM_BASE + (lba - FIRST_LBA) * 2048
            if [address for address, _ in sector_rows] != [
                    expected_address + index * 4 for index in range(512)]:
                raise ValueError(f"LBA {lba} RAM destination is discontinuous")
            expected_words = list(range(14, 1032, 2)) + [1031, 1031, 1031]
            if [word for _, word in sector_rows] != expected_words:
                raise ValueError(f"LBA {lba} FIFO ownership sequence changed")

        mapd_lba = TITLE_BIN_LBA + MAPD_FILE_OFFSET // 2048
        mapd_offset = MAPD_FILE_OFFSET % 2048
        palette_lba = TITLE_BIN_LBA + PALETTE_FILE_OFFSET // 2048
        palette_offset = PALETTE_FILE_OFFSET % 2048
        if track_bytes(track, mapd_lba, mapd_offset, 12)[:4] != b"MAPD" or \
                track_bytes(track, mapd_lba, mapd_offset, 12)[8:12] != b"TIBG":
            raise ValueError("authenticated RAM mapping does not point to MAPD/TIBG")
        palette = track_bytes(track, palette_lba, palette_offset, 32)
        if len(palette) != 32 or not any(palette):
            raise ValueError("authenticated MAPD palette is empty")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_MAPD_CD_RAM_SOURCE_INVALID: {error}")
        return 1

    mapd_ram = RAM_BASE + (MAPD_FILE_OFFSET - (FIRST_LBA - TITLE_BIN_LBA) * 2048)
    palette_ram = RAM_BASE + (PALETTE_FILE_OFFSET - (FIRST_LBA - TITLE_BIN_LBA) * 2048)
    print("title_mapd_fifo_lbas=6063-6088+6089:8-824")
    print("title_mapd_contiguous_ram_lbas=6063-6088")
    print("title_mapd_ram_range=0x060b7d80-0x060c4d7f")
    print(f"title_mapd_record_ram=0x{mapd_ram:08x}")
    print(f"title_mapd_palette_ram=0x{palette_ram:08x}")
    print("title_mapd_cd_to_ram=verified")
    print("title_mapd_ram_to_vdp2_consumer=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
