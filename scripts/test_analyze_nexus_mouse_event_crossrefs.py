#!/usr/bin/env python3
"""Regression tests for the Nexus retail mouse callback trace checks."""

from __future__ import annotations

import contextlib
import importlib.util
import io
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("analyze_nexus_mouse_event_crossrefs.py")
SPEC = importlib.util.spec_from_file_location("nexus_mouse_trace", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
trace = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(trace)


def put_u16(data: bytearray, offset: int, value: int) -> None:
    data[offset : offset + 2] = value.to_bytes(2, "big")


def put_mov_l_pc(data: bytearray, offset: int, register: int,
                 value: int, literal_offset: int) -> None:
    pc_base = (trace.DM_BASE + offset + 4) & ~3
    literal_address = trace.DM_BASE + literal_offset
    displacement = (literal_address - pc_base) // 4
    assert 0 <= displacement <= 0xFF
    put_u16(data, offset, 0xD000 | (register << 8) | displacement)
    data[literal_offset : literal_offset + 4] = value.to_bytes(4, "big")


def retail_mouse_registration_fixture() -> bytearray:
    data = bytearray(0x3B000)
    for offset, register, value, literal in (
        (0x16588, 6, trace.DM_BASE + 0x164A8, 0x16680),
        (0x1658A, 5, trace.DM_BASE + 0x3850C, 0x16684),
        (0x1658C, 4, trace.DM_BASE + 0x38040, 0x16688),
        (0x1658E, 2, trace.DM_BASE + trace.MOUSE_REGION_REGISTER_ROUTINE, 0x1668C),
    ):
        put_mov_l_pc(data, offset, register, value, literal)
    put_u16(data, 0x16590, 0x420B)  # JSR @R2

    put_u16(data, 0x24AD8, 0x7FF8)  # add #-8,R15
    put_u16(data, 0x24ADC, 0x2F62)  # mov.l R6,@R15
    put_u16(data, 0x24B0A, 0x60F2)  # mov.l @R15,R0
    put_u16(data, 0x24B0C, 0x2008)  # tst R0,R0
    put_u16(data, 0x24B0E, 0x8902)  # bt 0x24B16
    put_u16(data, 0x24B10, 0x63F2)  # delay slot: mov.l @R15,R3
    put_u16(data, 0x24B12, 0xA002)  # bra 0x24B18
    put_u16(data, 0x24B14, 0x2432)  # delay slot: mov.l R3,@R4
    put_u16(data, 0x24B16, 0xE100)
    put_u16(data, 0x24B18, 0x2412)
    put_u16(data, 0x24B1A, 0x7F08)  # add #8,R15

    put_mov_l_pc(data, 0x24B08, 4, 0x0607FB90, 0x24C00)
    put_mov_l_pc(data, 0x24B26, 3, 0x0607FB60, 0x24C04)
    put_u16(data, 0x24B28, 0x6032)  # mov.l @R3,R0
    put_u16(data, 0x24B2A, 0x2008)  # tst R0,R0
    put_u16(data, 0x24B2C, 0x8922)  # bt 0x24B74
    put_mov_l_pc(data, 0x24B2E, 2, 0x0607FB90, 0x24C08)
    put_u16(data, 0x24B30, 0x6E22)  # mov.l @R2,R14
    put_u16(data, 0x24B32, 0x2EE8)  # tst R14,R14
    put_u16(data, 0x24B34, 0x8901)  # bt 0x24B3A
    put_u16(data, 0x24B36, 0x4E0B)  # JSR @R14

    # Callback object-class/submode gates and the two numeric event submits.
    for offset, opcode in {
        0x16548: 0xE064, 0x1654A: 0x00EE, 0x1654C: 0x8806,
        0x1654E: 0x8B1B, 0x16550: 0xE068, 0x16552: 0x00EE,
        0x16554: 0x8800, 0x16556: 0x8905, 0x16558: 0x8801,
        0x1655A: 0x8915, 0x1655C: 0x8802, 0x1655E: 0x890A,
        0x1656A: 0xE40D, 0x1657A: 0xE507, 0x1657E: 0xE42C,
    }.items():
        put_u16(data, offset, opcode)
    put_u16(data, trace.MOUSE_CALLBACK_MODE0_CALL, 0x432B)
    put_u16(data, trace.MOUSE_CALLBACK_MODE0_CALL + 2, 0x6EF6)
    put_u16(data, trace.MOUSE_CALLBACK_MODE2_CALL, 0x422B)
    put_u16(data, trace.MOUSE_CALLBACK_MODE2_CALL + 2, 0x6EF6)
    put_mov_l_pc(data, trace.MOUSE_CALLBACK_EVENT_DISPATCH, 3,
                 0x06024288, 0x16690)
    put_mov_l_pc(data, trace.MOUSE_CALLBACK_MODE2_DISPATCH, 2,
                 0x06024288, 0x16694)
    for event_id, (name, string_offset) in trace.MOUSE_CALLBACK_EVENT_NAMES.items():
        pointer_offset = trace.EVENT_NAME_POINTER_TABLE_OFFSET + event_id * 4
        data[pointer_offset : pointer_offset + 4] = (
            trace.DM_BASE + string_offset).to_bytes(4, "big")
        data[string_offset : string_offset + len(name) + 1] = name + b"\0"
    for index, row in enumerate(trace.MOUSE_ACTION_RECT_RECORDS):
        offset = trace.MOUSE_ACTION_RECT_TABLE_OFFSET + index * trace.MOUSE_ACTION_RECT_RECORD_SIZE
        for field, value in enumerate(row):
            put_u16(data, offset + field * 2, value)
    sentinel_offset = (trace.MOUSE_ACTION_RECT_TABLE_OFFSET +
                       trace.MOUSE_ACTION_RECT_TABLE_COUNT *
                       trace.MOUSE_ACTION_RECT_RECORD_SIZE + 8)
    put_u16(data, sentinel_offset, 0xFFFF)
    for offset, opcode in {
        0x24EE2: 0x62E1, 0x24EE4: 0x3247, 0x24EE8: 0x85E2,
        0x24EEA: 0x3407, 0x24EEE: 0x85E1, 0x24EF0: 0x3057,
        0x24EF4: 0x85E3, 0x24EF6: 0x3507, 0x24EFC: 0x85E5,
        0x24EFE: 0x6503, 0x24F00: 0x85E4, 0x24F02: 0x4D0B,
        0x24F04: 0x6403,
    }.items():
        put_u16(data, offset, opcode)
    for offset, opcode in {
        0x24EE0: 0x6E63, 0x24EE6: 0x8910, 0x24EEC: 0x890D,
        0x24EF2: 0x890A, 0x24EF8: 0x8907, 0x24F08: 0xE001,
        0x24F0A: 0x7E0C, 0x24F0C: 0x85E4, 0x24F0E: 0x4015,
        0x24F10: 0x89E7, 0x24F12: 0xE000,
    }.items():
        put_u16(data, offset, opcode)
    put_mov_l_pc(data, 0x24EDC, 13, 0x06024288, 0x25080)
    for offset, opcode in {
        0x21CAE: 0x8826, 0x21CB0: 0x8B01, 0x21CB6: 0x8827,
        0x21CB8: 0x8941, 0x21CBA: 0x8828, 0x21CBC: 0x8945,
        0x21CBE: 0x8831, 0x21CC0: 0x8949, 0x21CC2: 0x8834,
        0x21CC4: 0x8905, 0x21CC6: 0x8838, 0x21CC8: 0x894A,
        0x21CCA: 0x8839, 0x21CCC: 0x895C, 0x21CCE: 0xA063,
        0x21D9A: 0x430B, 0x21D9C: 0x64B3, 0x21D9E: 0xE519,
        0x21DAC: 0x420B, 0x21DAE: 0xE400,
    }.items():
        put_u16(data, offset, opcode)
    put_mov_l_pc(data, 0x21D98, 3, 0x060241B4, 0x21ED8)
    put_mov_l_pc(data, 0x21DA4, 3, 0x060476A4, 0x21EDC)
    put_mov_l_pc(data, 0x21DAA, 2, 0x06023DB4, 0x21EE0)
    for offset, opcode in {
        0x250EE: 0x6042, 0x250F0: 0x882E, 0x250F2: 0x8B14,
        0x2511A: 0xBEDC, 0x2511C: 0x0009,
    }.items():
        put_u16(data, offset, opcode)
    put_mov_l_pc(data, 0x250F4, 2, 0x060242D6, 0x25150)
    diagnostic_format = b"bad event data ($%x:%s)\n\0"
    data[trace.EVENT_BAD_DATA_FORMAT_OFFSET:
         trace.EVENT_BAD_DATA_FORMAT_OFFSET + len(diagnostic_format)] = diagnostic_format
    return data


def retail_event_44_evidence_fixture() -> bytearray:
    data = bytearray(0x27200)
    for offset, value in {
        0x26E58: 0x2FE6, 0x26E5E: 0x6D43, 0x26E66: 0x6E53,
        0x26B88: 0x900B, 0x26B8A: 0x2E0F, 0x26B8C: 0xD310,
        0x26EE2: 0x4408, 0x26EE6: 0x343C, 0x26EE8: 0x4408,
        0x26EEA: 0x4400, 0x26EBE: 0xE054, 0x26EC0: 0x02CE,
        0x2705C: 0x2FE6, 0x271AC: 0x2FE6,
        0x26BA2: 0x013C, 0x26BD0: 0x0607, 0x26BD2: 0xFBA0,
        0x26F28: 0x0606, 0x26F2A: 0x4610, 0x26F2C: 0x0604,
        0x26F2E: 0x8874, 0x26F0E: 0x1000, 0x270B2: 0x013C,
        0x270B4: 0x0110, 0x270B6: 0xFFFF, 0x270B8: 0x0607,
        0x270BA: 0xFBA0, 0x27184: 0x00A1,
    }.items():
        put_u16(data, offset, value)
    return data


def retail_event_queue_fixture() -> bytearray:
    data = bytearray(0x21E00)
    for offset, value in {
        0x14292: 0x8808, 0x142B6: 0x7201, 0x142C8: 0x2742,
        0x142CA: 0x1751, 0x142CC: 0x1762, 0x21DB6: 0x420B,
        0x21DB8: 0x0009, 0x1462A: 0x6032, 0x1462C: 0x2008,
        0x14630: 0xE600, 0x14632: 0x6563, 0x14634: 0xBE28,
        0x14636: 0xE411,
    }.items():
        put_u16(data, offset, value)
    for offset, register, value, literal in (
        (0x1428C, 14, 0x06079814, 0x14380),
        (0x142A2, 3, 0x06079810, 0x14384),
        (0x142B4, 1, 0x060797B0, 0x14388),
        (0x142DC, 13, 0x06079814, 0x1438C),
        (0x142E8, 14, 0x06079810, 0x14390),
        (0x142F4, 2, 0x060797B0, 0x14394),
        (0x21DB4, 2, 0x060242D6, 0x21DC4),
    ):
        put_mov_l_pc(data, offset, register, value, literal)
    return data


class NexusMouseCallbackTraceTests(unittest.TestCase):
    def report(self, data: bytes) -> tuple[bool, str]:
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            result = trace.report_mouse_region_registration(data)
        return result, output.getvalue()

    def test_registration_and_dispatch_path_is_accepted(self) -> None:
        result, output = self.report(retail_mouse_registration_fixture())
        self.assertTrue(result)
        self.assertIn("mouse_region_registration_r6_callback_slot", output)

    def test_r6_must_be_saved_as_callback_input(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, 0x24ADC, 0x2F63)
        self.assertFalse(self.report(data)[0])

    def test_registration_branch_target_is_checked(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, 0x24B0E, 0x8901)
        self.assertFalse(self.report(data)[0])

    def test_active_flag_branch_target_is_checked(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, 0x24B2C, 0x8921)
        self.assertFalse(self.report(data)[0])

    def test_indirect_callback_register_is_checked(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, 0x24B36, 0x4E0A)
        self.assertFalse(self.report(data)[0])

    def test_mode_gated_event_ids_resolve_retail_names(self) -> None:
        data = retail_mouse_registration_fixture()
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_callback_event_ids(data)
        self.assertTrue(valid)
        self.assertIn("event_mode_0_id=13", output.getvalue())
        self.assertIn("event_mode_0_name=EV_EXIT_INV", output.getvalue())
        self.assertIn("event_mode_2_id=44", output.getvalue())
        self.assertIn("event_mode_2_name=EV_INV_EQUIP", output.getvalue())
        self.assertIn("event_44_handler_semantics=unverified", output.getvalue())
        self.assertIn("firestaff_inventory_mapping=unverified", output.getvalue())

    def test_event_name_pointer_mismatch_is_rejected(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, trace.EVENT_NAME_POINTER_TABLE_OFFSET + 13 * 4,
                0x0000)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_callback_event_ids(data)
        self.assertFalse(valid)

    def test_event_submission_mode_change_is_rejected(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, 0x1657E, 0xE42D)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_callback_event_ids(data)
        self.assertFalse(valid)

    def test_callback_dispatch_call_opcode_is_checked_for_both_modes(self) -> None:
        for offset in (trace.MOUSE_CALLBACK_MODE0_CALL,
                       trace.MOUSE_CALLBACK_MODE2_CALL):
            with self.subTest(offset=offset):
                data = retail_mouse_registration_fixture()
                put_u16(data, offset, 0x420B)
                output = io.StringIO()
                with contextlib.redirect_stdout(output):
                    valid = trace.report_mouse_callback_event_ids(data)
                self.assertFalse(valid)

    def test_retail_inventory_regions_dispatch_equip_slots(self) -> None:
        data = retail_mouse_registration_fixture()
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_inventory_hit_regions(data)
        self.assertTrue(valid)
        self.assertIn("event=44(EV_INV_EQUIP)", output.getvalue())
        self.assertIn("inventory_slots_0_to_7", output.getvalue())

    def test_inventory_hit_event_id_mismatch_is_rejected(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, trace.MOUSE_ACTION_RECT_TABLE_OFFSET + 8, 13)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_inventory_hit_regions(data)
        self.assertFalse(valid)

    def test_inventory_scanner_stride_change_is_rejected(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, 0x24F0A, 0x7E08)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_inventory_hit_regions(data)
        self.assertFalse(valid)

    def test_inventory_scanner_back_edge_change_is_rejected(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, 0x24F10, 0x89E6)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_inventory_hit_regions(data)
        self.assertFalse(valid)

    def test_inventory_scanner_sentinel_change_is_rejected(self) -> None:
        data = retail_mouse_registration_fixture()
        sentinel_offset = (trace.MOUSE_ACTION_RECT_TABLE_OFFSET +
                           trace.MOUSE_ACTION_RECT_TABLE_COUNT *
                           trace.MOUSE_ACTION_RECT_RECORD_SIZE + 8)
        put_u16(data, sentinel_offset, 0x0000)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_inventory_hit_regions(data)
        self.assertFalse(valid)

    def test_event_queue_fallback_does_not_claim_gameplay_effect(self) -> None:
        data = retail_mouse_registration_fixture()
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_event_queue_fallback(data)
        self.assertTrue(valid)
        self.assertIn("mouse_event_queue_special_case_ids=38,39,40,49,52,56,57",
                      output.getvalue())
        self.assertIn("mouse_later_loop_ids_13_44=generic_bad_event_diagnostic "
                      "consumer=file_0x21c84; not_the_only_consumer",
                      output.getvalue())

    def test_event_queue_consumer_change_is_rejected(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, 0x21CCE, 0xA062)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_event_queue_fallback(data)
        self.assertFalse(valid)

    def test_shared_event_queue_link_is_checked(self) -> None:
        data = retail_event_queue_fixture()
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_event_queue_link(data)
        self.assertTrue(valid)
        self.assertIn("mouse_event_queue=shared_append_dequeue", output.getvalue())

        put_u16(data, 0x142C8, 0x0000)
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertFalse(trace.report_mouse_event_queue_link(data))

    def test_event_44_selected_handler_evidence_is_checked_without_claiming_effects(self) -> None:
        data = retail_event_44_evidence_fixture()
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_event_44_state_evidence(data)
        self.assertTrue(valid)
        self.assertIn("selected_opcodes_and_literals", output.getvalue())
        self.assertIn("control_flow_and_side_effects=unverified", output.getvalue())

        put_u16(data, 0x26EC0, 0x0000)
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertFalse(trace.report_mouse_event_44_state_evidence(data))

    def test_event_44_dispatch_pointer_is_checked_but_effects_stay_unverified(self) -> None:
        data = bytearray(0x27200)
        for offset, value in {
            0x286: 0x70FE, 0x288: 0xE13B, 0x28A: 0x3012,
            0x28C: 0x8B01, 0x292: 0x4000, 0x294: 0x6103,
            0x296: 0xC707, 0x298: 0x001D, 0x29A: 0x0023,
            0x29C: 0x0009, 0x8C0: 0x55FE, 0x8C4: 0x410B,
            0x8C6: 0x0009, 0x26E58: 0x2FE6, 0x26E5E: 0x6D43,
            0x26E66: 0x6E53,
        }.items():
            put_u16(data, offset, value)
        put_u16(data, 0x308, 0x0622)
        put_mov_l_pc(data, 0x8C2, 1, 0x06036E58, 0x9F0)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_early_event_jump_table(data)
        self.assertTrue(valid)
        self.assertIn("r5_load=stack_offset_56 provenance=untraced",
                      output.getvalue())
        self.assertIn("handler_side_effects=unverified", output.getvalue())

        put_u16(data, 0x308, 0x0000)
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertFalse(trace.report_mouse_early_event_jump_table(data))

    def test_mouse_poller_filters_to_event_46_before_hit_scan(self) -> None:
        data = retail_mouse_registration_fixture()
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_motion_event_gate(data)
        self.assertTrue(valid)
        self.assertIn("event_46_only_then_retail_hit_scanner", output.getvalue())

    def test_mouse_poller_event_filter_change_is_rejected(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, 0x250F0, 0x882D)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_motion_event_gate(data)
        self.assertFalse(valid)


if __name__ == "__main__":
    unittest.main()
