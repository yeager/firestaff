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
RHIFIX_BIN_SHA256 = "472cfeb21b9023dde9d4857c6faad90b4794ee2a3195b2e100f4aedd007235e3"
WRITER_PC = 0x060230E8
WRITER_CALLER = 0x06029188
BLITTER_TARGET = 0x060230D2
OPAQUE_WRITER_PC = 0x06023128
OPAQUE_WRITER_CALLER = 0x06023108
OPAQUE_DISPATCH_TARGET = 0x06023144
MENU_SURFACE = "MENU.BPK[55]"
SOURCE_OFFSET = 0x4A
SOURCE_BYTES = 86
PANEL_SURFACE_INDEX = 60
PANEL_X = 16
PANEL_Y = 112
PANEL_WIDTH = 228
PANEL_HEIGHT = 100
WRITER_FRAME = 536
NBG0_OFFSET = 0xE6F
SH2_BASE = 0x06010000
SH2_RAM_BASE = 0x06000000
VDP2_VRAM_BASE = 0x05E00000
RESOURCE_GROUP_POINTER = 0x060798C4
ANIMATION_PALT0_POINTER = 0x06079B08
ANIMATION_PALT1_POINTER = 0x06080C7C
RESOURCE_ARCHIVE_SIZE = 0x1548
RESOURCE_ARCHIVE_SHA256 = "0ad2b7abd34674015d7f32ee36dbd5be155b1654e14273ca00fe0e0161928a4d"
ANIMATION_PALT0_SHA256 = "1c81908014b51fc25f52a6400048b4449f723a2a807b83842231b09d676bbf5a"
RHIFIX_PALT0_OFFSET = 0x1140


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


def verify_runtime_animation_palt(wram_hi: bytes, rhifix: bytes
                                  ) -> tuple[int, int, str]:
    """Join the animation table to its exact source bytes in RHIFIX.BIN."""
    def read_wram_u32(address: int) -> int:
        offset = address - SH2_RAM_BASE
        if offset < 0 or offset + 4 > len(wram_hi):
            raise ValueError(f"WRAM pointer {address:#x} is outside the captured image")
        return int.from_bytes(wram_hi[offset:offset + 4], "big")

    archive_address = read_wram_u32(RESOURCE_GROUP_POINTER)
    archive_offset = archive_address - SH2_RAM_BASE
    archive_end = archive_offset + RESOURCE_ARCHIVE_SIZE
    if archive_offset < 0 or archive_end > len(wram_hi):
        raise ValueError("runtime RES* archive is outside captured WorkRAM-HI")
    archive = wram_hi[archive_offset:archive_end]
    if (archive[:4] != b"RES*" or
            int.from_bytes(archive[4:8], "big") != RESOURCE_ARCHIVE_SIZE or
            int.from_bytes(archive[8:12], "big") != 0x00050001 or
            hashlib.sha256(archive).hexdigest() != RESOURCE_ARCHIVE_SHA256):
        raise ValueError("runtime RES* archive header or bytes differ")
    if (len(rhifix) != RESOURCE_ARCHIVE_SIZE or
            hashlib.sha256(rhifix).hexdigest() != RHIFIX_BIN_SHA256):
        raise ValueError("RHIFIX.BIN is not the authenticated Japanese retail resource")
    archive_differences = [index for index, (actual, source) in
                           enumerate(zip(archive, rhifix)) if actual != source]
    if (archive_differences != [0x0B] or rhifix[0x0B] != 0 or
            archive[0x0B] != 1):
        raise ValueError("runtime RES* bytes do not match RHIFIX.BIN's single version-byte update")

    entries: dict[tuple[bytes, int], int] = {}
    for index in range(5):
        entry_offset = 12 + index * 12
        tag = archive[entry_offset:entry_offset + 4]
        version = int.from_bytes(archive[entry_offset + 4:entry_offset + 8], "big")
        data_offset = int.from_bytes(archive[entry_offset + 8:entry_offset + 12], "big")
        entries[tag, version] = data_offset
    expected = {
        (b"DYNA", 0): 0x54,
        (b"CRET", 0): 0x20C,
        (b"SPLL", 0): 0xE74,
        (b"PALT", 0): 0x1138,
        (b"PALT", 1): 0x1340,
    }
    if entries != expected:
        raise ValueError("runtime RES* resource records differ from the captured menu")

    palt0_address = archive_address + entries[b"PALT", 0] + 8
    palt1_address = archive_address + entries[b"PALT", 1] + 8
    if (read_wram_u32(ANIMATION_PALT0_POINTER) != palt0_address or
            read_wram_u32(ANIMATION_PALT1_POINTER) != palt1_address):
        raise ValueError("animation PALT pointer slots do not resolve to runtime RES* records")
    palt0_offset = palt0_address - SH2_RAM_BASE
    palt0 = wram_hi[palt0_offset:palt0_offset + 512]
    palt0_words = [palt0[index:index + 2] for index in range(0, len(palt0), 2)]
    if (len(palt0) != 512 or
            hashlib.sha256(palt0).hexdigest() != ANIMATION_PALT0_SHA256 or
            b"\xff\xff" not in palt0_words):
        raise ValueError("runtime PALT version 0 payload differs")
    if rhifix[RHIFIX_PALT0_OFFSET:RHIFIX_PALT0_OFFSET + len(palt0)] != palt0:
        raise ValueError("runtime PALT version 0 is not byte-identical to RHIFIX.BIN")
    return archive_address, palt0_address, hashlib.sha256(palt0).hexdigest()


