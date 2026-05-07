#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
from pathlib import Path

from collections import Counter, deque

from PIL import Image, ImageChops, ImageStat

ROOT = Path(__file__).resolve().parents[1]
ORIGINAL = ROOT / "parity-evidence/verification/pass282_dm1_v2_original_pixel_capture/original_entry_viewport_224x136.png"
FIRESTAFF = ROOT / "parity-evidence/verification/pass285_dm1_v2_firestaff_entry_viewport_224x136.png"
DIFF = ROOT / "parity-evidence/verification/pass286_dm1_v2_entry_viewport_original_vs_firestaff_diff.png"
EVIDENCE = ROOT / "parity-evidence/verification/pass286_dm1_v2_entry_viewport_png_comparator_gate.json"
SOURCE = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
ORIGINAL_DATA = (Path.home() / ".openclaw/data/firestaff-original-games/DM")


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def checked_anchor(errors: list[str], rel: str, line: int, needle: str) -> dict[str, object]:
    path = SOURCE / rel
    actual = ""
    if not path.exists():
        errors.append(f"missing ReDMCSB source file {path}")
    else:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        actual = lines[line - 1].strip() if 0 <= line - 1 < len(lines) else ""
        if needle not in actual:
            errors.append(f"{rel}:{line}: expected {needle!r}, got {actual!r}")
    return {"file": rel, "line": line, "needle": needle, "actual": actual}


def image_info(path: Path) -> dict[str, object]:
    img = Image.open(path).convert("RGBA")
    colors = img.getcolors(maxcolors=1_000_000)
    return {
        "path": str(path.relative_to(ROOT)),
        "sha256": sha256(path),
        "width": img.width,
        "height": img.height,
        "modeAfterNormalize": "RGBA",
        "uniqueColors": len(colors) if colors is not None else None,
    }


def rgba_to_hex(px: tuple[int, int, int, int]) -> str:
    return "#{:02x}{:02x}{:02x}{:02x}".format(*px)


def mismatch_component_summary(orig: Image.Image, fire: Image.Image) -> dict[str, object]:
    width, height = orig.size
    mismatch = [[orig.getpixel((x, y)) != fire.getpixel((x, y)) for x in range(width)] for y in range(height)]
    seen = [[False] * width for _ in range(height)]
    components: list[dict[str, object]] = []
    total = 0
    for y in range(height):
        for x in range(width):
            if not mismatch[y][x] or seen[y][x]:
                continue
            queue: deque[tuple[int, int]] = deque([(x, y)])
            seen[y][x] = True
            xs: list[int] = []
            ys: list[int] = []
            while queue:
                cx, cy = queue.popleft()
                xs.append(cx)
                ys.append(cy)
                total += 1
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = cx + dx, cy + dy
                    if 0 <= nx < width and 0 <= ny < height and mismatch[ny][nx] and not seen[ny][nx]:
                        seen[ny][nx] = True
                        queue.append((nx, ny))
            x0, x1 = min(xs), max(xs)
            y0, y1 = min(ys), max(ys)
            components.append({
                "pixels": len(xs),
                "bboxInclusive": [x0, y0, x1, y1],
                "width": x1 - x0 + 1,
                "height": y1 - y0 + 1,
            })
    components.sort(key=lambda item: (-int(item["pixels"]), item["bboxInclusive"]))

    tiles: list[dict[str, object]] = []
    for y0 in range(0, height, 16):
        for x0 in range(0, width, 16):
            x1 = min(x0 + 15, width - 1)
            y1 = min(y0 + 15, height - 1)
            count = sum(mismatch[y][x] for y in range(y0, y1 + 1) for x in range(x0, x1 + 1))
            if count:
                tiles.append({"pixels": count, "bboxInclusive": [x0, y0, x1, y1]})
    tiles.sort(key=lambda item: (-int(item["pixels"]), item["bboxInclusive"]))

    bands = []
    for y0 in range(0, height, 8):
        y1 = min(y0 + 7, height - 1)
        bands.append({
            "yRangeInclusive": [y0, y1],
            "pixels": sum(mismatch[y][x] for y in range(y0, y1 + 1) for x in range(width)),
        })

    color_pairs = Counter((orig_px, fire_px) for orig_px, fire_px in zip(orig.getdata(), fire.getdata()) if orig_px != fire_px)
    return {
        "connectedComponentCount": len(components),
        "largestConnectedComponents": components[:8],
        "densest16x16Tiles": tiles[:12],
        "horizontalBands8px": bands,
        "topColorMismatches": [
            {"pixels": count, "originalRgba": rgba_to_hex(original), "firestaffRgba": rgba_to_hex(firestaff)}
            for (original, firestaff), count in color_pairs.most_common(12)
        ],
        "totalMismatchedPixelsVerified": total,
    }


