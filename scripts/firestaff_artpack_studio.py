#!/usr/bin/env python3
"""Firestaff V2.2 Artpack Studio.

Cross-platform Tk/Pillow tool for building modern V2.2 artpacks for all
Firestaff games. It edits the shared modern_asset_manifest.json convention,
loads V1/reference images and V2.2 target images, supports pixel/color edits,
imports generated art, and can call an external AI generator command.

AI generation is intentionally command-based instead of hard-coding one cloud
API. Set FIRESTAFF_ARTPACK_AI_COMMAND to a command template containing any of:
  {prompt_file} {output} {source} {game} {category} {asset_id} {width} {height}

Example:
  FIRESTAFF_ARTPACK_AI_COMMAND='my-generator --prompt {prompt_file} --ref {source} --out {output}'

The tool never ships copyrighted game data. It only reads local operator files
and writes artpack PNGs/manifests under ~/.firestaff/assets/<game>/modern by
default.
"""

from __future__ import annotations

import argparse
import datetime as _dt
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
import zipfile
from dataclasses import dataclass
from pathlib import Path
from string import Template
from typing import Any, Iterable

try:
    from PIL import Image, ImageColor, ImageDraw
except Exception as exc:  # pragma: no cover - exercised by startup path
    raise SystemExit(
        "Firestaff Artpack Studio requires Pillow. Install with: "
        "python3 -m pip install Pillow\n"
        f"Import error: {exc}"
    )

def require_supported_tk(tk_module: Any) -> None:
    """Reject the Apple-supplied Tk 8.5 runtime before it can paint a blank UI."""
    version = tuple(int(part) for part in str(tk_module.TkVersion).split(".")[:2])
    if version < (8, 6):
        raise RuntimeError(
            f"Tk {tk_module.TkVersion} is unsupported; Firestaff Artpack Studio needs Tk 8.6 or newer"
        )


if "--check-tkinter" in sys.argv:
    try:
        import tkinter as tk
        from PIL import ImageTk
        require_supported_tk(tk)
    except Exception as exc:  # pragma: no cover - packaged runtime smoke check
        raise SystemExit(f"Tkinter is required for the GUI: {exc}")
    print(f"Tkinter runtime check: PASS (Tk {tk.TkVersion})")
    raise SystemExit(0)

if "--self-test" in sys.argv:
    class _DummyTk:
        Tk = object
        Canvas = object
        Event = object
        Misc = object

    class _DummyTtk:
        Frame = object

    tk = _DummyTk()  # type: ignore[assignment]
    ttk = _DummyTtk()  # type: ignore[assignment]
    ImageTk = None  # type: ignore[assignment]
    colorchooser = filedialog = messagebox = simpledialog = None  # type: ignore[assignment]
else:
    try:
        import tkinter as tk
        from tkinter import colorchooser, filedialog, messagebox, simpledialog, ttk
        from PIL import ImageTk
    except Exception as exc:  # pragma: no cover - exercised by startup path
        raise SystemExit(f"Tkinter is required for the GUI: {exc}")


GAMES = ("dm1", "csb", "dm2", "theron", "nexus")
FSART_SUFFIX = ".fsart"
IMAGE_EXTENSIONS = {".png", ".bmp", ".gif", ".jpg", ".jpeg", ".tga", ".webp"}
REPO_ROOT = Path(__file__).resolve().parents[1]
FIRESTAFF_LOGO = REPO_ROOT / "assets" / "branding" / "firestaff-logo.png"
VERIFIED_HASHES = REPO_ROOT / "docs" / "VERIFIED_HASHES.md"

COMMON_CATEGORIES = (
    "wall_shapes",
    "floor_shapes",
    "creature_shapes",
    "object_shapes",
    "projectile_shapes",
    "explosion_shapes",
    "door_shapes",
    "field_shapes",
    "ui_chrome",
    "champion_portraits",
    "title_frames",
    "entrance_frames",
    "menu_surfaces",
)

GAME_EXTRA_CATEGORIES = {
    "csb": ("chaos_runes", "dsa_scrolls"),
    "dm2": ("weather_shapes", "hud_widgets", "tech_ui"),
    "theron": ("soul_room", "track02_levels", "pcengine_ui"),
    "nexus": ("saturn_menu", "dgn_textures", "structure2_textures", "sfx_icons"),
}

DM1_REQUIRED_SLOTS = [
    ("wall_shapes", "wall_d3_carved_hero_01", "wall_d3_carved_hero_01.png"),
    ("floor_shapes", "floor_plain_hero_01", "floor_plain_hero_01.png"),
    ("floor_shapes", "floor_pit_hero_01", "floor_pit_hero_01.png"),
    ("creature_shapes", "creature_demon_hero_01", "creature_demon_hero_01.png"),
    ("champion_portraits", "champion_warrior_hero_01", "champion_warrior_hero_01.png"),
    ("door_shapes", "door_hero_01", "door_hero_01.png"),
    ("field_shapes", "field_teleporter_hero_01", "field_teleporter_hero_01.png"),
]

DEFAULT_REQUIRED = {
    "dm1": DM1_REQUIRED_SLOTS,
    "csb": [
        ("wall_shapes", "csb_wall_d3_carved_hero_01", "csb_wall_d3_carved_hero_01.png"),
        ("floor_shapes", "csb_floor_plain_hero_01", "csb_floor_plain_hero_01.png"),
        ("creature_shapes", "csb_creature_demon_hero_01", "csb_creature_demon_hero_01.png"),
    ],
    "dm2": [
        ("wall_shapes", "dm2_wall_cave_hero_01", "dm2_wall_cave_hero_01.png"),
        ("floor_shapes", "dm2_floor_hero_01", "dm2_floor_hero_01.png"),
        ("ui_chrome", "dm2_hud_frame_hero_01", "dm2_hud_frame_hero_01.png"),
    ],
    "theron": [
        ("soul_room", "theron_soul_room_hero_01", "theron_soul_room_hero_01.png"),
        ("track02_levels", "theron_level_wall_hero_01", "theron_level_wall_hero_01.png"),
        ("ui_chrome", "theron_pcengine_frame_hero_01", "theron_pcengine_frame_hero_01.png"),
    ],
    "nexus": [
        ("saturn_menu", "nexus_menu_panel_hero_01", "nexus_menu_panel_hero_01.png"),
        ("dgn_textures", "nexus_dgn_wall_hero_01", "nexus_dgn_wall_hero_01.png"),
        ("structure2_textures", "nexus_structure2_hero_01", "nexus_structure2_hero_01.png"),
    ],
}


def fnv1a32(data: bytes) -> int:
    h = 2166136261
    for b in data:
        h = ((h ^ b) * 16777619) & 0xFFFFFFFF
    return h


def default_modern_dir(game: str) -> Path:
    return Path.home() / ".firestaff" / "assets" / game / "modern"


