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
MOUSE_CALLBACK_MODE0_CALL = 0x16572
MOUSE_CALLBACK_MODE2_CALL = 0x16584


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
    if opcode >> 8 not in (0x89, 0x8B, 0x8D, 0x8F):
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


def bra_target(instruction_offset: int, opcode: int) -> int | None:
    """Resolve a SH-2 BRA target (12-bit displacement, base PC+4)."""
    if opcode >> 12 != 0xA:
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

    # The dispatch literal alone is insufficient: the retail function must
    # actually call the target after loading each event ID. Check the JSR and
    # its delay-slot instruction for both mode-gated paths.
    for offset, expected in ((MOUSE_CALLBACK_MODE0_CALL, 0x432B),
                             (MOUSE_CALLBACK_MODE2_CALL, 0x422B)):
        actual = read_be16(data, offset)
        delay_slot = read_be16(data, offset + 2)
        if (actual, delay_slot) != (expected, 0x6EF6):
            print(f"mouse_callback_event_call=invalid file_offset=0x{offset:06x} "
                  f"opcode={actual} delay_slot={delay_slot}")
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
        print("mouse_callback_event_queue_target=0x06024288 "
              "event_names=verified event_44_handler_semantics=unverified "
              "firestaff_inventory_mapping=unverified")
    return valid


def report_mouse_inventory_hit_regions(data: bytes) -> bool:
    """Verify the retail inventory rows and the scanner's eight-row loop."""
    valid = True
    if len(MOUSE_ACTION_RECT_RECORDS) != MOUSE_ACTION_RECT_TABLE_COUNT:
        print("mouse_inventory_hit_region=invalid configured record count")
        valid = False
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

    # Registration supplies this table base to the retail scanner (checked
    # independently by report_mouse_region_registration). The scanner advances
    # by one 12-byte row and stops on the signed -1 event-ID sentinel after the
    # eight live records. Validate the complete loop shape and sentinel so a
    # changed stride, bound condition, or back-edge cannot still claim that all
    # eight inventory regions were scanned.
    scanner_words = {
        0x24EE0: 0x6E63,  # load current row pointer from the registered list
        0x24F08: 0xE001,  # next-row path
        0x24F0A: 0x7E0C,  # add #12,R14 (record stride)
        0x24F0C: 0x85E4,  # MOV.W @(8,R14),R0 (signed sentinel/event ID)
        0x24F0E: 0x4015,  # CMP/PL R0
        0x24F10: 0x89E7,  # BT back to the row scanner while ID >= 0
        0x24F12: 0xE000,  # delay slot: MOV #0,R0
    }
    for offset, expected in scanner_words.items():
        actual = read_be16(data, offset)
        if actual != expected:
            print(f"mouse_inventory_scanner=invalid file_offset=0x{offset:06x} "
                  f"expected=0x{expected:04x} actual="
                  f"{('none' if actual is None else f'0x{actual:04x}')}")
            valid = False
    for offset, expected_target in ((0x24EE6, 0x24F0A),
                                    (0x24EEC, 0x24F0A),
                                    (0x24EF2, 0x24F0A),
                                    (0x24EF8, 0x24F0A),
                                    (0x24F10, 0x24EE2)):
        opcode = read_be16(data, offset)
        if opcode is None or bt_target(offset, opcode) != expected_target:
            print(f"mouse_inventory_scanner_branch=invalid "
                  f"file_offset=0x{offset:06x} target="
                  f"{None if opcode is None else bt_target(offset, opcode)}")
            valid = False
    sentinel_offset = (MOUSE_ACTION_RECT_TABLE_OFFSET +
                       MOUSE_ACTION_RECT_TABLE_COUNT * MOUSE_ACTION_RECT_RECORD_SIZE + 8)
    sentinel = read_be16(data, sentinel_offset)
    if sentinel != 0xFFFF:
        print(f"mouse_inventory_hit_region_sentinel=invalid "
              f"file_offset=0x{sentinel_offset:06x} value={sentinel}")
        valid = False
    queue_writer = mov_l_pc_literal(data, 0x24EDC)
    if queue_writer != (13, 0x06024288):
        print(f"mouse_inventory_queue_target=invalid decoded={queue_writer}")
        valid = False
    if valid:
        print(f"mouse_inventory_hit_regions={MOUSE_ACTION_RECT_TABLE_COUNT} "
              "event=44(EV_INV_EQUIP) "
              "arguments=inventory_slots_0_to_7")
        print("mouse_inventory_hit_region_dispatch=registered_action_table_0x03850c "
              "stride_12_signed_ffff_sentinel_and_loop_back_edge_verified")
    return valid


