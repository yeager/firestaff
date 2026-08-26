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


def trace_records(path: Path) -> list[tuple[str, int, int, int, int, int]]:
    """Parse one VDP2 producer trace without losing byte-write semantics.

    Mednafen's VDP2 bus callback receives a 16-bit data-bus value even for
    an 8-bit transfer.  VDP2 itself applies ``0xff00`` to an even address
    and ``0x00ff`` to an odd address.  Consumers which simply truncate the
    recorded value therefore fabricate a byte stream that was never written.
    """
    try:
        lines = path.read_text(encoding="ascii").splitlines()
    except (OSError, UnicodeError) as error:
        raise ValueError(str(error)) from error
    if not lines or lines[0] != HEADER:
        raise ValueError("bad header")
    records: list[tuple[str, int, int, int, int, int]] = []
    for line_number, line in enumerate(lines[1:], 2):
        match = LINE.fullmatch(line)
        if not match:
            raise ValueError(f"malformed line {line_number}")
        size = int(match["size"], 10)
        if size not in (1, 2, 4):
            raise ValueError(f"invalid size at line {line_number}")
        records.append((match["area"], int(match["addr"], 16), size,
                        int(match["value"], 16), int(match["pc0"], 16),
                        int(match["pc1"], 16)))
    return records


def replay_vram_bus_writes(path: Path) -> tuple[bytes, int]:
    """Replay VDP2 VRAM writes in VDP2 bus-byte order.

    This is a narrow provenance reconstruction, not a renderer.  Addresses
    are folded exactly as VDP2's ``RW`` routine folds its 512 KiB VRAM
    backing store.  Four-byte records are deliberately rejected: the retail
    title witness uses byte writes and the trace schema cannot represent a
    32-bit VDP2 data value without truncation.
    """
    image = bytearray(0x80000)
    writes = 0
    for area, address, size, value, _, _ in trace_records(path):
        if area != "vram":
            continue
        offset = address & 0x7ffff
        if size == 1:
            image[offset] = (value >> 8) & 0xff if (address & 1) == 0 else value & 0xff
        elif size == 2:
            if offset + 2 > len(image):
                raise ValueError("word write crosses VRAM boundary")
            image[offset:offset + 2] = value.to_bytes(2, "big")
        else:
            raise ValueError("four-byte VDP2 write cannot be replayed safely")
        writes += 1
    return bytes(image), writes


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--require-area", action="append", choices=("vram", "cram", "regs"))
    parser.add_argument("--require-pc", type=lambda value: int(value, 0))
    parser.add_argument("--require-minimum", type=int, default=1)
    args = parser.parse_args()
    try:
        records = trace_records(args.trace)
    except ValueError as error:
        print(f"NEXUS_VDP2_WRITE_TRACE_INVALID: {error}")
        return 1

    counts: collections.Counter[str] = collections.Counter()
    ranges: dict[str, list[int]] = collections.defaultdict(list)
    pages: collections.Counter[tuple[str, int]] = collections.Counter()
    pcs: collections.Counter[tuple[str, int]] = collections.Counter()
    rows = 0
    for area, address, _size, _value, pc0, _pc1 in records:
        counts[area] += 1
        ranges[area].append(address)
        pages[(area, address // 0x1000)] += 1
        pcs[(area, pc0)] += 1
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
