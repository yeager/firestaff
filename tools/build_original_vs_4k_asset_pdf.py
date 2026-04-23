#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
from typing import Optional
from PIL import Image, ImageDraw, ImageFont

REPO = Path(__file__).resolve().parent.parent
OUT_DIR = REPO / "docs" / "reports"
OUT_PDF = OUT_DIR / "original-vs-v2-4k-asset-comparison.pdf"

PAGE_W = 2550
PAGE_H = 3300
MARGIN = 120
GUTTER = 80
HEADER_H = 260
FOOTER_H = 120
PANEL_W = (PAGE_W - MARGIN * 2 - GUTTER) // 2
PANEL_H = PAGE_H - HEADER_H - FOOTER_H - MARGIN
V2_OFFSET_X = 320
V2_OFFSET_Y = 80
V2_SCALE = 10

try:
    TITLE_FONT = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial Bold.ttf", 56)
    SUB_FONT = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 28)
    BODY_FONT = ImageFont.truetype("/System/Library/Fonts/Supplemental/Courier New.ttf", 24)
except OSError:
    TITLE_FONT = ImageFont.load_default()
    SUB_FONT = ImageFont.load_default()
    BODY_FONT = ImageFont.load_default()


def v2_box(x1: int, y1: int, x2: int, y2: int) -> tuple[int, int, int, int]:
    return (
        V2_OFFSET_X + x1 * V2_SCALE,
        V2_OFFSET_Y + y1 * V2_SCALE,
        V2_OFFSET_X + x2 * V2_SCALE,
        V2_OFFSET_Y + y2 * V2_SCALE,
    )


def asset(path: str, crop: Optional[tuple[int, int, int, int]] = None) -> dict:
    return {"path": str(REPO / path), "crop": crop}


