#!/usr/bin/env python3
"""Validate HuC6280 indexed-indirect jump reads against subsequent fetches."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

from verify_theron_jp_code_follow_trace import verify as verify_code_follow


HEADER = re.compile(
    r"^source=mednafen-pce-fast-indirect-jump-trace-v1 "
    r"event_sequence_source=TheronTraceCDSharedEventSequence "
    r"target_pcs=40e1 limit=([1-9][0-9]*)$"
)
ROW = re.compile(
    r"^indirect_jump event_sequence=(\d+) row=(\d+) instruction_pc=([0-9a-f]{4}) "
    r"instruction_physical_pc=([0-9a-f]{6}) base=([0-9a-f]{4}) index=([0-9a-f]{2}) "
    r"table_address=([0-9a-f]{4}) low=([0-9a-f]{2}) high=([0-9a-f]{2}) "
    r"low_slot=(\d+) low_mpr=([0-9a-f]{2}) low_physical=([0-9a-f]{6}) "
    r"high_slot=(\d+) high_mpr=([0-9a-f]{2}) high_physical=([0-9a-f]{6}) "
    r"target=([0-9a-f]{4}) mpr0=([0-9a-f]{2}) mpr1=([0-9a-f]{2}) "
    r"mpr2=([0-9a-f]{2}) mpr3=([0-9a-f]{2}) mpr4=([0-9a-f]{2}) "
    r"mpr5=([0-9a-f]{2}) mpr6=([0-9a-f]{2}) mpr7=([0-9a-f]{2})$"
)
FETCH = re.compile(
    r"^code_follow event_sequence=(\d+) index=(\d+) logical_pc=([0-9a-f]{4}) "
    r"physical_pc=([0-9a-f]{6}) bytes=([0-9a-f]{6}) "
    r"physical_bytes=([0-9a-f]{6}),([0-9a-f]{6}),([0-9a-f]{6}) "
    r"valid_mask=([0-7]) mpr0=([0-9a-f]{2}) mpr1=([0-9a-f]{2}) "
    r"mpr2=([0-9a-f]{2}) mpr3=([0-9a-f]{2}) mpr4=([0-9a-f]{2}) "
    r"mpr5=([0-9a-f]{2}) mpr6=([0-9a-f]{2}) mpr7=([0-9a-f]{2})$"
)


def parse(path: Path, pattern: re.Pattern[str], label: str) -> list[tuple[str, ...]]:
    try:
        lines = path.read_text(encoding="ascii").splitlines()
    except (OSError, UnicodeError) as exc:
        raise ValueError(f"cannot read {label}: {exc}") from exc
    rows: list[tuple[str, ...]] = []
    for number, line in enumerate(lines, 1):
        if number == 1:
            continue
        if line.startswith("indirect_jump_truncated"):
            raise ValueError(f"{label} reports truncation")
        match = pattern.fullmatch(line)
        if match is None:
            raise ValueError(f"malformed {label} row at line {number}")
        rows.append(match.groups())
    return rows


def verify(dispatch_path: Path, fetch_path: Path) -> int:
    try:
        lines = dispatch_path.read_text(encoding="ascii").splitlines()
    except (OSError, UnicodeError) as exc:
        raise ValueError(f"cannot read dispatch trace: {exc}") from exc
    if not lines:
        raise ValueError("empty dispatch trace")
    header = HEADER.fullmatch(lines[0])
    if header is None:
        raise ValueError("unexpected dispatch trace header")
    limit = int(header.group(1))
    if not 1 <= limit <= 4096:
        raise ValueError("dispatch trace limit is out of range")

    dispatch_rows = parse(dispatch_path, ROW, "dispatch")
    try:
        fetch_lines = fetch_path.read_text(encoding="ascii").splitlines()
    except (OSError, UnicodeError) as exc:
        raise ValueError(f"cannot read code-follow trace: {exc}") from exc
    fetch_header = (
        "source=mednafen-pce-fast-system-card-code-follow-v1 start_pc=6148 "
        "limit=256 terminal=next-instruction-fetch "
        "event_sequence_source=TheronTraceCDSharedEventSequence"
    )
    if not fetch_lines or fetch_lines[0] != fetch_header:
        raise ValueError("unexpected code-follow trace header")
    if not fetch_lines[-1].startswith("code_follow_complete "):
        raise ValueError("code-follow trace is incomplete")
    status, validated_rows = verify_code_follow(fetch_path, require_complete=True)
    if status != "complete" or validated_rows != 256:
        raise ValueError("code-follow verifier did not confirm the complete 256-row trace")
    fetch_rows, fetch_start_sequence, fetch_complete_sequence = parse_fetch_rows(fetch_lines)
    if not dispatch_rows:
        raise ValueError("dispatch trace contains no target rows")
    if len(dispatch_rows) > limit:
        raise ValueError("dispatch rows exceed the declared cap")

    expected_row = 0
    previous_event = -1
    observed_pcs: set[int] = set()
    matched_rows = 0
    out_of_window_rows = 0
    parsed_fetches = []
    for fields in fetch_rows:
        parsed_fetches.append((int(fields[0]), int(fields[2], 16), int(fields[3], 16),
                               [int(fields[4][offset:offset + 2], 16)
                                for offset in (0, 2, 4)],
                               [int(fields[index], 16) for index in (5, 6, 7)],
                               int(fields[8]),
                               [int(value, 16) for value in fields[9:17]]))

    for fields in dispatch_rows:
        (event_s, row_s, pc_s, instruction_physical_s, base_s, index_s,
         table_s, low_s, high_s, low_slot_s, low_mpr_s, low_physical_s,
         high_slot_s, high_mpr_s, high_physical_s, target_s, *mpr_s) = fields
        event, row, pc = int(event_s), int(row_s), int(pc_s, 16)
        instruction_physical = int(instruction_physical_s, 16)
        base, index = int(base_s, 16), int(index_s, 16)
        table, low, high = int(table_s, 16), int(low_s, 16), int(high_s, 16)
        target = int(target_s, 16)
        if row != expected_row or event <= previous_event:
            raise ValueError("dispatch rows are not contiguous and event-ordered")
        expected_row += 1
        previous_event = event
        observed_pcs.add(pc)
        if pc != 0x40E1:
            raise ValueError(f"unexpected indexed-jump PC ${pc:04X}")
        if table != base + index or table > 0xFFFF:
            raise ValueError("indexed dispatch address does not equal base plus X")
        low_slot, high_slot = int(low_slot_s), int(high_slot_s)
        low_mpr, high_mpr = int(low_mpr_s, 16), int(high_mpr_s, 16)
        mprs = [int(value, 16) for value in mpr_s]
        instruction_slot = pc >> 13
        if instruction_physical != (mprs[instruction_slot] << 13) | (pc & 0x1FFF):
            raise ValueError("dispatch instruction physical mapping mismatch")
        if low_slot != table >> 13 or high_slot != (table + 1) >> 13:
            raise ValueError("dispatch table MPR slot mismatch")
        if low_mpr != int(mpr_s[low_slot], 16) or high_mpr != int(mpr_s[high_slot], 16):
            raise ValueError("dispatch MPR snapshot mismatch")
        if int(low_physical_s, 16) != (low_mpr << 13) | (table & 0x1FFF):
            raise ValueError("low-byte physical mapping mismatch")
        if int(high_physical_s, 16) != (high_mpr << 13) | ((table + 1) & 0x1FFF):
            raise ValueError("high-byte physical mapping mismatch")
        if target != (low | (high << 8)):
            raise ValueError("resolved target differs from the two observed bytes")
        if instruction_physical > 0x1FFFFF:
            raise ValueError("instruction physical PC is out of range")
        if fetch_start_sequence < event < fetch_complete_sequence:
            preceding = next((item for item in reversed(parsed_fetches) if item[0] < event), None)
            if preceding is None or preceding[1] != pc or preceding[2] != instruction_physical:
                raise ValueError("dispatch row does not follow its recorded instruction fetch")
            _, _, _, instruction_bytes, physical_bytes, valid_mask, fetch_mprs = preceding
            if instruction_bytes != [0x7C, 0x0D, 0x41] or valid_mask & 0x7 != 0x7:
                raise ValueError("dispatch row is not preceded by a complete $7C $410D operand fetch")
            if physical_bytes[:3] != [instruction_physical + offset for offset in range(3)]:
                raise ValueError("indirect-jump instruction bytes have inconsistent physical addresses")
            if fetch_mprs != mprs:
                raise ValueError("instruction fetch and dispatch MPR snapshots differ")
            following = next((item for item in parsed_fetches if item[0] > event), None)
            if following is None or following[1] != target:
                raise ValueError("resolved target does not match the next captured fetch")
            _, _, physical_pc, _, _, _, fetch_mprs = following
            slot = target >> 13
            expected_physical_pc = (fetch_mprs[slot] << 13) | (target & 0x1FFF)
            if physical_pc != expected_physical_pc:
                raise ValueError("next instruction fetch has an inconsistent MPR mapping")
            matched_rows += 1
        else:
            out_of_window_rows += 1

    if observed_pcs != {0x40E1} or not matched_rows:
        raise ValueError("trace does not correlate the observed $40E1 dispatch to its next fetch")
    return len(dispatch_rows), matched_rows, out_of_window_rows


def parse_fetch_rows(lines: list[str]) -> tuple[list[tuple[str, ...]], int, int]:
    rows: list[tuple[str, ...]] = []
    complete = re.compile(r"^code_follow_complete event_sequence=(\d+) count=(\d+)$")
    previous_sequence = -1
    completion_sequence: int | None = None
    for number, line in enumerate(lines[1:], 2):
        if line.startswith("code_follow_complete "):
            match = complete.fullmatch(line)
            if (match is None or int(match.group(2)) != len(rows) or
                    number != len(lines)):
                raise ValueError("invalid code-follow completion record")
            completion_sequence = int(match.group(1))
            continue
        match = FETCH.fullmatch(line)
        if match is None:
            raise ValueError(f"malformed code-follow row at line {number}")
        fields = match.groups()
        sequence, index = int(fields[0]), int(fields[1])
        if sequence <= previous_sequence or index != len(rows):
            raise ValueError("code-follow rows are not ordered and contiguous")
        previous_sequence = sequence
        rows.append(fields)
    if len(rows) != 256:
        raise ValueError("code-follow trace did not contain all 256 rows")
    if completion_sequence is None or completion_sequence <= previous_sequence:
        raise ValueError("code-follow completion sequence is missing or unordered")
    return rows, int(rows[0][0]), completion_sequence


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dispatch_trace", type=Path)
    parser.add_argument("code_follow_trace", type=Path)
    args = parser.parse_args()
    try:
        rows, matched, out_of_window = verify(args.dispatch_trace, args.code_follow_trace)
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    print(
        f"PASS: rows={rows} next_fetch_matches={matched} "
        f"out_of_window_rows={out_of_window} truncated=0"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
