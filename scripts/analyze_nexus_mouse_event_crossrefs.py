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
EVENT_NAME_POINTER_TABLE_OFFSET = 0x3ADC4
EVENT_QUEUE_CONSUMER_OFFSET = 0x21C84
EVENT_BAD_DATA_FORMAT_OFFSET = 0x376A4
MOUSE_CALLBACK_EVENT_NAMES = {
    13: (b"EV_EXIT_INV", 0x36DB8),
    44: (b"EV_INV_EQUIP", 0x36F68),
}
HIT_RECT_TABLE_OFFSET = 0x38000
HIT_RECT_TABLE_SUBSET_OFFSET = HIT_RECT_TABLE_OFFSET + 8 * 8
MOUSE_SOURCE_PATH_OFFSET = 0x387FC
MOUSE_ACTION_RECT_TABLE_OFFSET = 0x3850C
MOUSE_ACTION_RECT_TABLE_COUNT = 8
MOUSE_ACTION_RECT_RECORD_SIZE = 12
MOUSE_ACTION_RECT_RECORDS = (
    (0x0101, 0x0068, 0x0111, 0x0078, 44, 0),
    (0x0120, 0x0068, 0x0130, 0x0078, 44, 1),
    (0x0110, 0x0048, 0x0120, 0x0058, 44, 2),
    (0x0110, 0x005B, 0x0120, 0x006B, 44, 3),
    (0x0111, 0x0073, 0x0121, 0x0083, 44, 4),
    (0x0110, 0x0088, 0x0120, 0x0098, 44, 5),
    (0x00F8, 0x0050, 0x0108, 0x0060, 44, 6),
    (0x00FC, 0x0088, 0x010C, 0x0098, 44, 7),
)

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

# The registered callback has a secondary path for mouse-region objects in
# interaction class 6. It submits two event IDs for object modes 0 and 2.
# Verify their names through the retail pointer table, but do not infer the
# downstream state changes from those names alone.
MOUSE_CALLBACK_MODE_CHECK = 0x16548
MOUSE_CALLBACK_MODE0_EVENT = 0x1656A
MOUSE_CALLBACK_MODE2_EVENT = 0x1657E
MOUSE_CALLBACK_EVENT_DISPATCH = 0x16568
MOUSE_CALLBACK_MODE2_DISPATCH = 0x1657C


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


def bsr_target(instruction_offset: int, opcode: int) -> int | None:
    """Resolve a SH-2 BSR target (12-bit displacement, base PC+4)."""
    if opcode >> 12 != 0xB:
        return None
    displacement = opcode & 0x0FFF
    if displacement & 0x0800:
        displacement -= 0x1000
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


