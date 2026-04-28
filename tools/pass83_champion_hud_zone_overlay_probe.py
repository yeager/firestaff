#!/usr/bin/env python3
"""Generate champion-HUD source-zone overlays for current V1 captures.

Evidence tooling only: overlays/crops DM1 PC 3.4 layout-696 champion HUD zones
on deterministic Firestaff V1 screenshots. No original-runtime parity claim is
made by this script.
"""
from __future__ import annotations

import argparse, binascii, hashlib, json, struct, zlib
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parent.parent
SCREEN_DIR = REPO / "verification-screens"
OUT_DIR = REPO / "parity-evidence" / "overlays" / "pass83"
STATS = OUT_DIR / "pass83_champion_hud_zone_overlay_stats.json"
MD = REPO / "parity-evidence" / "pass83_champion_hud_zone_overlay.md"
FRAME_W, FRAME_H = 320, 200
# DM1 PC 3.4 layout-696 anchors C151..C154 at source x=0 with 69px
# spacing. Firestaff's old 12px party-panel inset is V2-only chrome; keep
# this evidence overlay aligned with the V1 source-origin locked in pass90.
PARTY_X, PARTY_Y, SLOT_W, SLOT_H, SLOT_STEP = 0, 0, 67, 29, 69
# Source layout-696 C113/C116 champion icon clusters start at x=281.
# Keep this explicit so evidence generation fails if a future HUD-zone refresh
# accidentally pushes the fourth status box into the top-right icon lane.
CHAMPION_ICON_LANE_X = 281

@dataclass(frozen=True)
class Scene:
    name: str
    file: str

@dataclass(frozen=True)
class Zone:
    name: str
    xywh: tuple[int, int, int, int]
    source_reference: str

@dataclass
class RGBImage:
    width: int
    height: int
    pixels: list[tuple[int, int, int]]

    def copy(self) -> "RGBImage":
        return RGBImage(self.width, self.height, list(self.pixels))

    def crop(self, box: tuple[int, int, int, int]) -> "RGBImage":
        x0, y0, x1, y1 = box
        out = []
        for y in range(y0, y1):
            out.extend(self.pixels[y * self.width + x0:y * self.width + x1])
        return RGBImage(x1 - x0, y1 - y0, out)

    def rect(self, x: int, y: int, w: int, h: int, color: tuple[int, int, int]) -> None:
        for xx in range(x, x + w):
            if 0 <= xx < self.width and 0 <= y < self.height:
                self.pixels[y * self.width + xx] = color
            yy = y + h - 1
            if 0 <= xx < self.width and 0 <= yy < self.height:
                self.pixels[yy * self.width + xx] = color
        for yy in range(y, y + h):
            if 0 <= x < self.width and 0 <= yy < self.height:
                self.pixels[yy * self.width + x] = color
            xx = x + w - 1
            if 0 <= xx < self.width and 0 <= yy < self.height:
                self.pixels[yy * self.width + xx] = color

SCENES = (
    Scene("party_hud_with_champions", "07_party_hud_with_champions.png"),
    Scene("spell_panel_with_champions", "08_spell_panel_with_champions.png"),
    Scene("four_champion_priority", "pass81-champion-hud-priority/party_hud_four_champions_vga.png"),
)

def rel(path: Path) -> str:
    return str(path.resolve().relative_to(REPO))

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()

def _paeth(a: int, b: int, c: int) -> int:
    p = a + b - c; pa = abs(p - a); pb = abs(p - b); pc = abs(p - c)
    return a if pa <= pb and pa <= pc else (b if pb <= pc else c)

