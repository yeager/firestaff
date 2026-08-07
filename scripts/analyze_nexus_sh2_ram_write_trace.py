#!/usr/bin/env python3
"""Validate SH-2 high-RAM writes observed while a Nexus code image loads.

The trace identifies the runtime loader/writer corridor only. It does not
claim that the bytes came from DM.BIN, TM.BIN, a video asset, or any other
retail file until the source read is joined to an authenticated disc member.
"""

from __future__ import annotations

import argparse
import collections
import re
from pathlib import Path


HEADER = "FIRESTAFF_NEXUS_SH2_RAM_WRITE_TRACE_V1"
LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) size=(?P<size>[0-9]+) "
    r"value=0x(?P<value>[0-9a-fA-F]+) pc0=0x(?P<pc0>[0-9a-fA-F]+) "
    r"pc1=0x(?P<pc1>[0-9a-fA-F]+)$"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--require-pc", type=lambda value: int(value, 0))
    parser.add_argument("--require-address-min", type=lambda value: int(value, 0))
    parser.add_argument("--require-address-max", type=lambda value: int(value, 0))
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
    except (OSError, UnicodeError) as error:
        print(f"NEXUS_SH2_RAM_WRITE_TRACE_INVALID: {error}")
        return 1
    if not lines or lines[0] != HEADER:
        print("NEXUS_SH2_RAM_WRITE_TRACE_INVALID: bad header")
        return 1
    rows: list[tuple[int, int, int, int, int]] = []
    pcs: collections.Counter[int] = collections.Counter()
    for line_number, line in enumerate(lines[1:], 2):
        match = LINE.fullmatch(line)
        if not match:
            print(f"NEXUS_SH2_RAM_WRITE_TRACE_INVALID: malformed line {line_number}")
            return 1
        row = (
            int(match["addr"], 16),
            int(match["size"], 10),
            int(match["value"], 16),
            int(match["pc0"], 16),
            int(match["pc1"], 16),
        )
        rows.append(row)
        pcs[row[3]] += 1
        if row[4]:
            pcs[row[4]] += 1
    print(f"records={len(rows)}")
    print("pc0_counts=" + ",".join(f"0x{pc:08x}:{count}" for pc, count in pcs.most_common()))
    if rows:
        print(f"address_range=0x{min(row[0] for row in rows):08x}-0x{max(row[0] for row in rows):08x}")
    required = rows
    if args.require_pc is not None:
        required = [row for row in required if args.require_pc in row[3:5]]
        print(f"required_pc=0x{args.require_pc:08x}")
    if args.require_address_min is not None:
        required = [row for row in required if row[0] >= args.require_address_min]
        print(f"required_address_min=0x{args.require_address_min:08x}")
    if args.require_address_max is not None:
        required = [row for row in required if row[0] < args.require_address_max]
        print(f"required_address_max=0x{args.require_address_max:08x}")
    if args.require_pc is not None or args.require_address_min is not None or args.require_address_max is not None:
        print(f"required_matches={len(required)}")
    print("runtime_loader_source=BIOS_or_runtime_loader_only")
    print("retail_file_identity=unbound")
    print("semantic_admission=blocked")
    if not required:
        print("required_match=missing")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
