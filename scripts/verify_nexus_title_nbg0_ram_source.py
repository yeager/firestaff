#!/usr/bin/env python3
"""Verify the observed CD-labelled SH-2 RAM input to title NBG0 copying.

The receipt proves sector ownership and the bounded destination buffer. It
does not assign the later RAM-to-VDP2 transform performed by the title copy
routine.
"""
from __future__ import annotations

import argparse
import re
from pathlib import Path

HEADERS = {"FIRESTAFF_NEXUS_SH2_RAM_SOURCE_TRACE_V3",
           "FIRESTAFF_NEXUS_SH2_RAM_SOURCE_TRACE_V4"}
LINE = re.compile(r"^addr=0x([0-9a-fA-F]+) size=([0-9]+) value=0x([0-9a-fA-F]+) "
                  r"source=0x([0-9a-fA-F]+) source_value=0x([0-9a-fA-F]+) "
                  r"source_lba=0x([0-9a-fA-F]+)(?: source_word=0x([0-9a-fA-F]+))? "
                  r"pc0=0x([0-9a-fA-F]+) ")
RAM_START, RAM_END = 0x060AC2A7, 0x060B3E27
TITLE_BIN_LBA_START, TITLE_BIN_LBA_END = 6035, 6089

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
        if not lines or lines[0] not in HEADERS:
            raise ValueError("bad source-write trace header")
        title_rows = []
        for number, line in enumerate(lines[1:], 2):
            match = LINE.match(line)
            if not match:
                raise ValueError(f"malformed trace row {number}")
            address, size, value, source, source_value, lba, source_word_text, pc = match.groups()
            address, size, value, source, source_value, lba, pc = (
                int(field, 16) for field in
                (address, size, value, source, source_value, lba, pc))
            source_word = (int(source_word_text, 16)
                           if source_word_text is not None else None)
            if not RAM_START <= address < RAM_END or size != 4:
                raise ValueError("trace row lies outside bounded four-byte RAM lane")
            if TITLE_BIN_LBA_START <= lba <= TITLE_BIN_LBA_END:
                if pc != 0x06090D04:
                    raise ValueError("TITLE.BIN source row has an unexpected SH-2 PC")
                if source != 0x05818000:
                    raise ValueError("TITLE.BIN source row is not the CDB data register")
                if lines[0].endswith("V4") and (source_word is None or
                                              not 8 <= source_word < 1032):
                    raise ValueError("V4 source row lacks a CDB FIFO payload word")
                title_rows.append((lba, address))
        lbas = {lba for lba, _ in title_rows}
        if lbas != set(range(6039, 6056)) or len(title_rows) != 7904:
            raise ValueError("expected TITLE.BIN LBA 6039-6055 source receipt")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_NBG0_RAM_SOURCE_INVALID: {error}")
        return 1
    print("title_nbg0_ram_range=0x060ac2a7-0x060b3e26")
    print("title_nbg0_title_bin_lbas=6039-6055")
    print("title_nbg0_title_bin_cd_to_ram_pc=0x06090d04")
    print("title_nbg0_cd_ram_source=verified")
    print("title_nbg0_ram_to_vdp2_transform=unbound")
    print("semantic_admission=blocked")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
