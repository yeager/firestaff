#!/usr/bin/env python3
"""Exercise the strict System Card I/O/RAM receipt using parser-only fixtures."""

from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
import verify_theron_syscard_io_ram_trace as verifier  # noqa: E402


MPRS = [0xFF, 0xF8, 0x80, 0x81, 0x82, 0x83, 0x01, 0x00]
IMAGE = bytearray(verifier.PCE_HEADER_BYTES + 0x10000)


def source_bytes(pc: int) -> bytes:
    start = (MPRS[pc >> 13] << 13) | (pc & 0x1FFF)
    return bytes(IMAGE[verifier.PCE_HEADER_BYTES + start:
                       verifier.PCE_HEADER_BYTES + start + 4])


def make_row(index: int, event: int, access: str, address: int, pc: int) -> str:
    value = source_bytes(pc)
    physical_code = [
        (MPRS[((pc + offset) & 0xFFFF) >> 13] << 13)
        | ((pc + offset) & 0x1FFF)
        for offset in range(4)
    ]
    physical_address = (MPRS[address >> 13] << 13) | (address & 0x1FFF)
    physical_pc = (MPRS[pc >> 13] << 13) | (pc & 0x1FFF)
    fields = [
        f"access={access}", f"index={index}", f"event_sequence={event}",
        f"logical_address={address:04x}", f"physical_address={physical_address:06x}",
        f"access_mpr={MPRS[address >> 13]:02x}", "value=81",
        f"logical_pc={pc:04x}", f"physical_pc={physical_pc:06x}",
        f"code_bytes={value.hex()}",
        "code_physical_bytes=" + ",".join(f"{item:06x}" for item in physical_code),
        "code_valid_mask=f", f"pc_mpr={MPRS[pc >> 13]:02x}",
        *(f"mpr{slot}={mpr:02x}" for slot, mpr in enumerate(MPRS)),
    ]
    return "io_ram " + " ".join(fields)


def must_reject(
    lines: list[str], image: bytes, label: str, expected_message: str
) -> None:
    try:
        verifier.verify_lines(lines, image)
    except ValueError as error:
        if expected_message not in str(error):
            raise AssertionError(
                f"invalid I/O/RAM receipt rejected for the wrong reason ({label}): {error}"
            ) from error
        return
    raise AssertionError(f"invalid I/O/RAM receipt accepted: {label}")


def main() -> None:
    image = bytes(IMAGE)
    rows = [
        verifier.HEADER,
        make_row(0, 30, "read", 0x1800, 0xE900),
        make_row(1, 33, "write", 0x224C, 0xEC05),
        make_row(2, 37, "read", 0x22A4, 0xE95E),
        make_row(3, 41, "write", 0x224C, 0xCB1B),
    ]
    assert verifier.verify_lines(rows, image) == {
        "rows": 4, "reads": 2, "writes": 2, "addresses": 3,
        "missing_addresses": len(verifier.ADDRESSES) - 3,
    }
    valid = make_row(0, 30, "write", 0x224C, 0xEC05)
    must_reject(
        [verifier.HEADER, valid.replace("logical_address=224c", "logical_address=224b")],
        image, "scope", "out-of-scope logical address",
    )
    must_reject(
        [verifier.HEADER, valid.replace("physical_address=1f024c", "physical_address=1f024d")],
        image, "data mapping", "logical/physical data mapping mismatch",
    )
    must_reject(
        [verifier.HEADER, valid.replace("logical_address=224c", "logical_address=1800")],
        image, "MPR mapping", "data MPR disagrees with logical address",
    )
    must_reject(
        [verifier.HEADER, valid.replace("code_bytes=00000000", "code_bytes=ff000000")],
        image, "BIOS opcode", "instruction byte disagrees with authentic BIOS",
    )
    must_reject(
        [verifier.HEADER, valid.replace("code_valid_mask=f", "code_valid_mask=e")],
        image, "missing opcode", "opcode byte is not mapped",
    )
    must_reject([
        verifier.HEADER,
        make_row(0, 30, "read", 0x1800, 0xE900),
        make_row(1, 37, "read", 0x22A4, 0xE95E).replace("event_sequence=37", "event_sequence=29"),
    ], image, "event order", "non-increasing shared event sequence")
    must_reject(
        [verifier.HEADER, valid, "io_ram_truncated cap=4194304"],
        image, "truncation", "truncation marker disagrees with row count",
    )
    original_cap = verifier.CAP
    try:
        verifier.CAP = 1
        assert verifier.verify_lines([verifier.HEADER, valid], image)["rows"] == 1
        must_reject(
            [verifier.HEADER, valid, "io_ram_truncated cap=1"],
            image, "reached cap", "reached its configured row cap",
        )
        must_reject(
            [verifier.HEADER, valid, make_row(1, 31, "read", 0x1801, 0xE900)],
            image, "exceeded cap", "exceeds its configured cap",
        )
    finally:
        verifier.CAP = original_cap
    must_reject(
        ["source=other", valid], image, "header", "unexpected System Card I/O/RAM trace header"
    )
    print("test_verify_theron_syscard_io_ram_trace: PASS")


if __name__ == "__main__":
    main()
