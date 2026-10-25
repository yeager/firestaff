#!/usr/bin/env python3
"""Summarize authentic Ymir YMCMP1 per-pixel composition evidence.

The trace records candidate layer IDs, priorities, candidate Color888 words,
special-state bytes, and the final Color888 word. It does not prove retail
source ownership or explain why a candidate was blended.
"""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
from pathlib import Path
import struct
import sys


HEADER_SIZE = 32
PIXEL_RECORD_SIZE = 24
LAYER_NAMES = (
    "SPRITE", "RBG0", "NBG0_RBG1", "NBG1_EXBG", "NBG2", "NBG3",
    "BACK", "LINE_COLOR", "INVALID",
)


class TraceError(ValueError):
    pass


def read_trace(path: Path) -> bytes:
    try:
        data = path.read_bytes()
    except OSError as error:
        raise TraceError(f"cannot read {path}: {error}") from error
    if len(data) < HEADER_SIZE or data[:8] != b"YMCMP1\0\0":
        raise TraceError(f"{path}: missing YMCMP1 header")
    frame, = struct.unpack_from("<Q", data, 8)
    width, height, record_size, header_size = struct.unpack_from("<4I", data, 16)
    if width <= 0 or height <= 0 or record_size != PIXEL_RECORD_SIZE or \
            header_size != HEADER_SIZE:
        raise TraceError(
            f"{path}: unsupported header frame={frame} size={width}x{height} "
            f"record={record_size} header={header_size}"
        )
    expected_size = HEADER_SIZE + height * (4 + width * record_size)
    if len(data) != expected_size:
        raise TraceError(f"{path}: expected {expected_size} bytes, got {len(data)}")
    return data


def rgb(color888: int) -> tuple[int, int, int]:
    return color888 & 0xff, (color888 >> 8) & 0xff, (color888 >> 16) & 0xff


def analyze(data: bytes) -> None:
    frame, = struct.unpack_from("<Q", data, 8)
    width, height, record_size, header_size = struct.unpack_from("<4I", data, 16)
    slot_layers = [Counter() for _ in range(3)]
    slot_priorities = [Counter() for _ in range(3)]
    invalid_records = 0
    top_rgb_matches = 0
    final_rgb_equals_all_three = 0
    sprite_special = Counter()
    mesh_special = Counter()

    cursor = header_size
    for y in range(height):
        row_y, = struct.unpack_from("<I", data, cursor)
        cursor += 4
        if row_y != y:
            raise TraceError(f"row {y}: embedded row index is {row_y}")
        for _x in range(width):
            record = data[cursor:cursor + record_size]
            cursor += record_size
            candidate_colors: list[int] = []
            for slot in range(3):
                offset = slot * 6
                layer = record[offset]
                priority = record[offset + 1]
                color, = struct.unpack_from("<I", record, offset + 2)
                if layer >= len(LAYER_NAMES):
                    invalid_records += 1
                    layer_name = f"UNKNOWN_{layer}"
                else:
                    layer_name = LAYER_NAMES[layer]
                slot_layers[slot][layer_name] += 1
                slot_priorities[slot][priority] += 1
                candidate_colors.append(color)
            sprite_special[record[18]] += 1
            mesh_special[record[19]] += 1
            final_color, = struct.unpack_from("<I", record, 20)
            final_rgb = rgb(final_color)
            if final_rgb == rgb(candidate_colors[0]):
                top_rgb_matches += 1
            if all(final_rgb == rgb(candidate) for candidate in candidate_colors):
                final_rgb_equals_all_three += 1

    print(f"trace_sha256={hashlib.sha256(data).hexdigest()}")
    print(f"frame={frame}")
    print(f"dimensions={width}x{height}")
    print(f"pixel_records={width * height}")
    for slot in range(3):
        for layer_name, count in sorted(slot_layers[slot].items()):
            print(f"slot{slot}.{layer_name}={count}")
        for priority, count in sorted(slot_priorities[slot].items()):
            print(f"slot{slot}.priority{priority}={count}")
    print(f"unknown_layer_records={invalid_records}")
    print(f"final_rgb_matches_slot0={top_rgb_matches}")
    print(f"final_rgb_matches_all_candidate_colors={final_rgb_equals_all_three}")
    for kind, counts in (("sprite", sprite_special), ("mesh", mesh_special)):
        for value, count in sorted(counts.items()):
            print(f"{kind}_special_type{value}={count}")
    print("interpretation=raw Ymir candidates and output; no source or blend claim")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path, help="authentic Ymir YMCMP1 trace")
    args = parser.parse_args(argv)
    try:
        data = read_trace(args.trace)
        analyze(data)
    except TraceError as error:
        print(f"NEXUS_YMIR_COMPOSITION_TRACE_INVALID: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
