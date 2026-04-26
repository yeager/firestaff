#!/usr/bin/env python3
"""Overlay DM1 layout-696 viewport content anchor points on V1 screenshots.

Evidence only: this marks ReDMCSB/GRAPHICS.DAT zone points for object,
projectile, and creature placement. It does not claim pixel parity.
"""
from __future__ import annotations

import hashlib
import json
import struct
from dataclasses import dataclass
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
SCREEN_DIR = REPO / "verification-screens"
ZONES_JSON = REPO / "zones_h_reconstruction.json"
OUT_DIR = REPO / "parity-evidence" / "overlays" / "pass83"
STATS = OUT_DIR / "pass83_firestaff_viewport_content_points_stats.json"

FRAME_W = 320
FRAME_H = 200
VIEWPORT_X = 0
VIEWPORT_Y = 33
VIEWPORT_W = 224
VIEWPORT_H = 136


@dataclass(frozen=True)
class Scene:
    index: str
    name: str
    file: str


SCENES = (
    Scene("01", "ingame_start", "01_ingame_start_latest.png"),
    Scene("02", "ingame_turn_right", "02_ingame_turn_right_latest.png"),
    Scene("03", "ingame_move_forward", "03_ingame_move_forward_latest.png"),
    Scene("04", "ingame_spell_panel", "04_ingame_spell_panel_latest.png"),
    Scene("05", "ingame_after_cast", "05_ingame_after_cast_latest.png"),
    Scene("06", "ingame_inventory_panel", "06_ingame_inventory_panel_latest.png"),
)


def rel(path: Path) -> str:
    return str(path.resolve().relative_to(REPO))


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def png_size(path: Path) -> tuple[int, int]:
    with path.open("rb") as f:
        sig = f.read(8)
        if sig != b"\x89PNG\r\n\x1a\n":
            raise ValueError(f"not a PNG: {rel(path)}")
        length = struct.unpack(">I", f.read(4))[0]
        chunk = f.read(4)
        if length != 13 or chunk != b"IHDR":
            raise ValueError(f"missing IHDR: {rel(path)}")
        width, height = struct.unpack(">II", f.read(8))
        return int(width), int(height)


def load_records() -> tuple[dict[int, dict[str, int]], dict[str, object]]:
    data = json.loads(ZONES_JSON.read_text())
    return {int(k): v for k, v in data["records"].items()}, data.get("$provenance", {})


def point_row(family: str, zone: int, label: str, x: int, y: int) -> dict[str, object]:
    sx = VIEWPORT_X + x
    sy = VIEWPORT_Y + y
    return {
        "family": family,
        "zone": zone,
        "label": label,
        "viewport_xy": [x, y],
        "screen_xy": [sx, sy],
        "inside_viewport": (0 <= x < VIEWPORT_W and 0 <= y < VIEWPORT_H),
    }


def collect_points(records: dict[int, dict[str, int]]) -> list[dict[str, object]]:
    points: list[dict[str, object]] = []

    # C2500 and C2900 are 5 G2030 scale buckets × 4 relative cells.
    for base, family in ((2500, "C2500_object"), (2900, "C2900_projectile")):
        for scale in range(5):
            for rel_cell in range(4):
                zone = base + scale * 4 + rel_cell
                rec = records[zone]
                if rec["d1"] == 0 and rec["d2"] == 0:
                    continue
                points.append(point_row(
                    family,
                    zone,
                    f"{family} s{scale} r{rel_cell}",
                    int(rec["d1"]),
                    int(rec["d2"]),
                ))

    # C3200 records are arranged D3, D2, D1; each depth has center, left side,
    # then right side groups with five creature slot points. Keep source order.
    zone = 3200
    for depth_name in ("D3", "D2", "D1"):
        for group_name in ("center", "left", "right"):
            for slot in range(5):
                rec = records[zone]
                points.append(point_row(
                    "C3200_creature",
                    zone,
                    f"C3200 {depth_name} {group_name} p{slot}",
                    int(rec["d1"]),
                    int(rec["d2"]),
                ))
                zone += 1

    return points


def svg_href(target: Path) -> str:
    return Path("../../..").joinpath(target.relative_to(REPO)).as_posix()


