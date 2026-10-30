#!/usr/bin/env python3
"""Exercise the bootstrap trace parser with parser-only records."""

from pathlib import Path
import sys

repo = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repo / "scripts"))

import verify_theron_jp_syscard_bootstrap_follow as verifier


def instruction_row(
    kind: str, index: int, pc: int, mpr2: int = 0, event_sequence: int = 1
) -> str:
    mprs = [0] * 8
    mprs[2] = mpr2
    physical_pc = (mprs[(pc >> 13) & 7] << 13) | (pc & 0x1FFF)
    physical_bytes = [
        (mprs[((pc + i) & 0xFFFF) >> 13] << 13) | (((pc + i) & 0xFFFF) & 0x1FFF)
        for i in range(4)
    ]
    return (
        f"{kind} index={index} event_sequence={event_sequence} "
        f"logical_pc={pc:04x} physical_pc={physical_pc:06x} "
        f"bytes=00000000 physical_bytes={','.join(f'{value:06x}' for value in physical_bytes)} "
        f"valid_mask=f a=00 x=00 y=00 sp=00 p=00 "
        + " ".join(f"mpr{i}={value:02x}" for i, value in enumerate(mprs))
    )


def test_v4_uses_contiguous_branch_follow_after_independent_window() -> None:
    def row(kind: str, index: int, pc: int, event: int, bytes_value: str, p: int = 0, mpr2: int = 0) -> str:
        base = instruction_row(kind, index, pc, mpr2, event)
        return base.replace("bytes=00000000", f"bytes={bytes_value}").replace(
            "p=00", f"p={p:02x}"
        )

    rows = [
        verifier.HEADER_V4,
        instruction_row("bootstrap_follow", 0, 0xE11B, event_sequence=100),
        instruction_row("target_follow", 0, 0x4000, 0x80, event_sequence=101),
        row("bootstrap_branch_follow", 0, 0xE11D, 102, "900d0000"),
        row("bootstrap_branch_follow", 1, 0xE12C, 103, "a9040000"),
        row("bootstrap_branch_follow", 2, 0xE12E, 104, "440d0000"),
        row("bootstrap_branch_follow", 3, 0xE13D, 105, "00000000"),
        row("bootstrap_branch_follow", 4, 0xE130, 106, "4c004000"),
        row("bootstrap_branch_follow", 5, 0x4000, 107, "64007300", mpr2=0x80),
    ]
    counts = verifier.verify_lines(rows)
    if counts != {"bootstrap_follow": 1, "target_follow": 1}:
        raise AssertionError("v4 did not validate the contiguous BIOS-to-$4000 branch")

    no_branch = [
        verifier.HEADER_V4,
        instruction_row("bootstrap_follow", 0, 0xE11B, event_sequence=100),
        instruction_row("target_follow", 0, 0x4000, 0x80, event_sequence=101),
    ]
    verifier.verify_lines(no_branch, require_launch=False)
    try:
        verifier.verify_lines(no_branch)
    except ValueError as error:
        if "did not observe the $E11D BIOS carry branch" not in str(error):
            raise
    else:
        raise AssertionError("a missing BIOS branch was accepted in strict verification mode")

    set_carry = rows.copy()
    set_carry[3] = set_carry[3].replace("p=00", "p=01")
    set_carry[4] = row("bootstrap_branch_follow", 1, 0xE11F, 103, "a9015300")
    try:
        verifier.verify_lines(set_carry)
    except ValueError as error:
        if "set-carry" not in str(error):
            raise
    else:
        raise AssertionError("the authentic set-carry alternate path was accepted as a launch")

    skipped_next_pc = rows.copy()
    skipped_next_pc[4] = row("bootstrap_branch_follow", 1, 0xE11F, 103, "a9015300")
    try:
        verifier.verify_lines(skipped_next_pc)
    except ValueError as error:
        if "not immediately followed by $E12C" not in str(error):
            raise
    else:
        raise AssertionError("non-contiguous branch-follow rows were accepted")

    earlier_target_only = rows[:-1]
    try:
        verifier.verify_lines(earlier_target_only)
    except ValueError as error:
        if "$E130 JMP was not immediately followed" not in str(error):
            raise
    else:
        raise AssertionError("an independent earlier $4000 window was accepted as the BIOS jump target")

    for row_index, valid_mask, message in (
        (3, "1", "$E11D BCC"),
        (4, "1", "$E12C LDA"),
        (5, "1", "$E12E BSR"),
        (7, "3", "$E130 JMP"),
    ):
        missing_operand = rows.copy()
        missing_operand[row_index] = missing_operand[row_index].replace(
            "valid_mask=f", f"valid_mask={valid_mask}"
        )
        try:
            verifier.verify_lines(missing_operand)
        except ValueError as error:
            if f"{message} instruction snapshot has invalid" not in str(error):
                raise
        else:
            raise AssertionError(f"{message} was accepted without all encoded instruction bytes")


def main() -> None:
    test_v4_uses_contiguous_branch_follow_after_independent_window()
    rows = [
        verifier.HEADER,
        instruction_row("bootstrap_follow", 0, 0xE11B, event_sequence=100),
        instruction_row("bootstrap_follow", 1, 0xE11D, event_sequence=101),
        instruction_row("target_follow", 0, 0x4000, 0x80, event_sequence=102),
    ]
    counts = verifier.verify_lines(rows)
    if counts != {"bootstrap_follow": 2, "target_follow": 1}:
        raise AssertionError("valid bounded bootstrap trace counts were not retained")
    target_first_rows = [
        verifier.HEADER,
        instruction_row("target_follow", 0, 0x4000, 0x80, event_sequence=200),
        instruction_row("bootstrap_follow", 0, 0xE11B, event_sequence=201),
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
    invalid_pc[-1] = instruction_row(
        "target_follow", 0, 0x14000, 0x80, event_sequence=102
    )
    try:
        verifier.verify_lines(invalid_pc)
    except ValueError as error:
        if "logical PC is out of range" not in str(error):
            raise
    else:
        raise AssertionError("an out-of-range logical PC was accepted")
    unordered = rows.copy()
    unordered[-1] = instruction_row(
        "target_follow", 0, 0x4000, 0x80, event_sequence=99
    )
    try:
        verifier.verify_lines(unordered)
    except ValueError as error:
        if "non-increasing shared event sequence" not in str(error):
            raise
    else:
        raise AssertionError("a reordered shared event sequence was accepted")
    malformed_event = rows.copy()
    malformed_event[1] = malformed_event[1].replace(
        "event_sequence=100", "event_sequence=+100"
    )
    try:
        verifier.verify_lines(malformed_event)
    except ValueError as error:
        if "malformed shared event sequence" not in str(error):
            raise
    else:
        raise AssertionError("a noncanonical shared event sequence was accepted")
    print("test_verify_theron_jp_syscard_bootstrap_follow: PASS")


if __name__ == "__main__":
    main()
