#!/usr/bin/env python3
"""Source-lock the authentic JP/US ID $4E callee rooted at logical $4CE1."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
LISTING_PATH = (
    ROOT / "docs/source-lock/theron-disassembly/theron-jp-us-id4e-callee-4ce1-20261010.asm"
)
START_PC = 0x4CE1
WINDOW_LENGTH = 45
WINDOW_SHA256 = "e1479fdfa979233393e3454645708d6f344192fce6f69aa7ea2c3c21e4cf458c"
ERROR_PC = 0x4D0D
REGIONS = {
    "JP": {
        "filename": "TQJP02.bin",
        "md5": "b7afb338ad31be1025b53f9aff12d73a",
        "sha256": "d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb",
        "size": 8_102_640,
        "offset": 0x2BF271,
    },
    "US": {
        "filename": "TQUS02.bin",
        "md5": "f23601102138f87c33025877767ebf76",
        "sha256": "f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565",
        "size": 8_104_992,
        "offset": 0x2BFBA1,
    },
}
ROW = re.compile(r"^([0-9a-f]{6}):\s*(.*?)\s{2,}.+$")


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
            raise AssertionError(f"listing address gap at line {line_number}: {address:#06x}")
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
    if end_pc != ERROR_PC + 1 or decoded_listing[-2:] != bytes((0x60, 0x00)):
        raise AssertionError("listing must end with RTS $4D0C and BRK target $4D0D")
    for branch_pc, opcode_offset, operand_offset in (
        (0x4CEA, 0x4CEA - START_PC, 0x4CEB - START_PC),
        (0x4CF1, 0x4CF1 - START_PC, 0x4CF2 - START_PC),
    ):
        if decoded_listing[opcode_offset] != 0xB0:
            raise AssertionError(f"expected BCS opcode at {branch_pc:#06x}")
        target = branch_pc + 2 + decoded_listing[operand_offset]
        if target != ERROR_PC:
            raise AssertionError(f"branch at {branch_pc:#06x} targets {target:#06x}")

    windows: dict[str, bytes] = {}
    for region, metadata in REGIONS.items():
        path = media_paths[region]
        if path.stat().st_size != metadata["size"]:
            raise AssertionError(f"{metadata['filename']}: unexpected media length")
        image = path.read_bytes()
        if (
            hashlib.md5(image).hexdigest() != metadata["md5"]
            or hashlib.sha256(image).hexdigest() != metadata["sha256"]
        ):
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
    print(
        "PASS: ID $4E callee, BCS targets, RTS and BRK listing match authentic JP/US media"
    )
    print(
        "LIMIT: static source correspondence only; helper semantics and runtime binding remain open"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
