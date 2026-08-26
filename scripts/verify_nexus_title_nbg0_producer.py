#!/usr/bin/env python3
"""Verify the observed retail title NBG0 VDP2 producer window.

This is a hardware transport receipt only.  It proves the VDP2 byte-lane
writes and their two observed SH-2 PCs reproduce the captured title bitmap;
it does not identify the RAM/CD source of PC 0x0602312c or admit presentation.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file
from analyze_nexus_vdp2_write_trace import replay_vram_bus_writes, trace_records
from nexus_vdp2_registers import detect_byte_order, read_u16


NBG0_BYTES = 0x20000
TITLE_NBG0_SHA256 = "ad10d99f00c3eecdf9577b15af1a7b86870a4ba83299dc50a09881dc569ad5e8"
CLEAR_PC = 0x060230AC
COPY_PC = 0x0602312C


def word_swap(data: bytes) -> bytes:
    if len(data) % 2:
        raise ValueError("VDP2 bitmap byte count is not word-aligned")
    return b"".join(data[offset:offset + 2][::-1]
                    for offset in range(0, len(data), 2))


def capture_frame(capture: Path, capture_frames: int,
                  frame_index: int) -> dict[str, bytes]:
    """Return an explicitly selected frame from a validated raw witness."""
    if capture_frames <= 0 or frame_index < 0 or frame_index >= capture_frames:
        raise ValueError("title frame index is outside the capture window")
    for index, regions in iter_frame_regions_file(capture, capture_frames):
        if index == frame_index:
            return regions
    raise ValueError("selected title frame is absent from capture")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("writes", type=Path)
    parser.add_argument("--capture-frames", type=int, default=1,
                        help="validated frame count in CAPTURE (default: 1)")
    parser.add_argument("--frame", type=int, default=0,
                        help="zero-based title frame within CAPTURE (default: 0)")
    args = parser.parse_args()
    try:
        frame = capture_frame(args.capture, args.capture_frames, args.frame)
        registers = frame["vdp2-regs"]
        order = detect_byte_order(registers)
        if (read_u16(registers, 0x00, order) != 0x8000 or
                read_u16(registers, 0x20, order) != 0x0003 or
                read_u16(registers, 0x28, order) != 0x0013 or
                read_u16(registers, 0x2c, order) != 0):
            raise ValueError("capture is not the measured active title NBG0 mode")
        bitmap = frame["vdp2-vram"][:NBG0_BYTES]
        if hashlib.sha256(bitmap).hexdigest() != TITLE_NBG0_SHA256:
            raise ValueError("capture NBG0 hash differs from the JP title witness")
        records = trace_records(args.writes)
        pcs = {pc0 for area, address, _size, _value, pc0, _pc1 in records
               if area == "vram" and address < NBG0_BYTES}
        if pcs != {CLEAR_PC, COPY_PC}:
            raise ValueError("NBG0 trace PC set is not the bounded clear/copy pair")
        image, writes = replay_vram_bus_writes(args.writes)
        if writes != 67616:
            raise ValueError(f"expected 67616 VDP2 writes, got {writes}")
        # The raw runtime dump is host word-swapped relative to VDP2 bus
        # order. Never compare it directly and silently lose byte lanes.
        reconstructed = word_swap(image[:NBG0_BYTES])
        if reconstructed != bitmap:
            raise ValueError("word-swapped VDP2 write replay differs from title NBG0")
    except (OSError, StopIteration, ValueError) as error:
        print(f"NEXUS_TITLE_NBG0_PRODUCER_INVALID: {error}")
        return 1

    print("title_nbg0_mode=verified")
    print(f"title_nbg0_capture_frame={args.frame}")
    print("title_nbg0_vram_range=0x00000-0x1ffff")
    print(f"title_nbg0_sha256={TITLE_NBG0_SHA256}")
    print(f"title_nbg0_vdp2_writes={writes}")
    print(f"title_nbg0_clear_pc=0x{CLEAR_PC:08x}")
    print(f"title_nbg0_copy_pc=0x{COPY_PC:08x}")
    print("title_nbg0_vdp2_transport=verified")
    print("title_nbg0_sh2_ram_cd_source=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
