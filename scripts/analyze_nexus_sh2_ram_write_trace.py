#!/usr/bin/env python3
"""Summarize SH-2 WorkRAMH write rows from an external Nexus capture.

The producer has emitted both legacy rows and frame-stamped rows under the
same V1 header. The summary reports runtime writers only; it does not identify
the source file or assign semantic meaning to a state field.
"""

from __future__ import annotations

import argparse
import collections
import re
from pathlib import Path


HEADER = "FIRESTAFF_NEXUS_SH2_RAM_WRITE_TRACE_V1"
LINE = re.compile(
    r"(?:frame=(?P<frame>[0-9]+) )?"
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
    parser.add_argument("--frame-min", type=int)
    parser.add_argument("--frame-max", type=int)
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
    except (OSError, UnicodeError) as error:
        print(f"NEXUS_SH2_RAM_WRITE_TRACE_INVALID: {error}")
        return 1
    if not lines or lines[0] != HEADER:
        print("NEXUS_SH2_RAM_WRITE_TRACE_INVALID: bad header")
        return 1
    rows: list[tuple[int | None, int, int, int, int, int]] = []
    pcs: collections.Counter[int] = collections.Counter()
    for line_number, line in enumerate(lines[1:], 2):
        match = LINE.fullmatch(line)
        if not match:
            print(f"NEXUS_SH2_RAM_WRITE_TRACE_INVALID: malformed line {line_number}")
            return 1
        frame = int(match["frame"]) if match["frame"] is not None else None
        size = int(match["size"], 10)
        address = int(match["addr"], 16)
        if size not in (1, 2, 4):
            print(f"NEXUS_SH2_RAM_WRITE_TRACE_INVALID: invalid write width at line {line_number}")
            return 1
        if not 0x06000000 <= address or address + size > 0x06100000:
            print(f"NEXUS_SH2_RAM_WRITE_TRACE_INVALID: address outside WorkRAMH at line {line_number}")
            return 1
        row = (
            frame,
            address,
            size,
            int(match["value"], 16),
            int(match["pc0"], 16),
            int(match["pc1"], 16),
        )
        rows.append(row)
        pcs[row[4]] += 1
        if row[5]:
            pcs[row[5]] += 1
    if not rows:
        print("NEXUS_SH2_RAM_WRITE_TRACE_INVALID: no write rows")
        return 1
    framed = [row[0] is not None for row in rows]
    if any(framed) and not all(framed):
        print("NEXUS_SH2_RAM_WRITE_TRACE_INVALID: mixed framed and legacy rows")
        return 1
    if ((args.frame_min is not None and args.frame_min < 0) or
            (args.frame_max is not None and args.frame_max < 0) or
            (args.frame_min is not None and args.frame_max is not None and
             args.frame_min > args.frame_max)):
        print("NEXUS_SH2_RAM_WRITE_TRACE_INVALID: invalid frame bounds")
        return 1
    if any(framed):
        rows = [row for row in rows
                if (args.frame_min is None or row[0] >= args.frame_min) and
                   (args.frame_max is None or row[0] <= args.frame_max)]
        if not rows:
            print("NEXUS_SH2_RAM_WRITE_TRACE_INVALID: no rows inside requested frame bounds")
            return 1
    elif args.frame_min is not None or args.frame_max is not None:
        print("NEXUS_SH2_RAM_WRITE_TRACE_INVALID: legacy rows have no frame identity")
        return 1
    pcs.clear()
    for row in rows:
        pcs[row[4]] += 1
        if row[5]:
            pcs[row[5]] += 1
    print(f"records={len(rows)}")
    if any(framed):
        print(f"frames={min(row[0] for row in rows)}-{max(row[0] for row in rows)}")
    print("pc0_counts=" + ",".join(f"0x{pc:08x}:{count}" for pc, count in pcs.most_common()))
    if rows:
        print(f"address_range=0x{min(row[1] for row in rows):08x}-0x{max(row[1] for row in rows):08x}")
    required = rows
    if args.require_pc is not None:
        required = [row for row in required if args.require_pc in row[4:6]]
        print(f"required_pc=0x{args.require_pc:08x}")
    if args.require_address_min is not None:
        required = [row for row in required if row[1] >= args.require_address_min]
        print(f"required_address_min=0x{args.require_address_min:08x}")
    if args.require_address_max is not None:
        required = [row for row in required if row[1] < args.require_address_max]
        print(f"required_address_max=0x{args.require_address_max:08x}")
    if args.require_pc is not None or args.require_address_min is not None or args.require_address_max is not None:
        print(f"required_matches={len(required)}")
    print("runtime_writer_identity=observed")
    print("retail_file_identity=unbound")
    print("semantic_admission=blocked")
    if not required:
        print("required_match=missing")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
