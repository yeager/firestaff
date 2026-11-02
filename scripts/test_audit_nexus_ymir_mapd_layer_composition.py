#!/usr/bin/env python3
"""Regression tests for same-pass MAPD layer enablement checks."""

from __future__ import annotations

import csv
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))

import audit_nexus_ymir_mapd_layer_composition as audit


class RenderStateTests(unittest.TestCase):
    def write_inputs(self, root: Path, disabled_row: int | None = None) -> tuple[Path, Path]:
        state_path = root / "render-state.csv"
        receipt_path = root / "render-state.csv.receipt.txt"
        fields = (
            "frame_index", "vdp2_frame_serial", "row_vcnt",
            "color_calculation_ratio", "nbg0_enabled", "layer_enabled",
            "rbg1_enabled",
        )
        with state_path.open("w", newline="", encoding="utf-8") as stream:
            writer = csv.DictWriter(stream, fieldnames=fields)
            writer.writeheader()
            for y in range(audit.HEIGHT):
                enabled = int(y != disabled_row)
                writer.writerow({
                    "frame_index": audit.TARGET_FRAME,
                    "vdp2_frame_serial": audit.TARGET_SERIAL,
                    "row_vcnt": y,
                    "color_calculation_ratio": 18,
                    "nbg0_enabled": enabled,
                    "layer_enabled": enabled,
                    "rbg1_enabled": 0,
                })
        receipt_path.write_text(
            "frame_range_half_open=891:892\n"
            "row_vcnt_range_half_open=0:224\n"
            "write_error=0\n"
            "rendered_frame_output_count=2\n",
            encoding="utf-8",
        )
        return state_path, receipt_path

    def test_requires_active_nbg0_for_every_target_row(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            state_path, receipt_path = self.write_inputs(Path(temp_dir))
            ratios, enabled, unexpected = audit.read_render_state(state_path, receipt_path)

        self.assertEqual(len(ratios), audit.HEIGHT)
        self.assertTrue(all(enabled))
        self.assertEqual(unexpected, [])

    def test_rejects_stale_nbg0_layer_buffer_when_disabled(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            state_path, receipt_path = self.write_inputs(Path(temp_dir), disabled_row=84)
            with self.assertRaisesRegex(audit.AuditError, "not an enabled NBG0 row at 84"):
                audit.read_render_state(state_path, receipt_path)


if __name__ == "__main__":
    unittest.main()
