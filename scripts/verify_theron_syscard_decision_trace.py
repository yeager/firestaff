#!/usr/bin/env python3
"""Validate bounded, source-mapped System Card BIOS decision-PC rows."""

from __future__ import annotations

import re
import sys
from pathlib import Path


CAP = 2048
PCS = {
    "e11b", "e11d", "e12c", "e130", "e14c", "e174", "e176",
    "ec05", "ec69", "e900", "e9c5", "e209", "e20a",
}
HEADER = (
    "source=mednafen-pce-fast-system-card-decision-v1 "
    "event_sequence_source=TheronTraceCDSharedEventSequence limit=2048 "
    "logical_pcs=e11b,e11d,e12c,e130,e14c,e174,e176,ec05,ec69,e900,e9c5,e209,e20a"
)
FIELDS = (
    "index", "event_sequence", "logical_pc", "physical_pc", "bytes",
    "physical_bytes", "valid_mask", "a", "x", "y", "sp", "p",
    *(f"mpr{i}" for i in range(8)),
)
# Opcodes are bound to the checked-in disassembly sourced from the
# hash-verified System Card 3.0 BIOS. Include following bytes where the
# disassembly gives a complete instruction at the requested PC.
SOURCE_BYTES = {
    "e11b": "442f", "e11d": "900d", "e12c": "a904",
    "e130": "4c0040", "e14c": "ad6a22", "e174": "18",
    "e176": "8d6922", "ec05": "9c7322", "ec69": "9008",
    "e900": "ada422", "e9c5": "ad9b22", "e209": "38", "e20a": "60",
}


def verify_lines(lines: list[str]) -> dict[str, int]:
    if not lines or lines[0] != HEADER:
        raise ValueError("unexpected System Card decision trace header")
    rows = 0
    previous_event = -1
    truncated = False
    pcs: set[str] = set()
    for line_number, line in enumerate(lines[1:], start=2):
        if line == f"decision_truncated cap={CAP}":
            if truncated or rows != CAP:
                raise ValueError("decision trace truncation marker disagrees with row count")
            truncated = True
            continue
        if truncated or not line.startswith("decision "):
            raise ValueError(f"malformed or out-of-order decision row at line {line_number}")
        values: dict[str, str] = {}
        for item in line.removeprefix("decision ").split():
            key, separator, value = item.partition("=")
            if not separator or key in values:
                raise ValueError(f"malformed decision field at line {line_number}")
            values[key] = value
        if tuple(values) != FIELDS:
            raise ValueError(f"unexpected decision fields at line {line_number}")
        for key in ("index", "event_sequence"):
            if not re.fullmatch(r"(?:0|[1-9][0-9]*)", values[key]):
                raise ValueError(f"malformed {key} at line {line_number}")
        if int(values["index"]) != rows:
            raise ValueError(f"non-contiguous decision row at line {line_number}")
        event = int(values["event_sequence"])
        if event <= previous_event:
            raise ValueError(f"non-increasing shared event sequence at line {line_number}")
        previous_event = event
        pc = values["logical_pc"]
        if pc not in PCS:
            raise ValueError(f"decision row has an unrequested PC at line {line_number}")
        pcs.add(pc)
        if not re.fullmatch(r"[0-9a-f]{8}", values["bytes"]):
            raise ValueError(f"malformed instruction bytes at line {line_number}")
        if not re.fullmatch(r"[0-9a-f]{6}(?:,[0-9a-f]{6}){3}", values["physical_bytes"]):
            raise ValueError(f"malformed mapped instruction bytes at line {line_number}")
        if not re.fullmatch(r"[0-9a-f]", values["valid_mask"]):
            raise ValueError(f"malformed valid mask at line {line_number}")
        if not re.fullmatch(r"[0-9a-f]{4}", values["logical_pc"]):
            raise ValueError(f"malformed logical PC at line {line_number}")
        for key in ("physical_pc",):
            if not re.fullmatch(r"[0-9a-f]{6}", values[key]):
                raise ValueError(f"malformed physical PC at line {line_number}")
        for key in ("a", "x", "y", "sp", "p", *(f"mpr{i}" for i in range(8))):
            if not re.fullmatch(r"[0-9a-f]{2}", values[key]):
                raise ValueError(f"malformed register {key} at line {line_number}")
        mprs = [int(values[f"mpr{i}"], 16) for i in range(8)]
        logical_pc = int(pc, 16)
        expected_physical_pc = (mprs[logical_pc >> 13] << 13) | (logical_pc & 0x1FFF)
        if int(values["physical_pc"], 16) != expected_physical_pc:
            raise ValueError(f"logical/physical PC mapping mismatch at line {line_number}")
        physical_bytes = [int(value, 16) for value in values["physical_bytes"].split(",")]
        valid_mask = int(values["valid_mask"], 16)
        if valid_mask > 0xF:
            raise ValueError(f"valid mask exceeds four bytes at line {line_number}")
        source_bytes = SOURCE_BYTES[pc]
        # SOURCE_BYTES entries are compact hex strings (two characters per byte).
        if source_bytes:
            required_mask = (1 << (len(source_bytes) // 2)) - 1
            if valid_mask & required_mask != required_mask:
                raise ValueError(f"source opcode bytes are unmapped at line {line_number}")
            if not values["bytes"].startswith(source_bytes):
                raise ValueError(f"instruction bytes disagree with source lock at line {line_number}")
        for offset, physical in enumerate(physical_bytes):
            logical = (logical_pc + offset) & 0xFFFF
            expected = (mprs[logical >> 13] << 13) | (logical & 0x1FFF)
            if valid_mask & (1 << offset):
                if physical != expected:
                    raise ValueError(f"instruction byte mapping mismatch at line {line_number}")
            elif physical != 0:
                raise ValueError(f"unmapped instruction byte has a physical address at line {line_number}")
        rows += 1
        if rows > CAP:
            raise ValueError("decision trace exceeds its configured cap")
    if rows == 0:
        raise ValueError("decision trace contains no requested instruction starts")
    if rows == CAP:
        raise ValueError("decision trace reached its configured row cap")
    return {"rows": rows, "pcs": len(pcs)}


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: verify_theron_syscard_decision_trace.py TRACE", file=sys.stderr)
        return 2
    try:
        result = verify_lines(Path(sys.argv[1]).read_text(encoding="ascii").splitlines())
    except (OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(f"PASS: System Card decision rows={result['rows']} distinct_pcs={result['pcs']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
