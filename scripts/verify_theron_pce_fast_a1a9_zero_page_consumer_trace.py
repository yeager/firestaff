#!/usr/bin/env python3
"""Verify register-local A1A9 trace evidence for the $43AB selector path."""

from __future__ import annotations

import sys
from collections import Counter
from pathlib import Path


SOURCE_MARKER = "mednafen-pce-fast-a1a9-initializer-code-v1"
EXPECTED_A1A9 = ("a1a9", "0e01a9", "628d80288d8128")
EXPECTED_CALL = ("a1be", "0e01be", "20ab439c1f279c")
EXPECTED_BRANCH = ("43ad", "0d03ad", "0f0602a9028507")
EXPECTED_STORE = ("43b2", "0d03b2", "850760a9020f06")
EXPECTED_CONSUMER = ("c292", "0d2292", "a5070a0a0a8d9f")
EXPECTED_CONSUMER_STORE = ("c297", "0d2297", "8d9f27aabd4248")
EXPECTED_TAX = ("c29a", "0d229a", "aabd4248852664")
EXPECTED_INDEXED_READ = ("c29b", "0d229b", "bd424885266423")
EXPECTED_ASL = (
    ("c294", "0d2294", "0a"),
    ("c295", "0d2295", "0a"),
    ("c296", "0d2296", "0a"),
)
INSTRUCTION_WINDOW_SIZE = 128
EXPECTED_WINDOW_COUNT = 45


def parse_fields(line: str) -> dict[str, str]:
    fields: dict[str, str] = {}
    for token in line.split():
        key, separator, value = token.partition("=")
        if separator:
            fields[key] = value
    return fields


def instruction(row: dict[str, str]) -> tuple[str, str, str]:
    return row["logical_pc"], row["physical_pc"], row["bytes"]


def unique_instruction_index(
    window: list[dict[str, str]], expected: tuple[str, str, str], label: str
) -> int:
    matches = [index for index, row in enumerate(window) if instruction(row) == expected]
    if len(matches) != 1:
        raise ValueError(f"expected one {label} instruction in its A1A9 window, got {len(matches)}")
    return matches[0]


