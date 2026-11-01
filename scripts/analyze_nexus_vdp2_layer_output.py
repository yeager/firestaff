#!/usr/bin/env python3
"""Summarize a Ymir YMLYR001 VDP2 layer-output capture."""
from collections import Counter, defaultdict
from pathlib import Path
import argparse
import struct


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path, help="Ymir nexus-vdp2-layer-outputs.bin")
    args = parser.parse_args()
    data = args.capture.read_bytes()
    if data[:8] != b"YMLYR001":
        parser.error(f"unexpected layer-output magic: {data[:8]!r}")

    layers = defaultdict(lambda: {
        "rows": set(),
        "pixels": 0,
        "nonzero": 0,
        "colors": set(),
        "priority": Counter(),
        "special": Counter(),
    })
    offset = 8
    record_count = 0
    frames, serials = set(), set()
    while offset < len(data):
        frame, serial, ordinal, row, field, layer, width = struct.unpack_from(
            "<QQIIBBH", data, offset
        )
        record_size = 28 + 6 * width
        if width == 0 or offset + record_size > len(data):
            parser.error(f"truncated or invalid record {record_count} at {offset}")
        color_offset = offset + 28
        colors = struct.unpack_from(f"<{width}I", data, color_offset)
        priority_offset = color_offset + 4 * width
        priority = data[priority_offset:priority_offset + width]
        special = data[priority_offset + width:priority_offset + 2 * width]

        stats = layers[layer]
        stats["rows"].add(row)
        stats["pixels"] += width
        stats["nonzero"] += sum(color != 0 for color in colors)
        stats["colors"].update(color for color in colors if color)
        stats["priority"].update(priority)
        stats["special"].update(special)
        frames.add(frame)
        serials.add(serial)
        record_count += 1
        offset += record_size

    if offset != len(data):
        parser.error("record stream did not end at the file boundary")
    print(
        f"records={record_count} bytes={len(data)} "
        f"frames={sorted(frames)} vdp2_serials={sorted(serials)}"
    )
    for layer, stats in sorted(layers.items()):
        rows = stats["rows"]
        nonzero_percent = 100 * stats["nonzero"] / stats["pixels"]
        print(
            f"layer={layer} rows={len(rows)} range={min(rows)}:{max(rows)} "
            f"pixels={stats['pixels']} nonzero={stats['nonzero']} "
            f"({nonzero_percent:.3f}%) "
            f"unique_nonzero_colors={len(stats['colors'])} "
            f"priorities={dict(stats['priority'])} "
            f"special={dict(stats['special'])}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
