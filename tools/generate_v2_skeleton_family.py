#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
from PIL import Image, ImageFilter, ImageOps, ImageEnhance

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "assets/cards/creatures/skeleton.png"
BASE = ROOT / "assets-v2/creatures/wave1/skeleton-family"

TARGETS = {
    "front-near": {"master": (1200, 1200), "export": (600, 600), "scale": 0.92},
    "front-mid": {"master": (860, 860), "export": (430, 430), "scale": 0.84},
    "front-far": {"master": (560, 560), "export": (280, 280), "scale": 0.76},
}


def extract_subject(image: Image.Image) -> Image.Image:
    image = image.convert("RGBA")
    gray = ImageOps.grayscale(image)
    mask = gray.point(lambda v: 255 if v < 120 else 0)
    mask = mask.filter(ImageFilter.GaussianBlur(1.5))
    bbox = mask.getbbox()
    if not bbox:
        raise SystemExit("could not isolate skeleton subject")
    image = image.crop(bbox)
    mask = mask.crop(bbox)
    alpha = mask.point(lambda v: 0 if v < 24 else min(255, int(v * 1.35)))
    tinted = ImageEnhance.Color(image).enhance(0.55)
    tinted = ImageEnhance.Contrast(tinted).enhance(1.18)
    tinted = ImageEnhance.Sharpness(tinted).enhance(1.2)
    r, g, b, _ = tinted.split()
    merged = Image.merge("RGBA", (r.point(lambda v: min(255, int(v * 1.02))),
                                   g.point(lambda v: min(255, int(v * 0.96))),
                                   b.point(lambda v: min(255, int(v * 0.82))),
                                   alpha))
    return merged


def make_variant(subject: Image.Image, size: tuple[int, int], scale: float) -> Image.Image:
    canvas = Image.new("RGBA", size, (0, 0, 0, 0))
    sw, sh = subject.size
    max_w = int(size[0] * scale)
    max_h = int(size[1] * scale)
    ratio = min(max_w / sw, max_h / sh)
    resized = subject.resize((max(1, int(sw * ratio)), max(1, int(sh * ratio))), Image.Resampling.LANCZOS)
    shadow = Image.new("RGBA", resized.size, (8, 4, 2, 110))
    shadow = shadow.filter(ImageFilter.GaussianBlur(10))
    x = (size[0] - resized.width) // 2
    y = size[1] - resized.height - max(12, size[1] // 16)
    canvas.alpha_composite(shadow, (x + max(4, size[0] // 48), y + max(6, size[1] // 40)))
    canvas.alpha_composite(resized, (x, y))
    return canvas


def main() -> None:
    subject = extract_subject(Image.open(SRC))
    master_dir = BASE / "masters/4k"
    export_dir = BASE / "exports/1080p"
    master_dir.mkdir(parents=True, exist_ok=True)
    export_dir.mkdir(parents=True, exist_ok=True)

    for slug, spec in TARGETS.items():
        master = make_variant(subject, spec["master"], spec["scale"])
        export = master.resize(spec["export"], Image.Resampling.LANCZOS)
        master.save(master_dir / f"fs-v2-skeleton-{slug}.4k.png")
        export.save(export_dir / f"fs-v2-skeleton-{slug}.1080p.png")

    print(f"wrote skeleton family to {BASE.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
