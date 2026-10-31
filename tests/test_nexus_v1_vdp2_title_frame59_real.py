#!/usr/bin/env python3
"""Run the authentic frame-59 C replay and compare it with the Ymir image."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import subprocess
import sys


def main() -> int:
    if len(sys.argv) != 3:
        raise SystemExit("usage: test_nexus_v1_vdp2_title_frame59_real.py EXECUTABLE OUTPUT")
    capture_root = os.environ.get("FIRESTAFF_NEXUS_YMIR_FRAME59_CAPTURE_DIR")
    if not capture_root:
        print("SKIP: set FIRESTAFF_NEXUS_YMIR_FRAME59_CAPTURE_DIR")
        return 77

    from PIL import Image

    root = Path(capture_root)
    output = Path(sys.argv[2])
    output.parent.mkdir(parents=True, exist_ok=True)
    environment = os.environ.copy()
    environment["FIRESTAFF_NEXUS_YMIR_FRAME59_REPLAY_OUT"] = str(output)
    try:
        process = subprocess.run([sys.argv[1]], env=environment, check=False)
        if process.returncode == 77:
            return 77
        if process.returncode != 0:
            return process.returncode

        screenshot_path = root / "screenshots" / (
            "DUNGEON MASTER NEXUS [T-9111G]-20261030T085957.601.png"
        )
        screenshot_bytes = screenshot_path.read_bytes()
        expected_sha256 = (
            "2fa29fd409034fbbe404d698d33ee5f12e0c4c55caeebc9c7533ffcb540a0624"
        )
        if hashlib.sha256(screenshot_bytes).hexdigest() != expected_sha256:
            raise SystemExit("frame-59 screenshot SHA-256 mismatch")

        rgb = output.read_bytes()
        if len(rgb) != 320 * 224 * 3:
            raise SystemExit(f"unexpected frame-59 replay size: {len(rgb)}")
        image = Image.open(screenshot_path).convert("RGB")
        if image.size != (640, 448):
            raise SystemExit(f"unexpected frame-59 screenshot dimensions: {image.size}")

        max_error = 0
        within_eight = 0
        for y in range(224):
            for x in range(320):
                offset = (y * 320 + x) * 3
                actual = rgb[offset : offset + 3]
                for dy in range(2):
                    for dx in range(2):
                        expected = image.getpixel((x * 2 + dx, y * 2 + dy))
                        error = max(
                            abs(actual[channel] - expected[channel])
                            for channel in range(3)
                        )
                        max_error = max(max_error, error)
                        within_eight += error <= 8
        if within_eight != 286720 or max_error > 8:
            raise SystemExit(
                f"frame-59 screenshot mismatch: within8={within_eight}/286720, "
                f"max_error={max_error}"
            )
        print(
            "PASS: frame-59 replay matches all 286720 doubled screenshot samples "
            f"within 8 RGB levels; max_error={max_error}"
        )
        return 0
    finally:
        output.unlink(missing_ok=True)


if __name__ == "__main__":
    raise SystemExit(main())
