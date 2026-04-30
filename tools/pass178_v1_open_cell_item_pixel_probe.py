#!/usr/bin/env python3
"""Probe V1 viewport open-cell floor-item anchor pixel deltas.

Evidence only: this compares current Firestaff V1 viewport pixels against the
curated original DM1/V1 viewport crops around ReDMCSB layout-696 C2500 object
anchor points.  It identifies the strongest next pixel delta near source-backed
floor-object placement anchors; it does not claim sprite parity or make a
rendering change.
"""
from __future__ import annotations

import hashlib
import json
import struct
import zlib
from dataclasses import dataclass
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
ZONES_JSON = REPO / "zones_h_reconstruction.json"
CURRENT_DIR = REPO / "verification-screens"
ORIGINAL_VIEWPORT_DIR = REPO / "verification-screens" / "pass112-n2-stable-hud-route" / "viewport_224x136"
OUT_DIR = REPO / "parity-evidence" / "verification" / "pass178_v1_open_cell_item_pixel_probe"
STATS = OUT_DIR / "pass178_v1_open_cell_item_pixel_probe.json"
SUMMARY = OUT_DIR / "pass178_v1_open_cell_item_pixel_probe.md"
TSV = OUT_DIR / "pass178_v1_open_cell_item_pixel_probe.tsv"

FRAME_W = 320
FRAME_H = 200
VIEWPORT_X = 0
VIEWPORT_Y = 33
VIEWPORT_W = 224
VIEWPORT_H = 136
WINDOW_RADIUS = 4

SOURCE_CITATIONS = [
    "ReDMCSB DUNVIEW.C:4547-4582 defines F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF and its object -> creature -> projectile -> explosion per-cell structure.",
    "ReDMCSB DUNVIEW.C:4820-4862 starts the F0115 object pass, defers groups/projectiles/explosions, and uses M612_GRAPHIC_FIRST_OBJECT plus G0218 object coordinate sets.",
    "ReDMCSB DUNVIEW.C:5200-5256 draws creatures only after the object pass for the processed cell.",
    "ReDMCSB DUNVIEW.C:5648-5702 draws projectiles after creatures, using M613_GRAPHIC_FIRST_PROJECTILE when not drawn as objects.",
    "ReDMCSB DUNVIEW.C:5926-6013 restarts over the square things for explosions after cell/projectile passes and tracks fluxcages separately.",
    "ReDMCSB DUNGEON.C:1730-1750 F0161_DUNGEON_GetSquareFirstThing returns G0283_pT_SquareFirstThings for the square thing list head.",
]


@dataclass(frozen=True)
class Scene:
    index: str
    name: str
    current: str
    original_viewport: str


SCENES = (
    Scene("01", "ingame_start", "01_ingame_start_latest.png", "01_ingame_start_original_viewport_224x136.png"),
    Scene("02", "ingame_turn_right", "02_ingame_turn_right_latest.png", "02_ingame_turn_right_original_viewport_224x136.png"),
    Scene("03", "ingame_move_forward", "03_ingame_move_forward_latest.png", "03_ingame_move_forward_original_viewport_224x136.png"),
    Scene("04", "ingame_spell_panel", "04_ingame_spell_panel_latest.png", "04_ingame_spell_panel_original_viewport_224x136.png"),
    Scene("05", "ingame_after_cast", "05_ingame_after_cast_latest.png", "05_ingame_after_cast_original_viewport_224x136.png"),
    Scene("06", "ingame_inventory_panel", "06_ingame_inventory_panel_latest.png", "06_ingame_inventory_panel_original_viewport_224x136.png"),
)


def rel(path: Path) -> str:
    return str(path.resolve().relative_to(REPO))


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def paeth(a: int, b: int, c: int) -> int:
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


def png_rgb(path: Path) -> tuple[int, int, list[tuple[int, int, int]]]:
    with path.open("rb") as f:
        if f.read(8) != b"\x89PNG\r\n\x1a\n":
            raise ValueError(f"not a PNG: {rel(path)}")
        width = height = bit_depth = color_type = None
        compressed = bytearray()
        while True:
            raw_len = f.read(4)
            if not raw_len:
                break
            length = struct.unpack(">I", raw_len)[0]
            chunk_type = f.read(4)
            data = f.read(length)
            f.read(4)  # crc
            if chunk_type == b"IHDR":
                width, height, bit_depth, color_type, _comp, _filter, _interlace = struct.unpack(">IIBBBBB", data)
            elif chunk_type == b"IDAT":
                compressed.extend(data)
            elif chunk_type == b"IEND":
                break
    if width is None or height is None or bit_depth != 8 or color_type not in (0, 2, 6):
        raise ValueError(f"unsupported PNG format in {rel(path)}")
    channels = {0: 1, 2: 3, 6: 4}[color_type]
    bpp = channels
    stride = width * channels
    inflated = zlib.decompress(bytes(compressed))
    rows: list[bytearray] = []
    pos = 0
    prev = bytearray(stride)
    for _y in range(height):
        filter_type = inflated[pos]
        pos += 1
        scan = bytearray(inflated[pos : pos + stride])
        pos += stride
        recon = bytearray(stride)
        for i, val in enumerate(scan):
            left = recon[i - bpp] if i >= bpp else 0
            up = prev[i]
            up_left = prev[i - bpp] if i >= bpp else 0
            if filter_type == 0:
                out = val
            elif filter_type == 1:
                out = (val + left) & 0xFF
            elif filter_type == 2:
                out = (val + up) & 0xFF
            elif filter_type == 3:
                out = (val + ((left + up) >> 1)) & 0xFF
            elif filter_type == 4:
                out = (val + paeth(left, up, up_left)) & 0xFF
            else:
                raise ValueError(f"unknown PNG filter {filter_type} in {rel(path)}")
            recon[i] = out
        rows.append(recon)
        prev = recon
    pixels: list[tuple[int, int, int]] = []
    for row in rows:
        for x in range(width):
            base = x * channels
            if color_type == 0:
                g = row[base]
                pixels.append((g, g, g))
            else:
                pixels.append((row[base], row[base + 1], row[base + 2]))
    return int(width), int(height), pixels