def menu_surface_dimensions(data: bytes) -> dict[str, tuple[int, int]]:
    count = int.from_bytes(data[20:24], "big")
    dimensions: dict[str, tuple[int, int]] = {}
    for index in range(count):
        offset = int.from_bytes(data[24 + index * 4:28 + index * 4], "big")
        if data[offset + 20:offset + 24] != b"PRS3":
            continue
        width = int.from_bytes(data[offset + 12:offset + 14], "big")
        height = data[offset + 15]
        dimensions[f"MENU.BPK[{index}]"] = (width, height)
    return dimensions


def find_surface_crops(surfaces: list[tuple[str, bytes]],
                       dimensions: dict[str, tuple[int, int]],
                       target: bytes, width: int, height: int
                       ) -> list[tuple[str, int, int]]:
    """Find exact row-wise occurrences of one raster in decoded BPK surfaces."""
    first_row = target[:width]
    matches: list[tuple[str, int, int]] = []
    for name, pixels in surfaces:
        surface_width, surface_height = dimensions[name]
        if surface_width < width or surface_height < height:
            continue
        start = 0
        while True:
            offset = pixels.find(first_row, start)
            if offset < 0:
                break
            x, y = offset % surface_width, offset // surface_width
            if (x + width <= surface_width and y + height <= surface_height and
                    all(pixels[(y + row) * surface_width + x:
                               (y + row) * surface_width + x + width] ==
                        target[row * width:(row + 1) * width]
                        for row in range(height))):
                matches.append((name, x, y))
            start = offset + 1
    return matches


