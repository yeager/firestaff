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
CAPTURE_PROFILES = {
    59: {
        "trace_rows": 10668,
        "frame_rows": 178,
        "trace_last_frame": 59,
        "hashes": {
            "vdp1-fbs.bin": "2dc15ef07aaacd89a00af18fe3e0bf3677bdb626f3a852f37820eb29bc9ce1ce",
            "vdp2-vram.bin": "de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c",
            "vdp2-cram.bin": "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de",
            "vdp2-registers.bin": "79efa616c2db727749aa9018864d1ad32c1583935652fafb16534ba69f8e1f2d",
            "vdp1-command-trace.csv": "b80c7a118e7f9df4f941804cdc045f4024336080504c512bbdf5c4e0df19d8d3",
            "vdp1-capture-bundle.txt": "0e5cfa33070e33f24ae0e25ead7ce4fac561ae7c329ef7c59ff12382d3ea519e",
            "vdp-register-snapshot.txt": "490768f20d3c128307c375e793886e88488911feb9673cb93d8ac078b582da42",
            "screenshot.png": "8721cce3354bc20b1cd367fd44be3d4ecc60ca65fa89c7d00654e83f5ad7db23",
        },
        "requires_run_log": False,
    },
    8047: {
        "trace_rows": 331364,
        "frame_rows": 178,
        "trace_last_frame": 8047,
        "hashes": {
            "vdp1-fbs.bin": "2752a34f5d200c965e2fe5c35ff6f973361319d769a2d76944af2c7b0f480979",
            "vdp2-vram.bin": "de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c",
            "vdp2-cram.bin": "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de",
            "vdp2-registers.bin": "e69d03094ef856922413e073cb9078015e52849786268d6421c7df00a655965e",
            "vdp1-command-trace.csv": "d13fcede5adcad648b9c167df9c54eb928760185886bf667223f1272bd505e98",
            "vdp1-capture-bundle.txt": "7553b065bccc6554addfb7e59d3e98ff40defa97065bde34e505ba1828ab548c",
            "vdp-register-snapshot.txt": "9ae54db09e700497b0b7af44334dc645456038a5fdedf21eaddc8b9a0c869e89",
            "run.log": "c265d0970b7b6d966f927976daad6c3f3178d1feca90b95ee23ad553489c1bc2",
            "screenshot.png": "741c620e945b5ab3e05f6bc5d182fc4ada9439071b79718f67326bcdd56c6118",
        },
        "requires_run_log": True,
    },
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()
BUILD = Path(__file__).resolve().parents[1] / "build"
BUILD.mkdir(exist_ok=True)


def verify_bundle(bundle: Path) -> tuple[int, dict[str, str]]:
    manifest = {}
    for line in (bundle / "vdp1-capture-bundle.txt").read_text().splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            manifest[key] = value
    try:
        frame_index = int(manifest["capture_frame_index"])
    except (KeyError, ValueError) as error:
        raise ValueError("capture manifest has no valid frame index") from error
    profile = CAPTURE_PROFILES.get(frame_index)
    if profile is None:
        raise ValueError(f"no retained title-frame profile for frame {frame_index}")
    expected = {
        "capture_next_command_index": "178",
        "trace_enabled": "1",
        "trace_rows": str(profile["trace_rows"]),
        "trace_file_complete": "1",
        "trace_rows_for_capture_frame": str(profile["frame_rows"]),
        "trace_dropped_total": "0",
        "trace_last_frame": str(profile["trace_last_frame"]),
    }
    for key, value in expected.items():
        if manifest.get(key) != value:
            raise ValueError(f"capture manifest {key}={manifest.get(key)!r}, expected {value!r}")
    if manifest.get("trace_file") != "vdp1-command-trace.csv":
        raise ValueError("capture manifest points to a different command trace")
    trace = bundle / manifest["trace_file"]
    if not trace.is_file() or trace.stat().st_size == 0:
        raise ValueError("complete command trace is missing")
    hashes = profile["hashes"]
    if sha256(trace) != hashes["vdp1-command-trace.csv"]:
        raise ValueError("command trace SHA-256 does not match the retained Ymir capture")
    with trace.open(newline="") as stream:
        rows = list(csv.DictReader(stream))
    frame_text = str(frame_index)
    frame_rows = [row for row in rows if row.get("frame") == frame_text]
    if len(rows) != profile["trace_rows"] or len(frame_rows) != profile["frame_rows"] or [
        row.get("index") for row in frame_rows
    ] != [str(index) for index in range(178)]:
        raise ValueError("captured-frame command trace rows are incomplete or reordered")
    for name in ("vdp1-capture-bundle.txt", "vdp-register-snapshot.txt"):
        if sha256(bundle / name) != hashes[name]:
            raise ValueError(f"capture receipt SHA-256 mismatch: {name}")
    for name, size in EXPECTED_SIZES.items():
        path = bundle / name
        if not path.is_file() or path.stat().st_size != size:
            raise ValueError(f"incomplete capture component: {name}")
        if sha256(path) != hashes[name]:
            raise ValueError(f"capture component SHA-256 mismatch: {name}")
    register_receipt = {}
    for line in (bundle / "vdp-register-snapshot.txt").read_text().splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            register_receipt[key] = value
    if int(register_receipt.get("vdp1_frame_index", "-1")) != frame_index:
        raise ValueError("VDP1 register snapshot is not paired to the capture frame")
    return frame_index, hashes


def verify_screenshot_capture_pair(bundle: Path, screenshot: Path, run_log: Path,
                                   frame_index: int, expected_run_log_hash: str) -> None:
    if bundle.parent.resolve() != run_log.parent.resolve():
        raise ValueError("dump bundle and run log must belong to one capture directory")
    if screenshot.parent.resolve() != run_log.parent.resolve() / "screenshots":
        raise ValueError("screenshot must be retained in the capture's screenshots directory")
    if sha256(run_log) != expected_run_log_hash:
        raise ValueError("Ymir run log SHA-256 does not match the retained capture")
    logged = []
    for line in run_log.read_text(encoding="utf-8", errors="replace").splitlines():
        marker = "Screenshot saved to "
        if marker in line:
            logged.append(Path(line.split(marker, 1)[1].strip()).name)
    if logged.count(screenshot.name) != 1:
        raise ValueError("run log does not identify exactly one supplied screenshot")
    receipt = {}
    for line in (bundle / "vdp-register-snapshot.txt").read_text().splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            receipt[key] = value
    if int(receipt.get("vdp1_frame_index", "-1")) != frame_index:
        raise ValueError("VDP1 register receipt does not name the selected trace frame")


def verify(executable: Path, bundle: Path, screenshot: Path,
           run_log: Path | None = None) -> None:
    frame_index, hashes = verify_bundle(bundle)
    profile = CAPTURE_PROFILES[frame_index]
    if profile["requires_run_log"] and run_log is None:
        raise ValueError("this frame profile requires its same-run screenshot log")
    if run_log is not None:
        expected_run_log_hash = profile["hashes"].get("run.log")
        if expected_run_log_hash is None:
            raise ValueError("this frame profile has no authenticated run-log hash")
        verify_screenshot_capture_pair(bundle, screenshot, run_log, frame_index,
                                      expected_run_log_hash)
    if sha256(screenshot) != hashes["screenshot.png"]:
        raise ValueError("screenshot SHA-256 does not match the retained Ymir capture")
    image = Image.open(screenshot).convert("RGB")
    if image.size != (640, 448):
        raise ValueError(f"expected 640x448 screenshot, got {image.size}")
    with tempfile.TemporaryDirectory(prefix="firestaff-ymir-menu-", dir=BUILD) as temporary:
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
    print(f"capture-frame={frame_index} capture-only=1 renderer-permitted=0")
    print("screenshot-capture=paired; exact-frame-id=unproven" if run_log
          else "screenshot-capture=unproven; exact-frame-id=unproven")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("compositor_test", type=Path)
    parser.add_argument("bundle", type=Path)
    parser.add_argument("screenshot", type=Path)
    parser.add_argument("--run-log", type=Path,
                        help="Ymir log from the same capture directory as the dump bundle")
    args = parser.parse_args()
    verify(args.compositor_test.resolve(), args.bundle.resolve(),
           args.screenshot.resolve(), args.run_log.resolve() if args.run_log else None)
