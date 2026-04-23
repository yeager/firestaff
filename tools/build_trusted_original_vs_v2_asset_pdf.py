#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

REPO = Path(__file__).resolve().parent.parent
OUT_DIR = REPO / "docs" / "reports"
OUT_PDF = OUT_DIR / "original-vs-v2-4k-asset-comparison-trusted-scope.pdf"

PAGE_W = 2550
PAGE_H = 3300
MARGIN = 120
GUTTER = 80
HEADER_H = 360
FOOTER_H = 240
PANEL_W = (PAGE_W - MARGIN * 2 - GUTTER) // 2
PANEL_H = PAGE_H - HEADER_H - FOOTER_H - MARGIN

try:
    TITLE_FONT = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial Bold.ttf", 56)
    H2_FONT = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial Bold.ttf", 36)
    SUB_FONT = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 28)
    BODY_FONT = ImageFont.truetype("/System/Library/Fonts/Supplemental/Courier New.ttf", 24)
except OSError:
    TITLE_FONT = ImageFont.load_default()
    H2_FONT = ImageFont.load_default()
    SUB_FONT = ImageFont.load_default()
    BODY_FONT = ImageFont.load_default()

ENTRIES = [
    {
        "index": "0009",
        "trusted_label": "Spell area background",
        "status": "confirmed-correct mapping",
        "evidence": "ReDMCSB C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND; export size 87x25; Firestaff usage comments align.",
        "original": REPO / "extracted-graphics-v1/pgm/graphic_0009.pgm",
        "v2": REPO / "assets-v2/ui/wave1/spell-area/masters/4k/fs-v2-spell-area-base.4k.png",
        "v2_label": "Firestaff V2 spell area base",
        "notes": "Trusted original-reference anchor. Included because the mapping is confirmed by the audit and the V2 asset is an explicit spell-area base counterpart.",
    },
    {
        "index": "0010",
        "trusted_label": "Action area background",
        "status": "confirmed-correct mapping",
        "evidence": "ReDMCSB C010_GRAPHIC_MENU_ACTION_AREA; export size 87x45; Firestaff usage comments align.",
        "original": REPO / "extracted-graphics-v1/pgm/graphic_0010.pgm",
        "v2": REPO / "assets-v2/ui/wave1/vertical-slice/action-area/masters/4k/fs-v2-slice-action-area-base.4k.png",
        "v2_label": "Firestaff V2 action area base",
        "notes": "Trusted original-reference anchor. Included because the mapping is confirmed by the audit and the V2 asset is an explicit action-area base counterpart.",
    },
    {
        "index": "0033",
        "trusted_label": "Slot box normal",
        "status": "confirmed-correct mapping",
        "evidence": "ReDMCSB C033_GRAPHIC_SLOT_BOX_NORMAL; export size 18x18; frontend slot-box selection logic matches.",
        "original": REPO / "extracted-graphics-v1/pgm/graphic_0033.pgm",
        "v2": REPO / "assets-v2/ui/wave1/vertical-slice/party-hud-cell-family/masters/4k/fs-v2-slice-party-hud-cell-standard-base.4k.png",
        "v2_label": "Firestaff V2 party HUD cell standard base",
        "notes": "Trusted original-reference anchor. Included because the mapping is confirmed by the audit and the V2 asset is the direct standard/base counterpart.",
    },
]

EXCLUDED = [
    "0020 panel empty — trusted mapping, but no clear like-for-like V2 asset counterpart is present in the repo yet.",
    "0035 slot box acting hand — trusted mapping, but no distinct acting-hand V2 asset counterpart is present in the repo yet.",
    "0303 stone lock left side — trusted mapping, but no corresponding Firestaff V2 ornate-lock comparison asset is present in the repo yet.",
    "0304 stone lock front — trusted mapping, but no corresponding Firestaff V2 ornate-lock comparison asset is present in the repo yet.",
    "0000, 0007, 0008, 0034, 0078, 0079 — intentionally excluded because the audit marks them suspicious or confirms the prior comparison mapping was wrong.",
]


def fit_panel(img: Image.Image, width: int, height: int, *, pixel_art: bool) -> Image.Image:
    scale = min(width / img.width, height / img.height)
    size = (max(1, int(img.width * scale)), max(1, int(img.height * scale)))
    resample = Image.Resampling.NEAREST if pixel_art else Image.Resampling.LANCZOS
    return img.resize(size, resample=resample)



def load_image(path: Path) -> Image.Image:
    return Image.open(path).convert("RGB")



def text_block(draw: ImageDraw.ImageDraw, xy: tuple[int, int], text: str, font, *, fill=(40, 40, 40), line_gap: int = 8) -> int:
    x, y = xy
    for line in text.split("\n"):
        draw.text((x, y), line, font=font, fill=fill)
        bbox = draw.textbbox((x, y), line, font=font)
        y += (bbox[3] - bbox[1]) + line_gap
    return y



