#!/usr/bin/env python3
"""Verify an authentic Ymir YMLYR001 layer trace against raw Saturn VDP2 data."""
from collections import Counter
from pathlib import Path
import argparse
import csv
import re
import struct
import sys


FRAME = 7486
SERIAL = 8199
WIDTH = 320
VRAM_BITMAP_BYTES = 512 * 256
FRAMEBUFFER_NAME = "vdp1-7486-vdp2-8199-320x224.rgbx8888le"


def read_records(path: Path):
    data = path.read_bytes()
    if data[:8] != b"YMLYR001":
        raise ValueError(f"unexpected layer trace magic: {data[:8]!r}")
    records = {}
    offset = 8
    while offset < len(data):
        frame, serial, ordinal, row, field, layer, width = struct.unpack_from(
            "<QQIIBBH", data, offset
        )
        record_bytes = 28 + 6 * width
        if width != WIDTH or offset + record_bytes > len(data):
            raise ValueError(f"invalid/truncated record at byte {offset}")
        color_offset = offset + 28
        colors = struct.unpack_from(f"<{width}I", data, color_offset)
        priority_offset = color_offset + 4 * width
        priorities = data[priority_offset:priority_offset + width]
        special = data[priority_offset + width:priority_offset + 2 * width]
        key = (frame, serial, row, layer)
        if key in records:
            raise ValueError(f"duplicate layer record {key}")
        records[key] = (colors, priorities, special, ordinal, field)
        offset += record_bytes
    if offset != len(data):
        raise ValueError("layer trace ended between records")
    return records, len(data)


def parse_receipt(path: Path):
    values = {}
    for line in path.read_text().splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            values[key] = value
    return values