def crop_viewport(width: int, height: int, pixels: list[tuple[int, int, int]]) -> list[tuple[int, int, int]]:
    if width == VIEWPORT_W and height == VIEWPORT_H:
        return pixels
    if width != FRAME_W or height != FRAME_H:
        raise ValueError(f"current screenshot is {(width, height)}, expected {(FRAME_W, FRAME_H)} or viewport crop")
    out: list[tuple[int, int, int]] = []
    for y in range(VIEWPORT_H):
        start = (VIEWPORT_Y + y) * width + VIEWPORT_X
        out.extend(pixels[start : start + VIEWPORT_W])
    return out


def luma(pixel: tuple[int, int, int]) -> int:
    return (299 * pixel[0] + 587 * pixel[1] + 114 * pixel[2]) // 1000


def load_object_anchors() -> list[dict[str, int]]:
    data = json.loads(ZONES_JSON.read_text())
    records = {int(k): v for k, v in data["records"].items()}
    anchors: list[dict[str, int]] = []
    for zone in range(2500, 2568):
        rec = records[zone]
        x = int(rec["d1"])
        y = int(rec["d2"])
        if x == 0 and y == 0:
            continue
        anchors.append({"zone": zone, "x": x, "y": y})
    return anchors


def window_delta(cur: list[tuple[int, int, int]], orig: list[tuple[int, int, int]], x: int, y: int) -> dict[str, object]:
    x0 = max(0, x - WINDOW_RADIUS)
    y0 = max(0, y - WINDOW_RADIUS)
    x1 = min(VIEWPORT_W - 1, x + WINDOW_RADIUS)
    y1 = min(VIEWPORT_H - 1, y + WINDOW_RADIUS)
    count = 0
    changed = 0
    luma_sum = 0
    luma_max = 0
    rgb_sum = 0
    max_pixel = {"x": x0, "y": y0, "current_rgb": [0, 0, 0], "original_rgb": [0, 0, 0], "luma_abs": 0}
    for yy in range(y0, y1 + 1):
        for xx in range(x0, x1 + 1):
            idx = yy * VIEWPORT_W + xx
            a = cur[idx]
            b = orig[idx]
            ld = abs(luma(a) - luma(b))
            rd = abs(a[0] - b[0]) + abs(a[1] - b[1]) + abs(a[2] - b[2])
            count += 1
            luma_sum += ld
            rgb_sum += rd
            if rd:
                changed += 1
            if ld > luma_max:
                luma_max = ld
                max_pixel = {"x": xx, "y": yy, "current_rgb": list(a), "original_rgb": list(b), "luma_abs": ld}
    return {
        "window": [x0, y0, x1, y1],
        "pixels": count,
        "changed_pixels": changed,
        "mean_luma_abs": round(luma_sum / count, 3) if count else 0.0,
        "max_luma_abs": luma_max,
        "mean_rgb_abs_sum": round(rgb_sum / count, 3) if count else 0.0,
        "max_pixel": max_pixel,
    }


def write_tsv(rows: list[dict[str, object]]) -> None:
    lines = ["scene\tzone\tanchor_x\tanchor_y\twindow\tchanged_pixels\tpixels\tmean_luma_abs\tmax_luma_abs\tmax_pixel_x\tmax_pixel_y\tcurrent_rgb\toriginal_rgb"]
    for row in rows:
        max_pixel = row["max_pixel"]
        lines.append("\t".join([
            str(row["scene"]), str(row["zone"]), str(row["anchor_x"]), str(row["anchor_y"]),
            ",".join(map(str, row["window"])), str(row["changed_pixels"]), str(row["pixels"]),
            str(row["mean_luma_abs"]), str(row["max_luma_abs"]), str(max_pixel["x"]), str(max_pixel["y"]),
            ",".join(map(str, max_pixel["current_rgb"])), ",".join(map(str, max_pixel["original_rgb"])),
        ]))
    TSV.write_text("\n".join(lines) + "\n")