def report_mouse_event_queue_fallback(data: bytes) -> bool:
    """Verify the current event consumer's special cases and generic fallback."""
    valid = True
    # These are the only event IDs with branches in this later diagnostic
    # queue loop. IDs 13 and 44 take its generic fallback; a separate early
    # table dispatcher is checked by report_mouse_early_event_jump_table().
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
        print("mouse_later_loop_ids_13_44=generic_bad_event_diagnostic "
              "consumer=file_0x21c84; not_the_only_consumer")
        # Record exact direct-comparison candidates across the full image.
        # This is deliberately a byte-pattern scan: it does not identify code
        # versus data and cannot rule out register, table, or indirect dispatch.
        direct_event_44_cmp_r0 = [
            offset for offset in range(0, len(data) - 1, 2)
            if read_be16(data, offset) == 0x882C  # cmp/eq #44,r0
        ]
        locations = ",".join(f"0x{offset:06x}"
                             for offset in direct_event_44_cmp_r0)
        print("mouse_event_44_cmp_eq_r0_raw_candidates="
              f"{locations if locations else 'none'} "
              "scan=even_aligned_opcode_pattern_only")
    return valid


def report_mouse_event_name_resolver(data: bytes) -> bool:
    """Verify that the generic queue fallback resolves a name, not a handler."""
    expected_words = {
        0x141B4: 0x4411,  # cmp/pz R4
        0x141B6: 0x8B02,  # bf to out-of-range/default name path
        0x141B8: 0xE23D,  # mov #61,R2
        0x141BA: 0x3427,  # cmp/gt R2,R4
        0x141BC: 0x8B02,  # bf to in-range name table
        0x141BE: 0xD17C,  # load default name pointer
        0x141C0: 0x000B,  # rts
        0x141C2: 0x6012,  # delay slot: mov.l @R1,R0
        0x141C4: 0xD17B,  # load event-name pointer table
        0x141C6: 0x6043,  # mov R4,R0 (event id)
        0x141C8: 0x4008,  # shll2 R0
        0x141CA: 0x001E,  # mov.l @(R0,R1),R0
        0x141CC: 0x000B,  # rts
        0x141CE: 0x0009,  # nop
    }
    valid = True
    for offset, expected in expected_words.items():
        actual = read_be16(data, offset)
        if actual != expected:
            print(f"mouse_event_name_resolver=invalid file_offset=0x{offset:06x} "
                  f"expected=0x{expected:04x} actual="
                  f"{('none' if actual is None else f'0x{actual:04x}')}")
            valid = False

    table_base = mov_l_pc_literal(data, 0x141C4)
    default_name = mov_l_pc_literal(data, 0x141BE)
    if table_base != (1, DM_BASE + EVENT_NAME_POINTER_TABLE_OFFSET):
        print(f"mouse_event_name_table_base=invalid decoded={table_base}")
        valid = False
    if default_name != (1, 0x0604AEBC):
        print(f"mouse_event_default_name=invalid decoded={default_name}")
        valid = False

    for offset, expected_target in ((0x141B6, 0x141BE),
                                    (0x141BC, 0x141C4)):
        opcode = read_be16(data, offset)
        if opcode is None or bt_target(offset, opcode) != expected_target:
            print(f"mouse_event_name_resolver_branch=invalid "
                  f"file_offset=0x{offset:06x}")
            valid = False
    if valid:
        print("mouse_event_queue_fallback_helper=event_id_to_name_pointer "
              "table=0x0604adc4; no event-action dispatch")
    return valid


