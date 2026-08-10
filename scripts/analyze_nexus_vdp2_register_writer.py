#!/usr/bin/env python3
"""Join a VDP2 write trace with its SH-2 register witness.

The register snapshot establishes the runtime destination and source pointer
at a writer PC.  It does not expose the pointed-to RAM bytes, identify a
retail asset, or authorize a menu/HUD/viewport consumer.  This boundary is
kept explicit so a pointer is never mistaken for a FONT256 or CLUT join.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path


REG_LINE = re.compile(r"^addr=0x([0-9a-fA-F]+) pc=0x([0-9a-fA-F]+)(.*)$")
REG_VALUE = re.compile(r" r([0-9]+)=0x([0-9a-fA-F]+)")
WRITE_LINE = re.compile(
    r"^area=vram addr=0x([0-9a-fA-F]+) size=([0-9]+) value=0x([0-9a-fA-F]+) "
    r"pc0=0x([0-9a-fA-F]+) pc1=0x([0-9a-fA-F]+)$"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("writes", type=Path)
    parser.add_argument("registers", type=Path)
    parser.add_argument("--writer-pc", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--destination", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--minimum-writes", type=int, default=64)
    args = parser.parse_args()
    try:
        witness = None
        for line in args.registers.read_text(encoding="ascii").splitlines():
            match = REG_LINE.fullmatch(line)
            if match and int(match.group(1), 16) == args.destination and \
                    int(match.group(2), 16) == args.writer_pc:
                witness = {
                    int(index): int(value, 16)
                    for index, value in REG_VALUE.findall(match.group(3))
                }
                break
        if witness is None:
            raise ValueError("matching register witness is missing")

        rows = []
        for line in args.writes.read_text(encoding="ascii").splitlines():
            match = WRITE_LINE.fullmatch(line)
            if not match:
                continue
            if (int(match.group(1), 16) >= args.destination and
                    int(match.group(4), 16) == args.writer_pc):
                rows.append((int(match.group(1), 16), int(match.group(3), 16)))
        rows.sort()
        if len(rows) < args.minimum_writes:
            raise ValueError(f"only {len(rows)} matching writes")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP2_REGISTER_WRITER_INVALID: {error}")
        return 1

    end = rows[args.minimum_writes - 1][0] + 2
    print(f"writer_pc=0x{args.writer_pc:08x} destination=0x{args.destination:05x}")
    print(f"verified_writes={args.minimum_writes} destination_end=0x{end:05x}")
    print("source_pointer=" +
          (f"0x{witness[4]:08x}" if 4 in witness else "unobserved"))
    print("source_pointer_register=r4")
    print("source_bytes_capture=missing")
    print("vdp2_destination_transport=verified")
    print("asset_identity=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