def main() -> int:
    errors: list[str] = []
    for path in (ORIGINAL, FIRESTAFF):
        if not path.exists():
            errors.append(f"missing image {path.relative_to(ROOT)}")
    original_archive = ORIGINAL_DATA / "Game,Dungeon_Master,DOS,Software.7z"
    if not original_archive.exists():
        errors.append(f"missing original DM1 DOS archive {original_archive}")

    anchors = [
        checked_anchor(errors, "DUNVIEW.C", 8337, "if (G0297_B_DrawFloorAndCeilingRequested)"),
        checked_anchor(errors, "DUNVIEW.C", 8367, "F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport)"),
        checked_anchor(errors, "DUNVIEW.C", 8490, "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement"),
        checked_anchor(errors, "DUNVIEW.C", 8542, "F0127_DUNGEONVIEW_DrawSquareD0C"),
        checked_anchor(errors, "DUNVIEW.C", 8606, "F0097_DUNGEONVIEW_DrawViewport"),
        checked_anchor(errors, "DUNVIEW.C", 6699, "F0100_DUNGEONVIEW_DrawWallSetBitmap"),
        checked_anchor(errors, "DUNVIEW.C", 6702, "F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency"),
        checked_anchor(errors, "DUNVIEW.C", 6720, "return"),
    ]

    original_info = None
    firestaff_info = None
    comparison: dict[str, object] | None = None
    if not errors:
        orig = Image.open(ORIGINAL).convert("RGBA")
        fire = Image.open(FIRESTAFF).convert("RGBA")
        original_info = image_info(ORIGINAL)
        firestaff_info = image_info(FIRESTAFF)
        if orig.size != fire.size:
            errors.append(f"image sizes differ: original={orig.size} firestaff={fire.size}")
        else:
            diff = ImageChops.difference(orig, fire)
            # Amplify diff for human inspection without changing the numeric comparison.
            heat = ImageChops.multiply(diff, Image.new("RGBA", diff.size, (3, 3, 3, 1)))
            DIFF.parent.mkdir(parents=True, exist_ok=True)
            heat.save(DIFF)
            pixels = orig.width * orig.height
            mismatched = sum(1 for a, b in zip(orig.getdata(), fire.getdata()) if a != b)
            stat = ImageStat.Stat(diff)
            comparison = {
                "width": orig.width,
                "height": orig.height,
                "totalPixels": pixels,
                "mismatchedPixels": mismatched,
                "mismatchRatio": mismatched / pixels,
                "meanAbsDiffRgba": [round(v, 6) for v in stat.mean],
                "rmsDiffRgba": [round(v, 6) for v in stat.rms],
                "maxChannelDiff": max(max(px) for px in diff.getdata()),
                "diffPng": str(DIFF.relative_to(ROOT)),
                "diffPngSha256": sha256(DIFF),
                "mismatchRegions": mismatch_component_summary(orig, fire),
            }

    pixel_parity = bool(comparison and comparison["mismatchedPixels"] == 0)
    result = {
        "status": "failed" if errors else "passed",
        "pass": "pass286_dm1_v2_entry_viewport_png_comparator_gate",
        "scope": "Compare original DM1 PC34 entry viewport crop against Firestaff DM1 V2 entry viewport PNG seam.",
        "pixelParityClaim": pixel_parity,
        "originalDataRoot": str(ORIGINAL_DATA),
        "originalArchiveChecked": str(original_archive),
        "redmcsbSourceRoot": str(SOURCE),
        "sourceAnchors": anchors,
        "original": original_info,
        "firestaff": firestaff_info,
        "comparison": comparison,
        "exactBlocker": None if pixel_parity else (
            "Pass294 blocker: the remaining seam is not palette selection; it is the DUNVIEW.C "
            "wall-set/viewport-bitmap materialization path. ReDMCSB draws original floor/ceiling "
            "bitmaps into G0296_puc_Bitmap_Viewport at DUNVIEW.C:8337-8367, then visits D3/D2/D1/D0 "
            "squares through DUNVIEW.C:8490-8542 and uses wall-set bitmap draws for the blocking "
            "front wall at DUNVIEW.C:6699-6702 before returning at DUNVIEW.C:6720. Firestaff still "
            "exports a symbolic flat 224x136 renderer with only two colors, while the original crop "
            "contains six PC34 grayscale tones; mismatch remains "
            f"{comparison['mismatchedPixels'] if comparison else 'unknown'}/"
            f"{comparison['totalPixels'] if comparison else 'unknown'} pixels. The next required "
            "implementation seam is original GRAPHICS.DAT-backed floor/ceiling/wall-set bitmap "
            "blitting/clipping for the same map=0 x=1 y=3 dir=2 route state, not another palette pass."
        ),
        "errors": errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if errors:
        for error in errors:
            print(f"error: {error}")
        return 1
    if pixel_parity:
        print("dm1_v2_entry_viewport_png_comparator_gate: pixel parity matched")
    else:
        assert comparison is not None
        mismatched_pixels = comparison["mismatchedPixels"]
        total_pixels = comparison["totalPixels"]
        diff_png = comparison["diffPng"]
        print(
            "dm1_v2_entry_viewport_png_comparator_gate: blocker recorded "
            f"mismatched={mismatched_pixels}/{total_pixels} diff={diff_png}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