def utc_now() -> str:
    return _dt.datetime.now(_dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def safe_asset_filename(asset_id: str) -> str:
    cleaned = []
    for ch in asset_id.strip():
        if ch.isalnum() or ch in ("_", "-", "."):
            cleaned.append(ch)
        else:
            cleaned.append("_")
    name = "".join(cleaned).strip("._")
    return (name or "asset") + ".png"


def default_reference_dir(game: str) -> Path:
    return Path.home() / ".firestaff" / "data" / game


def infer_category(path: Path, game: str) -> str:
    categories = set(categories_for_game(game))
    for part in reversed(path.parts):
        if part in categories:
            return part
    lower = path.stem.lower()
    if "champ" in lower or "portrait" in lower:
        return "champion_portraits"
    if "title" in lower:
        return "title_frames"
    if "entrance" in lower:
        return "entrance_frames"
    if "wall" in lower:
        return "wall_shapes"
    if "floor" in lower:
        return "floor_shapes"
    if "door" in lower:
        return "door_shapes"
    if "creature" in lower or "monster" in lower:
        return "creature_shapes"
    if "object" in lower or "item" in lower:
        return "object_shapes"
    if "spell" in lower or "projectile" in lower:
        return "projectile_shapes"
    return "ui_chrome"


def ensure_rgba(img: Image.Image) -> Image.Image:
    if img.mode != "RGBA":
        return img.convert("RGBA")
    return img


def categories_for_game(game: str) -> list[str]:
    cats = list(COMMON_CATEGORIES)
    for cat in GAME_EXTRA_CATEGORIES.get(game, ()):
        if cat not in cats:
            cats.append(cat)
    return cats


@dataclass
class AssetEntry:
    category: str
    asset_id: str
    source_file: str
    width: int
    height: int
    generator: str
    notes: str = ""


@dataclass
class ReferenceAsset:
    category: str
    asset_id: str
    path: Path
    width: int
    height: int


@dataclass
class GameDataAsset:
    category: str
    asset_id: str
    path: Path
    width: int
    height: int
    source_game: str
    source_kind: str
    index: int
    offset: int
    compressed_bytes: int
    decompressed_bytes: int
    sha256: str
    warning: str = ""


@dataclass
class GameDataImportResult:
    path: Path
    detected_game: str
    detected_variant: str
    file_sha256: str
    file_size: int
    assets: list[GameDataAsset]
    warnings: list[str]


DM1_DUNGEON_PALETTE = [
    (0x00, 0x00, 0x00, 255),
    (0x66, 0x66, 0x66, 255),
    (0x88, 0x88, 0x88, 255),
    (0x66, 0x22, 0x00, 255),
    (0x00, 0xCC, 0xCC, 255),
    (0x88, 0x44, 0x00, 255),
    (0x00, 0x88, 0x00, 255),
    (0x00, 0xCC, 0x00, 255),
    (0xFF, 0x00, 0x00, 255),
    (0xFF, 0xAA, 0x00, 255),
    (0xCC, 0x88, 0x66, 255),
    (0xFF, 0xFF, 0x00, 255),
    (0x44, 0x44, 0x44, 255),
    (0xAA, 0xAA, 0xAA, 255),
    (0x00, 0x00, 0xFF, 255),
    (0xFF, 0xFF, 0xFF, 255),
]

DM2_PREVIEW_PALETTE = [
    (0x00, 0x00, 0x00, 255),
    (0x2A, 0x2A, 0x2A, 255),
    (0x55, 0x55, 0x55, 255),
    (0x7F, 0x7F, 0x7F, 255),
    (0xAA, 0xAA, 0xAA, 255),
    (0xD4, 0xD4, 0xD4, 255),
    (0xE6, 0xD0, 0xA8, 255),
    (0xB8, 0x88, 0x58, 255),
    (0x7E, 0x52, 0x2C, 255),
    (0x31, 0x64, 0x30, 255),
    (0x4F, 0x8E, 0x44, 255),
    (0x82, 0xB8, 0x60, 255),
    (0x2B, 0x6B, 0x8E, 255),
    (0x54, 0xA0, 0xBE, 255),
    (0xC8, 0x40, 0x30, 255),
    (0xFF, 0xD0, 0x40, 255),
]


def le16(data: bytes, offset: int) -> int:
    return data[offset] | (data[offset + 1] << 8)


def be16(data: bytes, offset: int) -> int:
    return (data[offset] << 8) | data[offset + 1]


def parse_verified_hashes() -> dict[str, tuple[str, str, int]]:
    out: dict[str, tuple[str, str, int]] = {}
    if not VERIFIED_HASHES.exists():
        return out
    for line in VERIFIED_HASHES.read_text(encoding="utf-8").splitlines():
        if not line.startswith("|"):
            continue
        cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
        if len(cells) != 4 or cells[0].lower() in {"game", "---"}:
            continue
        game, filename, sha, size = cells
        sha = sha.strip("`").lower()
        try:
            byte_count = int(size.replace(",", ""))
        except ValueError:
            continue
        if len(sha) == 64:
            out[sha] = (game, filename, byte_count)
    return out


def game_from_variant(variant: str) -> str:
    if variant.startswith("dm1"):
        return "dm1"
    if variant.startswith("csb"):
        return "csb"
    if variant.startswith("dm2"):
        return "dm2"
    if variant.startswith("theron"):
        return "theron"
    if variant.startswith("nexus"):
        return "nexus"
    return variant if variant in GAMES else "dm1"


def category_for_graphics_index(index: int, game: str) -> str:
    if index <= 8:
        return "title_frames" if index <= 1 else "entrance_frames"
    if 78 <= index <= 117:
        return "wall_shapes"
    if 118 <= index <= 189:
        return "object_shapes"
    if 190 <= index <= 339:
        return "creature_shapes"
    if 340 <= index <= 450:
        return "projectile_shapes"
    if 451 <= index <= 520:
        return "explosion_shapes"
    if 521 <= index <= 590:
        return "ui_chrome"
    if game == "dm2":
        return "menu_surfaces" if index < 80 else "object_shapes"
    return "floor_shapes"


class NibbleReader:
    def __init__(self, source: bytes) -> None:
        self.source = source
        self.nibble_offset = 8

    def nibble(self) -> int:
        byte = self.source[self.nibble_offset >> 1]
        value = (byte & 0x0F) if (self.nibble_offset & 1) else (byte >> 4)
        self.nibble_offset += 1
        return value

    def pixel_count(self) -> int:
        count = self.nibble()
        if count == 15:
            count = (self.nibble() << 4) | self.nibble()
            if count == 255:
                count = (
                    (self.nibble() << 12)
                    | (self.nibble() << 8)
                    | (self.nibble() << 4)
                    | self.nibble()
                )
            else:
                count += 17
        else:
            count += 2
        return count


def expand_img3_to_image(source: bytes, width: int, height: int) -> Image.Image:
    if len(source) < 8:
        raise ValueError("entry too short for IMG3")
    if le16(source, 0) != width or le16(source, 2) != height:
        raise ValueError("entry-local width/height prefix mismatch")
    stride_pixels = (width + 1) & ~1
    packed = bytearray((stride_pixels * height) // 2)

    def get_pixel(pixel_offset: int) -> int:
        if pixel_offset < 0:
            return 0
        byte = packed[pixel_offset >> 1]
        return byte & 0x0F if (pixel_offset & 1) else byte >> 4

    def set_pixel(pixel_offset: int, color: int) -> None:
        byte_index = pixel_offset >> 1
        if byte_index < 0 or byte_index >= len(packed):
            return
        color &= 0x0F
        if pixel_offset & 1:
            packed[byte_index] = (packed[byte_index] & 0xF0) | color
        else:
            packed[byte_index] = (packed[byte_index] & 0x0F) | (color << 4)

    reader = NibbleReader(source)
    local_palette = [reader.nibble() for _ in range(6)]
    destination_offset = 0
    total_visible_pixels = width * height
    if width == stride_pixels:
        while destination_offset < total_visible_pixels:
            command = reader.nibble()
            kind = command & 0x07
            if kind == 6:
                color = 0
            else:
                color = local_palette[kind] if kind < 6 else reader.nibble()
            count = reader.pixel_count() if (command & 0x08) else 1
            for _ in range(count):
                if destination_offset >= total_visible_pixels:
                    break
                set_pixel(destination_offset, get_pixel(destination_offset - stride_pixels) if kind == 6 else color)
                destination_offset += 1
    else:
        total_done = 0
        remaining_in_line = width
        while total_done < total_visible_pixels:
            command = reader.nibble()
            kind = command & 0x07
            color = local_palette[kind] if kind < 6 else (0 if kind == 6 else reader.nibble())
            count = reader.pixel_count() if (command & 0x08) else 1
            left = count
            while left > 0:
                chunk = remaining_in_line if left >= remaining_in_line else left
                for _ in range(chunk):
                    set_pixel(destination_offset, get_pixel(destination_offset - stride_pixels) if kind == 6 else color)
                    destination_offset += 1
                left -= chunk
                remaining_in_line -= chunk
                if remaining_in_line == 0:
                    destination_offset += stride_pixels - width
                    remaining_in_line = width
            total_done += count
    img = Image.new("RGBA", (max(1, width), max(1, height)))
    pix = img.load()
    for y in range(height):
        for x in range(width):
            pix[x, y] = DM1_DUNGEON_PALETTE[get_pixel(y * stride_pixels + x) & 0x0F]
    return img


def dm2_img3_signed_offset(value: int) -> int:
    if value & 0x8000:
        value -= 0x10000
    return value >> 10


def dm2_record_dimensions(raw: bytes) -> tuple[int, int, int, int]:
    if len(raw) < 10:
        return 0, 0, 0, 0
    cx = le16(raw, 0)
    cy = le16(raw, 2)
    w4 = le16(raw, 4)
    return cx & 0x03FF, cy & 0x03FF, w4, dm2_img3_signed_offset(cy)


class Dm2NibbleReader:
    def __init__(self, raw: bytes, start_nibble: int = 16) -> None:
        self.raw = raw
        self.cursor = start_nibble

    def nibble(self) -> int:
        byte_pos = self.cursor >> 1
        if byte_pos >= len(self.raw):
            raise ValueError("truncated IMG3 nibble stream")
        value = (self.raw[byte_pos] & 0x0F) if (self.cursor & 1) else (self.raw[byte_pos] >> 4)
        self.cursor += 1
        return value

    def duration(self) -> int:
        n = self.nibble()
        if n == 0x0F:
            v = (self.nibble() << 4) | self.nibble()
            if v == 0xFF:
                v = (self.nibble() << 12) | (self.nibble() << 8) | (self.nibble() << 4) | self.nibble()
                if v <= 0:
                    raise ValueError("invalid IMG3 long duration")
                return v
            return v + 0x11
        return n + 2


def dm2_emit_run(
    padded: bytearray,
    width: int,
    even_width: int,
    pos_ref: list[int],
    line_left_ref: list[int],
    count: int,
    color: int,
    copy_previous_line: bool,
) -> None:
    total = len(padded)
    while count > 0:
        pos = pos_ref[0]
        line_left = line_left_ref[0]
        if line_left <= 0 or pos < 0 or pos >= total:
            raise ValueError("invalid IMG3 destination state")
        n = min(count, line_left)
        if pos + n > total:
            raise ValueError("IMG3 run exceeds output")
        if copy_previous_line:
            copy_n = line_left if n < line_left else n
            if pos < even_width or pos + copy_n > total:
                raise ValueError("IMG3 previous-line copy out of range")
            for i in range(copy_n):
                padded[pos + i] = padded[pos - even_width + i]
        else:
            padded[pos : pos + n] = bytes([color & 0xFF]) * n
        pos += n
        line_left -= n
        count -= n
        if line_left == 0 and pos < total:
            pos += even_width - width
            line_left = width
        pos_ref[0] = pos
        line_left_ref[0] = line_left


def dm2_decode_img3_c4(raw: bytes, width: int, height: int) -> bytes:
    if len(raw) < 10 or width <= 0 or height <= 0:
        raise ValueError("invalid IMG3 C4 header")
    even_width = (width + 1) & ~1
    padded = bytearray(even_width * height)
    palette = list(raw[10:16])
    if len(palette) < 6:
        raise ValueError("missing IMG3 local palette")
    reader = Dm2NibbleReader(raw, 16)
    pos_ref = [0]
    line_left_ref = [width]
    while pos_ref[0] < len(padded):
        command = reader.nibble()
        code = command & 0x07
        if code == 6:
            run = reader.duration() if (command & 0x08) else 1
            dm2_emit_run(padded, width, even_width, pos_ref, line_left_ref, run, 0, True)
            continue
        if code < 6:
            color = palette[code]
        else:
            color = reader.nibble()
        run = reader.duration() if (command & 0x08) else 1
        dm2_emit_run(padded, width, even_width, pos_ref, line_left_ref, run, color, False)
    pixels = bytearray(width * height)
    for y in range(height):
        start = y * even_width
        pixels[y * width : (y + 1) * width] = padded[start : start + width]
    return bytes(pixels)


def dm2_decode_uncompressed(raw: bytes, width: int, height: int, bpp: int) -> bytes:
    if len(raw) < 10 or width <= 0 or height <= 0:
        raise ValueError("invalid uncompressed GDAT image")
    pixel_count = width * height
    if bpp == 4:
        src_size = (((width + 1) & ~1) // 2) * height
        payload = raw[10 : 10 + src_size]
        if len(payload) < src_size:
            raise ValueError("truncated U4 image")
        out = bytearray(pixel_count)
        for i in range(pixel_count):
            byte = payload[i >> 1]
            out[i] = (byte & 0x0F) if (i & 1) else ((byte >> 4) & 0x0F)
        return bytes(out)
    if bpp == 8:
        payload = raw[10 : 10 + pixel_count]
        if len(payload) < pixel_count:
            raise ValueError("truncated U8 image")
        return bytes(payload)
    raise ValueError(f"unsupported uncompressed bpp {bpp}")


def dm2_indices_to_image(pixels: bytes, width: int, height: int) -> Image.Image:
    img = Image.new("RGBA", (max(1, width), max(1, height)))
    pix = img.load()
    for y in range(height):
        for x in range(width):
            pix[x, y] = DM2_PREVIEW_PALETTE[pixels[y * width + x] & 0x0F]
    return img


def dm2_decode_gdat_image(raw: bytes) -> tuple[Image.Image, str]:
    width, height, w4, offset_y = dm2_record_dimensions(raw)
    if width <= 0 or height <= 0:
        raise ValueError("GDAT image has zero dimensions")
    if offset_y == -32:
        pixels = dm2_decode_uncompressed(raw, width, height, w4)
        return dm2_indices_to_image(pixels, width, height), f"U{w4}"
    if offset_y == 31:
        raise ValueError("IMG9 C8 preview not implemented")
    pixels = dm2_decode_img3_c4(raw, width, height)
    return dm2_indices_to_image(pixels, width, height), "IMG3-C4"


def parse_graphics_dat_assets(path: Path, data: bytes, detected_game: str) -> tuple[list[GameDataAsset], list[str]]:
    warnings: list[str] = []
    assets: list[GameDataAsset] = []
    if len(data) < 6:
        return assets, ["file is too small for GRAPHICS.DAT"]
    signature_le = le16(data, 0)
    big_endian = False
    if signature_le == 0x0180:
        signature = 0x8001
        big_endian = True
    else:
        signature = signature_le
    read16 = be16 if big_endian else le16
    if signature & 0x8000:
        fmt = signature & 0x7FFF
        if fmt != 1:
            return assets, [f"unsupported GRAPHICS.DAT format {fmt}"]
        count = read16(data, 2)
        header_bytes = 4 + count * 8
        if count <= 0 or count > 4096 or len(data) < header_bytes:
            return assets, [f"invalid GRAPHICS.DAT header count={count}"]
        cursor = header_bytes
        for index in range(count):
            comp = read16(data, 4 + 2 * index)
            decomp = read16(data, 4 + 2 * count + 2 * index)
            width = read16(data, 4 + 4 * count + 4 * index)
            height = read16(data, 4 + 4 * count + 4 * index + 2)
            warning = ""
            if width <= 0 or height <= 0:
                warning = "zero-size entry"
            if cursor + comp > len(data):
                warning = "entry exceeds file size"
            record = data[cursor : min(len(data), cursor + comp)]
            if record and not warning:
                try:
                    expand_img3_to_image(record, width, height)
                except Exception as exc:
                    warning = f"metadata only; IMG3 decode warning: {exc}"
            assets.append(
                GameDataAsset(
                    category=category_for_graphics_index(index, detected_game),
                    asset_id=f"graphics_{index:04d}",
                    path=path,
                    width=width,
                    height=height,
                    source_game=detected_game,
                    source_kind="GRAPHICS.DAT",
                    index=index,
                    offset=cursor,
                    compressed_bytes=comp,
                    decompressed_bytes=decomp,
                    sha256=hashlib.sha256(record).hexdigest() if record else "",
                    warning=warning,
                )
            )
            cursor += comp
        if cursor != len(data):
            warnings.append(f"record byte sum {cursor} differs from file size {len(data)}")
        if big_endian:
            warnings.append("big-endian GRAPHICS.DAT header detected")
        return assets, warnings
    count = signature
    if count <= 0 or count > 4096:
        return assets, [f"unsupported or invalid old GRAPHICS.DAT count={count}"]
    header_bytes = 2 + count * 2
    if len(data) < header_bytes:
        return assets, ["file too small for old GRAPHICS.DAT header"]
    cursor = header_bytes
    for index in range(count):
        comp = read16(data, 2 + 2 * index)
        width = read16(data, cursor) if cursor + 4 <= len(data) else 0
        height = read16(data, cursor + 2) if cursor + 4 <= len(data) else 0
        assets.append(
            GameDataAsset(
                category=category_for_graphics_index(index, detected_game),
                asset_id=f"graphics_{index:04d}",
                path=path,
                width=width,
                height=height,
                source_game=detected_game,
                source_kind="GRAPHICS.DAT-old",
                index=index,
                offset=cursor,
                compressed_bytes=comp,
                decompressed_bytes=0,
                sha256=hashlib.sha256(data[cursor : min(len(data), cursor + comp)]).hexdigest(),
                warning="old-format metadata; preview decode not implemented",
            )
        )
        cursor += comp
    return assets, warnings


def dm2_category_to_artpack_category(category: int) -> str:
    if category == 0x05:
        return "title_frames"
    if category in (0x01, 0x07, 0x1A):
        return "ui_chrome"
    if category in (0x08, 0x09):
        return "wall_shapes"
    if category == 0x0A:
        return "floor_shapes"
    if category in (0x0B, 0x0C, 0x0E):
        return "door_shapes"
    if category in (0x0D,):
        return "projectile_shapes"
    if category == 0x0F:
        return "creature_shapes"
    if category in (0x10, 0x11, 0x12, 0x13, 0x14, 0x15):
        return "object_shapes"
    if category == 0x17:
        return "weather_shapes"
    return "menu_surfaces"


def dm2_parse_gdat_assets(path: Path, data: bytes) -> tuple[list[GameDataAsset], list[str]]:
    warnings: list[str] = []
    assets: list[GameDataAsset] = []
    if len(data) < 8 or le16(data, 0) != 0x8005:
        return assets, ["not a supported DM2 GDAT container"]
    raw_count = le16(data, 2)
    if raw_count <= 0 or raw_count > 20000:
        return assets, [f"invalid DM2 raw count {raw_count}"]
    if len(data) < 8 + ((raw_count - 1) * 2):
        return assets, ["DM2 raw table exceeds file"]
    raw_offsets = [0] * raw_count
    raw_sizes = [0] * raw_count
    raw0_size = int.from_bytes(data[4:8], "little")
    offset = 6 + raw_count * 2
    if offset + raw0_size > len(data):
        return assets, ["DM2 raw[0] exceeds file"]
    raw_offsets[0] = offset
    raw_sizes[0] = raw0_size
    offset += raw0_size
    for i in range(1, raw_count):
        size = le16(data, 8 + ((i - 1) * 2))
        raw_offsets[i] = offset
        raw_sizes[i] = size
        if offset + size > len(data):
            return assets, [f"DM2 raw[{i}] exceeds file"]
        offset += size
    if offset != len(data):
        warnings.append(f"DM2 raw byte sum {offset} differs from file size {len(data)}")
    ent = data[raw_offsets[0] : raw_offsets[0] + raw_sizes[0]]
    if len(ent) < 6:
        return assets, ["DM2 ENT1 raw is too small"]
    if le16(ent, 0) == 0x8001:
        read16 = le16
    elif be16(ent, 0) == 0x8001:
        read16 = be16
    else:
        return assets, ["DM2 ENT1 marker missing"]
    entry_count = read16(ent, 2)
    group_count = read16(ent, 4)
    if entry_count <= 0 or group_count <= 0 or 6 + group_count * 2 > len(ent):
        return assets, ["DM2 ENT1 has invalid counts"]
    ep_for = {ord("T"): 0, ord("I"): 1, ord("D"): 2, ord("S"): 3, ord("P"): 4, ord("F"): 5, ord("G"): 6}
    offsets = [None] * 7
    lengths = [0] * 7
    stride = 0
    pos = 6
    for _ in range(group_count):
        ep = ep_for.get(ent[pos])
        length = ent[pos + 1]
        if ep is not None:
            offsets[ep] = stride
            lengths[ep] = length
        stride += length
        pos += 2
    required = (0, 1, 2, 3, 4)
    if stride <= 0 or any(offsets[i] is None for i in required):
        return assets, ["DM2 ENT1 missing required T/I/D/S/P groups"]
    if pos + entry_count * stride > len(ent):
        return assets, ["DM2 ENT1 rows exceed raw[0]"]

    def read_group(row: bytes, ep: int) -> int:
        start = offsets[ep]
        if start is None:
            return 0
        value = 0
        for b in row[start : start + lengths[ep]]:
            value = (value << 8) | b
        return value

    for i in range(entry_count):
        row = ent[pos + i * stride : pos + (i + 1) * stride]
        cls1 = read_group(row, 0)
        cls2 = read_group(row, 1)
        cls3 = read_group(row, 2)
        cls4 = read_group(row, 3)
        data_index = read_group(row, 4) & 0x7FFF
        if data_index >= raw_count:
            warning = "GDAT entry data index exceeds raw table"
            raw = b""
            raw_offset = 0
            raw_size = 0
            width = 0
            height = 0
        else:
            raw_offset = raw_offsets[data_index]
            raw_size = raw_sizes[data_index]
            raw = data[raw_offset : raw_offset + raw_size]
            width, height, _w4, _offset_y = dm2_record_dimensions(raw) if raw_size >= 10 and cls3 == 1 else (0, 0, 0, 0)
            warning = "metadata only; non-image or unsupported GDAT payload"
            if cls3 == 1 and width and height:
                try:
                    _img, decode_kind = dm2_decode_gdat_image(raw)
                    warning = "" if decode_kind != "IMG9-C8" else "metadata only; IMG9 preview not implemented"
                except Exception as exc:
                    warning = f"metadata only; GDAT decode warning: {exc}"
        assets.append(
            GameDataAsset(
                category=dm2_category_to_artpack_category(cls1),
                asset_id=f"gdat_c{cls1:02x}_i{cls2:02x}_t{cls3:02x}_f{cls4:02x}_{i:04d}",
                path=path,
                width=width,
                height=height,
                source_game="dm2",
                source_kind="DM2-GDAT",
                index=i,
                offset=raw_offset,
                compressed_bytes=raw_size,
                decompressed_bytes=width * height if width and height else raw_size,
                sha256=hashlib.sha256(raw).hexdigest() if raw else "",
                warning=warning,
            )
        )
    warnings.append(f"DM2 GDAT raw records: {raw_count}; ENT1 rows: {entry_count}")
    return assets, warnings


def import_game_data_file(path: Path) -> GameDataImportResult:
    path = path.expanduser().resolve()
    data = path.read_bytes()
    sha = hashlib.sha256(data).hexdigest()
    known = parse_verified_hashes().get(sha)
    detected_variant = known[0] if known else "unknown"
    detected_game = game_from_variant(detected_variant)
    warnings: list[str] = []
    if known and known[2] != len(data):
        warnings.append(f"verified hash row size {known[2]} differs from file size {len(data)}")
    if not known:
        lower = path.name.lower()
        if "graphics" in lower or lower.endswith(".dat"):
            if len(data) > 2_000_000:
                detected_game = "dm2"
            elif len(data) > 380_000:
                detected_game = "csb"
            else:
                detected_game = "dm1"
            detected_variant = f"{detected_game}-unverified"
            warnings.append("unknown SHA256; game inferred from filename/size only")
    assets: list[GameDataAsset] = []
    if detected_game == "dm2" and path.name.lower().endswith(".dat"):
        parsed, parse_warnings = dm2_parse_gdat_assets(path, data)
        assets.extend(parsed)
        warnings.extend(parse_warnings)
    elif path.name.lower().endswith(".dat") and ("graphics" in path.name.lower() or known):
        parsed, parse_warnings = parse_graphics_dat_assets(path, data, detected_game)
        assets.extend(parsed)
        warnings.extend(parse_warnings)
    if not assets and path.suffix.lower() in IMAGE_EXTENSIONS:
        with Image.open(path) as img:
            assets.append(
                GameDataAsset(
                    category=infer_category(path, detected_game),
                    asset_id=path.stem,
                    path=path,
                    width=img.width,
                    height=img.height,
                    source_game=detected_game,
                    source_kind="image",
                    index=0,
                    offset=0,
                    compressed_bytes=len(data),
                    decompressed_bytes=img.width * img.height,
                    sha256=sha,
                )
            )
    if not assets:
        warnings.append("no supported graphical assets found in file")
    return GameDataImportResult(path, detected_game, detected_variant, sha, len(data), assets, warnings)


def scan_reference_assets(game: str, root: Path) -> list[ReferenceAsset]:
    root = root.expanduser()
    if not root.exists():
        return []
    out: list[ReferenceAsset] = []
    for path in sorted(root.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in IMAGE_EXTENSIONS:
            continue
        try:
            with Image.open(path) as img:
                width, height = img.size
        except Exception:
            continue
        out.append(
            ReferenceAsset(
                category=infer_category(path.relative_to(root), game),
                asset_id=path.stem,
                path=path,
                width=width,
                height=height,
            )
        )
    return out


class Artpack:
    def __init__(self, game: str, root: Path):
        if game not in GAMES:
            raise ValueError(f"unsupported game: {game}")
        self.game = game
        self.root = root.expanduser().resolve()
        self.manifest_path = self.root / "modern_asset_manifest.json"
        self.data: dict[str, Any] = {}

    def load_or_create(self) -> None:
        self.root.mkdir(parents=True, exist_ok=True)
        if self.manifest_path.exists():
            with self.manifest_path.open("r", encoding="utf-8") as fp:
                loaded = json.load(fp)
            if not isinstance(loaded, dict):
                raise ValueError("modern_asset_manifest.json must be a JSON object")
            self.data = loaded
        else:
            self.data = {}
        self.data.setdefault("manifestVersion", "1.0.0")
        self.data.setdefault("packId", f"firestaff-{self.game}-v22-modern")
        self.data.setdefault("game", self.game)
        self.data.setdefault("tool", "firestaff_artpack_studio")
        for category in categories_for_game(self.game):
            self.data.setdefault(category, [])

    def save(self) -> None:
        self.root.mkdir(parents=True, exist_ok=True)
        tmp = self.manifest_path.with_suffix(".json.tmp")
        with tmp.open("w", encoding="utf-8") as fp:
            json.dump(self.data, fp, indent=2, ensure_ascii=False)
            fp.write("\n")
        tmp.replace(self.manifest_path)

    def entries(self) -> list[AssetEntry]:
        out: list[AssetEntry] = []
        for category in categories_for_game(self.game):
            raw = self.data.get(category, [])
            if not isinstance(raw, list):
                continue
            for entry in raw:
                if not isinstance(entry, dict):
                    continue
                asset_id = str(entry.get("id") or "").strip()
                if not asset_id:
                    continue
                out.append(
                    AssetEntry(
                        category=category,
                        asset_id=asset_id,
                        source_file=str(entry.get("source_file") or safe_asset_filename(asset_id)),
                        width=int(entry.get("width") or 0),
                        height=int(entry.get("height") or 0),
                        generator=str(entry.get("generator") or ""),
                        notes=str(entry.get("notes") or ""),
                    )
                )
        return out

    def find_raw_entry(self, category: str, asset_id: str) -> dict[str, Any] | None:
        raw = self.data.setdefault(category, [])
        if not isinstance(raw, list):
            raise ValueError(f"manifest category {category!r} must be a list")
        for entry in raw:
            if isinstance(entry, dict) and entry.get("id") == asset_id:
                return entry
        return None

    def upsert_asset(
        self,
        category: str,
        asset_id: str,
        image_path: Path,
        generator: str,
        notes: str = "",
    ) -> AssetEntry:
        if not category:
            raise ValueError("category is required")
        if not asset_id:
            raise ValueError("asset id is required")
        img = ensure_rgba(Image.open(image_path))
        out_dir = self.root / category
        out_dir.mkdir(parents=True, exist_ok=True)
        source_file = safe_asset_filename(asset_id)
        out_path = out_dir / source_file
        img.save(out_path)
        raw = self.data.setdefault(category, [])
        if not isinstance(raw, list):
            raise ValueError(f"manifest category {category!r} must be a list")
        entry = self.find_raw_entry(category, asset_id)
        if entry is None:
            entry = {"id": asset_id}
            raw.append(entry)
        entry.update(
            {
                "id": asset_id,
                "source_file": source_file,
                "width": img.width,
                "height": img.height,
                "generator": generator or "operator_import",
                "updatedAtUtc": utc_now(),
            }
        )
        if notes:
            entry["notes"] = notes
        self.save()
        return AssetEntry(category, asset_id, source_file, img.width, img.height, entry["generator"], notes)

    def asset_path(self, entry: AssetEntry) -> Path:
        return self.root / entry.category / entry.source_file

    def required_slots(self) -> list[tuple[str, str, str]]:
        return list(DEFAULT_REQUIRED.get(self.game, []))

    def validate_required(self) -> list[str]:
        errors: list[str] = []
        for category, asset_id, default_file in self.required_slots():
            raw = self.find_raw_entry(category, asset_id)
            if raw is None:
                errors.append(f"{category}/{asset_id}: missing manifest entry")
                continue
            generator = str(raw.get("generator") or "")
            if not generator or generator == "placeholder":
                errors.append(f"{category}/{asset_id}: generator is not real")
            source_file = str(raw.get("source_file") or default_file)
            path = self.root / category / source_file
            if not path.exists():
                errors.append(f"{category}/{asset_id}: missing file {path}")
                continue
            try:
                img = Image.open(path)
            except Exception as exc:
                errors.append(f"{category}/{asset_id}: cannot read image: {exc}")
                continue
            if int(raw.get("width") or 0) != img.width or int(raw.get("height") or 0) != img.height:
                errors.append(f"{category}/{asset_id}: manifest size does not match image")
        return errors

    def write_finish_receipt(self, reviewer: str) -> Path:
        self.save()
        required = self.required_slots()
        if not required:
            reviewed = [entry.asset_id for entry in self.entries()]
        else:
            errors = self.validate_required()
            if errors:
                raise ValueError("Cannot write finish_receipt.json:\n" + "\n".join(errors))
            reviewed = [asset_id for _, asset_id, _ in required]
        receipt = {
            "receiptVersion": "1.0.0",
            "manifestPath": str(self.manifest_path),
            "manifestHashFnv1a": f"{fnv1a32(self.manifest_path.read_bytes()):08x}",
            "reviewer": reviewer or os.environ.get("USER", "operator"),
            "reviewedAtUtc": utc_now(),
            "gateTarget": "FINISHED_REAL",
            "reviewedSlots": reviewed,
            "notes": "Generated by Firestaff V2.2 Artpack Studio after local validation.",
        }
        out = self.root / "finish_receipt.json"
        tmp = out.with_suffix(".json.tmp")
        with tmp.open("w", encoding="utf-8") as fp:
            json.dump(receipt, fp, indent=2)
            fp.write("\n")
        tmp.replace(out)
        return out

    def export_fsart(self, archive_path: Path) -> Path:
        self.save()
        archive_path = archive_path.expanduser()
        if archive_path.suffix.lower() != FSART_SUFFIX:
            archive_path = archive_path.with_suffix(FSART_SUFFIX)
        archive_path.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(archive_path, "w", compression=zipfile.ZIP_DEFLATED) as zf:
            zf.write(self.manifest_path, "modern_asset_manifest.json")
            receipt = self.root / "finish_receipt.json"
            if receipt.exists():
                zf.write(receipt, "finish_receipt.json")
            for entry in self.entries():
                path = self.asset_path(entry)
                if path.exists():
                    zf.write(path, f"{entry.category}/{entry.source_file}")
        return archive_path

    def import_fsart(self, archive_path: Path) -> None:
        archive_path = archive_path.expanduser()
        with zipfile.ZipFile(archive_path, "r") as zf:
            names = zf.namelist()
            if "modern_asset_manifest.json" not in names:
                raise ValueError(".fsart archive is missing modern_asset_manifest.json")
            for name in names:
                candidate = Path(name)
                if candidate.is_absolute() or ".." in candidate.parts:
                    raise ValueError(f"Unsafe archive path: {name}")
            self.root.mkdir(parents=True, exist_ok=True)
            zf.extractall(self.root)
        self.load_or_create()
        if str(self.data.get("game") or self.game) != self.game:
            raise ValueError(f".fsart game mismatch: expected {self.game}, got {self.data.get('game')}")
        self.save()


class PixelCanvas(ttk.Frame):
    def __init__(self, master: tk.Misc, title: str, editable: bool):
        super().__init__(master)
        self.title = title
        self.editable = editable
        self.image: Image.Image | None = None
        self.photo: ImageTk.PhotoImage | None = None
        self.zoom = tk.IntVar(value=2)
        self.tool = tk.StringVar(value="pencil")
        self.brush = tk.IntVar(value=1)
        self.color = "#00ffff"
        self.path: Path | None = None
        self._dragging = False

        top = ttk.Frame(self)
        top.pack(fill="x")
        ttk.Label(top, text=title).pack(side="left")
        ttk.Label(top, text="Zoom").pack(side="left", padx=(12, 2))
        ttk.Spinbox(top, from_=1, to=16, width=4, textvariable=self.zoom, command=self.render).pack(side="left")
        if editable:
            ttk.Label(top, text="Brush").pack(side="left", padx=(12, 2))
            ttk.Spinbox(top, from_=1, to=32, width=4, textvariable=self.brush).pack(side="left")
            ttk.Button(top, text="Color", command=self.choose_color).pack(side="left", padx=4)
            ttk.Button(top, text="Pencil", command=lambda: self.tool.set("pencil")).pack(side="left")
            ttk.Button(top, text="Pick", command=lambda: self.tool.set("pick")).pack(side="left")
            ttk.Button(top, text="Fill", command=lambda: self.tool.set("fill")).pack(side="left")

        self.canvas = tk.Canvas(self, width=512, height=384, background="#202020", highlightthickness=0)
        self.canvas.pack(fill="both", expand=True)
        self.canvas.bind("<Button-1>", self.on_down)
        self.canvas.bind("<B1-Motion>", self.on_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_up)
        self.zoom.trace_add("write", lambda *_: self.render())

    def load(self, path: Path) -> None:
        self.path = path
        self.image = ensure_rgba(Image.open(path))
        self.render()

    def set_image(self, img: Image.Image, path: Path | None = None) -> None:
        self.path = path
        self.image = ensure_rgba(img.copy())
        self.render()

    def choose_color(self) -> None:
        chosen = colorchooser.askcolor(self.color, parent=self)
        if chosen and chosen[1]:
            self.color = chosen[1]

    def render(self) -> None:
        self.canvas.delete("all")
        if self.image is None:
            self.canvas.create_text(20, 20, anchor="nw", fill="#aaaaaa", text="No image")
            return
        z = max(1, int(self.zoom.get() or 1))
        view = self.image.resize((self.image.width * z, self.image.height * z), Image.Resampling.NEAREST)
        self.photo = ImageTk.PhotoImage(view)
        self.canvas.create_image(0, 0, anchor="nw", image=self.photo)
        self.canvas.config(scrollregion=(0, 0, view.width, view.height))
        if z >= 8:
            self.draw_grid(view.width, view.height, z)

    def draw_grid(self, w: int, h: int, z: int) -> None:
        for x in range(0, w + 1, z):
            self.canvas.create_line(x, 0, x, h, fill="#333333")
        for y in range(0, h + 1, z):
            self.canvas.create_line(0, y, w, y, fill="#333333")

    def canvas_to_pixel(self, event: tk.Event) -> tuple[int, int] | None:
        if self.image is None:
            return None
        z = max(1, int(self.zoom.get() or 1))
        x = int(self.canvas.canvasx(event.x) // z)
        y = int(self.canvas.canvasy(event.y) // z)
        if x < 0 or y < 0 or x >= self.image.width or y >= self.image.height:
            return None
        return x, y

    def on_down(self, event: tk.Event) -> None:
        if not self.editable:
            return
        self._dragging = True
        self.apply_tool(event)

    def on_drag(self, event: tk.Event) -> None:
        if self.editable and self._dragging and self.tool.get() == "pencil":
            self.apply_tool(event)

    def on_up(self, _event: tk.Event) -> None:
        self._dragging = False

    def apply_tool(self, event: tk.Event) -> None:
        pos = self.canvas_to_pixel(event)
        if pos is None or self.image is None:
            return
        x, y = pos
        tool = self.tool.get()
        if tool == "pick":
            rgba = self.image.getpixel((x, y))
            self.color = "#%02x%02x%02x" % rgba[:3]
            return
        if tool == "fill":
            self.flood_fill(x, y, ImageColor.getcolor(self.color, "RGBA"))
        else:
            draw = ImageDraw.Draw(self.image)
            b = max(1, int(self.brush.get() or 1))
            rgba = ImageColor.getcolor(self.color, "RGBA")
            draw.rectangle((x, y, x + b - 1, y + b - 1), fill=rgba)
        self.render()

    def flood_fill(self, x: int, y: int, color: tuple[int, int, int, int]) -> None:
        if self.image is None:
            return
        target = self.image.getpixel((x, y))
        if target == color:
            return
        stack = [(x, y)]
        px = self.image.load()
        w, h = self.image.size
        while stack:
            cx, cy = stack.pop()
            if cx < 0 or cy < 0 or cx >= w or cy >= h:
                continue
            if px[cx, cy] != target:
                continue
            px[cx, cy] = color
            stack.extend(((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)))


class ArtpackStudio(tk.Tk):
    def __init__(self, initial_game: str, initial_root: Path | None):
        super().__init__()
        self.title("Firestaff V2.2 Artpack Studio")
        self.geometry("1280x820")
        self.minsize(1000, 680)
        self.game = tk.StringVar(value=initial_game)
        self.root = tk.StringVar(value=str(initial_root or default_modern_dir(initial_game)))
        self.reference_root = tk.StringVar(value=str(default_reference_dir(initial_game)))
        self.category = tk.StringVar(value="wall_shapes")
        self.asset_id = tk.StringVar(value="wall_d3_carved_hero_01")
        self.generator = tk.StringVar(value="operator_import")
        self.ai_command = tk.StringVar(value=os.environ.get("FIRESTAFF_ARTPACK_AI_COMMAND", ""))
        self.status = tk.StringVar(value="Ready")
        self.stats = tk.StringVar(value="No game data imported")
        self.pack: Artpack | None = None
        self.reference_assets: list[ReferenceAsset] = []
        self.game_data_result: GameDataImportResult | None = None
        self.game_data_assets: list[GameDataAsset] = []
        self.asset_rows: list[ReferenceAsset | AssetEntry | GameDataAsset] = []
        self.source_path: Path | None = None
        self.target_path: Path | None = None
        self.prompt_extra = tk.StringVar(value="")
        self.watermark_photo: ImageTk.PhotoImage | None = None
        self._build_ui()
        self.enable_drag_and_drop()
        self.open_pack()

    def _build_ui(self) -> None:
        top = ttk.Frame(self, padding=8)
        top.pack(fill="x")
        ttk.Label(top, text="Game").pack(side="left")
        game_box = ttk.Combobox(top, textvariable=self.game, values=GAMES, width=8, state="readonly")
        game_box.pack(side="left", padx=4)
        game_box.bind("<<ComboboxSelected>>", lambda _e: self.on_game_changed())
        ttk.Label(top, text="Pack").pack(side="left", padx=(12, 2))
        ttk.Entry(top, textvariable=self.root, width=70).pack(side="left", fill="x", expand=True)
        ttk.Button(top, text="Browse", command=self.browse_pack).pack(side="left", padx=4)
        ttk.Button(top, text="Open/Create", command=self.open_pack).pack(side="left")
        ttk.Button(top, text="Validate", command=self.validate_pack).pack(side="left", padx=4)
        ttk.Button(top, text="Write Receipt", command=self.write_receipt).pack(side="left")
        ttk.Button(top, text="Import .fsart", command=self.import_fsart_dialog).pack(side="left", padx=(10, 4))
        ttk.Button(top, text="Export .fsart", command=self.export_fsart_dialog).pack(side="left")

        main = ttk.PanedWindow(self, orient="horizontal")
        main.pack(fill="both", expand=True, padx=8, pady=8)

        left = tk.Frame(main, width=360, background="#101214")
        main.add(left, weight=0)
        self.watermark = tk.Label(left, background="#101214", borderwidth=0)
        self.watermark.place(x=8, y=42)
        self.load_watermark()
        asset_header = ttk.Frame(left)
        asset_header.pack(fill="x", padx=6, pady=(6, 2))
        ttk.Label(asset_header, text="V1 assets / V2.2 targets").pack(side="left")
        ttk.Button(asset_header, text="Rescan", command=self.rescan_reference_assets).pack(side="right")
        ref = ttk.Frame(left)
        ref.pack(fill="x", padx=6, pady=(0, 4))
        ttk.Label(ref, text="V1 root").pack(side="left")
        ttk.Entry(ref, textvariable=self.reference_root, width=28).pack(side="left", fill="x", expand=True, padx=4)
        ttk.Button(ref, text="Browse", command=self.browse_reference_root).pack(side="left")
        data_buttons = ttk.Frame(left)
        data_buttons.pack(fill="x", padx=6, pady=(0, 4))
        ttk.Button(data_buttons, text="Import game data", command=self.import_game_data_dialog).pack(side="left", fill="x", expand=True)
        ttk.Button(data_buttons, text="Warnings", command=self.show_game_data_warnings).pack(side="left", padx=(4, 0))
        ttk.Label(left, textvariable=self.stats, background="#101214", foreground="#b8c6ca", wraplength=330).pack(
            fill="x", padx=6, pady=(0, 4)
        )
        self.asset_list = tk.Listbox(
            left,
            height=22,
            font=("Menlo", 11),
            background="#15191d",
            foreground="#d9e4e6",
            selectbackground="#225b62",
            borderwidth=0,
            highlightthickness=1,
            highlightbackground="#2f3a40",
        )
        self.asset_list.pack(fill="both", expand=True, padx=6, pady=(0, 6))
        self.asset_list.bind("<<ListboxSelect>>", lambda _e: self.on_asset_selected())

        form = ttk.LabelFrame(left, text="Current asset", padding=6)
        form.pack(fill="x", pady=8)
        ttk.Label(form, text="Category").grid(row=0, column=0, sticky="w")
        self.category_box = ttk.Combobox(form, textvariable=self.category, values=categories_for_game(self.game.get()), width=28)
        self.category_box.grid(row=0, column=1, sticky="ew")
        ttk.Label(form, text="Asset id").grid(row=1, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.asset_id).grid(row=1, column=1, sticky="ew")
        ttk.Label(form, text="Generator").grid(row=2, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.generator).grid(row=2, column=1, sticky="ew")
        form.columnconfigure(1, weight=1)

        buttons = ttk.Frame(left)
        buttons.pack(fill="x")
        ttk.Button(buttons, text="Load V1/ref", command=self.load_source).pack(fill="x", pady=2)
        ttk.Button(buttons, text="Load V2.2 target", command=self.load_target).pack(fill="x", pady=2)
        ttk.Button(buttons, text="Import target to pack", command=self.import_target).pack(fill="x", pady=2)
        ttk.Button(buttons, text="Save edited target", command=self.save_edited_target).pack(fill="x", pady=2)

        ai = ttk.LabelFrame(left, text="AI generation hook", padding=6)
        ai.pack(fill="x", pady=8)
        ttk.Entry(ai, textvariable=self.prompt_extra).pack(fill="x", pady=(0, 2))
        ttk.Entry(ai, textvariable=self.ai_command).pack(fill="x")
        ttk.Button(ai, text="Write prompt", command=self.write_prompt_only).pack(fill="x", pady=2)
        ttk.Button(ai, text="Run AI command", command=self.run_ai_command).pack(fill="x", pady=2)
        batch_ai = ttk.Frame(ai)
        batch_ai.pack(fill="x")
        ttk.Button(batch_ai, text="AI selected", command=self.run_ai_command).pack(side="left", fill="x", expand=True)
        ttk.Button(batch_ai, text="AI missing/all", command=self.run_ai_batch).pack(side="left", fill="x", expand=True, padx=(4, 0))

        right = ttk.PanedWindow(main, orient="vertical")
        main.add(right, weight=1)
        canvases = ttk.PanedWindow(right, orient="horizontal")
        right.add(canvases, weight=1)
        self.source_canvas = PixelCanvas(canvases, "V1/reference", editable=False)
        self.target_canvas = PixelCanvas(canvases, "V2.2 target/editor", editable=True)
        canvases.add(self.source_canvas, weight=1)
        canvases.add(self.target_canvas, weight=1)

        log_frame = ttk.Frame(right)
        right.add(log_frame, weight=0)
        self.log = tk.Text(log_frame, height=8, wrap="word")
        self.log.pack(fill="both", expand=True)

        status = ttk.Label(self, textvariable=self.status, anchor="w")
        status.pack(fill="x", padx=8, pady=(0, 8))

    def log_line(self, msg: str) -> None:
        self.log.insert("end", msg + "\n")
        self.log.see("end")
        self.status.set(msg)

    def enable_drag_and_drop(self) -> None:
        try:
            self.tk.call("package", "require", "tkdnd")
            for widget in (self, self.asset_list, self.target_canvas.canvas):
                widget.tk.call("tkdnd::drop_target", "register", widget, "DND_Files")
                widget.bind("<<Drop>>", self.on_drop_file)
            self.log_line("Drag-and-drop enabled")
        except Exception:
            pass

    def on_drop_file(self, event: tk.Event) -> None:
        raw = getattr(event, "data", "")
        if not raw:
            return
        first = self.tk.splitlist(raw)[0]
        path = Path(first)
        if path.suffix.lower() == FSART_SUFFIX:
            if self.pack:
                self.pack.import_fsart(path)
                self.refresh_asset_list()
                self.log_line(f"Imported .fsart: {path}")
            return
        if path.suffix.lower() in IMAGE_EXTENSIONS:
            self.target_path = path
            self.target_canvas.load(path)
            self.log_line(f"Loaded dropped target: {path}")
            return
        self.import_game_data(path)

    def on_game_changed(self) -> None:
        self.root.set(str(default_modern_dir(self.game.get())))
        self.reference_root.set(str(default_reference_dir(self.game.get())))
        self.category_box.configure(values=categories_for_game(self.game.get()))
        self.category.set(categories_for_game(self.game.get())[0])
        required = DEFAULT_REQUIRED.get(self.game.get(), [])
        if required:
            self.category.set(required[0][0])
            self.asset_id.set(required[0][1])
        self.open_pack()

    def load_watermark(self) -> None:
        if not FIRESTAFF_LOGO.exists():
            return
        try:
            img = ensure_rgba(Image.open(FIRESTAFF_LOGO))
            max_w, max_h = 160, 220
            scale = min(max_w / img.width, max_h / img.height, 1.0)
            img = img.resize((max(1, int(img.width * scale)), max(1, int(img.height * scale))), Image.Resampling.LANCZOS)
            alpha = img.getchannel("A").point(lambda v: int(v * 0.18))
            img.putalpha(alpha)
            self.watermark_photo = ImageTk.PhotoImage(img)
            self.watermark.configure(image=self.watermark_photo)
        except Exception:
            self.watermark_photo = None

    def browse_pack(self) -> None:
        selected = filedialog.askdirectory(title="Select V2.2 modern artpack directory")
        if selected:
            self.root.set(selected)
            self.open_pack()

    def browse_reference_root(self) -> None:
        selected = filedialog.askdirectory(title="Select V1/reference asset root")
        if selected:
            self.reference_root.set(selected)
            self.rescan_reference_assets()

    def open_pack(self) -> None:
        try:
            self.pack = Artpack(self.game.get(), Path(self.root.get()))
            self.pack.load_or_create()
            self.pack.save()
            self.rescan_reference_assets(log=False)
            self.refresh_asset_list()
            self.log_line(f"Opened {self.pack.game} artpack: {self.pack.root}")
        except Exception as exc:
            messagebox.showerror("Open artpack failed", str(exc), parent=self)

    def refresh_asset_list(self) -> None:
        self.asset_list.delete(0, "end")
        self.asset_rows = []
        if not self.pack:
            return
        target_by_key = {(entry.category, entry.asset_id): entry for entry in self.pack.entries()}
        seen: set[tuple[str, str]] = set()
        for ref in self.reference_assets:
            key = (ref.category, ref.asset_id)
            seen.add(key)
            target = target_by_key.get(key)
            status = "OK" if target else "--"
            dims = f"{ref.width}x{ref.height}"
            target_dims = f"{target.width}x{target.height}" if target else "missing"
            self.asset_rows.append(ref)
            self.asset_list.insert(
                "end",
                f"{status:2} V1 {ref.category}/{ref.asset_id} {dims:>9}  | V2.2 {target_dims}",
            )
        for gd in self.game_data_assets:
            key = (gd.category, gd.asset_id)
            seen.add(key)
            target = target_by_key.get(key)
            status = "OK" if target else "!!" if gd.warning else "--"
            dims = f"{gd.width}x{gd.height}"
            target_dims = f"{target.width}x{target.height}" if target else "missing"
            self.asset_rows.append(gd)
            self.asset_list.insert(
                "end",
                f"{status:2} DAT {gd.category}/{gd.asset_id} {dims:>9}  | V2.2 {target_dims}",
            )
        for entry in self.pack.entries():
            key = (entry.category, entry.asset_id)
            if key in seen:
                continue
            self.asset_rows.append(entry)
            self.asset_list.insert(
                "end",
                f"OK V1 {'missing':<34} | V2.2 {entry.category}/{entry.asset_id} {entry.width}x{entry.height}",
            )

    def rescan_reference_assets(self, log: bool = True) -> None:
        self.reference_assets = scan_reference_assets(self.game.get(), Path(self.reference_root.get()))
        self.refresh_asset_list()
        if log:
            self.log_line(f"Scanned {len(self.reference_assets)} V1/reference assets")

    def on_asset_selected(self) -> None:
        if not self.pack:
            return
        sel = self.asset_list.curselection()
        if not sel:
            return
        entries = self.pack.entries()
        if sel[0] >= len(self.asset_rows):
            return
        row = self.asset_rows[sel[0]]
        self.category.set(row.category)
        self.asset_id.set(row.asset_id)
        if isinstance(row, ReferenceAsset):
            self.source_path = row.path
            self.source_canvas.load(row.path)
            target = self.pack.find_raw_entry(row.category, row.asset_id)
            if target is not None:
                entry = AssetEntry(
                    row.category,
                    row.asset_id,
                    str(target.get("source_file") or safe_asset_filename(row.asset_id)),
                    int(target.get("width") or 0),
                    int(target.get("height") or 0),
                    str(target.get("generator") or ""),
                    str(target.get("notes") or ""),
                )
                path = self.pack.asset_path(entry)
                if path.exists():
                    self.target_path = path
                    self.target_canvas.load(path)
            return
        if isinstance(row, GameDataAsset):
            self.source_path = row.path
            img = self.decode_game_data_asset(row)
            if img is not None:
                self.source_canvas.set_image(img, row.path)
            else:
                self.source_canvas.set_image(self.metadata_image(row), row.path)
            target = self.pack.find_raw_entry(row.category, row.asset_id)
            if target is not None:
                entry = AssetEntry(
                    row.category,
                    row.asset_id,
                    str(target.get("source_file") or safe_asset_filename(row.asset_id)),
                    int(target.get("width") or 0),
                    int(target.get("height") or 0),
                    str(target.get("generator") or ""),
                    str(target.get("notes") or ""),
                )
                path = self.pack.asset_path(entry)
                if path.exists():
                    self.target_path = path
                    self.target_canvas.load(path)
            if row.warning:
                self.log_line(f"{row.asset_id}: {row.warning}")
            return
        entry = row
        self.generator.set(entry.generator or "operator_import")
        path = self.pack.asset_path(entry)
        if path.exists():
            self.target_path = path
            self.target_canvas.load(path)

    def load_source(self) -> None:
        path = filedialog.askopenfilename(
            title="Load V1/reference image",
            filetypes=[("Images", "*.png *.bmp *.gif *.jpg *.jpeg *.tga *.webp"), ("All files", "*.*")],
        )
        if path:
            self.source_path = Path(path)
            self.source_canvas.load(self.source_path)
            self.log_line(f"Loaded reference: {self.source_path}")

    def load_target(self) -> None:
        path = filedialog.askopenfilename(
            title="Load V2.2 target image",
            filetypes=[("Images", "*.png *.bmp *.gif *.jpg *.jpeg *.tga *.webp"), ("All files", "*.*")],
        )
        if path:
            self.target_path = Path(path)
            self.target_canvas.load(self.target_path)
            self.log_line(f"Loaded target: {self.target_path}")

    def import_game_data_dialog(self) -> None:
        path = filedialog.askopenfilename(
            title="Import original game data",
            filetypes=[
                ("Game data", "*.dat *.DAT *.bin *.BIN *.iso *.ISO *.cue *.CUE"),
                ("Images", "*.png *.bmp *.gif *.jpg *.jpeg *.tga *.webp"),
                ("All files", "*.*"),
            ],
        )
        if path:
            self.import_game_data(Path(path))

    def import_game_data(self, path: Path) -> None:
        try:
            result = import_game_data_file(path)
            self.game_data_result = result
            self.game_data_assets = result.assets
            if result.detected_game in GAMES and result.detected_game != self.game.get():
                self.game.set(result.detected_game)
                self.root.set(str(default_modern_dir(result.detected_game)))
                self.reference_root.set(str(default_reference_dir(result.detected_game)))
                self.category_box.configure(values=categories_for_game(result.detected_game))
                self.pack = Artpack(result.detected_game, Path(self.root.get()))
                self.pack.load_or_create()
                self.pack.save()
            self.refresh_asset_list()
            warning_count = sum(1 for asset in result.assets if asset.warning) + len(result.warnings)
            self.stats.set(
                f"{result.detected_variant}: {len(result.assets)} assets, "
                f"{warning_count} warnings, {result.file_size:,} bytes"
            )
            self.log_line(f"Imported game data: {result.path} ({result.detected_variant})")
            if result.warnings:
                self.log_line("Warnings: " + "; ".join(result.warnings[:4]))
        except Exception as exc:
            messagebox.showerror("Game data import failed", str(exc), parent=self)

    def show_game_data_warnings(self) -> None:
        if not self.game_data_result:
            messagebox.showinfo("Warnings", "No game data imported.", parent=self)
            return
        lines = list(self.game_data_result.warnings)
        lines.extend(f"{asset.asset_id}: {asset.warning}" for asset in self.game_data_assets if asset.warning)
        if not lines:
            lines = ["No warnings."]
        messagebox.showwarning("Game data warnings", "\n".join(lines[:80]), parent=self)

    def decode_game_data_asset(self, asset: GameDataAsset) -> Image.Image | None:
        if asset.source_kind.startswith("GRAPHICS.DAT"):
            data = asset.path.read_bytes()
            record = data[asset.offset : asset.offset + asset.compressed_bytes]
            try:
                return expand_img3_to_image(record, asset.width, asset.height)
            except Exception:
                return None
        if asset.source_kind == "DM2-GDAT":
            data = asset.path.read_bytes()
            record = data[asset.offset : asset.offset + asset.compressed_bytes]
            try:
                image, _kind = dm2_decode_gdat_image(record)
                return image
            except Exception:
                return None
        if asset.source_kind == "image":
            return ensure_rgba(Image.open(asset.path))
        return None

    def metadata_image(self, asset: GameDataAsset) -> Image.Image:
        img = Image.new("RGBA", (max(320, asset.width or 320), max(160, asset.height or 160)), "#202020")
        draw = ImageDraw.Draw(img)
        lines = [
            f"{asset.source_game} {asset.source_kind}",
            asset.asset_id,
            f"{asset.width}x{asset.height} offset={asset.offset}",
            f"compressed={asset.compressed_bytes} decompressed={asset.decompressed_bytes}",
            asset.warning or "metadata only",
        ]
        y = 14
        for line in lines:
            draw.text((14, y), line[:90], fill="#d9e4e6")
            y += 22
        return img

    def import_target(self) -> None:
        if not self.pack:
            return
        if self.target_canvas.image is None:
            messagebox.showwarning("No target image", "Load or generate a V2.2 target first.", parent=self)
            return
        with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp:
            tmp_path = Path(tmp.name)
        try:
            self.target_canvas.image.save(tmp_path)
            entry = self.pack.upsert_asset(
                self.category.get().strip(),
                self.asset_id.get().strip(),
                tmp_path,
                self.generator.get().strip() or "operator_import",
                "Imported by Firestaff Artpack Studio.",
            )
            self.target_path = self.pack.asset_path(entry)
            self.refresh_asset_list()
            self.log_line(f"Imported {entry.category}/{entry.asset_id}")
        except Exception as exc:
            messagebox.showerror("Import failed", str(exc), parent=self)
        finally:
            tmp_path.unlink(missing_ok=True)

    def save_edited_target(self) -> None:
        if self.target_canvas.image is None:
            return
        if self.target_path is None:
            path = filedialog.asksaveasfilename(defaultextension=".png", filetypes=[("PNG", "*.png")])
            if not path:
                return
            self.target_path = Path(path)
        self.target_canvas.image.save(self.target_path)
        self.log_line(f"Saved edited image: {self.target_path}")

    def current_prompt(self) -> str:
        source_note = str(self.source_path) if self.source_path else "no reference image loaded"
        target_size = "same as reference"
        if self.source_canvas.image:
            target_size = f"{self.source_canvas.image.width}x{self.source_canvas.image.height}"
        extra = self.prompt_extra.get().strip()
        prompt = (
            f"Create Firestaff V2.2 modern art for {self.game.get()}.\n"
            f"Category: {self.category.get()}\n"
            f"Asset id: {self.asset_id.get()}\n"
            f"Target size: {target_size}\n"
            f"Reference: {source_note}\n"
            "Style requirements: preserve gameplay readability, clear pixel silhouettes, "
            "no text overlays, no UI labels unless the source asset contains them, "
            "transparent background only when the reference uses transparency, "
            "and no copyrighted replacement from another game.\n"
        )
        if extra:
            prompt += f"Custom prompt: {extra}\n"
        return prompt

    def write_prompt_only(self) -> None:
        path = filedialog.asksaveasfilename(defaultextension=".txt", filetypes=[("Text", "*.txt")])
        if not path:
            return
        Path(path).write_text(self.current_prompt(), encoding="utf-8")
        self.log_line(f"Wrote AI prompt: {path}")

    def run_ai_command(self) -> None:
        command = self.ai_command.get().strip()
        if not command:
            messagebox.showinfo(
                "AI command not configured",
                "Set FIRESTAFF_ARTPACK_AI_COMMAND or enter a command template first.",
                parent=self,
            )
            return
        with tempfile.TemporaryDirectory(prefix="firestaff-art-ai-") as td:
            tmp = Path(td)
            prompt_file = tmp / "prompt.txt"
            output = tmp / "generated.png"
            source = tmp / "source.png"
            prompt_file.write_text(self.current_prompt(), encoding="utf-8")
            if self.source_canvas.image:
                self.source_canvas.image.save(source)
            values = {
                "prompt_file": str(prompt_file),
                "output": str(output),
                "source": str(source if source.exists() else ""),
                "game": self.game.get(),
                "category": self.category.get(),
                "asset_id": self.asset_id.get(),
                "width": str(self.source_canvas.image.width if self.source_canvas.image else 512),
                "height": str(self.source_canvas.image.height if self.source_canvas.image else 512),
            }
            expanded = Template(command.replace("{", "${").replace("}", "}")).safe_substitute(values)
            try:
                subprocess.run(expanded, shell=True, check=True)
                if not output.exists():
                    raise RuntimeError(f"AI command did not create {output}")
                self.target_canvas.load(output)
                self.target_path = None
                self.generator.set("ai_command")
                self.log_line("AI command generated target image")
            except Exception as exc:
                messagebox.showerror("AI generation failed", str(exc), parent=self)

    def run_ai_batch(self) -> None:
        if not self.pack:
            return
        command = self.ai_command.get().strip()
        if not command:
            messagebox.showinfo("AI command not configured", "Enter an AI command template first.", parent=self)
            return
        rows = [row for row in self.asset_rows if isinstance(row, (ReferenceAsset, GameDataAsset))]
        target_keys = {(entry.category, entry.asset_id) for entry in self.pack.entries()}
        missing = [row for row in rows if (row.category, row.asset_id) not in target_keys]
        if not missing:
            missing = rows
        limit = simpledialog.askinteger(
            "AI batch",
            f"Generate how many assets? {len(missing)} available.",
            initialvalue=min(10, len(missing)),
            minvalue=1,
            maxvalue=max(1, len(missing)),
            parent=self,
        )
        if not limit:
            return
        generated = 0
        for row in missing[:limit]:
            self.category.set(row.category)
            self.asset_id.set(row.asset_id)
            if isinstance(row, ReferenceAsset):
                self.source_path = row.path
                self.source_canvas.load(row.path)
            else:
                img = self.decode_game_data_asset(row)
                self.source_path = row.path
                self.source_canvas.set_image(img if img else self.metadata_image(row), row.path)
            self.run_ai_command()
            if self.target_canvas.image is not None:
                self.import_target()
                generated += 1
        self.log_line(f"AI batch generated/imported {generated} assets")

    def validate_pack(self) -> None:
        if not self.pack:
            return
        errors = self.pack.validate_required()
        if errors:
            self.log_line("Validation failed:\n" + "\n".join(errors))
            messagebox.showwarning("Validation failed", "\n".join(errors), parent=self)
        else:
            self.log_line("Validation passed for required slots")
            messagebox.showinfo("Validation passed", "Required slots are complete.", parent=self)

    def write_receipt(self) -> None:
        if not self.pack:
            return
        reviewer = simpledialog.askstring("Reviewer", "Reviewer name:", parent=self) or os.environ.get("USER", "operator")
        try:
            path = self.pack.write_finish_receipt(reviewer)
            self.log_line(f"Wrote receipt: {path}")
        except Exception as exc:
            messagebox.showerror("Receipt failed", str(exc), parent=self)

    def export_fsart_dialog(self) -> None:
        if not self.pack:
            return
        path = filedialog.asksaveasfilename(
            title="Export Firestaff artpack",
            defaultextension=FSART_SUFFIX,
            filetypes=[("Firestaff artpack", f"*{FSART_SUFFIX}"), ("All files", "*.*")],
        )
        if not path:
            return
        try:
            out = self.pack.export_fsart(Path(path))
            self.log_line(f"Exported .fsart: {out}")
        except Exception as exc:
            messagebox.showerror("Export failed", str(exc), parent=self)

    def import_fsart_dialog(self) -> None:
        if not self.pack:
            return
        path = filedialog.askopenfilename(
            title="Import Firestaff artpack",
            filetypes=[("Firestaff artpack", f"*{FSART_SUFFIX}"), ("All files", "*.*")],
        )
        if not path:
            return
        try:
            self.pack.import_fsart(Path(path))
            self.refresh_asset_list()
            self.log_line(f"Imported .fsart: {path}")
        except Exception as exc:
            messagebox.showerror("Import failed", str(exc), parent=self)


def self_test() -> int:
    with tempfile.TemporaryDirectory(prefix="firestaff-artpack-studio-test-") as td:
        root = Path(td) / "assets" / "dm1" / "modern"
        pack = Artpack("dm1", root)
        pack.load_or_create()
        img_path = Path(td) / "wall.png"
        Image.new("RGBA", (8, 8), (12, 34, 56, 255)).save(img_path)
        pack.upsert_asset("wall_shapes", "wall_d3_carved_hero_01", img_path, "operator_import")
        assert pack.manifest_path.exists()
        assert pack.find_raw_entry("wall_shapes", "wall_d3_carved_hero_01") is not None
        errors = pack.validate_required()
        assert any("floor_plain_hero_01" in err for err in errors)
        for category, asset_id, filename in DM1_REQUIRED_SLOTS[1:]:
            p = Path(td) / filename
            Image.new("RGBA", (8, 8), (1, 2, 3, 255)).save(p)
            pack.upsert_asset(category, asset_id, p, "operator_import")
        assert pack.validate_required() == []
        receipt = pack.write_finish_receipt("self-test")
        assert receipt.exists()
        data = json.loads(receipt.read_text(encoding="utf-8"))
        assert data["gateTarget"] == "FINISHED_REAL"
        assert len(data["reviewedSlots"]) == len(DM1_REQUIRED_SLOTS)
        archive = Path(td) / "dm1-modern.fsart"
        pack.export_fsart(archive)
        assert archive.exists()
        imported = Artpack("dm1", Path(td) / "imported" / "dm1" / "modern")
        imported.load_or_create()
        imported.import_fsart(archive)
        assert imported.validate_required() == []
        refs = scan_reference_assets("dm1", root)
        assert any(ref.asset_id == "wall_d3_carved_hero_01" for ref in refs)
        graphics_dat = Path(td) / "GRAPHICS.DAT"
        graphics_dat.write_bytes(
            b"\x01\x80"  # new-format little-endian signature 0x8001
            b"\x01\x00"  # one entry
            b"\x08\x00"  # compressed bytes
            b"\x08\x00"  # decompressed bytes
            b"\x02\x00\x02\x00"  # width/height table
            b"\x02\x00\x02\x00\x01\x23\x45\x67"  # deliberately tiny IMG3-like record
        )
        imported_data = import_game_data_file(graphics_dat)
        assert imported_data.detected_game == "dm1"
        assert len(imported_data.assets) == 1
        assert imported_data.assets[0].asset_id == "graphics_0000"
        assert imported_data.assets[0].width == 2
        dm2_raw = (
            (2).to_bytes(2, "little")
            + (0x8001).to_bytes(2, "little")  # height 1, offsetY -32
            + (4).to_bytes(2, "little")
            + b"\x00\x00\x00\x00"
            + b"\x12"
        )
        dm2_img, dm2_kind = dm2_decode_gdat_image(dm2_raw)
        assert dm2_kind == "U4"
        assert dm2_img.size == (2, 1)
    print("firestaff_artpack_studio self-test: PASS")
    return 0


def render_demo_screenshot(out_path: Path, game: str) -> Path:
    out_path = out_path.expanduser().resolve()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    w, h = 1440, 900
    img = Image.new("RGB", (w, h), "#101214")
    draw = ImageDraw.Draw(img)
    if FIRESTAFF_LOGO.exists():
        try:
            logo = ensure_rgba(Image.open(FIRESTAFF_LOGO))
            scale = min(230 / logo.width, 310 / logo.height, 1.0)
            logo = logo.resize((max(1, int(logo.width * scale)), max(1, int(logo.height * scale))), Image.Resampling.LANCZOS)
            alpha = logo.getchannel("A").point(lambda v: int(v * 0.16))
            logo.putalpha(alpha)
            img.paste(logo, (24, 110), logo)
        except Exception:
            pass
    draw.rectangle((0, 0, w, 64), fill="#15191d")
    draw.text((24, 20), "Firestaff V2.2 Artpack Studio", fill="#e8f4f5")
    draw.rounded_rectangle((28, 82, 404, 840), radius=8, fill="#15191d", outline="#2f3a40")
    draw.text((44, 102), "V1 assets / V2.2 targets", fill="#e8f4f5")
    draw.rounded_rectangle((44, 134, 388, 166), radius=4, fill="#202830", outline="#41515a")
    draw.text((56, 143), f"V1 root  ~/.firestaff/data/{game}", fill="#b8c6ca")
    draw.rounded_rectangle((44, 174, 220, 206), radius=4, fill="#225b62", outline="#50bfc8")
    draw.text((58, 183), "Import game data", fill="#e8f4f5")
    draw.rounded_rectangle((230, 174, 388, 206), radius=4, fill="#202830", outline="#ffcc66")
    draw.text((244, 183), "Warnings", fill="#ffcc66")
    draw.text((52, 218), "dm1 GRAPHICS.DAT: 713 assets, 18 warnings", fill="#91d7df")
    rows = [
        ("OK", "DAT title_frames/graphics_0001", "320x200", "640x400"),
        ("--", "DAT wall_shapes/graphics_0107", "224x136", "missing"),
        ("!!", "DAT object_shapes/graphics_0144", "16x16", "decode warning"),
        ("OK", "V1 wall_shapes/wall_d3_carved_hero_01", "224x136", "224x136"),
        ("--", "DAT creature_shapes/graphics_0221", "48x56", "missing"),
    ]
    y = 250
    for status, name, v1_size, v22_size in rows:
        color = "#30d4a0" if status == "OK" else "#ffcc66"
        draw.text((52, y), status, fill=color)
        draw.text((86, y), f"V1 {name}", fill="#d9e4e6")
        draw.text((300, y), v1_size, fill="#91a4aa")
        draw.text((86, y + 22), f"V2.2 {v22_size}", fill="#91d7df")
        y += 58
    draw.rounded_rectangle((432, 82, 930, 682), radius=8, fill="#202020", outline="#3b474d")
    draw.rounded_rectangle((958, 82, 1412, 682), radius=8, fill="#202020", outline="#3b474d")
    draw.text((448, 102), "V1/reference", fill="#e8f4f5")
    draw.text((974, 102), "V2.2 target/editor", fill="#e8f4f5")
    for x0, y0, ww, hh, c1, c2 in (
        (472, 158, 392, 250, "#c8c2b5", "#5f5a52"),
        (1000, 158, 356, 250, "#9fd5da", "#293c47"),
    ):
        draw.rectangle((x0, y0, x0 + ww, y0 + hh), fill=c2)
        for i in range(0, ww, 16):
            draw.line((x0 + i, y0, x0 + i // 3, y0 + hh), fill=c1)
        draw.rectangle((x0 + 70, y0 + 48, x0 + ww - 70, y0 + hh - 32), outline="#111111", width=5)
    draw.rounded_rectangle((432, 704, 1412, 840), radius=8, fill="#15191d", outline="#2f3a40")
    draw.text((448, 724), "Opened dm1 artpack: ~/.firestaff/assets/dm1/modern", fill="#b8c6ca")
    draw.text((448, 754), "Export .fsart: shareable Firestaff artpack archive", fill="#91d7df")
    draw.text((448, 784), "Imported target image into selected manifest slot", fill="#b8c6ca")
    img.save(out_path)
    return out_path


def parse_args(argv: list[str]) -> argparse.Namespace:
    ap = argparse.ArgumentParser(description="Firestaff V2.2 Artpack Studio")
    ap.add_argument("--game", choices=GAMES, default="dm1")
    ap.add_argument("--pack-dir", type=Path)
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--screenshot", type=Path, help="Render a static UI screenshot preview and exit")
    ap.add_argument("--smoke-ui", action="store_true", help="Create and validate the GUI widget tree, then exit")
    return ap.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    if args.self_test:
        return self_test()
    if args.screenshot:
        out = render_demo_screenshot(args.screenshot, args.game)
        print(out)
        return 0
    app = ArtpackStudio(args.game, args.pack_dir)
    if args.smoke_ui:
        app.withdraw()
        app.update_idletasks()
        if not app.winfo_children() or not app.asset_list.winfo_exists():
            raise RuntimeError("Artpack Studio UI did not create its widget tree")
        app.destroy()
        print("Artpack Studio UI smoke test: PASS")
        return 0
    app.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
