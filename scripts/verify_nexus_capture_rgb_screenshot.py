#!/usr/bin/env python3
"""Compare a captured RGB replay against an authentic emulator screenshot."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from PIL import Image


def verify_manifest_screenshot(root: Path, path: Path) -> None:
    manifest = root / "SHA256SUMS.txt"
    relative = path.resolve().relative_to(root.resolve()).as_posix()
    entries = {}
    for line in manifest.read_text(encoding="utf-8").splitlines():
        if line.strip():
            digest, name = line.split("  ", 1)
            entries[name.removeprefix("./")] = digest
    expected = entries.get(relative)
    actual = hashlib.sha256(path.read_bytes()).hexdigest()
    if expected is None or actual != expected:
        raise ValueError(f"file is absent from or disagrees with the manifest: {relative}")


def compare(rgb_path: Path, screenshot_path: Path, width: int, height: int,
            scale: int, tolerance: int, manifest_root: Path | None) -> tuple[int, int, int]:
    from PIL import Image

    rgb = rgb_path.read_bytes()
    if len(rgb) != width * height * 3:
        raise ValueError("RGB replay has an unexpected byte count")
    if manifest_root is not None:
        verify_manifest_screenshot(manifest_root, screenshot_path)
    with Image.open(screenshot_path) as image:
        image = image.convert("RGB")
        if image.size != (width * scale, height * scale):
            raise ValueError("screenshot dimensions do not match replay size and scale")
        samples = 0
        mismatched = 0
        max_error = 0
        for y in range(height):
            for x in range(width):
                offset = (y * width + x) * 3
                replay = rgb[offset:offset + 3]
                for dy in range(scale):
                    for dx in range(scale):
                        reference = image.getpixel((x * scale + dx, y * scale + dy))
                        error = max(abs(replay[channel] - reference[channel])
                                    for channel in range(3))
                        samples += 1
                        max_error = max(max_error, error)
                        mismatched += error > tolerance
    if mismatched:
        raise ValueError(
            f"{mismatched}/{samples} screenshot samples exceed tolerance "
            f"{tolerance}; maximum channel error is {max_error}")
    return samples, max_error, mismatched


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rgb", type=Path, help="packed RGB replay, three bytes per pixel")
    parser.add_argument("screenshot", type=Path, help="authentic emulator screenshot")
    parser.add_argument("--width", type=int, default=320)
    parser.add_argument("--height", type=int, default=224)
    parser.add_argument("--scale", type=int, default=2)
    parser.add_argument("--tolerance", type=int, default=8)
    parser.add_argument("--manifest-root", type=Path,
                        help="optionally verify the screenshot via SHA256SUMS.txt")
    args = parser.parse_args()
    if (args.width <= 0 or args.height <= 0 or args.scale <= 0 or
            args.tolerance < 0 or args.tolerance > 255):
        parser.error("dimensions and scale must be positive; tolerance must be 0..255")
    try:
        samples, max_error, _ = compare(
            args.rgb, args.screenshot, args.width, args.height,
            args.scale, args.tolerance, args.manifest_root)
    except (OSError, ValueError) as error:
        parser.error(str(error))
    print(f"samples={samples}")
    print(f"max_channel_error={max_error}")
    print(f"tolerance={args.tolerance}")
    print("renderer_permitted=false")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
