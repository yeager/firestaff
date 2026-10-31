#!/usr/bin/env python3
"""Regression cases for the bounded System Card BIOS decision trace."""

from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
import verify_theron_syscard_decision_trace as verifier  # noqa: E402


MPRS = [0xFF, 0xF8, 0x80, 0x81, 0x82, 0x83, 0x01, 0x00]


SOURCE_BYTES = {
    "e11b": "442f", "e11d": "900d", "e12c": "a904",
    "e130": "4c0040", "e14c": "ad6a22", "e174": "18",
    "e176": "8d6922", "ec05": "9c7322", "ec69": "9008",
    "e900": "ada422", "e9c5": "ad9b22", "e209": "38", "e20a": "60",
}


def row(index: int, event: int, pc: str) -> str:
    logical_pc = int(pc, 16)
    physical_pc = (MPRS[logical_pc >> 13] << 13) | (logical_pc & 0x1FFF)
    physical_bytes = [
        (MPRS[((logical_pc + offset) & 0xFFFF) >> 13] << 13)
        | ((logical_pc + offset) & 0x1FFF)
        for offset in range(4)
    ]
    fields = [
        f"index={index}", f"event_sequence={event}", f"logical_pc={pc}",
        f"physical_pc={physical_pc:06x}",
        f"bytes={SOURCE_BYTES[pc]:<8}".replace(" ", "0"),
        "physical_bytes=" + ",".join(f"{value:06x}" for value in physical_bytes),
        "valid_mask=f", "a=00", "x=25", "y=14", "sp=ff", "p=03",
        *(f"mpr{slot}={value:02x}" for slot, value in enumerate(MPRS)),
    ]
    return "decision " + " ".join(fields)


def expect_rejected(lines: list[str], expected_message: str) -> None:
    try:
        verifier.verify_lines(lines)
    except ValueError as error:
        assert expected_message in str(error), str(error)
    else:
        raise AssertionError(f"accepted trace; expected: {expected_message}")


def main() -> None:
    result = verifier.verify_lines([
        verifier.HEADER,
        row(0, 30, "e209"),
        row(1, 32, "e20a"),
        row(2, 35, "e11d"),
        row(3, 38, "e9c5"),
    ])
    assert result == {"rows": 4, "pcs": 4}

    mutations = (
        (row(0, 30, "e209").replace("physical_pc=000209", "physical_pc=000208"),
         "logical/physical PC mapping mismatch"),
        (row(0, 30, "e209").replace("event_sequence=30", "event_sequence=030"),
         "malformed event_sequence"),
        (row(0, 30, "e209").replace("logical_pc=e209", "logical_pc=e208"),
         "decision row has an unrequested PC"),
        (row(0, 30, "e209").replace("bytes=38000000", "bytes=18000000"),
         "instruction bytes disagree with source lock"),
        (row(0, 30, "e209").replace("valid_mask=f", "valid_mask=e"),
         "source opcode bytes are unmapped"),
    )
    for bad, message in mutations:
        expect_rejected([verifier.HEADER, bad], message)

    expect_rejected(
        [verifier.HEADER, row(0, 30, "e209"), row(1, 29, "e20a")],
        "non-increasing shared event sequence",
    )

    cap_rows = [verifier.HEADER] + [
        row(index, 30 + index, "e209") for index in range(verifier.CAP)
    ]
    marker = f"decision_truncated cap={verifier.CAP}"
    cap_cases = (
        (cap_rows + [marker], "reached its configured row cap"),
        (cap_rows, "reached its configured row cap"),
        ([verifier.HEADER], "contains no requested instruction starts"),
        ([verifier.HEADER, marker], "truncation marker disagrees"),
        (cap_rows + [row(verifier.CAP, 30 + verifier.CAP, "e209")],
         "exceeds its configured cap"),
    )
    for lines, message in cap_cases:
        expect_rejected(lines, message)

    print("test_verify_theron_syscard_decision_trace: PASS")


if __name__ == "__main__":
    main()