COMPARISONS = [
    {
        "title": "Viewport frame base",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0000.pgm"),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/viewport-frame/masters/4k/fs-v2-slice-viewport-frame-base.4k.png"),
        "notes": "Original DM1 viewport frame graphic 0000 vs first-pass 4K V2 frame shell.",
    },
    {
        "title": "Viewport aperture / inner mask",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0000.pgm", (8, 8, 216, 128)),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/viewport-frame/masters/4k/fs-v2-slice-viewport-frame-inner-mask.4k.png"),
        "notes": "Original viewport opening crop vs 4K integration mask asset.",
    },
    {
        "title": "Action area base",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0010.pgm"),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/action-area/masters/4k/fs-v2-slice-action-area-base.4k.png"),
        "notes": "Original DM1 action strip (graphic 0010) vs V2 base shell.",
    },
    {
        "title": "Action area recess bed",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0010.pgm", (6, 10, 81, 39)),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/action-area/masters/4k/fs-v2-slice-action-area-recess-bed.4k.png"),
        "notes": "Interior crop of the DM1 action strip vs V2 icon/text bed.",
    },
    {
        "title": "Action area highlight state",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0010.pgm", (6, 10, 81, 39)),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/action-area/masters/4k/fs-v2-slice-action-area-highlight-overlay.4k.png"),
        "notes": "Reference interior vs V2 highlight overlay.",
    },
    {
        "title": "Action area active state",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0010.pgm", (6, 10, 81, 39)),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/action-area/masters/4k/fs-v2-slice-action-area-active-overlay.4k.png"),
        "notes": "Reference interior vs V2 active overlay.",
    },
    {
        "title": "Spell area base",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0009.pgm"),
        "v2": asset("assets-v2/ui/wave1/spell-area/masters/4k/fs-v2-spell-area-base.4k.png"),
        "notes": "Original DM1 spell strip (graphic 0009) vs V2 spell-area base.",
    },
    {
        "title": "Spell area rune bed",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0009.pgm", (6, 6, 81, 19)),
        "v2": asset("assets-v2/ui/wave1/spell-area/masters/4k/fs-v2-spell-area-rune-bed.4k.png"),
        "notes": "Interior crop of the DM1 spell strip vs V2 rune bed.",
    },
    {
        "title": "Spell area highlight state",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0009.pgm", (6, 6, 81, 19)),
        "v2": asset("assets-v2/ui/wave1/spell-area/masters/4k/fs-v2-spell-area-highlight-overlay.4k.png"),
        "notes": "Reference spell-strip interior vs V2 highlight overlay.",
    },
    {
        "title": "Spell area active state",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0009.pgm", (6, 6, 81, 19)),
        "v2": asset("assets-v2/ui/wave1/spell-area/masters/4k/fs-v2-spell-area-active-overlay.4k.png"),
        "notes": "Reference spell-strip interior vs V2 active overlay.",
    },
    {
        "title": "Status box left frame",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0007.pgm"),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/status-box-family/masters/4k/fs-v2-slice-status-box-left-frame.4k.png"),
        "notes": "Original DM1 status box shell (graphic 0007) vs V2 left frame.",
    },
    {
        "title": "Status box right frame",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0008.pgm"),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/status-box-family/masters/4k/fs-v2-slice-status-box-right-frame.4k.png"),
        "notes": "Original DM1 status box shell (graphic 0008) vs V2 right frame.",
    },
    {
        "title": "Party HUD cell standard",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0033.pgm"),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/party-hud-cell-family/masters/4k/fs-v2-slice-party-hud-cell-standard-base.4k.png"),
        "notes": "Original DM1 slot box 0033 vs V2 HUD cell base.",
    },
    {
        "title": "Party HUD cell highlight",
        "kind": "asset",
        "original": asset("extracted-graphics-v1/pgm/graphic_0034.pgm"),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/party-hud-cell-family/masters/4k/fs-v2-slice-party-hud-cell-highlight-overlay.4k.png"),
        "notes": "Original slot-box family 0034 vs V2 highlight overlay.",
    },
    {
        "title": "Party HUD four-slot base",
        "kind": "composite",
        "original": asset("verification-m11/game-view/party_hud_statusbox_gfx.pgm", (0, 145, 224, 200)),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/party-hud-four-slot/masters/4k/fs-v2-slice-party-hud-four-slot-base.4k.png"),
        "notes": "Original bottom-row runtime crop vs V2 shared 4-slot party strip.",
    },
    {
        "title": "Party HUD four-slot active slot",
        "kind": "composite",
        "original": asset("verification-m11/game-view/party_hud_statusbox_gfx.pgm", (0, 145, 224, 200)),
        "v2": asset("assets-v2/ui/wave1/vertical-slice/party-hud-four-slot/masters/4k/fs-v2-slice-party-hud-four-slot-active-slot-overlay.4k.png"),
        "notes": "Original active-party-row runtime crop vs V2 active-slot emphasis.",
    },
    {
        "title": "Skeleton family — near encounter",
        "kind": "creature",
        "original": asset("verification-screens/v2-initial-4k/creature_scene.ppm", (70, 30, 170, 135)),
        "v2": asset("assets-v2/creatures/wave1/skeleton-family/masters/4k/fs-v2-skeleton-front-near.4k.png"),
        "notes": "Low-resolution live creature capture vs first-pass 4K skeleton near variant.",
    },
    {
        "title": "Skeleton family — mid encounter",
        "kind": "creature",
        "original": asset("verification-screens/v2-initial-4k/creature_scene.ppm", (85, 45, 160, 120)),
        "v2": asset("assets-v2/creatures/wave1/skeleton-family/masters/4k/fs-v2-skeleton-front-mid.4k.png"),
        "notes": "Low-resolution live creature capture vs first-pass 4K skeleton mid variant.",
    },
    {
        "title": "Skeleton family — far encounter",
        "kind": "creature",
        "original": asset("verification-screens/v2-initial-4k/creature_scene.ppm", (95, 55, 150, 108)),
        "v2": asset("assets-v2/creatures/wave1/skeleton-family/masters/4k/fs-v2-skeleton-front-far.4k.png"),
        "notes": "Low-resolution live creature capture vs first-pass 4K skeleton far variant.",
    },
    {
        "title": "Composite screen region — viewport",
        "kind": "screen",
        "original": asset("verification-screens/01_ingame_start_latest.png", (0, 16, 224, 152)),
        "v2": asset("verification-screens/v2-initial-4k/firestaff-v2-initial-ingame-4k.png", v2_box(0, 16, 224, 152)),
        "notes": "Original V1 viewport region vs first 4K V2 in-game composition viewport.",
    },
    {
        "title": "Composite screen region — right column",
        "kind": "screen",
        "original": asset("verification-screens/01_ingame_start_latest.png", (224, 16, 320, 152)),
        "v2": asset("verification-screens/v2-initial-4k/firestaff-v2-initial-ingame-4k.png", v2_box(224, 16, 320, 152)),
        "notes": "Original V1 right column vs 4K V2 right-column composition.",
    },
    {
        "title": "Composite screen region — action strip",
        "kind": "screen",
        "original": asset("verification-screens/01_ingame_start_latest.png", (224, 45, 311, 90)),
        "v2": asset("verification-screens/v2-initial-4k/firestaff-v2-initial-ingame-4k.png", v2_box(224, 45, 311, 90)),
        "notes": "Original action strip runtime crop vs 4K V2 action strip.",
    },
    {
        "title": "Composite screen region — spell strip",
        "kind": "screen",
        "original": asset("verification-screens/04_ingame_spell_panel_latest.png", (224, 20, 311, 45)),
        "v2": asset("verification-screens/v2-initial-4k/firestaff-v2-initial-ingame-4k.png", v2_box(224, 20, 311, 45)),
        "notes": "Original spell-strip runtime crop vs 4K V2 spell strip.",
    },
    {
        "title": "Composite screen region — party HUD row",
        "kind": "screen",
        "original": asset("verification-screens/01_ingame_start_latest.png", (0, 145, 224, 200)),
        "v2": asset("verification-screens/v2-initial-4k/firestaff-v2-initial-ingame-4k.png", v2_box(0, 145, 224, 200)),
        "notes": "Original bottom-row runtime crop vs 4K V2 party HUD row.",
    },
    {
        "title": "Composite screen region — full in-game composition",
        "kind": "screen",
        "original": asset("verification-screens/01_ingame_start_latest.png"),
        "v2": asset("verification-screens/v2-initial-4k/firestaff-v2-initial-ingame-4k.png", v2_box(0, 0, 320, 200)),
        "notes": "Original V1 in-game screenshot vs first bounded V2 4K in-game composition.",
    },
]