def verify_nbg0_screenshot(capture: Path, regions: dict[str, bytes],
                           screenshot: Path, frame: int, palt: bytes
                           ) -> tuple[int, int, set[int], int, int, str]:
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
    if len(palt) != 512:
        raise ValueError("MENU.BPK PALT must contain 256 words")
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
    source_palette_indices = {
        index for index in seen_indices
        if palt[index * 2:index * 2 + 2] == cram[index * 2:index * 2 + 2]
    }
    unmatched_palette_indices = seen_indices - source_palette_indices
    if unmatched_palette_indices != {0x83}:
        raise ValueError(
            "visible MENU.BPK PALT/CRAM differences changed: "
            f"{sorted(unmatched_palette_indices)}")
    if palt[0x83 * 2:0x83 * 2 + 2] != b"\0\0" or \
            cram[0x83 * 2:0x83 * 2 + 2] != b"\xff\xff":
        raise ValueError("index 0x83 is not the observed animated CRAM override")

    sh2_receipt = read_receipt(
        capture / "sh2-vdp2-write-trace-bundle.txt",
        "ymir-sh2-vdp2-write-trace-bundle-v1",
    )
    if (sh2_receipt.get("trace_file_complete") != "1" or
            sh2_receipt.get("master_dropped") != "0" or
            sh2_receipt.get("slave_dropped") != "0"):
        raise ValueError("CRAM animation SH-2 trace is incomplete or dropped")
    animation_writer_found = False
    trace_path = capture / sh2_receipt.get(
        "trace_file", "sh2-vdp2-write-trace.csv")
    with trace_path.open(encoding="ascii", newline="") as stream:
        for row in csv.DictReader(stream):
            if (row.get("sh2") == "master" and
                    row.get("vdp1_frame") == "655" and
                    row.get("pc") == "0x6038eda" and
                    row.get("caller_target") == "0x6038e30" and
                    row.get("address") == "0x5f00106" and
                    row.get("width") == "2" and row.get("value") == "0xffff"):
                animation_writer_found = True
                break
    if not animation_writer_found:
        raise ValueError("retail animation writer for CRAM index 0x83 is unbound")

    vdp2_animation_write_found = False
    with (capture / "vdp2-memory-write-trace.csv").open(
            encoding="ascii", newline="") as stream:
        for row in csv.DictReader(stream):
            if (row.get("memory") == "CRAM" and
                    row.get("vdp1_frame") == "655" and
                    row.get("address") == "0x106" and
                    row.get("width") == "2" and row.get("value") == "0xffff"):
                vdp2_animation_write_found = True
                break
    if not vdp2_animation_write_found:
        raise ValueError("CRAM bus trace misses the index 0x83 animation write")

    palt_cram_mismatches = {
        index for index in range(256)
        if palt[index * 2:index * 2 + 2] != cram[index * 2:index * 2 + 2]
    }
    if palt_cram_mismatches != {0x0C, 0x0D, 0x80, 0x81, 0x82, 0x83}:
        raise ValueError(
            "MENU.BPK PALT/CRAM difference set changed: "
            f"{sorted(palt_cram_mismatches)}")
    palt_cram_exact_entries = 256 - len(palt_cram_mismatches)
    screenshot_hash = hashlib.sha256(screenshot.read_bytes()).hexdigest()
    return (source_pixels, compared_pixels, seen_indices,
            len(source_palette_indices), palt_cram_exact_entries, screenshot_hash)


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

        assets = read_assets(args.data_dir, args.cue,
                             {"DM.BIN", "MENU.BPK", "RHIFIX.BIN"})
        dm_bin = assets["DM.BIN"]
        menu_bpk = assets["MENU.BPK"]
        rhifix = assets["RHIFIX.BIN"]
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

        menu_count = int.from_bytes(menu_bpk[20:24], "big")
        if PANEL_SURFACE_INDEX >= menu_count:
            raise ValueError("authentic MENU.BPK is missing panel surface 60")
        panel_offset = int.from_bytes(
            menu_bpk[24 + PANEL_SURFACE_INDEX * 4:
                     28 + PANEL_SURFACE_INDEX * 4], "big")
        panel_width = int.from_bytes(menu_bpk[panel_offset + 12:
                                               panel_offset + 14], "big")
        panel_height = menu_bpk[panel_offset + 15]
        panel = dict(surfaces).get(f"MENU.BPK[{PANEL_SURFACE_INDEX}]")
        if ((panel_width, panel_height) != (PANEL_WIDTH, PANEL_HEIGHT) or
                panel is None or len(panel) != PANEL_WIDTH * PANEL_HEIGHT):
            raise ValueError("authentic MENU.BPK[60] dimensions or pixels differ")
        panel_matches = find_surface_crops(
            surfaces, menu_surface_dimensions(menu_bpk), panel,
            PANEL_WIDTH, PANEL_HEIGHT)
        if panel_matches != [(f"MENU.BPK[{PANEL_SURFACE_INDEX}]", 0, 0)]:
            raise ValueError(
                f"NBG0 panel rectangle is not unique in MENU.BPK: {panel_matches}")
        for y in range(PANEL_HEIGHT):
            source_row = panel[y * PANEL_WIDTH:(y + 1) * PANEL_WIDTH]
            dest_start = (PANEL_Y + y) * 512 + PANEL_X
            if bitmap[dest_start:dest_start + PANEL_WIDTH] != source_row:
                raise ValueError(
                    f"MENU.BPK[60] row {y} does not match the captured NBG0 bitmap")

        # The raw Ymir WorkRAM-HI image is addressed from 0x06000000. Check the
        # observed store opcode against the authenticated DM.BIN code mapping.
        wram_hi = (args.capture / "wram-hi.bin").read_bytes()
        runtime_archive, runtime_palt0, runtime_palt0_hash = \
            verify_runtime_animation_palt(wram_hi, rhifix)
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
        copy_dispatch_offset = 0x060292CC - SH2_BASE
        if (int.from_bytes(dm_bin[copy_dispatch_offset:copy_dispatch_offset + 4],
                           "big") != OPAQUE_DISPATCH_TARGET):
            raise ValueError("DM.BIN opaque menu-copy dispatch pointer differs")
        opaque_code_offset = OPAQUE_WRITER_PC - SH2_BASE
        opaque_ram_offset = OPAQUE_WRITER_PC - SH2_RAM_BASE
        opaque_code = dm_bin[opaque_code_offset:opaque_code_offset + 32]
        if (len(opaque_code) != 32 or
                wram_hi[opaque_ram_offset:opaque_ram_offset + 32] != opaque_code):
            raise ValueError("frame opaque-copy writer is not bound to retail DM.BIN")

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
        matched_panel_writer: set[int] = set()
        eligible_events = 0
        eligible_panel_events = 0
        trace_path = args.capture / sh2_receipt.get("trace_file", "sh2-vdp2-write-trace.csv")
        with trace_path.open(encoding="ascii", newline="") as stream:
            rows = csv.DictReader(stream)
            for row in rows:
                if row.get("sh2") != "master":
                    continue
                if int(row["vdp1_frame"]) != WRITER_FRAME:
                    continue
                pc = int(row["pc"], 16)
                address = int(row["address"], 16)
                if pc == OPAQUE_WRITER_PC:
                    stack = {int(value, 16) for value in
                             row["caller_stack_targets"].split("|")}
                    if (int(row["caller_target"], 16) == OPAQUE_WRITER_CALLER and
                            WRITER_CALLER in stack):
                        row_data = be_value_bytes(int(row["value"], 16),
                                                  int(row["width"]))
                        for index, value in enumerate(row_data):
                            destination = address + index - VDP2_VRAM_BASE
                            y_offset, x_offset = divmod(destination, 512)
                            panel_y = y_offset - PANEL_Y
                            panel_x = x_offset - PANEL_X
                            if (0 <= panel_y < PANEL_HEIGHT and
                                    0 <= panel_x < PANEL_WIDTH):
                                eligible_panel_events += 1
                                source_offset = panel_y * PANEL_WIDTH + panel_x
                                if value != panel[source_offset]:
                                    raise ValueError(
                                        "retail opaque-copy trace differs from MENU.BPK[60]")
                                matched_panel_writer.add(source_offset)
                if pc != WRITER_PC:
                    continue
                if int(row["caller_target"], 16) != BLITTER_TARGET:
                    continue
                stack = {int(value, 16) for value in row["caller_stack_targets"].split("|")}
                if WRITER_CALLER not in stack:
                    continue
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
        if (eligible_panel_events != PANEL_WIDTH * PANEL_HEIGHT or
                matched_panel_writer != set(range(PANEL_WIDTH * PANEL_HEIGHT))):
            raise ValueError(
                "retail opaque-copy trace did not bind every MENU.BPK[60] pixel: "
                f"{len(matched_panel_writer)}/{PANEL_WIDTH * PANEL_HEIGHT}")
        panel_bus_values: dict[int, int] = {}
        with (args.capture / "vdp2-memory-write-trace.csv").open(
                encoding="ascii", newline="") as stream:
            for row in csv.DictReader(stream):
                if row.get("memory") != "VRAM" or \
                        int(row["vdp1_frame"]) != WRITER_FRAME:
                    continue
                address = int(row["address"], 16)
                data = be_value_bytes(int(row["value"], 16), int(row["width"]))
                for index, value in enumerate(data):
                    destination = address + index
                    y, x = divmod(destination, 512)
                    if (PANEL_Y <= y < PANEL_Y + PANEL_HEIGHT and
                            PANEL_X <= x < PANEL_X + PANEL_WIDTH):
                        panel_bus_values[(y - PANEL_Y) * PANEL_WIDTH +
                                         (x - PANEL_X)] = value
        if (len(panel_bus_values) != PANEL_WIDTH * PANEL_HEIGHT or
                any(panel_bus_values[index] != panel[index]
                    for index in range(PANEL_WIDTH * PANEL_HEIGHT))):
            raise ValueError(
                "frame-bounded VDP2 bus writes do not preserve all MENU.BPK[60] pixels")
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
                args.capture, regions, args.screenshot, capture_frame,
                _palette)
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
    print(f"opaque_menu_surface=MENU.BPK[{PANEL_SURFACE_INDEX}]")
    print(f"opaque_menu_surface_dimensions={PANEL_WIDTH}x{PANEL_HEIGHT}")
    print(f"opaque_menu_surface_destination={PANEL_X},{PANEL_Y}")
    print(f"opaque_menu_surface_pixels={len(matched_panel_writer)}")
    print(f"opaque_menu_surface_writer_events={eligible_panel_events}")
    print(f"opaque_menu_surface_vdp2_bus_pixels={len(panel_bus_values)}")
    print("opaque_menu_surface_unique_bpk_crop=verified")
    print("opaque_menu_surface_source_to_nbg0=verified")
    print("runtime_resource_archive=RES*")
    print(f"runtime_resource_archive_address=0x{runtime_archive:08x}")
    print("runtime_resource_archive_source=RHIFIX.BIN")
    print("runtime_archive_to_source=exact_except_byte_0x0b_0_to_1")
    print(f"runtime_animation_palt0_source=RHIFIX.BIN@0x{RHIFIX_PALT0_OFFSET:x}")
    print("runtime_animation_palt0_source_bytes=exact")
    print(f"runtime_animation_palt0_address=0x{runtime_palt0:08x}")
    print(f"runtime_animation_palt0_sha256={runtime_palt0_hash}")
    print("runtime_animation_palt0_to_res_record=verified")
    print("animation_cram_value_membership=0xffff-present-in-runtime-palt0")
    if palette_receipt:
        (source_pixels, compared_pixels, indices,
         visible_palt_matches, total_palt_matches,
         screenshot_hash) = palette_receipt
        print(f"visible_nbg0_palette_indices={len(indices)}")
        print(f"visible_nbg0_source_pixels={source_pixels}")
        print(f"screenshot_pixels_matched={compared_pixels}")
        print(f"visible_menu_palt_exact_indices={visible_palt_matches}/{len(indices)}")
        palt_unmatched = sorted(indices - {
            index for index in indices
            if _palette[index * 2:index * 2 + 2] ==
            regions["vdp2-cram"][index * 2:index * 2 + 2]
        })
        print("visible_menu_palt_unmatched_indices=" +
              ",".join(f"0x{index:02x}" for index in palt_unmatched))
        print(f"menu_palt_cram_exact_entries={total_palt_matches}/256")
        print("menu_palt_cram_mismatch_indices=" +
              ",".join(f"0x{index:02x}" for index in sorted(
                  {0x0C, 0x0D, 0x80, 0x81, 0x82, 0x83})))
        print("animated_palette_override_index=0x83")
        print("animated_palette_override_writer=0x06038eda@frame655")
        print("animated_palette_override_source_index=not-captured")
        print(f"screenshot_sha256={screenshot_hash}")
        print("visible_nbg0_palette_to_screenshot=verified")
    else:
        print("visible_nbg0_palette_to_screenshot=unverified")
    print("full_screen_composition=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
