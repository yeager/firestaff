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
    parser.add_argument("--require-pc", type=lambda value: int(value, 0))
    parser.add_argument("--require-address", type=lambda value: int(value, 0))
    parser.add_argument("--require-address-min", type=lambda value: int(value, 0))
    parser.add_argument("--require-address-max", type=lambda value: int(value, 0))
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
    rows: list[tuple[int, int, int, int, int]] = []
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
        rows.append((address, int(match["size"], 10), value, pc0, pc1))
        addresses[address] += 1
        values[value] += 1
        pcs[f"0x{pc0:08x}"] += 1
        if pc1:
            pcs[f"0x{pc1:08x}"] += 1

    print(f"records={records}")
    print("pc0_counts=" + ",".join(f"{pc}:{count}" for pc, count in pcs.most_common()))
    print(f"address_range=0x{min(addresses):05x}-0x{max(addresses):05x}" if addresses else "address_range=empty")
    print("top_values=" + ",".join(f"0x{value:04x}:{count}" for value, count in values.most_common(8)))
    required = rows
    if args.require_pc is not None:
        required = [row for row in required if args.require_pc in row[3:5]]
        print(f"required_pc=0x{args.require_pc:08x}")
    if args.require_address is not None:
        required = [row for row in required if row[0] == args.require_address]
        print(f"required_address=0x{args.require_address:05x}")
    if args.require_address_min is not None:
        required = [row for row in required if row[0] >= args.require_address_min]
        print(f"required_address_min=0x{args.require_address_min:05x}")
    if args.require_address_max is not None:
        required = [row for row in required if row[0] < args.require_address_max]
        print(f"required_address_max=0x{args.require_address_max:05x}")
    if args.require_pc is not None or args.require_address is not None or \
            args.require_address_min is not None or args.require_address_max is not None:
        print(f"required_matches={len(required)}")
        for address, size, value, pc0, pc1 in required[:16]:
            print(f"match addr=0x{address:05x} size={size} value=0x{value:04x} "
                  f"pc0=0x{pc0:08x} pc1=0x{pc1:08x}")
    print("semantic_admission=blocked")
    if (args.require_pc is not None or args.require_address is not None or
            args.require_address_min is not None or args.require_address_max is not None) and not required:
        print("required_match=missing")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
