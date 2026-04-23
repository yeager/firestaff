#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
from PIL import Image, ImageColor, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
MASTER_DIR = ROOT / "assets-v2/ui/wave1/spell-area/masters/4k"
EXPORT_DIR = ROOT / "assets-v2/ui/wave1/spell-area/exports/1080p"
W4K, H4K = 870, 250
W1080, H1080 = 435, 125

ASSETS = {
    "base": "fs-v2-spell-area-base",
    "rune-bed": "fs-v2-spell-area-rune-bed",
    "highlight-overlay": "fs-v2-spell-area-highlight-overlay",
    "active-overlay": "fs-v2-spell-area-active-overlay",
}


def rgba(value: str, alpha: int = 255):
    r, g, b = ImageColor.getrgb(value)
    return (r, g, b, alpha)


def rounded_rect(draw: ImageDraw.ImageDraw, box, radius: int, fill=None, outline=None, width: int = 1):
    draw.rounded_rectangle(box, radius=radius, fill=fill, outline=outline, width=width)


def add_vertical_gradient(im: Image.Image, box, top: tuple[int, int, int, int], bottom: tuple[int, int, int, int], radius: int):
    x0, y0, x1, y1 = box
    w = x1 - x0
    h = y1 - y0
    grad = Image.new("RGBA", (w, h), 0)
    px = grad.load()
    for y in range(h):
        t = 0 if h <= 1 else y / (h - 1)
        color = tuple(int(top[i] * (1 - t) + bottom[i] * t) for i in range(4))
        for x in range(w):
            px[x, y] = color
    mask = Image.new("L", (w, h), 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, w - 1, h - 1), radius=radius, fill=255)
    im.alpha_composite(grad, (x0, y0), (0, 0, w, h))
    im.putalpha(Image.composite(im.getchannel("A"), Image.new("L", im.size, 0), Image.new("L", im.size, 0)))
    alpha = im.getchannel("A")
    alpha.paste(mask, (x0, y0))
    im.putalpha(ImageChops.lighter(alpha, Image.new("L", im.size, 0)))


