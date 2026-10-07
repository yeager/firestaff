#!/usr/bin/env python3
"""Inventory authentic Track 02 copies of the runtime-sampled $489f window."""

from __future__ import annotations

import hashlib
import pathlib
import sys


RUNTIME_WINDOW = bytes.fromhex("1600041c0280650000a90820fb")
RAW_SECTOR_BYTES = 2352
MODE1_USER_BYTES = 2048
MODE1_USER_DATA_OFFSET = 16
TARGET_ENTRY_DELTA = 0x44FB - 0x489F
TARGET_ENTRY_SPAN_BYTES = 33
RUNTIME_CONTEXT_PREFIX_BYTES = 64
RUNTIME_CONTEXT_SUFFIX_BYTES = 64
RUNTIME_CONTEXT_BYTES = (
    RUNTIME_CONTEXT_PREFIX_BYTES + len(RUNTIME_WINDOW) +
    RUNTIME_CONTEXT_SUFFIX_BYTES
)
RUNTIME_CALL_WINDOW = bytes.fromhex("a90820fb444c20b9")
RUNTIME_CONTEXT_SHA256 = (
    "1f6df3c02976c33d01f8e15cd088ce186e46a7b7405c3bc6ef865c2cd2a94b10"
)
MAME_LISTING_SHA256 = (
    "526a0e4061e81a18786ac3cccb15879b5dfc25ec0f03e697c266e6e0c980e955"
)
MAME_LISTING_PATH = (
    pathlib.Path(__file__).resolve().parents[1] / "docs" / "source-lock" /
    "theron-disassembly" /
    "theron-jp-3879-runtime-window-candidate-20261007.asm"
)
TARGET_LISTING_SHA256 = (
    "d05c8c2848469977cbfa23f87de74b916dcda10514d4b9f6d61dae41256eac01"
)
TARGET_LISTING_PATH = (
    pathlib.Path(__file__).resolve().parents[1] / "docs" / "source-lock" /
    "theron-disassembly" /
    "theron-jp-44fb-runtime-target-candidate-20261007.asm"
)
EXPECTED = {
    "US": {
        "name": "TQUS02.bin",
        "size": 8104992,
        "sha256": "f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565",
        "offsets": (),
    },
    "JP": {
        "name": "TQJP02.bin",
        "size": 8102640,
        "sha256": "d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb",
        "offsets": (611695, 912751, 1213807, 1514863, 1815919, 2116975),
        "target_offsets": (610459, 911515, 1212571, 1513627, 1814683, 2115739),
    },
}


def find_offsets(raw: bytes, needle: bytes) -> tuple[int, ...]:
    offsets = []
    offset = 0
    while True:
        offset = raw.find(needle, offset)
        if offset < 0:
            return tuple(offsets)
        offsets.append(offset)
        offset += 1


def verify_recorded_listing() -> None:
    try:
        lines = MAME_LISTING_PATH.read_text(encoding="utf-8").splitlines(
            keepends=True
        )
    except OSError as error:
        raise ValueError(f"cannot read recorded MAME listing: {error}") from error

    try:
        start = lines.index("; BEGIN MAME OUTPUT\n") + 1
        end = lines.index("; END MAME OUTPUT\n")
    except ValueError as error:
        raise ValueError("recorded MAME listing delimiters are missing") from error

    listing = "".join(lines[start:end]).encode("utf-8")
    if hashlib.sha256(listing).hexdigest() != MAME_LISTING_SHA256:
        raise ValueError("recorded MAME listing digest differs")
    if not any(line.startswith("0048a8: a9 08") for line in lines[start:end]):
        raise ValueError("recorded MAME listing lost the candidate entry")
    print("PASS: recorded MAME 0.285 candidate listing hash and entry")


def verify_recorded_target_listing() -> None:
    try:
        lines = TARGET_LISTING_PATH.read_text(encoding="utf-8").splitlines(
            keepends=True
        )
    except OSError as error:
        raise ValueError(f"cannot read target MAME listing: {error}") from error

    try:
        start = lines.index("; BEGIN MAME OUTPUT\n") + 1
        end = lines.index("; END MAME OUTPUT\n")
    except ValueError as error:
        raise ValueError("target MAME listing delimiters are missing") from error

    listing = "".join(lines[start:end]).encode("utf-8")
    if hashlib.sha256(listing).hexdigest() != TARGET_LISTING_SHA256:
        raise ValueError("recorded $44fb MAME listing digest differs")
    if not lines[start].startswith("0044fb: 08"):
        raise ValueError("recorded $44fb MAME listing lost its entry")
    print("PASS: recorded MAME 0.285 $44fb candidate listing hash and entry")


