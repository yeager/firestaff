#!/usr/bin/env python3
"""Parser regressions for the authentic Ymir staging-to-command analyzer."""

from __future__ import annotations

import csv
import tempfile
import unittest
from pathlib import Path

import analyze_nexus_ymir_multiframe_staging_to_vdp1 as analyzer


def write_capture(dumps: Path, duplicate_slot: bool = False) -> None:
    dumps.mkdir(parents=True)
    stage = [(0xFF8B, 0x0021), (0xFFBA, 0x0021),
             (0xFFAA, 0x0028), (0xFF71, 0x0028)]
    pcs = list(analyzer.WRITE_PCS)
    write_rows = []
    for slot, ((x, y), pc) in enumerate(zip(stage, pcs)):
        write_rows.append({"sh2": "master", "vdp1_frame": "1598", "pc": pc,
                           "address": f"0x{0x0604E690 + slot * 4:x}",
                           "width": "4", "value": f"0x{(x << 16) | y:08x}"})
    if duplicate_slot:
        write_rows.append(write_rows[0].copy())
    with (dumps / "sh2-memory-write-trace.csv").open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=["sh2", "vdp1_frame", "pc", "address", "width", "value"])
        writer.writeheader()
        writer.writerows(write_rows)
    command = {f"word{index}": "0x0000" for index in range(16)}
    command.update({"frame": "1598", "index": "3", "address": "0x0100"})
    for slot, (x, y) in enumerate(stage):
        command[f"word{6 + slot * 2}"] = f"0x{x:04x}"
        command[f"word{7 + slot * 2}"] = f"0x{y:04x}"
    with (dumps / "vdp1-command-trace.csv").open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=["frame", "index", "address",
                                                    *[f"word{index}" for index in range(16)]])
        writer.writeheader()
        writer.writerow(command)
    (dumps / "sh2-memory-write-trace-bundle.txt").write_text(
        "format=ymir-sh2-memory-write-trace-bundle-v1\n"
        f"trace_file_complete=1\ntrace_rows={len(write_rows)}\nmaster_dropped=0\nslave_dropped=0\n"
        "bundle_complete=1\nsh2_ram_write_frame_min=1598\nsh2_ram_write_frame_max=1598\n"
        "sh2_ram_write_frame_begin=1598\nsh2_ram_write_frame_end_exclusive=1599\n",
        encoding="utf-8")
    (dumps / "vdp1-capture-bundle.txt").write_text(
        "format=ymir-vdp1-capture-bundle-v1\ntrace_file_complete=1\n"
        "trace_rows=1\ntrace_dropped_total=0\ntrace_first_frame=0\ntrace_last_frame=1598\n",
        encoding="utf-8")


class MultiframeStagingJoinTests(unittest.TestCase):
    def test_compares_paired_run_coordinates_by_frame_and_address(self) -> None:
        control = [{"frame": 1598, "staging_address": "0x0604e690", "staged_xy": "a"},
                   {"frame": 1598, "staging_address": "0x0604e6b0", "staged_xy": "b"}]
        up = [{"frame": 1598, "staging_address": "0x0604e690", "staged_xy": "c"},
              {"frame": 1599, "staging_address": "0x0604e6b0", "staged_xy": "b"}]
        self.assertEqual(analyzer.compare_runs(control, up), {
            "shared_staging_records": 1, "same_xy_records": 0,
            "different_xy_records": 1, "only_first_records": 1,
            "only_second_records": 1})

    def test_joins_real_trace_shape_and_classifies_geometry(self) -> None:
        with tempfile.TemporaryDirectory(prefix="ymir-multiframe-join-") as directory:
            dumps = Path(directory) / "dumps"
            write_capture(dumps)
            summary, rows = analyzer.analyze(dumps, "fixture")
        self.assertEqual(summary["retained_frames"], [1598, 1598])
        self.assertEqual(summary["per_frame"][0]["unique_geometry_joins"], 1)
        self.assertEqual(summary["per_frame"][0]["ordered"], 1)
        self.assertEqual(rows[0]["geometry_match"], "ordered")

    def test_rejects_duplicate_staging_slot(self) -> None:
        with tempfile.TemporaryDirectory(prefix="ymir-multiframe-duplicate-") as directory:
            dumps = Path(directory) / "dumps"
            write_capture(dumps, duplicate_slot=True)
            with self.assertRaisesRegex(ValueError, "duplicate staging slot"):
                analyzer.analyze(dumps, "fixture")


if __name__ == "__main__":
    unittest.main()
