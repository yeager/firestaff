#!/usr/bin/env python3
"""Source-lock the authentic JP/US ID $53 continuation at logical $4C71."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
LISTING_PATH = (
    ROOT
    / "docs/source-lock/theron-disassembly/theron-jp-us-id53-post-4b00-4c71-20261010.asm"
)
START_PC = 0x4C71
WINDOW_LENGTH = 112
WINDOW_SHA256 = "e656afc5e5dcbd8c22d0763699e586f80785fc83ba17c0cfe94923a6fcf38d86"
REGIONS = {
    "JP": {
        "filename": "TQJP02.bin",
        "md5": "b7afb338ad31be1025b53f9aff12d73a",
        "sha256": "d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb",
        "size": 8_102_640,
        "offset": 0x2BF201,
    },
    "US": {
        "filename": "TQUS02.bin",
        "md5": "f23601102138f87c33025877767ebf76",
        "sha256": "f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565",
        "size": 8_104_992,
        "offset": 0x2BFB31,
    },
}
ROW = re.compile(r"^([0-9a-f]{6}):\s*(.*?)\s{2,}.+$")
EXPECTED_RETURN = bytes.fromhex("60")


def listing_bytes(path: Path) -> tuple[int, bytes]:
    decoded = bytearray()
    pc = START_PC
    rows = 0
    for line_number, line in enumerate(path.read_text(encoding="ascii").splitlines(), start=1):
        if not line or line.startswith(";"):
            continue
        match = ROW.fullmatch(line)
        if match is None:
            raise AssertionError(f"malformed MAME listing row at line {line_number}")
        address = int(match.group(1), 16)
        tokens = match.group(2).split()
        if not tokens or any(not re.fullmatch(r"[0-9a-f]{2}", token) for token in tokens):
            raise AssertionError(f"invalid MAME byte column at line {line_number}")
        if address != pc:
            raise AssertionError(
                f"listing address gap at line {line_number}: expected {pc:#06x}, got {address:#06x}"
            )
        decoded.extend(bytes.fromhex("".join(tokens)))
        pc += len(tokens)
        rows += 1
    if not rows or len(decoded) != WINDOW_LENGTH:
        raise AssertionError(f"expected {WINDOW_LENGTH} decoded bytes, got {len(decoded)}")
    return pc, bytes(decoded)


def main() -> int:
    media_root = Path(
        os.environ.get("FIRESTAFF_THERON_TEST_DATA_DIR", Path.home() / ".firestaff/data/theron")
    )
    media_paths = {region: media_root / item["filename"] for region, item in REGIONS.items()}
    if any(not path.is_file() for path in media_paths.values()):
        print("SKIP: authentic JP and US Theron Track 02 media is unavailable")
        return 77

    end_pc, decoded_listing = listing_bytes(LISTING_PATH)
    if end_pc != START_PC + WINDOW_LENGTH or decoded_listing[-1:] != EXPECTED_RETURN:
        raise AssertionError("rooted listing must end at the RTS opcode at $4CE0")

    windows: dict[str, bytes] = {}
    for region, metadata in REGIONS.items():
        path = media_paths[region]
        if path.stat().st_size != metadata["size"]:
            raise AssertionError(f"{metadata['filename']}: unexpected media length")
        image = path.read_bytes()
        actual_md5 = hashlib.md5(image).hexdigest()
        actual_sha256 = hashlib.sha256(image).hexdigest()
        if actual_md5 != metadata["md5"] or actual_sha256 != metadata["sha256"]:
            raise AssertionError(f"{metadata['filename']}: authentic media identity mismatch")

        offset = metadata["offset"]
        window = image[offset : offset + WINDOW_LENGTH]
        if len(window) != WINDOW_LENGTH or hashlib.sha256(window).hexdigest() != WINDOW_SHA256:
            raise AssertionError(f"{region} candidate window hash/length mismatch at {offset:#x}")
        if image.find(window) != offset or image.find(window, offset + 1) != -1:
            raise AssertionError(f"{region} candidate window is not unique at {offset:#x}")
        for mutation_offset in (0, WINDOW_LENGTH - 1):
            mutated = bytearray(window)
            mutated[mutation_offset] ^= 0xFF
            if hashlib.sha256(mutated).hexdigest() == WINDOW_SHA256:
                raise AssertionError(
                    f"{region} candidate mutation survived at {mutation_offset:#x}"
                )
        windows[region] = window
        print(f"PASS: authentic {region} Track 02 window at raw offset {offset:#x}")

    if windows["JP"] != windows["US"] or decoded_listing != windows["JP"]:
        raise AssertionError("regional source windows and rooted MAME listing must match exactly")

    print("PASS: unique 112-byte JP/US window and rooted HuC6280 listing match through RTS $4CE0")
    print(
        "LIMIT: static media identity only; runtime source bank and gameplay semantics remain open"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