assert len(COMPARISONS) == 25, len(COMPARISONS)


def load_image(spec: dict) -> Image.Image:
    img = Image.open(spec["path"]).convert("RGB")
    if spec.get("crop"):
        img = img.crop(spec["crop"])
    return img


def fit_panel(img: Image.Image, width: int, height: int, upscale_pixel_art: bool = False) -> Image.Image:
    scale = min(width / img.width, height / img.height)
    size = (max(1, int(img.width * scale)), max(1, int(img.height * scale)))
    resample = Image.Resampling.NEAREST if upscale_pixel_art else Image.Resampling.LANCZOS
    return img.resize(size, resample=resample)


def text_block(draw: ImageDraw.ImageDraw, xy: tuple[int, int], text: str, font, fill=(40, 40, 40), line_gap: int = 8):
    x, y = xy
    for line in text.split("\n"):
        draw.text((x, y), line, font=font, fill=fill)
        bbox = draw.textbbox((x, y), line, font=font)
        y += (bbox[3] - bbox[1]) + line_gap
    return y


def build_pages() -> list[Image.Image]:
    pages = []
    for index, entry in enumerate(COMPARISONS, start=1):
        page = Image.new("RGB", (PAGE_W, PAGE_H), (250, 247, 241))
        draw = ImageDraw.Draw(page)
        draw.text((MARGIN, 55), f"Original vs 4K asset comparison — {index}/25", font=SUB_FONT, fill=(90, 90, 90))
        draw.text((MARGIN, 95), entry["title"], font=TITLE_FONT, fill=(15, 15, 15))
        draw.rounded_rectangle((MARGIN, HEADER_H, MARGIN + PANEL_W, HEADER_H + PANEL_H), 24, outline=(180, 170, 155), width=4, fill=(255,255,255))
        draw.rounded_rectangle((MARGIN + PANEL_W + GUTTER, HEADER_H, MARGIN + PANEL_W + GUTTER + PANEL_W, HEADER_H + PANEL_H), 24, outline=(180, 170, 155), width=4, fill=(255,255,255))
        draw.text((MARGIN + 28, HEADER_H + 20), "Original / reference", font=SUB_FONT, fill=(40, 40, 40))
        draw.text((MARGIN + PANEL_W + GUTTER + 28, HEADER_H + 20), "Firestaff 4K asset / composition", font=SUB_FONT, fill=(40, 40, 40))

        orig = load_image(entry["original"])
        v2 = load_image(entry["v2"])
        orig_fit = fit_panel(orig, PANEL_W - 60, PANEL_H - 220, upscale_pixel_art=True)
        v2_fit = fit_panel(v2, PANEL_W - 60, PANEL_H - 220, upscale_pixel_art=False)

        ox = MARGIN + (PANEL_W - orig_fit.width) // 2
        oy = HEADER_H + 80 + (PANEL_H - 250 - orig_fit.height) // 2
        vx = MARGIN + PANEL_W + GUTTER + (PANEL_W - v2_fit.width) // 2
        vy = HEADER_H + 80 + (PANEL_H - 250 - v2_fit.height) // 2
        page.paste(orig_fit, (ox, oy))
        page.paste(v2_fit, (vx, vy))

        draw.text((MARGIN + 28, HEADER_H + PANEL_H - 150), f"Source: {Path(entry['original']['path']).relative_to(REPO)}", font=BODY_FONT, fill=(70, 70, 70))
        draw.text((MARGIN + PANEL_W + GUTTER + 28, HEADER_H + PANEL_H - 150), f"Source: {Path(entry['v2']['path']).relative_to(REPO)}", font=BODY_FONT, fill=(70, 70, 70))
        draw.line((MARGIN, PAGE_H - FOOTER_H - 20, PAGE_W - MARGIN, PAGE_H - FOOTER_H - 20), fill=(180,170,155), width=2)
        text_block(draw, (MARGIN, PAGE_H - FOOTER_H + 8), entry["notes"], BODY_FONT, fill=(55,55,55), line_gap=6)
        pages.append(page)
    return pages


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    pages = build_pages()
    first, rest = pages[0], pages[1:]
    first.save(OUT_PDF, save_all=True, append_images=rest, resolution=150.0)
    print(f"wrote {OUT_PDF}")


if __name__ == "__main__":
    main()
