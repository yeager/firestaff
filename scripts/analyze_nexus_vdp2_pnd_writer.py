#!/usr/bin/env python3
"""Verify an authenticated VDP2 PND writer and its captured destination.

This is a register/transport receipt.  It does not identify FONT256, text
encoding, page-entry meaning, palette bank, or host presentation semantics.
"""

from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions


REG = re.compile(r"addr=0x([0-9a-f]+) pc=0x([0-9a-f]+)(.*)$")
REG_VALUE = re.compile(r" r([0-9]+)=0x([0-9a-f]+)")
WRITE = re.compile(
    r"area=vram addr=0x([0-9a-f]+) size=2 value=0x([0-9a-f]+) "
    r"pc0=0x([0-9a-f]+) pc1=0x([0-9a-f]+)"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("writes", type=Path)
    parser.add_argument("registers", type=Path)
    parser.add_argument("--frame", type=int, required=True)
    parser.add_argument("--capture-frames", type=int, required=True)
    parser.add_argument("--destination", type=lambda value: int(value, 0), default=0x10000)
    parser.add_argument("--writer-pc", type=lambda value: int(value, 0), default=0x0601184C)
    parser.add_argument("--min-writes", type=int, default=64)
    args = parser.parse_args()
    try:
        frames, _ = frame_regions(args.capture.read_bytes(), args.capture_frames)
        if args.frame >= len(frames):
            raise ValueError("frame index outside capture")
        register_witness = None
        for line in args.registers.read_text(encoding="ascii").splitlines():
            match = REG.fullmatch(line)
            if match and int(match.group(1), 16) == args.destination and \
                    int(match.group(2), 16) == args.writer_pc:
                register_witness = {int(index): int(value, 16)
                                    for index, value in REG_VALUE.findall(match.group(3))}
                break
        if register_witness is None:
            raise ValueError("writer register witness not found")
        rows = []
        for line in args.writes.read_text(encoding="ascii").splitlines():
            match = WRITE.fullmatch(line)
            if not match:
                continue
            address = int(match.group(1), 16)
            if address >= args.destination and int(match.group(3), 16) == args.writer_pc:
                rows.append((address, int(match.group(2), 16)))
        rows.sort()
        if len(rows) < args.min_writes:
            raise ValueError(f"only {len(rows)} matching VDP2 writes")
        vram = frames[args.frame]["vdp2-vram"]
        for address, value in rows[:args.min_writes]:
            if address + 2 > len(vram) or vram[address:address + 2] != value.to_bytes(2, "little"):
                raise ValueError(f"VDP2 write mismatch at 0x{address:05x}")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP2_PND_WRITER_INVALID: {error}")
        return 1
    end = rows[args.min_writes - 1][0] + 2
    print(f"writer_pc=0x{args.writer_pc:08x} destination=0x{args.destination:05x} "
          f"verified_writes={args.min_writes} destination_end=0x{end:05x}")
    print("register_source_pointer=" +
          (f"0x{register_witness[3]:08x}" if 3 in register_witness else "unobserved"))
    print("vdp2_pnd_transport=verified")
    print("font256_text_palette_owner=blocked")
    print("host_text_composition_permission=blocked")
    print("destination_sha256=" + hashlib.sha256(vram[args.destination:end]).hexdigest())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