def load_rgb(path: Path) -> RGBImage:
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise SystemExit(f"not a PNG: {rel(path)}")
    pos = 8; width = height = bit_depth = color_type = None; chunks = []
    while pos < len(data):
        length = struct.unpack(">I", data[pos:pos + 4])[0]; pos += 4
        typ = data[pos:pos + 4]; pos += 4
        payload = data[pos:pos + length]; pos += length + 4
        if typ == b"IHDR":
            width, height, bit_depth, color_type, _comp, _filt, interlace = struct.unpack(">IIBBBBB", payload)
            if bit_depth != 8 or color_type not in (2, 6) or interlace != 0:
                raise SystemExit(f"unsupported PNG format in {rel(path)}")
        elif typ == b"IDAT":
            chunks.append(payload)
        elif typ == b"IEND":
            break
    assert width is not None and height is not None and color_type is not None
    channels = 3 if color_type == 2 else 4
    bpp = channels
    raw = zlib.decompress(b"".join(chunks))
    stride = width * channels
    rows: list[bytearray] = []
    off = 0
    for _y in range(height):
        f = raw[off]; off += 1
        cur = bytearray(raw[off:off + stride]); off += stride
        prev = rows[-1] if rows else bytearray(stride)
        for i in range(stride):
            left = cur[i - bpp] if i >= bpp else 0
            up = prev[i]
            ul = prev[i - bpp] if i >= bpp else 0
            if f == 1: cur[i] = (cur[i] + left) & 255
            elif f == 2: cur[i] = (cur[i] + up) & 255
            elif f == 3: cur[i] = (cur[i] + ((left + up) >> 1)) & 255
            elif f == 4: cur[i] = (cur[i] + _paeth(left, up, ul)) & 255
            elif f != 0: raise SystemExit(f"unsupported PNG filter {f} in {rel(path)}")
        rows.append(cur)
    pixels = []
    for row in rows:
        for i in range(0, len(row), channels):
            pixels.append((row[i], row[i + 1], row[i + 2]))
    img = RGBImage(width, height, pixels)
    if (img.width, img.height) != (FRAME_W, FRAME_H):
        raise SystemExit(f"{rel(path)} is {(img.width, img.height)}, expected {(FRAME_W, FRAME_H)}")
    return img

def save_png(img: RGBImage, path: Path) -> None:
    def chunk(typ: bytes, payload: bytes) -> bytes:
        return struct.pack(">I", len(payload)) + typ + payload + struct.pack(">I", binascii.crc32(typ + payload) & 0xffffffff)
    raw = bytearray()
    for y in range(img.height):
        raw.append(0)
        for r, g, b in img.pixels[y * img.width:(y + 1) * img.width]:
            raw.extend((r, g, b))
    payload = struct.pack(">IIBBBBB", img.width, img.height, 8, 2, 0, 0, 0)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", payload) + chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b""))

def zone_table() -> list[Zone]:
    zones: list[Zone] = []
    for slot in range(4):
        x = PARTY_X + slot * SLOT_STEP
        zones.extend([
            Zone(f"C{151 + slot}_status_box_slot{slot}", (x, PARTY_Y, SLOT_W, SLOT_H), "layout-696 C151..C154: 67x29 champion status boxes; DEFS.H C69 spacing"),
            Zone(f"C{159 + slot}_name_clear_slot{slot}", (x, PARTY_Y, 43, 7), "layout-696 C159..C162: F0292 clears champion-name field before centered text"),
            Zone(f"C{163 + slot}_name_text_slot{slot}", (x + 1, PARTY_Y, 42, 7), "layout-696 C163..C166: clipped centered champion-name text zone"),
            Zone(f"C{211 + slot * 2}_ready_hand_slot{slot}", (x + 4, PARTY_Y + 10, 16, 16), "layout-696 C211/C213/C215/C217 ready-hand icon parent"),
            Zone(f"C{212 + slot * 2}_action_hand_slot{slot}", (x + 24, PARTY_Y + 10, 16, 16), "layout-696 C212/C214/C216/C218 action-hand icon parent"),
            Zone(f"C{195 + slot}_hp_bar_slot{slot}", (x + 46, PARTY_Y + 4, 4, 25), "layout-696 C195..C198: F0287 HP vertical bar container"),
            Zone(f"C{199 + slot}_stamina_bar_slot{slot}", (x + 53, PARTY_Y + 4, 4, 25), "layout-696 C199..C202: F0287 stamina vertical bar container"),
            Zone(f"C{203 + slot}_mana_bar_slot{slot}", (x + 60, PARTY_Y + 4, 4, 25), "layout-696 C203..C206: F0287 mana vertical bar container"),
        ])
    return zones

ZONES = tuple(zone_table())

