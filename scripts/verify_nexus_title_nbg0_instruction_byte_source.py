#!/usr/bin/env python3
"""Bind the title NBG0 byte copier to its observed SH-2 load values.

This verifier consumes one same-session instruction-byte trace and the VDP2
write trace already used to prove the title-copy route.  It is deliberately
fail-closed: it accepts neither a partial capture nor a reordered set of
matching bytes.  Passing it proves only the observed WorkRAM-byte to VDP2-byte
transport; it does not identify the earlier CD loader or the VDP2 display
consumer.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

from analyze_nexus_vdp2_write_trace import trace_records
from verify_nexus_title_nbg0_copy_routing import (
    COPY_BYTES,
    DESTINATION_START,
    ROW_BYTES,
    ROW_STRIDE,
    SOURCE_START,
)


HEADER = "FIRESTAFF_NEXUS_SH2_INSTRUCTION_BYTE_READ_TRACE_V1"
LINE = re.compile(
    r"^cpu=(?P<cpu>[0-9]+) addr=0x(?P<address>[0-9a-fA-F]+) "
    r"value=0x(?P<value>[0-9a-fA-F]{2}) pc=0x(?P<pc>[0-9a-fA-F]+) "
    r"r=(?P<register>[0-9]+)$"
)
COPY_PC = 0x0602312C


def vdp2_byte(address: int, value: int) -> int:
    """Return the byte accepted by VDP2's addressed byte lane."""
    return (value >> 8) & 0xFF if (address & 1) == 0 else value & 0xFF


def instruction_rows(path: Path, expected_pc: int,
                     expected_register: int) -> list[tuple[int, int, int]]:
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != HEADER:
        raise ValueError("bad instruction-byte trace header")
    rows: list[tuple[int, int, int]] = []
    for line_number, line in enumerate(lines[1:], 2):
        match = LINE.fullmatch(line)
        if not match:
            raise ValueError(f"malformed instruction-byte line {line_number}")
        if (int(match["pc"], 16) == expected_pc and
                int(match["register"], 10) == expected_register):
            rows.append((int(match["cpu"], 10), int(match["address"], 16),
                         int(match["value"], 16)))
    return rows


def title_copy_writes(path: Path) -> list[tuple[int, int]]:
    rows: list[tuple[int, int]] = []
    for area, address, size, value, pc0, _pc1 in trace_records(path):
        if area != "vram" or pc0 != COPY_PC:
            continue
        if size != 1:
            raise ValueError("title copy contains a non-byte VDP2 write")
        rows.append((address, vdp2_byte(address, value)))
    if len(rows) != COPY_BYTES:
        raise ValueError(f"expected {COPY_BYTES} title VDP2 bytes, got {len(rows)}")
    for index, (address, _value) in enumerate(rows):
        row, column = divmod(index, ROW_BYTES)
        expected = (DESTINATION_START + row * ROW_STRIDE + column) & 0x7FFFF
        if (address & 0x7FFFF) != expected:
            raise ValueError(f"VDP2 destination discontinuity at byte {index}")
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("instruction_bytes", type=Path)
    parser.add_argument("vdp2_writes", type=Path)
    parser.add_argument("--instruction-pc", type=lambda value: int(value, 0),
                        required=True)
    parser.add_argument("--instruction-register", type=int, required=True)
    args = parser.parse_args()
    try:
        reads = instruction_rows(args.instruction_bytes, args.instruction_pc,
                                 args.instruction_register)
        writes = title_copy_writes(args.vdp2_writes)
        if len(reads) != COPY_BYTES:
            raise ValueError(f"expected {COPY_BYTES} filtered instruction reads, got {len(reads)}")
        cpu = reads[0][0]
        for index, ((read_cpu, address, value), (_destination, output)) in enumerate(
                zip(reads, writes)):
            # The VDP2 writer observes r5 after MOV.B @r5+, so the byte-load
            # which fed its output is exactly one address before that
            # post-increment corridor.
            expected_address = SOURCE_START - 1 + index
            if read_cpu != cpu:
                raise ValueError("instruction reads cross SH-2 CPUs")
            if address != expected_address:
                raise ValueError(f"SH-2 source discontinuity at byte {index}")
            if value != output:
                raise ValueError(f"SH-2/VDP2 byte mismatch at byte {index}")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_NBG0_INSTRUCTION_SOURCE_INVALID: {error}")
        return 1

    print(f"title_nbg0_instruction_cpu={cpu}")
    print(f"title_nbg0_instruction_pc=0x{args.instruction_pc:08x}")
    print(f"title_nbg0_instruction_register={args.instruction_register}")
    print("title_nbg0_instruction_source_range=0x060ac2a6-0x060b3e25")
    print(f"title_nbg0_instruction_source_bytes={COPY_BYTES}")
    print("title_nbg0_instruction_to_vdp2_values=verified")
    print("title_nbg0_cd_to_ram_source=unbound")
    print("title_nbg0_display_consumer=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
