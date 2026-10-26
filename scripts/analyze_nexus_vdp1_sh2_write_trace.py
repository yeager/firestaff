#!/usr/bin/env python3
"""Summarize a bounded Ymir SH-2 write trace for Saturn VDP1 VRAM.

This is an evidence tool.  It neither decodes graphics nor assigns a game
asset to a write.  Its purpose is to retain the program-counter and address
range facts needed before a Nexus renderer can claim source ownership.
"""

from __future__ import annotations

import argparse
import csv
from collections import Counter
from pathlib import Path


VDP1_VRAM_BEGIN = 0x05C00000
VDP1_VRAM_END = 0x05C80000
REQUIRED_COLUMNS = {
    "sh2", "pc", "caller_return_address", "caller_target", "address", "width", "value"
}


def parse_number(text: str) -> int:
    return int(text, 0)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--source-address", action="append", default=[],
                        help="VDP1 physical address to check for a trace write")
    args = parser.parse_args()
    try:
        with args.trace.open(newline="", encoding="utf-8") as handle:
            reader = csv.DictReader(handle)
            if reader.fieldnames is None or not REQUIRED_COLUMNS.issubset(reader.fieldnames):
                raise ValueError("missing Ymir SH-2 write-trace columns")
            rows = list(reader)
    except (OSError, csv.Error, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP1_SH2_WRITE_TRACE_INVALID: {error}")
        return 1

    try:
        parsed = [
            (parse_number(row["pc"]), parse_number(row["address"]), parse_number(row["width"]))
            for row in rows
        ]
    except (KeyError, TypeError, ValueError) as error:
        print(f"NEXUS_VDP1_SH2_WRITE_TRACE_INVALID: {error}")
        return 1
    if any(address < VDP1_VRAM_BEGIN or address + width > VDP1_VRAM_END
           for _, address, width in parsed):
        print("NEXUS_VDP1_SH2_WRITE_TRACE_INVALID: write is outside VDP1 VRAM")
        return 1

    by_pc = Counter(pc for pc, _, _ in parsed)
    print(f"vdp1_write_rows={len(parsed)}")
    for pc, count in by_pc.most_common(12):
        addresses = [address for entry_pc, address, _ in parsed if entry_pc == pc]
        print(f"pc=0x{pc:08x} writes={count} address_min=0x{min(addresses):08x} "
              f"address_max=0x{max(addresses):08x}")
    for text in args.source_address:
        try:
            source = parse_number(text)
        except ValueError:
            print(f"NEXUS_VDP1_SH2_WRITE_TRACE_INVALID: invalid source address {text!r}")
            return 1
        hits = sum(address <= source < address + width for _, address, width in parsed)
        print(f"source_address=0x{source:08x} writes_covering_source={hits}")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
