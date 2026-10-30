#!/usr/bin/env python3
"""Validate source-observed PCE Fast reads of System Card status register $1800."""

from __future__ import annotations

import re
import sys
from collections import Counter
from pathlib import Path


HEADER = (
    "source=mednafen-pce-fast-system-card-status-read-v1 "
    "event_sequence_source=TheronTraceCDSharedEventSequence "
    "pc=e95e address=1800 limit=65536"
)
CAP = 65536
FIELDS = {
    "index", "event_sequence", "timestamp", "logical_pc", "physical_pc",
    "logical_address", "status", "bsy", "req", "msg", "cd", "io", "db",
}


def verify_lines(lines: list[str]) -> tuple[int, Counter[str]]:
    if not lines or lines[0] != HEADER:
        raise ValueError("unexpected System Card status trace header")
    rows = 0
    previous_event = -1
    truncated = False
    statuses: Counter[str] = Counter()
    for line_number, line in enumerate(lines[1:], start=2):
        if line == f"status_read_truncated cap={CAP}":
            if truncated or rows != CAP:
                raise ValueError("status trace truncation marker disagrees with row count")
            truncated = True
            continue
        if truncated:
            raise ValueError("status row appears after truncation marker")
        if not line.startswith("status_read "):
            raise ValueError(f"malformed status row at line {line_number}")
        values: dict[str, str] = {}
        for item in line.removeprefix("status_read ").split():
            key, separator, value = item.partition("=")
            if not separator or key in values:
                raise ValueError(f"malformed status field at line {line_number}")
            values[key] = value
        if set(values) != FIELDS:
            raise ValueError(f"unexpected status fields at line {line_number}")
        for key in ("index", "event_sequence", "timestamp"):
            if not re.fullmatch(r"(?:0|[1-9][0-9]*)", values[key]):
                raise ValueError(f"malformed {key} at line {line_number}")
        if int(values["index"]) != rows:
            raise ValueError(f"non-contiguous status row index at line {line_number}")
        event = int(values["event_sequence"])
        if event <= previous_event:
            raise ValueError(f"non-increasing shared event sequence at line {line_number}")
        previous_event = event
        if values["logical_pc"] != "e95e" or values["logical_address"] != "1800":
            raise ValueError(f"status row is outside the observed BIOS read at line {line_number}")
        try:
            physical_pc = int(values["physical_pc"], 16)
            status = int(values["status"], 16)
            data_bus = int(values["db"], 16)
            signal_bits = {
                "bsy": 0x80, "req": 0x40, "msg": 0x20, "cd": 0x10, "io": 0x08
            }
            expected_status = 0
            for name, bit in signal_bits.items():
                if values[name] not in ("0", "1"):
                    raise ValueError("signal is not boolean")
                expected_status |= int(values[name]) * bit
        except ValueError as error:
            raise ValueError(f"malformed status value at line {line_number}") from error
        if not 0 <= physical_pc <= 0xFFFFFF or not 0 <= data_bus <= 0xFF or not 0 <= status <= 0xFF:
            raise ValueError(f"status value out of range at line {line_number}")
        if int(values["timestamp"]) > 0xFFFFFFFF or physical_pc != 0x00095E:
            raise ValueError(f"status row has an invalid timestamp or BIOS mapping at line {line_number}")
        if status != expected_status:
            raise ValueError(f"status byte disagrees with sampled SCSI signals at line {line_number}")
        statuses[f"{status:02x}"] += 1
        rows += 1
    if rows == 0:
        raise ValueError("status trace contains no observed $1800 reads")
    return rows, statuses


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: verify_theron_syscard_status_trace.py TRACE", file=sys.stderr)
        return 2
    try:
        rows, statuses = verify_lines(Path(sys.argv[1]).read_text(encoding="ascii").splitlines())
    except (OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    detail = ",".join(f"{key}:{statuses[key]}" for key in sorted(statuses))
    print(f"PASS: System Card status rows={rows} values={detail}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
