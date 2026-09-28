#!/usr/bin/env python3
"""Parser-only tests for authentic Nexus SH-2 read receipts (V1/V2)."""

import importlib.util
import unittest
from pathlib import Path


SCRIPT = Path(__file__).resolve().parents[1] / "scripts/analyze_nexus_sh2_ram_read_trace.py"
SPEC = importlib.util.spec_from_file_location("nexus_sh2_ram_read_trace", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def row(frame, pc0, addr, **registers):
    values = {
        "frame": str(frame), "addr": f"0x{addr:08x}", "size": "4",
        "value": "0x00000000", "pc0": f"0x{pc0:08x}", "pc1": "0x00000000",
    }
    values.update({f"r{index}": "0x00000000" for index in range(16)})
    values.update({key: f"0x{value:08x}" for key, value in registers.items()})
    return " ".join(f"{key}={value}" for key, value in values.items())


def chain_rows(frame, table_state=0x10):
    return [
        row(frame, 0x06014388, 0x0602C90C,
            r3=0x0602C8F8, r4=0x0602C908),
        row(frame, 0x0601439A, 0x0602C91C,
            r1=0x0602C900, r4=0x0602C91C),
        row(frame, 0x06014510, 0x0602C90C,
            r0=0x20100021, r6=0x0602C908),
        row(frame, 0x0601457E, 0x0602C91C,
            r0=0x0602C918),
        row(frame, 0x0601462C, 0x0602C940,
            r4=table_state),
    ]


class NexusSh2RamReadTraceTests(unittest.TestCase):
    def parse_text(self, header, rows, frame_filter=None):
        return MODULE.parse_lines(
            [header, *rows], frame_filter)

    def test_v1_single_frame_chain_remains_supported(self):
        rows = self.parse_text(
            "FIRESTAFF_NEXUS_SH2_RAM_READ_TRACE_V1",
            [line + " pr=0x06014500" for line in chain_rows(10511)])
        self.assertEqual(MODULE.validate_controller_consumer_chain(rows), 10511)

    def test_pipeline_filter_keeps_only_master_reads_in_retail_corridor(self):
        rows = [
            {"pc0": 0x06014388, "pc1": 0},
            {"pc0": 0x0601462C, "pc1": 0},
            {"pc0": 0x06015100, "pc1": 0},
            {"pc0": 0x06014388, "pc1": 0x06014000},
        ]
        self.assertEqual(MODULE.select_pipeline_rows(rows), rows[:2])
        self.assertEqual(MODULE.select_pipeline_rows(
            rows, 0x06015000, 0x06015200), rows[2:3])

    def test_pipeline_filter_rejects_invalid_pc_ranges(self):
        with self.assertRaises(SystemExit):
            MODULE.select_pipeline_rows([], 0x06015000, 0x06015000)

    def test_table_reader_state_is_reported_without_assuming_button_value(self):
        for state in (0, 0x10):
            with self.subTest(state=state):
                rows = self.parse_text(
                    "FIRESTAFF_NEXUS_SH2_RAM_READ_TRACE_V2",
                    [line + " cpu=0" for line in chain_rows(10511, state)])
                self.assertEqual(MODULE.validate_controller_consumer_chain(rows),
                                 10511)
                self.assertEqual(MODULE.summarize_table_reader(rows), state)

    def test_v2_register_owner_format_and_frame_selection(self):
        first = [line + " cpu=0" for line in chain_rows(10511)]
        second = [line.replace("frame=10511", "frame=10512") + " cpu=0"
                  for line in chain_rows(10512)]
        rows = self.parse_text(
            "FIRESTAFF_NEXUS_SH2_RAM_READ_TRACE_V2", first + second, 10511)
        self.assertEqual(len(rows), 5)
        self.assertEqual(MODULE.validate_controller_consumer_chain(rows), 10511)

    def test_v2_rejects_slave_register_owner(self):
        line = chain_rows(10511)[0] + " cpu=1"
        with self.assertRaises(SystemExit):
            self.parse_text("FIRESTAFF_NEXUS_SH2_RAM_READ_TRACE_V2", [line])

    def test_v2_multi_frame_receipt_requires_selection(self):
        rows = [line + " cpu=0" for line in chain_rows(10511)]
        rows += [line.replace("frame=10511", "frame=10512") + " cpu=0"
                 for line in chain_rows(10512)]
        parsed = self.parse_text(
            "FIRESTAFF_NEXUS_SH2_RAM_READ_TRACE_V2", rows)
        with self.assertRaises(SystemExit):
            MODULE.validate_controller_consumer_chain(parsed)


if __name__ == "__main__":
    unittest.main()