def _inside(parent: tuple[int, int, int, int], child: tuple[int, int, int, int]) -> bool:
    px, py, pw, ph = parent
    cx, cy, cw, ch = child
    return cx >= px and cy >= py and cx + cw <= px + pw and cy + ch <= py + ph

def geometry_acceptance(zones: Iterable[Zone] = ZONES) -> dict[str, object]:
    zone_list = list(zones)
    status_boxes = [z for z in zone_list if "status_box" in z.name]
    problems: list[str] = []
    if len(status_boxes) != 4:
        problems.append(f"expected 4 status boxes, found {len(status_boxes)}")
    for i, box in enumerate(status_boxes):
        bx, by, bw, bh = box.xywh
        expected_x = PARTY_X + i * SLOT_STEP
        if box.xywh != (expected_x, PARTY_Y, SLOT_W, SLOT_H):
            problems.append(f"slot {i} status box {box.xywh} != {(expected_x, PARTY_Y, SLOT_W, SLOT_H)}")
        if bx + bw > CHAMPION_ICON_LANE_X:
            problems.append(f"slot {i} status box right edge {bx + bw} overlaps icon lane x={CHAMPION_ICON_LANE_X}")
        if i and bx < status_boxes[i - 1].xywh[0] + status_boxes[i - 1].xywh[2]:
            problems.append(f"slot {i} status box overlaps previous slot")
        for child in zone_list:
            if child is box or not child.name.endswith(f"_slot{i}"):
                continue
            if not _inside(box.xywh, child.xywh):
                problems.append(f"{child.name} {child.xywh} escapes {box.name} {box.xywh}")
    right_edge = max(x + w for x, _y, w, _h in (z.xywh for z in status_boxes)) if status_boxes else 0
    return {
        "pass": not problems,
        "status_box_count": len(status_boxes),
        "status_box_stride": SLOT_STEP,
        "status_box_width": SLOT_W,
        "status_box_right_edge": right_edge,
        "champion_icon_lane_x": CHAMPION_ICON_LANE_X,
        "icon_lane_gap_px": CHAMPION_ICON_LANE_X - right_edge,
        "problems": problems,
    }

def crop_metrics(crop: RGBImage) -> dict[str, object]:
    nonblack = sum(1 for p in crop.pixels if p != (0, 0, 0))
    return {"nonblack_pixels": nonblack, "total_pixels": len(crop.pixels), "nonblack_ratio": round(nonblack / len(crop.pixels), 4), "unique_colors": len(set(crop.pixels))}

def draw_overlay(img: RGBImage, out_path: Path) -> None:
    overlay = img.copy()
    for zone in ZONES:
        x, y, w, h = zone.xywh
        if "status_box" in zone.name: color = (255, 64, 64)
        elif "name" in zone.name: color = (255, 220, 64)
        elif "hand" in zone.name: color = (64, 255, 255)
        else: color = (64, 255, 64)
        overlay.rect(x, y, w, h, color)
    save_png(overlay, out_path)

def write_markdown(result: dict[str, object]) -> None:
    lines = [
        "# Pass 83 — champion HUD source-zone overlay", "",
        "This pass adds visual evidence for the top-row V1 champion HUD. It overlays and crops the DM1 PC 3.4 layout-696 champion status zones already asserted by the M11 game-view probe.", "",
        "Pass 96 refresh: these overlays now use the V1 source-origin champion-box x=0 geometry from pass90, not Firestaff's legacy V2-only 12px party-panel inset.", "",
        "Honesty: these files are evidence aids only. They do not claim pixel-perfect parity with original runtime screenshots.", "",
        f"- stats: `{result['stats']}`", f"- scenes: {len(result['scenes'])}", f"- pass: `{result['pass']}`", "",
        "## Geometry acceptance", "",
        f"- status boxes: `{result['geometry_acceptance']['status_box_count']}` at `{result['geometry_acceptance']['status_box_width']}x{SLOT_H}` with `{result['geometry_acceptance']['status_box_stride']}` px stride",
        f"- right edge: `x={result['geometry_acceptance']['status_box_right_edge']}`; champion icon lane starts at `x={result['geometry_acceptance']['champion_icon_lane_x']}`; gap `{result['geometry_acceptance']['icon_lane_gap_px']}` px",
        f"- geometry pass: `{result['geometry_acceptance']['pass']}`", "",
        "## Generated overlays", "",
    ]
    for scene in result["scenes"]:  # type: ignore[index]
        lines.append(f"- `{scene['scene']}`: `{scene['overlay']}` from `{scene['screenshot']}`")
    lines.extend(["", "## Source zones", "", "| zone | xywh | source reference |", "|------|------|------------------|"])
    for zone in result["zones"]:  # type: ignore[index]
        lines.append(f"| `{zone['name']}` | `{zone['xywh']}` | {zone['source_reference']} |")
    lines.append("")
    MD.write_text("\n".join(lines))

