#!/usr/bin/env python3
"""Regression tests for Ymir row-state/register snapshot comparisons."""

from __future__ import annotations

import csv
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))

import audit_nexus_ymir_render_state_register_coherence as audit


class CoherenceAuditTests(unittest.TestCase):
    def test_reports_frame_serial_match_with_opposite_nbg0_state(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            state = root / "render-state.csv"
            index = root / "index.csv"
            dump_dir = root / "registers"
            dump_dir.mkdir()
            with state.open("w", newline="", encoding="utf-8") as stream:
                writer = csv.DictWriter(stream, fieldnames=(
                    "frame_index", "vdp2_frame_serial", "row_vcnt", "nbg0_enabled",
                ))
                writer.writeheader()
                writer.writerows((
                    {"frame_index": 7, "vdp2_frame_serial": 11,
                     "row_vcnt": y, "nbg0_enabled": 1}
                    for y in range(2)
                ))
            with index.open("w", newline="", encoding="utf-8") as stream:
                writer = csv.DictWriter(stream, fieldnames=(
                    "vdp1_frame", "vdp2_frame_serial", "file_prefix",
                ))
                writer.writeheader()
                writer.writerow({
                    "vdp1_frame": 7,
                    "vdp2_frame_serial": 11,
                    "file_prefix": "vdp1-7-vdp2-11",
                })
            registers = bytearray(512)
            registers[0x20:0x22] = b"\x00\x02"
            (dump_dir / "vdp1-7-vdp2-11-vdp2-registers.bin").write_bytes(registers)

            result = audit.audit(state, index, dump_dir)

        self.assertEqual(result["result"], "MISMATCH")
        self.assertEqual(result["disagreement_count"], 1)
        self.assertEqual(result["disagreements"][0]["render_state_nbg0_enabled"], [1])

    def test_rejects_missing_snapshot_for_render_pass(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            state = root / "render-state.csv"
            index = root / "index.csv"
            dump_dir = root / "registers"
            dump_dir.mkdir()
            state.write_text(
                "frame_index,vdp2_frame_serial,row_vcnt,nbg0_enabled\n7,11,0,1\n",
                encoding="utf-8",
            )
            index.write_text("vdp1_frame,vdp2_frame_serial,file_prefix\n", encoding="utf-8")

            with self.assertRaisesRegex(audit.AuditError, "missing indexed snapshots"):
                audit.audit(state, index, dump_dir)


if __name__ == "__main__":
    unittest.main()
