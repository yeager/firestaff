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

# Retail mouse hit-region setup discovered by following the direct
# MOV.L-literal references to the DM.BIN table. These are file offsets in the
# same SH-2 image as the event strings above; they are not source symbols.
MOUSE_REGION_SETUP_SITE = 0x16588
MOUSE_REGION_REGISTER_ROUTINE = 0x24AD0
MOUSE_REGION_CALLBACK_SLOT_LOAD = 0x24B08
MOUSE_REGION_CALLBACK_LOAD = 0x24B2E
MOUSE_REGION_CALLBACK_CALL = 0x24B36
MOUSE_REGION_ACTIVE_FLAG_LOAD = 0x24B26
MOUSE_REGION_ACTIVE_FLAG_TEST = 0x24B2A
MOUSE_REGION_ACTIVE_FLAG_BRANCH = 0x24B2C
MOUSE_REGION_CALLBACK_NULL_TEST = 0x24B32
MOUSE_REGION_CALLBACK_NULL_BRANCH = 0x24B34


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


def mov_l_pc_literal(data: bytes, instruction_offset: int) -> tuple[int, int] | None:
    """Return (register, value) for one SH-2 MOV.L @(disp,PC),Rn."""
    if instruction_offset < 0 or instruction_offset + 2 > len(data):
        return None
    opcode = int.from_bytes(data[instruction_offset : instruction_offset + 2], "big")
    if opcode >> 12 != 0xD:
        return None
    pc = DM_BASE + instruction_offset
    literal_address = ((pc + 4) & ~3) + (opcode & 0xFF) * 4
    literal_offset = literal_address - DM_BASE
    if literal_offset < 0 or literal_offset + 4 > len(data):
        return None
    return (opcode >> 8) & 0xF, read_be32(data, literal_offset)


def read_be16(data: bytes, offset: int) -> int | None:
    if offset < 0 or offset + 2 > len(data):
        return None
    return int.from_bytes(data[offset : offset + 2], "big")


def bt_target(instruction_offset: int, opcode: int) -> int | None:
    """Resolve a SH-2 BT/BF target (the branch base is PC+4)."""
    if opcode >> 8 not in (0x89, 0x8B):
        return None
    displacement = opcode & 0xFF
    if displacement & 0x80:
        displacement -= 0x100
    return instruction_offset + 4 + displacement * 2


