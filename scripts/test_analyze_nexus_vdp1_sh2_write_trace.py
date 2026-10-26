#!/usr/bin/env python3
"""Regression tests for validating captured VDP1 writes."""

from __future__ import annotations

import contextlib
import csv
import io
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import analyze_nexus_vdp1_sh2_write_trace


FIELDS = ("sh2", "pc", "caller_return_address", "caller_target",
          "address", "width", "value")


def run_trace(width: int, address: int = 0x05C00000) -> tuple[int, str]:
    output = io.StringIO()
    with tempfile.TemporaryDirectory(prefix="nexus-vdp1-sh2-trace-") as directory:
        trace = Path(directory) / "trace.csv"
        with trace.open("w", newline="", encoding="utf-8") as handle:
            writer = csv.DictWriter(handle, fieldnames=FIELDS)
            writer.writeheader()
            writer.writerow({"sh2": "master", "pc": "0x06001000",
                             "caller_return_address": "0", "caller_target": "0",
                             "address": hex(address), "width": str(width),
                             "value": "0"})
        with patch("sys.argv", ["analyze_nexus_vdp1_sh2_write_trace.py", str(trace)]), \
             contextlib.redirect_stdout(output):
            result = analyze_nexus_vdp1_sh2_write_trace.main()
    return result, output.getvalue()


class Vdp1Sh2TraceTests(unittest.TestCase):
    def test_accepts_supported_widths(self) -> None:
        for width in (1, 2, 4):
            with self.subTest(width=width):
                result, output = run_trace(width)
                self.assertEqual(result, 0)
                self.assertIn("vdp1_write_rows=1", output)
                self.assertIn("vdp1_unique_write_addresses=1", output)
                self.assertIn(f"vdp1_unique_written_bytes={width}", output)

    def test_rejects_zero_negative_and_unsupported_widths(self) -> None:
        for width in (0, -1, 3):
            with self.subTest(width=width):
                result, output = run_trace(width)
                self.assertEqual(result, 1)
                self.assertIn("invalid width", output)

    def test_rejects_addresses_at_or_beyond_vram_end(self) -> None:
        for address in (0x05C80000, 0x05C7FFFF):
            with self.subTest(address=address):
                result, output = run_trace(2, address)
                self.assertEqual(result, 1)
                self.assertIn("outside VDP1 VRAM", output)


if __name__ == "__main__":
    unittest.main()
