#!/usr/bin/env python3
"""Verify the observed $43AB -> $07 -> $279F Theron runtime data flow."""

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


def parse_fields(line: str) -> dict[str, str]:
    fields: dict[str, str] = {}
    for token in line.split():
        key, separator, value = token.partition("=")
        if separator:
            fields[key] = value
    return fields


def instruction(row: dict[str, str]) -> tuple[str, str, str]:
    return row["logical_pc"], row["physical_pc"], row["bytes"]


def verify(lines: list[str]) -> tuple[int, Counter[str]]:
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

    branches: Counter[str] = Counter()
    stores: list[str] = []
    consumer_inputs: list[str] = []
    consumers: list[int] = []
    indexed_x: Counter[tuple[str, str]] = Counter()
    for index, row in enumerate(rows):
        current = instruction(row)
        if current == EXPECTED_A1A9:
            if index + 7 >= len(rows) or instruction(rows[index + 7]) != EXPECTED_CALL:
                raise ValueError(f"A1A9 entry at row {index} lacks the $A1BE JSR")
        if current == EXPECTED_BRANCH:
            if index + 2 >= len(rows):
                raise ValueError(f"truncated $43AB branch at row {index}")
            target = rows[index + 1]["logical_pc"]
            if target == "43b0":
                if instruction(rows[index + 2]) != EXPECTED_STORE:
                    raise ValueError("fall-through branch did not store through $43B2")
                stored_value = rows[index + 2]["a"]
            elif target == "43b2":
                if instruction(rows[index + 1]) != EXPECTED_STORE:
                    raise ValueError("taken branch did not store through $43B2")
                stored_value = rows[index + 1]["a"]
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

        if current != EXPECTED_CONSUMER:
            continue
        if index + 9 >= len(rows):
            raise ValueError(f"truncated $C292 consumer sequence at row {index}")
        result = rows[index + 1]["a"]
        if result not in {"00", "02"}:
            raise ValueError(f"$C292 loaded unexpected A=${result}")
        consumer_inputs.append(result)
        expected = int(result, 16)
        for offset in range(2, 4):
            row_after_asl = rows[index + offset]
            if row_after_asl["bytes"][:2] != "0a":
                raise ValueError(f"expected ASL A after $C292 at row {index + offset}")
            expected = (expected << 1) & 0xFF
            if row_after_asl["a"] != f"{expected:02x}":
                raise ValueError("$07 consumer ASL result does not match its input")
        expected = (expected << 1) & 0xFF
        if instruction(rows[index + 4]) != EXPECTED_CONSUMER_STORE:
            raise ValueError("shifted $07 value was not stored at $C297 to $279F")
        stored = rows[index + 4]["a"]
        if stored != f"{expected:02x}":
            raise ValueError("$279F store value differs from the three ASL result")
        if instruction(rows[index + 5]) != EXPECTED_TAX:
            raise ValueError("expected sampled TAX bytes after the $279F store")
        if instruction(rows[index + 6]) != EXPECTED_INDEXED_READ:
            raise ValueError("expected sampled LDA $4842,X bytes after TAX")
        indexed_x[(result, rows[index + 6]["x"])] += 1
        consumers.append(expected)

    if not stores or len(stores) != len(consumers):
        raise ValueError(
            f"helper stores ({len(stores)}) and $07 consumers ({len(consumers)}) differ"
        )
    if stores != consumer_inputs:
        raise ValueError("$07 consumer values do not match the preceding helper stores")
    if len(stores) != 45:
        raise ValueError(f"expected 45 helper calls, got {len(stores)}")
    if branches != Counter({"43b0": 23, "43b2": 22}):
        raise ValueError(f"unexpected $43AB branch counts: {dict(branches)}")
    if Counter(f"{value:02x}" for value in consumers) != Counter({"10": 23, "00": 22}):
        raise ValueError("unexpected $279F selector result counts")
    if indexed_x != Counter({("02", "10"): 23, ("00", "10"): 22}):
        raise ValueError(f"unexpected following X-register observations: {dict(indexed_x)}")

    return len(rows), branches


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} TRACE", file=sys.stderr)
        return 2
    try:
        rows, branches = verify(Path(sys.argv[1]).read_text(encoding="utf-8").splitlines())
    except (OSError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(
        "PASS: rows={} calls=45 branch_fallthrough_43b0={} "
        "branch_taken_43b2={} zp07_to_279f_trace_correlation=verified "
        "following_indexed_X_stays_10=observed".format(
            rows, branches["43b0"], branches["43b2"]
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
