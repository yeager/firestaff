#!/usr/bin/env python3
"""Unit checks for the Ymir VDP1 trace auditor."""

from __future__ import annotations

import io
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from audit_nexus_vdp1_ymir_trace import (
    TraceError,
    audit_runtime_capture,
    audit_trace,
)


COLUMNS = [
    "frame", "index", "address",
    *(f"word{i}" for i in range(16)),
    "frame_root", "local_x", "local_y",
]


def csv_row(frame: int, index: int, address: int, control: int,
            link: int = 0, local_x: int = 0, local_y: int = 0,
            word10: int = 0, word11: int = 0) -> list[str]:
    words = [0] * 16
    words[0] = control
    words[1] = link
    words[10] = word10
    words[11] = word11
    return [
        str(frame), str(index), f"0x{address:x}",
        *(f"0x{word:x}" for word in words),
        "0x0", str(local_x), str(local_y),
    ]


def encode(rows: list[list[str]]) -> bytes:
    return (",".join(COLUMNS) + "\n" + "\n".join(",".join(r) for r in rows) + "\n").encode()


class YmirVdp1TraceTests(unittest.TestCase):
    def run_trace(self, data: bytes, frame: int | None = None):
        path = Path("authentic-ymir-trace.csv")
        def open_path(_path, mode="r", *args, **kwargs):
            if mode == "rb":
                return io.BytesIO(data)
            return io.StringIO(data.decode("utf-8"), newline="")

        with patch.object(Path, "open", autospec=True, side_effect=open_path), \
             patch.object(Path, "stat", autospec=True,
                          return_value=SimpleNamespace(st_size=len(data))):
            return audit_trace(path, frame)

    def test_dispatch_order_reports_per_draw_local_and_clip_state(self):
        rows = [
            csv_row(647, 0, 0x00, 0x0009, word10=319, word11=223),
            csv_row(647, 1, 0x20, 0x000A),
            csv_row(647, 2, 0x40, 0x0002, local_x=158, local_y=107),
            csv_row(647, 3, 0x60, 0x8000, local_x=158, local_y=107),
        ]
        report = self.run_trace(encode(rows), 647)
        self.assertEqual(report["frames"]["all_dispatch_roots_zero"], True)
        self.assertEqual(report["frames"]["count"], 1)
        draw = report["selected_frame"]["draw_state"][0]
        self.assertEqual(draw["address"], "0x00040")
        self.assertEqual(draw["local"], [158, 107])
        self.assertEqual(draw["system_clip_words"], [319, 223])

    def test_call_and_return_follow_vdp1_link_stack(self):
        rows = [
            csv_row(648, 0, 0x00, 0x0009),
            csv_row(648, 1, 0x20, 0x000A),
            csv_row(648, 2, 0x40, 0x2002, link=0x14),
            csv_row(648, 3, 0xA0, 0x3000),
            csv_row(648, 4, 0x60, 0x0002),
            csv_row(648, 5, 0x80, 0x8000),
        ]
        report = self.run_trace(encode(rows), 648)
        self.assertEqual(report["selected_frame"]["records"], 6)
        self.assertEqual(report["selected_frame"]["draws"], 3)

    def test_rejects_nonzero_dispatch_root_and_broken_link(self):
        nonzero = [
            csv_row(649, 0, 0x20, 0x000A),
            csv_row(649, 1, 0x40, 0x8000),
        ]
        with self.assertRaisesRegex(TraceError, "expected VDP1 command address zero"):
            self.run_trace(encode(nonzero))

        broken = [
            csv_row(650, 0, 0x00, 0x0009),
            csv_row(650, 1, 0x40, 0x8000),
        ]
        with self.assertRaisesRegex(TraceError, "expected next address"):
            self.run_trace(encode(broken))

    def test_raw_capture_flags_a_higher_scoring_nonroot_candidate(self):
        vram = bytearray(0x80000)
        def command(offset: int, control: int, link: int = 0,
                    x: int = 0, y: int = 0) -> None:
            words = [0] * 16
            words[0] = control
            words[1] = link
            words[6] = x
            words[7] = y
            for index, word in enumerate(words):
                vram[offset + index * 2] = word & 0xFF
                vram[offset + index * 2 + 1] = word >> 8

        command(0x00, 0x0009)
        command(0x20, 0x000A, x=160, y=112)
        command(0x40, 0x0002)
        command(0x60, 0x8000)
        command(0x100, 0x0009)
        command(0x120, 0x000A, x=12, y=24)
        command(0x140, 0x0002)
        command(0x160, 0x0002)
        command(0x180, 0x1001, link=0x8)

        with patch(
            "audit_nexus_vdp1_ymir_trace.iter_frame_regions_file",
            return_value=iter([(0, {
                "vdp1-vram": bytes(vram), "vdp1-state": "copr:0x8"
            })]),
        ), patch(
            "audit_nexus_vdp1_ymir_trace._sha256_file", return_value="fixture"
        ), patch.object(
            Path, "stat", autospec=True,
            return_value=SimpleNamespace(st_size=len(vram)),
        ):
            report = audit_runtime_capture(Path("fixture.raw"), 1, 0)

        self.assertEqual(report["hardware_root_zero_chain"]["start"], "0x00000")
        self.assertEqual(report["candidate_scan_selection"]["start"], "0x00100")
        self.assertEqual(report["comparison"]["candidate_scan_starts_at_zero"], False)
        self.assertEqual(
            report["comparison"]["classification"],
            "candidate-scan-selects-a-different-chain",
        )


if __name__ == "__main__":
    unittest.main()
