#!/usr/bin/env python3
"""Compare an authentic NBG1 bitmap span with decoded retail source pixels.

The VDP2 register receipt identifies the active bitmap geometry, while this
tool performs only bounded byte-domain joins against authenticated retail
`MENU.BPK` PRS3 output and `FONT256.S2D` character-generator bytes. A partial
or absent match is evidence, never permission to render or assign ownership.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions


ASSET_HASHES = {
    "MENU.BPK": "f2f78dddfe37a5ff414775ae888f164624e987059934b034ba36299cc769d2ca",
    "FONT256.S2D": "764a2d6ce11b463817f5c1f2dfefbf55ff9221a1362cb5e4366998100d8ff3bb",
}


def read_asset(data_dir: Path, name: str) -> bytes:
    data = (data_dir / name).read_bytes()
    if hashlib.sha256(data).hexdigest() != ASSET_HASHES[name]:
        raise ValueError(f"{name} hash mismatch")
    return data


def decode_prs3(stream: bytes, target: int) -> bytes | None:
    output = bytearray()
    cursor = 0
    while len(output) < target and cursor < len(stream):
        control = stream[cursor]
        cursor += 1
        for bit in range(8):
            if len(output) >= target:
                break
            if control & (1 << bit):
                if cursor >= len(stream):
                    return None
                output.append(stream[cursor])
                cursor += 1
                continue
            if cursor + 1 >= len(stream):
                return None
            b0, b1 = stream[cursor], stream[cursor + 1]
            cursor += 2
            length = 3 + (b1 & 0x0F)
            raw_offset = ((b1 >> 4) << 8) | b0
            offset = raw_offset - 0xFEE if raw_offset >= 0xFDC else raw_offset + 18
            while len(output) - offset > 4095:
                offset += 4096
            for _ in range(length):
                if len(output) >= target:
                    break
                if offset < 0:
                    output.append(0)
                elif offset >= len(output):
                    return None
                else:
                    output.append(output[offset])
                offset += 1
    return bytes(output) if len(output) == target else None


def menu_surfaces(data: bytes) -> list[tuple[str, bytes]]:
    count = int.from_bytes(data[20:24], "big")
    offsets = [int.from_bytes(data[24 + i * 4:28 + i * 4], "big") for i in range(count)]
    trailer = len(data) - 524
    if trailer < 0 or data[trailer:trailer + 4] != b"PALT":
        raise ValueError("MENU.BPK PALT trailer missing")
    surfaces: list[tuple[str, bytes]] = []
    for index, start in enumerate(offsets):
        end = offsets[index + 1] if index + 1 < count else trailer
        if end <= start or data[start + 20:start + 24] != b"PRS3":
            continue
        expected = int.from_bytes(data[start + 28:start + 32], "big")
        compressed_size = int.from_bytes(data[start + 32:start + 36], "big")
        body_start = start + 36
        body_end = body_start + compressed_size
        if body_end > len(data):
            raise ValueError(f"MENU.BPK PRS3 entry {index} exceeds archive")
        pixels = decode_prs3(data[body_start:body_end], expected)
        if pixels is None:
            raise ValueError(f"MENU.BPK PRS3 entry {index} failed decode")
        surfaces.append((f"MENU.BPK[{index}]", pixels))
    return surfaces


def font_tiles(data: bytes) -> list[tuple[str, bytes]]:
    offset = int.from_bytes(data[0x20 + 2 * 8:0x24 + 2 * 8], "big")
    size = int.from_bytes(data[0x24 + 2 * 8:0x28 + 2 * 8], "big")
    payload = data[offset + 16:offset + size]
    if len(payload) % 64 != 0:
        raise ValueError("FONT256 character-generator region is not tile-aligned")
    return [(f"FONT256.S2D[tile={i}]", payload[i * 64:(i + 1) * 64])
            for i in range(len(payload) // 64)]


def longest_nonzero_match(source: bytes, target: bytes) -> tuple[int, int, int]:
    best = (0, 0, 0)
    if len(source) < 32:
        return best
    for source_offset in range(0, len(source) - 31, 16):
        chunk = source[source_offset:source_offset + 32]
        if sum(value != 0 for value in chunk) < 8:
            continue
        target_offset = target.find(chunk)
        if target_offset < 0:
            continue
        length = 32
        while (source_offset + length < len(source) and
               target_offset + length < len(target) and
               source[source_offset + length] == target[target_offset + length]):
            length += 1
        if length > best[0]:
            best = (length, source_offset, target_offset)
    return best


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=0)
    args = parser.parse_args()
    if args.frame < 0:
        print("NEXUS_VDP2_BITMAP_SOURCE_INVALID: negative frame")
        return 1
    try:
        frames, _ = frame_regions(args.capture.read_bytes(), args.frame + 1)
        menu = read_asset(args.data_dir, "MENU.BPK")
        font = read_asset(args.data_dir, "FONT256.S2D")
        sources = menu_surfaces(menu) + font_tiles(font)
    except (OSError, ValueError) as error:
        print(f"NEXUS_VDP2_BITMAP_SOURCE_INVALID: {error}")
        return 1

    frame = frames[args.frame]
    registers = frame["vdp2-regs"]
    bgon = int.from_bytes(registers[0x20:0x22], "big")
    chctla = int.from_bytes(registers[0x28:0x2A], "big")
    bmpna = int.from_bytes(registers[0x2C:0x2E], "big")
    if not (bgon & 0x02) or not (chctla & 0x0200):
        print("NEXUS_VDP2_BITMAP_SOURCE_INVALID: NBG1 bitmap mode is not active")
        return 1
    bitmap_size = 512 * 256  # BMSize code 0, 8bpp; guarded by the receipt below.
    if ((chctla >> 10) & 3) != 0 or ((chctla >> 12) & 3) != 1:
        print("NEXUS_VDP2_BITMAP_SOURCE_INVALID: unsupported NBG1 geometry")
        return 1
    map_offset = (int.from_bytes(registers[0x3C:0x3E], "big") >> 4) & 7
    byte_offset = map_offset * 0x20000
    bitmap = frame["vdp2-vram"][byte_offset:byte_offset + bitmap_size]
    if len(bitmap) != bitmap_size:
        print("NEXUS_VDP2_BITMAP_SOURCE_INVALID: bitmap span outside VRAM")
        return 1

    print(f"frame={args.frame} NBG1_bitmap_offset=0x{byte_offset:06x} bytes={len(bitmap)}")
    print(f"NBG1_bmpna=0x{bmpna:04x} colour_code=1 bitmap_size_code=0")
    exact = 0
    ranked: list[tuple[int, str, int, int]] = []
    for name, source in sources:
        full = bitmap.find(source) if source and any(source) else -1
        if full >= 0:
            exact += 1
            print(f"exact_match={name} bitmap_offset=0x{full:06x} bytes={len(source)}")
        run, source_offset, target_offset = longest_nonzero_match(source, bitmap)
        if run:
            ranked.append((run, name, source_offset, target_offset))
    for run, name, source_offset, target_offset in sorted(ranked, reverse=True)[:8]:
        print(f"partial_match={name} bytes={run} source_offset=0x{source_offset:x} bitmap_offset=0x{target_offset:x}")
    print(f"decoded_sources={len(sources)}")
    print(f"exact_source_matches={exact}")
    print("source_join=verified" if exact else "source_join=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