def report_mouse_event_queue_link(data: bytes) -> bool:
    """Verify the retail queue append/dequeue pair used by the event loop."""
    valid = True
    for offset, expected in {
        0x14292: 0x8808,  # queue capacity is eight records
        0x142B6: 0x7201,  # increment queued-record count
        0x142C8: 0x2742,  # store event ID (R4) at record +0
        0x142CA: 0x1751,  # store argument (R5) at record +4
        0x142CC: 0x1762,  # store argument (R6) at record +8
        0x21DB6: 0x420B,  # event loop calls the dequeue helper
        0x21DB8: 0x0009,  # delay-slot nop
        0x1462A: 0x6032,  # load gated global slot
        0x1462C: 0x2008,  # test global slot
        0x14630: 0xE600,  # event 17's third argument
        0x14632: 0x6563,  # event 17's second argument
        0x14636: 0xE411,  # event 17's first argument
    }.items():
        actual = read_be16(data, offset)
        if actual != expected:
            print(f"mouse_event_queue_link=invalid file_offset=0x{offset:06x} "
                  f"expected=0x{expected:04x} actual="
                  f"{('none' if actual is None else f'0x{actual:04x}')}")
            valid = False

    expected_literals = {
        0x1428C: (14, 0x06079814),  # record count
        0x142A2: (3, 0x06079810),   # queue index/state
        0x142B4: (1, 0x060797B0),   # record storage
        0x142DC: (13, 0x06079814),  # dequeue reads same count
        0x142E8: (14, 0x06079810),  # dequeue reads same queue state
        0x142F4: (2, 0x060797B0),   # dequeue reads same record storage
        0x21DB4: (2, 0x060242D6),  # dispatcher dequeue helper
    }
    for offset, expected in expected_literals.items():
        actual = mov_l_pc_literal(data, offset)
        if actual != expected:
            print(f"mouse_event_queue_literal=invalid file_offset=0x{offset:06x} "
                  f"expected={expected} actual={actual}")
            valid = False

    event17_branch = read_be16(data, 0x14634)
    if event17_branch is None or bsr_target(0x14634, event17_branch) != 0x14288:
        print("mouse_event_17_queue_target=invalid "
              f"target={None if event17_branch is None else bsr_target(0x14634, event17_branch)}")
        valid = False

    if valid:
        print("mouse_event_queue=shared_append_dequeue count=0x06079814 "
              "records=0x060797b0 stride=12 capacity=8")
        print("mouse_event_17_global_slot_path=appends_to_shared_queue "
              "consumer_dispatch_semantics_require_context")
    return valid


