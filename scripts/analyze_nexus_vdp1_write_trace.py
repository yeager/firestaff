#!/usr/bin/env python3
"""Summarize an operator-owned SH-2-PC-addressed VDP1 VRAM write trace.

This is a source-owner diagnostic only. A PC and VRAM address do not identify
the retail file or make host rendering safe; the tool therefore always keeps
semantic admission blocked.
"""

from __future__ import annotations

import argparse
import collections
import re
from pathlib import Path


HEADER = "FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V1"
LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) size=(?P<size>[0-9]+) "
    r"value=0x(?P<value>[0-9a-fA-F]+) pc0=0x(?P<pc0>[0-9a-fA-F]+) "
    r"pc1=0x(?P<pc1>[0-9a-fA-F]+)$"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--require-pc", action="store_true")
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
    except (OSError, UnicodeError) as error:
        print(f"NEXUS_VDP1_WRITE_TRACE_INVALID: {error}")
        return 1
    if not lines or lines[0] != HEADER:
        print("NEXUS_VDP1_WRITE_TRACE_INVALID: bad header")
        return 1

    pcs: collections.Counter[str] = collections.Counter()
    addresses: collections.Counter[int] = collections.Counter()
    values: collections.Counter[int] = collections.Counter()
    records = 0
    for line_number, line in enumerate(lines[1:], 2):
        match = LINE.fullmatch(line)
        if not match:
            print(f"NEXUS_VDP1_WRITE_TRACE_INVALID: malformed line {line_number}")
            return 1
        records += 1
        address = int(match["addr"], 16)
        value = int(match["value"], 16)
        pc0 = int(match["pc0"], 16)
        pc1 = int(match["pc1"], 16)
        addresses[address] += 1
        values[value] += 1
        pcs[f"0x{pc0:08x}"] += 1
        if pc1:
            pcs[f"0x{pc1:08x}"] += 1

    print(f"records={records}")
    print("pc0_counts=" + ",".join(f"{pc}:{count}" for pc, count in pcs.most_common()))
    print(f"address_range=0x{min(addresses):05x}-0x{max(addresses):05x}" if addresses else "address_range=empty")
    print("top_values=" + ",".join(f"0x{value:04x}:{count}" for value, count in values.most_common(8)))
    print("semantic_admission=blocked")
    if args.require_pc and not pcs:
        print("required_pc=missing")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
