#!/usr/bin/env python3
"""Generate source-anchor zone overlays for current Firestaff V1 captures.

This is evidence tooling, not a parity claim.  It overlays DM1/ReDMCSB-backed
source zones on the current Firestaff 320x200 screenshots and records exact
pixel measurements against extracted GRAPHICS.DAT anchor images where a static
anchor exists.
"""
from __future__ import annotations

import hashlib
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parent.parent
SCREEN_DIR = REPO / "verification-screens"
ANCHOR_DIR = REPO / "reference-artifacts" / "anchors"
PROVENANCE = REPO / "reference-artifacts" / "provenance.json"
OUT_DIR = REPO / "parity-evidence" / "overlays" / "pass82"
STATS = OUT_DIR / "pass82_firestaff_source_zone_overlay_stats.json"

FRAME_W = 320
FRAME_H = 200


@dataclass(frozen=True)
class Scene:
    index: str
    name: str
    file: str


@dataclass(frozen=True)
class Zone:
    name: str
    xywh: tuple[int, int, int, int]
    source_reference: str
    anchor: str | None = None


SCENES = (
    Scene("01", "ingame_start", "01_ingame_start_latest.png"),
    Scene("02", "ingame_turn_right", "02_ingame_turn_right_latest.png"),
    Scene("03", "ingame_move_forward", "03_ingame_move_forward_latest.png"),
    Scene("04", "ingame_spell_panel", "04_ingame_spell_panel_latest.png"),
    Scene("05", "ingame_after_cast", "05_ingame_after_cast_latest.png"),
    Scene("06", "ingame_inventory_panel", "06_ingame_inventory_panel_latest.png"),
)

# Coordinates are DM1 320x200 V1 screen coordinates used by existing parity
# passes.  Anchor names point at extracted GRAPHICS.DAT/ReDMCSB evidence from
# reference-artifacts/provenance.json.
ZONES = (
    Zone("viewport_C000", (0, 33, 224, 136), "C000_DERIVED_BITMAP_VIEWPORT; DEFS.H/COORD.C viewport aperture", "0000_viewport_full_frame.png"),
    Zone("action_area_C010", (224, 45, 87, 45), "C010_GRAPHIC_MENU_ACTION_AREA; m11_game_view.c comment and DEFS-family graphic index", "0010_action_area.png"),
    Zone("spell_area_C009", (224, 90, 87, 25), "C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND (DEFS.H:1815)", "0009_spell_area_backdrop.png"),
    Zone("right_column_action_spell", (224, 45, 87, 70), "Composite of C010 action area plus C009/C011 spell surface", None),
    Zone("inventory_panel_C020", (80, 53, 144, 73), "C020_GRAPHIC_PANEL_EMPTY (DEFS.H:1780)", "0020_panel_empty.png"),
    Zone("message_area", (0, 169, 224, 31), "DM1 message/text line area from DUNVIEW/PANEL/TEXT layout reconstruction", None),
)


def rel(path: Path) -> str:
    return str(path.resolve().relative_to(REPO))


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def load_rgb(path: Path):
    from PIL import Image

    img = Image.open(path).convert("RGB")
    if img.size != (FRAME_W, FRAME_H):
        raise SystemExit(f"{rel(path)} is {img.size}, expected {(FRAME_W, FRAME_H)}")
    return img