def report_mouse_callback_event_ids(data: bytes) -> bool:
    """Verify the callback's mode-gated numeric event submissions.

    It checks the retail SH-2 path which reads object fields 100/104, passes
    IDs 13/44 to the event dispatcher for modes 0/2, and resolves those IDs
    through the retail event-name pointer table. The names do not establish
    the downstream state changes.
    """
    words = {
        0x16548: 0xE064,  # mov #100,r0
        0x1654A: 0x00EE,  # mov.l @(r0,r14),r0
        0x1654C: 0x8806,  # cmp/eq #6,r0
        0x1654E: 0x8B1B,  # bf 0x16588
        0x16550: 0xE068,  # mov #104,r0
        0x16552: 0x00EE,  # mov.l @(r0,r14),r0
        0x16554: 0x8800,  # cmp/eq #0,r0
        0x16556: 0x8905,  # bt 0x16564
        0x16558: 0x8801,  # cmp/eq #1,r0
        0x1655A: 0x8915,  # bt 0x16588 (no event submission)
        0x1655C: 0x8802,  # cmp/eq #2,r0
        0x1655E: 0x890A,  # bt 0x16576
        0x1656A: 0xE40D,  # mov #13,r4
        0x1657A: 0xE507,  # mov #7,r5
        0x1657E: 0xE42C,  # mov #44,r4
    }
    valid = True
    for offset, expected in words.items():
        actual = read_be16(data, offset)
        if actual != expected:
            print(f"mouse_callback_event_path=invalid file_offset=0x{offset:06x} "
                  f"expected=0x{expected:04x} actual="
                  f"{('none' if actual is None else f'0x{actual:04x}')}")
            valid = False

    for offset, expected_target in ((0x1654E, 0x16588),
                                    (0x16556, 0x16564),
                                    (0x1655A, 0x16588),
                                    (0x1655E, 0x16576)):
        opcode = read_be16(data, offset)
        if opcode is None or bt_target(offset, opcode) != expected_target:
            print(f"mouse_callback_event_branch=invalid file_offset=0x{offset:06x}")
            valid = False

    for offset in (MOUSE_CALLBACK_EVENT_DISPATCH,
                   MOUSE_CALLBACK_MODE2_DISPATCH):
        decoded = mov_l_pc_literal(data, offset)
        if decoded != (3 if offset == MOUSE_CALLBACK_EVENT_DISPATCH else 2,
                       0x06024288):
            print(f"mouse_callback_event_dispatch=invalid file_offset=0x{offset:06x} "
                  f"decoded={decoded}")
            valid = False

    event_names: dict[int, str] = {}
    for event_id, (expected_name, expected_string_offset) in MOUSE_CALLBACK_EVENT_NAMES.items():
        pointer_offset = EVENT_NAME_POINTER_TABLE_OFFSET + event_id * 4
        pointer = (int.from_bytes(bytes(data[pointer_offset:pointer_offset + 4]), "big")
                   if pointer_offset + 4 <= len(data) else None)
        expected_pointer = DM_BASE + expected_string_offset
        end = bytes(data).find(b"\0", expected_string_offset)
        actual_name = (data[expected_string_offset:end]
                       if end >= 0 else b"")
        if pointer != expected_pointer or actual_name != expected_name:
            print(f"mouse_callback_event_name=invalid id={event_id} "
                  f"pointer={('none' if pointer is None else f'0x{pointer:08x}')} "
                  f"name={actual_name!r}")
            valid = False
        else:
            event_names[event_id] = expected_name.decode("ascii")

    if valid:
        print("mouse_callback_object_mode_field_offset=100 value=6")
        print("mouse_callback_submode_field_offset=104 event_mode_0_id=13 "
              f"event_mode_0_name={event_names[13]} event_mode_2_id=44 "
              f"event_mode_2_name={event_names[44]} event_mode_1=no-submit")
        print("mouse_callback_event_dispatch_target=0x06024288 "
              "event_names=verified gameplay_effects=unbound")
    return valid


