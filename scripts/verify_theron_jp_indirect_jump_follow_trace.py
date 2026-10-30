#!/usr/bin/env python3
"""Validate bounded post-dispatch instruction windows against the JP dispatch trace."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

from verify_theron_indirect_jump_trace import ROW as DISPATCH_ROW
from verify_theron_indirect_jump_trace import verify as verify_indirect_jump


HEADER = (
    "source=mednafen-pce-fast-indirect-jump-follow-v1 "
    "event_sequence_source=TheronTraceCDSharedEventSequence "
    "window_limit=64 per_window_limit=256 total_limit=16384"
)
FOLLOW_ROW = re.compile(
    r"^indirect_jump_follow dispatch_event=(\d+) target_pc=([0-9a-f]{4}) "
    r"event_sequence=(\d+) index=(\d+) logical_pc=([0-9a-f]{4}) "
    r"physical_pc=([0-9a-f]{6}) bytes=([0-9a-f]{6}) "
    r"physical_bytes=([0-9a-f]{6}),([0-9a-f]{6}),([0-9a-f]{6}) "
    r"valid_mask=([0-7]) mpr0=([0-9a-f]{2}) mpr1=([0-9a-f]{2}) "
    r"mpr2=([0-9a-f]{2}) mpr3=([0-9a-f]{2}) mpr4=([0-9a-f]{2}) "
    r"mpr5=([0-9a-f]{2}) mpr6=([0-9a-f]{2}) mpr7=([0-9a-f]{2})$"
)
COMPLETE = re.compile(
    r"^indirect_jump_follow_complete dispatch_event=(\d+) event_sequence=(\d+) "
    r"count=(\d+) terminal=next-indirect-dispatch$"
)
TRUNCATED = re.compile(
    r"^indirect_jump_follow_truncated dispatch_event=(\d+) event_sequence=(\d+) "
    r"after_rows=(\d+) limit=(256|16384) reason=(instruction-cap|target-fetch-mismatch|total-limit)$"
)
SKIPPED = re.compile(
    r"^indirect_jump_follow_skipped dispatch_event=(\d+) event_sequence=(\d+) "
    r"reason=window-limit limit=64$"
)


def verify(dispatch_path: Path, code_follow_path: Path, follow_path: Path) -> tuple[int, int, int, int, int]:
    # This validates the dispatch rows and their in-window target against the
    # independent bounded code-follow trace before the new sidecar is parsed.
    verify_indirect_jump(dispatch_path, code_follow_path)

    dispatch_lines = dispatch_path.read_text(encoding="ascii").splitlines()
    dispatch_events: dict[int, int] = {}
    previous_dispatch_event = -1
    for line_number, line in enumerate(dispatch_lines[1:], 2):
        if line.startswith("indirect_jump_truncated "):
            raise ValueError("dispatch trace reports truncation")
        match = DISPATCH_ROW.fullmatch(line)
        if match is None:
            raise ValueError(f"malformed dispatch row at line {line_number}")
        fields = match.groups()
        event = int(fields[0])
        target = int(fields[15], 16)
        if event <= previous_dispatch_event:
            raise ValueError("dispatch event sequences are not strictly increasing")
        dispatch_events[event] = target
        previous_dispatch_event = event

    lines = follow_path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != HEADER:
        raise ValueError("unexpected indirect-jump follow header")

    windows: dict[int, dict[str, int | bool]] = {}
    last_event = -1
    total_rows = 0
    truncated_windows = 0
    skipped_windows = 0
    recorded_dispatches: set[int] = set()

    def dispatch_before(event: int, dispatch_event: int, line_number: int) -> None:
        if dispatch_event not in dispatch_events or dispatch_event >= event:
            raise ValueError(f"record at line {line_number} does not follow its dispatch")
        latest_dispatch = max((candidate for candidate in dispatch_events if candidate < event), default=-1)
        if latest_dispatch != dispatch_event:
            raise ValueError(f"record at line {line_number} does not belong to the latest dispatch")

    for line_number, line in enumerate(lines[1:], 2):
        row_match = FOLLOW_ROW.fullmatch(line)
        if row_match:
            fields = row_match.groups()
            dispatch_event = int(fields[0])
            target_pc = int(fields[1], 16)
            event = int(fields[2])
            index = int(fields[3])
            logical_pc = int(fields[4], 16)
            physical_pc = int(fields[5], 16)
            opcode_bytes = [int(fields[6][offset:offset + 2], 16) for offset in (0, 2, 4)]
            physical_bytes = [int(fields[i], 16) for i in (7, 8, 9)]
            valid_mask = int(fields[10])
            mprs = [int(value, 16) for value in fields[11:19]]
            if event <= last_event:
                raise ValueError(f"event sequence is not increasing at line {line_number}")
            last_event = event
            dispatch_before(event, dispatch_event, line_number)
            if target_pc != dispatch_events[dispatch_event]:
                raise ValueError(f"follow row at line {line_number} has no matching dispatch target")
            if index == 0:
                if dispatch_event in recorded_dispatches:
                    raise ValueError(f"dispatch has more than one follow record at line {line_number}")
                recorded_dispatches.add(dispatch_event)
                windows[dispatch_event] = {"count": 0, "closed": False, "truncated": False}
            window = windows.get(dispatch_event)
            if window is None:
                raise ValueError(f"follow row has no initial fetch at line {line_number}")
            if window["closed"]:
                raise ValueError(f"follow data resumes after a terminal record at line {line_number}")
            expected_index = int(window["count"])
            if index != expected_index or index >= 256:
                raise ValueError(f"follow row index is discontinuous or exceeds 256 at line {line_number}")
            if index == 0 and (logical_pc != target_pc or event <= dispatch_event):
                raise ValueError(f"first follow fetch does not match dispatch target at line {line_number}")
            slot = logical_pc >> 13
            if physical_pc != (mprs[slot] << 13) | (logical_pc & 0x1FFF):
                raise ValueError(f"follow physical PC mapping mismatch at line {line_number}")
            for offset in range(3):
                logical = (logical_pc + offset) & 0xFFFF
                byte_slot = logical >> 13
                expected_physical = (mprs[byte_slot] << 13) | (logical & 0x1FFF)
                valid = bool(valid_mask & (1 << offset))
                if valid and physical_bytes[offset] != expected_physical:
                    raise ValueError(f"follow opcode byte mapping mismatch at line {line_number}")
                if not valid and (physical_bytes[offset] != 0 or opcode_bytes[offset] != 0):
                    raise ValueError(f"invalid follow opcode bytes are not zeroed at line {line_number}")
            window["count"] = expected_index + 1
            total_rows += 1
            if total_rows > 16384:
                raise ValueError("follow trace exceeds its 16384-row total cap")
            continue

        complete = COMPLETE.fullmatch(line)
        truncated = TRUNCATED.fullmatch(line)
        skipped = SKIPPED.fullmatch(line)
        if complete:
            dispatch_event, event, count = map(int, complete.groups())
            dispatch_before(event, dispatch_event, line_number)
            marker_window = windows.get(dispatch_event)
            if event <= last_event or marker_window is None or marker_window["closed"]:
                raise ValueError(f"invalid completion marker at line {line_number}")
            if count != marker_window["count"] or not 1 <= count <= 256:
                raise ValueError(f"completion count mismatch at line {line_number}")
            last_event = event
            marker_window["closed"] = True
            continue
        if truncated:
            dispatch_event, event, count, limit, reason = truncated.groups()
            dispatch_event, event, count, limit = map(int, (dispatch_event, event, count, limit))
            dispatch_before(event, dispatch_event, line_number)
            marker_window = windows.get(dispatch_event)
            if event <= last_event or marker_window is None or marker_window["closed"]:
                raise ValueError(f"invalid truncation marker at line {line_number}")
            if count != marker_window["count"] or count < 1:
                raise ValueError(f"truncation count mismatch at line {line_number}")
            if reason == "instruction-cap" and (limit != 256 or count != 256):
                raise ValueError(f"instruction-cap marker does not match 256 rows at line {line_number}")
            if reason == "total-limit" and (limit != 16384 or total_rows != 16384):
                raise ValueError(f"total-limit marker does not match 16384 rows at line {line_number}")
            if reason == "target-fetch-mismatch" and limit != 256:
                raise ValueError(f"target mismatch marker has an invalid limit at line {line_number}")
            last_event = event
            marker_window["closed"] = True
            marker_window["truncated"] = True
            truncated_windows += 1
            continue
        if skipped:
            dispatch_event, event = map(int, skipped.groups())
            dispatch_before(event, dispatch_event, line_number)
            if event <= last_event or dispatch_event in recorded_dispatches:
                raise ValueError(f"invalid skipped-window marker at line {line_number}")
            last_event = event
            recorded_dispatches.add(dispatch_event)
            skipped_windows += 1
            continue
        raise ValueError(f"malformed or unknown follow record at line {line_number}")

    if len(windows) + skipped_windows > 64:
        raise ValueError("follow trace exceeds its 64-window cap")
    missing = set(dispatch_events) - recorded_dispatches
    if missing:
        final_dispatch = max(dispatch_events)
        if missing != {final_dispatch} or last_event >= final_dispatch:
            raise ValueError("dispatches are missing follow records before capture EOF")
    if not windows:
        if not dispatch_events or missing != {max(dispatch_events)}:
            raise ValueError("follow trace contains no instruction windows")
        return 0, 0, 0, 0, 0
    open_windows = sum(not bool(window["closed"]) for window in windows.values())
    if open_windows > 1:
        raise ValueError("multiple follow windows remain open")
    if open_windows and max(windows) != max(dispatch_events):
        raise ValueError("an open follow window is not the final dispatch")
    return total_rows, len(windows), open_windows, truncated_windows, skipped_windows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dispatch_trace", type=Path)
    parser.add_argument("code_follow_trace", type=Path)
    parser.add_argument("follow_trace", type=Path)
    args = parser.parse_args()
    try:
        rows, windows, open_windows, truncated_windows, skipped_windows = verify(
            args.dispatch_trace, args.code_follow_trace, args.follow_trace
        )
    except (OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print("PASS: bounded per-dispatch instruction windows are structurally valid")
    print(f"rows={rows}")
    print(f"windows={windows}")
    print(f"open_windows={open_windows}")
    print(f"truncated_windows={truncated_windows}")
    print(f"skipped_windows={skipped_windows}")
    print("LIMIT: a capped window is not evidence of a return or gameplay handoff")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
