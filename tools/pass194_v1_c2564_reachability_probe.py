#!/usr/bin/env python3
"""Classify pass178's C2564 viewport delta against ReDMCSB F0115 zone reachability.

Evidence only.  C2564 looks like a C2500-family anchor if the whole
C2500..C2567 layout block is treated as one raw point table, but ReDMCSB's
F0115 has two distinct object-zone paths:

* normal floor/open-cell objects: C2500 + G2028[viewSquare] * 4 + viewCell
* alcove objects: C2548 + objectCoordinateSet * 7 + G2029[viewSquare]

This probe records which path can produce C2564 and adds a tiny pixel-window
classification for the pass178 target scene.
"""
from __future__ import annotations

import json
import struct
import zlib
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
ZONES_JSON = REPO / "zones_h_reconstruction.json"
CURRENT = REPO / "verification-screens" / "03_ingame_move_forward_latest.png"
ORIGINAL = REPO / "verification-screens" / "pass112-n2-stable-hud-route" / "viewport_224x136" / "03_ingame_move_forward_original_viewport_224x136.png"
OUT_DIR = REPO / "parity-evidence" / "verification" / "pass194_v1_c2564_reachability_probe"
OUT_JSON = OUT_DIR / "pass194_v1_c2564_reachability_probe.json"
OUT_MD = OUT_DIR / "pass194_v1_c2564_reachability_probe.md"

VIEWPORT_W = 224
VIEWPORT_H = 136
FRAME_W = 320
FRAME_H = 200
VIEWPORT_X = 0
VIEWPORT_Y = 33
TARGET_ZONE = 2564
TARGET_ANCHOR = (181, 62)
TARGET_WINDOW = (177, 58, 185, 66)

# ReDMCSB DUNVIEW.C:371-374 / DEFS.H:2600-2614 (Towns/PC media branch)
G2027_VIEW_DEPTH = [0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4]
G2028_NORMAL_OBJECT_ROW = [11, -1, -1, 8, 9, 10, 5, 6, 7, -1, -1, 0, 1, 2, 3, 4, -1, -1, -1, -1, -1, -1, -1]
G2029_ALCOVE_SLOT = [-1, -1, -1, 6, -1, -1, 3, 4, 5, -1, -1, 0, 1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1]

SOURCE_CITATIONS = [
    "ReDMCSB DUNVIEW.C:371-374 defines G2028 normal-object rows and G2029 alcove slots; G2028 never maps any view square to row 16.",
    "ReDMCSB DEFS.H:2600-2614 defines the media-720 view-square indices including D3C=11, D3L=12, D3R=13, D4C=16, D4L=17, D4R=18.",
    "ReDMCSB DUNVIEW.C:4922-4927 gates object drawing on thing type, L2476_i_ >= 0, matching thing cell, and depth/cell visibility.",
    "ReDMCSB DUNVIEW.C:5071-5078 selects C2548 + coordinateSet*7 + G2029[viewSquare] for alcove objects, otherwise C2500 + G2028[viewSquare]*4 + viewCell for normal floor/open-cell objects, then applies object pile shifts.",
    "ReDMCSB DUNVIEW.C:8468-8477 still calls F0115 for D4L/D4R/D4C, but G2028 entries for view-square indices 16-18 are -1, so those calls cannot emit normal C2500 object zones.",
]


def rel(path: Path) -> str:
    return str(path.resolve().relative_to(REPO))


