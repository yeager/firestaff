#!/usr/bin/env python3
"""Regression tests for the real-media Nexus snapshot join analyzer."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from analyze_nexus_menu_bpk_vdp1_snapshot_join import (
    all_offsets,
    command_trace_source_starts,
    surface_offsets,
)

BUILD = Path(__file__).resolve().parents[1] / "build"
BUILD.mkdir(exist_ok=True)


class SnapshotJoinTests(unittest.TestCase):
    def test_enumerates_overlapping_direct_and_word_swapped_occurrences(self) -> None:
        direct, swapped = surface_offsets(b"\x12\x34\x12\x34", b"\x12\x34")
        self.assertEqual(direct, [0, 2])
        self.assertEqual(swapped, [1])
        self.assertEqual(all_offsets(b"aaaa", b"aa"), [0, 1, 2])

    def test_uses_cmdsrca_word4_and_ignores_untextured_commands(self) -> None:
        trace = (
            "frame,index,address,word0,word2,word4\n"
            "0,0,0x0,0x0,0x0,0x20\n"
            "0,1,0x20,0x4,0x20,0x0\n"
            "0,2,0x40,0x5000,0x0,0x20\n"
        )
        with tempfile.TemporaryDirectory(prefix="nexus-menu-vdp1-trace-", dir=BUILD) as directory:
            path = Path(directory) / "trace.csv"
            path.write_text(trace, encoding="utf-8")
            self.assertEqual(command_trace_source_starts(path, [(0x100, 0x110)]),
                             (1, 1))

    def test_rejects_empty_missing_header_and_header_only_traces(self) -> None:
        invalid_traces = ("", "frame,index\n", "word0,word4\n")
        with tempfile.TemporaryDirectory(prefix="nexus-menu-vdp1-invalid-", dir=BUILD) as directory:
            path = Path(directory) / "trace.csv"
            for trace in invalid_traces:
                with self.subTest(trace=trace):
                    path.write_text(trace, encoding="utf-8")
                    with self.assertRaises(ValueError):
                        command_trace_source_starts(path, [])


if __name__ == "__main__":
    unittest.main()
