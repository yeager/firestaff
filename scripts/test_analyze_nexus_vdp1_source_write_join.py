#!/usr/bin/env python3
"""Regression tests for frame-bounded VDP1 source-write joins."""

from __future__ import annotations

import unittest

from analyze_nexus_vdp1_source_write_join import parse_trace_lines


HEADER_V1 = "FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V1"
HEADER_V2 = "FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V2"
WRITE_A = "addr=0x100 size=2 value=0x1234 pc0=0x06001000 pc1=0x00000000"
WRITE_B = "addr=0x200 size=1 value=0x0056 pc0=0x06002000 pc1=0x00000000"


class SourceWriteTraceTests(unittest.TestCase):
    def test_v2_keeps_pre_capture_writes_separate_from_selected_frame(self) -> None:
        prefix, frame = parse_trace_lines(
            [HEADER_V2, WRITE_A, "frame=0", WRITE_B, "frame=1", WRITE_A], 1)
        self.assertEqual(prefix, [(0x100, 2, 0x06001000, 0)])
        self.assertEqual(frame, [(0x100, 2, 0x06001000, 0)])

    def test_v2_unselected_trace_includes_each_frame_after_prefix(self) -> None:
        prefix, rows = parse_trace_lines(
            [HEADER_V2, WRITE_A, "frame=0", WRITE_B, "frame=1", WRITE_A])
        self.assertEqual(prefix, [(0x100, 2, 0x06001000, 0)])
        self.assertEqual(rows, [
            (0x200, 1, 0x06002000, 0),
            (0x100, 2, 0x06001000, 0),
        ])

    def test_v1_rows_remain_unframed(self) -> None:
        prefix, rows = parse_trace_lines([HEADER_V1, WRITE_A])
        self.assertEqual(prefix, [])
        self.assertEqual(rows, [(0x100, 2, 0x06001000, 0)])

    def test_malformed_prefix_and_v1_frame_marker_fail_closed(self) -> None:
        with self.assertRaises(ValueError):
            parse_trace_lines([HEADER_V2, "not a write"])
        with self.assertRaises(ValueError):
            parse_trace_lines([HEADER_V1, "frame=0"])


if __name__ == "__main__":
    unittest.main()
