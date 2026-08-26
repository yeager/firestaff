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
        # The VDP2 trace can deliberately contain earlier writes by this same
        # SH-2 routine.  The writer-register trace is frame-gated to the title
        # copy, so locate its contiguous destination sequence rather than
        # assuming both trace files begin at the same event.
        if len(writes) < len(registers):
            raise ValueError("VDP2 write trace is shorter than writer-register trace")
        routes = []
        for register_line in registers[1:]:
            route = LINE.match(register_line)
            if not route:
                raise ValueError("malformed writer-register trace row")
            routes.append(int(route["address"], 16))
        if len(routes) != COPY_BYTES:
            raise ValueError(f"expected {COPY_BYTES} title route rows, got {len(routes)}")

        parsed_writes = []
        for write_line in writes[1:]:
            write = WRITE.match(write_line)
            if not write:
                raise ValueError("malformed VDP2 write row")
            parsed_writes.append(write)

        starts = [index for index, write in enumerate(parsed_writes)
                  if (write["area"] == "vram" and int(write["size"]) == 1 and
                      int(write["pc0"], 16) == 0x0602312C and
                      int(write["address"], 16) == routes[0])]
        if not starts:
            raise ValueError("VDP2 write trace does not contain title-copy start")
        start = None
        for candidate in starts:
            candidate_rows = parsed_writes[candidate:candidate + len(routes)]
            if len(candidate_rows) != len(routes):
                continue
            if all(write["area"] == "vram" and int(write["size"]) == 1 and
                   int(write["pc0"], 16) == 0x0602312C and
                   int(write["address"], 16) == address
                   for address, write in zip(routes, candidate_rows)):
                start = candidate
                break
        if start is None:
            raise ValueError("VDP2 write trace lacks contiguous title-copy destination sequence")
        rows = []
        for address, write in zip(routes, parsed_writes[start:start + len(routes)]):
            # VDP2 receives a 16-bit bus value even for an 8-bit access.  The
            # addressed byte lane is selected by A0 (see vdp2.cpp's mask), so
            # the low byte alone is not the written byte at odd addresses.
            bus_value = int(write["value"], 16)
            rows.append((bus_value >> (8 if (address & 1) == 0 else 0)) & 0xFF)
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
