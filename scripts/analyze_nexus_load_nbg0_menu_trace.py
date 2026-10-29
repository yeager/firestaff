#!/usr/bin/env python3
"""Join one authentic MENU.BPK surface to the Load-panel NBG0 writer trace.

This is a narrow source-to-VRAM and palette-to-screenshot receipt. It does
not prove the full screen composition, so it never admits rendering.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path

from analyze_nexus_vdp2_bitmap_source import (
    menu_surfaces,
    read_assets,
    ymir_bundle_frame,
)
from nexus_vdp2_registers import read_u16


DM_BIN_SHA256 = "3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124"
WRITER_PC = 0x060230E8
WRITER_CALLER = 0x06029188
BLITTER_TARGET = 0x060230D2
MENU_SURFACE = "MENU.BPK[55]"
SOURCE_OFFSET = 0x4A
SOURCE_BYTES = 86
WRITER_FRAME = 536
NBG0_OFFSET = 0xE6F
SH2_BASE = 0x06010000
SH2_RAM_BASE = 0x06000000
VDP2_VRAM_BASE = 0x05E00000


def read_receipt(path: Path, expected_format: str,
                 require_complete: bool = True) -> dict[str, str]:
    fields: dict[str, str] = {}
    for line in path.read_text(encoding="ascii").splitlines():
        if not line or "=" not in line:
            continue
        key, value = line.split("=", 1)
        if key in fields:
            raise ValueError(f"duplicate {key} in {path.name}")
        fields[key] = value
    if (fields.get("format") != expected_format or
            (require_complete and fields.get("bundle_complete") != "1")):
        raise ValueError(f"incomplete or unsupported {path.name}")
    return fields


def be_value_bytes(value: int, width: int) -> bytes:
    if width not in (1, 2, 4) or value < 0 or value >= 1 << (8 * width):
        raise ValueError("invalid SH-2 VDP2 trace value")
    return bytes((value,)) if width == 1 else value.to_bytes(width, "big")


def verify_nbg0_screenshot(capture: Path, regions: dict[str, bytes],
                           screenshot: Path, frame: int
                           ) -> tuple[int, int, set[int], str]:
    """Match every visible nonzero NBG0 pixel to its retail CRAM output."""
    from PIL import Image

    registers = regions["vdp2-regs"]
    reg = lambda offset: read_u16(registers, offset, "big")
    snapshot = read_receipt(capture / "vdp-register-snapshot.txt",
                            "ymir-vdp-register-snapshot-v1", False)
    vdp1 = read_receipt(capture / "vdp1-capture-bundle.txt",
                        "ymir-vdp1-capture-bundle-v1", False)
    if (int(snapshot.get("vdp1_frame_index", "-1")) != frame or
            int(vdp1.get("capture_frame_index", "-1")) != frame or
            vdp1.get("trace_file_complete") != "1" or
            vdp1.get("trace_dropped_total") != "0" or
            int(vdp1.get("trace_rows_for_capture_frame", "0")) < 1 or
            int(snapshot.get("vdp2_register_bytes", "0")) != 512):
        raise ValueError("screenshot evidence does not match the captured VDP frame")
    if (reg(0x00) != 0x8000 or not (reg(0x20) & 1) or
            reg(0x28) != 0x0013 or reg(0x2C) != 0 or reg(0xE6) != 0 or
            (reg(0xEC) & 1) == 0 or reg(0x108) != 0x1000):
        raise ValueError("frame does not enable NBG0 8bpp bitmap CRAM rendering")
    ratio = 31 - (reg(0x108) & 31)

    image = Image.open(screenshot).convert("RGB")
    if image.size != (640, 448):
        raise ValueError("expected the retained 640x448 2x Ymir screenshot")
    vram = regions["vdp2-vram"]
    cram = regions["vdp2-cram"]
    seen_indices: set[int] = set()
    source_pixels = 0
    compared_pixels = 0
    for y in range(224):
        for x in range(320):
            color_index = vram[y * 512 + x]
            if color_index == 0:
                continue
            source_pixels += 1
            seen_indices.add(color_index)
            word = read_u16(cram, color_index * 2, "big")
            expected_rgb = tuple(
                ((((word >> shift) & 31) << 3) * ratio) // 32
                for shift in (0, 5, 10)
            )
            for dy in (0, 1):
                for dx in (0, 1):
                    if image.getpixel((x * 2 + dx, y * 2 + dy)) != expected_rgb:
                        raise ValueError(
                            f"NBG0 palette/screenshot mismatch at ({x}, {y}), "
                            f"index {color_index:#04x}")
                    compared_pixels += 1
    if source_pixels == 0:
        raise ValueError("visible NBG0 bitmap contains no nonzero pixels")
    screenshot_hash = hashlib.sha256(screenshot.read_bytes()).hexdigest()
    return source_pixels, compared_pixels, seen_indices, screenshot_hash


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path,
                        help="complete Ymir VDP2 memory-bundle directory")
    source_group = parser.add_mutually_exclusive_group(required=True)
    source_group.add_argument("--data-dir", type=Path,
                              help="authenticated extracted Japanese Nexus data")
    source_group.add_argument("--cue", type=Path,
                              help="authenticated Japanese retail CUE")
    parser.add_argument("--screenshot", type=Path,
                        help="same-state 640x448 Ymir screenshot for NBG0 palette verification")
    args = parser.parse_args()

    try:
        regions, capture_frame, byte_order = ymir_bundle_frame(args.capture)
        if capture_frame != 659:
            raise ValueError("capture is not the retained Load-panel frame 659")
        registers = regions["vdp2-regs"]
        bgon = read_u16(registers, 0x20, byte_order)
        chctla = read_u16(registers, 0x28, byte_order)
        if not (bgon & 1) or (chctla & 0x0002) == 0 or \
                (chctla & 0x000C) != 0 or (chctla & 0x0070) != 0x0010:
            raise ValueError("frame 659 does not have the captured NBG0 8bpp bitmap mode")

        assets = read_assets(args.data_dir, args.cue, {"DM.BIN", "MENU.BPK"})
        dm_bin = assets["DM.BIN"]
        menu_bpk = assets["MENU.BPK"]
        if hashlib.sha256(dm_bin).hexdigest() != DM_BIN_SHA256:
            raise ValueError("DM.BIN is not the authenticated Japanese retail executable")
        surfaces, _palette = menu_surfaces(menu_bpk)
        surface = dict(surfaces).get(MENU_SURFACE)
        if surface is None or SOURCE_OFFSET + SOURCE_BYTES > len(surface):
            raise ValueError("authenticated MENU.BPK surface span is unavailable")
        span = surface[SOURCE_OFFSET:SOURCE_OFFSET + SOURCE_BYTES]
        bitmap = regions["vdp2-vram"][:0x20000]
        if bitmap[NBG0_OFFSET:NBG0_OFFSET + SOURCE_BYTES] != span:
            raise ValueError("MENU.BPK source bytes do not match the captured NBG0 span")
        if bitmap.find(span) != NBG0_OFFSET or bitmap.count(span) != 1:
            raise ValueError("MENU.BPK span is not unique at the expected NBG0 offset")

        # The raw Ymir WorkRAM-HI image is addressed from 0x06000000. Check the
        # observed store opcode against the authenticated DM.BIN code mapping.
        wram_hi = (args.capture / "wram-hi.bin").read_bytes()
        code_offset = WRITER_PC - SH2_BASE
        ram_offset = WRITER_PC - SH2_RAM_BASE
        code = dm_bin[code_offset:code_offset + 32]
        if (code_offset < 0 or ram_offset + len(code) > len(wram_hi) or
                len(code) != 32 or code[:2] != bytes.fromhex("2410") or
                wram_hi[ram_offset:ram_offset + len(code)] != code):
            raise ValueError("frame writer opcode is not bound to the retail DM.BIN image")
        dispatch_offset = 0x060292C8 - SH2_BASE
        if int.from_bytes(dm_bin[dispatch_offset:dispatch_offset + 4], "big") != BLITTER_TARGET:
            raise ValueError("DM.BIN menu blitter dispatch pointer differs")

        sh2_receipt = read_receipt(
            args.capture / "sh2-vdp2-write-trace-bundle.txt",
            "ymir-sh2-vdp2-write-trace-bundle-v1",
        )
        if (sh2_receipt.get("trace_file_complete") != "1" or
                sh2_receipt.get("master_dropped") != "0" or
                sh2_receipt.get("slave_dropped") != "0"):
            raise ValueError("SH-2 VDP2 trace is incomplete or dropped events")
        expected_rows = int(sh2_receipt["trace_rows"])
        matched_sh2: set[int] = set()
        eligible_events = 0
        trace_path = args.capture / sh2_receipt.get("trace_file", "sh2-vdp2-write-trace.csv")
        with trace_path.open(encoding="ascii", newline="") as stream:
            rows = csv.DictReader(stream)
            for row in rows:
                if row.get("sh2") != "master":
                    continue
                if int(row["vdp1_frame"]) != WRITER_FRAME or \
                        int(row["pc"], 16) != WRITER_PC:
                    continue
                if int(row["caller_target"], 16) != BLITTER_TARGET:
                    continue
                stack = {int(value, 16) for value in row["caller_stack_targets"].split("|")}
                if WRITER_CALLER not in stack:
                    continue
                address = int(row["address"], 16)
                if not (VDP2_VRAM_BASE + NBG0_OFFSET <= address <
                        VDP2_VRAM_BASE + NBG0_OFFSET + SOURCE_BYTES):
                    continue
                data = be_value_bytes(int(row["value"], 16), int(row["width"]))
                eligible_events += 1
                for index, value in enumerate(data):
                    pixel_offset = address + index - VDP2_VRAM_BASE - NBG0_OFFSET
                    if (0 <= pixel_offset < SOURCE_BYTES and
                            span[pixel_offset] != 0 and span[pixel_offset] == value):
                        matched_sh2.add(pixel_offset)
            actual_rows = rows.line_num - 1
        if actual_rows != expected_rows:
            raise ValueError("SH-2 trace row count differs from its receipt")
        expected_nonzero = {i for i, value in enumerate(span) if value != 0}
        if not expected_nonzero or matched_sh2 != expected_nonzero:
            raise ValueError(
                f"SH-2 writer did not bind every nonzero source pixel: "
                f"{len(matched_sh2)}/{len(expected_nonzero)}")

        matched_vdp2: set[int] = set()
        vdp2_events = 0
        with (args.capture / "vdp2-memory-write-trace.csv").open(
                encoding="ascii", newline="") as stream:
            for row in csv.DictReader(stream):
                if row.get("memory") != "VRAM" or \
                        int(row["vdp1_frame"]) != WRITER_FRAME:
                    continue
                address = int(row["address"], 16)
                data = be_value_bytes(int(row["value"], 16), int(row["width"]))
                if not (NBG0_OFFSET <= address < NBG0_OFFSET + SOURCE_BYTES):
                    continue
                vdp2_events += 1
                for index, value in enumerate(data):
                    pixel_offset = address + index - NBG0_OFFSET
                    if (0 <= pixel_offset < SOURCE_BYTES and
                            span[pixel_offset] != 0 and span[pixel_offset] == value):
                        matched_vdp2.add(pixel_offset)
        if matched_vdp2 != expected_nonzero:
            raise ValueError(
                f"VDP2 bus trace did not bind every nonzero source pixel: "
                f"{len(matched_vdp2)}/{len(expected_nonzero)}")
        palette_receipt = None
        if args.screenshot:
            palette_receipt = verify_nbg0_screenshot(
                args.capture, regions, args.screenshot, capture_frame)
    except (KeyError, OSError, TypeError, ValueError, UnicodeError) as error:
        print(f"NEXUS_LOAD_NBG0_MENU_TRACE_INVALID: {error}")
        return 1

    print(f"capture_frame={capture_frame}")
    print(f"writer_frame={WRITER_FRAME}")
    print("nbg0_mode=active_512x256_8bpp_bitmap")
    print(f"dm_bin_sha256={DM_BIN_SHA256}")
    print(f"menu_bpk_sha256={hashlib.sha256(menu_bpk).hexdigest()}")
    print(f"menu_surface={MENU_SURFACE}")
    print(f"source_offset=0x{SOURCE_OFFSET:x}")
    print(f"nbg0_offset=0x{NBG0_OFFSET:x}")
    print(f"matched_bytes={SOURCE_BYTES}")
    print(f"nonzero_source_pixels={len(expected_nonzero)}")
    print(f"distinct_source_values={len(set(span))}")
    print(f"writer_pc=0x{WRITER_PC:08x}")
    print(f"writer_dispatch_target=0x{BLITTER_TARGET:08x}")
    print(f"writer_caller=0x{WRITER_CALLER:08x}")
    print(f"eligible_writer_events={eligible_events}")
    print(f"matched_nonzero_sh2_writer_pixels={len(matched_sh2)}/{len(expected_nonzero)}")
    print(f"vdp2_bus_events_in_span={vdp2_events}")
    print(f"matched_nonzero_vdp2_bus_pixels={len(matched_vdp2)}/{len(expected_nonzero)}")
    print("source_to_nbg0_transport=verified")
    if palette_receipt:
        source_pixels, compared_pixels, indices, screenshot_hash = palette_receipt
        print(f"visible_nbg0_palette_indices={len(indices)}")
        print(f"visible_nbg0_source_pixels={source_pixels}")
        print(f"screenshot_pixels_matched={compared_pixels}")
        print(f"screenshot_sha256={screenshot_hash}")
        print("visible_nbg0_palette_to_screenshot=verified")
    else:
        print("visible_nbg0_palette_to_screenshot=unverified")
    print("full_screen_composition=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
