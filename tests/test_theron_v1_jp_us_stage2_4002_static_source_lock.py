#!/usr/bin/env python3
"""Lock the static JP/US Track 02 candidate through sampled cold-boot PC $40d4."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
LISTING_PATH = (
    ROOT
    / "docs/source-lock/theron-disassembly/"
    "theron-jp-us-cold-boot-stage2-4002-candidate-20261010.asm"
)
PARAMETER_NOTE_PATH = (
    ROOT
    / "docs/source-lock/theron-jp-us-stage2-parameter-blocks-20261010.md"
)
WINDOW_LENGTH = 211
PARAMETER_DATA_LENGTH = 14
LISTING_START_PC = 0x4002
CALLBACK_ENTRY_PREFIX = bytes.fromhex("6400")
ALT_ENTRY_PC = 0x40E3
ALT_ENTRY_DELTA = ALT_ENTRY_PC - LISTING_START_PC
ALT_ENTRY_LENGTH = 201
EXPECTED_WINDOW = bytes.fromhex("73002001200f007300200027800062")
EXPECTED_CALL_40E3 = bytes.fromhex("20e340")
EXPECTED_OVERLAPPING_TIA_BYTES = bytes.fromhex("e30302002b0d20")
EXPECTED_PARAMETER_DATA = {
    "JP": bytes.fromhex("00e7031100268500e30302002b0d"),
    "US": bytes.fromhex("00e7031100fc8300e30302002b0d"),
}
REGIONS = {
    "JP": {
        "filename": "TQJP02.bin",
        "md5": "b7afb338ad31be1025b53f9aff12d73a",
        "sha256": "d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb",
        "size": 8_102_640,
        "offset": 0x2973A2,
        "parameter_data_sha256": "1c2e057f0ab72fd57419891c821f32c41c47f835156daa177dd461c57e122e41",
        "span_sha256": "3d16abea96bf6ce9610c7b3c9bc827f73e67f922dcda63e1c8681c7e3cd71a96",
        "alt_entry_sha256": "2c41de75cc88b527a3ed8606c24bfe1cfc4e11f39a781714601890b638e5a573",
        "alt_entry_listing": "theron-jp-cold-boot-stage2-alt-entry-40e3-20261010.asm",
    },
    "US": {
        "filename": "TQUS02.bin",
        "md5": "f23601102138f87c33025877767ebf76",
        "sha256": "f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565",
        "size": 8_104_992,
        "offset": 0x297CD2,
        "parameter_data_sha256": "b74b4fc9fdd5e8cf0d51167ad910c06740721e9ad1de661c273a13ba8e7f21ca",
        "span_sha256": "3d16abea96bf6ce9610c7b3c9bc827f73e67f922dcda63e1c8681c7e3cd71a96",
        "alt_entry_sha256": "fa686ced729a6563d0f4d00c6d9e6bdf2f9c3f547d8a21afbd51ee6c26c0cd05",
        "alt_entry_listing": "theron-us-cold-boot-stage2-alt-entry-40e3-20261010.asm",
    },
}
LISTING_ROW = re.compile(r"^([0-9a-f]{6}):\s*(.*?)\s{2,}[^\s].*$")


def all_offsets(data: bytes, needle: bytes) -> tuple[int, ...]:
    found: list[int] = []
    cursor = 0
    while True:
        cursor = data.find(needle, cursor)
        if cursor < 0:
            return tuple(found)
        found.append(cursor)
        cursor += 1


def listing_bytes(path: Path, start_pc: int, expected_length: int) -> tuple[int, bytes]:
    lines = path.read_text(encoding="ascii").splitlines()
    decoded = bytearray()
    pc = start_pc
    rows = 0
    for line_number, line in enumerate(lines, start=1):
        if not line or line.startswith(";"):
            continue
        match = LISTING_ROW.fullmatch(line)
        if match is None:
            raise AssertionError(f"malformed MAME listing row at line {line_number}")
        address = int(match.group(1), 16)
        byte_tokens = match.group(2).split()
        if not byte_tokens or any(not re.fullmatch(r"[0-9a-f]{2}", token) for token in byte_tokens):
            raise AssertionError(f"invalid MAME byte column at line {line_number}")
        if address != pc:
            raise AssertionError(
                f"listing address gap at line {line_number}: "
                f"expected {pc:#06x}, got {address:#06x}"
            )
        decoded.extend(bytes.fromhex("".join(byte_tokens)))
        pc += len(byte_tokens)
        rows += 1
    if not rows or len(decoded) != expected_length:
        raise AssertionError(
            f"expected a {expected_length}-byte listing in {path.name}, "
            f"found {len(decoded)} bytes"
        )
    return pc, bytes(decoded)


def main() -> int:
    parameter_note = PARAMETER_NOTE_PATH.read_text(encoding="utf-8")
    normalized_parameter_note = " ".join(parameter_note.split())
    for source_fact in (
        "| `$40d5-$40d8` | `00 e7 03 11` | `00 e7 03 11` |",
        "| `$40d9-$40db` | `00 26 85` | `00 fc 83` |",
        "| `$40dc-$40df` | `00 e3 03 02` | `00 e3 03 02` |",
        "| `$40e0-$40e2` | `00 2b 0d` | `00 2b 0d` |",
        "`64 00` at `$4000`, which decode as `STZ $00`.",
        "`$FC=00`, `$FE=e7`, `$FD=03` and `$F8=11`",
        "`$FC=00`, `$FE=e3`, `$FD=03` and `$F8=02`",
    ):
        if source_fact not in normalized_parameter_note:
            raise AssertionError(f"stage-2 parameter note is missing source fact: {source_fact}")

    media_root = Path(
        os.environ.get("FIRESTAFF_THERON_TEST_DATA_DIR", Path.home() / ".firestaff/data/theron")
    )
    media_paths = {
        region: media_root / metadata["filename"]
        for region, metadata in REGIONS.items()
    }
    if any(not path.is_file() for path in media_paths.values()):
        print("SKIP: authentic JP and US Theron Track 02 media is unavailable")
        return 77

    end_pc, decoded = listing_bytes(LISTING_PATH, LISTING_START_PC, WINDOW_LENGTH)
    if end_pc != LISTING_START_PC + WINDOW_LENGTH:
        raise AssertionError("MAME listing does not end at the expected 211-byte boundary")

    spans = {}
    alternate_entries = {}
    for region, metadata in REGIONS.items():
        media_path = media_paths[region]
        actual_size = media_path.stat().st_size
        if actual_size != metadata["size"]:
            raise AssertionError(
                f"{metadata['filename']}: expected {metadata['size']} bytes, "
                f"got {actual_size}"
            )
        with media_path.open("rb") as media_file:
            image = media_file.read(metadata["size"] + 1)
        if len(image) != metadata["size"]:
            raise AssertionError(
                f"{metadata['filename']}: expected bounded read of "
                f"{metadata['size']} bytes, got {len(image)}"
            )
        actual_md5 = hashlib.md5(image).hexdigest()
        actual_sha256 = hashlib.sha256(image).hexdigest()
        if actual_md5 != metadata["md5"] or actual_sha256 != metadata["sha256"]:
            raise AssertionError(
                f"{metadata['filename']}: expected authentic MD5/SHA-256 "
                f"{metadata['md5']}/{metadata['sha256']}, got {actual_md5}/{actual_sha256}"
            )
        offset = metadata["offset"]
        span = image[offset : offset + WINDOW_LENGTH]
        if len(span) != WINDOW_LENGTH:
            raise AssertionError(f"{region} candidate is truncated at {offset:#x}")
        if span[: len(EXPECTED_WINDOW)] != EXPECTED_WINDOW:
            raise AssertionError(f"{region} candidate no longer begins with sampled PC windows")
        if hashlib.sha256(span).hexdigest() != metadata["span_sha256"]:
            raise AssertionError(f"{region} candidate span hash changed at {offset:#x}")
        if all_offsets(image, span) != (offset,):
            raise AssertionError(f"{region} 211-byte candidate is not unique at {offset:#x}")
        callback_offset = offset - len(CALLBACK_ENTRY_PREFIX)
        if callback_offset < 0 or image[callback_offset:offset] != CALLBACK_ENTRY_PREFIX:
            raise AssertionError(f"{region} callback entry bytes at $4000 changed")
        call_offset = 0x404D - LISTING_START_PC
        if decoded[call_offset : call_offset + len(EXPECTED_CALL_40E3)] != EXPECTED_CALL_40E3:
            raise AssertionError("static candidate no longer calls alternate entry $40e3")
        overlap_offset = offset + (0x40DD - LISTING_START_PC)
        overlap_end = overlap_offset + len(EXPECTED_OVERLAPPING_TIA_BYTES)
        if image[overlap_offset:overlap_end] != EXPECTED_OVERLAPPING_TIA_BYTES:
            raise AssertionError(f"{region} overlapping $40dd TIA byte sequence changed")
        alt_offset = offset + ALT_ENTRY_DELTA
        parameter_offset = offset + WINDOW_LENGTH
        parameter_data = image[parameter_offset : parameter_offset + PARAMETER_DATA_LENGTH]
        if parameter_data != EXPECTED_PARAMETER_DATA[region]:
            raise AssertionError(
                f"{region} stage-2 parameter data changed at {parameter_offset:#x}"
            )
        if hashlib.sha256(parameter_data).hexdigest() != metadata["parameter_data_sha256"]:
            raise AssertionError(f"{region} stage-2 parameter-data hash changed")
        if parameter_data[0:4] != bytes.fromhex("00e70311"):
            raise AssertionError(f"{region} first indexed parameter tuple changed")
        if parameter_data[7:11] != bytes.fromhex("00e30302"):
            raise AssertionError(f"{region} second indexed parameter tuple changed")
        alternate_entry = image[alt_offset : alt_offset + ALT_ENTRY_LENGTH]
        if len(alternate_entry) != ALT_ENTRY_LENGTH:
            raise AssertionError(f"{region} alternate entry is truncated at {alt_offset:#x}")
        if hashlib.sha256(alternate_entry).hexdigest() != metadata["alt_entry_sha256"]:
            raise AssertionError(f"{region} alternate-entry hash changed at {alt_offset:#x}")
        alt_path = ROOT / "docs/source-lock/theron-disassembly" / metadata["alt_entry_listing"]
        alt_end_pc, alt_decoded = listing_bytes(alt_path, ALT_ENTRY_PC, ALT_ENTRY_LENGTH)
        if alt_end_pc != ALT_ENTRY_PC + ALT_ENTRY_LENGTH or alt_decoded != alternate_entry:
            raise AssertionError(f"{region} alternate-entry listing differs from authentic media")
        spans[region] = span
        alternate_entries[region] = alternate_entry
        print(f"PASS: authentic {region} stage-2 parameter bytes at raw offset {parameter_offset:#x}")
        print(f"PASS: authentic {region} Track 02 candidate at raw offset {offset:#x}")
        print(f"PASS: {region} alternate entry and listing at raw offset {alt_offset:#x}")

    if spans["JP"] != spans["US"]:
        raise AssertionError("authentic JP and US candidate bytes differ")
    if decoded != spans["JP"]:
        raise AssertionError("MAME listing bytes differ from the authentic candidate span")
    if alternate_entries["JP"][:20] != alternate_entries["US"][:20]:
        raise AssertionError("JP/US alternate entries no longer share the initial 20-byte prefix")
    if alternate_entries["JP"][20:23] == alternate_entries["US"][20:23]:
        raise AssertionError("JP/US alternate entries unexpectedly share the $40f7 call")
    if EXPECTED_PARAMETER_DATA["JP"][0:4] != EXPECTED_PARAMETER_DATA["US"][0:4] or \
       EXPECTED_PARAMETER_DATA["JP"][7:11] != EXPECTED_PARAMETER_DATA["US"][7:11]:
        raise AssertionError("JP/US indexed parameter tuples unexpectedly differ")

    print("PASS: unique 211-byte JP/US candidate and MAME HuC6280 listing match exactly")
    print("PASS: JP/US callback entry prefix at $4000 is byte-identical")
    print("PASS: JP/US $40e3 alternate-entry listings match their distinct authentic spans")
    print("PASS: JP/US $40d5/$40dc indexed parameter tuples are source-locked")
    print(
        "LIMIT: static media/listing correspondence only; runtime source binding, "
        "execution, and semantics remain unproven"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
