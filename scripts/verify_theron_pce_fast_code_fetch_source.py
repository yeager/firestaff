#!/usr/bin/env python3
"""Match bounded PCE Fast instruction windows against authentic US Track 02."""

from __future__ import annotations

import hashlib
import re
import sys
from pathlib import Path


US_TRACK02_MD5 = "f23601102138f87c33025877767ebf76"
WINDOW_BYTES = 512
ROW_RE = re.compile(r"^(row_reader_fetch|c852_fetch)\s+(.*)$")
FIELD_RE = re.compile(r"([a-z0-9_]+)=([^ ]+)")
TARGETS = {0xCBC6, 0x45C3}


def fail(message: str) -> int:
    print(f"FAIL: {message}", file=sys.stderr)
    return 2


def source_offsets(source: bytes, window: bytes) -> list[int]:
    offsets: list[int] = []
    offset = source.find(window)
    while offset >= 0:
        offsets.append(offset)
        offset = source.find(window, offset + 1)
    return offsets


def main() -> int:
    if len(sys.argv) != 3:
        return fail(
            "usage: verify_theron_pce_fast_code_fetch_source.py TRACE US_TRACK02_BIN"
        )

    trace_path = Path(sys.argv[1])
    source_path = Path(sys.argv[2])
    try:
        trace_text = trace_path.read_text(encoding="ascii")
        source = source_path.read_bytes()
    except OSError as exc:
        return fail(f"cannot read input: {exc}")
    except UnicodeError:
        return fail("trace is not ASCII")

    actual_md5 = hashlib.md5(source).hexdigest()
    if actual_md5 != US_TRACK02_MD5:
        return fail(
            f"Track 02 identity mismatch: expected {US_TRACK02_MD5}, got {actual_md5}"
        )

    header = (
        "source=mednafen-pce-fast-c852-opcode-fetch "
        "event_sequence_source=TheronTraceCDSharedEventSequence"
    )
    if header not in trace_text.splitlines():
        return fail("missing recognized PCE Fast instruction-fetch trace header")

    truncated_sites: set[int] = set()
    matches: dict[int, set[int]] = {pc: set() for pc in TARGETS}
    rows = 0
    for line_number, line in enumerate(trace_text.splitlines(), 1):
        if line.startswith("row_reader_fetch_truncated "):
            fields = dict(FIELD_RE.findall(line))
            try:
                truncated_sites.add(int(fields["logical_pc"], 16))
            except (KeyError, ValueError):
                return fail(f"malformed truncation marker on line {line_number}")
            continue
        match = ROW_RE.match(line)
        if not match or match.group(1) != "row_reader_fetch":
            continue
        fields = dict(FIELD_RE.findall(match.group(2)))
        try:
            pc = int(fields["logical_pc"], 16)
            physical_pc = int(fields["physical_pc"], 16)
            opcode = int(fields["opcode"], 16)
            event_sequence = int(fields["event_sequence"], 10)
            mprs = [int(fields[f"mpr{index}"], 16) for index in range(8)]
            window = bytes.fromhex(fields["code_bytes"])
        except (KeyError, ValueError):
            return fail(f"malformed instruction row on line {line_number}")
        if pc not in TARGETS:
            return fail(f"unexpected reader code PC ${pc:04X} on line {line_number}")
        if len(window) != WINDOW_BYTES:
            return fail(f"instruction window is not {WINDOW_BYTES} bytes on line {line_number}")
        if window[0] != opcode:
            return fail(f"opcode/window mismatch on line {line_number}")
        expected_physical_pc = (mprs[pc >> 13] << 13) | (pc & 0x1FFF)
        if any(value > 0xFF for value in mprs):
            return fail(f"invalid MPR value on line {line_number}")
        if physical_pc != expected_physical_pc:
            return fail(f"logical/physical PC disagrees with MPRs on line {line_number}")
        if event_sequence <= 0 or physical_pc > 0xFFFFFF:
            return fail(f"invalid execution provenance on line {line_number}")
        rows += 1
        matches[pc].update(source_offsets(source, window))

    if rows == 0:
        return fail("no Drator row-reader instruction windows were recorded")

    print(f"PASS: rows={rows} source=US-Track-02 md5={actual_md5}")
    for pc in sorted(TARGETS):
        offsets = sorted(matches[pc])
        state = "truncated" if pc in truncated_sites else "bounded"
        rendered = ",".join(f"0x{offset:07x}" for offset in offsets) or "none"
        print(f"pc=${pc:04X} status={state} matching_raw_offsets={rendered}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