def verify(lines: list[str]) -> tuple[int, Counter[str], int]:
    if not lines or lines[0] != f"source={SOURCE_MARKER}":
        raise ValueError("unexpected or missing trace source marker")

    rows = [
        parse_fields(line)
        for line in lines[1:]
        if line.startswith("a1a9_init_instruction ")
    ]
    if not rows:
        raise ValueError("trace contains no A1A9 initializer instructions")
    for sequence, row in enumerate(rows):
        if int(row["sequence"]) != sequence:
            raise ValueError(
                f"non-contiguous instruction sequence at row {sequence}: "
                f"got {row.get('sequence', 'missing')}"
            )

    if len(rows) % INSTRUCTION_WINDOW_SIZE:
        raise ValueError(
            f"trace has {len(rows)} instruction rows, not complete "
            f"{INSTRUCTION_WINDOW_SIZE}-row A1A9 windows"
        )

    branches: Counter[str] = Counter()
    stores: list[str] = []
    consumer_inputs: list[str] = []
    consumers: list[int] = []
    indexed_x: Counter[tuple[str, str]] = Counter()
    validated_a1a9_entries = 0
    for window_number, start in enumerate(range(0, len(rows), INSTRUCTION_WINDOW_SIZE)):
        window = rows[start : start + INSTRUCTION_WINDOW_SIZE]
        if instruction(window[0]) != EXPECTED_A1A9:
            raise ValueError(f"A1A9 window {window_number} does not start at the authenticated entry")

        call_index = unique_instruction_index(window, EXPECTED_CALL, "$A1BE JSR")
        if call_index == 0:
            raise ValueError(f"A1A9 window {window_number} has no preceding entry instruction")
        branch_index = unique_instruction_index(window, EXPECTED_BRANCH, "$43AB branch")
        consumer_index = unique_instruction_index(window, EXPECTED_CONSUMER, "$C292 consumer")
        if not call_index < branch_index < consumer_index:
            raise ValueError(
                f"A1A9 window {window_number} does not order its $A1BE call, "
                "$43AB helper and $C292 consumer"
            )
        validated_a1a9_entries += 1

        if instruction(window[branch_index]) == EXPECTED_BRANCH:
            if branch_index + 2 >= len(window):
                raise ValueError(f"truncated $43AB branch in A1A9 window {window_number}")
            target = window[branch_index + 1]["logical_pc"]
            if target == "43b0":
                if instruction(window[branch_index + 2]) != EXPECTED_STORE:
                    raise ValueError("fall-through branch did not store through $43B2")
                stored_value = window[branch_index + 2]["a"]
            elif target == "43b2":
                if instruction(window[branch_index + 1]) != EXPECTED_STORE:
                    raise ValueError("taken branch did not store through $43B2")
                stored_value = window[branch_index + 1]["a"]
            else:
                raise ValueError(f"unexpected $43AB branch target ${target}")
            expected_value = "02" if target == "43b0" else "00"
            if stored_value != expected_value:
                raise ValueError(
                    f"$43AB branch to ${target} stored A=${stored_value}, "
                    f"expected ${expected_value}"
                )
            branches[target] += 1
            stores.append(stored_value)

        if consumer_index + 6 >= len(window):
            raise ValueError(f"truncated $C292 consumer sequence in A1A9 window {window_number}")
        result = window[consumer_index + 1]["a"]
        if result not in {"00", "02"}:
            raise ValueError(f"$C292 loaded unexpected A=${result}")
        consumer_inputs.append(result)
        expected = int(result, 16)
        for offset, expected_asl in enumerate(EXPECTED_ASL, start=1):
            row_after_asl = window[consumer_index + offset]
            actual_asl = (
                row_after_asl["logical_pc"],
                row_after_asl["physical_pc"],
                row_after_asl["bytes"][:2],
            )
            if actual_asl != expected_asl:
                raise ValueError(
                    f"unexpected ASL A address/opcode in A1A9 window {window_number} "
                    f"at row {consumer_index + offset}"
                )
            if row_after_asl["a"] != f"{expected:02x}":
                raise ValueError("$07 consumer ASL result does not match its input")
            expected = (expected << 1) & 0xFF
        if instruction(window[consumer_index + 4]) != EXPECTED_CONSUMER_STORE:
            raise ValueError("shifted $07 value was not stored at $C297 to $279F")
        stored = window[consumer_index + 4]["a"]
        if stored != f"{expected:02x}":
            raise ValueError("$279F store value differs from the three ASL result")
        if instruction(window[consumer_index + 5]) != EXPECTED_TAX:
            raise ValueError("expected sampled TAX bytes after the $279F store")
        if instruction(window[consumer_index + 6]) != EXPECTED_INDEXED_READ:
            raise ValueError("expected sampled LDA $4842,X bytes after TAX")
        if window[consumer_index + 6]["x"] != stored:
            raise ValueError(
                f"live X=${window[consumer_index + 6]['x']} does not match the "
                f"$279F value ${stored} in A1A9 window {window_number}"
            )
        indexed_x[(result, window[consumer_index + 6]["x"])] += 1
        consumers.append(expected)

    if not stores or len(stores) != len(consumers):
        raise ValueError(
            f"helper stores ({len(stores)}) and $07 consumers ({len(consumers)}) differ"
        )
    if stores != consumer_inputs:
        raise ValueError("$07 consumer values do not match the preceding helper stores")
    if validated_a1a9_entries != EXPECTED_WINDOW_COUNT:
        raise ValueError(
            f"expected {EXPECTED_WINDOW_COUNT} validated A1A9 entries, "
            f"got {validated_a1a9_entries}"
        )
    if len(stores) != validated_a1a9_entries:
        raise ValueError(
            f"validated A1A9 entries ({validated_a1a9_entries}) and "
            f"$43AB helper calls ({len(stores)}) differ"
        )
    if branches != Counter({"43b0": 23, "43b2": 22}):
        raise ValueError(f"unexpected $43AB branch counts: {dict(branches)}")
    if Counter(f"{value:02x}" for value in consumers) != Counter({"10": 23, "00": 22}):
        raise ValueError("unexpected $279F selector result counts")
    if indexed_x != Counter({("02", "10"): 23, ("00", "00"): 22}):
        raise ValueError(f"unexpected live X-register observations: {dict(indexed_x)}")

    return len(rows), branches, validated_a1a9_entries


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} TRACE", file=sys.stderr)
        return 2
    try:
        rows, branches, calls = verify(Path(sys.argv[1]).read_text(encoding="utf-8").splitlines())
    except (OSError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(
        "PASS: rows={} calls={} branch_fallthrough_43b0={} "
        "branch_taken_43b2={} zp07_to_279f_to_X=verified".format(
            rows, calls, branches["43b0"], branches["43b2"]
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
