#!/usr/bin/env python3
"""Stream-validate bounded System Card I/O/RAM events against the authentic BIOS."""

from __future__ import annotations

import hashlib
import re
import sys
from pathlib import Path


CAP = 8388608
SYSTEM_CARD_MD5 = "ff1a674273fe3540ccef576376407d1d"
PCE_HEADER_BYTES = 512
BIOS_LOGICAL_BASE = 0xC000
RANGES = ((0x1800, 0x1803), (0x224C, 0x2255), (0x2273, 0x2280), (0x22A4, 0x22A4))
ADDRESSES = {address for start, end in RANGES for address in range(start, end + 1)}
HEADER = (
    "source=mednafen-pce-fast-system-card-io-ram-v1 "
    "event_sequence_source=TheronTraceCDSharedEventSequence limit=8388608 "
    "logical_ranges=1800-1803,224c-2255,2273-2280,22a4"
)
FIELDS = (
    "access", "index", "event_sequence", "logical_address", "physical_address",
    "access_mpr", "value", "logical_pc", "physical_pc", "code_bytes",
    "code_physical_bytes", "code_valid_mask", "pc_mpr",
    *(f"mpr{i}" for i in range(8)),
)
HEX_WIDTHS = {
    "logical_address": 4, "physical_address": 6, "access_mpr": 2,
    "value": 2, "logical_pc": 4, "physical_pc": 6, "code_bytes": 8,
    "code_valid_mask": 1, "pc_mpr": 2,
    "code_physical_bytes": None,
    **{f"mpr{i}": 2 for i in range(8)},
}


def in_scope(address: int) -> bool:
    return any(start <= address <= end for start, end in RANGES)


