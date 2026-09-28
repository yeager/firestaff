#!/usr/bin/env python3
"""Parser regressions for legacy and frame-stamped Nexus SH-2 write traces."""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT = (Path(__file__).resolve().parents[1] /
          "scripts/analyze_nexus_sh2_ram_write_trace.py")
HEADER = "FIRESTAFF_NEXUS_SH2_RAM_WRITE_TRACE_V1"


def row(frame=None, address=0x0602C940, size=1, pc=0x06014636):
    prefix = f"frame={frame} " if frame is not None else ""
    return (f"{prefix}addr=0x{address:08x} size={size} value=0x00000001 "
            f"pc0=0x{pc:08x} pc1=0x00000000")


class NexusSh2RamWriteTraceTests(unittest.TestCase):
    def analyze(self, rows, *args):
        build_dir = SCRIPT.parents[1] / "build"
        build_dir.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="nexus-sh2-write-",
                                         dir=build_dir) as temp:
            trace = Path(temp) / "writes.trace"
            trace.write_text("\n".join((HEADER, *rows)) + "\n",
                             encoding="ascii")
            return subprocess.run(
                [sys.executable, str(SCRIPT), str(trace), *args],
                check=False, capture_output=True, text=True)

    def test_legacy_rows_remain_supported_without_frame_claim(self):
        result = self.analyze([row()])
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("runtime_writer_identity=observed", result.stdout)
        self.assertIn("retail_file_identity=unbound", result.stdout)
        self.assertNotIn("frames=", result.stdout)

    def test_frame_stamped_rows_and_inclusive_filter(self):
        result = self.analyze(
            [row(10500), row(10507, address=0x0602C944)],
            "--frame-min", "10507", "--frame-max", "10507")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("records=1", result.stdout)
        self.assertIn("frames=10507-10507", result.stdout)
        self.assertIn("address_range=0x0602c944-0x0602c944", result.stdout)

    def test_mixed_legacy_and_frame_rows_are_rejected(self):
        result = self.analyze([row(), row(10507)])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("mixed framed and legacy rows", result.stdout)

    def test_frame_filter_is_rejected_for_legacy_rows(self):
        result = self.analyze([row()], "--frame-min", "10500")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("legacy rows have no frame identity", result.stdout)

    def test_invalid_width_and_out_of_range_ram_are_rejected(self):
        for bad_row in (row(size=3), row(address=0x06100000)):
            with self.subTest(row=bad_row):
                result = self.analyze([bad_row])
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("INVALID", result.stdout)


if __name__ == "__main__":
    unittest.main()