def png_rgb(path: Path):
    with path.open("rb") as f:
        if f.read(8) != b"\x89PNG\r\n\x1a\n":
            raise ValueError(f"not a PNG: {path}")
        width = height = bit_depth = color_type = None
        chunks = bytearray()
        while True:
            raw_len = f.read(4)
            if not raw_len:
                break
            length = struct.unpack(">I", raw_len)[0]
            kind = f.read(4)
            data = f.read(length)
            f.read(4)
            if kind == b"IHDR":
                width, height, bit_depth, color_type, _c, _f, _i = struct.unpack(">IIBBBBB", data)
            elif kind == b"IDAT":
                chunks.extend(data)
            elif kind == b"IEND":
                break
    if width is None or height is None or bit_depth != 8 or color_type not in (0, 2, 6):
        raise ValueError(f"unsupported PNG format: {path}")
    channels = {0: 1, 2: 3, 6: 4}[color_type]
    stride = width * channels
    raw = zlib.decompress(bytes(chunks))
    prev = bytearray(stride)
    rows = []
    pos = 0
    for _ in range(height):
        ft = raw[pos]
        pos += 1
        scan = bytearray(raw[pos:pos + stride])
        pos += stride
        recon = bytearray(stride)
        for i, val in enumerate(scan):
            left = recon[i - channels] if i >= channels else 0
            up = prev[i]
            ul = prev[i - channels] if i >= channels else 0
            if ft == 0:
                out = val
            elif ft == 1:
                out = (val + left) & 255
            elif ft == 2:
                out = (val + up) & 255
            elif ft == 3:
                out = (val + ((left + up) >> 1)) & 255
            elif ft == 4:
                p = left + up - ul
                pa, pb, pc = abs(p - left), abs(p - up), abs(p - ul)
                pred = left if pa <= pb and pa <= pc else (up if pb <= pc else ul)
                out = (val + pred) & 255
            else:
                raise ValueError(f"unsupported PNG filter {ft}: {path}")
            recon[i] = out
        rows.append(recon)
        prev = recon
    pixels = []
    for row in rows:
        for x in range(width):
            base = x * channels
            if color_type == 0:
                pixels.append((row[base], row[base], row[base]))
            else:
                pixels.append((row[base], row[base + 1], row[base + 2]))
    return width, height, pixels


def crop_viewport(width, height, pixels):
    if (width, height) == (VIEWPORT_W, VIEWPORT_H):
        return pixels
    if (width, height) != (FRAME_W, FRAME_H):
        raise ValueError(f"unexpected image size {(width, height)}")
    out = []
    for y in range(VIEWPORT_H):
        start = (VIEWPORT_Y + y) * width + VIEWPORT_X
        out.extend(pixels[start:start + VIEWPORT_W])
    return out


def luma(p):
    return (299 * p[0] + 587 * p[1] + 114 * p[2]) // 1000


def window_stats(cur, orig):
    x0, y0, x1, y1 = TARGET_WINDOW
    pixels = []
    cur_dark = 0
    orig_bright = 0
    changed = 0
    max_delta = {"x": x0, "y": y0, "current_rgb": [0, 0, 0], "original_rgb": [0, 0, 0], "luma_abs": 0}
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            c = cur[y * VIEWPORT_W + x]
            o = orig[y * VIEWPORT_W + x]
            lc = luma(c)
            lo = luma(o)
            d = abs(lc - lo)
            pixels.append((lc, lo, d))
            cur_dark += int(lc <= 8)
            orig_bright += int(lo >= 160)
            changed += int(c != o)
            if d > max_delta["luma_abs"]:
                max_delta = {"x": x, "y": y, "current_rgb": list(c), "original_rgb": list(o), "luma_abs": d}
    n = len(pixels)
    return {
        "window": list(TARGET_WINDOW),
        "pixels": n,
        "changed_pixels": changed,
        "current_dark_pixels_luma_le_8": cur_dark,
        "original_bright_pixels_luma_ge_160": orig_bright,
        "mean_current_luma": round(sum(p[0] for p in pixels) / n, 3),
        "mean_original_luma": round(sum(p[1] for p in pixels) / n, 3),
        "mean_luma_abs": round(sum(p[2] for p in pixels) / n, 3),
        "max_delta": max_delta,
    }


def normal_object_zones():
    zones = []
    for view_square, row in enumerate(G2028_NORMAL_OBJECT_ROW):
        if row < 0:
            continue
        depth = G2027_VIEW_DEPTH[view_square]
        for view_cell in range(4):
            if depth == 3 and view_cell <= 1:
                continue
            if depth == 0 and view_cell >= 2:
                continue
            zones.append({"zone": 2500 + row * 4 + view_cell, "row": row, "view_square_index": view_square, "view_cell": view_cell, "view_depth": depth})
    return zones


