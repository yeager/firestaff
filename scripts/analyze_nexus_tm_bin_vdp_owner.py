#!/usr/bin/env python3
"""Extract static SH-2 VDP register literal corridors from retail DM/TM.BIN.

This is disassembly evidence only. A literal load proves that the binary can
address a VDP register window; it does not prove that a particular runtime
frame, asset, or VDP1 command came from that instruction.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
from pathlib import Path

from analyze_nexus_title_vdp2_source import cue_track1, iso_members_in_memory


EXPECTED = {
    "DM.BIN": (555144, "3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124"),
    "TM.BIN": (160044, "d87485fe6eba1f6e9fbbf487f5fcdd994911136905e6172e5bb5bc0122407eb6"),
}
VDP1_REQUIRED = {0x25D00000, 0x25D00002, 0x25D00006, 0x25D00008, 0x25D0000A, 0x25D00010}
LOAD_BASE = {"DM.BIN": 0x06010040, "TM.BIN": 0x06010000}


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
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--retail-bin", type=Path,
                        help="hash-verified loose DM.BIN or TM.BIN")
    source.add_argument("--cue", type=Path,
                        help="retail Nexus CUE; DM.BIN and TM.BIN stay in memory")
    parser.add_argument("--member", choices=tuple(EXPECTED), default="DM.BIN",
                        help="member selected from --cue (default: DM.BIN)")
    args = parser.parse_args()
    try:
        if args.retail_bin is not None:
            name = args.retail_bin.name.upper()
            if name not in EXPECTED:
                raise ValueError("expected DM.BIN or TM.BIN")
            blob = args.retail_bin.read_bytes()
        else:
            name = args.member
            blob = iso_members_in_memory(cue_track1(args.cue), {name})[name]
        expected = EXPECTED[name]
    except (KeyError, OSError, ValueError) as error:
        print(f"NEXUS_TM_BIN_VDP_OWNER_INVALID: {error}")
        return 1
    digest = hashlib.sha256(blob).hexdigest()
    if len(blob) != expected[0] or digest != expected[1]:
        print(f"NEXUS_TM_BIN_VDP_OWNER_INVALID: size={len(blob)} sha256={digest}")
        return 1
    rows = scan(blob)
    values = {value for _, _, _, value in rows}
    print(f"source_file={name}")
    for instruction, literal, register, value in rows:
        bank = "VDP1" if (value & 0xFFFFFFE0) == 0x25D00000 else "VDP2"
        print(
            f"{bank} instruction=0x{instruction:06x} runtime_instruction="
            f"0x{LOAD_BASE[name] + instruction:08x} literal=0x{literal:06x} "
            f"runtime_literal=0x{LOAD_BASE[name] + literal:08x} "
            f"load_register=r{register} value=0x{value:08x}"
        )
    missing = sorted(VDP1_REQUIRED - values)
    vdp2_values = sorted(value for value in values
                         if (value & 0xFFFFFF00) == 0x25F00000)
    print("vdp2_literal_values=" + ",".join(f"0x{value:08x}" for value in vdp2_values))
    print(f"source_sha256={digest} size={len(blob)}")
    print("semantic_admission=blocked")
    if missing:
        print("missing_vdp1_literals=" + ",".join(f"0x{x:08x}" for x in missing))
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
