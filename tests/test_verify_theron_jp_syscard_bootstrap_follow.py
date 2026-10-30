#!/usr/bin/env python3
"""Exercise the bootstrap trace parser with parser-only records."""

from pathlib import Path
import sys

repo = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repo / "scripts"))

import verify_theron_jp_syscard_bootstrap_follow as verifier


def instruction_row(kind: str, index: int, pc: int, mpr2: int = 0) -> str:
    mprs = [0] * 8
    mprs[2] = mpr2
    physical_pc = (mprs[(pc >> 13) & 7] << 13) | (pc & 0x1FFF)
    physical_bytes = [
        (mprs[((pc + i) & 0xFFFF) >> 13] << 13) | (((pc + i) & 0xFFFF) & 0x1FFF)
        for i in range(4)
    ]
    return (
        f"{kind} index={index} logical_pc={pc:04x} physical_pc={physical_pc:06x} "
        f"bytes=00000000 physical_bytes={','.join(f'{value:06x}' for value in physical_bytes)} "
        f"valid_mask=f a=00 x=00 y=00 sp=00 p=00 "
        + " ".join(f"mpr{i}={value:02x}" for i, value in enumerate(mprs))
    )


def main() -> None:
    rows = [
        verifier.HEADER,
        instruction_row("bootstrap_follow", 0, 0xE11B),
        instruction_row("bootstrap_follow", 1, 0xE11D),
        instruction_row("target_follow", 0, 0x4000, 0x80),
    ]
    counts = verifier.verify_lines(rows)
    if counts != {"bootstrap_follow": 2, "target_follow": 1}:
        raise AssertionError("valid bounded bootstrap trace counts were not retained")
    target_first_rows = [
        verifier.HEADER,
        instruction_row("target_follow", 0, 0x4000, 0x80),
        instruction_row("bootstrap_follow", 0, 0xE11B),
    ]
    if verifier.verify_lines(target_first_rows) != {"bootstrap_follow": 1, "target_follow": 1}:
        raise AssertionError("independently armed target-before-bootstrap windows were rejected")
    invalid = rows.copy()
    invalid[-1] = invalid[-1].replace("physical_pc=100000", "physical_pc=000000")
    try:
        verifier.verify_lines(invalid)
    except ValueError as error:
        if "mapping mismatch" not in str(error):
            raise
    else:
        raise AssertionError("incorrect logical/physical mapping was accepted")
    try:
        verifier.verify_lines([verifier.HEADER])
    except ValueError as error:
        if "did not observe both" not in str(error):
            raise
    else:
        raise AssertionError("an empty capture was accepted as a verified handoff trace")
    invalid_pc = rows.copy()
    invalid_pc[-1] = instruction_row("target_follow", 0, 0x14000, 0x80)
    try:
        verifier.verify_lines(invalid_pc)
    except ValueError as error:
        if "logical PC is out of range" not in str(error):
            raise
    else:
        raise AssertionError("an out-of-range logical PC was accepted")
    print("test_verify_theron_jp_syscard_bootstrap_follow: PASS")


if __name__ == "__main__":
    main()