def report_mouse_early_event_jump_table(data: bytes) -> bool:
    """Verify the early main event switch's table-routed event-44 handler."""
    expected_words = {
        0x286: 0x70FE,  # event id - 2 -> table index
        0x288: 0xE13B,  # 59 relative event slots
        0x28A: 0x3012,  # cmp/hs R1,R0
        0x28C: 0x8B01,  # branch to table dispatch when in range
        0x292: 0x4000,  # shll R0 (16-bit table index)
        0x294: 0x6103,  # mov R0,R1
        0x296: 0xC707,  # mova relative event table base
        0x298: 0x001D,  # load signed 16-bit relative case target
        0x29A: 0x0023,  # braf R0
        0x29C: 0x0009,  # nop
        0x282: 0x6422,  # load current context index through global pointer
        0x8C0: 0x55FE,  # mov.l @(56,R15),R5; event-case slot argument
        0x8C4: 0x410B,  # jsr @R1
        0x8C6: 0x0009,  # nop
        0x26E58: 0x2FE6,  # event-44 handler saves R14
        0x26E64: 0xBE78,  # bsr context resolver at 0x26b58
        0x26E5E: 0x6D43,  # handler context R4 -> R13
        0x26E66: 0x6E53,  # hit-row argument R5 -> R14
        0x26E68: 0x6C03,  # context record pointer from resolver
        0x26E6C: 0xBDEE,  # bsr staged-pair initializer at 0x26a4c
        0x26E6E: 0x2F26,  # delay slot: pass staged-pair pointer
        0x26E72: 0x8807,  # slot 7 is a separate path
        0x26E74: 0x8F0E,  # bf normal slots to 0x26e94
        0x26E98: 0x4211,  # compare staged pair word with zero
        0x26E9A: 0x8916,  # bt place/compatibility path at 0x26eca
        0x26EA4: 0xB0DA,  # bsr slot-pair lookup/clear helper 0x2705c
        0x26EAC: 0x88FF,  # compare staged item ID with -1
        0x26EAE: 0x8B01,  # bf held-pair helper path at 0x26eb4
        0x26F38: 0xB090,  # bsr slot-pair lookup/clear helper 0x2705c
        0x26F42: 0xB133,  # bsr slot-pair write helper 0x271ac
    }
    valid = True
    for offset, expected in expected_words.items():
        actual = read_be16(data, offset)
        if actual != expected:
            print(f"mouse_early_event_dispatch=invalid file_offset=0x{offset:06x} "
                  f"expected=0x{expected:04x} actual="
                  f"{('none' if actual is None else f'0x{actual:04x}')}")
            valid = False

    table_index = 44 - 2
    table_offset = 0x2B4 + table_index * 2
    relative_target = read_be16(data, table_offset)
    target = (0x29E + (relative_target - 0x10000
                       if relative_target is not None and relative_target & 0x8000
                       else relative_target)
              if relative_target is not None else None)
    if target != 0x8C0:
        print(f"mouse_event_44_jump_table=invalid file_offset=0x{table_offset:06x} "
              f"target={None if target is None else f'0x{target:06x}'}")
        valid = False

    handler = mov_l_pc_literal(data, 0x8C2)
    if handler != (1, 0x06036E58):
        print(f"mouse_event_44_handler_pointer=invalid decoded={handler}")
        valid = False
    for offset, expected_target in ((0x26E64, 0x26B58),
                                    (0x26E6C, 0x26A4C),
                                    (0x26E74, 0x26E94),
                                    (0x26E9A, 0x26ECA),
                                    (0x26EA4, 0x2705C),
                                    (0x26EAE, 0x26EB4),
                                    (0x26F38, 0x2705C),
                                    (0x26F42, 0x271AC)):
        opcode = read_be16(data, offset)
        target = (bt_target(offset, opcode)
                  if offset in (0x26E74, 0x26E9A, 0x26EAE)
                  else bsr_target(offset, opcode)) if opcode is not None else None
        if target != expected_target:
            print(f"mouse_event_44_handler_branch=invalid file_offset=0x{offset:06x} "
                  f"target={None if target is None else f'0x{target:06x}'}")
            valid = False
    if valid:
        print("mouse_early_event_dispatch=relative_jump_table "
              "event_44_case=DM.BIN:0x0008c0 "
              "handler_pointer=0x06036e58 "
              "context_index=global_context_pointer "
              "slot_argument=stack_offset_56 "
              "slot7=separate_path normal_slot_pair_path=verified "
              "item_compatibility_path=verified state_model_mapping=unverified")
    return valid


