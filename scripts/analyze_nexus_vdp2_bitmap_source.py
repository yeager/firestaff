#!/usr/bin/env python3
"""Compare an authentic NBG1 bitmap span with decoded retail source pixels.

The VDP2 register receipt identifies the active bitmap geometry, while this
tool performs only bounded byte-domain joins against authenticated retail
`MENU.BPK` PRS3 output, `FONT256.S2D` character-generator bytes, and the five
DMWeb TITLE.BIN MAPD/TIBG maps expanded through TITLE.CG. A partial or absent
match is evidence, never permission to render or assign ownership.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions
from fixtures.nexus_v1_disc_file_hashes import DISC_HASH


ASSET_HASHES = {
    "MENU.BPK": "f2f78dddfe37a5ff414775ae888f164624e987059934b034ba36299cc769d2ca",
    "FONT256.S2D": "764a2d6ce11b463817f5c1f2dfefbf55ff9221a1362cb5e4366998100d8ff3bb",
    "TITLE.BIN": "a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44",
    "TITLE.CG": "fda4da4ca1f344c93a4ae8455dcd7d92bcae0510784e5e4fa40e2ffc9e4fb580",
    "STABG.BIN": "7b8e44ffd1249175da1c407993b983a26bc180204e63f9b69274014b336c6913",
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


def menu_surfaces(data: bytes) -> tuple[list[tuple[str, bytes]], bytes]:
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
    return surfaces, data[trailer + 12:trailer + 524]


def font_tiles(data: bytes) -> list[tuple[str, bytes]]:
    offset = int.from_bytes(data[0x20 + 2 * 8:0x24 + 2 * 8], "big")
    size = int.from_bytes(data[0x24 + 2 * 8:0x28 + 2 * 8], "big")
    payload = data[offset + 16:offset + size]
    if len(payload) % 64 != 0:
        raise ValueError("FONT256 character-generator region is not tile-aligned")
    return [(f"FONT256.S2D[tile={i}]", payload[i * 64:(i + 1) * 64])
            for i in range(len(payload) // 64)]


def title_maps(title_bin: bytes, title_cg: bytes) -> tuple[list[tuple[str, bytes]], bytes]:
    record = title_bin[0xE278:]
    if len(record) < 0x8C74 or record[:4] != b"MAPD" or record[8:12] != b"TIBG":
        raise ValueError("TITLE.BIN MAPD/TIBG record missing")
    if len(title_cg) % 32 != 0:
        raise ValueError("TITLE.CG tile payload is not 4bpp aligned")
    maps: list[tuple[str, bytes]] = []
    for map_index in range(5):
        base = 0x40 + map_index * 0x1C04
        if int.from_bytes(record[base:base + 2], "big") != 0x40:
            raise ValueError("TITLE.BIN map width is not 64 cells")
        if int.from_bytes(record[base + 2:base + 4], "big") != 0x1C:
            raise ValueError("TITLE.BIN map height is not 28 cells")
        pixels = bytearray(512 * 224)
        for cell in range(64 * 28):
            cell_offset = base + 4 + cell * 4
            tile = (int.from_bytes(record[cell_offset + 2:cell_offset + 4], "big") & 0x7FFF) - 4608
            tile_offset = tile * 32
            if tile < 0 or tile_offset + 32 > len(title_cg):
                raise ValueError(f"TITLE.CG tile {tile} outside corpus")
            for y in range(8):
                for x in range(4):
                    packed = title_cg[tile_offset + y * 4 + x]
                    pixel = (cell // 64 * 8 + y) * 512 + cell % 64 * 8 + x * 2
                    pixels[pixel] = packed >> 4
                    pixels[pixel + 1] = packed & 0x0F
        maps.append((f"TITLE.BIN[map={map_index}]", bytes(pixels)))
    palette_offset = 0x40 + 5 * 0x1C04
    return maps, record[palette_offset:palette_offset + 32]


def stabg_first_map(data: bytes) -> tuple[bytes, bytes]:
    if data[:4] != b"STMP" or int.from_bytes(data[4:8], "big") != len(data):
        raise ValueError("STABG.BIN STMP header invalid")
    part1 = int.from_bytes(data[8:12], "big")
    part1_size = int.from_bytes(data[12:16], "big")
    part2 = int.from_bytes(data[16:20], "big")
    part2_size = int.from_bytes(data[20:24], "big")
    part3 = int.from_bytes(data[24:28], "big")
    part3_size = int.from_bytes(data[28:32], "big")
    if (part1 < 0x20 or part2 != part1 + part1_size or part2_size != 512 or
            part3 != part2 + part2_size or part3 + part3_size != len(data) or
            part3_size != 791 * 64):
        raise ValueError("STABG.BIN STMP regions invalid")
    cursor = part1
    while cursor + 4 <= part2:
        offset = int.from_bytes(data[cursor:cursor + 4], "big")
        cursor += 4
        if offset == 0:
            break
    if cursor + 2 > part2:
        raise ValueError("STABG.BIN map directory unterminated")
    map_offset = int.from_bytes(data[part1:part1 + 4], "big")
    if map_offset < 48 or (map_offset - 48) & 1:
        raise ValueError("STABG.BIN first map offset invalid")
    word_addr = cursor + ((map_offset - 48) // 2) * 2
    width = int.from_bytes(data[word_addr:word_addr + 2], "big")
    height = int.from_bytes(data[word_addr + 2:word_addr + 4], "big")
    if (width, height) != (40, 21):
        raise ValueError("STABG.BIN first map dimensions invalid")
    pixels = bytearray(width * 8 * height * 8)
    for y in range(height):
        for x in range(width):
            cell_offset = word_addr + 4 + (y * width + x) * 2
            cell = int.from_bytes(data[cell_offset:cell_offset + 2], "big")
            if cell & 0x8000:
                raise ValueError("STABG.BIN vertical flip is not supported")
            tile = (cell // 2) & 0x07FF
            hflip = bool(cell & 0x4000)
            for tile_y in range(8):
                for tile_x in range(8):
                    source_x = 7 - tile_x if hflip else tile_x
                    dst = ((y * 8 + tile_y) * (width * 8) + x * 8 + tile_x)
                    pixels[dst] = data[part3 + tile * 64 + tile_y * 8 + source_x]
    return bytes(pixels), data[part2:part2 + 512]


def dgn_structure2_palettes(data_dir: Path) -> list[tuple[str, bytes]]:
    """Return only bounded nonzero 32-byte palette anchors from retail DGN."""
    palettes: list[tuple[str, bytes]] = []
    for level in range(16):
        name = f"LEV{level:02d}.DGN"
        data = (data_dir / name).read_bytes()
        expected = DISC_HASH.get(name)
        if expected is None or hashlib.sha256(data).hexdigest() != expected:
            raise ValueError(f"{name} hash mismatch")
        if len(data) < 0x1C:
            raise ValueError(f"{name} header is truncated")
        block = int.from_bytes(data[0x14:0x16], "big")
        blocks = int.from_bytes(data[0x16:0x18], "big")
        useful = int.from_bytes(data[0x18:0x1C], "big")
        start = block * 0x800
        capacity = blocks * 0x800
        if (block == 0 or blocks == 0 or start > len(data) or
                capacity > len(data) - start or useful > capacity):
            raise ValueError(f"{name} Structure2 envelope invalid")
        cursor = 0
        descriptor_index = 0
        while cursor + 20 <= useful:
            descriptor = data[start + cursor:start + cursor + 20]
            image_id = int.from_bytes(descriptor[0:2], "big")
            if image_id == 0xFFFF:
                break
            if image_id != descriptor_index:
                raise ValueError(f"{name} Structure2 ids are not sequential")
            palette = int.from_bytes(descriptor[16:20], "big")
            if palette:
                if palette < cursor + 20 or palette > useful or 32 > useful - palette:
                    raise ValueError(f"{name} Structure2 palette span invalid")
                palettes.append((f"{name}[Structure2={image_id}]", data[start + palette:start + palette + 32]))
            descriptor_index += 1
            cursor += 20
        else:
            raise ValueError(f"{name} Structure2 terminator missing")
    return palettes


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
    parser.add_argument(
        "--capture-frames",
        type=int,
        default=None,
        help="number of frames in the authenticated capture (defaults to frame + 1)",
    )
    args = parser.parse_args()
    if args.frame < 0:
        print("NEXUS_VDP2_BITMAP_SOURCE_INVALID: negative frame")
        return 1
    capture_frames = args.capture_frames if args.capture_frames is not None else args.frame + 1
    if capture_frames <= args.frame:
        print("NEXUS_VDP2_BITMAP_SOURCE_INVALID: capture frame count does not include selected frame")
        return 1
    try:
        frames, _ = frame_regions(args.capture.read_bytes(), capture_frames)
        menu = read_asset(args.data_dir, "MENU.BPK")
        font = read_asset(args.data_dir, "FONT256.S2D")
        title_bin = read_asset(args.data_dir, "TITLE.BIN")
        title_cg = read_asset(args.data_dir, "TITLE.CG")
        title, title_palette = title_maps(title_bin, title_cg)
        stabg = read_asset(args.data_dir, "STABG.BIN")
        stabg_pixels, stabg_palette = stabg_first_map(stabg)
        menu_sources, menu_palette = menu_surfaces(menu)
        dgn_palettes = dgn_structure2_palettes(args.data_dir)
        sources = menu_sources + font_tiles(font) + title
        sources.append(("STABG.BIN[map=0]", stabg_pixels))
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
    cram = frame["vdp2-cram"]
    palette_swap = b"".join(title_palette[offset:offset + 2][::-1]
                             for offset in range(0, len(title_palette), 2))
    print(f"title_palette_cram_match={cram.find(title_palette)}")
    print(f"title_palette_cram_word_swap_match={cram.find(palette_swap)}")
    menu_palette_swap = b"".join(menu_palette[offset:offset + 2][::-1]
                                  for offset in range(0, len(menu_palette), 2))
    print(f"menu_palette_cram_match={cram.find(menu_palette)}")
    print(f"menu_palette_cram_word_swap_match={cram.find(menu_palette_swap)}")
    stabg_palette_swap = b"".join(stabg_palette[offset:offset + 2][::-1]
                                   for offset in range(0, len(stabg_palette), 2))
    print(f"stabg_palette_cram_match={cram.find(stabg_palette)}")
    print(f"stabg_palette_cram_word_swap_match={cram.find(stabg_palette_swap)}")
    dgn_exact = [(name, cram.find(palette)) for name, palette in dgn_palettes
                 if cram.find(palette) >= 0]
    dgn_word_swap_exact = []
    for name, palette in dgn_palettes:
        swapped = b"".join(palette[offset:offset + 2][::-1]
                            for offset in range(0, len(palette), 2))
        offset = cram.find(swapped)
        if offset >= 0:
            dgn_word_swap_exact.append((name, offset))
    print("dgn_structure2_palette_cram_matches=" +
          ("|".join(f"{name}@0x{offset:x}" for name, offset in dgn_exact)
           if dgn_exact else "none"))
    print("dgn_structure2_palette_cram_word_swap_matches=" +
          ("|".join(f"{name}@0x{offset:x}" for name, offset in dgn_word_swap_exact)
           if dgn_word_swap_exact else "none"))
    print(f"dgn_structure2_palettes={len(dgn_palettes)}")
    print(f"decoded_sources={len(sources)}")
    print(f"exact_source_matches={exact}")
    print("source_join=verified" if exact else "source_join=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