def alcove_zones_for_coordinate_sets(max_coordinate_set=3):
    zones = []
    for view_square, slot in enumerate(G2029_ALCOVE_SLOT):
        if slot < 0:
            continue
        for coordinate_set in range(max_coordinate_set):
            zones.append({"zone": 2548 + coordinate_set * 7 + slot, "coordinate_set": coordinate_set, "alcove_slot": slot, "view_square_index": view_square})
    return zones


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    records = json.loads(ZONES_JSON.read_text())["records"]
    target_record = records[str(TARGET_ZONE)]
    normal_hits = [z for z in normal_object_zones() if z["zone"] == TARGET_ZONE]
    alcove_hits = [z for z in alcove_zones_for_coordinate_sets(4) if z["zone"] == TARGET_ZONE]
    cw, ch, cp = png_rgb(CURRENT)
    ow, oh, op = png_rgb(ORIGINAL)
    stats = window_stats(crop_viewport(cw, ch, cp), crop_viewport(ow, oh, op))
    result = {
        "schema": "pass194_v1_c2564_reachability_probe.v1",
        "source_citations": SOURCE_CITATIONS,
        "target": {"scene": "ingame_move_forward", "zone": TARGET_ZONE, "anchor": list(TARGET_ANCHOR), "layout_record": target_record},
        "classification": {
            "normal_open_cell_floor_item_zone_reachable": bool(normal_hits),
            "alcove_object_zone_reachable": bool(alcove_hits),
            "normal_hits": normal_hits,
            "alcove_hits_for_coordinate_sets_0_to_3": alcove_hits,
            "finding": "C2564 is not emitted by the normal C2500 open-cell/floor-item path; it is in the C2548 alcove-object subfamily (C2548 + coordinateSet*7 + G2029 slot).",
        },
        "pixel_window": stats,
        "interpretation": "The pass178 C2564 delta should not be treated as missing normal floor-item sprite placement. The 9x9 window is mostly/currently black while the original is bright textured wall/viewport content, so this evidence points to wall/open-cell/base-geometry or alcove-adjacent rendering, not a low-risk floor-item placement fix.",
        "non_claims": [
            "This does not prove which wall/open-cell geometry primitive is missing.",
            "This does not modify rendering.",
            "This only classifies the C2564 source-zone family and the local target pixels.",
        ],
    }
    OUT_JSON.write_text(json.dumps(result, indent=2) + "\n")
    OUT_MD.write_text("\n".join([
        "# Pass 194 — V1 C2564 reachability probe",
        "",
        "## Source citations audited first",
        *[f"- {c}" for c in SOURCE_CITATIONS],
        "",
        "## Finding",
        "",
        f"- Target: `ingame_move_forward`, `C{TARGET_ZONE}`, anchor `{list(TARGET_ANCHOR)}`, window `{list(TARGET_WINDOW)}`.",
        f"- Normal/open-cell floor-item path reachable? `{bool(normal_hits)}`.",
        f"- Alcove-object path reachable? `{bool(alcove_hits)}`; hits: `{alcove_hits}`.",
        "- Classification: `C2564` is part of the `C2548` alcove-object subfamily, not the normal `C2500 + G2028*4 + viewCell` open-cell/floor-item path.",
        "",
        "## Pixel window",
        "",
        f"- Changed pixels: `{stats['changed_pixels']}/{stats['pixels']}`; mean luma current/original/abs: `{stats['mean_current_luma']}` / `{stats['mean_original_luma']}` / `{stats['mean_luma_abs']}`.",
        f"- Current dark pixels (luma <= 8): `{stats['current_dark_pixels_luma_le_8']}/{stats['pixels']}`; original bright pixels (luma >= 160): `{stats['original_bright_pixels_luma_ge_160']}/{stats['pixels']}`.",
        f"- Max delta: `{stats['max_delta']}`.",
        "",
        "## Interpretation",
        "",
        "Do not spend the next lane on normal floor-item sprite placement for this target. The evidence points to wall/open-cell/base-geometry or alcove-adjacent rendering around the right side of the viewport; no low-risk code fix was made.",
        "",
        f"JSON: `{rel(OUT_JSON)}`",
    ]) + "\n")
    print(json.dumps({"pass": True, "json": rel(OUT_JSON), "summary": rel(OUT_MD), "normal_reachable": bool(normal_hits), "alcove_reachable": bool(alcove_hits), "pixel_window": stats}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
