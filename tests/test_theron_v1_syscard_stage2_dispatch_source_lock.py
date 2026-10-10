#!/usr/bin/env python3
"""Lock optional System Card 3.0 vectors used by the static stage-2 listing."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import sys


BIOS_SIZE = 262_656
BIOS_MD5 = "ff1a674273fe3540ccef576376407d1d"
BIOS_SHA256 = "df4f75feebb95e53dfef72dea0787df743e3474ba046ff5a4cb88e34dda93ff1"
COPIER_HEADER_BYTES = 512
BIOS_BASE = 0xE000
E009_VECTOR = bytes.fromhex("4c05ec")
E00F_VECTOR = bytes.fromhex("4ceceb")
E009_CORE_PREFIX = bytes.fromhex(
    "9c73222009f320eef0a9088d4c222004f1a204a0012027f3"
    "a5ffc902900ec9fff052c9fed0034cbbec4c03eda5fff00e"
    "a5f88d80220a0a0a85f964f88003209f"
)
E00F_TRAMPOLINE = bytes.fromhex(
    "a5fa8d8222a5fb8d8322440dc900f0034cf3e0a2ff9a6c8222"
)
NOTE_PATH = (
    Path(__file__).resolve().parents[1]
    / "docs/source-lock/theron-syscard-stage2-callback-dispatch-20261010.md"
)


def address_to_file_offset(address: int) -> int:
    return COPIER_HEADER_BYTES + address - BIOS_BASE


def main() -> int:
    note = " ".join(NOTE_PATH.read_text(encoding="utf-8").split())
    for fact in (
        "The vector at logical `$E009` (file offset `0x209`) is `JMP $EC05`.",
        "The vector at `$E00F` (file offset `0x20f`) is `JMP $EBEC`.",
        "The static stage-2 routine at `$4080` sets `$FA=0`, `$FB=$40`",
        "same two bytes `64 00` there (`STZ $00`)",
        "no runtime BIOS dependency",
    ):
        if fact not in note:
            raise AssertionError(f"System Card source note is missing fact: {fact}")

    image_path = Path(
        os.environ.get(
            "FIRESTAFF_THERON_TEST_SYSCARD_PATH",
            Path.home() / ".mednafen/firmware/syscard3.pce",
        )
    )
    if not image_path.is_file():
        print("SKIP: optional authentic System Card 3.0 image is unavailable")
        return 77

    with image_path.open("rb") as image_file:
        image = image_file.read(BIOS_SIZE + 1)
    if len(image) != BIOS_SIZE:
        raise AssertionError(f"expected {BIOS_SIZE} bytes, got {len(image)}")
    if hashlib.md5(image).hexdigest() != BIOS_MD5:
        raise AssertionError("System Card 3.0 MD5 does not match the source lock")
    if hashlib.sha256(image).hexdigest() != BIOS_SHA256:
        raise AssertionError("System Card 3.0 SHA-256 does not match the source lock")

    vectors = {
        0xE009: E009_VECTOR,
        0xE00F: E00F_VECTOR,
    }
    for address, expected in vectors.items():
        offset = address_to_file_offset(address)
        if image[offset : offset + len(expected)] != expected:
            raise AssertionError(f"System Card vector at {address:#06x} changed")

    e009_offset = address_to_file_offset(0xEC05)
    if image[e009_offset : e009_offset + len(E009_CORE_PREFIX)] != E009_CORE_PREFIX:
        raise AssertionError("System Card $E009 core entry at $EC05 changed")
    e00f_offset = address_to_file_offset(0xEBEC)
    if image[e00f_offset : e00f_offset + len(E00F_TRAMPOLINE)] != E00F_TRAMPOLINE:
        raise AssertionError("System Card $E00F trampoline at $EBEC changed")

    print("PASS: authentic System Card 3.0 identity and $E009/$E00F vectors")
    print("PASS: $E009 core branch and $E00F callback trampoline byte windows")
    print("LIMIT: reference-only BIOS source lock; no runtime BIOS dependency is added")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