def write_summary(result: dict[str, object]) -> None:
    best = result["best_next_delta"]
    lines = [
        "# Pass 178 — V1 open-cell item anchor pixel probe", "",
        "Evidence-only probe for the narrow floor-object/open-cell item placement lane.",
        "It compares current Firestaff viewport pixels with curated original DM1/V1 viewport crops in small windows around source-backed C2500 layout-696 object anchors.", "",
        "## Source citations audited before probe", "",
    ]
    lines.extend(f"- {cite}" for cite in SOURCE_CITATIONS)
    lines.extend(["", "## Best next pixel delta", ""])
    if best:
        lines.extend([
            f"- Scene: `{best['scene']}`",
            f"- Source object anchor zone: `C{best['zone']}` at viewport `{best['anchor']}`",
            f"- Window: `{best['window']}` (radius {WINDOW_RADIUS})",
            f"- Changed pixels: `{best['changed_pixels']}/{best['pixels']}`",
            f"- Mean luma delta: `{best['mean_luma_abs']}`; max luma delta: `{best['max_luma_abs']}` at viewport `{[best['max_pixel']['x'], best['max_pixel']['y']]}`",
            f"- Current RGB / original RGB at max pixel: `{best['max_pixel']['current_rgb']}` / `{best['max_pixel']['original_rgb']}`",
        ])
    else:
        lines.append("- No comparable rows produced.")
    lines.extend(["", "## Artifacts", "", f"- JSON: `{rel(STATS)}`", f"- TSV: `{rel(TSV)}`", "", "Non-claims: this probe does not identify a specific object instance, does not prove asset-index correctness, and does not modify rendering. It only ranks source-anchor-adjacent pixel deltas for the next parity fix."])
    SUMMARY.write_text("\n".join(lines) + "\n")


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    anchors = load_object_anchors()
    rows: list[dict[str, object]] = []
    scene_results: list[dict[str, object]] = []
    problems: list[str] = []
    for scene in SCENES:
        current_path = CURRENT_DIR / scene.current
        original_path = ORIGINAL_VIEWPORT_DIR / scene.original_viewport
        if not current_path.exists():
            problems.append(f"missing current screenshot: {rel(current_path)}")
            continue
        if not original_path.exists():
            problems.append(f"missing original viewport: {rel(original_path)}")
            continue
        cw, ch, cp = png_rgb(current_path)
        ow, oh, op = png_rgb(original_path)
        if (ow, oh) != (VIEWPORT_W, VIEWPORT_H):
            problems.append(f"{rel(original_path)} is {(ow, oh)}, expected {(VIEWPORT_W, VIEWPORT_H)}")
            continue
        cur_view = crop_viewport(cw, ch, cp)
        scene_rows = []
        for anchor in anchors:
            x = anchor["x"]
            y = anchor["y"]
            if not (0 <= x < VIEWPORT_W and 0 <= y < VIEWPORT_H):
                continue
            delta = window_delta(cur_view, op, x, y)
            row = {"scene": scene.name, "scene_index": scene.index, "zone": anchor["zone"], "anchor_x": x, "anchor_y": y, "anchor": [x, y], **delta}
            rows.append(row)
            scene_rows.append(row)
        scene_rows.sort(key=lambda r: (r["mean_luma_abs"], r["changed_pixels"], r["max_luma_abs"]), reverse=True)
        scene_results.append({
            "scene": scene.name,
            "current": rel(current_path),
            "current_sha256": sha256(current_path),
            "original_viewport": rel(original_path),
            "original_viewport_sha256": sha256(original_path),
            "top_deltas": scene_rows[:8],
        })
    rows.sort(key=lambda r: (r["mean_luma_abs"], r["changed_pixels"], r["max_luma_abs"]), reverse=True)
    write_tsv(rows)
    result = {
        "schema": "pass178_v1_open_cell_item_pixel_probe.v1",
        "honesty": "Evidence only; source C2500 object anchors ranked by current-vs-original viewport pixel deltas.",
        "source_citations": SOURCE_CITATIONS,
        "zone_source": rel(ZONES_JSON),
        "zone_source_sha256": sha256(ZONES_JSON),
        "viewport_rect_in_current_screens": [VIEWPORT_X, VIEWPORT_Y, VIEWPORT_W, VIEWPORT_H],
        "window_radius": WINDOW_RADIUS,
        "anchor_family": "C2500_object_floor_item_open_cell",
        "anchor_count_total": len(anchors),
        "anchor_count_inside_viewport": sum(1 for a in anchors if 0 <= a["x"] < VIEWPORT_W and 0 <= a["y"] < VIEWPORT_H),
        "best_next_delta": rows[0] if rows else None,
        "top_deltas": rows[:20],
        "scenes": scene_results,
        "tsv": rel(TSV),
        "summary": rel(SUMMARY),
        "problems": problems,
        "pass": not problems and bool(rows),
    }
    STATS.write_text(json.dumps(result, indent=2) + "\n")
    write_summary(result)
    print(json.dumps({"pass": result["pass"], "rows": len(rows), "best_next_delta": result["best_next_delta"], "stats": rel(STATS), "summary": rel(SUMMARY), "problems": problems}, indent=2))
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