def report_mouse_inventory_hit_regions(data: bytes) -> bool:
    """Verify the retail inventory hit rectangles and event queue dispatch."""
    valid = True
    for index, expected in enumerate(MOUSE_ACTION_RECT_RECORDS):
        offset = MOUSE_ACTION_RECT_TABLE_OFFSET + index * MOUSE_ACTION_RECT_RECORD_SIZE
        actual = tuple(read_be16(data, offset + field * 2) for field in range(6))
        if actual != expected:
            print(f"mouse_inventory_hit_region=invalid index={index} "
                  f"expected={expected} actual={actual}")
            valid = False
    # Retail SH-2 bounds scan: compare pointer x/y with each 12-byte record,
    # then load event id at +8 and argument at +10 before calling 0x06024288.
    expected_words = {
        0x24EE2: 0x62E1,  # MOV.W @R14,R2 (x1)
        0x24EE4: 0x3247,  # CMP/GT R4,R2
        0x24EE8: 0x85E2,  # MOV.W @(4,R14),R0 (x2)
        0x24EEA: 0x3407,  # CMP/GT R0,R4
        0x24EEE: 0x85E1,  # MOV.W @(2,R14),R0 (y1)
        0x24EF0: 0x3057,  # CMP/GT R0,R5
        0x24EF4: 0x85E3,  # MOV.W @(6,R14),R0 (y2)
        0x24EF6: 0x3507,  # CMP/GT R0,R5
        0x24EFC: 0x85E5,  # MOV.W @(10,R14),R0 (event argument)
        0x24EFE: 0x6503,  # MOV R0,R5
        0x24F00: 0x85E4,  # MOV.W @(8,R14),R0 (event id)
        0x24F02: 0x4D0B,  # JSR @R13
        0x24F04: 0x6403,  # delay slot: MOV R0,R4
    }
    for offset, expected in expected_words.items():
        actual = read_be16(data, offset)
        if actual != expected:
            print(f"mouse_inventory_dispatch=invalid file_offset=0x{offset:06x} "
                  f"expected=0x{expected:04x} actual="
                  f"{('none' if actual is None else f'0x{actual:04x}')}")
            valid = False
    queue_writer = mov_l_pc_literal(data, 0x24EDC)
    if queue_writer != (13, 0x06024288):
        print(f"mouse_inventory_queue_target=invalid decoded={queue_writer}")
        valid = False
    if valid:
        print("mouse_inventory_hit_regions=8 event=44(EV_INV_EQUIP) "
              "arguments=inventory_slots_0_to_7")
        print("mouse_inventory_hit_region_dispatch=retail_bounds_scan_and_queued_event_verified")
    return valid


def report_mouse_event_queue_fallback(data: bytes) -> bool:
    """Verify the current event consumer's special cases and generic fallback."""
    valid = True
    # These are the only event IDs with branches in the queue loop. IDs 13 and
    # 44 take the fallback, which resolves a name and reports bad event data.
    expected_words = {
        0x21CAE: 0x8826,  # cmp/eq #38,r0
        0x21CB0: 0x8B01,  # bf 0x21cb6
        0x21CB6: 0x8827,  # cmp/eq #39,r0
        0x21CB8: 0x8941,  # bt 0x21d3e
        0x21CBA: 0x8828,  # cmp/eq #40,r0
        0x21CBC: 0x8945,  # bt 0x21d4a
        0x21CBE: 0x8831,  # cmp/eq #49,r0
        0x21CC0: 0x8949,  # bt 0x21d56
        0x21CC2: 0x8834,  # cmp/eq #52,r0
        0x21CC4: 0x8905,  # bt 0x21cd2
        0x21CC6: 0x8838,  # cmp/eq #56,r0
        0x21CC8: 0x894A,  # bt 0x21d60
        0x21CCA: 0x8839,  # cmp/eq #57,r0
        0x21CCC: 0x895C,  # bt 0x21d88
        0x21CCE: 0xA063,  # bra 0x21d98, generic fallback
        0x21D98: 0xD34F,  # name lookup function address
        0x21D9A: 0x430B,  # jsr @r3
        0x21D9C: 0x64B3,  # event ID -> lookup argument
        0x21D9E: 0xE519,  # diagnostic argument length
        0x21DA4: 0xD34D,  # format string address
        0x21DAA: 0xD24D,  # diagnostic function address
        0x21DAC: 0x420B,  # jsr @r2
        0x21DAE: 0xE400,  # delay slot: diagnostic context 0
    }
    for offset, expected in expected_words.items():
        actual = read_be16(data, offset)
        if actual != expected:
            print(f"mouse_event_queue_consumer=invalid file_offset=0x{offset:06x} "
                  f"expected=0x{expected:04x} actual="
                  f"{('none' if actual is None else f'0x{actual:04x}')}")
            valid = False
    for offset, expected_target in ((0x21CC8, 0x21D60),
                                    (0x21CCC, 0x21D88),
                                    (0x21CCE, 0x21D98)):
        opcode = read_be16(data, offset)
        target = (bt_target(offset, opcode) if offset != 0x21CCE else
                  offset + 4 + (((opcode & 0xFFF) ^ 0x800) - 0x800) * 2
                  if opcode is not None and opcode >> 12 == 0xA else None)
        if target != expected_target:
            print(f"mouse_event_queue_branch=invalid file_offset=0x{offset:06x} "
                  f"target={target}")
            valid = False
    for offset, expected in ((0x21D98, (3, 0x060241B4)),
                             (0x21DA4, (3, 0x060476A4)),
                             (0x21DAA, (2, 0x06023DB4))):
        if mov_l_pc_literal(data, offset) != expected:
            print(f"mouse_event_queue_fallback_literal=invalid "
                  f"file_offset=0x{offset:06x} "
                  f"decoded={mov_l_pc_literal(data, offset)}")
            valid = False
    format_end = bytes(data).find(b"\0", EVENT_BAD_DATA_FORMAT_OFFSET)
    format_string = (bytes(data[EVENT_BAD_DATA_FORMAT_OFFSET:format_end])
                     if format_end >= 0 else b"")
    if format_string != b"bad event data ($%x:%s)\n":
        print(f"mouse_event_queue_fallback_format=invalid value={format_string!r}")
        valid = False
    if valid:
        print("mouse_event_queue_special_case_ids=38,39,40,49,52,56,57")
        print("mouse_event_ids_13_44_consumer=generic_bad_event_diagnostic "
              "gameplay_state_effect=not_verified")
    return valid


