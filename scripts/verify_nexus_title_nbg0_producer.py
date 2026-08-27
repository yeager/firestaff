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
TITLE_NBG0_BASELINE_SHA256 = "fa43239bcee7b97ca62f007cc68487560a39e19f74f3dde7486db3f98df8e471"
CLEAR_PC = 0x060230AC
COPY_PC = 0x0602312C
CLEAR_WRITES = 32850
COPY_WRITES = 31616
TOTAL_WRITES = CLEAR_WRITES + COPY_WRITES


def word_swap(data: bytes) -> bytes:
    if len(data) % 2:
        raise ValueError("VDP2 bitmap byte count is not word-aligned")
    return b"".join(data[offset:offset + 2][::-1]
                    for offset in range(0, len(data), 2))


def replay_vram_records(initial_bus_order: bytes,
                        records: list[tuple[str, int, int, int, int, int]]) -> tuple[bytes, int]:
    """Apply a bounded VDP2 trace over an observed bus-order baseline.

    This is intentionally a bus-level receipt, not a title renderer.  The
    baseline is always a separately captured VDP2 state; callers must verify
    its identity before using the resulting image.
    """
    if len(initial_bus_order) != 0x80000:
        raise ValueError("VDP2 baseline has an invalid size")
    image = bytearray(initial_bus_order)
    writes = 0
    for area, address, size, value, _pc0, _pc1 in records:
        if area != "vram":
            continue
        offset = address & 0x7ffff
        if size == 1:
            image[offset] = ((value >> 8) & 0xff
                             if (address & 1) == 0 else value & 0xff)
        elif size == 2:
            if offset + 2 > len(image):
                raise ValueError("word write crosses VRAM boundary")
            image[offset:offset + 2] = value.to_bytes(2, "big")
        else:
            raise ValueError("four-byte VDP2 write cannot be replayed safely")
        writes += 1
    return bytes(image), writes


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
    parser.add_argument("--baseline-frame", type=int,
                        help="observed pre-copy frame in the same CAPTURE; enables the bounded copy-delta receipt")
    args = parser.parse_args()
    try:
        frame = capture_frame(args.capture, args.capture_frames, args.frame)
        baseline = None
        if args.baseline_frame is not None:
            if args.baseline_frame < 0 or args.baseline_frame >= args.frame:
                raise ValueError("baseline frame must precede the title frame")
            baseline = capture_frame(args.capture, args.capture_frames,
                                     args.baseline_frame)
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
        nbg0_records = [record for record in records
                         if record[0] == "vram" and record[1] < NBG0_BYTES]
        pcs = {pc0 for _area, _address, _size, _value, pc0, _pc1 in nbg0_records}
        clear_writes = sum(1 for area, address, _size, _value, pc0, _pc1 in records
                           if area == "vram" and address < NBG0_BYTES and
                           pc0 == CLEAR_PC)
        copy_writes = sum(1 for area, address, _size, _value, pc0, _pc1 in records
                          if area == "vram" and address < NBG0_BYTES and
                          pc0 == COPY_PC)
        if baseline is None:
            if pcs != {CLEAR_PC, COPY_PC}:
                raise ValueError("NBG0 trace PC set is not the bounded clear/copy pair")
            if clear_writes != CLEAR_WRITES or copy_writes != COPY_WRITES:
                raise ValueError(
                    f"expected {CLEAR_WRITES} clear and {COPY_WRITES} copy writes, got "
                    f"{clear_writes} and {copy_writes}")
            image, writes = replay_vram_bus_writes(args.writes)
            if writes != TOTAL_WRITES:
                raise ValueError(f"expected {TOTAL_WRITES} VDP2 writes, got {writes}")
        else:
            baseline_bitmap = baseline["vdp2-vram"][:NBG0_BYTES]
            if hashlib.sha256(baseline_bitmap).hexdigest() != TITLE_NBG0_BASELINE_SHA256:
                raise ValueError("baseline differs from the measured pre-title NBG0 state")
            if (pcs != {COPY_PC} or len(records) != COPY_WRITES or
                    len(nbg0_records) != COPY_WRITES or copy_writes != COPY_WRITES):
                raise ValueError("copy-delta trace is not the bounded title-copy sequence")
            image, writes = replay_vram_records(
                word_swap(baseline["vdp2-vram"]), records)
            if writes != COPY_WRITES:
                raise ValueError(f"expected {COPY_WRITES} copy writes, got {writes}")
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
    print(f"title_nbg0_clear_writes={CLEAR_WRITES}")
    print(f"title_nbg0_copy_writes={COPY_WRITES}")
    print(f"title_nbg0_clear_pc=0x{CLEAR_PC:08x}")
    print(f"title_nbg0_copy_pc=0x{COPY_PC:08x}")
    if args.baseline_frame is not None:
        print(f"title_nbg0_baseline_frame={args.baseline_frame}")
        print(f"title_nbg0_baseline_sha256={TITLE_NBG0_BASELINE_SHA256}")
        print("title_nbg0_copy_delta=verified")
    print("title_nbg0_vdp2_transport=verified")
    print("title_nbg0_sh2_ram_cd_source=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
