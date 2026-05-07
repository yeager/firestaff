#!/usr/bin/env python3
"""Generate/verify pass302 DM1 GRAPHICS.DAT index manifest.

Bounded to pass304 wall-comparator GRAPHICS.DAT entries 93..107.
No image files are dumped; checksums are computed from the canonical source
record bytes and the deterministic IMG3 expanded buffers.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

PASS = "pass305_dm1_wall_graphics_93_107_manifest"
CANONICAL_GRAPHICS_DAT = Path(
    str(Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT")
)
EXPECTED_GRAPHICS_SHA256 = "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"
OUT_JSON = Path("parity-evidence/verification/pass305_dm1_wall_graphics_93_107_manifest.json")
TARGETS = {
    93: {
        "role": "wall.set0.index_93",
        "symbol": "C93_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    94: {
        "role": "wall.set0.index_94",
        "symbol": "C94_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    95: {
        "role": "wall.set0.index_95",
        "symbol": "C95_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    96: {
        "role": "wall.set0.index_96",
        "symbol": "C96_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    97: {
        "role": "wall.set0.index_97",
        "symbol": "C97_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    98: {
        "role": "wall.set0.index_98",
        "symbol": "C98_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    99: {
        "role": "wall.set0.index_99",
        "symbol": "C99_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    100: {
        "role": "wall.set0.index_100",
        "symbol": "C100_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    101: {
        "role": "wall.set0.index_101",
        "symbol": "C101_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    102: {
        "role": "wall.set0.index_102",
        "symbol": "C102_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    103: {
        "role": "wall.set0.index_103",
        "symbol": "C103_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    104: {
        "role": "wall.set0.index_104",
        "symbol": "C104_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    105: {
        "role": "wall.set0.index_105",
        "symbol": "C105_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    106: {
        "role": "wall.set0.index_106",
        "symbol": "C106_GRAPHIC_WALLSET_0",
        "nativeBitmap": "G2107_WallSet[...]",
        "viewportZone": "pass300 pixelRegion/layout-696 zone pending per render event",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    },
    107: {
        "role": "wall.front.blocking.d3c",
        "symbol": "C107_GRAPHIC_WALLSET_0_D3C",
        "nativeBitmap": "G2107_WallSet[C14_WALL_D3C]",
        "viewportZone": "C704_ZONE_WALL_D3C",
        "pass304Need": "wall comparator decoded byte/palette manifest for GRAPHICS.DAT wall graphics 93..107",
    }
}


def le16(data: bytes, offset: int) -> int:
    return data[offset] | (data[offset + 1] << 8)


class NibbleReader:
    def __init__(self, source: bytes) -> None:
        self.source = source
        self.nibble_offset = 8  # skip 4-byte width/height prefix

    def nibble(self) -> int:
        byte = self.source[self.nibble_offset >> 1]
        if self.nibble_offset & 1:
            value = byte & 0x0F
        else:
            value = byte >> 4
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


def parse_entries(data: bytes) -> list[dict[str, Any]]:
    signature = le16(data, 0)
    count = le16(data, 2)
    if signature != 0x8001 or count != 713:
        raise ValueError(f"unexpected GRAPHICS.DAT header signature={signature:#x} count={count}")
    header_bytes = 4 + count * 8
    if len(data) < header_bytes:
        raise ValueError("GRAPHICS.DAT too small for declared header")
    cursor = header_bytes
    entries: list[dict[str, Any]] = []
    for index in range(count):
        compressed_bytes = le16(data, 4 + 2 * index)
        decompressed_bytes = le16(data, 4 + 2 * count + 2 * index)
        width = le16(data, 4 + 4 * count + 4 * index)
        height = le16(data, 4 + 4 * count + 4 * index + 2)
        entries.append(
            {
                "index": index,
                "fileOffset": cursor,
                "compressedBytes": compressed_bytes,
                "decompressedBytes": decompressed_bytes,
                "width": width,
                "height": height,
            }
        )
        cursor += compressed_bytes
    if cursor != len(data):
        raise ValueError(f"record byte sum {cursor} != file size {len(data)}")
    return entries


def expand_img3(source: bytes, width: int, height: int) -> dict[str, Any]:
    if le16(source, 0) != width or le16(source, 2) != height:
        raise ValueError("entry-local width/height prefix does not match header table")
    stride_pixels = (width + 1) & ~1
    packed = bytearray((stride_pixels * height) // 2)

    def get_pixel(pixel_offset: int) -> int:
        byte = packed[pixel_offset >> 1]
        return byte & 0x0F if (pixel_offset & 1) else byte >> 4

    def set_pixel(pixel_offset: int, color: int) -> None:
        byte_index = pixel_offset >> 1
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
        start_offset = destination_offset
        while destination_offset - start_offset < total_visible_pixels:
            command = reader.nibble()
            kind = command & 0x07
            if kind == 6:
                count = reader.pixel_count() if (command & 0x08) else 1
                for _ in range(count):
                    set_pixel(destination_offset, get_pixel(destination_offset - stride_pixels))
                    destination_offset += 1
            else:
                color = local_palette[kind] if kind < 6 else reader.nibble()
                count = reader.pixel_count() if (command & 0x08) else 1
                for _ in range(count):
                    set_pixel(destination_offset, color)
                    destination_offset += 1
    else:
        total_done = 0
        remaining_in_line = width
        while total_done < total_visible_pixels:
            command = reader.nibble()
            kind = command & 0x07
            if kind < 6:
                color = local_palette[kind]
            elif kind == 6:
                color = 0
            else:
                color = reader.nibble()
            count = reader.pixel_count() if (command & 0x08) else 1
            left = count
            while left > 0:
                chunk = remaining_in_line if left >= remaining_in_line else left
                if kind == 6:
                    for _ in range(chunk):
                        set_pixel(destination_offset, get_pixel(destination_offset - stride_pixels))
                        destination_offset += 1
                else:
                    for _ in range(chunk):
                        set_pixel(destination_offset, color)
                        destination_offset += 1
                left -= chunk
                remaining_in_line -= chunk
                if remaining_in_line == 0:
                    destination_offset += stride_pixels - width
                    remaining_in_line = width
            total_done += count

    unpacked = bytearray(total_visible_pixels)
    for y in range(height):
        for x in range(width):
            unpacked[y * width + x] = get_pixel(y * stride_pixels + x)

    return {
        "localPaletteNibbles": local_palette,
        "stridePixels": stride_pixels,
        "packedBytes": len(packed),
        "unpackedBytes": len(unpacked),
        "decoderNibbleOffsetAfterVisiblePixels": reader.nibble_offset,
        "packedSha256": hashlib.sha256(packed).hexdigest(),
        "unpackedPixelSha256": hashlib.sha256(unpacked).hexdigest(),
    }


def build_manifest(graphics_dat: Path) -> dict[str, Any]:
    data = graphics_dat.read_bytes()
    source_sha = hashlib.sha256(data).hexdigest()
    if source_sha != EXPECTED_GRAPHICS_SHA256:
        raise ValueError(f"canonical GRAPHICS.DAT sha mismatch: {source_sha}")
    entries = parse_entries(data)
    target_records = []
    for index, meta in TARGETS.items():
        entry = entries[index]
        start = entry["fileOffset"]
        end = start + entry["compressedBytes"]
        record = data[start:end]
        decode = expand_img3(record, entry["width"], entry["height"])
        target_records.append(
            {
                **meta,
                "graphicIndex": index,
                "sourceFile": "GRAPHICS.DAT",
                "fileOffset": start,
                "compressedBytes": entry["compressedBytes"],
                "decompressedBytesHeaderValue": entry["decompressedBytes"],
                "width": entry["width"],
                "height": entry["height"],
                "entryPrefixHex": record[:4].hex(),
                "compressedRecordSha256": hashlib.sha256(record).hexdigest(),
                "decode": decode,
            }
        )
    return {
        "pass": PASS,
        "status": "passed",
        "scope": "Deterministic source/data inventory for GRAPHICS.DAT wall indices 93..107 required by pass304 wall-comparator promotion; no bitmap dumps emitted.",
        "canonicalGraphicsDat": {
            "path": str(graphics_dat),
            "sha256": source_sha,
            "expectedSha256": EXPECTED_GRAPHICS_SHA256,
            "signature": "0x8001",
            "itemCount": 713,
            "headerBytes": 5708,
            "fileBytes": len(data),
        },
        "decoder": {
            "algorithm": "Firestaff IMG3-compatible nibble RLE expansion, then row-stride-aware unpack to one byte per 4-bit pixel",
            "sourceAnchors": [
                "asset_loader_m11.c:M11_AssetLoader_Load",
                "image_backend_pc34_compat.c:IMG3_Compat_ExpandFromSource",
                "graphics_dat_snd3_loader_v1.c:V1_GraphicsSnd3_ParseManifest header table layout",
            ],
            "outputs": ["compressedRecordSha256", "packedSha256", "unpackedPixelSha256"],
        },
        "records": target_records,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true", help="write canonical JSON manifest")
    parser.add_argument("--graphics-dat", type=Path, default=CANONICAL_GRAPHICS_DAT)
    args = parser.parse_args()
    manifest = build_manifest(args.graphics_dat)
    text = json.dumps(manifest, indent=2, sort_keys=True) + "\n"
    if args.write:
        OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
        OUT_JSON.write_text(text, encoding="utf-8")
    else:
        existing = OUT_JSON.read_text(encoding="utf-8")
        if existing != text:
            raise SystemExit(f"{OUT_JSON} is not up to date; run with --write")
    print(f"{PASS}: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
