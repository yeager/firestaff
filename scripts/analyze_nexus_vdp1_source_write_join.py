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
import hashlib
import re
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions
from analyze_nexus_vdp1_command_window import command_window


HEADERS = {
    "FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V1",
    "FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V2",
}
LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) size=(?P<size>[0-9]+) "
    r"value=0x(?P<value>[0-9a-fA-F]+) pc0=0x(?P<pc0>[0-9a-fA-F]+) "
    r"pc1=0x(?P<pc1>[0-9a-fA-F]+)$"
)


def parse_trace_lines(
        lines: list[str], selected_frame: int = -1
        ) -> tuple[list[tuple[int, int, int, int]],
                   list[tuple[int, int, int, int]]]:
    if not lines or lines[0] not in HEADERS:
        raise ValueError("invalid VDP1 write-trace header")
    version = lines[0]
    rows: list[tuple[int, int, int, int]] = []
    frame_rows: dict[int, list[tuple[int, int, int, int]]] = {}
    pre_capture_rows: list[tuple[int, int, int, int]] = []
    active_frame = -1
    for line_number, line in enumerate(lines[1:], 2):
        if line.startswith("frame="):
            if version != "FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V2":
                raise ValueError(f"frame marker in V1 line {line_number}")
            try:
                frame = int(line[6:], 10)
            except ValueError as error:
                raise ValueError(f"malformed frame marker line {line_number}") from error
            if frame in frame_rows:
                raise ValueError(f"duplicate frame {frame}")
            frame_rows[frame] = []
            active_frame = frame
            continue
        match = LINE.fullmatch(line)
        if not match:
            raise ValueError(f"malformed VDP1 write-trace line {line_number}")
        row = (
            int(match["addr"], 16),
            int(match["size"], 10),
            int(match["pc0"], 16),
            int(match["pc1"], 16),
        )
        if version == "FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V2":
            if active_frame < 0:
                # The V2 trace file opens on the first VDP1 write, which can
                # precede frame capture. Preserve those writes as a distinct
                # pre-capture prefix; they are not attributable to frame 0.
                pre_capture_rows.append(row)
            else:
                frame_rows[active_frame].append(row)
        else:
            rows.append(row)
    if version == "FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V2":
        # The producer emits a frame marker, then the writes observed for
        # that frame. Empty frames are represented by an empty list.
        if selected_frame >= 0:
            if selected_frame not in frame_rows:
                raise ValueError(f"missing frame {selected_frame}")
            return pre_capture_rows, frame_rows[selected_frame]
        return pre_capture_rows, [row for frame in sorted(frame_rows)
                                  for row in frame_rows[frame]]
    if selected_frame >= 0:
        raise ValueError("--frame requires a V2 write trace")
    return [], rows


def load_trace(path: Path, selected_frame: int = -1
               ) -> tuple[list[tuple[int, int, int, int]],
                          list[tuple[int, int, int, int]]]:
    return parse_trace_lines(path.read_text(encoding="ascii").splitlines(), selected_frame)


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


def manifest_trace_binding(manifest: Path, raw: Path, write_trace: Path) -> str:
    try:
        fields = {
            key: value for key, value in
            (line.split("=", 1) for line in manifest.read_text(encoding="utf-8").splitlines()
             if "=" in line)
        }
        raw_ok = fields.get("raw_sha256", "").lower() == hashlib.sha256(
            raw.read_bytes()).hexdigest()
        trace_ok = fields.get("vdp1_write_trace_sha256", "").lower() == hashlib.sha256(
            write_trace.read_bytes()).hexdigest()
    except (OSError, UnicodeError, ValueError):
        return "invalid"
    return "verified" if raw_ok and trace_ok else "unbound"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("write_trace", type=Path)
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--capture-frames", type=int, default=2)
    parser.add_argument("--frame", type=int, default=-1,
                        help="join one frame; default joins every captured frame")
    args = parser.parse_args()
    try:
        frames, states = frame_regions(args.capture.read_bytes(), args.capture_frames)
        pre_capture_writes, writes = load_trace(args.write_trace, args.frame)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP1_SOURCE_WRITE_JOIN_INVALID: {error}")
        return 1
    if args.frame >= len(frames):
        print("NEXUS_VDP1_SOURCE_WRITE_JOIN_INVALID: frame outside capture")
        return 1

    frame_indexes = [args.frame] if args.frame >= 0 else range(len(frames))
    print(f"capture_frames={len(frames)} write_records={len(writes)} "
          f"pre_capture_write_records={len(pre_capture_writes)}")
    identity = (manifest_trace_binding(args.manifest, args.capture, args.write_trace)
                if args.manifest else "unbound")
    print(f"capture_writer_session_identity={identity}")
    for frame_index in frame_indexes:
        for command_offset, command_type, colour_mode, source_offset, source_size in source_spans(
                frames[frame_index], states[frame_index]):
            source_end = source_offset + source_size
            def write_summary(
                    rows: list[tuple[int, int, int, int]]) -> tuple[int, int, str]:
                covered: set[int] = set()
                pcs: collections.Counter[int] = collections.Counter()
                matching_rows = 0
                for address, size, pc0, pc1 in rows:
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
                return matching_rows, len(covered), pc_text

            frame_rows, frame_covered, frame_pcs = write_summary(writes)
            prefix_rows, prefix_covered, prefix_pcs = write_summary(pre_capture_writes)
            print(
                f"frame={frame_index} command=0x{command_offset:05x} "
                f"type={command_type} colour_mode={colour_mode} "
                f"source=0x{source_offset:05x}-0x{source_end:05x} "
                f"bytes={source_size} frame_write_rows={frame_rows} "
                f"frame_covered_bytes={frame_covered} "
                f"frame_coverage={frame_covered}/{source_size} "
                f"frame_writer_pcs={frame_pcs} "
                f"pre_capture_write_rows={prefix_rows} "
                f"pre_capture_covered_bytes={prefix_covered} "
                f"pre_capture_writer_pcs={prefix_pcs}"
            )
    print("asset_owner=unbound")
    print("clut_placement_consumer=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