def run_self_test() -> int:
    zones = zone_table()
    assert zones[0].xywh == (0, 0, 67, 29)
    assert zones[8].xywh == (69, 0, 67, 29)
    assert zones[24].xywh == (207, 0, 67, 29)
    assert any(z.name == "C203_mana_bar_slot0" and z.xywh == (60, 4, 4, 25) for z in zones)
    assert any(z.name == "C206_mana_bar_slot3" and z.xywh == (267, 4, 4, 25) for z in zones)
    acceptance = geometry_acceptance(zones)
    assert acceptance["pass"], acceptance["problems"]
    assert acceptance["status_box_right_edge"] == 274
    assert acceptance["champion_icon_lane_x"] == 281
    assert acceptance["icon_lane_gap_px"] == 7
    print(json.dumps({"pass": True, "zones": len(zones), "geometry_acceptance": acceptance}, indent=2)); return 0

def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser(); ap.add_argument("--self-test", action="store_true"); args = ap.parse_args(argv)
    if args.self_test: return run_self_test()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    scenes: list[dict[str, object]] = []; problems: list[str] = []
    for scene in SCENES:
        screen_path = SCREEN_DIR / scene.file
        if not screen_path.exists(): problems.append(f"missing screenshot: {rel(screen_path)}"); continue
        img = load_rgb(screen_path)
        overlay_path = OUT_DIR / f"{scene.name}_champion_hud_zones_overlay.png"; draw_overlay(img, overlay_path)
        zone_rows = []
        for zone in ZONES:
            x, y, w, h = zone.xywh; crop = img.crop((x, y, x + w, y + h))
            crop_path = OUT_DIR / f"{scene.name}_{zone.name}.png"; save_png(crop, crop_path)
            zone_rows.append({"name": zone.name, "xywh": [x, y, w, h], "source_reference": zone.source_reference, "crop": rel(crop_path), "crop_sha256": sha256(crop_path), "metrics": crop_metrics(crop)})
        scenes.append({"scene": scene.name, "screenshot": rel(screen_path), "screenshot_sha256": sha256(screen_path), "overlay": rel(overlay_path), "overlay_sha256": sha256(overlay_path), "zones": zone_rows})
    acceptance = geometry_acceptance()
    problems.extend(acceptance["problems"])
    result = {"schema": "pass83_champion_hud_zone_overlay_probe.v3", "honesty": "Evidence only. These overlays/crops expose current Firestaff V1 pixels against source-zone geometry; they are not original-runtime parity claims. Pass96 refresh keeps the evidence geometry at source x=0, matching pass90 and layout-696 C151..C154. Pass100 adds a geometry-acceptance guard for status-box children, inter-slot overlap, and the C113/C116 champion-icon lane gap.", "frame_size": [FRAME_W, FRAME_H], "zones": [{"name": z.name, "xywh": list(z.xywh), "source_reference": z.source_reference} for z in ZONES], "geometry_acceptance": acceptance, "scenes": scenes, "problems": problems, "pass": bool(scenes) and not problems, "stats": rel(STATS)}
    STATS.write_text(json.dumps(result, indent=2) + "\n"); write_markdown(result)
    print(json.dumps({"scenes": len(scenes), "zones": len(ZONES), "problems": problems, "stats": rel(STATS), "markdown": rel(MD)}, indent=2))
    return 0 if result["pass"] else 1

if __name__ == "__main__":
    raise SystemExit(main())
