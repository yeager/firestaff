#!/usr/bin/env python3
"""Summarize authenticated sound-CPU SCSP reads without inferring semantics."""

from __future__ import annotations

import argparse
import re
from collections import Counter
from pathlib import Path


TRACE_HEADER = "FIRESTAFF_NEXUS_SCSP_READ_TRACE_V1"
TRACE_LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) size=(?P<size>[0-9]+) "
    r"value=0x(?P<value>[0-9a-fA-F]+) pc=0x(?P<pc>[0-9a-fA-F]+)$"
)


def read_rows(path: Path) -> list[dict[str, int]]:
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != TRACE_HEADER:
        raise ValueError(f"{path}: bad trace header")
    rows: list[dict[str, int]] = []
    for number, line in enumerate(lines[1:], 2):
        match = TRACE_LINE.fullmatch(line)
        if not match:
            raise ValueError(f"{path}: malformed line {number}")
        rows.append({key: int(value, 16 if key != "size" else 10)
                     for key, value in match.groupdict().items()})
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--require-pc", type=lambda value: int(value, 0))
    parser.add_argument("--require-address", type=lambda value: int(value, 0))
    args = parser.parse_args()
    try:
        rows = read_rows(args.trace)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_SCSP_READ_TRACE_INVALID: {error}")
        return 1

    addresses = Counter(row["addr"] for row in rows)
    pcs = Counter(row["pc"] for row in rows)
    print(f"read_count={len(rows)}")
    print("addresses=" + ",".join(f"0x{addr:06x}:{count}" for addr, count in addresses.most_common()))
    print("pcs=" + ",".join(f"0x{pc:08x}:{count}" for pc, count in pcs.most_common()))
    if args.require_pc is not None:
        matches = [row for row in rows if row["pc"] == args.require_pc]
        print(f"required_pc=0x{args.require_pc:08x}")
        print(f"required_pc_matches={len(matches)}")
    else:
        matches = rows
    if args.require_address is not None:
        address_matches = [row for row in matches if row["addr"] == args.require_address]
        print(f"required_address=0x{args.require_address:06x}")
        print(f"required_address_matches={len(address_matches)}")
    else:
        address_matches = matches
    for row in address_matches[:16]:
        print("match addr=0x{addr:06x} size={size} value=0x{value:08x} pc=0x{pc:08x}".format(**row))
    print("semantic_admission=blocked")
    return 0 if matches and address_matches else 1


if __name__ == "__main__":
    raise SystemExit(main())
