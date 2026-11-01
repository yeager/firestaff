#!/usr/bin/env python3
"""Compare an authentic Ymir VDP2 layer with its same-pass RGBX frame and optional decoded plane."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
from pathlib import Path
import struct
import sys


MAGIC = b"YMLYR001"
HEADER_SIZE = 28
BYTES_PER_PIXEL = 6
LAYER_NAMES = (
    "sprite", "rbg0", "nbg0_or_rbg1", "nbg1_or_exbg", "nbg2", "nbg3",
    "transparent_mesh",
)


class TraceError(ValueError):
    pass


def read_receipt(path: Path) -> dict[str, str]:
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except OSError as error:
        raise TraceError(f"cannot read receipt {path}: {error}") from error
    receipt: dict[str, str] = {}
    for line in lines:
        if not line:
            continue
        if "=" not in line:
            raise TraceError(f"malformed receipt line: {line!r}")
        key, value = line.split("=", 1)
        if key in receipt:
            raise TraceError(f"duplicate receipt field {key!r}")
        receipt[key] = value
    required = {
        "format": "ymir-vdp2-layer-output-trace-v1",
        "magic": MAGIC.decode("ascii"),
        "record_header_bytes": str(HEADER_SIZE),
        "record_header": "frame_index:u64le,vdp2_frame_serial:u64le,line_call_ordinal:u32le,row_vcnt:u32le,alternate_field:u8,layer_id:u8,width:u16le",
        "pixel_planes": "color_u32le[width],priority_u8[width],special_color_calc_u8[width]",
        "capture_status": "complete",
        "write_error": "0",
        "limit_exceeded": "0",
    }
    for key, expected in required.items():
        if receipt.get(key) != expected:
            raise TraceError(f"receipt {key} must be {expected!r}, got {receipt.get(key)!r}")
    return receipt


def parse_range(value: str, label: str) -> tuple[int, int]:
    try:
        begin_text, end_text = value.split(":", 1)
        begin, end = int(begin_text), int(end_text)
    except (ValueError, TypeError) as error:
        raise TraceError(f"invalid {label} range {value!r}") from error
    if begin < 0 or begin >= end:
        raise TraceError(f"invalid {label} range {value!r}")
    return begin, end


def verify_frame_index(path: Path, frame: int, serial: int, width: int, height: int,
                       reference: Path) -> None:
    expected_file = f"vdp1-{frame}-vdp2-{serial}-{width}x{height}.rgbx8888le"
    if reference.name != expected_file:
        raise TraceError(f"reference filename must identify this pass as {expected_file!r}")
    matches = 0
    try:
        with path.open(newline="", encoding="utf-8") as stream:
            for row in csv.DictReader(stream):
                if row.get("frame_index") == str(frame) and row.get("vdp2_frame_serial") == str(serial):
                    matches += 1
                    if (row.get("width") != str(width) or row.get("height") != str(height) or
                            row.get("pixel_format") != "rgbx8888le" or row.get("file") != expected_file):
                        raise TraceError("rendered-frame index metadata does not match the requested pass")
    except OSError as error:
        raise TraceError(f"cannot read rendered-frame index {path}: {error}") from error
    if matches != 1:
        raise TraceError(f"expected one rendered-frame index row for frame/serial, found {matches}")


def analyze(args: argparse.Namespace) -> dict[str, object]:
    receipt = read_receipt(args.receipt)
    try:
        trace = args.trace.read_bytes()
        reference = args.reference_rgbx.read_bytes()
        decoded_plane = args.decoded_plane.read_bytes() if args.decoded_plane else None
    except OSError as error:
        raise TraceError(f"cannot read input: {error}") from error
    if len(trace) < len(MAGIC) or trace[: len(MAGIC)] != MAGIC:
        raise TraceError(f"{args.trace}: missing YMLYR001 header")
    if args.frame < 0 or args.serial < 0 or not 0 <= args.layer < len(LAYER_NAMES):
        raise TraceError("frame and serial must be nonnegative; layer must be in 0..6")
    if args.width <= 0 or args.height <= 0:
        raise TraceError("width and height must be positive")

    frame_begin, frame_end = parse_range(receipt.get("frame_range_half_open", ""), "frame")
    row_begin, row_end = parse_range(receipt.get("row_vcnt_range_half_open", ""), "row")
    if not frame_begin <= args.frame < frame_end:
        raise TraceError("requested frame is outside the receipt's captured frame range")
    if row_begin != 0 or row_end != args.height:
        raise TraceError("full-frame comparison requires the receipt row range to match 0:height")
    if str(args.serial) not in receipt.get("observed_vdp2_serial_ids", "").split(","):
        raise TraceError("requested VDP2 serial is absent from the complete receipt")
    try:
        receipt_serials = {int(value) for value in receipt["observed_vdp2_serial_ids"].split(",") if value}
    except (KeyError, ValueError) as error:
        raise TraceError("receipt has invalid observed_vdp2_serial_ids") from error
    verify_frame_index(args.rendered_frames_index, args.frame, args.serial, args.width, args.height,
                       args.reference_rgbx)
    expected_reference_bytes = args.width * args.height * 4
    if len(reference) != expected_reference_bytes:
        raise TraceError(
            f"reference must contain {expected_reference_bytes} RGBX bytes, got {len(reference)}"
        )
    if decoded_plane is not None and len(decoded_plane) != expected_reference_bytes:
        raise TraceError(
            "decoded plane must contain "
            f"{expected_reference_bytes} index/RGB bytes, got {len(decoded_plane)}"
        )
    try:
        expected_trace_bytes = int(receipt["bytes_written"])
    except (KeyError, ValueError) as error:
        raise TraceError("receipt has invalid bytes_written") from error
    if len(trace) != expected_trace_bytes:
        raise TraceError(f"receipt expects {expected_trace_bytes} trace bytes, got {len(trace)}")

    rows: dict[int, tuple[bytes, bytes, bytes]] = {}
    seen_records: set[tuple[int, int, int, int, int, int]] = set()
    line_layers: dict[tuple[int, int, int, int, int], dict[int, int]] = {}
    pass_rows: dict[tuple[int, int, int], set[int]] = {}
    observed_frames: set[int] = set()
    record_count = 0
    cursor = len(MAGIC)
    while cursor < len(trace):
        if len(trace) - cursor < HEADER_SIZE:
            raise TraceError("truncated layer record header")
        frame, serial, ordinal, y, field, layer, width = struct.unpack_from("<QQIIBBH", trace, cursor)
        record_start = cursor + HEADER_SIZE
        record_size = width * BYTES_PER_PIXEL
        record_end = record_start + record_size
        if width == 0 or width > 1024 or record_end > len(trace):
            raise TraceError(f"invalid or truncated layer record at byte {cursor}")
        if field not in (0, 1) or layer >= len(LAYER_NAMES):
            raise TraceError(f"invalid field or layer ID at byte {cursor}")
        if (not frame_begin <= frame < frame_end or not row_begin <= y < row_end or
                serial not in receipt_serials):
            raise TraceError(f"record at byte {cursor} lies outside the receipt's frame/row/serial coverage")
        key = (frame, serial, ordinal, y, field, layer)
        if key in seen_records:
            raise TraceError(f"duplicate layer row record {key}")
        seen_records.add(key)
        record_count += 1
        line_key = (frame, serial, ordinal, y, field)
        line_layers.setdefault(line_key, {})[layer] = width
        pass_rows.setdefault((frame, serial, field), set()).add(y)
        observed_frames.add(frame)
        color_end = record_start + width * 4
        priority_end = color_end + width
        if frame == args.frame and serial == args.serial and field == 0 and layer == args.layer:
            if width != args.width or not 0 <= y < args.height:
                raise TraceError(f"selected layer dimensions/row do not match the reference at row {y}")
            if y in rows:
                raise TraceError(f"selected layer has multiple main-field records for row {y}")
            rows[y] = (trace[record_start:color_end], trace[color_end:priority_end], trace[priority_end:record_end])
        cursor = record_end

    try:
        expected_records = int(receipt["records_written"])
    except (KeyError, ValueError) as error:
        raise TraceError("receipt has invalid records_written") from error
    if record_count != expected_records:
        raise TraceError(f"receipt expects {expected_records} records, found {record_count}")
    expected_layers = set(range(len(LAYER_NAMES)))
    for line_key, layers in line_layers.items():
        if set(layers) != expected_layers or len(set(layers.values())) != 1:
            raise TraceError(f"incomplete or inconsistent layer set for scanline {line_key}")
    expected_rows = set(range(row_begin, row_end))
    for pass_key, captured_rows in pass_rows.items():
        if captured_rows != expected_rows:
            raise TraceError(f"incomplete row coverage for pass {pass_key}")
    if observed_frames != set(range(frame_begin, frame_end)):
        raise TraceError("binary frame coverage does not match the complete receipt range")

    if set(rows) != set(range(args.height)):
        missing = sorted(set(range(args.height)) - set(rows))
        raise TraceError(f"selected layer is missing rows: {missing[:12]}")

    rgb_matches = 0
    priority_nonzero = 0
    special_color_calc = 0
    colors_seen: set[bytes] = set()
    rgba = bytearray(args.width * args.height * 4)
    decoded_active_pixels = 0
    decoded_active_rgb_matches = 0
    decoded_active_index_zero = 0
    decoded_inactive_pixels = 0
    decoded_inactive_rgb_matches = 0
    decoded_inactive_index_zero = 0
    decoded_active_rows: list[int] = []
    for y in range(args.height):
        colors, priorities, special_flags = rows[y]
        if any(priorities):
            decoded_active_rows.append(y)
        for x in range(args.width):
            offset = x * 4
            color = colors[offset : offset + 4]
            colors_seen.add(color[:3])
            if color[:3] == reference[(y * args.width + x) * 4 : (y * args.width + x) * 4 + 3]:
                rgb_matches += 1
            priority_nonzero += priorities[x] != 0
            special_color_calc += special_flags[x] != 0
            out = (y * args.width + x) * 4
            rgba[out : out + 3] = color[:3]
            rgba[out + 3] = 0xFF
            if decoded_plane is not None:
                plane_pixel = (y * args.width + x) * 4
                plane_index = decoded_plane[plane_pixel]
                plane_rgb = decoded_plane[plane_pixel + 1 : plane_pixel + 4]
                if priorities[x] != 0:
                    decoded_active_pixels += 1
                    decoded_active_index_zero += plane_index == 0
                    decoded_active_rgb_matches += color[:3] == plane_rgb
                else:
                    decoded_inactive_pixels += 1
                    decoded_inactive_index_zero += plane_index == 0
                    decoded_inactive_rgb_matches += color[:3] == plane_rgb

    result: dict[str, object] = {
        "trace_sha256": hashlib.sha256(trace).hexdigest(),
        "reference_rgbx_sha256": hashlib.sha256(reference).hexdigest(),
        "frame": args.frame,
        "vdp2_frame_serial": args.serial,
        "layer_id": args.layer,
        "layer_name": LAYER_NAMES[args.layer],
        "dimensions": [args.width, args.height],
        "pixels": args.width * args.height,
        "exact_rgb_matches": rgb_matches,
        "exact_rgb_mismatches": args.width * args.height - rgb_matches,
        "unique_layer_rgb_values": len(colors_seen),
        "nonzero_priority_pixels": priority_nonzero,
        "special_color_calc_pixels": special_color_calc,
        "interpretation": "same-pass layer-to-final comparison; this layer alone is not the composed frame",
    }
    if decoded_plane is not None:
        row_ranges: list[list[int]] = []
        for y in decoded_active_rows:
            if not row_ranges or y != row_ranges[-1][1] + 1:
                row_ranges.append([y, y])
            else:
                row_ranges[-1][1] = y
        result["decoded_plane_comparison"] = {
            "decoded_plane_sha256": hashlib.sha256(decoded_plane).hexdigest(),
            "format": "palette_index_u8,rgb_u8x3",
            "active_priority_pixels": decoded_active_pixels,
            "active_priority_rgb_matches": decoded_active_rgb_matches,
            "active_priority_rgb_mismatches": decoded_active_pixels - decoded_active_rgb_matches,
            "active_priority_index_zero_pixels": decoded_active_index_zero,
            "active_priority_row_ranges_inclusive": row_ranges,
            "zero_priority_pixels": decoded_inactive_pixels,
            "zero_priority_rgb_matches": decoded_inactive_rgb_matches,
            "zero_priority_index_zero_pixels": decoded_inactive_index_zero,
            "interpretation": "compare plane RGB only against Ymir layer pixels with nonzero priority; zero-priority pixels are reported separately",
        }
    if args.output_rgba:
        try:
            args.output_rgba.parent.mkdir(parents=True, exist_ok=True)
            args.output_rgba.write_bytes(rgba)
        except OSError as error:
            raise TraceError(f"cannot write {args.output_rgba}: {error}") from error
    if args.output_json:
        try:
            args.output_json.parent.mkdir(parents=True, exist_ok=True)
            args.output_json.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
        except OSError as error:
            raise TraceError(f"cannot write {args.output_json}: {error}") from error
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path, help="Ymir YMLYR001 binary layer trace")
    parser.add_argument("receipt", type=Path, help="adjacent .receipt.txt capture receipt")
    parser.add_argument("reference_rgbx", type=Path, help="same-pass Ymir RGBX8888LE frame")
    parser.add_argument("rendered_frames_index", type=Path, help="index.csv from the same Ymir capture")
    parser.add_argument("--frame", required=True, type=int)
    parser.add_argument("--serial", required=True, type=int, help="VDP2 frame serial")
    parser.add_argument("--layer", required=True, type=int, choices=range(len(LAYER_NAMES)))
    parser.add_argument("--width", required=True, type=int)
    parser.add_argument("--height", required=True, type=int)
    parser.add_argument(
        "--decoded-plane",
        type=Path,
        help="optional Firestaff plane with one palette-index byte followed by RGB bytes per pixel",
    )
    parser.add_argument("--output-rgba", type=Path, help="optional extracted layer RGBA image")
    parser.add_argument("--output-json", type=Path, help="optional machine-readable report")
    args = parser.parse_args(argv)
    try:
        result = analyze(args)
    except (TraceError, OSError, struct.error) as error:
        print(f"NEXUS_YMIR_VDP2_LAYER_TRACE_INVALID: {error}", file=sys.stderr)
        return 1
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