def diff_crop(crop, anchor_path: Path, out_mask: Path) -> dict[str, object]:
    from PIL import Image

    anchor = Image.open(anchor_path).convert("RGB")
    if anchor.size != crop.size:
        raise SystemExit(f"anchor size mismatch for {rel(anchor_path)}: {anchor.size} vs crop {crop.size}")
    w, h = crop.size
    mask = Image.new("RGB", (w, h), (255, 255, 255))
    cp = crop.load(); ap = anchor.load(); mp = mask.load()
    differing = 0
    max_delta = 0
    total_abs = 0
    for y in range(h):
        for x in range(w):
            cr, cg, cb = cp[x, y]
            ar, ag, ab = ap[x, y]
            dr, dg, db = abs(cr - ar), abs(cg - ag), abs(cb - ab)
            delta = max(dr, dg, db)
            if delta:
                differing += 1
                mp[x, y] = (255, 0, 0)
            max_delta = max(max_delta, delta)
            total_abs += dr + dg + db
    out_mask.parent.mkdir(parents=True, exist_ok=True)
    mask.save(out_mask, optimize=True)
    return {
        "anchor": rel(anchor_path),
        "anchor_sha256": sha256(anchor_path),
        "mask": rel(out_mask),
        "differing_pixels": differing,
        "total_pixels": w * h,
        "delta_percent": round(100.0 * differing / (w * h), 4),
        "max_channel_delta": max_delta,
        "mean_abs_delta_rgb": round(total_abs / (w * h * 3), 4),
    }


def draw_overlay(img, scene: Scene, out_path: Path) -> None:
    from PIL import ImageDraw, ImageFont

    overlay = img.copy()
    draw = ImageDraw.Draw(overlay)
    palette = [
        (255, 64, 64), (64, 255, 64), (64, 160, 255),
        (255, 220, 64), (255, 64, 255), (64, 255, 255),
    ]
    for idx, zone in enumerate(ZONES):
        x, y, w, h = zone.xywh
        color = palette[idx % len(palette)]
        draw.rectangle((x, y, x + w - 1, y + h - 1), outline=color, width=1)
        label = zone.name.split("_")[0]
        ty = max(0, y - 8) if y > 8 else y + 1
        draw.text((x + 1, ty), label, fill=color)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    overlay.save(out_path, optimize=True)


def main(_: Iterable[str] | None = None) -> int:
    rows: list[dict[str, object]] = []
    problems: list[str] = []
    for scene in SCENES:
        screen_path = SCREEN_DIR / scene.file
        if not screen_path.exists():
            problems.append(f"missing Firestaff screenshot: {rel(screen_path)}")
            continue
        img = load_rgb(screen_path)
        overlay_path = OUT_DIR / f"{scene.index}_{scene.name}_source_zones_overlay.png"
        draw_overlay(img, scene, overlay_path)
        scene_rows = []
        for zone in ZONES:
            x, y, w, h = zone.xywh
            crop = img.crop((x, y, x + w, y + h))
            crop_path = OUT_DIR / f"{scene.index}_{scene.name}_{zone.name}.png"
            crop.save(crop_path, optimize=True)
            zr: dict[str, object] = {
                "name": zone.name,
                "xywh": [x, y, w, h],
                "source_reference": zone.source_reference,
                "crop": rel(crop_path),
                "crop_sha256": sha256(crop_path),
            }
            if zone.anchor:
                anchor_path = ANCHOR_DIR / zone.anchor
                if anchor_path.exists():
                    zr["anchor_compare"] = diff_crop(crop, anchor_path, OUT_DIR / f"{scene.index}_{scene.name}_{zone.name}_anchor_mask.png")
                else:
                    problems.append(f"missing source anchor: {rel(anchor_path)}")
            scene_rows.append(zr)
        rows.append({
            "index": scene.index,
            "scene": scene.name,
            "screenshot": rel(screen_path),
            "screenshot_sha256": sha256(screen_path),
            "overlay": rel(overlay_path),
            "zones": scene_rows,
        })

    result = {
        "schema": "pass82_firestaff_source_zone_overlay_probe.v1",
        "honesty": "Evidence only. Static GRAPHICS.DAT anchor deltas are measurements, not parity claims; dynamic overlays and route/state differences must be interpreted against source.",
        "frame_size": [FRAME_W, FRAME_H],
        "source_anchor_provenance": rel(PROVENANCE),
        "scenes": rows,
        "problems": problems,
        "pass": not problems,
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    STATS.write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({"scenes": len(rows), "problems": problems, "stats": rel(STATS)}, indent=2))
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
