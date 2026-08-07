#!/usr/bin/env python3
"""Summarize an authenticated producer-side Saturn VDP2 write witness.

The trace identifies which VDP2 memory windows were written by the retail
runtime. The current external hook intentionally leaves SH-2 PC identity
unbound; address/colour/tile observations therefore remain diagnostics and
never authorize host tilemaps, CLUTs, HUDs, or viewport pixels.
"""

from __future__ import annotations

import argparse
import collections
import re
from pathlib import Path


HEADER = "FIRESTAFF_NEXUS_VDP2_WRITE_TRACE_V1"
LINE = re.compile(
    r"area=(?P<area>vram|cram|regs) addr=0x(?P<addr>[0-9a-fA-F]+) "
    r"size=(?P<size>[0-9]+) value=0x(?P<value>[0-9a-fA-F]+) "
    r"pc0=0x(?P<pc0>[0-9a-fA-F]+) pc1=0x(?P<pc1>[0-9a-fA-F]+)$"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--require-area", action="append", choices=("vram", "cram", "regs"))
    parser.add_argument("--require-pc", type=lambda value: int(value, 0))
    parser.add_argument("--require-minimum", type=int, default=1)
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
    except (OSError, UnicodeError) as error:
        print(f"NEXUS_VDP2_WRITE_TRACE_INVALID: {error}")
        return 1
    if not lines or lines[0] != HEADER:
        print("NEXUS_VDP2_WRITE_TRACE_INVALID: bad header")
        return 1

    counts: collections.Counter[str] = collections.Counter()
    ranges: dict[str, list[int]] = collections.defaultdict(list)
    pages: collections.Counter[tuple[str, int]] = collections.Counter()
    pcs: collections.Counter[tuple[str, int]] = collections.Counter()
    rows = 0
    for line_number, line in enumerate(lines[1:], 2):
        match = LINE.fullmatch(line)
        if not match:
            print(f"NEXUS_VDP2_WRITE_TRACE_INVALID: malformed line {line_number}")
            return 1
        area = match["area"]
        address = int(match["addr"], 16)
        size = int(match["size"], 10)
        if size not in (1, 2, 4):
            print(f"NEXUS_VDP2_WRITE_TRACE_INVALID: invalid size at line {line_number}")
            return 1
        counts[area] += 1
        ranges[area].append(address)
        pages[(area, address // 0x1000)] += 1
        pcs[(area, int(match["pc0"], 16))] += 1
        rows += 1

    print(f"records={rows}")
    for area in ("regs", "vram", "cram"):
        values = ranges.get(area, [])
        if values:
            print(f"{area}_writes={counts[area]} range=0x{min(values):06x}-0x{max(values):06x}")
        else:
            print(f"{area}_writes=0 range=empty")
    print(
        "top_pages="
        + ",".join(
            f"{area}:0x{page * 0x1000:06x}:{count}"
            for (area, page), count in pages.most_common(12)
        )
    )
    observed_pcs = [(area, pc, count) for (area, pc), count in pcs.items() if pc]
    print(
        "top_pcs="
        + ",".join(
            f"{area}:0x{pc:08x}:{count}"
            for area, pc, count in sorted(observed_pcs, key=lambda row: row[2], reverse=True)[:12]
        )
    )
    print("pc_binding=observed" if observed_pcs else "pc_binding=unavailable")
    print("semantic_admission=blocked")
    required = set(args.require_area or ())
    missing = sorted(area for area in required if counts[area] < args.require_minimum)
    if missing:
        print("required_areas_missing=" + ",".join(missing))
        return 1
    if args.require_pc is not None and not any(pc == args.require_pc for _, pc, _ in observed_pcs):
        print(f"required_pc_missing=0x{args.require_pc:08x}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