def report_mouse_region_registration(data: bytes) -> bool:
    """Verify the retail hit-region registration and indirect callback path."""
    expected_setup = (
        (0x16588, 6, DM_BASE + 0x164A8),  # R6: callback code
        (0x1658A, 5, DM_BASE + 0x3850C),  # R5: companion retail table
        (0x1658C, 4, DM_BASE + 0x38040),  # R4: hit rectangles, entries 8+
        (0x1658E, 2, DM_BASE + MOUSE_REGION_REGISTER_ROUTINE),
    )
    valid = True
    resolved: list[tuple[int, int, int]] = []
    for instruction_offset, expected_register, expected_value in expected_setup:
        decoded = mov_l_pc_literal(data, instruction_offset)
        if decoded is None:
            print(f"mouse_region_setup=invalid instruction=0x{instruction_offset:06x}")
            valid = False
            continue
        register, value = decoded
        resolved.append((instruction_offset, register, value))
        print(f"mouse_region_setup_movl=insn:0x{instruction_offset:06x} "
              f"R{register}=0x{value:08x}")
        if (register, value) != (expected_register, expected_value):
            valid = False

    call_opcode = read_be16(data, 0x16590)
    if call_opcode != 0x420B:
        print(f"mouse_region_setup_call=invalid opcode=0x{call_opcode:04x}")
        valid = False
    else:
        print("mouse_region_setup_call=JSR @R2 at file_offset=0x016590")

    # The registration routine stores the supplied R6 callback in the global
    # callback slot. The dispatch routine checks its active flag, then calls
    # that stored function pointer. Validate the relevant instructions and
    # resolve the two global addresses instead of inferring from a source path.
    callback_store_slot = mov_l_pc_literal(
        data, MOUSE_REGION_CALLBACK_SLOT_LOAD)
    callback_slot = mov_l_pc_literal(data, MOUSE_REGION_CALLBACK_LOAD)
    active_flag = mov_l_pc_literal(data, MOUSE_REGION_ACTIVE_FLAG_LOAD)
    # R6 is saved at the top of this routine's 8-byte stack frame. The
    # registration path reloads that value from @R15 and writes it to the
    # callback slot. Check the stack setup, load, null-test/branch and its
    # delay-slot store together; checking only the final store opcode would
    # also accept an unrelated R3 value.
    registration_path = (
        read_be16(data, 0x24AD8) == 0x7FF8,  # add #-8,R15
        read_be16(data, 0x24ADC) == 0x2F62,  # mov.l R6,@R15
        read_be16(data, 0x24B0A) == 0x60F2,  # mov.l @R15,R0
        read_be16(data, 0x24B0C) == 0x2008,  # tst R0,R0
        read_be16(data, 0x24B0E) == 0x8902,  # bt to 0x24B16
        bt_target(0x24B0E, read_be16(data, 0x24B0E) or 0) == 0x24B16,
        read_be16(data, 0x24B10) == 0x63F2,  # delay slot: mov.l @R15,R3
        read_be16(data, 0x24B12) == 0xA002,  # bra to 0x24B18
        read_be16(data, 0x24B14) == 0x2432,  # delay slot: mov.l R3,@R4
        read_be16(data, 0x24B16) == 0xE100,
        read_be16(data, 0x24B18) == 0x2412,
        read_be16(data, 0x24B1A) == 0x7F08,  # add #8,R15
    )
    active_test_opcode = read_be16(data, MOUSE_REGION_ACTIVE_FLAG_TEST)
    active_branch_opcode = read_be16(data, MOUSE_REGION_ACTIVE_FLAG_BRANCH)
    callback_null_test = read_be16(data, MOUSE_REGION_CALLBACK_NULL_TEST)
    callback_null_branch = read_be16(data, MOUSE_REGION_CALLBACK_NULL_BRANCH)
    callback_call_opcode = read_be16(data, MOUSE_REGION_CALLBACK_CALL)
    if (not all(registration_path) or
        not callback_store_slot or callback_store_slot[0] != 4 or
        not callback_slot or callback_slot[0] != 2 or
        callback_store_slot[1] != callback_slot[1] or
        not active_flag or active_flag[0] != 3):
        print("mouse_region_callback_dispatch=invalid registration provenance or global pointer loads")
        valid = False
    elif (active_test_opcode, active_branch_opcode,
          callback_null_test, callback_null_branch, callback_call_opcode) != (
        0x2008, 0x8922, 0x2EE8, 0x8901, 0x4E0B
    ) or bt_target(MOUSE_REGION_ACTIVE_FLAG_BRANCH, active_branch_opcode or 0) != 0x24B74 \
      or bt_target(MOUSE_REGION_CALLBACK_NULL_BRANCH, callback_null_branch or 0) != 0x24B3A:
        print("mouse_region_callback_dispatch=invalid active-test, branch-target, or callback-call sequence")
        valid = False
    else:
        print(f"mouse_region_registration_r6_callback_slot=0x{callback_slot[1]:08x}")
        print(f"mouse_region_dispatch_active_flag=0x{active_flag[1]:08x}")
        print("mouse_region_dispatch=active-flag-guarded indirect JSR @R14 "
              "after loading callback slot")
    return valid


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

    if not report_mouse_region_registration(data):
        print("NEXUS_MOUSE_REGION_REGISTRATION_INVALID: retail instruction path changed")
        return 1

    print("interpretation=verified retail hit-region registration and callback transport; "
          "callback semantics and resulting gameplay actions remain unbound")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
