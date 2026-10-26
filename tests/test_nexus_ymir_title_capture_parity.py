#!/usr/bin/env python3
"""Compare the Firestaff C capture replay with a retained Ymir title frame."""

from __future__ import annotations

import argparse
import csv
import hashlib
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
EXPECTED_SHA256 = {
    "vdp1-fbs.bin": "2dc15ef07aaacd89a00af18fe3e0bf3677bdb626f3a852f37820eb29bc9ce1ce",
    "vdp2-vram.bin": "de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c",
    "vdp2-cram.bin": "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de",
    "vdp2-registers.bin": "79efa616c2db727749aa9018864d1ad32c1583935652fafb16534ba69f8e1f2d",
    "vdp1-command-trace.csv": "b80c7a118e7f9df4f941804cdc045f4024336080504c512bbdf5c4e0df19d8d3",
    "screenshot.png": "8721cce3354bc20b1cd367fd44be3d4ecc60ca65fa89c7d00654e83f5ad7db23",
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


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
    if sha256(trace) != EXPECTED_SHA256["vdp1-command-trace.csv"]:
        raise ValueError("command trace SHA-256 does not match the retained Ymir capture")
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
        if sha256(path) != EXPECTED_SHA256[name]:
            raise ValueError(f"capture component SHA-256 mismatch: {name}")


def verify(executable: Path, bundle: Path, screenshot: Path) -> None:
    verify_bundle(bundle)
    if sha256(screenshot) != EXPECTED_SHA256["screenshot.png"]:
        raise ValueError("screenshot SHA-256 does not match the retained Ymir capture")
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
