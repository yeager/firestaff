#!/usr/bin/env python3
"""Bind authentic VDP2 title-copy writes to the observed SH-2 pointer route.

This deliberately records destination byte values only.  It does not infer
that the SH-2 cache's source byte is identical to a disc member.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

from verify_nexus_title_nbg0_copy_routing import COPY_BYTES, LINE


WRITE_HEADER = "FIRESTAFF_NEXUS_VDP2_WRITE_TRACE_V1"
WRITE = re.compile(
    r"^area=(?P<area>[A-Za-z]+) addr=0x(?P<address>[0-9a-fA-F]+) "
    r"size=(?P<size>[0-9]+) value=0x(?P<value>[0-9a-fA-F]+) "
    r"pc0=0x(?P<pc0>[0-9a-fA-F]+) pc1=0x(?P<pc1>[0-9a-fA-F]+)$"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("writer_registers", type=Path)
    parser.add_argument("writes", type=Path)
    args = parser.parse_args()
    try:
        registers = args.writer_registers.read_text(encoding="ascii").splitlines()
        writes = args.writes.read_text(encoding="ascii").splitlines()
        if not registers or not registers[0].startswith(
                "FIRESTAFF_NEXUS_VDP2_WRITER_REGISTER_TRACE_V1"):
            raise ValueError("bad writer-register trace header")
        if not writes or writes[0] != WRITE_HEADER:
            raise ValueError("bad VDP2 write trace header")
        if len(registers) != len(writes):
            raise ValueError("writer-register and VDP2 write trace lengths differ")
        rows = []
        for register_line, write_line in zip(registers[1:], writes[1:]):
            route = LINE.match(register_line)
            if not route:
                continue
            write = WRITE.match(write_line)
            if not write:
                raise ValueError("malformed paired VDP2 write row")
            address = int(route["address"], 16)
            if (write["area"] != "vram" or int(write["size"]) != 1 or
                    int(write["pc0"], 16) != 0x0602312C or
                    int(write["address"], 16) != address):
                raise ValueError("title route row is not its paired VDP2 byte write")
            rows.append(int(write["value"], 16) & 0xFF)
        if len(rows) != COPY_BYTES:
            raise ValueError(f"expected {COPY_BYTES} paired title bytes, got {len(rows)}")
        if not any(rows):
            raise ValueError("captured title byte sequence is unexpectedly all zero")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_NBG0_COPY_VALUES_INVALID: {error}")
        return 1

    print("title_nbg0_copy_pc=0x0602312c")
    print(f"title_nbg0_copy_value_bytes={len(rows)}")
    print("title_nbg0_copy_vdp2_values=verified")
    print("title_nbg0_copy_source_byte_value=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