def raw_offset_for_target(raw_candidate_offset: int) -> int:
    sector, raw_in_sector = divmod(raw_candidate_offset, RAW_SECTOR_BYTES)
    user_byte = raw_in_sector - MODE1_USER_DATA_OFFSET
    if user_byte < 0 or user_byte >= MODE1_USER_BYTES:
        raise ValueError("candidate offset is outside MODE1 user data")
    user_position = sector * MODE1_USER_BYTES + user_byte
    target_user_position = user_position + TARGET_ENTRY_DELTA
    if target_user_position < 0:
        raise ValueError("target entry precedes MODE1 user data")
    target_sector, target_user_byte = divmod(
        target_user_position, MODE1_USER_BYTES
    )
    return (
        target_sector * RAW_SECTOR_BYTES + MODE1_USER_DATA_OFFSET +
        target_user_byte
    )


def verify_edition(edition: str, path: pathlib.Path) -> bool:
    expected = EXPECTED[edition]
    if not path.is_file():
        print(f"SKIP: authentic {edition} Track 02 unavailable: {path}")
        return False

    observed_hashes = []
    for pass_number in range(1, 4):
        if path.stat().st_size != expected["size"]:
            raise ValueError(f"{edition} Track 02 size mismatch")
        with path.open("rb") as media:
            raw = media.read(expected["size"] + 1)
        if len(raw) != expected["size"]:
            raise ValueError(f"{edition} Track 02 bounded read changed size")
        digest = hashlib.sha256(raw).hexdigest()
        if digest != expected["sha256"]:
            raise ValueError(f"{edition} Track 02 SHA-256 mismatch: {digest}")

        offsets = find_offsets(raw, RUNTIME_WINDOW)
        if offsets != expected["offsets"]:
            raise ValueError(
                f"{edition} runtime-window offsets differ: {offsets}"
            )
        if any(offset % RAW_SECTOR_BYTES != 175 for offset in offsets):
            raise ValueError(f"{edition} runtime-window sector positions differ")

        contexts = []
        target_spans = []
        target_offsets = ()
        if edition == "JP":
            target_offsets = tuple(
                raw_offset_for_target(offset) for offset in offsets
            )
            if target_offsets != expected["target_offsets"]:
                raise ValueError(
                    f"{edition} target offsets differ: {target_offsets}"
                )
        for offset in offsets:
            context_start = offset - RUNTIME_CONTEXT_PREFIX_BYTES
            context_end = (
                offset + len(RUNTIME_WINDOW) + RUNTIME_CONTEXT_SUFFIX_BYTES
            )
            if context_start < 0 or context_end > len(raw):
                raise ValueError(f"{edition} runtime context is out of bounds")
            context = raw[context_start:context_end]
            if len(context) != RUNTIME_CONTEXT_BYTES:
                raise ValueError(f"{edition} runtime context has wrong length")
            if hashlib.sha256(context).hexdigest() != RUNTIME_CONTEXT_SHA256:
                raise ValueError(f"{edition} runtime context digest differs")
            call_start = RUNTIME_CONTEXT_PREFIX_BYTES + 9
            call_end = call_start + len(RUNTIME_CALL_WINDOW)
            if context[call_start:call_end] != RUNTIME_CALL_WINDOW:
                raise ValueError(f"{edition} runtime call window differs")
            contexts.append(context)
            if edition == "JP":
                target_offset = raw_offset_for_target(offset)
                target_end = target_offset + TARGET_ENTRY_SPAN_BYTES
                if target_end > len(raw):
                    raise ValueError(f"{edition} target span is out of bounds")
                target_span = raw[target_offset:target_end]
                if len(target_span) != TARGET_ENTRY_SPAN_BYTES:
                    raise ValueError(f"{edition} target span has wrong length")
                if hashlib.sha256(target_span).hexdigest() != (
                    "30c4e29752ff4fc5363876072d67f7b1f8e68f96af208064f0ee0e58fc62463d"
                ):
                    raise ValueError(f"{edition} target span digest differs")
                target_spans.append(target_span)
        if contexts and len(set(contexts)) != 1:
            raise ValueError(f"{edition} candidate contexts are not identical")
        if edition == "JP":
            if len(set(target_spans)) != 1:
                raise ValueError(f"{edition} $44fb target spans are not identical")

        observed_hashes.append(digest)
        print(f"PASS: {edition} authentic runtime-window scan {pass_number}")

    if len(set(observed_hashes)) != 1:
        raise ValueError(f"{edition} Track 02 changed between passes")
    return True


def main() -> int:
    verify_recorded_listing()
    verify_recorded_target_listing()
    data_root = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else (
        pathlib.Path.home() / ".firestaff" / "data"
    )
    theron_root = data_root / "theron"
    us_present = verify_edition("US", theron_root / EXPECTED["US"]["name"])
    jp_present = verify_edition("JP", theron_root / EXPECTED["JP"]["name"])
    if not us_present or not jp_present:
        return 77
    print(
        "PASS: runtime window is absent from authentic US Track 02 and has "
        "six identical JP candidate contexts, including the runtime call window"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
