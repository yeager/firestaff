#!/usr/bin/env python3
"""Regression tests for hash-authenticated regional VDP1 source scans."""

from __future__ import annotations

import unittest
import csv
import tempfile
from pathlib import Path

from analyze_nexus_title_vdp1_geometry import geometry_for_chain
from analyze_nexus_vdp1_command_sequence import Record, describe
from analyze_nexus_vdp1_source_join import (
    accepted_retail_hashes,
    aggregate_source_join_status,
    has_informative_source,
    swapped_words,
    texture_dimensions,
    ymir_commands,
)

BUILD = Path(__file__).resolve().parents[1] / "build"
BUILD.mkdir(exist_ok=True)


class AcceptedRetailHashesTests(unittest.TestCase):
    def test_title_geometry_ignores_skipped_local_and_draw_commands(self) -> None:
        def command(offset: int, control: int, words=None) -> Record:
            payload = list(words or [0] * 16)
            payload[0] = control
            return Record(offset, tuple(payload))

        skipped_local = [0] * 16
        skipped_local[6:8] = [100, 200]
        local = [0] * 16
        local[6:8] = [10, 20]
        skipped_draw = [0] * 16
        skipped_draw[6:14] = [30, 40, 50, 60, 70, 80, 90, 100]
        draw = [0] * 16
        draw[6:14] = [1, 2, 3, 4, 5, 6, 7, 8]
        chain = [
            command(0x00, 0x400A, skipped_local),
            command(0x20, 0x4002, skipped_draw),
            command(0x40, 0x000A, local),
            command(0x60, 0x0002, draw),
            command(0x80, 0x8000),
        ]
        distorted, vertices, local_updates = geometry_for_chain(chain)
        self.assertEqual(distorted, 1)
        self.assertEqual(vertices, [(11, 22), (13, 24), (15, 26), (17, 28)])
        self.assertEqual(local_updates, [(0x40, 10, 20)])

    def test_command_summary_excludes_skipped_commands_from_active_state(self) -> None:
        def command(offset: int, control: int) -> Record:
            return Record(offset, (control,) + (0,) * 15)

        report = describe([
            command(0x00, 0x4000),       # skipped draw
            command(0x20, 0x400A),       # skipped local-coordinate command
            command(0x40, 0x000A),       # active local-coordinate command
            command(0x60, 0x0000),       # active draw
            command(0x80, 0x8000),       # END
        ], 0)
        self.assertEqual(report["draw_count"], 1)
        self.assertEqual(report["local_coordinate_count"], 1)
        self.assertEqual(report["types"],
                         ["SKIP 0x0", "SKIP 0xa", "0xa", "0x0", "END"])

    def test_zero_filled_texture_is_not_source_identity(self) -> None:
        self.assertFalse(has_informative_source(bytes(448)))
        self.assertTrue(has_informative_source(b"\x00\x01"))

    def test_saturn_cmdsize_dimensions(self) -> None:
        self.assertEqual(texture_dimensions(0x0410), (32, 16))

    def test_ymir_bundle_checks_frame_and_rejects_dropped_rows(self) -> None:
        with tempfile.TemporaryDirectory(dir=BUILD) as directory:
            root = Path(directory)
            receipt = root / "vdp1-capture-bundle.txt"
            receipt.write_text("format=ymir-vdp1-capture-bundle-v1\n"
                               "trace_file_complete=1\ntrace_dropped_total=0\n"
                               "capture_frame_index=7\ntrace_rows=1\n"
                               "trace_rows_for_capture_frame=1\n", encoding="ascii")
            fields = ["frame", "index", "address"] + [f"word{i}" for i in range(16)]
            with (root / "vdp1-command-trace.csv").open("w", newline="", encoding="ascii") as stream:
                writer = csv.DictWriter(stream, fieldnames=fields)
                writer.writeheader()
                writer.writerow({**{f"word{i}": "0" for i in range(16)},
                                 "word0": "0x8000", "frame": "7", "index": "0", "address": "0x20"})
            (root / "vdp1-vram.bin").write_bytes(bytes(512 * 1024))
            _, frame, commands = ymir_commands(root)
            self.assertEqual((frame, commands[0][0]), (7, 0x20))
            trace = root / "vdp1-command-trace.csv"
            trace.write_text(trace.read_text().replace("0x8000", "0x1000"))
            with self.assertRaises(ValueError):
                ymir_commands(root)
            receipt.write_text(receipt.read_text().replace("trace_dropped_total=0", "trace_dropped_total=1"))
            with self.assertRaises(ValueError):
                ymir_commands(root)

    def test_menu_bpk_accepts_only_the_three_verified_regions(self) -> None:
        self.assertEqual(
            accepted_retail_hashes("MENU.BPK"),
            frozenset({
                "740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636",
                "f2f78dddfe37a5ff414775ae888f164624e987059934b034ba36299cc769d2ca",
                "c4e2427f54083e92cdf38f3b1f296e135bdb007de227431be690cc41381fd543",
            }),
        )
        self.assertNotIn("0" * 64, accepted_retail_hashes("MENU.BPK"))

    def test_other_resources_retain_their_known_identity(self) -> None:
        self.assertEqual(
            accepted_retail_hashes("TITLE.BIN"),
            frozenset({
                "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3",
                "a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44",
            }),
        )

    def test_unknown_resource_has_no_filename_only_identity(self) -> None:
        self.assertEqual(accepted_retail_hashes("UNLISTED.BIN"), frozenset())

    def test_word_swap_reverses_words_and_preserves_a_trailing_byte(self) -> None:
        self.assertEqual(swapped_words(b"\x12\x34\xab\xcd"),
                         b"\x34\x12\xcd\xab")
        self.assertEqual(swapped_words(b"\x12\x34\xab"), b"\x34\x12\xab")

    def test_summary_requires_every_draw_for_complete_join(self) -> None:
        self.assertEqual(aggregate_source_join_status(0, 0), "no_draws")
        self.assertEqual(aggregate_source_join_status(3, 0), "unbound")
        self.assertEqual(aggregate_source_join_status(3, 2), "partial")
        self.assertEqual(aggregate_source_join_status(3, 3), "complete")


if __name__ == "__main__":
    unittest.main()
