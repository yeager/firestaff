#!/usr/bin/env python3
"""Verify one authenticated SH-2 source-buffer to VDP1-VRAM copy.

This is a transport receipt, not a renderer.  It requires the runtime
register witness, the source-buffer dump, and the same-session Saturn raw
capture.  The only normalization performed is Saturn's captured 16-bit
word byte order; no pixel, palette, command, or asset meaning is inferred.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions


REG_LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) pc=0x(?P<pc>[0-9a-fA-F]+)"
    r"(?P<regs>(?: r[0-9]+=0x[0-9a-fA-F]+)+)(?: peek_.*)?"
)
REG_VALUE = re.compile(r" r(?P<index>[0-9]+)=0x(?P<value>[0-9a-fA-F]+)")
SOURCE_HEAD = re.compile(
    r"FIRESTAFF_NEXUS_VDP1_SOURCE_DUMP_V1 addr=0x(?P<addr>[0-9a-fA-F]+)"
    r" size=0x(?P<size>[0-9a-fA-F]+)"
)


def read_register_witness(path: Path, target: int, required_pc: int | None) -> tuple[int, int, int]:
    for line in path.read_text(encoding="ascii").splitlines():
        match = REG_LINE.fullmatch(line)
        if not match or int(match["addr"], 16) != target:
            continue
        pc = int(match["pc"], 16)
        if required_pc is not None and pc != required_pc:
            continue
        values = {int(item["index"]): int(item["value"], 16)
                  for item in REG_VALUE.finditer(match["regs"])}
        if 0 not in values or 5 not in values:
            raise ValueError("target register witness lacks R0/R5")
        return pc, values[0], values[5]
    raise ValueError("required target register witness not found")


def read_source_dump(path: Path) -> tuple[int, bytes]:
    text = path.read_text(encoding="ascii").strip()
    if "\\n" in text:
        header, payload = text.split("\\n", 1)
        payload = payload.split("\\n", 1)[0]
    else:
        lines = text.splitlines()
        if len(lines) < 2:
            raise ValueError("source dump has no payload")
        header, payload = lines[0], lines[1]
    match = SOURCE_HEAD.fullmatch(header)
    if not match:
        raise ValueError("invalid source dump header")
    data = bytes.fromhex(payload)
    size = int(match["size"], 16)
    if len(data) != size:
        raise ValueError("source dump size does not match header")
    return int(match["addr"], 16), data


def saturn_vram_bytes(source_words: bytes) -> bytes:
    if len(source_words) & 1:
        raise ValueError("source dump is not word aligned")
    return b"".join(source_words[index + 1:index + 2] + source_words[index:index + 1]
                     for index in range(0, len(source_words), 2))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("registers", type=Path)
    parser.add_argument("source_dump", type=Path)
    parser.add_argument("--frame", type=int, required=True)
    parser.add_argument("--capture-frames", type=int, required=True)
    parser.add_argument("--target", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--require-pc", type=lambda value: int(value, 0))
    args = parser.parse_args()
    try:
        pc, source_address, source_size = read_register_witness(
            args.registers, args.target, args.require_pc)
        dump_address, source_words = read_source_dump(args.source_dump)
        if dump_address != source_address:
            raise ValueError("source dump address does not match R0")
        if len(source_words) != source_size:
            raise ValueError("source dump size does not match R5")
        if args.frame >= args.capture_frames:
            raise ValueError("requested frame is outside capture")
        frames, _ = frame_regions(args.capture.read_bytes(), args.capture_frames)
        vram = frames[args.frame]["vdp1-vram"]
        expected = saturn_vram_bytes(source_words)
        actual = vram[args.target:args.target + len(expected)]
        if len(actual) != len(expected) or actual != expected:
            raise ValueError("source buffer does not match VDP1 VRAM target")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP1_SOURCE_TO_VRAM_INVALID: {error}")
        return 1
    print(f"writer_pc=0x{pc:08x} source_address=0x{source_address:08x} "
          f"source_bytes=0x{source_size:x} vram_target=0x{args.target:05x} "
          f"frame={args.frame}")
    print("saturn_word_byte_order=verified")
    print("source_to_vram_copy=verified")
    print("pixel_palette_command_semantics=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
