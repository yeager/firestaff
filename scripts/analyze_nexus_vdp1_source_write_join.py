#!/usr/bin/env python3
"""Join VDP1 draw source spans to an authenticated VRAM-write trace.

The join proves only that a captured runtime writer touched the observed
source interval.  It does not identify MENU.BPK/DGN ownership, CLUT, placement
or a production rendering consumer; semantic admission therefore stays
blocked.
"""

from __future__ import annotations

import argparse
import collections
import re
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions
from analyze_nexus_vdp1_command_window import command_window


HEADER = "FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V1"
LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) size=(?P<size>[0-9]+) "
    r"value=0x(?P<value>[0-9a-fA-F]+) pc0=0x(?P<pc0>[0-9a-fA-F]+) "
    r"pc1=0x(?P<pc1>[0-9a-fA-F]+)$"
)


def load_trace(path: Path) -> list[tuple[int, int, int, int]]:
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != HEADER:
        raise ValueError("invalid VDP1 write-trace header")
    rows: list[tuple[int, int, int, int]] = []
    for line_number, line in enumerate(lines[1:], 2):
        match = LINE.fullmatch(line)
        if not match:
            raise ValueError(f"malformed VDP1 write-trace line {line_number}")
        rows.append((
            int(match["addr"], 16),
            int(match["size"], 10),
            int(match["pc0"], 16),
            int(match["pc1"], 16),
        ))
    return rows


def source_spans(frame: dict[str, bytes], state: str) -> list[tuple[int, int, int, int, int]]:
    spans: list[tuple[int, int, int, int, int]] = []
    for command_offset, words in command_window(frame["vdp1-vram"], state):
        control = words[0]
        command_type = control & 0x000F
        if control & 0x8000 or command_type > 2:
            continue
        colour_mode = (words[2] >> 3) & 0x7
        width = (words[5] & 0x003F) * 8
        height = (words[5] >> 8) & 0x00FF
        bits_per_pixel = 4 if colour_mode <= 1 else 8 if colour_mode <= 4 else 16
        source_offset = words[4] * 8
        source_size = (width * height * bits_per_pixel) // 8
        if source_size <= 0 or source_offset + source_size > len(frame["vdp1-vram"]):
            continue
        spans.append((command_offset, command_type, colour_mode, source_offset, source_size))
    return spans


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("write_trace", type=Path)
    parser.add_argument("--capture-frames", type=int, default=2)
    parser.add_argument("--frame", type=int, default=-1,
                        help="join one frame; default joins every captured frame")
    args = parser.parse_args()
    try:
        frames, states = frame_regions(args.capture.read_bytes(), args.capture_frames)
        writes = load_trace(args.write_trace)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP1_SOURCE_WRITE_JOIN_INVALID: {error}")
        return 1
    if args.frame >= len(frames):
        print("NEXUS_VDP1_SOURCE_WRITE_JOIN_INVALID: frame outside capture")
        return 1

    frame_indexes = [args.frame] if args.frame >= 0 else range(len(frames))
    print(f"capture_frames={len(frames)} write_records={len(writes)}")
    print("capture_writer_session_identity=unbound")
    for frame_index in frame_indexes:
        for command_offset, command_type, colour_mode, source_offset, source_size in source_spans(
                frames[frame_index], states[frame_index]):
            source_end = source_offset + source_size
            covered: set[int] = set()
            pcs: collections.Counter[int] = collections.Counter()
            matching_rows = 0
            for address, size, pc0, pc1 in writes:
                if address < source_offset or address + size > source_end:
                    continue
                matching_rows += 1
                covered.update(range(address, address + size))
                pcs[pc0] += 1
                if pc1:
                    pcs[pc1] += 1
            pc_text = ",".join(
                f"0x{pc:08x}:{count}" for pc, count in pcs.most_common()
            ) or "none"
            print(
                f"frame={frame_index} command=0x{command_offset:05x} "
                f"type={command_type} colour_mode={colour_mode} "
                f"source=0x{source_offset:05x}-0x{source_end:05x} "
                f"bytes={source_size} write_rows={matching_rows} "
                f"covered_bytes={len(covered)} coverage={len(covered)}/{source_size} "
                f"writer_pcs={pc_text}"
            )
    print("asset_owner=unbound")
    print("clut_placement_consumer=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