def verify_lines(lines, system_card_image: bytes) -> dict[str, int]:
    iterator = iter(lines)
    try:
        header = next(iterator)
    except StopIteration:
        raise ValueError("unexpected System Card I/O/RAM trace header") from None
    if header.rstrip("\r\n") != HEADER:
        raise ValueError("unexpected System Card I/O/RAM trace header")
    if len(system_card_image) <= PCE_HEADER_BYTES:
        raise ValueError("System Card image is shorter than its PCE header")
    rows = reads = writes = 0
    previous_event = -1
    truncated = False
    observed_addresses: set[int] = set()
    for line_number, raw_line in enumerate(iterator, start=2):
        line = raw_line.rstrip("\r\n")
        if line == f"io_ram_truncated cap={CAP}":
            if truncated or rows != CAP:
                raise ValueError("I/O/RAM truncation marker disagrees with row count")
            truncated = True
            continue
        if truncated or not line.startswith("io_ram "):
            raise ValueError(f"malformed or out-of-order I/O/RAM row at line {line_number}")
        values: dict[str, str] = {}
        for token in line.removeprefix("io_ram ").split():
            key, separator, value = token.partition("=")
            if not separator or key in values:
                raise ValueError(f"malformed I/O/RAM field at line {line_number}")
            values[key] = value
        if tuple(values) != FIELDS:
            raise ValueError(f"unexpected I/O/RAM fields at line {line_number}")
        if values["access"] not in ("read", "write"):
            raise ValueError(f"invalid access kind at line {line_number}")
        for key, width in HEX_WIDTHS.items():
            if key == "code_physical_bytes":
                if not re.fullmatch(r"[0-9a-f]{6}(?:,[0-9a-f]{6}){3}", values[key]):
                    raise ValueError(f"invalid {key} encoding at line {line_number}")
            elif not re.fullmatch(rf"[0-9a-f]{{{width}}}", values[key]):
                raise ValueError(f"invalid {key} encoding at line {line_number}")
        for key in ("index", "event_sequence"):
            if not re.fullmatch(r"(?:0|[1-9][0-9]*)", values[key]):
                raise ValueError(f"invalid {key} at line {line_number}")
        if int(values["index"]) != rows:
            raise ValueError(f"non-contiguous row index at line {line_number}")
        event = int(values["event_sequence"])
        if event <= previous_event:
            raise ValueError(f"non-increasing shared event sequence at line {line_number}")
        previous_event = event

        address = int(values["logical_address"], 16)
        if not in_scope(address):
            raise ValueError(f"out-of-scope logical address at line {line_number}")
        mprs = [int(values[f"mpr{i}"], 16) for i in range(8)]
        access_mpr = int(values["access_mpr"], 16)
        if access_mpr != mprs[address >> 13]:
            raise ValueError(f"data MPR disagrees with logical address at line {line_number}")
        expected_address = (access_mpr << 13) | (address & 0x1FFF)
        if int(values["physical_address"], 16) != expected_address:
            raise ValueError(f"logical/physical data mapping mismatch at line {line_number}")

        pc = int(values["logical_pc"], 16)
        # System Card code can execute through either half of the HuC6280's
        # top 16 KiB window. In particular, callbacks at $C000-$DFFF are
        # backed by the physical bank selected in MPR6 (commonly bank $01),
        # while $E000-$FFFF is backed by MPR7. Validate source bytes through
        # the captured physical mapping below instead of assuming all BIOS
        # instructions live at logical $E000+.
        if not BIOS_LOGICAL_BASE <= pc <= 0xFFFF or mprs[7] != 0:
            raise ValueError(f"event is not mapped to the System Card BIOS at line {line_number}")
        pc_slot = pc >> 13
        if int(values["pc_mpr"], 16) != mprs[pc_slot]:
            raise ValueError(f"PC MPR disagrees with logical PC at line {line_number}")
        expected_pc = (mprs[pc_slot] << 13) | (pc & 0x1FFF)
        if int(values["physical_pc"], 16) != expected_pc:
            raise ValueError(f"logical/physical PC mapping mismatch at line {line_number}")

        code_bytes = bytes.fromhex(values["code_bytes"])
        code_physical = [
            int(item, 16) for item in values["code_physical_bytes"].split(",")
        ]
        valid_mask = int(values["code_valid_mask"], 16)
        if not valid_mask & 1:
            raise ValueError(f"opcode byte is not mapped at line {line_number}")
        for offset, physical in enumerate(code_physical):
            code_logical = (pc + offset) & 0xFFFF
            slot = code_logical >> 13
            expected = (mprs[slot] << 13) | (code_logical & 0x1FFF)
            if valid_mask & (1 << offset):
                if physical != expected:
                    raise ValueError(f"instruction-byte mapping mismatch at line {line_number}")
                if BIOS_LOGICAL_BASE <= code_logical <= 0xFFFF:
                    image_offset = PCE_HEADER_BYTES + physical
                    if image_offset >= len(system_card_image):
                        raise ValueError("System Card image does not cover the BIOS PC")
                    if code_bytes[offset] != system_card_image[image_offset]:
                        raise ValueError(f"instruction byte disagrees with authentic BIOS at line {line_number}")
            elif physical != 0:
                raise ValueError(f"unmapped instruction byte has a physical address at line {line_number}")

        observed_addresses.add(address)
        if values["access"] == "read":
            reads += 1
        else:
            writes += 1
        rows += 1
        if rows > CAP:
            raise ValueError("I/O/RAM trace exceeds its configured cap")
    if rows == 0:
        raise ValueError("I/O/RAM trace contains no scoped System Card accesses")
    if truncated:
        raise ValueError("I/O/RAM trace reached its configured row cap")
    return {
        "rows": rows,
        "reads": reads,
        "writes": writes,
        "addresses": len(observed_addresses),
        "missing_addresses": len(ADDRESSES - observed_addresses),
    }


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: verify_theron_syscard_io_ram_trace.py TRACE SYSTEM_CARD_PCE", file=sys.stderr)
        return 2
    try:
        trace_path = Path(sys.argv[1])
        image_path = Path(sys.argv[2])
        system_card_image = image_path.read_bytes()
        digest = hashlib.md5(system_card_image).hexdigest()
        if digest != SYSTEM_CARD_MD5:
            raise ValueError("System Card image does not match the authenticated BIOS MD5")
        with trace_path.open("r", encoding="ascii", newline="") as trace_file:
            result = verify_lines(trace_file, system_card_image)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(
        "PASS: System Card I/O/RAM "
        f"rows={result['rows']} reads={result['reads']} writes={result['writes']} "
        f"addresses={result['addresses']}/{len(ADDRESSES)} "
        f"missing_addresses={result['missing_addresses']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
