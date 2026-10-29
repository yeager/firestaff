#!/usr/bin/env python3
"""Compare authentic NBG0/NBG1 bitmap spans with decoded retail source pixels.

The VDP2 register receipt identifies the active bitmap geometry, while this
tool performs only bounded byte-domain joins against authenticated retail
`MENU.BPK` PRS3 output, `FONT256.S2D` character-generator bytes, and the five
DMWeb TITLE.BIN MAPD/TIBG maps expanded through TITLE.CG. For NBG0 it also
reports a sampled raw TITLE.BIN span and any exact copy in captured WRAM as an
unproven producer candidate. A partial or absent match is evidence, never
permission to render or assign ownership.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions
from analyze_nexus_title_vdp2_source import cue_track1, iso_members_in_memory
from fixtures.nexus_v1_disc_file_hashes import DISC_HASH
from nexus_vdp2_registers import detect_byte_order, read_u16


ASSET_HASHES = {
    "FONT256.S2D": "764a2d6ce11b463817f5c1f2dfefbf55ff9221a1362cb5e4366998100d8ff3bb",
    "TITLE.BIN": "a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44",
    "TITLE.CG": "fda4da4ca1f344c93a4ae8455dcd7d92bcae0510784e5e4fa40e2ffc9e4fb580",
    "STABG.BIN": "7b8e44ffd1249175da1c407993b983a26bc180204e63f9b69274014b336c6913",
}

# DMWeb/verified manifest admits both European retail menu revisions.  The
# English and French archives have different MENU.BPK sizes and hashes; a
# capture must be compared with the revision it actually booted.
MENU_BPK_SHA256 = frozenset({
    "f2f78dddfe37a5ff414775ae888f164624e987059934b034ba36299cc769d2ca",
    "c4e2427f54083e92cdf38f3b1f296e135bdb007de227431be690cc41381fd543",
})


def verify_asset(data: bytes, name: str) -> bytes:
    digest = hashlib.sha256(data).hexdigest()
    if name == "MENU.BPK":
        accepted = set(MENU_BPK_SHA256)
        if name in DISC_HASH:
            accepted.add(DISC_HASH[name])
    elif name in ASSET_HASHES:
        accepted = {ASSET_HASHES[name]}
        if name in DISC_HASH:
            accepted.add(DISC_HASH[name])
    elif name in DISC_HASH:
        accepted = {DISC_HASH[name]}
    else:
        raise ValueError(f"unexpected source asset: {name}")
    if digest not in accepted:
        raise ValueError(f"{name} hash mismatch")
    return data


def read_assets(data_dir: Path | None, cue: Path | None,
                names: set[str]) -> dict[str, bytes]:
    """Read admitted retail members without materialising CUE entries."""
    if cue is not None:
        members = iso_members_in_memory(cue_track1(cue), names)
    elif data_dir is not None:
        members = {name: (data_dir / name).read_bytes() for name in names
                   if (data_dir / name).is_file()}
    else:
        raise ValueError("either --data-dir or --cue is required")
    return {name: verify_asset(data, name) for name, data in members.items()}


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


def dgn_structure2_palettes(assets: dict[str, bytes]) -> list[tuple[str, bytes]]:
    """Return only bounded nonzero 32-byte palette anchors from retail DGN."""
    palettes: list[tuple[str, bytes]] = []
    for level in range(16):
        name = f"LEV{level:02d}.DGN"
        data = assets[name]
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


def ymir_bundle_frame(bundle: Path) -> tuple[dict[str, bytes], int, str]:
    """Read one complete Ymir VDP2 dump and bind it to its captured frame."""
    def receipt(path: Path, expected_format: str) -> dict[str, str]:
        values: dict[str, str] = {}
        for line in path.read_text(encoding="ascii").splitlines():
            if not line or "=" not in line:
                continue
            key, value = line.split("=", 1)
            if key in values:
                raise ValueError(f"duplicate {key} in {path.name}")
            values[key] = value
        if values.get("format") != expected_format:
            raise ValueError(f"unsupported {path.name} format")
        return values

    if (bundle / "vdp2-memory-write-bundle.txt").is_file():
        capture = receipt(bundle / "vdp2-memory-write-bundle.txt",
                          "ymir-vdp2-memory-write-bundle-v1")
        # Ymir's VDP::DumpVDP2Registers() writes high byte before low byte.
        register_byte_order = "big"
        try:
            frame = int(capture["current_vdp1_frame_index"])
            expected_rows = int(capture["trace_rows"])
            dropped = int(capture["trace_dropped_total"])
            first_frame = int(capture["first_vdp1_frame_index"])
            last_frame = int(capture["last_vdp1_frame_index"])
        except (KeyError, ValueError) as error:
            raise ValueError("Ymir VDP2 memory receipt is incomplete or malformed") from error
        if (frame < 0 or first_frame < 0 or first_frame > last_frame or
                last_frame > frame or expected_rows < 1 or dropped != 0 or
                capture.get("bundle_complete") != "1" or
                capture.get("trace_file_complete") != "1" or
                capture.get("vdp2_vram_file_complete") != "1" or
                capture.get("vdp2_cram_file_complete") != "1"):
            raise ValueError("Ymir VDP2 receipt does not identify one complete frame snapshot")
        try:
            with (bundle / "vdp2-memory-write-trace.csv").open(
                    encoding="ascii", newline="") as stream:
                rows = list(csv.DictReader(stream))
        except (OSError, UnicodeError) as error:
            raise ValueError("Ymir VDP2 write trace is unreadable") from error
        if len(rows) != expected_rows or not rows:
            raise ValueError("Ymir VDP2 write trace row count does not match its receipt")
        if any(row.get("vdp1_frame") == "unknown" for row in rows):
            raise ValueError("Ymir VDP2 trace contains writes without frame identity")
        try:
            frame_indexes = [int(row["vdp1_frame"]) for row in rows]
            if (min(frame_indexes) != first_frame or
                    max(frame_indexes) != last_frame or last_frame > frame):
                raise ValueError("Ymir VDP2 trace frame range does not match its receipt")
            for row in rows:
                space = row["memory"]
                address = int(row["address"], 16)
                width = int(row["width"])
                value = int(row["value"], 16)
                limit = (512 * 1024 if space == "VRAM" else
                         4096 if space == "CRAM" else 0)
                if (limit == 0 or width not in (1, 2) or address < 0 or
                        address + width > limit or value < 0 or
                        value >= 1 << (width * 8)):
                    raise ValueError("Ymir VDP2 trace contains an invalid memory write")
        except (KeyError, TypeError, ValueError) as error:
            raise ValueError("Ymir VDP2 trace does not match its frame receipt") from error
    else:
        capture = receipt(bundle / "vdp1-capture-bundle.txt",
                          "ymir-vdp1-capture-bundle-v1")
        snapshot = receipt(bundle / "vdp-register-snapshot.txt",
                           "ymir-vdp-register-snapshot-v1")
        try:
            frame = int(capture["capture_frame_index"])
            snapshot_frame = int(snapshot["vdp1_frame_index"])
            register_bytes = int(snapshot["vdp2_register_bytes"])
            rows_for_frame = int(capture["trace_rows_for_capture_frame"])
        except (KeyError, ValueError) as error:
            raise ValueError("Ymir frame receipt is incomplete or malformed") from error
        if (frame < 0 or snapshot_frame != frame or
                capture.get("trace_file_complete") != "1" or
                rows_for_frame < 1 or register_bytes != 512 or
                snapshot.get("register_byte_order") != "big_endian"):
            raise ValueError("Ymir bundle receipts do not identify one complete VDP frame")
        register_byte_order = "big"
    expected = {
        "vdp2-regs": ("vdp2-registers.bin", 512),
        "vdp2-vram": ("vdp2-vram.bin", 512 * 1024),
        "vdp2-cram": ("vdp2-cram.bin", 4096),
    }
    regions: dict[str, bytes] = {}
    for name, (filename, size) in expected.items():
        data = (bundle / filename).read_bytes()
        if len(data) != size:
            raise ValueError(f"{filename} has unexpected size")
        regions[name] = data
    return regions, frame, register_byte_order


def longest_nonzero_match(source: bytes, target: bytes) -> tuple[int, int, int]:
    """Return the longest sampled exact source span, considering every target hit."""
    best = (0, 0, 0)
    if len(source) < 32:
        return best
    for source_offset in range(0, len(source) - 31, 16):
        chunk = source[source_offset:source_offset + 32]
        if sum(value != 0 for value in chunk) < 8:
            continue
        search_from = 0
        while True:
            target_offset = target.find(chunk, search_from)
            if target_offset < 0:
                break
            left = 0
            while (source_offset - left - 1 >= 0 and
                   target_offset - left - 1 >= 0 and
                   source[source_offset - left - 1] ==
                   target[target_offset - left - 1]):
                left += 1
            length = 32
            while (source_offset + length < len(source) and
                   target_offset + length < len(target) and
                   source[source_offset + length] ==
                   target[target_offset + length]):
                length += 1
            total = left + length
            if total > best[0]:
                best = (total, source_offset - left, target_offset - left)
            search_from = target_offset + 1
    return best


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path,
                        help="authenticated raw capture file or Ymir memory-bundle directory")
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--data-dir", type=Path)
    source.add_argument("--cue", type=Path,
                        help="retail CUE; ISO members are retained in memory")
    parser.add_argument("--frame", type=int, default=0)
    parser.add_argument(
        "--exact-only",
        action="store_true",
        help="skip sampled partial-span ranking and report only exact source matches",
    )
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
    bundle_byte_order = None
    try:
        if args.capture.is_dir():
            if args.frame != 0:
                raise ValueError("--frame applies only to multi-frame raw captures")
            frame_data, frame_index, bundle_byte_order = ymir_bundle_frame(args.capture)
            frames = [frame_data]
        else:
            frames, _ = frame_regions(args.capture.read_bytes(), capture_frames)
            frame_index = args.frame
        required = {"MENU.BPK", "FONT256.S2D"} | {
            f"LEV{level:02d}.DGN" for level in range(16)
        }
        optional_names = {"TITLE.BIN", "TITLE.CG", "STABG.BIN"}
        assets = read_assets(
            args.data_dir, args.cue,
            required | optional_names)
        missing = sorted(required - set(assets))
        if missing:
            raise ValueError("required asset missing: " + ",".join(missing))
        menu = assets["MENU.BPK"]
        font = assets["FONT256.S2D"]
        title_bin = assets.get("TITLE.BIN")
        title_cg = assets.get("TITLE.CG")
        title = []
        title_palette = b""
        if title_bin is not None and title_cg is not None:
            title, title_palette = title_maps(title_bin, title_cg)
        stabg = assets.get("STABG.BIN")
        stabg_pixels = b""
        stabg_palette = b""
        if stabg is not None:
            stabg_pixels, stabg_palette = stabg_first_map(stabg)
        menu_sources, menu_palette = menu_surfaces(menu)
        dgn_palettes = dgn_structure2_palettes(assets)
        sources = menu_sources + font_tiles(font) + title
        if stabg_pixels:
            sources.append(("STABG.BIN[map=0]", stabg_pixels))
    except (OSError, ValueError) as error:
        print(f"NEXUS_VDP2_BITMAP_SOURCE_INVALID: {error}")
        return 1

    frame = frames[args.frame]
    registers = frame["vdp2-regs"]
    byte_order = bundle_byte_order or detect_byte_order(registers)
    bgon = read_u16(registers, 0x20, byte_order)
    chctla = read_u16(registers, 0x28, byte_order)
    bmpna = read_u16(registers, 0x2C, byte_order)
    mpo = read_u16(registers, 0x3C, byte_order)
    layers: list[tuple[str, int, int]] = []
    # NBG0 and NBG1 each use their own register fields in CHCTLA/MPOFN.
    # This comparator admits only the captured 512x256, 8bpp bitmap mode.
    if bgon & 0x01 and chctla & 0x0002:
        if (chctla & 0x000C) != 0 or (chctla & 0x0070) != 0x0010:
            print("NEXUS_VDP2_BITMAP_SOURCE_INVALID: unsupported NBG0 geometry")
            return 1
        layers.append(("NBG0", mpo & 7, bmpna & 7))
    if bgon & 0x02 and chctla & 0x0200:
        if ((chctla >> 10) & 3) != 0 or ((chctla >> 12) & 3) != 1:
            print("NEXUS_VDP2_BITMAP_SOURCE_INVALID: unsupported NBG1 geometry")
            return 1
        layers.append(("NBG1", (mpo >> 4) & 7, (bmpna >> 8) & 7))
    if not layers:
        print("NEXUS_VDP2_BITMAP_SOURCE_INVALID: no supported bitmap layer is active")
        return 1
    bitmap_size = 512 * 256
    exact = 0
    for layer, map_offset, palette in layers:
        byte_offset = map_offset * 0x20000
        bitmap = frame["vdp2-vram"][byte_offset:byte_offset + bitmap_size]
        if len(bitmap) != bitmap_size:
            print(f"NEXUS_VDP2_BITMAP_SOURCE_INVALID: {layer} span outside VRAM")
            return 1
        print(f"frame={frame_index} register_byte_order={byte_order} "
              f"{layer}_bitmap_offset=0x{byte_offset:06x} bytes={len(bitmap)}")
        print(f"{layer}_bmpna_palette={palette} colour_code=1 bitmap_size_code=0")
        ranked: list[tuple[int, str, int, int]] = []
        layer_exact = 0
        for name, source in sources:
            full = bitmap.find(source) if source and any(source) else -1
            if full >= 0:
                exact += 1
                layer_exact += 1
                print(f"exact_match={layer}:{name} bitmap_offset=0x{full:06x} bytes={len(source)}")
            if not args.exact_only:
                run, source_offset, target_offset = longest_nonzero_match(source, bitmap)
                if run:
                    ranked.append((run, name, source_offset, target_offset))
        for run, name, source_offset, target_offset in sorted(ranked, reverse=True)[:8]:
            print(f"{layer}_partial_match={name} bytes={run} source_offset=0x{source_offset:x} bitmap_offset=0x{target_offset:x}")
        if layer == "NBG0" and title_bin and not args.exact_only:
            run, source_offset, target_offset = longest_nonzero_match(
                title_bin, bitmap)
            if run:
                span = title_bin[source_offset:source_offset + run]
                nonzero = sum(value != 0 for value in span)
                occurrences = title_bin.count(span)
                target_occurrences = bitmap.count(span)
                print(
                    f"NBG0_raw_TITLE.BIN_candidate_bytes={run} "
                    f"nonzero_bytes={nonzero} "
                    f"title_bin_offset=0x{source_offset:x} "
                    f"bitmap_offset=0x{target_offset:x} "
                    f"title_bin_occurrences={occurrences} "
                    f"bitmap_occurrences={target_occurrences}")
                wram_path = (
                    args.capture / "wram-hi.bin"
                    if args.capture.is_dir() else None)
                if wram_path and wram_path.is_file():
                    wram_hi = wram_path.read_bytes()
                    wram_offset = wram_hi.find(span)
                    wram_occurrences = wram_hi.count(span)
                    wram_offset_text = (
                        f"0x{wram_offset:x}" if wram_offset >= 0 else "none")
                    print(
                        f"NBG0_raw_TITLE.BIN_WRAM_hi_offset="
                        f"{wram_offset_text} "
                        f"occurrences={wram_occurrences}")
                print("NBG0_raw_TITLE.BIN_candidate_status=unproven")
        print(f"{layer}_exact_source_matches={layer_exact}")
    cram = frame["vdp2-cram"]
    palette_swap = b"".join(title_palette[offset:offset + 2][::-1]
                             for offset in range(0, len(title_palette), 2))
    print(f"title_palette_cram_match={cram.find(title_palette) if title_palette else -1}")
    print(f"title_palette_cram_word_swap_match={cram.find(palette_swap) if title_palette else -1}")
    menu_palette_swap = b"".join(menu_palette[offset:offset + 2][::-1]
                                  for offset in range(0, len(menu_palette), 2))
    print(f"menu_palette_cram_match={cram.find(menu_palette)}")
    print(f"menu_palette_cram_word_swap_match={cram.find(menu_palette_swap)}")
    stabg_palette_swap = b"".join(stabg_palette[offset:offset + 2][::-1]
                                   for offset in range(0, len(stabg_palette), 2))
    print(f"stabg_palette_cram_match={cram.find(stabg_palette) if stabg_palette else -1}")
    print(f"stabg_palette_cram_word_swap_match={cram.find(stabg_palette_swap) if stabg_palette else -1}")
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
    missing = sorted(optional_names - set(assets))
    print("optional_source_assets_missing=" + ("|".join(missing) if missing else "none"))
    print(f"exact_source_matches={exact}")
    print("source_join=verified" if exact else "source_join=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
