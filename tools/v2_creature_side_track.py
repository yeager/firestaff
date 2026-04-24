#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
from typing import Iterable

from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter, ImageOps

ROOT = Path(__file__).resolve().parents[1]
WAVE1 = ROOT / "assets-v2/creatures/wave1"
MANIFESTS = ROOT / "assets-v2/manifests"

TARGETS = {
    "front-near": {"master": (1200, 1200), "export": (600, 600), "scale": 0.92},
    "front-mid": {"master": (860, 860), "export": (430, 430), "scale": 0.84},
    "front-far": {"master": (560, 560), "export": (280, 280), "scale": 0.76},
}

CONFIG = {
    "skeleton": {
        "display_name": "Skeleton",
        "source": ROOT / "assets/cards/creatures/skeleton.png",
        "family_dir": WAVE1 / "skeleton-family",
        "spec": WAVE1 / "specs/skeleton-family.md",
        "manifest": MANIFESTS / "firestaff-v2-wave1-skeleton-family.manifest.json",
        "guide_polygons": [
            [(250, 120), (700, 90), (1140, 170), (1390, 210), (1440, 470), (1350, 760), (1120, 980), (690, 1000), (360, 930), (210, 680)],
            [(950, 230), (1485, 140), (1510, 500), (1305, 590), (1000, 510)],
        ],
        "keep_ellipses": [
            (410, 120, 845, 520),
            (300, 300, 1110, 980),
            (960, 150, 1500, 610),
        ],
        "drop_rects": [
            (0, 0, 180, 1024),
            (0, 0, 1536, 70),
            (0, 970, 1536, 1024),
            (1480, 0, 1536, 1024),
        ],
        "rim": (214, 218, 232, 58),
        "shadow": (18, 14, 16, 120),
    },
    "mummy": {
        "display_name": "Mummy",
        "source": ROOT / "assets/cards/creatures/mummy.png",
        "family_dir": WAVE1 / "mummy-family",
        "spec": WAVE1 / "specs/mummy-family.md",
        "manifest": MANIFESTS / "firestaff-v2-wave1-mummy-family.manifest.json",
        "guide_polygons": [
            [(320, 80), (760, 70), (1150, 120), (1320, 220), (1390, 470), (1360, 810), (1010, 1000), (600, 1010), (340, 860), (240, 500)],
            [(320, 350), (1040, 250), (1260, 460), (910, 735), (330, 680)],
        ],
        "keep_ellipses": [
            (470, 90, 980, 500),
            (360, 320, 1110, 1010),
            (930, 210, 1360, 780),
            (220, 350, 520, 760),
        ],
        "drop_rects": [
            (0, 0, 180, 1024),
            (0, 0, 1536, 50),
            (1360, 0, 1536, 1024),
            (0, 980, 1536, 1024),
        ],
        "rim": (240, 217, 171, 68),
        "shadow": (24, 16, 10, 128),
    },
    "giant-scorpion": {
        "display_name": "Giant Scorpion",
        "source": ROOT / "assets/cards/creatures/giant-scorpion.png",
        "family_dir": WAVE1 / "giant-scorpion-family",
        "spec": WAVE1 / "specs/giant-scorpion-family.md",
        "manifest": MANIFESTS / "firestaff-v2-wave1-giant-scorpion-family.manifest.json",
        "guide_polygons": [
            [(120, 250), (520, 120), (1140, 120), (1470, 360), (1430, 790), (1040, 970), (420, 960), (120, 700)],
            [(760, 90), (1320, 40), (1520, 240), (1380, 580), (900, 520)],
        ],
        "keep_ellipses": [
            (220, 200, 880, 850),
            (680, 200, 1390, 840),
            (1010, 70, 1500, 700),
        ],
        "drop_rects": [
            (0, 0, 70, 1024),
            (0, 0, 1536, 70),
            (1460, 0, 1536, 1024),
            (0, 970, 1536, 1024),
        ],
        "rim": (109, 222, 224, 62),
        "shadow": (8, 20, 22, 126),
    },
}