def report_mouse_motion_event_gate(data: bytes) -> bool:
    """Verify mouse motion events are the only inputs reaching the hit scanner."""
    expected_words = {
        0x250EE: 0x6042,  # read queued event ID
        0x250F0: 0x882E,  # cmp/eq #46,r0
        0x250F2: 0x8B14,  # bf past mouse processing unless ID 46
        0x2511A: 0xBEDC,  # bsr to the 0x34ed6 table hit scanner
        0x2511C: 0x0009,  # delay-slot nop
    }
    valid = True
    for offset, expected in expected_words.items():
        actual = read_be16(data, offset)
        if actual != expected:
            print(f"mouse_motion_event_gate=invalid file_offset=0x{offset:06x} "
                  f"expected=0x{expected:04x} actual="
                  f"{('none' if actual is None else f'0x{actual:04x}')}")
            valid = False
    if bt_target(0x250F2, 0x8B14) != 0x2511E:
        print("mouse_motion_event_gate=invalid nonmouse branch target")
        valid = False
    if bsr_target(0x2511A, 0xBEDC) != 0x24ED6:
        print("mouse_motion_event_gate=invalid hit-scanner call target")
        valid = False
    if mov_l_pc_literal(data, 0x250F4) != (2, 0x060242D6):
        print(f"mouse_motion_event_gate=invalid queue-pop call "
              f"decoded={mov_l_pc_literal(data, 0x250F4)}")
        valid = False
    if valid:
        print("mouse_motion_event_gate=event_46_only_then_retail_hit_scanner")
        print("mouse_motion_event_44_is_not_a_separate_mouse_poller_input")
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
    if not report_mouse_callback_event_ids(data):
        print("NEXUS_MOUSE_CALLBACK_EVENT_PATH_INVALID: retail callback path changed")
        return 1
    if not report_mouse_inventory_hit_regions(data):
        print("NEXUS_MOUSE_INVENTORY_HIT_PATH_INVALID: retail hit path changed")
        return 1
    if not report_mouse_event_queue_fallback(data):
        print("NEXUS_MOUSE_EVENT_QUEUE_CONSUMER_INVALID: retail consumer changed")
        return 1
    if not report_mouse_motion_event_gate(data):
        print("NEXUS_MOUSE_MOTION_EVENT_GATE_INVALID: retail poller changed")
        return 1

    print("interpretation=verified retail hit-region registration and callback transport; "
          "verified callback event IDs and retail names; downstream gameplay "
          "actions remain unbound")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
