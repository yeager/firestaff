#!/usr/bin/env python3
"""Regression cases for the bounded callback-state memory receipt."""

from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
import verify_theron_syscard_callback_state_trace as verifier  # noqa: E402


def row(index: int = 0, event: int = 7, address: str = "226a") -> str:
    logical = int(address, 16)
    slot = logical >> 13
    mprs = [0xFF, 0xF8, 0x80, 0x81, 0x82, 0x83, 0x01, 0x00]
    physical = (mprs[slot] << 13) | (logical & 0x1FFF)
    pc = 0xE14C
    pc_physical = (mprs[pc >> 13] << 13) | (pc & 0x1FFF)
    values = [
        "access=read", f"index={index}", f"event_sequence={event}",
        f"logical_address={logical:04x}", f"physical_address={physical:06x}",
        f"mpr={mprs[slot]:02x}", "value=02", f"logical_pc={pc:04x}",
        f"physical_pc={pc_physical:06x}",
        *(f"mpr{i}={mpr:02x}" for i, mpr in enumerate(mprs)),
    ]
    return "callback_state " + " ".join(values)


def test_valid_ordered_callback_state_rows() -> None:
    result = verifier.verify_lines([verifier.HEADER, row(), row(1, 9, "2259")])
    assert result == {"rows": 2, "reads": 2, "writes": 0}


def test_rejects_bad_scope_mapping_and_order() -> None:
    valid = [verifier.HEADER, row()]
    mutations = (
        row(address="226c"),
        row().replace("physical_address=1f026a", "physical_address=1f026b"),
        row().replace(" mpr=f8 ", " mpr=f9 "),
        row().replace("physical_pc=00014c", "physical_pc=00014d"),
        row().replace("event_sequence=7", "event_sequence=07"),
    )
    for bad in mutations:
        try:
            verifier.verify_lines([verifier.HEADER, bad])
        except ValueError:
            pass
        else:
            raise AssertionError(f"invalid callback-state receipt was accepted: {bad}")
    try:
        verifier.verify_lines([verifier.HEADER, row(0, 9), row(1, 8)])
    except ValueError:
        pass
    else:
        raise AssertionError("non-increasing shared event sequence was accepted")
    try:
        verifier.verify_lines([*valid, "callback_state_truncated cap=131072"])
    except ValueError:
        pass
    else:
        raise AssertionError("truncated callback-state trace was accepted")


def main() -> None:
    test_valid_ordered_callback_state_rows()
    test_rejects_bad_scope_mapping_and_order()
    print("test_verify_theron_syscard_callback_state_trace: PASS")


if __name__ == "__main__":
    main()
