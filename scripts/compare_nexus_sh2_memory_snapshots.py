#!/usr/bin/env python3
"""Compare matched Nexus WorkRAMH captures against a forced-no-input run.

This is an evidence reader for authentic, frame-aligned emulator captures. It
reports changed memory bytes and their input masks; it does not assign game
state or action semantics to those bytes.
"""

from __future__ import annotations

import argparse
import re
from collections import defaultdict
from pathlib import Path
from typing import Iterator


SNAPSHOT_HEADER = b"FIRESTAFF_NEXUS_SH2_MEMORY_SNAPSHOT_V1\n"
SNAPSHOT_ROW = re.compile(
    rb"frame=(?P<frame>[0-9]+) base=0x(?P<base>[0-9a-fA-F]+) "
    rb"size=(?P<size>[0-9]+)\n"
)
INPUT_HEADER = "FIRESTAFF_NEXUS_INPUT_TRACE_V1"
INPUT_ROW = re.compile(r"^frame=(?P<frame>[0-9]+) mask=0x(?P<mask>[0-9a-fA-F]{4})$")
WORKRAMH_SIZE = 0x100000


def iter_snapshots(path: Path) -> Iterator[tuple[int, int, bytes]]:
    """Yield validated snapshots one at a time to bound memory use."""
    with path.open("rb") as stream:
        if stream.readline() != SNAPSHOT_HEADER:
            raise ValueError(f"{path}: invalid WorkRAMH snapshot header")
        previous_frame = -1
        while True:
            header = stream.readline()
            if not header:
                break
            match = SNAPSHOT_ROW.fullmatch(header)
            if not match:
                raise ValueError(f"{path}: malformed snapshot row")
            frame = int(match["frame"])
            base = int(match["base"], 16)
            size = int(match["size"])
            if frame <= previous_frame or base != 0x06000000 or size != WORKRAMH_SIZE:
                raise ValueError(f"{path}: invalid, duplicate or unordered frame {frame}")
            payload = stream.read(size)
            if len(payload) != size or stream.read(1) != b"\n":
                raise ValueError(f"{path}: truncated WorkRAMH payload at frame {frame}")
            previous_frame = frame
            yield frame, base, payload


def read_input_masks(path: Path) -> dict[int, int]:
    """Read input frames, allowing duplicate producer rows for one mask."""
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != INPUT_HEADER:
        raise ValueError(f"{path}: invalid input trace header")
    masks: dict[int, int] = {}
    for number, line in enumerate(lines[1:], 2):
        match = INPUT_ROW.fullmatch(line)
        if not match:
            raise ValueError(f"{path}: malformed input trace row {number}")
        frame = int(match["frame"])
        mask = int(match["mask"], 16)
        if frame in masks and masks[frame] != mask:
            raise ValueError(f"{path}: conflicting input masks at frame {frame}")
        masks[frame] = mask
    return masks


def differing_spans(control: bytes, observed: bytes, base: int) -> list[tuple[int, int]]:
    if len(control) != len(observed):
        raise ValueError("snapshot payload sizes differ")
    spans: list[tuple[int, int]] = []
    start = -1
    for offset, (before, after) in enumerate(zip(control, observed)):
        if before != after:
            if start < 0:
                start = offset
        elif start >= 0:
            spans.append((base + start, base + offset))
            start = -1
    if start >= 0:
        spans.append((base + start, base + len(control)))
    return spans


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("control", type=Path,
                        help="forced-no-button WorkRAMH snapshot stream")
    parser.add_argument("input", type=Path,
                        help="matched input-run WorkRAMH snapshot stream")
    parser.add_argument("--input-events", required=True, type=Path,
                        help="same-session FIRESTAFF Nexus input event trace")
    args = parser.parse_args()

    try:
        masks = read_input_masks(args.input_events)
        control_iter = iter_snapshots(args.control)
        input_iter = iter_snapshots(args.input)
        by_mask: dict[int, list[int]] = defaultdict(list)
        changed_frames = 0
        compared = 0
        compared_frames: set[int] = set()
        while True:
            control_row = next(control_iter, None)
            input_row = next(input_iter, None)
            if control_row is None or input_row is None:
                if control_row is not None or input_row is not None:
                    raise ValueError("control and input captures have different frame counts")
                break
            frame, control_base, control_bytes = control_row
            input_frame, input_base, input_bytes = input_row
            if frame != input_frame or control_base != input_base:
                raise ValueError("control and input captures are not frame-aligned")
            compared_frames.add(frame)
            spans = differing_spans(control_bytes, input_bytes, control_base)
            compared += 1
            if spans:
                changed_frames += 1
                mask = masks.get(frame, 0)
                by_mask[mask].append(frame)
                rendered = ",".join(
                    f"0x{start:08x}:{control_bytes[start - control_base:end - control_base].hex()}"
                    f">{input_bytes[start - input_base:end - input_base].hex()}"
                    for start, end in spans
                )
                print(f"frame={frame} mask=0x{mask:04x} changed_bytes="
                      f"{sum(end - start for start, end in spans)} spans={rendered}")
        if not compared:
            raise ValueError("snapshot streams contain no matched frames")
        unmatched_events = sorted(set(masks) - compared_frames)
        if unmatched_events:
            raise ValueError(f"input trace names frames without snapshots: {unmatched_events}")
    except (OSError, UnicodeError, ValueError, StopIteration) as error:
        print(f"NEXUS_SH2_MEMORY_SNAPSHOT_COMPARE_INVALID: {error}")
        return 1

    print(f"matched_frames={compared}")
    print(f"frames_with_ram_differences={changed_frames}")
    seen_masks = set(masks.values())
    if len(masks) < compared:
        seen_masks.add(0)
    print("changed_frames_while_mask_active=" + ",".join(
        f"0x{mask:04x}:{len(by_mask.get(mask, []))}"
        for mask in sorted(seen_masks)))
    print("button_action_semantics=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