def build_cover_page() -> Image.Image:
    page = Image.new("RGB", (PAGE_W, PAGE_H), (250, 247, 241))
    draw = ImageDraw.Draw(page)
    draw.text((MARGIN, 90), "Trusted-scope original vs V2 comparison", font=TITLE_FONT, fill=(15, 15, 15))
    draw.text((MARGIN, 165), "Replacement for the invalid comparison PDF", font=SUB_FONT, fill=(85, 85, 85))

    y = 280
    y = text_block(
        draw,
        (MARGIN, y),
        "Scope rule: Greatstone/SCK is the primary asset-reference source; ReDMCSB is the primary code/usage reference; Firestaff local exports are raw material only.",
        BODY_FONT,
    )
    y += 18
    y = text_block(
        draw,
        (MARGIN, y),
        "This PDF includes only comparison pages whose original-side mappings are confirmed by GRAPHICS_DAT_EXPORT_MAPPING_AUDIT.md and whose Firestaff V2 counterpart is explicit in the repo.",
        BODY_FONT,
    )
    y += 36
    draw.text((MARGIN, y), "Included trusted mappings", font=H2_FONT, fill=(20, 20, 20))
    y += 56
    for entry in ENTRIES:
        y = text_block(draw, (MARGIN + 20, y), f"- {entry['index']} {entry['trusted_label']}", BODY_FONT)
    y += 24
    draw.text((MARGIN, y), "Intentionally excluded in this trusted PDF", font=H2_FONT, fill=(20, 20, 20))
    y += 56
    for item in EXCLUDED:
        y = text_block(draw, (MARGIN + 20, y), f"- {item}", BODY_FONT)
    y += 24
    text_block(draw, (MARGIN, y), f"Output: {OUT_PDF.relative_to(REPO)}", BODY_FONT)
    return page



def build_entry_page(entry: dict, page_number: int, total_pages: int) -> Image.Image:
    page = Image.new("RGB", (PAGE_W, PAGE_H), (250, 247, 241))
    draw = ImageDraw.Draw(page)
    draw.text((MARGIN, 55), f"Trusted-scope comparison — {page_number}/{total_pages}", font=SUB_FONT, fill=(90, 90, 90))
    draw.text((MARGIN, 95), f"{entry['index']} — {entry['trusted_label']}", font=TITLE_FONT, fill=(15, 15, 15))
    draw.text((MARGIN, 170), entry["status"], font=SUB_FONT, fill=(44, 110, 70))

    y = 225
    y = text_block(draw, (MARGIN, y), f"Evidence: {entry['evidence']}", BODY_FONT, fill=(50, 50, 50))
    y = text_block(draw, (MARGIN, y + 10), entry["notes"], BODY_FONT, fill=(50, 50, 50))

    draw.rounded_rectangle((MARGIN, HEADER_H, MARGIN + PANEL_W, HEADER_H + PANEL_H), 24, outline=(180, 170, 155), width=4, fill=(255, 255, 255))
    draw.rounded_rectangle((MARGIN + PANEL_W + GUTTER, HEADER_H, MARGIN + PANEL_W + GUTTER + PANEL_W, HEADER_H + PANEL_H), 24, outline=(180, 170, 155), width=4, fill=(255, 255, 255))
    draw.text((MARGIN + 28, HEADER_H + 20), "Original reference asset", font=SUB_FONT, fill=(40, 40, 40))
    draw.text((MARGIN + PANEL_W + GUTTER + 28, HEADER_H + 20), entry["v2_label"], font=SUB_FONT, fill=(40, 40, 40))

    original = fit_panel(load_image(entry["original"]), PANEL_W - 60, PANEL_H - 220, pixel_art=True)
    v2 = fit_panel(load_image(entry["v2"]), PANEL_W - 60, PANEL_H - 220, pixel_art=False)

    ox = MARGIN + (PANEL_W - original.width) // 2
    oy = HEADER_H + 100 + (PANEL_H - 260 - original.height) // 2
    vx = MARGIN + PANEL_W + GUTTER + (PANEL_W - v2.width) // 2
    vy = HEADER_H + 100 + (PANEL_H - 260 - v2.height) // 2
    page.paste(original, (ox, oy))
    page.paste(v2, (vx, vy))

    draw.text((MARGIN + 28, HEADER_H + PANEL_H - 150), f"Source: {entry['original'].relative_to(REPO)}", font=BODY_FONT, fill=(70, 70, 70))
    draw.text((MARGIN + PANEL_W + GUTTER + 28, HEADER_H + PANEL_H - 150), f"Source: {entry['v2'].relative_to(REPO)}", font=BODY_FONT, fill=(70, 70, 70))
    draw.line((MARGIN, PAGE_H - FOOTER_H - 20, PAGE_W - MARGIN, PAGE_H - FOOTER_H - 20), fill=(180, 170, 155), width=2)
    text_block(draw, (MARGIN, PAGE_H - FOOTER_H + 8), "Trusted-scope note: this page claims only that the original-side mapping is trustworthy and that the shown Firestaff V2 asset is the intended counterpart. It does not claim finished one-to-one visual parity.", BODY_FONT, fill=(55, 55, 55), line_gap=6)
    return page



def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    pages = [build_cover_page()]
    total_pages = len(ENTRIES) + 1
    for idx, entry in enumerate(ENTRIES, start=2):
        pages.append(build_entry_page(entry, idx, total_pages))
    first, rest = pages[0], pages[1:]
    first.save(OUT_PDF, save_all=True, append_images=rest, resolution=150.0)
    print(f"wrote {OUT_PDF}")


if __name__ == "__main__":
    main()
