#!/usr/bin/env python3
"""Tests for the raw Ymir high-WRAM inventory pair analyzer."""

from __future__ import annotations

import contextlib
import io
import struct
import tempfile
import unittest
from pathlib import Path

import analyze_nexus_inventory_wram_pair as analyzer


class InventoryWramPairTests(unittest.TestCase):
    def test_reports_event44_queue_and_slot_pair_delta(self) -> None:
        before = bytearray(analyzer.WRAM_SIZE)
        after = bytearray(analyzer.WRAM_SIZE)
        struct.pack_into(">III", after, analyzer.EVENT_QUEUE_OFFSET, 44, 3, 0)
        slot_offset = (
            analyzer.CONTEXT_BASE_OFFSET
            + 2 * analyzer.CONTEXT_STRIDE
            + analyzer.CONTEXT_ITEM_PAIRS_OFFSET
            + 4 * 4
        )
        struct.pack_into(">HH", before, slot_offset, 0xFFFF, 0)
        struct.pack_into(">HH", after, slot_offset, 0x0014, 0x4000)

        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            result = analyzer.analyze(bytes(before), bytes(after), 2, 1)

        self.assertEqual(result, 0)
        text = output.getvalue()
        self.assertIn("after_event44_present_in_queue_storage=yes", text)
        self.assertIn("context[2].slot[4]", text)
        self.assertIn("empty/aux=0x0000 -> 0x0014/aux=0x4000", text)
        self.assertIn("changed_item_slots=1", text)
        self.assertIn("click causality", text)

    def test_rejects_wrong_capture_size(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "too-small.bin"
            path.write_bytes(b"not-a-high-wram-dump")
            with self.assertRaisesRegex(analyzer.CaptureError, "expected 1048576 bytes"):
                analyzer.read_capture(path)

    def test_identical_captures_are_explicitly_flagged(self) -> None:
        capture = bytes(analyzer.WRAM_SIZE)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            analyzer.analyze(capture, capture, 0, 1)
        self.assertIn("capture_warning=before and after snapshots are byte-identical", output.getvalue())


if __name__ == "__main__":
    unittest.main()
