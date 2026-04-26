#!/usr/bin/env python3
"""Build compact V1 champion-HUD screenshot evidence from current Firestaff captures.

Evidence tooling only: crops the top-row champion HUD from deterministic current
Firestaff V1 screenshots, emits zone-marked companion crops, and writes a
checksum manifest. This does not claim original-runtime pixel parity.
"""
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import sys
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parent.parent
PASS83_PATH = REPO / "tools" / "pass83_champion_hud_zone_overlay_probe.py"
OUT_DIR = REPO / "parity-evidence" / "overlays" / "pass85_v1_hud_screenshot_delivery"
MD = REPO / "parity-evidence" / "pass85_v1_hud_screenshot_delivery.md"
STATS = OUT_DIR / "pass85_v1_hud_screenshot_delivery_stats.json"
MANIFEST = OUT_DIR / "pass85_v1_hud_screenshot_delivery_sha256.tsv"
HUD_BOX = (12, 0, 274, 29)  # four 67x29 champion slots plus 3 source-layout gaps
SCENES = (
    ("party_hud_with_champions", "verification-screens/07_party_hud_with_champions.png"),
    ("spell_panel_with_champions", "verification-screens/08_spell_panel_with_champions.png"),
    ("four_champion_priority", "verification-screens/pass81-champion-hud-priority/party_hud_four_champions_vga.png"),
)


def _load_pass83():
    spec = importlib.util.spec_from_file_location("pass83_hud", PASS83_PATH)
    if spec is None or spec.loader is None:
        raise SystemExit("cannot import {}".format(PASS83_PATH.relative_to(REPO)))
    mod = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = mod
    spec.loader.exec_module(mod)
    return mod


def rel(path: Path) -> str:
    return str(path.resolve().relative_to(REPO))


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def crop_metrics(img) -> dict[str, object]:
    nonblack = sum(1 for p in img.pixels if p != (0, 0, 0))
    return {
        "width": img.width,
        "height": img.height,
        "nonblack_pixels": nonblack,
        "total_pixels": len(img.pixels),
        "nonblack_ratio": round(nonblack / len(img.pixels), 4),
        "unique_colors": len(set(img.pixels)),
    }


def draw_hud_overlay(mod, img):
    overlay = img.copy()
    x0, y0, w0, h0 = HUD_BOX
    overlay.rect(x0, y0, w0, h0, (255, 255, 255))
    for zone in mod.ZONES:
        x, y, w, h = zone.xywh
        if "status_box" in zone.name:
            color = (255, 64, 64)
        elif "name" in zone.name:
            color = (255, 220, 64)
        elif "hand" in zone.name:
            color = (64, 255, 255)
        else:
            color = (64, 255, 64)
        overlay.rect(x, y, w, h, color)
    return overlay


def run_self_test() -> int:
    assert HUD_BOX == (12, 0, 274, 29)
    x, y, w, h = HUD_BOX
    assert x + w == 286 and y + h == 29
    print(json.dumps({"pass": True, "hud_box": list(HUD_BOX)}, indent=2))
    return 0


def write_markdown(result: dict[str, object]) -> None:
    lines = [
        "# Pass 85 — V1 champion HUD screenshot delivery",
        "",
        "This pass packages small, reviewable top-row champion HUD screenshots from the current Firestaff V1 capture state.",
        "",
        "Honesty: these are current Firestaff evidence artifacts only. They do not claim original-runtime pixel parity.",
        "",
        "- stats: `{}`".format(result["stats"]),
        "- checksum manifest: `{}`".format(result["manifest"]),
        "- HUD crop box: `{}` (x, y, width, height)".format(result["hud_box"]),
        "- scenes: {}".format(len(result["scenes"])),
        "",
        "## Delivered artifacts",
        "",
    ]
    for scene in result["scenes"]:  # type: ignore[index]
        lines.append("- `{}` from `{}`".format(scene["scene"], scene["source"]))
        lines.append("  - raw HUD crop: `{}`".format(scene["hud_crop"]))
        lines.append("  - zoned HUD crop: `{}`".format(scene["zoned_hud_crop"]))
    lines.extend([
        "",
        "## Notes",
        "",
        "- The raw crop is the top-row champion HUD rectangle only, kept small for quick visual review.",
        "- The zoned crop uses the same DM1 PC 3.4 layout-696 zone geometry as pass 83, clipped to the HUD row.",
        "- Checksums in the TSV manifest fingerprint the exact bytes committed in this pass.",
        "",
    ])
    MD.write_text("\n".join(lines))


def main(argv: Iterable[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args(argv)
    if args.self_test:
        return run_self_test()

    mod = _load_pass83()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    x, y, w, h = HUD_BOX
    scenes: list[dict[str, object]] = []
    manifest_rows: list[tuple[str, Path, int, int, str]] = []
    problems: list[str] = []

    for scene_name, source_rel in SCENES:
        source = REPO / source_rel
        if not source.exists():
            problems.append("missing source screenshot: {}".format(source_rel))
            continue
        img = mod.load_rgb(source)
        raw_crop = img.crop((x, y, x + w, y + h))
        zoned_crop = draw_hud_overlay(mod, img).crop((x, y, x + w, y + h))
        raw_path = OUT_DIR / "{}_top_row_hud_raw.png".format(scene_name)
        zoned_path = OUT_DIR / "{}_top_row_hud_zoned.png".format(scene_name)
        mod.save_png(raw_crop, raw_path)
        mod.save_png(zoned_crop, zoned_path)
        manifest_rows.extend([
            ("source", source, img.width, img.height, sha256(source)),
            ("hud_raw", raw_path, raw_crop.width, raw_crop.height, sha256(raw_path)),
            ("hud_zoned", zoned_path, zoned_crop.width, zoned_crop.height, sha256(zoned_path)),
        ])
        scenes.append({
            "scene": scene_name,
            "source": source_rel,
            "source_sha256": sha256(source),
            "hud_crop": rel(raw_path),
            "hud_crop_sha256": sha256(raw_path),
            "hud_crop_metrics": crop_metrics(raw_crop),
            "zoned_hud_crop": rel(zoned_path),
            "zoned_hud_crop_sha256": sha256(zoned_path),
            "zoned_hud_crop_metrics": crop_metrics(zoned_crop),
        })

    with MANIFEST.open("w") as f:
        f.write("# Pass 85 V1 champion HUD screenshot delivery checksum manifest\n")
        f.write("# kind\tpath\twidth\theight\tsha256\n")
        for kind, path, width, height, digest in manifest_rows:
            f.write("{}\t{}\t{}\t{}\t{}\n".format(kind, rel(path), width, height, digest))

    result = {
        "schema": "pass85_v1_hud_screenshot_delivery.v1",
        "honesty": "Current Firestaff V1 evidence only; no original-runtime pixel parity claim.",
        "hud_box": list(HUD_BOX),
        "scenes": scenes,
        "problems": problems,
        "manifest": rel(MANIFEST),
        "stats": rel(STATS),
        "pass": len(scenes) == len(SCENES) and not problems,
    }
    STATS.write_text(json.dumps(result, indent=2) + "\n")
    write_markdown(result)
    print(json.dumps({"scenes": len(scenes), "problems": problems, "manifest": rel(MANIFEST), "stats": rel(STATS), "markdown": rel(MD)}, indent=2))
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
