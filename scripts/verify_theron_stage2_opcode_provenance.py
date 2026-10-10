#!/usr/bin/env python3
"""Check consistency of a claimed PCE Fast source-to-opcode trace.

This validates trace structure and byte mappings against authentic JP media.
It does not authenticate who produced the trace or prove that an emulator ran.
The runtime producer is not yet implemented; a passing trace is not runtime,
opcode-origin, or gameplay evidence. Synthetic rows are contract-test input
only and must never be reported as an emulator capture.
"""

from __future__ import annotations

import argparse
import hashlib
import re
import sys
from pathlib import Path


MEDIA_MD5 = {"JP": "b7afb338ad31be1025b53f9aff12d73a"}
MEDIA_SIZE = {"JP": 8_102_640}
TRACK02_START_LBA = {"JP": 3_590}
TRACK02_INDEX1_OFFSET = {"JP": 526_848}
TRACK02_END_LBA = {"JP": 6_961}
SECTOR_BYTES = 2_352
USER_DATA_RAW_OFFSET = 16
STAGE2_LBA = {"JP": 4_521}
STAGE2_USER_OFFSET = {"JP": 2}
STAGE2_PC = 0x4002
STAGE2_BYTES = 211
STAGE2_SHA256 = "3d16abea96bf6ce9610c7b3c9bc827f73e67f922dcda63e1c8681c7e3cd71a96"
PHYSICAL_STAGE2_BASE = 0x100002
TRACE_HEADER = "format=mednafen-pce-fast-opcode-origin-v1"
ROW_PREFIX = "opcode_origin_fetch"
ROW_RE = re.compile(r"[0-9a-f]{2}\Z")


def parse_fields(line: str, line_number: int) -> dict[str, str]:
    if not line.startswith(ROW_PREFIX + " "):
        raise ValueError(f"unexpected trace row at line {line_number}")
    result: dict[str, str] = {}
    for item in line[len(ROW_PREFIX) + 1 :].split():
        key, separator, value = item.partition("=")
        if not separator or not key or not value:
            raise ValueError(f"malformed field at line {line_number}")
        if key in result:
            raise ValueError(f"duplicate field {key!r} at line {line_number}")
        result[key] = value
    return result


def required_int(row: dict[str, str], key: str, base: int = 10) -> int:
    try:
        return int(row[key], base)
    except (KeyError, ValueError) as error:
        raise ValueError(f"missing or invalid {key}") from error