def soft_glow(im: Image.Image, box, color, alpha_scale: float = 1.0):
    x0, y0, x1, y1 = box
    glow = Image.new("RGBA", im.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(glow)
    cx = (x0 + x1) / 2
    cy = (y0 + y1) / 2
    rx = max(1, int((x1 - x0) / 2))
    ry = max(1, int((y1 - y0) / 2))
    for step in range(20, 0, -1):
        t = step / 20
        a = int(color[3] * (t ** 2) * alpha_scale)
        bx = (int(cx - rx * t), int(cy - ry * t), int(cx + rx * t), int(cy + ry * t))
        draw.ellipse(bx, fill=(color[0], color[1], color[2], a))
    im.alpha_composite(glow)


def create_base() -> Image.Image:
    im = Image.new("RGBA", (W4K, H4K), (0, 0, 0, 0))
    draw = ImageDraw.Draw(im)
    rounded_rect(draw, (10, 8, 860, 242), 30, fill=rgba("#17110f"), outline=rgba("#080605"), width=8)
    rounded_rect(draw, (22, 18, 848, 232), 24, fill=rgba("#5b4335"), outline=rgba("#b99158"), width=6)
    rounded_rect(draw, (34, 30, 836, 220), 20, fill=rgba("#2a1f1a"), outline=rgba("#2b2019"), width=4)
    rounded_rect(draw, (60, 58, 810, 192), 16, fill=rgba("#120d0d"), outline=rgba("#71533a"), width=4)
    rounded_rect(draw, (76, 72, 794, 178), 14, fill=rgba("#1b1414"), outline=rgba("#3b2b24"), width=3)
    for x in (128, 280, 432, 584, 736):
        draw.line((x, 80, x, 170), fill=rgba("#6b4d37", 160), width=3)
    for y in (93, 155):
        draw.line((92, y, 778, y), fill=rgba("#6b4d37", 120), width=2)
    draw.arc((44, 18, 240, 126), 190, 350, fill=rgba("#c79d60"), width=5)
    draw.arc((630, 18, 826, 126), 190, 350, fill=rgba("#c79d60"), width=5)
    soft_glow(im, (120, 48, 750, 172), rgba("#f1be6f", 36))
    return im


def create_rune_bed() -> Image.Image:
    im = Image.new("RGBA", (W4K, H4K), (0, 0, 0, 0))
    draw = ImageDraw.Draw(im)
    rounded_rect(draw, (62, 60, 808, 190), 15, fill=rgba("#130e0e", 220), outline=rgba("#8b6543", 220), width=3)
    for idx in range(6):
        x0 = 88 + idx * 116
        x1 = x0 + 94
        rounded_rect(draw, (x0, 82, x1, 166), 10, fill=rgba("#1f1919", 180), outline=rgba("#9a7448", 190), width=2)
        draw.line((x0 + 47, 92, x0 + 47, 156), fill=rgba("#70513a", 100), width=1)
    soft_glow(im, (90, 82, 780, 166), rgba("#f1be6f", 24), 0.8)
    return im


def create_highlight_overlay() -> Image.Image:
    im = Image.new("RGBA", (W4K, H4K), (0, 0, 0, 0))
    draw = ImageDraw.Draw(im)
    rounded_rect(draw, (26, 22, 844, 228), 22, outline=rgba("#d8b37a", 140), width=3)
    rounded_rect(draw, (68, 66, 802, 184), 15, outline=rgba("#f2cd8f", 110), width=2)
    soft_glow(im, (120, 56, 750, 168), rgba("#f4c77f", 44))
    return im


def create_active_overlay() -> Image.Image:
    im = Image.new("RGBA", (W4K, H4K), (0, 0, 0, 0))
    draw = ImageDraw.Draw(im)
    rounded_rect(draw, (18, 14, 852, 236), 26, outline=rgba("#7cd6ff", 150), width=4)
    rounded_rect(draw, (74, 70, 796, 180), 15, outline=rgba("#87dfff", 120), width=3)
    for idx in range(4):
        x = 160 + idx * 170
        draw.polygon([(x, 44), (x + 18, 26), (x + 36, 44), (x + 18, 62)], outline=rgba("#8ee4ff", 120), fill=(0, 0, 0, 0), width=2)
    soft_glow(im, (140, 58, 732, 170), rgba("#67d2ff", 52))
    return im


def svg_for(kind: str) -> str:
    if kind == "base":
        extra = """
  <rect x=\"10\" y=\"8\" width=\"850\" height=\"234\" rx=\"30\" fill=\"#17110f\" stroke=\"#080605\" stroke-width=\"8\"/>
  <rect x=\"22\" y=\"18\" width=\"826\" height=\"214\" rx=\"24\" fill=\"#5b4335\" stroke=\"#b99158\" stroke-width=\"6\"/>
  <rect x=\"34\" y=\"30\" width=\"802\" height=\"190\" rx=\"20\" fill=\"#2a1f1a\" stroke=\"#2b2019\" stroke-width=\"4\"/>
  <rect x=\"60\" y=\"58\" width=\"750\" height=\"134\" rx=\"16\" fill=\"#120d0d\" stroke=\"#71533a\" stroke-width=\"4\"/>
  <rect x=\"76\" y=\"72\" width=\"718\" height=\"106\" rx=\"14\" fill=\"#1b1414\" stroke=\"#3b2b24\" stroke-width=\"3\"/>
  <g stroke=\"#6b4d37\" stroke-opacity=\"0.6\">
    <line x1=\"128\" y1=\"80\" x2=\"128\" y2=\"170\" stroke-width=\"3\"/>
    <line x1=\"280\" y1=\"80\" x2=\"280\" y2=\"170\" stroke-width=\"3\"/>
    <line x1=\"432\" y1=\"80\" x2=\"432\" y2=\"170\" stroke-width=\"3\"/>
    <line x1=\"584\" y1=\"80\" x2=\"584\" y2=\"170\" stroke-width=\"3\"/>
    <line x1=\"736\" y1=\"80\" x2=\"736\" y2=\"170\" stroke-width=\"3\"/>
    <line x1=\"92\" y1=\"93\" x2=\"778\" y2=\"93\" stroke-width=\"2\"/>
    <line x1=\"92\" y1=\"155\" x2=\"778\" y2=\"155\" stroke-width=\"2\"/>
  </g>
  <path d=\"M44 72 C88 30, 174 24, 240 72\" fill=\"none\" stroke=\"#c79d60\" stroke-width=\"5\"/>
  <path d=\"M630 72 C674 30, 760 24, 826 72\" fill=\"none\" stroke=\"#c79d60\" stroke-width=\"5\"/>
  <ellipse cx=\"435\" cy=\"110\" rx=\"315\" ry=\"62\" fill=\"#f1be6f\" opacity=\"0.12\"/>
"""
    elif kind == "rune-bed":
        rects = []
        for idx in range(6):
            x0 = 88 + idx * 116
            rects.append(f'<rect x="{x0}" y="82" width="94" height="84" rx="10" fill="#1f1919" fill-opacity="0.72" stroke="#9a7448" stroke-opacity="0.74" stroke-width="2"/>')
        extra = "\n".join([
            '  <rect x="62" y="60" width="746" height="130" rx="15" fill="#130e0e" fill-opacity="0.86" stroke="#8b6543" stroke-opacity="0.86" stroke-width="3"/>',
            *rects,
            '  <ellipse cx="435" cy="124" rx="345" ry="42" fill="#f1be6f" opacity="0.09"/>'
        ]) + "\n"
    elif kind == "highlight-overlay":
        extra = """
  <rect x=\"26\" y=\"22\" width=\"818\" height=\"206\" rx=\"22\" fill=\"none\" stroke=\"#d8b37a\" stroke-opacity=\"0.55\" stroke-width=\"3\"/>
  <rect x=\"68\" y=\"66\" width=\"734\" height=\"118\" rx=\"15\" fill=\"none\" stroke=\"#f2cd8f\" stroke-opacity=\"0.43\" stroke-width=\"2\"/>
  <ellipse cx=\"435\" cy=\"112\" rx=\"315\" ry=\"56\" fill=\"#f4c77f\" opacity=\"0.18\"/>
"""
    else:
        extra = """
  <rect x=\"18\" y=\"14\" width=\"834\" height=\"222\" rx=\"26\" fill=\"none\" stroke=\"#7cd6ff\" stroke-opacity=\"0.6\" stroke-width=\"4\"/>
  <rect x=\"74\" y=\"70\" width=\"722\" height=\"110\" rx=\"15\" fill=\"none\" stroke=\"#87dfff\" stroke-opacity=\"0.47\" stroke-width=\"3\"/>
  <g fill=\"none\" stroke=\"#8ee4ff\" stroke-opacity=\"0.46\" stroke-width=\"2\">
    <polygon points=\"160,44 178,26 196,44 178,62\"/>
    <polygon points=\"330,44 348,26 366,44 348,62\"/>
    <polygon points=\"500,44 518,26 536,44 518,62\"/>
    <polygon points=\"670,44 688,26 706,44 688,62\"/>
  </g>
  <ellipse cx=\"436\" cy=\"114\" rx=\"296\" ry=\"56\" fill=\"#67d2ff\" opacity=\"0.17\"/>
"""
    return f'''<svg xmlns="http://www.w3.org/2000/svg" width="{W4K}" height="{H4K}" viewBox="0 0 {W4K} {H4K}">
{extra}</svg>
'''


def save_asset(kind: str, image: Image.Image):
    base = ASSETS[kind]
    MASTER_DIR.mkdir(parents=True, exist_ok=True)
    EXPORT_DIR.mkdir(parents=True, exist_ok=True)
    (MASTER_DIR / f"{base}.4k.svg").write_text(svg_for(kind))
    image.save(MASTER_DIR / f"{base}.4k.png")
    image.resize((W1080, H1080), Image.Resampling.LANCZOS).save(EXPORT_DIR / f"{base}.1080p.png")


def main() -> None:
    save_asset("base", create_base())
    save_asset("rune-bed", create_rune_bed())
    save_asset("highlight-overlay", create_highlight_overlay())
    save_asset("active-overlay", create_active_overlay())
    print("generated spell-area assets")


if __name__ == "__main__":
    from PIL import ImageChops
    main()
