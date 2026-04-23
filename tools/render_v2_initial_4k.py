#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
from PIL import Image, ImageChops, ImageStat

ROOT = Path(__file__).resolve().parents[1]
CAPTURE_DIR = ROOT / "verification-screens/v2-initial-4k"
OUT_PATH = CAPTURE_DIR / "firestaff-v2-initial-ingame-4k.png"

SCREEN_ORIGIN = (320, 80)
SCREEN_SCALE = 10
VIEWPORT_RECT = (12, 24, 196, 112)
VIEWPORT_FRAME_ORIGIN = (0, 16)
ACTION_AREA_ORIGIN = (224, 45)
SPELL_AREA_ORIGIN = (224, 90)
PARTY_HUD_ORIGIN = (12, 160)


def upscale_rect(rect: tuple[int, int, int, int]) -> tuple[int, int, int, int]:
    x, y, w, h = rect
    return (
        SCREEN_ORIGIN[0] + x * SCREEN_SCALE,
        SCREEN_ORIGIN[1] + y * SCREEN_SCALE,
        w * SCREEN_SCALE,
        h * SCREEN_SCALE,
    )


def overlay(canvas: Image.Image, asset: Image.Image, origin_xy: tuple[int, int]) -> None:
    x = SCREEN_ORIGIN[0] + origin_xy[0] * SCREEN_SCALE
    y = SCREEN_ORIGIN[1] + origin_xy[1] * SCREEN_SCALE
    canvas.alpha_composite(asset.convert("RGBA"), (x, y))


def bbox_from_diff(base: Image.Image, creature: Image.Image) -> tuple[int, int, int, int] | None:
    diff = ImageChops.difference(base.crop((VIEWPORT_RECT[0], VIEWPORT_RECT[1], VIEWPORT_RECT[0] + VIEWPORT_RECT[2], VIEWPORT_RECT[1] + VIEWPORT_RECT[3])),
                                 creature.crop((VIEWPORT_RECT[0], VIEWPORT_RECT[1], VIEWPORT_RECT[0] + VIEWPORT_RECT[2], VIEWPORT_RECT[1] + VIEWPORT_RECT[3])))
    gray = diff.convert("L").point(lambda v: 255 if v > 18 else 0)
    box = gray.getbbox()
    if not box:
        return None
    stat = ImageStat.Stat(gray.crop(box))
    if stat.sum[0] < 255 * 24:
        return None
    return box


def main() -> None:
    base = Image.open(CAPTURE_DIR / "base_scene.ppm").convert("RGBA")
    creature = Image.open(CAPTURE_DIR / "creature_scene.ppm").convert("RGBA")
    canvas = Image.new("RGBA", (3840, 2160), (6, 5, 5, 255))

    scaled_screen = creature.resize((3200, 2000), Image.Resampling.NEAREST)
    canvas.alpha_composite(scaled_screen, SCREEN_ORIGIN)

    viewport = creature.crop((VIEWPORT_RECT[0], VIEWPORT_RECT[1], VIEWPORT_RECT[0] + VIEWPORT_RECT[2], VIEWPORT_RECT[1] + VIEWPORT_RECT[3]))
    vx, vy, vw, vh = upscale_rect(VIEWPORT_RECT)
    canvas.alpha_composite(viewport.resize((vw, vh), Image.Resampling.NEAREST), (vx, vy))

    overlay(canvas, Image.open(ROOT / "assets-v2/ui/wave1/vertical-slice/viewport-frame/masters/4k/fs-v2-slice-viewport-frame-base.4k.png"), VIEWPORT_FRAME_ORIGIN)
    overlay(canvas, Image.open(ROOT / "assets-v2/ui/wave1/vertical-slice/action-area/masters/4k/fs-v2-slice-action-area-base.4k.png"), ACTION_AREA_ORIGIN)
    overlay(canvas, Image.open(ROOT / "assets-v2/ui/wave1/spell-area/masters/4k/fs-v2-spell-area-base.4k.png"), SPELL_AREA_ORIGIN)
    overlay(canvas, Image.open(ROOT / "assets-v2/ui/wave1/vertical-slice/party-hud-four-slot/masters/4k/fs-v2-slice-party-hud-four-slot-base.4k.png"), PARTY_HUD_ORIGIN)
    overlay(canvas, Image.open(ROOT / "assets-v2/ui/wave1/vertical-slice/party-hud-four-slot/masters/4k/fs-v2-slice-party-hud-four-slot-active-slot-overlay.4k.png"), PARTY_HUD_ORIGIN)

    creature_box = bbox_from_diff(base, creature)
    if creature_box:
        cx = vx + creature_box[0] * SCREEN_SCALE
        cy = vy + creature_box[1] * SCREEN_SCALE
        cw = (creature_box[2] - creature_box[0]) * SCREEN_SCALE
        ch = (creature_box[3] - creature_box[1]) * SCREEN_SCALE
    else:
        cx = vx + vw // 2 - 360
        cy = vy + vh // 2 - 80
        cw = 720
        ch = 820
    skeleton = Image.open(ROOT / "assets-v2/creatures/wave1/skeleton-family/masters/4k/fs-v2-skeleton-front-near.4k.png").convert("RGBA")
    target_h = max(ch + 120, 720)
    target_w = int(skeleton.width * (target_h / skeleton.height))
    skeleton = skeleton.resize((target_w, target_h), Image.Resampling.LANCZOS)
    sx = cx + (cw - target_w) // 2
    sy = cy + ch - target_h + 30
    canvas.alpha_composite(skeleton, (sx, sy))

    CAPTURE_DIR.mkdir(parents=True, exist_ok=True)
    canvas.save(OUT_PATH)
    print(f"wrote {OUT_PATH.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
