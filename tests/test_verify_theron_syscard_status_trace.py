#!/usr/bin/env python3
"""Exercise the System Card $1800 status receipt parser."""

from pathlib import Path
import sys

repo = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repo / "scripts"))

import verify_theron_syscard_status_trace as verifier


def row(index: int, event: int, status: str, bsy: int, req: int, msg: int, cd: int, io: int) -> str:
    return (
        f"status_read index={index} event_sequence={event} timestamp=123 "
        f"logical_pc=e95e physical_pc=00095e logical_address=1800 status={status} "
        f"bsy={bsy} req={req} msg={msg} cd={cd} io={io} db=00"
    )


def main() -> None:
    valid = [
        verifier.HEADER,
        row(0, 100, "90", 1, 0, 0, 1, 0),
        row(1, 101, "d0", 1, 1, 0, 1, 0),
        row(2, 104, "c8", 1, 1, 0, 0, 1),
    ]
    rows, statuses = verifier.verify_lines(valid)
    if rows != 3 or statuses != {"90": 1, "d0": 1, "c8": 1}:
        raise AssertionError("valid SCSI status rows were not summarized accurately")

    invalid = valid.copy()
    invalid[1] = invalid[1].replace("status=90", "status=d0")
    try:
        verifier.verify_lines(invalid)
    except ValueError as error:
        if "disagrees with sampled SCSI signals" not in str(error):
            raise
    else:
        raise AssertionError("a status byte inconsistent with signal bits was accepted")

    invalid = valid.copy()
    invalid[2] = invalid[2].replace("event_sequence=101", "event_sequence=100")
    try:
        verifier.verify_lines(invalid)
    except ValueError as error:
        if "non-increasing shared event sequence" not in str(error):
            raise
    else:
        raise AssertionError("a repeated shared event sequence was accepted")

    invalid = valid.copy()
    invalid[1] = invalid[1].replace("logical_address=1800", "logical_address=1801")
    try:
        verifier.verify_lines(invalid)
    except ValueError as error:
        if "outside the observed BIOS read" not in str(error):
            raise
    else:
        raise AssertionError("a status row for another register was accepted")

    invalid = valid.copy()
    invalid[1] = invalid[1].replace("physical_pc=00095e", "physical_pc=00085e")
    try:
        verifier.verify_lines(invalid)
    except ValueError as error:
        if "invalid timestamp or BIOS mapping" not in str(error):
            raise
    else:
        raise AssertionError("a status read with an incorrect physical BIOS mapping was accepted")

    print("test_verify_theron_syscard_status_trace: PASS")


if __name__ == "__main__":
    main()