def draw_overlay_svg(screen_path: Path, points: list[dict[str, object]], out_path: Path) -> None:
    colors = {
        "C2500_object": "#ffdc40",
        "C2900_projectile": "#40dcff",
        "C3200_creature": "#ff50b4",
    }
    parts = [
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>",
        f"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"{FRAME_W}\" height=\"{FRAME_H}\" viewBox=\"0 0 {FRAME_W} {FRAME_H}\">",
        f"  <image href=\"{svg_href(screen_path)}\" x=\"0\" y=\"0\" width=\"{FRAME_W}\" height=\"{FRAME_H}\"/>",
        f"  <rect x=\"{VIEWPORT_X}\" y=\"{VIEWPORT_Y}\" width=\"{VIEWPORT_W}\" height=\"{VIEWPORT_H}\" fill=\"none\" stroke=\"#ffffff\" stroke-width=\"1\"/>",
        "  <text x=\"2\" y=\"30\" fill=\"#ffffff\" font-family=\"monospace\" font-size=\"7\">C2500 objects / C2900 projectiles / C3200 creatures</text>",
    ]
    for row in points:
        sx, sy = row["screen_xy"]
        color = colors[row["family"]]
        if row["inside_viewport"]:
            parts.append(f"  <g stroke=\"{color}\" fill=\"none\" stroke-width=\"1\"><line x1=\"{sx-3}\" y1=\"{sy}\" x2=\"{sx+3}\" y2=\"{sy}\"/><line x1=\"{sx}\" y1=\"{sy-3}\" x2=\"{sx}\" y2=\"{sy+3}\"/><circle cx=\"{sx}\" cy=\"{sy}\" r=\"2\"/></g>")
        else:
            cx = min(max(int(sx), VIEWPORT_X), VIEWPORT_X + VIEWPORT_W - 1)
            cy = min(max(int(sy), VIEWPORT_Y), VIEWPORT_Y + VIEWPORT_H - 1)
            ex = min(max(int(sx), 0), FRAME_W - 1)
            ey = min(max(int(sy), 0), FRAME_H - 1)
            parts.append(f"  <g stroke=\"{color}\" fill=\"none\" stroke-width=\"1\"><line x1=\"{cx}\" y1=\"{cy}\" x2=\"{ex}\" y2=\"{ey}\"/><rect x=\"{cx-2}\" y=\"{cy-2}\" width=\"4\" height=\"4\"/></g>")
    parts.append("</svg>")
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(parts) + "\n")


def main() -> int:
    records, provenance = load_records()
    points = collect_points(records)
    problems: list[str] = []
    scenes: list[dict[str, object]] = []
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    for scene in SCENES:
        screen_path = SCREEN_DIR / scene.file
        if not screen_path.exists():
            problems.append(f"missing Firestaff screenshot: {rel(screen_path)}")
            continue
        try:
            size = png_size(screen_path)
        except ValueError as exc:
            problems.append(str(exc))
            continue
        if size != (FRAME_W, FRAME_H):
            problems.append(f"{rel(screen_path)} is {size}, expected {(FRAME_W, FRAME_H)}")
            continue
        overlay_path = OUT_DIR / f"{scene.index}_{scene.name}_viewport_content_points.svg"
        draw_overlay_svg(screen_path, points, overlay_path)
        scenes.append({
            "index": scene.index,
            "scene": scene.name,
            "screenshot": rel(screen_path),
            "screenshot_sha256": sha256(screen_path),
            "overlay": rel(overlay_path),
            "overlay_sha256": sha256(overlay_path),
        })

    family_counts: dict[str, int] = {}
    family_inside: dict[str, int] = {}
    for row in points:
        fam = str(row["family"])
        family_counts[fam] = family_counts.get(fam, 0) + 1
        if row["inside_viewport"]:
            family_inside[fam] = family_inside.get(fam, 0) + 1

    result = {
        "schema": "pass83_firestaff_viewport_content_points_probe.v1",
        "honesty": "Evidence only. Points are layout-696 source anchors from GRAPHICS.DAT/ReDMCSB reconstruction; overlays do not claim sprite pixel parity.",
        "source_zone_reconstruction": rel(ZONES_JSON),
        "source_zone_reconstruction_sha256": sha256(ZONES_JSON),
        "source_provenance": provenance,
        "viewport_screen_rect": [VIEWPORT_X, VIEWPORT_Y, VIEWPORT_W, VIEWPORT_H],
        "families": family_counts,
        "inside_viewport_by_family": family_inside,
        "points": points,
        "scenes": scenes,
        "problems": problems,
        "pass": not problems,
    }
    STATS.write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({"points": len(points), "scenes": len(scenes), "problems": problems, "stats": rel(STATS)}, indent=2))
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
