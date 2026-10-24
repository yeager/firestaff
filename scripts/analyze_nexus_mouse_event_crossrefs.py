#!/usr/bin/env python3
"""Inspect direct SH-2 literal references to Nexus mouse/event data in DM.BIN.

This is a static triage report, not a semantic decoder. A missing direct
reference does not prove that a symbol is unused: tables, offsets, indirect
dispatch, and runtime-generated pointers remain possible. The report keeps
that boundary explicit so event-name strings are not promoted into commands.
"""

from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path


DM_BASE = 0x06010000
DM_SHA256 = "3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124"
EVENT_TABLE_OFFSET = 0x36D04
EVENT_TABLE_END = 0x37024
HIT_RECT_TABLE_OFFSET = 0x38000
HIT_RECT_TABLE_SUBSET_OFFSET = HIT_RECT_TABLE_OFFSET + 8 * 8
MOUSE_SOURCE_PATH_OFFSET = 0x387FC


def resolve_dm_bin(explicit: Path | None) -> Path:
    if explicit is not None:
        return explicit
    data_dir = os.environ.get("FIRESTAFF_NEXUS_DATA_DIR")
    if data_dir:
        return Path(data_dir) / "DM.BIN"
    return Path.home() / ".firestaff" / "data" / "nexus" / "DM.BIN"


def read_be32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "big")


def mov_l_literal_pointer_xrefs(data: bytes, target_offset: int) -> list[tuple[int, int, int]]:
    """Find SH-2 MOV.L @(disp,PC),Rn literals equal to base+target_offset."""
    target_address = DM_BASE + target_offset
    refs: list[tuple[int, int, int]] = []
    for instruction_offset in range(0, len(data) - 1, 2):
        opcode = int.from_bytes(data[instruction_offset : instruction_offset + 2], "big")
        if opcode >> 12 != 0xD:
            continue
        pc = DM_BASE + instruction_offset
        literal_address = ((pc + 4) & ~3) + (opcode & 0xFF) * 4
        literal_offset = literal_address - DM_BASE
        if 0 <= literal_offset <= len(data) - 4 and read_be32(data, literal_offset) == target_address:
            refs.append((instruction_offset, opcode, literal_offset))
    return refs


def event_string_offsets(data: bytes) -> list[tuple[int, bytes]]:
    pool = data[EVENT_TABLE_OFFSET:EVENT_TABLE_END]
    result: list[tuple[int, bytes]] = []
    cursor = 0
    while cursor < len(pool):
        start = pool.find(b"EV_", cursor)
        if start < 0:
            break
        end = pool.find(b"\0", start)
        if end < 0:
            break
        result.append((EVENT_TABLE_OFFSET + start, pool[start:end]))
        cursor = end + 1
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dm_bin", nargs="?", type=Path)
    parser.add_argument("--allow-unpinned-hash", action="store_true",
                        help="report a noncanonical binary without failing")
    args = parser.parse_args()
    path = resolve_dm_bin(args.dm_bin)
    try:
        data = path.read_bytes()
    except OSError as error:
        print(f"NEXUS_MOUSE_XREF_INVALID: {error}")
        return 1

    digest = hashlib.sha256(data).hexdigest()
    print(f"dm_bin={path}")
    print(f"size={len(data)}")
    print(f"sha256={digest}")
    if digest != DM_SHA256:
        print("canonical_retail_dm_bin=unverified")
        if not args.allow_unpinned_hash:
            return 1
    else:
        print("canonical_retail_dm_bin=verified")

    if EVENT_TABLE_END > len(data) or MOUSE_SOURCE_PATH_OFFSET >= len(data):
        print("NEXUS_MOUSE_XREF_INVALID: expected source data lies outside DM.BIN")
        return 1

    for offset, symbol in event_string_offsets(data):
        refs = mov_l_literal_pointer_xrefs(data, offset)
        rendered = ",".join(f"0x{insn:06x}" for insn, _opcode, _literal in refs) or "none"
        print(f"event_symbol={symbol.decode('ascii')} file_offset=0x{offset:06x} "
              f"direct_mov_l_literal_xrefs={rendered}")

    for label, offset in (("hud_hit_rect_table", HIT_RECT_TABLE_OFFSET),
                          ("hud_hit_rect_table_after_first_8_records",
                           HIT_RECT_TABLE_SUBSET_OFFSET),
                          ("mouse_source_path", MOUSE_SOURCE_PATH_OFFSET)):
        refs = mov_l_literal_pointer_xrefs(data, offset)
        rendered = ",".join(
            f"insn=0x{insn:06x}/literal=0x{literal:06x}"
            for insn, _opcode, literal in refs) or "none"
        print(f"data_symbol={label} file_offset=0x{offset:06x} "
              f"direct_mov_l_literal_xrefs={rendered}")

    print("interpretation=direct-reference inventory only; no event-to-command or gameplay action is admitted")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
