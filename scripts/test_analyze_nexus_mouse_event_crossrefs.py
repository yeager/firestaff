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
    data = bytearray(0x39000)
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
    put_mov_l_pc(data, trace.MOUSE_CALLBACK_EVENT_DISPATCH, 3,
                 0x06024288, 0x16690)
    put_mov_l_pc(data, trace.MOUSE_CALLBACK_MODE2_DISPATCH, 2,
                 0x06024288, 0x16694)
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

    def test_mode_gated_event_ids_are_reported_without_names(self) -> None:
        data = retail_mouse_registration_fixture()
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_callback_event_ids(data)
        self.assertTrue(valid)
        self.assertIn("event_mode_0_id=13", output.getvalue())
        self.assertIn("event_mode_2_id=44", output.getvalue())
        self.assertIn("event_names_and_gameplay_effects=unbound", output.getvalue())

    def test_event_submission_mode_change_is_rejected(self) -> None:
        data = retail_mouse_registration_fixture()
        put_u16(data, 0x1657E, 0xE42D)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            valid = trace.report_mouse_callback_event_ids(data)
        self.assertFalse(valid)


if __name__ == "__main__":
    unittest.main()
