#!/usr/bin/env python3
"""Verify one authentic gameplay frame's composition and retail DGN joins."""

from __future__ import annotations

import os
import csv
from collections import Counter
from pathlib import Path
import struct
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]

FRAME_WIDTH = 320
FRAME_HEIGHT = 224
VDP1_BUFFER_BYTES = 0x40000
VDP1_FRAMEBUFFER_PITCH = 512
NO_OWNER = 0xFFFF
VDP1_LAYER = 0
RETAIL_SOURCE_KINDS = {
    "retail-dgn-indexed",
    "retail-dgn-direct-rgb",
}
CAPTURE_SOURCE_KINDS = {
    "captured-mode4-vram",
    "captured-polyline",
}


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: test_nexus_v1_gameplay_frame300_composition_source_join_real.py <tilemap-test> <vdp1-stepper-test>")
        return 2
    evidence = Path(os.environ.get("FIRESTAFF_NEXUS_FRAME300_EVIDENCE_DIR", ""))
    capture = evidence / "capture-301"
    dumps = capture / "dumps"
    if not dumps.is_dir():
        print("SKIP: set FIRESTAFF_NEXUS_FRAME300_EVIDENCE_DIR to the authentic frame-300 Ymir evidence")
        return 77
    reference = capture / "vdp2-rendered" / "vdp1-300-vdp2-301-320x224.rgbx8888le"
    if not reference.is_file():
        print("SKIP: authentic frame-300 Ymir composed reference is unavailable")
        return 77

    with tempfile.TemporaryDirectory(prefix="firestaff-nexus-frame300-") as temp:
        output = Path(temp) / "firestaff-priority-stack.bin"
        owner_output = Path(temp) / "vdp1-command-owner-le16.bin"
        source_receipt_output = Path(temp) / "vdp1-command-source-receipt.csv"
        env = os.environ.copy()
        env.update({
            "FIRESTAFF_NEXUS_YMIR_FRAME300_VDP1_FBS": str(dumps / "vdp1-fbs.bin"),
            "FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_VRAM": str(dumps / "vdp2-vram.bin"),
            "FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_CRAM": str(dumps / "vdp2-cram.bin"),
            "FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_REGS": str(dumps / "vdp2-registers.bin"),
            "FIRESTAFF_NEXUS_YMIR_FRAME300_RENDERED": str(reference),
            "FIRESTAFF_NEXUS_YMIR_FRAME300_STACK_OUT": str(output),
        })
        composition = subprocess.run(
            [sys.argv[1]], check=False, env=env, capture_output=True, text=True
        )
        sys.stdout.write(composition.stdout)
        sys.stderr.write(composition.stderr)
        if composition.returncode != 0:
            return composition.returncode

        env["FIRESTAFF_NEXUS_YMIR_GAMEPLAY_FRAME300_CAPTURE_DIR"] = str(dumps)
        configured_data = os.environ.get("FIRESTAFF_NEXUS_DATA_DIR")
        env["FIRESTAFF_NEXUS_DATA_DIR"] = configured_data or str(
            Path.home() / ".firestaff" / "data" / "nexus"
        )
        if not Path(env["FIRESTAFF_NEXUS_DATA_DIR"]).is_dir():
            print("SKIP: authentic Japanese Nexus data directory is unavailable")
            return 77
        env["FIRESTAFF_NEXUS_YMIR_STEPPER_OWNER_OUT"] = str(owner_output)
        env["FIRESTAFF_NEXUS_YMIR_STEPPER_SOURCE_RECEIPT_OUT"] = str(source_receipt_output)
        stepper = subprocess.run(
            [sys.argv[2]], check=False, env=env, capture_output=True, text=True
        )
        sys.stdout.write(stepper.stdout)
        sys.stderr.write(stepper.stderr)
        if stepper.returncode != 0:
            return stepper.returncode

        vdp1_fbs = (dumps / "vdp1-fbs.bin").read_bytes()
        stack = output.read_bytes()
        owners = owner_output.read_bytes()
        if (len(vdp1_fbs) != 2 * VDP1_BUFFER_BYTES or
                len(stack) != FRAME_WIDTH * FRAME_HEIGHT * 4 or
                len(owners) != FRAME_WIDTH * FRAME_HEIGHT * 2):
            print("FAIL: frame-300 owner/composition evidence has an unexpected size")
            return 1
        with source_receipt_output.open(newline="") as source_file:
            source_rows = list(csv.DictReader(source_file))
        source_by_row = {int(row["index"]): row["source_kind"] for row in source_rows}
        if len(source_by_row) != 83 or any(
                source_by_row.get(index) in (None, "unset") for index in range(83)):
            print("FAIL: frame-300 command ownership lacks a complete 83-row source receipt")
            return 1

        vdp1_top_pixels = 0
        cram_pixels = 0
        cram_owner_counts: Counter[int] = Counter()
        source_owner_counts: Counter[str] = Counter()
        for pixel in range(FRAME_WIDTH * FRAME_HEIGHT):
            x = pixel % FRAME_WIDTH
            y = pixel // FRAME_WIDTH
            fbs_offset = (VDP1_BUFFER_BYTES +
                          (y * VDP1_FRAMEBUFFER_PITCH + x) * 2)
            sprite = struct.unpack_from(">H", vdp1_fbs, fbs_offset)[0]
            top_layer = stack[pixel * 4]
            owner = struct.unpack_from("<H", owners, pixel * 2)[0]
            replay_sprite = struct.unpack_from(
                ">H", vdp1_fbs, fbs_offset - VDP1_BUFFER_BYTES
            )[0]
            cram_sprite = top_layer == VDP1_LAYER and sprite != 0 and not (sprite & 0x8000)
            if top_layer == VDP1_LAYER and sprite != 0 and replay_sprite != sprite:
                print(f"FAIL: display and replay VDP1 buffers differ at visible pixel {pixel}")
                return 1
            if cram_sprite:
                cram_pixels += 1
                cram_owner_counts[owner] += 1
            if top_layer == VDP1_LAYER and sprite != 0:
                vdp1_top_pixels += 1
                if owner == NO_OWNER or owner not in source_by_row:
                    print(f"FAIL: visible VDP1 pixel {pixel} has no command owner")
                    return 1
                source_kind = source_by_row[owner]
                if source_kind not in RETAIL_SOURCE_KINDS | CAPTURE_SOURCE_KINDS:
                    print(f"FAIL: VDP1 pixel {pixel} maps to unbound command row {owner} ({source_kind})")
                    return 1
                source_owner_counts[source_kind] += 1
        if vdp1_top_pixels != 65991 or cram_pixels != 491:
            print("FAIL: VDP1 visible/CRAM pixel counts differ from the authenticated frame")
            return 1
        if cram_owner_counts != Counter({79: 60, 80: 431}):
            print(f"FAIL: CRAM VDP1 pixels do not map to the verified polyline/sprite rows: {dict(cram_owner_counts)}")
            return 1
        expected_source_owner_counts = Counter({
            "retail-dgn-indexed": 65500,
            "captured-mode4-vram": 431,
            "captured-polyline": 60,
        })
        if source_owner_counts != expected_source_owner_counts:
            print(f"FAIL: visible VDP1 pixel source classes differ from the authenticated frame: {dict(source_owner_counts)}")
            return 1
        print(f"Pixel ownership: {vdp1_top_pixels} visible VDP1 pixels map to source-verified command rows; the 491 CRAM pixels map to row 79={cram_owner_counts[79]} and row 80={cram_owner_counts[80]}")
        print(f"VDP1 pixel source counts: {dict(source_owner_counts)}")

        join = subprocess.run(
            [sys.executable, str(ROOT / "tests" /
             "test_nexus_v1_vdp1_gameplay_frame300_source_join_real.py")],
            check=False,
            env=env,
            capture_output=True,
            text=True,
        )
        sys.stdout.write(join.stdout)
        sys.stderr.write(join.stderr)
        if join.returncode != 0:
            return join.returncode
        print("PASS: the authentic frame-300 capture passed VDP1 command/source ownership, VDP2 pixel composition, and retail DGN source-join checks")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