def exact_ymer_byte(color_word: int) -> int:
    """Expand RGB555 the way Ymir's ConvertRGB555to888 does."""
    red = (color_word & 0x1F) << 3
    green = ((color_word >> 5) & 0x1F) << 3
    blue = ((color_word >> 10) & 0x1F) << 3
    msb = (color_word >> 15) & 1
    return red | (green << 8) | (blue << 16) | (msb << 31)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path, help="directory containing the retained Ymir capture")
    args = parser.parse_args()
    root = args.bundle
    dumps = root / "dumps"

    layer_receipt = parse_receipt(dumps / "nexus-vdp2-layer-outputs.bin.receipt.txt")
    render_receipt = parse_receipt(dumps / "nexus-nbg0-vdp2-render-state.csv.receipt.txt")
    write_receipt = parse_receipt(dumps / "vdp2-memory-write-bundle.txt")
    if layer_receipt.get("capture_status") != "complete" or layer_receipt.get("write_error") != "0":
        raise ValueError("Ymir layer-output capture is incomplete")
    if layer_receipt.get("limit_exceeded") != "0":
        raise ValueError("Ymir layer-output capture exceeded its limit")
    if render_receipt.get("capture_status") != "complete" or render_receipt.get("write_error") != "0":
        raise ValueError("Ymir row-state capture is incomplete")
    if write_receipt.get("bundle_complete") != "1" or write_receipt.get("trace_dropped_total") != "0":
        raise ValueError("VDP2 memory-write capture is incomplete")
    if int(write_receipt["trace_vdp1_frame_begin"]) > FRAME or int(write_receipt["trace_vdp1_frame_end_exclusive"]) <= FRAME:
        raise ValueError("VDP2 memory-write trace does not cover target frame")
    snapshot_frame = int(write_receipt["current_vdp1_frame_index"])
    if snapshot_frame < FRAME:
        raise ValueError("VDP2 memory snapshot precedes target frame")

    rows = {}
    with (dumps / "nexus-nbg0-vdp2-render-state.csv").open(newline="") as stream:
        for item in csv.DictReader(stream):
            if int(item["frame_index"]) != FRAME or int(item["vdp2_frame_serial"]) != SERIAL:
                raise ValueError("unexpected frame/serial in row-state trace")
            row = int(item["row_vcnt"])
            if row in rows:
                raise ValueError(f"duplicate NBG0 row state {row}")
            rows[row] = item
    if not rows:
        raise ValueError("row-state trace is empty")
    expected_rows = set(range(min(rows), max(rows) + 1))
    if set(rows) != expected_rows:
        raise ValueError(f"row-state coverage has gaps: {len(rows)} rows")
    if len(rows) != int(render_receipt["expected_main_samples"]):
        raise ValueError("row-state coverage does not match its receipt")
    row_range = render_receipt["row_vcnt_range_half_open"]
    if row_range != f"{min(rows)}:{max(rows) + 1}":
        raise ValueError("row-state range does not match its receipt")

    records, layer_bytes = read_records(dumps / "nexus-vdp2-layer-outputs.bin")
    expected_keys = {(FRAME, SERIAL, row, layer) for row in expected_rows for layer in range(7)}
    if set(records) != expected_keys:
        raise ValueError(f"layer-record coverage mismatch: {len(records)} records")
    if int(layer_receipt.get("records_written", "-1")) != len(records):
        raise ValueError("layer receipt count does not match binary records")

    vram = (dumps / "vdp2-vram.bin").read_bytes()
    cram = (dumps / "vdp2-cram.bin").read_bytes()
    if len(vram) != 0x80000 or len(cram) != 0x1000:
        raise ValueError("unexpected VDP2 memory snapshot size")
    if len(vram) < VRAM_BITMAP_BYTES:
        raise ValueError("NBG0 bitmap range is absent from VDP2 VRAM")

    # The final memory snapshot is at frame 7663. Verify that the complete
    # VDP2 write trace shows no intervening writes to NBG0's 0x00000..0x1ffff
    # bitmap span or to CRAM during the traced frame interval.
    write_rows = 0
    nbg0_writes = 0
    cram_writes = 0
    with (dumps / "vdp2-memory-write-trace.csv").open(newline="") as stream:
        for item in csv.DictReader(stream):
            write_rows += 1
            frame = int(item["vdp1_frame"])
            if FRAME <= frame <= snapshot_frame:
                memory = item["memory"]
                address = int(item["address"], 0)
                width = int(item["width"])
                if memory == "CRAM":
                    cram_writes += 1
                elif memory == "VRAM" and address < VRAM_BITMAP_BYTES and address + width > 0:
                    nbg0_writes += 1
    if write_rows != int(write_receipt["trace_rows"]):
        raise ValueError("VDP2 write CSV row count does not match receipt")
    if nbg0_writes or cram_writes:
        raise ValueError(f"source memory changed after target frame: VRAM={nbg0_writes}, CRAM={cram_writes}")

    # The final snapshot reports base-zero BMPNA/CRAOFA. The write trace binds
    # the bitmap and CRAM bytes across the target-frame-to-snapshot interval.
    registers = (dumps / "vdp2-registers.bin").read_bytes()
    read_be16 = lambda offset: int.from_bytes(registers[offset:offset + 2], "big")
    if read_be16(0x28) != 0x0013 or read_be16(0x2C) != 0 or read_be16(0xE4) != 0:
        raise ValueError("final snapshot does not match the base-zero 8bpp bitmap lane")
    palette_base = 0

    raw_frame = (dumps / "rendered-frames" / FRAMEBUFFER_NAME).read_bytes()
    if len(raw_frame) != WIDTH * 224 * 4:
        raise ValueError("unexpected same-pass framebuffer size")
    output_pixels = struct.unpack(f"<{WIDTH * 224}I", raw_frame)

    source_matches = source_total = priority_matches = 0
    active = transparent = 0
    special_active = 0
    underlays = Counter()
    composed_matches = composed_total = 0
    row_modes = Counter()

    for row in sorted(expected_rows):
        state = rows[row]
        if (int(state["nbg0_enabled"]) != 1 or int(state["layer_enabled"]) != 1 or
                int(state["rbg1_enabled"]) != 0 or int(state["bitmap"]) != 1 or
                int(state["color_format"]) != 1 or int(state["priority"]) != 4 or
                int(state["color_calculation"]) != 1 or int(state["color_calculation_ratio"]) != 15):
            raise ValueError(f"unexpected NBG0 render state at row {row}")
        row_modes[(int(state["scroll_inc_h_11_8"]), int(state["scroll_inc_v_11_8"]))] += 1
        sprite, sprite_priority, _, _, _ = records[(FRAME, SERIAL, row, 0)]
        nbg0, nbg0_priority, nbg0_special, _, _ = records[(FRAME, SERIAL, row, 2)]
        for inactive_layer in (1, 3, 4, 5, 6):
            other, other_priority, _, _, _ = records[(FRAME, SERIAL, row, inactive_layer)]
            if any(other) or any(other_priority):
                raise ValueError(f"unexpected active layer {inactive_layer} at row {row}")
        if any(priority not in (0, 2) for priority in sprite_priority):
            raise ValueError(f"unexpected sprite priority at row {row}")
        if any(priority not in (0, 4) for priority in nbg0_priority):
            raise ValueError(f"unexpected NBG0 priority at row {row}")

        for x in range(WIDTH):
            source_x_fp = (int(state["scroll_h_11_8"]) +
                           int(state["fractional_scroll_x_11_8"]) +
                           x * int(state["scroll_inc_h_11_8"]))
            source_y_fp = (int(state["scroll_v_11_8"]) +
                           int(state["fractional_scroll_y_11_8"]))
            source_x = (source_x_fp >> 8) & 0x1FF
            source_y = (source_y_fp >> 8) & 0xFF
            source_index = vram[source_y * 512 + source_x]
            expected_priority = 4 if source_index else 0
            expected_color = 0
            if source_index:
                cram_index = (palette_base + source_index) & 0x3FF
                color_word = int.from_bytes(cram[cram_index * 2:cram_index * 2 + 2], "big")
                expected_color = exact_ymer_byte(color_word)
                active += 1
                special_active += int(nbg0_special[x] != 0)
            else:
                transparent += 1

            source_total += 1
            source_matches += int(nbg0[x] == expected_color)
            priority_matches += int(nbg0_priority[x] == expected_priority)
            if nbg0[x] != expected_color or nbg0_priority[x] != expected_priority:
                raise ValueError(f"NBG0 source mismatch at row {row}, x={x}")

            if nbg0_priority[x] > 0:
                if nbg0_special[x] != 0:
                    lower = sprite[x] if sprite_priority[x] > 0 else 0xFF000000
                    rgb = 0
                    for shift in (0, 8, 16):
                        top_channel = (nbg0[x] >> shift) & 0xFF
                        lower_channel = (lower >> shift) & 0xFF
                        channel = lower_channel + (((top_channel - lower_channel) * 15) >> 5)
                        rgb |= (channel & 0xFF) << shift
                    underlays["sprite" if sprite_priority[x] > 0 else "black_back"] += 1
                else:
                    rgb = nbg0[x] & 0x00FFFFFF
            elif sprite_priority[x] > 0:
                rgb = sprite[x] & 0x00FFFFFF
                underlays["sprite_only"] += 1
            else:
                rgb = 0
                underlays["black_back_only"] += 1

            actual_rgb = output_pixels[row * WIDTH + x] & 0x00FFFFFF
            composed_total += 1
            composed_matches += int(rgb == actual_rgb)
            if rgb != actual_rgb:
                raise ValueError(f"composition mismatch at row {row}, x={x}: expected {rgb:06x}, got {actual_rgb:06x}")

    expected_pixel_count = len(expected_rows) * WIDTH
    if (source_matches != expected_pixel_count or priority_matches != expected_pixel_count or
            composed_matches != expected_pixel_count):
        raise ValueError("unexpected partial pixel verification")

    complete_frame = len(expected_rows) == 224 and min(expected_rows) == 0 and max(expected_rows) == 223
    print(f"capture=frame-{FRAME}/vdp2-{SERIAL} rows={len(expected_rows)} pixels={expected_pixel_count} layer_records={len(records)} bytes={layer_bytes}")
    print(f"same_pass_nbg0_vram_cram_writes=0 snapshot_frame={snapshot_frame} traced_write_rows={write_rows}")
    print(f"nbg0_source_to_layer_color={source_matches}/{source_total} priority={priority_matches}/{source_total} active={active} transparent={transparent} special_calc={special_active}")
    print(f"two_layer_composition_rgb={composed_matches}/{composed_total} underlays={dict(underlays)}")
    print(f"claim_scope={'full captured RGB frame' if complete_frame else 'captured rows only'}; no NBG1, arbitrary-state, or retail-hardware claim")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, KeyError, ValueError, struct.error) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
