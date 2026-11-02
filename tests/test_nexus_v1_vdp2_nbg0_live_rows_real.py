#!/usr/bin/env python3
"""Validate the authentic full-frame NBG0 replay against Ymir output."""
from __future__ import annotations

import csv
import hashlib
import os
from pathlib import Path
import subprocess
import sys

from PIL import Image


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_receipt(path: Path) -> dict[str, str]:
    return dict(
        line.split("=", 1)
        for line in path.read_text(encoding="utf-8").splitlines()
        if "=" in line
    )


def main() -> int:
    if len(sys.argv) != 3:
        raise SystemExit("usage: test_nexus_v1_vdp2_nbg0_live_rows_real.py EXECUTABLE OUTPUT")
    root_value = os.environ.get("FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR")
    baseline_value = os.environ.get("FIRESTAFF_NEXUS_NBG0_LIVE_BASELINE_REPLAY")
    trace_value = os.environ.get("FIRESTAFF_NEXUS_NBG0_LIVE_ROW_TRACE")
    if not root_value or not baseline_value or not trace_value:
        print("SKIP: set FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR, FIRESTAFF_NEXUS_NBG0_LIVE_BASELINE_REPLAY, and FIRESTAFF_NEXUS_NBG0_LIVE_ROW_TRACE")
        return 77

    root = Path(root_value)
    screenshot = root / "screenshots" / "DUNGEON MASTER NEXUS [T-9111G]-20261029T151400.554.png"
    baseline_path = Path(baseline_value)
    trace_path = Path(trace_value)
    output = Path(sys.argv[2])

    vdp1_fbs = (root / "dumps" / "vdp1-fbs.bin").read_bytes()
    vram = (root / "dumps" / "vdp2-vram.bin").read_bytes()
    cram = (root / "dumps" / "vdp2-cram.bin").read_bytes()
    registers = (root / "dumps" / "vdp2-registers.bin").read_bytes()
    rgbx_path = root / "dumps" / "rendered-frames" / "vdp1-7486-vdp2-8199-320x224.rgbx8888le"
    rgbx = rgbx_path.read_bytes()
    trace_bytes = trace_path.read_bytes()

    expected_hashes = {
        "authentic VDP1 framebuffer pair": (vdp1_fbs, "0570c79b067adde55161ba2e16c09a628dcdf99b7b49f168d803c948b4479420"),
        "authentic VDP2 VRAM": (vram, "2b6fe0392b1ff23c0c55f08423d179a74d7f481990e37aadd4e6bfd657d9cf45"),
        "authentic VDP2 CRAM": (cram, "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de"),
        "same-pass Ymir RGBX frame": (rgbx, "6d0742b5e7c96c523890c4f15b6fe0c51538c8330393b33eaeb66ce29cbd11a8"),
        "full-height Ymir NBG0 row trace": (trace_bytes, "d45c8a334d8c74bf3db8b950ffa8dc0e999f50c0e9e5da1d637e2719d366c02b"),
        "authentic 640x448 Ymir screenshot": (screenshot.read_bytes(), "f9f2175b5f75616cbd7c942a7d65f5fcdb20012186b5069aebe77be4e4a85c94"),
        "retained Firestaff baseline replay": (baseline_path.read_bytes(), "ab4e62971ef82515764de5433b7ce933a27693f53f714115b2ff216cae847ad7"),
    }
    for label, (data, expected) in expected_hashes.items():
        if digest(data) != expected:
            raise SystemExit(f"{label} SHA-256 mismatch")

    if len(vdp1_fbs) != 524288 or len(vram) != 0x80000 or len(cram) != 4096 or len(registers) != 512:
        raise SystemExit("unexpected authentic Saturn capture sizes")
    if len(rgbx) != 320 * 224 * 4:
        raise SystemExit("unexpected same-pass Ymir RGBX dimensions")

    render_receipt = read_receipt(root / "dumps" / "nexus-nbg0-vdp2-render-state.csv.receipt.txt")
    expected_render_receipt = {
        "format": "ymir-vdp2-render-state-trace-v3",
        "frame_range_half_open": "7486:7487",
        "row_vcnt_range_half_open": "0:224",
        "expected_main_samples": "224",
        "observed_main_samples": "224",
        "missing_main_samples": "0",
        "observed_main_frame_ids": "7486",
        "observed_vdp2_frame_serial_ids": "8199",
        "rendered_frame_output_status": "complete",
        "rows_written": "224",
        "write_error": "0",
        "capture_status": "complete",
    }
    if any(render_receipt.get(key) != value for key, value in expected_render_receipt.items()):
        raise SystemExit("full-height Ymir NBG0 render-state receipt is incomplete or mismatched")

    layer_receipt = read_receipt(root / "dumps" / "nexus-vdp2-layer-outputs.bin.receipt.txt")
    expected_layer_receipt = {
        "format": "ymir-vdp2-layer-output-trace-v1",
        "records_written": "1568",
        "bytes_written": "3054472",
        "frame_range_half_open": "7486:7487",
        "row_vcnt_range_half_open": "0:224",
        "observed_vdp2_serial_ids": "8199",
        "capture_status": "complete",
        "write_error": "0",
    }
    if any(layer_receipt.get(key) != value for key, value in expected_layer_receipt.items()):
        raise SystemExit("full-height Ymir layer-output receipt is incomplete or mismatched")

    memory_receipt = read_receipt(root / "dumps" / "vdp2-memory-write-bundle.txt")
    expected_memory_receipt = {
        "current_vdp1_frame_index": "7486",
        "trace_file_complete": "1",
        "vdp2_vram_file_complete": "1",
        "vdp2_cram_file_complete": "1",
        "vdp2_register_file_complete": "1",
        "trace_rows": "0",
        "trace_dropped_total": "0",
        "trace_vdp1_frame_begin": "7486",
        "trace_vdp1_frame_end_exclusive": "7487",
        "bundle_complete": "1",
    }
    if any(memory_receipt.get(key) != value for key, value in expected_memory_receipt.items()):
        raise SystemExit("frame-matched Ymir VDP2 memory receipt is incomplete or mismatched")

    with trace_path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        required_columns = {
            "frame_index", "vdp2_frame_serial", "row_vcnt", "alternate_field",
            "scroll_h_11_8", "scroll_v_11_8", "scroll_inc_h_11_8",
            "scroll_inc_v_11_8", "priority", "color_calculation",
            "color_calculation_ratio", "bitmap", "color_format",
            "nbg0_enabled", "layer_enabled", "rbg1_enabled",
        }
        if not reader.fieldnames or not required_columns.issubset(reader.fieldnames):
            raise SystemExit("full-height Ymir row trace lacks required fields")
        rows = list(reader)
    if len(rows) != 224 or [int(row["row_vcnt"]) for row in rows] != list(range(224)):
        raise SystemExit("Ymir trace must contain every output row 0 through 223 exactly once")

    expected_row = {
        "frame_index": "7486", "vdp2_frame_serial": "8199", "alternate_field": "0",
        "scroll_h_11_8": "8057", "scroll_v_11_8": "3021",
        "scroll_inc_h_11_8": "205", "scroll_inc_v_11_8": "205",
        "priority": "4", "color_calculation": "1", "color_calculation_ratio": "15",
        "bitmap": "1", "color_format": "1", "nbg0_enabled": "1",
        "layer_enabled": "1", "rbg1_enabled": "0",
    }
    for row in rows:
        if any(row.get(key) != value for key, value in expected_row.items()):
            raise SystemExit(f"Ymir row {row.get('row_vcnt')} differs from the full-height capture state")

    register_words = {
        offset: int.from_bytes(registers[offset:offset + 2], "big")
        for offset in (0x20, 0x28, 0x32, 0x3a, 0x3c, 0x44, 0x46)
    }
    if register_words != {0x20: 0x0003, 0x28: 0x0013, 0x32: 0, 0x3a: 0,
                          0x3c: 0, 0x44: 0x1717, 0x46: 0x1717}:
        raise SystemExit("frame-matched VDP2 NBG0/NBG1 register profile mismatch")

    # MAP1A/B=0x1717 with PLSZ/MPOFN zero places the NBG1 map at 0x5c000.
    # Verify every authentic PND and the 4bpp character bytes before attesting
    # transparent NBG1 beneath the NBG0 scanline replay.
    map_base = 0x5c000
    pnds = [vram[map_base + cell * 4:map_base + cell * 4 + 4] for cell in range(64 * 64)]
    if any(pnd != b"\x00\x00\x10\x00" for pnd in pnds):
        raise SystemExit("authentic NBG1 64x64 map is not uniformly (attr=0,charno=0x1000)")
    if vram[0x20000:0x20020] != bytes(32):
        raise SystemExit("authentic NBG1 character 0x1000 is not fully transparent")

    env = os.environ.copy()
    env["FIRESTAFF_NEXUS_NBG0_LIVE_REPLAY_RGB"] = str(output)
    process = subprocess.run([sys.argv[1]], env=env, check=False)
    if process.returncode:
        return process.returncode
    try:
        actual = output.read_bytes()
        baseline = baseline_path.read_bytes()
        if len(actual) != 320 * 224 * 3 or len(baseline) != len(actual):
            raise SystemExit("unexpected RGB replay dimensions")
        exact = sum(
            actual[pixel * 3:pixel * 3 + 3] == rgbx[pixel * 4:pixel * 4 + 3]
            for pixel in range(320 * 224)
        )
        if exact != 320 * 224:
            raise SystemExit(f"Firestaff C replay matches {exact}/71680 same-pass Ymir RGB pixels")
        screenshot_rgb = Image.open(screenshot).convert("RGB")
        if screenshot_rgb.size != (640, 448):
            raise SystemExit(f"unexpected screenshot size: {screenshot_rgb.size}")
        old_exact = live_exact = 0
        for y in range(224):
            for x in range(320):
                offset = (y * 320 + x) * 3
                prior = baseline[offset:offset + 3]
                current = actual[offset:offset + 3]
                old_exact += prior == bytes(screenshot_rgb.getpixel((x * 2, y * 2)))
                for dy in range(2):
                    for dx in range(2):
                        live_exact += current == bytes(screenshot_rgb.getpixel((x * 2 + dx, y * 2 + dy)))
        if old_exact != 70454 or live_exact != 286720:
            raise SystemExit(f"unexpected authentic screenshot match: old={old_exact}/71680 live={live_exact}/286720")
        print(f"PASS: full-height authentic C NBG0 replay={exact}/71680 same-pass Ymir RGB pixels; NBG1 transparency checked across 4096 PNDs; screenshot={live_exact}/286720 doubled pixels exact")
        return 0
    finally:
        output.unlink(missing_ok=True)


if __name__ == "__main__":
    raise SystemExit(main())
