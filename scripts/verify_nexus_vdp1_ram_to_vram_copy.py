#!/usr/bin/env python3
"""Verify the observed retail Nexus SH-2 RAM to VDP1 copy transport.

This accepts only an authenticated, same-session raw capture, SH-2 memory
snapshot and VDP1 writer register receipt.  It proves bytes and byte order;
it deliberately does not infer a texture, command, palette, or title-screen
meaning from the copy.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file


HEADER = "FIRESTAFF_NEXUS_VDP1_WRITER_REGISTER_TRACE_V1"
ROW = re.compile(r"pc=0x(?P<pc>[0-9a-fA-F]+) vram_addr=0x(?P<addr>[0-9a-fA-F]+)"
                 r"(?P<registers>(?: r[0-9]+=0x[0-9a-fA-F]+)+)$")
REGISTER = re.compile(r" r(?P<index>[0-9]+)=0x(?P<value>[0-9a-fA-F]+)")
SNAPSHOT_HEADER = b"FIRESTAFF_NEXUS_SH2_MEMORY_SNAPSHOT_V1\n"
SNAPSHOT_ROW = re.compile(rb"frame=(?P<frame>[0-9]+) base=0x(?P<base>[0-9a-fA-F]+) size=(?P<size>[0-9]+)\n")


def registers(path: Path) -> tuple[int, int, dict[int, int]]:
    lines = path.read_text(encoding="ascii").splitlines()
    if len(lines) != 2 or lines[0] != HEADER:
        raise ValueError("invalid VDP1 writer register receipt")
    match = ROW.fullmatch(lines[1])
    if not match:
        raise ValueError("malformed VDP1 writer register receipt")
    values = {int(item["index"]): int(item["value"], 16)
              for item in REGISTER.finditer(match["registers"])}
    if not all(index in values for index in (0, 6)):
        raise ValueError("register receipt lacks R0 or R6")
    return int(match["pc"], 16), int(match["addr"], 16), values


def ram_at(path: Path, wanted_frame: int) -> tuple[int, bytes]:
    stream = path.read_bytes()
    if not stream.startswith(SNAPSHOT_HEADER):
        raise ValueError("invalid SH-2 snapshot header")
    cursor = len(SNAPSHOT_HEADER)
    while cursor < len(stream):
        line_end = stream.find(b"\n", cursor)
        if line_end < 0:
            break
        match = SNAPSHOT_ROW.fullmatch(stream[cursor:line_end + 1])
        if not match:
            raise ValueError("malformed SH-2 snapshot frame header")
        frame = int(match["frame"])
        base = int(match["base"], 16)
        size = int(match["size"])
        cursor = line_end + 1
        payload = stream[cursor:cursor + size]
        if len(payload) != size or cursor + size >= len(stream) or stream[cursor + size] != 10:
            raise ValueError("truncated SH-2 snapshot payload")
        if frame == wanted_frame:
            return base, payload
        cursor += size + 1
    raise ValueError("requested SH-2 snapshot frame is absent")


def swap_words(data: bytes) -> bytes:
    if len(data) & 1:
        raise ValueError("copy size is not 16-bit aligned")
    return b"".join(data[index:index + 2][::-1] for index in range(0, len(data), 2))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("registers", type=Path)
    parser.add_argument("snapshot", type=Path)
    parser.add_argument("--frame", type=int, required=True)
    parser.add_argument("--capture-frames", type=int, required=True)
    parser.add_argument("--capture-frame", type=int,
                        help="zero-based raw-capture frame (defaults to --frame)")
    parser.add_argument("--require-pc", type=lambda value: int(value, 0))
    args = parser.parse_args()
    try:
        pc, target_end, values = registers(args.registers)
        if args.require_pc is not None and pc != args.require_pc:
            raise ValueError("writer PC differs from the required retail PC")
        size = values[6]
        if size == 0 or size > 0x100000:
            raise ValueError("R6 is not a bounded copy size")
        # At the observed backwards byte-copy loop PC, R0 and the VDP address
        # name the final source and destination byte respectively.
        source_end = values[0]
        source_start = source_end - size + 1
        target_start = target_end - size + 1
        base, ram = ram_at(args.snapshot, args.frame)
        offset = source_start - base
        if offset < 0 or offset + size > len(ram):
            raise ValueError("source span lies outside the SH-2 snapshot")
        expected = swap_words(ram[offset:offset + size])
        capture_frame = args.frame if args.capture_frame is None else args.capture_frame
        selected = None
        for frame, regions in iter_frame_regions_file(args.capture, args.capture_frames):
            if frame == capture_frame:
                selected = regions["vdp1-vram"]
        if selected is None or target_start < 0 or target_start + size > len(selected):
            raise ValueError("destination span lies outside captured VDP1 VRAM")
        if expected != selected[target_start:target_start + size]:
            raise ValueError("SH-2 source bytes do not match captured VDP1 VRAM")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP1_RAM_TO_VRAM_INVALID: {error}")
        return 1
    print(f"writer_pc=0x{pc:08x} source=0x{source_start:08x}:0x{source_end + 1:08x} "
          f"vdp1=0x{target_start:05x}:0x{target_end + 1:05x} bytes=0x{size:x}")
    print("saturn_word_byte_order=verified")
    print("sh2_ram_to_vdp1_transport=verified")
    print("pixel_palette_command_title_semantics=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
