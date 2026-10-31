#!/usr/bin/env python3
"""Validate the authentic NBG0 live-row replay against retained Ymir output."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import subprocess
import sys


def main() -> int:
    if len(sys.argv) != 3:
        raise SystemExit("usage: test_nexus_v1_vdp2_nbg0_live_rows_real.py EXECUTABLE OUTPUT")
    root_value = os.environ.get("FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR")
    baseline_value = os.environ.get("FIRESTAFF_NEXUS_NBG0_LIVE_BASELINE_REPLAY")
    if not root_value or not baseline_value:
        print("SKIP: set FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR and FIRESTAFF_NEXUS_NBG0_LIVE_BASELINE_REPLAY")
        return 77
    from PIL import Image

    root = Path(root_value)
    screenshot = root / "screenshots" / "DUNGEON MASTER NEXUS [T-9111G]-20261029T151400.554.png"
    old_replay = Path(baseline_value)
    output = Path(sys.argv[2])
    expected_screenshot_hash = "f9f2175b5f75616cbd7c942a7d65f5fcdb20012186b5069aebe77be4e4a85c94"
    if hashlib.sha256(screenshot.read_bytes()).hexdigest() != expected_screenshot_hash:
        raise SystemExit("authentic intro screenshot SHA-256 mismatch")
    if hashlib.sha256(old_replay.read_bytes()).hexdigest() != "ab4e62971ef82515764de5433b7ce933a27693f53f714115b2ff216cae847ad7":
        raise SystemExit("retained Firestaff baseline replay SHA-256 mismatch")
    env = os.environ.copy()
    env["FIRESTAFF_NEXUS_NBG0_LIVE_REPLAY_RGB"] = str(output)
    process = subprocess.run([sys.argv[1]], env=env, check=False)
    if process.returncode:
        return process.returncode
    try:
        actual = output.read_bytes()
        baseline = old_replay.read_bytes()
        if len(actual) != 320 * 224 * 3 or len(baseline) != len(actual):
            raise SystemExit("unexpected RGB replay dimensions")
        screenshot_rgb = Image.open(screenshot).convert("RGB")
        if screenshot_rgb.size != (640, 448):
            raise SystemExit(f"unexpected screenshot size: {screenshot_rgb.size}")
        old_exact = live_exact = 0
        for y in range(224):
            for x in range(320):
                offset = (y * 320 + x) * 3
                prior = baseline[offset:offset + 3]
                current = actual[offset:offset + 3]
                ref = screenshot_rgb.getpixel((x * 2, y * 2))
                old_exact += prior == bytes(ref)
                for dy in range(2):
                    for dx in range(2):
                        live_exact += current == bytes(
                            screenshot_rgb.getpixel((x * 2 + dx, y * 2 + dy))
                        )
        if old_exact != 70454 or live_exact != 286720:
            raise SystemExit(f"unexpected authentic screenshot match: old={old_exact}/71680 live={live_exact}/286720")
        print(f"PASS: baseline={old_exact}/71680 top-left samples; live-row replay={live_exact}/286720 doubled screenshot samples exact")
        return 0
    finally:
        output.unlink(missing_ok=True)


if __name__ == "__main__":
    raise SystemExit(main())