def average_border_color(image: Image.Image) -> tuple[int, int, int]:
    rgb = image.convert("RGB")
    width, height = rgb.size
    samples = []
    stride_x = max(1, width // 24)
    stride_y = max(1, height // 24)
    for x in range(0, width, stride_x):
        samples.append(rgb.getpixel((x, 0)))
        samples.append(rgb.getpixel((x, height - 1)))
    for y in range(0, height, stride_y):
        samples.append(rgb.getpixel((0, y)))
        samples.append(rgb.getpixel((width - 1, y)))
    return tuple(sum(px[i] for px in samples) // len(samples) for i in range(3))


def heuristic_mask(image: Image.Image) -> Image.Image:
    rgb = image.convert("RGB")
    bg = average_border_color(rgb)
    thumb = rgb.copy()
    thumb.thumbnail((256, 256), Image.Resampling.LANCZOS)
    w, h = thumb.size
    bg_luma = int(sum(bg) / 3)
    candidate = [[False for _ in range(w)] for _ in range(h)]
    for y in range(h):
        for x in range(w):
            r, g, b = thumb.getpixel((x, y))
            dist = abs(r - bg[0]) + abs(g - bg[1]) + abs(b - bg[2])
            luma = (r + g + b) // 3
            candidate[y][x] = dist < 58 and luma <= bg_luma + 18

    from collections import deque
    visited = [[False for _ in range(w)] for _ in range(h)]
    q = deque()
    for x in range(w):
        q.append((x, 0)); q.append((x, h - 1))
    for y in range(h):
        q.append((0, y)); q.append((w - 1, y))
    while q:
        x, y = q.popleft()
        if x < 0 or x >= w or y < 0 or y >= h or visited[y][x] or not candidate[y][x]:
            continue
        visited[y][x] = True
        q.extend(((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)))
    bg_mask = Image.new("L", (w, h), 0)
    for y in range(h):
        for x in range(w):
            if visited[y][x]:
                bg_mask.putpixel((x, y), 255)
    bg_mask = bg_mask.resize(rgb.size, Image.Resampling.LANCZOS)
    diff = ImageOps.grayscale(ImageChops.difference(rgb, Image.new("RGB", rgb.size, bg)))
    diff = ImageOps.autocontrast(diff)
    fg_from_diff = diff.point(lambda v: 255 if v > 24 else 0)
    fg_mask = ImageChops.screen(ImageOps.invert(bg_mask), fg_from_diff)
    fg_mask = fg_mask.filter(ImageFilter.MedianFilter(5))
    fg_mask = fg_mask.filter(ImageFilter.GaussianBlur(1.8))
    return fg_mask.point(lambda v: 0 if v < 20 else min(255, int(v * 1.25)))


def manual_mask(image: Image.Image, cfg: dict) -> Image.Image:
    base = Image.new("L", image.size, 0)
    draw = ImageDraw.Draw(base)
    for poly in cfg["guide_polygons"]:
        draw.polygon(poly, fill=255)
    for ellipse in cfg["keep_ellipses"]:
        draw.ellipse(ellipse, fill=255)
    for rect in cfg.get("drop_rects", []):
        draw.rectangle(rect, fill=0)
    base = base.filter(ImageFilter.GaussianBlur(18))
    base = ImageOps.autocontrast(base)
    return base


def refined_mask(image: Image.Image, cfg: dict) -> Image.Image:
    hmask = heuristic_mask(image)
    mmask = manual_mask(image, cfg)
    keep = ImageChops.lighter(hmask.filter(ImageFilter.GaussianBlur(3)), mmask)
    keep = keep.filter(ImageFilter.MaxFilter(7)).filter(ImageFilter.GaussianBlur(2.2))
    return keep.point(lambda v: 0 if v < 18 else min(255, int(v * 1.08)))


def extract_clean_subject(image: Image.Image, mask: Image.Image, cfg: dict) -> Image.Image:
    rgba = image.convert("RGBA")
    bbox = mask.getbbox()
    if not bbox:
        raise RuntimeError("empty mask")
    cropped = rgba.crop(bbox)
    alpha = mask.crop(bbox)
    toned = ImageEnhance.Color(cropped).enhance(0.9)
    toned = ImageEnhance.Contrast(toned).enhance(1.12)
    toned = ImageEnhance.Sharpness(toned).enhance(1.2)
    toned = ImageEnhance.Brightness(toned).enhance(1.04)
    subject = Image.merge("RGBA", (*toned.convert("RGB").split(), alpha))

    rim_mask = alpha.filter(ImageFilter.MaxFilter(25)).filter(ImageFilter.GaussianBlur(8))
    rim_mask = ImageChops.subtract(rim_mask, alpha.filter(ImageFilter.GaussianBlur(3)))
    rim_layer = Image.new("RGBA", subject.size, cfg["rim"])
    rim_layer.putalpha(rim_mask.point(lambda v: min(255, int(v * 0.7))))
    subject = Image.alpha_composite(rim_layer, subject)
    return subject


def render_variant(subject: Image.Image, size: tuple[int, int], scale: float, shadow_rgba: tuple[int, int, int, int]) -> Image.Image:
    canvas = Image.new("RGBA", size, (0, 0, 0, 0))
    sw, sh = subject.size
    max_w = int(size[0] * scale)
    max_h = int(size[1] * scale)
    ratio = min(max_w / sw, max_h / sh)
    resized = subject.resize((max(1, int(sw * ratio)), max(1, int(sh * ratio))), Image.Resampling.LANCZOS)

    shadow = Image.new("RGBA", resized.size, shadow_rgba)
    shadow.putalpha(resized.getchannel("A").filter(ImageFilter.GaussianBlur(max(10, size[0] // 64))))
    x = (size[0] - resized.width) // 2
    baseline = max(12, size[1] // 15)
    y = size[1] - resized.height - baseline
    canvas.alpha_composite(shadow, (x + max(4, size[0] // 52), y + max(6, size[1] // 38)))
    canvas.alpha_composite(resized, (x, y))
    return canvas


def ensure_dirs(path: Path) -> None:
    (path / "masters/4k").mkdir(parents=True, exist_ok=True)
    (path / "exports/1080p").mkdir(parents=True, exist_ok=True)
    (path / "workflow").mkdir(parents=True, exist_ok=True)


def write_family(cfg: dict) -> None:
    family_dir = cfg["family_dir"]
    ensure_dirs(family_dir)
    source = Image.open(cfg["source"])
    mask = refined_mask(source, cfg)
    subject = extract_clean_subject(source, mask, cfg)
    slug = family_dir.name.removesuffix("-family")

    mask.save(family_dir / "workflow" / f"fs-v2-{slug}-manual-mask.png")
    subject.save(family_dir / "workflow" / f"fs-v2-{slug}-clean-subject.png")
    source.save(family_dir / "workflow" / f"fs-v2-{slug}-source-plate.png")


def make_animation_sheet() -> None:
    cfg = CONFIG["skeleton"]
    subject = Image.open(cfg["family_dir"] / "workflow" / "fs-v2-skeleton-clean-subject.png").convert("RGBA")
    frame_size = (420, 420)
    timeline = [
        {"y": 0, "scale": 0.84, "rot": 0, "x": 0},
        {"y": -8, "scale": 0.848, "rot": -1.2, "x": -2},
        {"y": -14, "scale": 0.855, "rot": -2.2, "x": -5},
        {"y": -8, "scale": 0.848, "rot": -1.0, "x": -2},
        {"y": 0, "scale": 0.84, "rot": 0.3, "x": 0},
        {"y": 8, "scale": 0.832, "rot": 1.2, "x": 2},
        {"y": 14, "scale": 0.826, "rot": 2.4, "x": 5},
        {"y": 8, "scale": 0.832, "rot": 1.1, "x": 2},
    ]
    frames = []
    for pose in timeline:
        canvas = Image.new("RGBA", frame_size, (0, 0, 0, 0))
        sw, sh = subject.size
        ratio = min((frame_size[0] * pose["scale"]) / sw, (frame_size[1] * pose["scale"]) / sh)
        sized = subject.resize((max(1, int(sw * ratio)), max(1, int(sh * ratio))), Image.Resampling.LANCZOS)
        sized = sized.rotate(pose["rot"], resample=Image.Resampling.BICUBIC, expand=True)
        shadow = Image.new("RGBA", sized.size, (18, 14, 16, 110))
        shadow.putalpha(sized.getchannel("A").filter(ImageFilter.GaussianBlur(12)))
        x = (frame_size[0] - sized.width) // 2 + int(pose["x"])
        y = frame_size[1] - sized.height - 30 + int(pose["y"])
        canvas.alpha_composite(shadow, (x + 6, y + 10))
        canvas.alpha_composite(sized, (x, y))
        frames.append(canvas)
    sheet = Image.new("RGBA", (frame_size[0] * len(frames), frame_size[1]), (0, 0, 0, 0))
    for i, frame in enumerate(frames):
        sheet.alpha_composite(frame, (i * frame_size[0], 0))
    out_dir = cfg["family_dir"] / "animations"
    out_dir.mkdir(parents=True, exist_ok=True)
    sheet.save(out_dir / "fs-v2-skeleton-idle-cycle.4k-sheet.png")
    sheet.resize((sheet.width // 2, sheet.height // 2), Image.Resampling.LANCZOS).save(out_dir / "fs-v2-skeleton-idle-cycle.1080p-sheet.png")
    timing = {
        "family": "skeleton",
        "clip": "idle_cycle",
        "frameCount": len(frames),
        "frameSize": {"width": frame_size[0], "height": frame_size[1]},
        "cycleDurationMs": 400,
        "frameDurationMs": 50,
        "timingLock": "8 in-between frames keep a 400 ms total cycle so perceived speed does not drift slower than the base 4-beat motion.",
    }
    (out_dir / "fs-v2-skeleton-idle-cycle.timing.json").write_text(json.dumps(timing, indent=2) + "\n")


def make_preview() -> None:
    out_dir = WAVE1 / "previews"
    out_dir.mkdir(parents=True, exist_ok=True)
    bg = Image.new("RGBA", (1280, 720), (11, 9, 14, 255))
    grad = Image.new("L", bg.size, 0)
    gdraw = ImageDraw.Draw(grad)
    for y in range(bg.height):
        v = int(18 + 55 * (y / bg.height))
        gdraw.line((0, y, bg.width, y), fill=v)
    color = Image.new("RGBA", bg.size, (32, 26, 22, 255))
    bg = Image.composite(color, bg, grad)
    draw = ImageDraw.Draw(bg)
    draw.rectangle((0, 540, 1280, 720), fill=(36, 28, 20, 255))
    draw.rectangle((0, 500, 1280, 508), fill=(66, 52, 42, 255))

    placements = [
        ("skeleton", (170, 165), "near"),
        ("mummy", (500, 185), "mid"),
        ("giant-scorpion", (760, 170), "near"),
    ]
    for slug, (x, y), depth in placements:
        img = Image.open(WAVE1 / f"{slug}-family" / "exports/1080p" / f"fs-v2-{slug}-front-{depth}.1080p.png").convert("RGBA")
        bg.alpha_composite(img, (x, y))
    draw.text((36, 30), "Firestaff V2 offline creature preview", fill=(227, 220, 205, 255))
    draw.text((36, 58), "Skeleton manual cleanup • Mummy manual cleanup • Giant Scorpion third family", fill=(182, 174, 160, 255))
    draw.text((36, 92), "Timing note: smoother frames may be added, but perceived motion speed stays locked.", fill=(160, 154, 146, 255))
    bg.save(out_dir / "fs-v2-wave1-offline-preview.png")


def patch_readmes_and_specs() -> None:
    for slug in ["skeleton", "mummy", "giant-scorpion"]:
        family_dir = WAVE1 / f"{slug}-family"
        readme_path = family_dir / "README.md"
        text = readme_path.read_text()
        if "workflow/" not in text:
            text = text.replace(
                "- exact 50% 1080p derivatives in `exports/1080p/`\n",
                "- exact 50% 1080p derivatives in `exports/1080p/`\n- manual cleanup workflow assets in `workflow/`\n",
            )
            text = text.replace(
                "- source isolation is still heuristic unless a manual mask is supplied\n",
                "- source isolation now includes a checked-in manual mask and clean cutout workflow for this family\n",
            )
            readme_path.write_text(text)
        spec_path = WAVE1 / "specs" / f"{slug}-family.md"
        spec = spec_path.read_text()
        if "Workflow coverage" not in spec:
            spec += "\n## Workflow coverage\n- checked-in manual mask for bounded offline cleanup\n- clean subject cutout for paintover-safe follow-up passes\n"
            if slug == "skeleton":
                spec += "- idle animation sheet locked to a 400 ms total cycle in `skeleton-family/animations/`\n"
            spec_path.write_text(spec)


def patch_manifests() -> None:
    for slug in ["skeleton", "mummy", "giant-scorpion"]:
        manifest_path = MANIFESTS / f"firestaff-v2-wave1-{slug}-family.manifest.json"
        manifest = json.loads(manifest_path.read_text())
        for asset in manifest["assets"]:
            notes = asset.setdefault("notes", [])
            extra = "Manual mask + clean-subject workflow assets are checked in for offline V2 paintover-safe cleanup."
            if extra not in notes:
                notes.append(extra)
        manifest_path.write_text(json.dumps(manifest, indent=2) + "\n")


def main() -> None:
    for slug in ["skeleton", "mummy", "giant-scorpion"]:
        write_family(CONFIG[slug])
    make_animation_sheet()
    make_preview()
    patch_readmes_and_specs()
    patch_manifests()
    print("refreshed skeleton, mummy, and giant scorpion bounded V2 creature deliverables")


if __name__ == "__main__":
    main()
