#!/usr/bin/env python3
"""Validate the observed SH-2 pointer corridor for the retail title copy.

This validates only the post-instruction register route observed at each VDP2
byte write.  It intentionally does not infer the loaded byte value, palette,
or a direct TITLE.BIN-to-bitmap transform.
"""
from __future__ import annotations

import argparse
import re
from pathlib import Path


HEADER = "FIRESTAFF_NEXUS_VDP2_WRITER_REGISTER_TRACE_V1"
LINE = re.compile(
    r"^frame=12596 addr=0x(?P<address>[0-9a-fA-F]+) pc=0x0602312c"
    r"(?:\s+r[0-9]+=0x[0-9a-fA-F]+)*"
    r"\s+r4=0x(?P<r4>[0-9a-fA-F]+)"
    r"(?:\s+r[0-9]+=0x[0-9a-fA-F]+)*"
    r"\s+r5=0x(?P<r5>[0-9a-fA-F]+)\b"
)
SOURCE_START = 0x060AC2A7
DESTINATION_START = 0x25E01008
COPY_BYTES = 31616
ROW_BYTES = 0x130
ROW_STRIDE = 0x200


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
        if not lines or lines[0] != HEADER:
            raise ValueError("bad writer-register trace header")
        rows = [match for line in lines[1:] if (match := LINE.match(line))]
        if len(rows) != COPY_BYTES:
            raise ValueError(f"expected {COPY_BYTES} title copy rows, got {len(rows)}")
        for index, match in enumerate(rows):
            address = int(match["address"], 16)
            r4 = int(match["r4"], 16)
            r5 = int(match["r5"], 16)
            row, column = divmod(index, ROW_BYTES)
            expected_address = 0x1008 + row * ROW_STRIDE + column
            if address != expected_address:
                raise ValueError(f"VDP2 byte address discontinuity at row {index}")
            if r4 != DESTINATION_START + row * ROW_STRIDE + column:
                raise ValueError(f"destination pointer discontinuity at row {index}")
            # The VDP2 callback observes r5 after the retail MOV.B @r5+,...
            # sequence.  The source byte is therefore immediately before it.
            if r5 != SOURCE_START + index:
                raise ValueError(f"source pointer discontinuity at row {index}")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_NBG0_COPY_ROUTING_INVALID: {error}")
        return 1

    print("title_nbg0_copy_pc=0x0602312c")
    print("title_nbg0_copy_bytes=31616")
    print("title_nbg0_copy_source_post_range=0x060ac2a7-0x060b3e26")
    print("title_nbg0_copy_destination_range=0x25e01008-0x25e0df37")
    print("title_nbg0_copy_layout=104_rows_x_304_bytes_stride_512")
    print("title_nbg0_copy_pointer_route=verified")
    print("title_nbg0_copy_byte_value_transform=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
