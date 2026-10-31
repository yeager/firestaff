#!/usr/bin/env python3
"""Validate the authentic NBG0 live-row replay against retained Ymir output."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import subprocess
import sys


def main() -> int:
    if len(sys.argv) != 3:
        raise SystemExit("usage: test_nexus_v1_vdp2_nbg0_live_rows_real.py EXECUTABLE OUTPUT")
    root_value = os.environ.get("FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR")
    baseline_value = os.environ.get("FIRESTAFF_NEXUS_NBG0_LIVE_BASELINE_REPLAY")
    trace_value = os.environ.get("FIRESTAFF_NEXUS_NBG0_LIVE_ROW_TRACE")
    if not root_value or not baseline_value or not trace_value:
        print("SKIP: set FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR, FIRESTAFF_NEXUS_NBG0_LIVE_BASELINE_REPLAY, and FIRESTAFF_NEXUS_NBG0_LIVE_ROW_TRACE")
        return 77
    from PIL import Image

    root = Path(root_value)
    screenshot = root / "screenshots" / "DUNGEON MASTER NEXUS [T-9111G]-20261029T151400.554.png"
    old_replay = Path(baseline_value)
    row_trace = Path(trace_value)
    output = Path(sys.argv[2])
    expected_screenshot_hash = "f9f2175b5f75616cbd7c942a7d65f5fcdb20012186b5069aebe77be4e4a85c94"
    if hashlib.sha256(screenshot.read_bytes()).hexdigest() != expected_screenshot_hash:
        raise SystemExit("authentic intro screenshot SHA-256 mismatch")
    if hashlib.sha256(old_replay.read_bytes()).hexdigest() != "ab4e62971ef82515764de5433b7ce933a27693f53f714115b2ff216cae847ad7":
        raise SystemExit("retained Firestaff baseline replay SHA-256 mismatch")
    trace_bytes = row_trace.read_bytes()
    if hashlib.sha256(trace_bytes).hexdigest() != "1f1ddff055c5189e8b42db9b15936135601f43bc7320873c14e0269a323c40ff":
        raise SystemExit("retained Ymir live-row trace SHA-256 mismatch")
    trace = trace_bytes.decode("utf-8")
    if len(re.findall(r"^TARGET frame_counter=7486$", trace, re.MULTILINE)) != 1:
        raise SystemExit("retained Ymir trace does not identify exactly frame 7486")
    rows = re.findall(r"^ROW y=(\d+) (.*)$", trace, re.MULTILINE)
    expected_rows = list(range(4, 117))
    if [int(y) for y, _ in rows] != expected_rows:
        raise SystemExit("retained Ymir trace must contain each output row 4 through 116 exactly once")
    expected_fields = {
        "bgon_n0": "1", "n0_transparent": "1", "n0_scroll_h": "0x1f79",
        "n0_scroll_v": "0xbcd", "n0_inc_h": "0xcd", "n0_inc_v": "0xcd",
        "n0_prio": "4", "n0_cc": "1", "n0_ratio": "15", "n0_sccm": "0",
    }
    for y, fields_text in rows:
        fields = dict(re.findall(r"([a-z0-9_]+)=([^ ]+)", fields_text))
        if any(fields.get(key) != value for key, value in expected_fields.items()):
            raise SystemExit(f"retained Ymir row {y} does not match the verified NBG0 tuple")
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