def verify(trace_path: Path, media_path: Path, edition: str) -> bytes:
    if edition not in MEDIA_MD5:
        raise ValueError("only JP has an authenticated capture-to-file offset contract")
    raw = media_path.read_bytes()
    if len(raw) != MEDIA_SIZE[edition] or hashlib.md5(raw).hexdigest() != MEDIA_MD5[edition]:
        raise ValueError(f"Track 02 is not the authenticated {edition} raw BIN")

    start_lba = STAGE2_LBA[edition]
    user_start = STAGE2_USER_OFFSET[edition]
    data = raw_source_span(raw, edition, start_lba, user_start, STAGE2_BYTES)
    if hashlib.sha256(data).hexdigest() != STAGE2_SHA256:
        raise ValueError("candidate source bytes differ from the locked JP/US static span")
    lines = trace_path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != TRACE_HEADER:
        raise ValueError("trace lacks the expected format header")

    observed = bytearray()
    origin_keys: set[tuple[int, int, int]] = set()
    previous_fetch_sequence = None
    previous_generation = None
    for line_number, line in enumerate(lines[1:], 2):
        row = parse_fields(line, line_number)
        fetch_sequence = required_int(row, "fetch_sequence")
        generation = required_int(row, "generation")
        pc = required_int(row, "logical_pc", 16)
        physical_pc = required_int(row, "physical_pc", 16)
        mpr_slot = required_int(row, "mpr_slot")
        mpr_bank = required_int(row, "mpr_bank", 16)
        opcode = row.get("opcode", "")
        if not ROW_RE.fullmatch(opcode):
            raise ValueError(f"invalid opcode byte at line {line_number}")
        lba = required_int(row, "source_lba")
        user_offset = required_int(row, "source_user_offset")
        raw_offset = required_int(row, "source_raw_offset")
        source_value = required_int(row, "source_value", 16)

        index = len(observed)
        if index >= STAGE2_BYTES:
            raise ValueError("trace has extra rows after the bounded source window")
        expected_pc = STAGE2_PC + index
        expected_physical_pc = PHYSICAL_STAGE2_BASE + index
        expected_user_offset = user_start + index
        expected_raw_offset = USER_DATA_RAW_OFFSET + expected_user_offset % 2_048
        expected_lba = start_lba + expected_user_offset // 2_048
        expected_mpr_slot = (expected_pc >> 13) & 7
        if (fetch_sequence != index or pc != expected_pc or
                physical_pc != expected_physical_pc or
                mpr_slot != expected_mpr_slot or mpr_bank != 0x80):
            raise ValueError(f"non-contiguous or incorrectly mapped opcode fetch at line {line_number}")
        if (lba != expected_lba or user_offset != expected_user_offset or
                raw_offset != expected_raw_offset or source_value != int(opcode, 16)):
            raise ValueError(f"source origin does not match fetched byte at line {line_number}")
        raw_file_offset = (TRACK02_INDEX1_OFFSET[edition] +
                           (lba - TRACK02_START_LBA[edition]) * SECTOR_BYTES + raw_offset)
        if not (TRACK02_START_LBA[edition] <= lba < TRACK02_END_LBA[edition]) or \
                raw_file_offset >= len(raw) or raw[raw_file_offset] != source_value:
            raise ValueError(f"origin byte differs from authenticated media at line {line_number}")
        origin_key = (generation, lba, user_offset)
        if origin_key in origin_keys:
            raise ValueError(f"duplicate source-byte origin at line {line_number}")
        origin_keys.add(origin_key)
        if previous_fetch_sequence is not None and fetch_sequence != previous_fetch_sequence + 1:
            raise ValueError("fetch sequence is discontinuous")
        if previous_generation is not None and generation != previous_generation:
            raise ValueError("capture generation changed inside the source window")
        previous_fetch_sequence = fetch_sequence
        previous_generation = generation
        observed.append(int(opcode, 16))

    if len(observed) != STAGE2_BYTES:
        raise ValueError(f"expected {STAGE2_BYTES} consecutive origin-tagged opcode fetches, found {len(observed)}")
    if bytes(observed) != data:
        raise ValueError("runtime opcode bytes differ from the authenticated Track 02 candidate")
    return bytes(observed)


def raw_source_span(raw: bytes, edition: str, start_lba: int,
                    user_start: int, length: int) -> bytes:
    result = bytearray()
    for index in range(length):
        user_offset = user_start + index
        lba = start_lba + user_offset // 2_048
        raw_offset = USER_DATA_RAW_OFFSET + user_offset % 2_048
        file_offset = (TRACK02_INDEX1_OFFSET[edition] +
                       (lba - TRACK02_START_LBA[edition]) * SECTOR_BYTES + raw_offset)
        result.append(raw[file_offset])
    return bytes(result)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--edition", required=True, choices=("JP",))
    parser.add_argument("trace", type=Path)
    parser.add_argument("track02", type=Path)
    args = parser.parse_args()
    try:
        span = verify(args.trace, args.track02, args.edition)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(f"PASS: trace is structurally consistent with {len(span)} consecutive bytes in {args.edition} Track 02")
    print(f"media_candidate_sha256={hashlib.sha256(span).hexdigest()}")
    print("LIMIT: trace provenance is unauthenticated; this does not prove emulator execution, opcode fetches, or gameplay")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
