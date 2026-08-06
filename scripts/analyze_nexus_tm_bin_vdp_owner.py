#!/usr/bin/env python3
"""Extract static SH-2 VDP register literal corridors from retail TM.BIN.

This is disassembly evidence only. A literal load proves that the binary can
address a VDP register window; it does not prove that a particular runtime
frame, asset, or VDP1 command came from that instruction.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
from pathlib import Path


EXPECTED_SIZE = 160044
EXPECTED_SHA256 = "d87485fe6eba1f6e9fbbf487f5fcdd994911136905e6172e5bb5bc0122407eb6"
VDP1_REQUIRED = {0x25D00000, 0x25D00002, 0x25D00006, 0x25D00008, 0x25D0000A, 0x25D00010}


def scan(blob: bytes) -> list[tuple[int, int, int, int]]:
    rows: list[tuple[int, int, int, int]] = []
    for offset in range(0, len(blob) - 1, 2):
        opcode = struct.unpack_from(">H", blob, offset)[0]
        if opcode >> 12 != 0xD:
            continue
        register = (opcode >> 8) & 0xF
        displacement = opcode & 0xFF
        literal_offset = ((offset + 4) & ~3) + displacement * 4
        if literal_offset + 4 > len(blob):
            continue
        value = struct.unpack_from(">I", blob, literal_offset)[0]
        if (value & 0xFFFFFFE0) == 0x25D00000 or (value & 0xFFFFFF00) == 0x25F00000:
            rows.append((offset, literal_offset, register, value))
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("tm_bin", type=Path)
    args = parser.parse_args()
    try:
        blob = args.tm_bin.read_bytes()
    except OSError as error:
        print(f"NEXUS_TM_BIN_VDP_OWNER_INVALID: {error}")
        return 1
    digest = hashlib.sha256(blob).hexdigest()
    if len(blob) != EXPECTED_SIZE or digest != EXPECTED_SHA256:
        print(f"NEXUS_TM_BIN_VDP_OWNER_INVALID: size={len(blob)} sha256={digest}")
        return 1
    rows = scan(blob)
    values = {value for _, _, _, value in rows}
    for instruction, literal, register, value in rows:
        bank = "VDP1" if (value & 0xFFFFFFE0) == 0x25D00000 else "VDP2"
        print(
            f"{bank} instruction=0x{instruction:06x} literal=0x{literal:06x} "
            f"load_register=r{register} value=0x{value:08x}"
        )
    missing = sorted(VDP1_REQUIRED - values)
    print(f"tm_bin_sha256={digest} size={len(blob)}")
    print("semantic_admission=blocked")
    if missing:
        print("missing_vdp1_literals=" + ",".join(f"0x{x:08x}" for x in missing))
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
