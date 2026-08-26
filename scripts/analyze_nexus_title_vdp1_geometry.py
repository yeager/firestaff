#!/usr/bin/env python3
"""Bind the authenticated title VDP1 chain to its sequential draw geometry.

This models only the documented VDP1 command-coordinate widths: 13-bit signed
sprite/polygon vertices and 11-bit signed local-coordinate commands.  It does
not rasterize, infer a camera, resolve draw priority, or authorize native
presentation by itself.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file
from analyze_nexus_vdp1_command_sequence import find_chain, parse_copr


def signed(value: int, width: int) -> int:
    value &= (1 << width) - 1
    return value - (1 << width) if value & (1 << (width - 1)) else value


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--frame", type=int, default=0)
    parser.add_argument("--summary", action="store_true")
    parser.add_argument("--require-distorted", type=int)
    args = parser.parse_args()
    try:
        frame = next(regions for index, regions in
                     iter_frame_regions_file(args.capture, args.frame + 1)
                     if index == args.frame)
        chain = find_chain(frame["vdp1-vram"], parse_copr(frame["vdp1-state"]))
    except (OSError, StopIteration, ValueError) as error:
        print(f"NEXUS_TITLE_VDP1_GEOMETRY_INVALID: {error}")
        return 1
    if not chain:
        print("NEXUS_TITLE_VDP1_GEOMETRY_INVALID: title frame has no active chain")
        return 1

    local_x = local_y = 0
    distorted = 0
    vertices: list[tuple[int, int]] = []
    local_updates = []
    for record in chain:
        if record.end:
            continue
        words = record.words
        if record.command_type == 10:
            local_x, local_y = signed(words[6], 11), signed(words[7], 11)
            local_updates.append((record.offset, local_x, local_y))
            continue
        if record.command_type != 2:
            continue
        points = tuple((signed(words[index], 13) + local_x,
                        signed(words[index + 1], 13) + local_y)
                       for index in (6, 8, 10, 12))
        distorted += 1
        vertices.extend(points)
        if not args.summary:
            print("command=0x%05x local=(%d,%d) vertices=%s" %
                  (record.offset, local_x, local_y,
                   ",".join("(%d,%d)" % point for point in points)))
    if not vertices:
        print("NEXUS_TITLE_VDP1_GEOMETRY_INVALID: title chain has no distorted sprites")
        return 1
    print(f"frame={args.frame} distorted_sprites={distorted}")
    print("local_coordinate_updates=" + ",".join(
        "0x%05x:(%d,%d)" % update for update in local_updates))
    print("vertex_bounds=(%d,%d)-(%d,%d)" %
          (min(x for x, _ in vertices), min(y for _, y in vertices),
           max(x for x, _ in vertices), max(y for _, y in vertices)))
    print("vdp1_coordinate_widths=vertices:13,local:11")
    print("draw_geometry=authentic_command_observation")
    print("raster_priority_compositor_timing_semantics=blocked")
    if args.require_distorted is not None and distorted != args.require_distorted:
        print("required_distorted_sprite_count=missing")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
