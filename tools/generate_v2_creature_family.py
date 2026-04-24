#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from collections import deque
from pathlib import Path
from typing import Iterable

from PIL import Image, ImageChops, ImageEnhance, ImageFilter, ImageOps

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_TARGETS = {
    "front-near": {"master": (1200, 1200), "export": (600, 600), "scale": 0.92},
    "front-mid": {"master": (860, 860), "export": (430, 430), "scale": 0.84},
    "front-far": {"master": (560, 560), "export": (280, 280), "scale": 0.76},
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate a bounded Firestaff V2 creature family from one source card asset.")
    parser.add_argument("--source", required=True, help="Source PNG, relative to repo root or absolute path.")
    parser.add_argument("--slug", required=True, help="Creature slug, for example 'skeleton' or 'mummy'.")
    parser.add_argument("--display-name", required=True, help="Human display name, for example 'Skeleton'.")
    parser.add_argument("--family-dir", required=True, help="Output family directory, relative to repo root or absolute path.")
    parser.add_argument("--origin", default="Existing project creature card art", help="Source provenance text for the manifest.")
    parser.add_argument("--graphics-index", type=int, action="append", default=[], help="Optional GRAPHICS.DAT index. Repeat to add more than one.")
    parser.add_argument("--original-width", type=int, help="Original DM1 logical width for the reference role.")
    parser.add_argument("--original-height", type=int, help="Original DM1 logical height for the reference role.")
    parser.add_argument("--mask", help="Optional manual mask PNG, white=keep, black=drop.")
    return parser.parse_args()


def resolve(path_str: str | None) -> Path | None:
    if not path_str:
        return None
    path = Path(path_str)
    return path if path.is_absolute() else ROOT / path


def average_border_color(image: Image.Image) -> tuple[int, int, int]:
    rgb = image.convert("RGB")
    width, height = rgb.size
    samples: list[tuple[int, int, int]] = []
    stride_x = max(1, width // 24)
    stride_y = max(1, height // 24)
    for x in range(0, width, stride_x):
        samples.append(rgb.getpixel((x, 0)))
        samples.append(rgb.getpixel((x, height - 1)))
    for y in range(0, height, stride_y):
        samples.append(rgb.getpixel((0, y)))
        samples.append(rgb.getpixel((width - 1, y)))
    r = sum(px[0] for px in samples) // len(samples)
    g = sum(px[1] for px in samples) // len(samples)
    b = sum(px[2] for px in samples) // len(samples)
    return r, g, b


def build_mask(image: Image.Image, manual_mask: Path | None) -> Image.Image:
    if manual_mask and manual_mask.exists():
        mask = Image.open(manual_mask).convert("L")
        if mask.size != image.size:
            mask = mask.resize(image.size, Image.Resampling.LANCZOS)
        return mask

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

    visited = [[False for _ in range(w)] for _ in range(h)]
    q: deque[tuple[int, int]] = deque()
    for x in range(w):
        q.append((x, 0))
        q.append((x, h - 1))
    for y in range(h):
        q.append((0, y))
        q.append((w - 1, y))

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


def extract_subject(image: Image.Image, manual_mask: Path | None) -> Image.Image:
    image = image.convert("RGBA")
    mask = build_mask(image, manual_mask)
    bbox = mask.getbbox()
    if not bbox:
        raise SystemExit("could not isolate creature subject")

    cropped = image.crop(bbox)
    alpha = mask.crop(bbox)
    tinted = ImageEnhance.Color(cropped).enhance(0.82)
    tinted = ImageEnhance.Contrast(tinted).enhance(1.1)
    tinted = ImageEnhance.Sharpness(tinted).enhance(1.15)
    r, g, b, _ = tinted.split()
    return Image.merge("RGBA", (r, g, b, alpha))


def make_variant(subject: Image.Image, size: tuple[int, int], scale: float) -> Image.Image:
    canvas = Image.new("RGBA", size, (0, 0, 0, 0))
    sw, sh = subject.size
    max_w = int(size[0] * scale)
    max_h = int(size[1] * scale)
    ratio = min(max_w / sw, max_h / sh)
    resized = subject.resize((max(1, int(sw * ratio)), max(1, int(sh * ratio))), Image.Resampling.LANCZOS)

    shadow = Image.new("RGBA", resized.size, (10, 6, 4, 120))
    shadow = shadow.filter(ImageFilter.GaussianBlur(max(8, size[0] // 80)))
    x = (size[0] - resized.width) // 2
    baseline = max(12, size[1] // 16)
    y = size[1] - resized.height - baseline
    canvas.alpha_composite(shadow, (x + max(4, size[0] // 56), y + max(6, size[1] // 44)))
    canvas.alpha_composite(resized, (x, y))
    return canvas


def write_readme(base: Path, slug: str, display_name: str) -> None:
    text = f"# Firestaff V2 {display_name.lower()} family\n\nThis directory holds a bounded Wave 1 creature-family prototype for **{display_name}**.\n\n## What exists now\n\n- front-view distance set: near / mid / far\n- 4K masters in `masters/4k/`\n- exact 50% 1080p derivatives in `exports/1080p/`\n- schema-compatible manifest entry for this family\n\n## Workflow contract\n\n- Start from one cleaned source pose reference.\n- Generate or paint **4K-first** masters.\n- Derive 1080p by exact 50% downscale.\n- Keep the original DM depth ladder feel: **smoother is allowed, slower is not**.\n- When animation frames are added later, preserve the original cycle duration and gameplay travel speed.\n\n## Current limitations\n\n- one front-view family only\n- no side, back, hit, or attack poses yet\n- no runtime wiring in the active V1 parity path\n- source isolation is still heuristic unless a manual mask is supplied\n\n## Generator\n\nUse `tools/generate_v2_creature_family.py` for repeatable first-pass family exports.\n"
    (base / "README.md").write_text(text)


def write_spec(base: Path, slug: str, display_name: str, original_size: tuple[int, int] | None) -> Path:
    specs_dir = base.parent / "specs"
    specs_dir.mkdir(parents=True, exist_ok=True)
    size_line = f"- DM1 reference role: approximately `{original_size[0]}x{original_size[1]}`\n" if original_size else "- DM1 reference role: capture during next extraction pass\n"
    text = f"# {display_name} V2 family spec\n\n## Intent\nCreate a bounded V2 creature family for **{display_name}** that is cleaner and smoother than the original presentation without changing perceived gameplay timing.\n\n## Current deliverables\n- front-near\n- front-mid\n- front-far\n\n## Scale contract\n{size_line}- 4K masters are canonical\n- 1080p exports are exact 50% derivatives\n- near/mid/far should preserve the existing DM depth impression rather than invent a new distance curve\n\n## Animation timing rule\nIf later passes add in-between frames, keep the original action cycle duration and perceived movement speed. Extra frames should smooth interpolation, not lengthen attacks, walks, or idle loops.\n\n## Source policy\n- first-pass generation may start from an existing project creature card reference\n- upgrade to manual masks or repaint layers when heuristic extraction loses silhouette detail\n- keep this family isolated from V1 runtime assets until a dedicated V2 render path is ready\n"
    spec_path = specs_dir / f"{slug}-family.md"
    spec_path.write_text(text)
    return spec_path


def write_manifest(
    manifest_path: Path,
    family_dir: Path,
    slug: str,
    display_name: str,
    source_path: Path,
    origin: str,
    graphics_indices: Iterable[int],
    original_size: tuple[int, int] | None,
) -> None:
    source_w, source_h = Image.open(source_path).size
    manifest = {
        "manifestVersion": "1.0.0",
        "packId": f"firestaff-v2-wave1-{slug}-family",
        "description": f"Bounded first-pass V2 {slug} creature family for offline asset prep and export workflow validation.",
        "targetPolicy": {
            "masterResolution": "4k-first",
            "derivedResolutions": ["1080p"],
            "layoutSkeleton": "dm1-depth-ladder"
        },
        "assets": []
    }
    spec_rel = str((family_dir.parent / "specs" / f"{slug}-family.md").relative_to(ROOT))
    master_rel = str((family_dir / "masters/4k").relative_to(ROOT))
    export_rel = str((family_dir / "exports/1080p").relative_to(ROOT))
    original_ref = {
        "origin": origin,
        "originalSize": {
            "width": original_size[0] if original_size else source_w,
            "height": original_size[1] if original_size else source_h,
        },
    }
    indices = list(graphics_indices)
    if indices:
        original_ref["graphicsDatIndices"] = indices

    for slug_suffix, spec in DEFAULT_TARGETS.items():
        manifest["assets"].append(
            {
                "id": f"fs.v2.creature.{slug}.{slug_suffix}",
                "family": f"{slug}_family",
                "role": f"first-pass {slug_suffix.replace('-', ' ')} {display_name.lower()} sprite",
                "productionClass": "redraw-native",
                "sourceReference": original_ref,
                "sizes": {
                    "master4k": {"width": spec["master"][0], "height": spec["master"][1]},
                    "derived1080p": {"width": spec["export"][0], "height": spec["export"][1]},
                },
                "paths": {
                    "masterDir": master_rel,
                    "derivedDir": export_rel,
                    "spec": spec_rel,
                },
                "status": "in-progress",
                "notes": [
                    "Generated from a bounded first-pass creature workflow.",
                    "Animation smoothing in later passes must preserve original perceived timing.",
                ],
            }
        )
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n")


def main() -> None:
    args = parse_args()
    source_path = resolve(args.source)
    family_dir = resolve(args.family_dir)
    if source_path is None or family_dir is None:
        raise SystemExit("source and family-dir are required")
    family_dir.mkdir(parents=True, exist_ok=True)
    (family_dir / "masters/4k").mkdir(parents=True, exist_ok=True)
    (family_dir / "exports/1080p").mkdir(parents=True, exist_ok=True)

    mask_path = resolve(args.mask)
    source_image = Image.open(source_path)
    subject = extract_subject(source_image, mask_path)

    for distance_slug, spec in DEFAULT_TARGETS.items():
        master = make_variant(subject, spec["master"], spec["scale"])
        export = master.resize(spec["export"], Image.Resampling.LANCZOS)
        master.save(family_dir / "masters/4k" / f"fs-v2-{args.slug}-{distance_slug}.4k.png")
        export.save(family_dir / "exports/1080p" / f"fs-v2-{args.slug}-{distance_slug}.1080p.png")

    write_readme(family_dir, args.slug, args.display_name)
    original_size = None
    if args.original_width and args.original_height:
        original_size = (args.original_width, args.original_height)
    write_spec(family_dir, args.slug, args.display_name, original_size)
    manifest_path = ROOT / "assets-v2/manifests" / f"firestaff-v2-wave1-{args.slug}-family.manifest.json"
    write_manifest(
        manifest_path=manifest_path,
        family_dir=family_dir,
        slug=args.slug,
        display_name=args.display_name,
        source_path=source_path,
        origin=args.origin,
        graphics_indices=args.graphics_index,
        original_size=original_size,
    )
    print(f"wrote {args.slug} family to {family_dir.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
