#!/usr/bin/env python3
"""Verify authentic Nexus TITLE.BIN CDB FIFO words against raw Track 1."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


HEADER = "FIRESTAFF_NEXUS_CD_FIFO_WORD_TRACE_V1"
LINE = re.compile(r"^lba=([0-9]+) word=([0-9]+) value=0x([0-9a-fA-F]+)$")
FIRST_LBA = 6039
LAST_LBA = 6055
WORDS_PER_SECTOR = 1024
RAW_SECTOR_BYTES = 2352


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("track1", type=Path)
    args = parser.parse_args()
    try:
        rows = args.trace.read_text(encoding="ascii").splitlines()
        track = args.track1.read_bytes()
        if not rows or rows[0] != HEADER:
            raise ValueError("bad CDB FIFO-word trace header")
        if len(rows) != 1 + (LAST_LBA - FIRST_LBA + 1) * WORDS_PER_SECTOR:
            raise ValueError("unexpected CDB FIFO-word count")
        expected = [(lba, word)
                    for lba in range(FIRST_LBA, LAST_LBA + 1)
                    for word in range(8, 8 + WORDS_PER_SECTOR)]
        for row, (lba_expected, word_expected) in zip(rows[1:], expected):
            match = LINE.match(row)
            if not match:
                raise ValueError("malformed CDB FIFO-word row")
            lba, word, value = (int(match[1]), int(match[2]), int(match[3], 16))
            if (lba, word) != (lba_expected, word_expected):
                raise ValueError("CDB FIFO-word sequence is discontinuous")
            offset = lba * RAW_SECTOR_BYTES + word * 2
            if offset + 2 > len(track):
                raise ValueError("FIFO word lies outside Track 1")
            if int.from_bytes(track[offset:offset + 2], "big") != value:
                raise ValueError("FIFO word differs from authentic Track 1")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_CDB_FIFO_WORDS_INVALID: {error}")
        return 1

    print("title_bin_fifo_lbas=6039-6055")
    print("title_bin_fifo_words=17408")
    print("title_bin_fifo_track1_bytes=verified")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
