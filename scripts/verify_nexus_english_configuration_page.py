#!/usr/bin/env python3
"""Verify authentic English Nexus Configuration captures and Firestaff output.

The verifier reads retail TITLE.BIN, complete Ymir VDP1/VDP2 captures, and
paired screenshots. It also compiles the production renderer probe and checks
its complete indexed framebuffer and palette against pixels decoded directly
from the retail resource. No generated or substitute game data is used.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import os
import struct
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path

from analyze_nexus_vdp1_source_join import accepted_retail_hashes


ENGLISH_TITLE_SHA256 = "a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44"
TITLE_BIN_BYTES = 112216
SCREEN_W = 320
SCREEN_H = 224
ACTIVE_PALETTE_OFFSETS = (
    0x170AA, 0x1739E, 0x1757A, 0x1785E, 0x17C1A, 0x17F4A,
    0x183D2, 0x188DE, 0x18BAE, 0x19112, 0x1968E, 0x1986A,
    0x19C02, 0x19FA6, 0x1A182, 0x1A3EE, 0x1A572,
)
INACTIVE_PALETTE_OFFSETS = (
    0x172D2, 0x1746A, 0x17A6A, 0x17DCA, 0x18742,
    0x18A7A, 0x195C2, 0x1975A, 0x19EDA, 0x1A072,
)
MARKER_FRAMES = (
    (0x15F6, 0x15D6), (0x1746, 0x1726), (0x1896, 0x1876),
    (0x19E6, 0x19C6), (0x1B36, 0x1B16), (0x1C86, 0x1C66),
    (0x1DD6, 0x1DB6), (0x0B76, 0x0B56), (0x0CC6, 0x0CA6),
    (0x0E16, 0x0DF6), (0x0F66, 0x0F46), (0x10B6, 0x1096),
    (0x1206, 0x11E6), (0x1356, 0x1336), (0x14A6, 0x1486),
)
MARKER_FRAME_STARTS = (0, 2, 4, 7, 9, 12, 14, 17, 19, 22, 24, 27, 29, 32, 34)
MARKER_VISIBLE_TEXELS = (213, 192, 180, 185, 186, 184, 195, 188, 196, 196, 196, 184, 192, 194, 191)


@dataclass(frozen=True)
class PageCase:
    row: int
    music: int
    stereo: int
    easy: int
    gauge: int
    animation_frame: int
    marker_frame: int
    marker_color: int
    marker_y: int
    marker_right_x: int


CASES = {
    "initial": PageCase(0, 1, 1, 0, 1, 0, 0, 0x52F0, 60, 234),
    "music_off": PageCase(0, 0, 1, 0, 1, 22, 9, 0x5290, 60, 234),
    "sound_selected": PageCase(1, 1, 1, 0, 1, 22, 9, 0x5290, 83, 295),
    "mono_selected": PageCase(1, 1, 0, 0, 1, 7, 3, 0x5320, 83, 295),
    "easy_level": PageCase(2, 1, 1, 1, 1, 22, 9, 0x5290, 106, 271),
    "monster_gauge_off": PageCase(3, 1, 1, 0, 0, 22, 9, 0x5290, 128, 233),
    "exit_selected": PageCase(4, 1, 1, 0, 1, 22, 9, 0x5290, 175, 68),
}


TEXT_SPRITES = (
    (0x1A592, 232, 37, 45, 20, "header"),
    (0x170CA, 56, 18, 31, 65, "music_label"),
    (0x172F2, 24, 13, 175, 67, "music_on"),
    (0x1748A, 32, 14, 205, 66, "music_off"),
    (0x1787E, 56, 17, 31, 88, "sound_label"),
    (0x17A8A, 64, 12, 174, 89, "mono"),
    (0x17DEA, 56, 12, 243, 89, "stereo"),
    (0x183F2, 104, 16, 29, 111, "level_label"),
    (0x18762, 56, 13, 175, 111, "normal"),
    (0x18A9A, 40, 13, 236, 111, "easy"),
    (0x19132, 128, 18, 31, 132, "gauge_label"),
    (0x195E2, 24, 13, 175, 134, "gauge_on"),
    (0x1977A, 32, 14, 204, 133, "gauge_off"),
    (0x1A40E, 40, 17, 31, 179, "exit"),
)
ROW_Y = (60, 83, 106, 128, 175)
MARKER_RIGHT_X = (234, 295, 271, 233, 68)


def be16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 2], "big")


def fnv1a64(data: bytes) -> int:
    value = 1469598103934665603
    for byte in data:
        value ^= byte
        value = (value * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return value


def parse_receipt(path: Path) -> dict[str, str]:
    return dict(line.split("=", 1) for line in path.read_text(encoding="ascii").splitlines())


def read_rows(trace_path: Path, frame: str) -> list[dict[str, str]]:
    with trace_path.open(encoding="ascii", newline="") as stream:
        return [row for row in csv.DictReader(stream) if row["frame"] == frame]


def command_words(row: dict[str, str]) -> list[int]:
    return [int(row[f"word{index}"], 16) for index in range(16)]


def find_texture_offset(title: bytes, texture: bytes) -> int:
    offset = title.find(texture)
    if offset < 0:
        return -1
    return offset


def pixel_rgb(word: int) -> tuple[int, int, int]:
    return tuple(((((word >> shift) & 31) << 3) * 31) // 32 for shift in (0, 5, 10))


def expected_colour(name: str, case: PageCase) -> int:
    if name == "header":
        return 0x5390
    if name.endswith("_label") or name == "exit":
        return 0x5380
    if name == "music_on":
        return 0x5380 if case.music else 0x5370
    if name == "music_off":
        return 0x5370 if case.music else 0x5380
    if name == "mono":
        return 0x5370 if case.stereo else 0x5380
    if name == "stereo":
        return 0x5380 if case.stereo else 0x5370
    if name == "normal":
        return 0x5370 if case.easy else 0x5380
    if name == "easy":
        return 0x5380 if case.easy else 0x5370
    if name == "gauge_on":
        return 0x5380 if case.gauge else 0x5370
    if name == "gauge_off":
        return 0x5370 if case.gauge else 0x5380
    raise ValueError(f"unknown text role: {name}")


def expected_text_base(name: str, case: PageCase) -> int:
    if name == "music_on":
        return 240 if case.music else 208
    if name == "music_off":
        return 208 if case.music else 240
    if name == "mono":
        return 208 if case.stereo else 240
    if name == "stereo":
        return 240 if case.stereo else 208
    if name == "normal":
        return 208 if case.easy else 240
    if name == "easy":
        return 240 if case.easy else 208
    if name == "gauge_on":
        return 240 if case.gauge else 208
    if name == "gauge_off":
        return 208 if case.gauge else 240
    return 240


def source_palette_for_role(name: str, case: PageCase) -> int:
    if expected_colour(name, case) == 0x5370:
        return 0x172D2
    return 0x170AA


def visible_pixels(texture: bytes, width: int, height: int,
                   flip_x: bool = False) -> list[tuple[int, int, int]]:
    result = []
    for y in range(height):
        for x in range(width):
            sx = width - 1 - x if flip_x else x
            index = y * width + sx
            packed = texture[index >> 1]
            texel = packed & 15 if index & 1 else packed >> 4
            if texel:
                result.append((x, y, texel))
    return result


def trace_sprite(rows: list[dict[str, str]], vdp1: bytes, title: bytes,
                 source_offset: int, width: int, height: int, x: int, y: int,
                 flip: bool, color: int) -> tuple[dict[str, str], bytes]:
    matches = []
    for row in rows:
        words = command_words(row)
        actual_width = ((words[5] >> 8) & 0x3F) * 8
        actual_height = words[5] & 0xFF
        texture_start = words[4] * 8
        texture_size = width * height // 2
        texture = vdp1[texture_start:texture_start + texture_size]
        if (actual_width == width and actual_height == height and
                words[6:8] == [x, y] and words[0] == (0x1010 if flip else 0x1000) and
                words[2] == 0x80 and words[3] == color and
                texture == title[source_offset:source_offset + texture_size]):
            matches.append((row, texture))
    if len(matches) != 1:
        raise ValueError(
            f"expected one sprite source={source_offset:#x} position=({x},{y}); "
            f"found {len(matches)}")
    return matches[0]


def load_capture_palette(source: bytes, offset: int, color_base: int,
                         palette: list[int]) -> None:
    for index in range(1, 16):
        word = be16(source, offset + index * 2)
        channels = [((((word >> shift) & 31) << 3) * 31) // 32
                    for shift in (0, 5, 10)]
        red, green, blue = channels
        palette[color_base + index] = (
            0xFF000000 | (red << 16) | (green << 8) | blue
        )


def expected_index_map(title: bytes, case: PageCase) -> bytes:
    framebuffer = bytearray(SCREEN_W * SCREEN_H)

    def draw(source_offset: int, width: int, height: int, x: int, y: int,
             color_base: int, flip_x: bool = False) -> None:
        texture = title[source_offset:source_offset + width * height // 2]
        for px, py, texel in visible_pixels(texture, width, height, flip_x):
            framebuffer[(y + py) * SCREEN_W + x + px] = color_base + texel

    for source_offset, width, height, x, y, role in TEXT_SPRITES:
        draw(source_offset, width, height, x, y, expected_text_base(role, case))
    marker_source = MARKER_FRAMES[case.marker_frame][0]
    draw(marker_source, 24, 24, 10, case.marker_y, 224)
    draw(marker_source, 24, 24, case.marker_right_x, case.marker_y, 224, True)
    return bytes(framebuffer)


def verify_production_renderer(root: Path, title: bytes, title_path: Path,
                               evidence_root: Path, cc: str,
                               source_region: str = "english") -> None:
    with tempfile.TemporaryDirectory(prefix="firestaff-nexus-config-") as tmp:
        binary = Path(tmp) / "probe"
        output = Path(tmp) / "renderer.bin"
        compile_command = [
            cc, "-std=c11", "-Wall", "-Wextra", "-Werror",
            "-I", str(root / "include"),
            str(root / "scripts" / "probe_nexus_english_configuration_renderer.c"),
            str(root / "src/nexus/nexus_v1_retail_title_menu_renderer.c"),
            str(root / "src/nexus/nexus_v1_retail_title_menu.c"),
            "-o", str(binary),
        ]
        subprocess.run(compile_command, check=True, cwd=root)
        for name, case in CASES.items():
            subprocess.run([
                str(binary), str(title_path), str(output),
                str(case.row), str(case.music), str(case.stereo),
                str(case.easy), str(case.gauge), str(case.animation_frame),
                source_region,
            ], check=True, cwd=root)
            rendered = output.read_bytes()
            expected = expected_index_map(title, case)
            if len(rendered) != SCREEN_W * SCREEN_H + 256 * 4:
                raise ValueError(f"{name}: renderer probe output size is invalid")
            if rendered[:SCREEN_W * SCREEN_H] != expected:
                different = sum(a != b for a, b in zip(
                    rendered[:SCREEN_W * SCREEN_H], expected))
                raise ValueError(f"{name}: production index map differs at {different} pixels")

            expected_palette = [0] * 256
            for source_offset, width, height, x, y, role in TEXT_SPRITES:
                load_capture_palette(
                    title, source_palette_for_role(role, case),
                    expected_text_base(role, case), expected_palette)
            marker_palette_offset = MARKER_FRAMES[case.marker_frame][1]
            load_capture_palette(title, marker_palette_offset, 224, expected_palette)
            actual_palette = struct.unpack("=256I", rendered[SCREEN_W * SCREEN_H:])
            if tuple(expected_palette) != actual_palette:
                raise ValueError(f"{name}: production palette differs from retail CRAM source")
            print(f"renderer={name} indexed_pixels=71680 palette=256/256")


def verify_capture(name: str, case: PageCase, folder: Path, title: bytes) -> None:
    bundle_dir = folder / "dumps"
    receipt = parse_receipt(bundle_dir / "vdp1-capture-bundle.txt")
    if (receipt.get("format") != "ymir-vdp1-capture-bundle-v1" or
            receipt.get("trace_file_complete") != "1" or
            receipt.get("trace_dropped_total") != "0"):
        raise ValueError(f"{name}: Ymir VDP1 trace is incomplete")
    capture_frame = receipt["capture_frame_index"]
    rows = read_rows(bundle_dir / receipt["trace_file"], capture_frame)
    if (len(rows) != int(receipt["trace_rows_for_capture_frame"]) or
            len(rows) != int(receipt["capture_next_command_index"]) or
            len(rows) != 189):
        raise ValueError(f"{name}: expected the complete 189-command frame")

    vdp1 = (bundle_dir / "vdp1-vram.bin").read_bytes()
    cram = (bundle_dir / "vdp2-cram.bin").read_bytes()
    regs = (bundle_dir / "vdp2-registers.bin").read_bytes()
    if len(vdp1) != 512 * 1024 or len(cram) != 4096 or len(regs) != 512:
        raise ValueError(f"{name}: Ymir VDP memory capture is incomplete")
    reg = lambda offset: be16(regs, offset)
    if reg(0xE0) != 0x1536 or reg(0xEC) & 0x740 != 0x40:
        raise ValueError(f"{name}: unexpected VDP2 sprite mode")

    screenshot = next((folder / "screenshots").glob("*.png"), None)
    if screenshot is None:
        raise ValueError(f"{name}: screenshot is missing")
    from PIL import Image

    image = Image.open(screenshot).convert("RGB")
    if image.size != (640, 448):
        raise ValueError(f"{name}: screenshot is not the captured 640x448 surface")

    text_pixels = 0
    text_matches = 0
    for source_offset, width, height, x, y, role in TEXT_SPRITES:
        color = expected_colour(role, case)
        row, texture = trace_sprite(
            rows, vdp1, title, source_offset, width, height, x, y, False, color)
        words = command_words(row)
        palette_start = (color & 0x03F0) * 2
        palette = cram[palette_start:palette_start + 32]
        allowed = ACTIVE_PALETTE_OFFSETS if color != 0x5370 else INACTIVE_PALETTE_OFFSETS
        if not any(title[offset:offset + 32] == palette for offset in allowed):
            raise ValueError(f"{name}: {role} palette is not source-bound")
        priority_index = (color >> 12) & 7
        priority_word = reg(0xF0 + (priority_index // 2) * 2)
        priority = (priority_word >> (8 if priority_index & 1 else 0)) & 7
        ratio_index = (color >> 10) & 3
        ratio_word = reg(0x100 + (ratio_index // 2) * 2)
        ratio = 31 - ((ratio_word >> (8 if ratio_index & 1 else 0)) & 31)
        if priority != 5 or ratio != 31:
            raise ValueError(f"{name}: {role} priority or 31/32 ratio differs")
        for px, py, texel in visible_pixels(texture, width, height):
            text_pixels += 1
            color_index = (color & 0x03F0) | texel
            rgb = pixel_rgb(be16(cram, color_index * 2))
            if image.getpixel(((x + px) * 2, (y + py) * 2)) == rgb:
                text_matches += 1

    marker_source, marker_palette = MARKER_FRAMES[case.marker_frame]
    marker_specs = (
        (10, case.marker_y, False),
        (case.marker_right_x, case.marker_y, True),
    )
    for x, y, flip in marker_specs:
        row, texture = trace_sprite(
            rows, vdp1, title, marker_source, 24, 24, x, y, flip,
            case.marker_color)
        words = command_words(row)
        palette_start = (words[3] & 0x03F0) * 2
        if title[marker_palette:marker_palette + 32] != cram[palette_start:palette_start + 32]:
            raise ValueError(f"{name}: animated marker palette is not source-bound")
        if len(visible_pixels(texture, 24, 24, flip)) != \
                MARKER_VISIBLE_TEXELS[case.marker_frame]:
            raise ValueError(f"{name}: unexpected marker frame texel count")

    if text_pixels != 12518 or text_matches < 12500:
        raise ValueError(
            f"{name}: text screenshot matches {text_matches}/{text_pixels} pixels")
    print(
        f"capture={name} frame={capture_frame} commands=189 "
        f"source_text_pixels={text_matches}/{text_pixels} marker={marker_source:#x}"
    )


def verify(root: Path, evidence_root: Path, title_path: Path, cc: str) -> None:
    title = title_path.read_bytes()
    digest = hashlib.sha256(title).hexdigest()
    if (len(title) != TITLE_BIN_BYTES or digest != ENGLISH_TITLE_SHA256 or
            digest not in accepted_retail_hashes("TITLE.BIN")):
        raise ValueError("TITLE.BIN is not the authenticated English retail resource")
    if fnv1a64(title) != 0xF512153Befe61770:
        raise ValueError("English TITLE.BIN has an unexpected runtime fingerprint")

    for name, case in CASES.items():
        verify_capture(name, case, evidence_root / name, title)
    verify_production_renderer(root, title, title_path, evidence_root, cc)

    exit_return = evidence_root / "exit_return" / "screenshots"
    returned = next(exit_return.glob("*.png"), None)
    if returned is None or hashlib.sha256(returned.read_bytes()).hexdigest() != (
            "9d3223cd2fcfbc32acf01e86f25194258fd06bb2852384800a720ad1604d775c"):
        raise ValueError("Exit accept did not return to the captured title menu")
    print("exit_accept=returned_to_title_menu")
    print("semantic_admission=english_configuration_source_bound")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("evidence_root", type=Path,
                        help="English Configuration evidence directory")
    parser.add_argument("title_bin", type=Path,
                        help="authenticated English retail TITLE.BIN")
    parser.add_argument("--cc", default=os.environ.get("CC", "cc"),
                        help="C compiler for the production renderer probe")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    try:
        verify(root, args.evidence_root, args.title_bin, args.cc)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.error(str(error))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