def report_mouse_event_44_state_evidence(data: bytes) -> bool:
    """Check selected opcodes and literals near the event-44 handler."""
    valid = True
    expected_words = {
        0x26E58: 0x2FE6,  # event-44 handler prologue
        0x26E5E: 0x6D43,  # preserve context in R13
        0x26E66: 0x6E53,  # preserve hit-row slot in R14
        0x27064: 0x9325,  # load context stride literal
        0x27070: 0x2C3F,  # multiply context index by stride
        0x27072: 0xD211,  # load retail context table base
        0x27074: 0x911E,  # load slot-array offset literal
        0x27084: 0x4408,  # slot index * 4
        0x27086: 0x73D4,  # add stride - 44 (0x110)
        0x27088: 0x33CC,  # add context base
        0x2708A: 0x334C,  # add slot offset
        0x2708C: 0x6031,  # read first pair word
        0x27094: 0x8531,  # read second pair word
        0x2709A: 0x2431,  # clear source first word
        0x271CC: 0x9375,  # load context stride for write helper
        0x2721E: 0x4308,  # slot index * 4
        0x27226: 0x2301,  # write first staged pair word
        0x2722A: 0x8131,  # write second staged pair word
        0x27244: 0x2E31,  # clear held first word
        0x27246: 0x81E1,  # clear held second word
        0x26EBC: 0x9327,  # load 0x1000 context-state mask
        0x26EBE: 0xE054,  # context field offset 84
        0x26EC0: 0x02CE,  # context-field state update
        0x26EC2: 0x223B,
        0x26EC4: 0x0C26,
        0x26F58: 0x936E,  # second 0x1000 context-state update
        0x26F5A: 0xE054,
        0x26F5C: 0x02CE,
        0x26F5E: 0x223B,
        0x26F60: 0x0C26,
        0x27260: 0xD31F,  # load 0x0200 context-state mask
        0x27262: 0xE054,
        0x27264: 0x02CE,
        0x27266: 0x223B,
        0x2726A: 0x0C26,
        0x26B88: 0x900B,  # load context stride
        0x26B8A: 0x2E0F,  # multiply context by stride
        0x26B8C: 0xD310,  # load context-block base
        0x26EE2: 0x4408,  # item-record offset calculation
        0x26EE6: 0x343C,
        0x26EE8: 0x4408,
        0x26EEA: 0x4400,
        0x26EBE: 0xE054,  # MOV #84,R0 (context-record field)
        0x26EC0: 0x02CE,  # adjacent opcode; full operation is not decoded here
        0x2705C: 0x2FE6,  # candidate helper prologue
        0x271AC: 0x2FE6,  # candidate helper prologue
    }
    for offset, expected in expected_words.items():
        actual = read_be16(data, offset)
        if actual != expected:
            print(f"mouse_event_44_state=invalid file_offset=0x{offset:06x} "
                  f"expected=0x{expected:04x} actual="
                  f"{('none' if actual is None else f'0x{actual:04x}')}")
            valid = False

    expected_literals = {
        0x26BA2: 0x013C,  # stride-like literal
        0x26BD0: 0x0607,
        0x26BD2: 0xFBA0,  # address literal
        0x26F28: 0x0606,
        0x26F2A: 0x4610,  # address literal
        0x26F2C: 0x0604,
        0x26F2E: 0x8874,  # context eligibility masks
        0x26F0E: 0x1000,  # mask-like literal
        0x270B2: 0x013C,  # stride-like literal
        0x270B4: 0x0110,  # offset-like literal
        0x270B6: 0xFFFF,  # sentinel-like literal
        0x270B8: 0x0607,
        0x270BA: 0xFBA0,  # address literal
        0x27184: 0x00A1,  # item eligibility threshold
        0x272BA: 0x013C,  # context stride
        0x272BE: 0x0110,  # slot-array offset
        0x272C0: 0x0200,  # context dirty flag literal
        0x272C2: 0xFFFF,
        0x272C4: 0x0607,
        0x272C6: 0xFBA0,  # context table base
    }
    for offset, expected in expected_literals.items():
        actual = read_be16(data, offset)
        if actual != expected:
            print(f"mouse_event_44_literal=invalid file_offset=0x{offset:06x} "
                  f"expected=0x{expected:04x} actual="
                  f"{('none' if actual is None else f'0x{actual:04x}')}")
            valid = False

    if valid:
        print("mouse_event_44_state_evidence=slot_pair_reads_writes_and_context_stride "
              "literal_0x013c=present literal_0x0607fba0=present "
              "literal_0x0110=present literal_0x0200=present "
              "literal_0x06064610=present context_masks_0x1000_0x0200=verified "
              "slot_pair_writes=verified held_item_helpers=untraced "
              "runtime_deltas_and_firestaff_mapping=unverified")
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
    if not report_mouse_event_name_resolver(data):
        print("NEXUS_MOUSE_EVENT_NAME_RESOLVER_INVALID: retail helper changed")
        return 1
    if not report_mouse_event_queue_link(data):
        print("NEXUS_MOUSE_EVENT_QUEUE_LINK_INVALID: retail queue changed")
        return 1
    if not report_mouse_early_event_jump_table(data):
        print("NEXUS_MOUSE_EARLY_EVENT_DISPATCH_INVALID: retail switch changed")
        return 1
    if not report_mouse_event_44_state_evidence(data):
        print("NEXUS_MOUSE_EVENT_44_EVIDENCE_INVALID: retail image changed")
        return 1
    if not report_mouse_motion_event_gate(data):
        print("NEXUS_MOUSE_MOTION_EVENT_GATE_INVALID: retail poller changed")
        return 1

    print("interpretation=verified retail hit-region registration, event-44 "
          "dispatch path, and slot-pair access/writes; held-item UI effects "
          "and Firestaff field mapping remain unverified")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
