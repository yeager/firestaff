#!/usr/bin/env python3
"""Compare the Firestaff C capture replay with a retained Ymir title frame."""

from __future__ import annotations

import argparse
import csv
import os
import subprocess
import tempfile
from pathlib import Path

from PIL import Image


EXPECTED_SIZES = {
    "vdp1-fbs.bin": 512 * 1024,
    "vdp2-vram.bin": 512 * 1024,
    "vdp2-cram.bin": 4096,
    "vdp2-registers.bin": 512,
}


def verify_bundle(bundle: Path) -> None:
    manifest = {}
    for line in (bundle / "vdp1-capture-bundle.txt").read_text().splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            manifest[key] = value
    expected = {
        "capture_frame_index": "59",
        "capture_next_command_index": "178",
        "trace_enabled": "1",
        "trace_rows": "10668",
        "trace_file_complete": "1",
        "trace_rows_for_capture_frame": "178",
        "trace_dropped_total": "0",
        "trace_last_frame": "59",
    }
    for key, value in expected.items():
        if manifest.get(key) != value:
            raise ValueError(f"capture manifest {key}={manifest.get(key)!r}, expected {value!r}")
    if manifest.get("trace_file") != "vdp1-command-trace.csv":
        raise ValueError("capture manifest points to a different command trace")
    trace = bundle / manifest["trace_file"]
    if not trace.is_file() or trace.stat().st_size == 0:
        raise ValueError("complete command trace is missing")
    with trace.open(newline="") as stream:
        rows = list(csv.DictReader(stream))
    frame_rows = [row for row in rows if row.get("frame") == "59"]
    if len(rows) != 10668 or len(frame_rows) != 178 or [
        row.get("index") for row in frame_rows
    ] != [str(index) for index in range(178)]:
        raise ValueError("frame-59 command trace rows are incomplete or reordered")
    for name, size in EXPECTED_SIZES.items():
        path = bundle / name
        if not path.is_file() or path.stat().st_size != size:
            raise ValueError(f"incomplete capture component: {name}")


def verify(executable: Path, bundle: Path, screenshot: Path) -> None:
    verify_bundle(bundle)
    image = Image.open(screenshot).convert("RGB")
    if image.size != (640, 448):
        raise ValueError(f"expected 640x448 screenshot, got {image.size}")
    with tempfile.TemporaryDirectory(prefix="firestaff-ymir-menu-") as temporary:
        replay = Path(temporary) / "capture.rgb"
        env = os.environ.copy()
        env.update(
            FIRESTAFF_NEXUS_YMIR_FBS=str(bundle / "vdp1-fbs.bin"),
            FIRESTAFF_NEXUS_YMIR_VDP2_VRAM=str(bundle / "vdp2-vram.bin"),
            FIRESTAFF_NEXUS_YMIR_CRAM=str(bundle / "vdp2-cram.bin"),
            FIRESTAFF_NEXUS_YMIR_REGS=str(bundle / "vdp2-registers.bin"),
            FIRESTAFF_NEXUS_YMIR_CAPTURE_ATTESTED="1",
            FIRESTAFF_NEXUS_YMIR_TRACE_ATTESTED="1",
            FIRESTAFF_NEXUS_YMIR_REPLAY_OUT=str(replay),
        )
        subprocess.run([str(executable)], env=env, check=True)
        pixels = replay.read_bytes()
    width, height = 320, 224
    if len(pixels) != width * height * 3:
        raise ValueError(f"C replay produced {len(pixels)} bytes, expected {width * height * 3}")
    compared = 0
    for y in range(height):
        for x in range(width):
            offset = (y * width + x) * 3
            expected_rgb = tuple(pixels[offset:offset + 3])
            for dy in range(2):
                for dx in range(2):
                    if image.getpixel((2 * x + dx, 2 * y + dy)) != expected_rgb:
                        raise ValueError(
                            f"Ymir screenshot mismatch at output pixel ({x}, {y}), "
                            f"subpixel ({dx}, {dy})"
                        )
                    compared += 1
    print(f"Ymir title frame: exact C replay match for {compared} screenshot pixels")
    print("capture-only=1 renderer-permitted=0 frame-identity=unproven")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("compositor_test", type=Path)
    parser.add_argument("bundle", type=Path)
    parser.add_argument("screenshot", type=Path)
    args = parser.parse_args()
    verify(args.compositor_test.resolve(), args.bundle.resolve(), args.screenshot.resolve())
