#!/usr/bin/env python3
"""Regression tests for the VDP1 write-trace command-line analyzer."""

from __future__ import annotations

import io
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from unittest.mock import patch

import analyze_nexus_vdp1_write_trace


HEADER_V1 = "FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V1"
HEADER_V2 = "FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V2"
WRITE_A = "addr=0x100 size=2 value=0x1234 pc0=0x06001000 pc1=0x00000000"
WRITE_B = "addr=0x200 size=1 value=0x0056 pc0=0x06002000 pc1=0x00000000"


def run_analyzer(arguments: list[str], lines: list[str]) -> tuple[int, str]:
    output = io.StringIO()
    with patch("sys.argv", ["analyze_nexus_vdp1_write_trace.py", *arguments]), \
         patch.object(Path, "read_text", return_value="\n".join(lines)), \
         redirect_stdout(output):
        result = analyze_nexus_vdp1_write_trace.main()
    return result, output.getvalue()


class Vdp1WriteTraceTests(unittest.TestCase):
    def test_frame_selection_does_not_mix_in_pre_capture_records(self) -> None:
        result, output = run_analyzer(
            ["trace", "--frame", "0"],
            [HEADER_V2, WRITE_A, "frame=0", WRITE_B],
        )
        self.assertEqual(result, 0)
        self.assertIn("pre_capture_records=1", output)
        self.assertIn("selected_frame=0", output)
        self.assertIn("pc0_counts=0x06002000:1", output)

    def test_pre_capture_records_can_be_selected_explicitly(self) -> None:
        result, output = run_analyzer(
            ["trace", "--pre-capture", "--require-address", "0x100"],
            [HEADER_V2, WRITE_A, "frame=0", WRITE_B],
        )
        self.assertEqual(result, 0)
        self.assertIn("selected_frame=pre-capture", output)
        self.assertIn("pc0_counts=0x06001000:1", output)
        self.assertIn("required_matches=1", output)

    def test_pre_capture_selection_rejects_v1(self) -> None:
        result, output = run_analyzer(["trace", "--pre-capture"],
                                      [HEADER_V1, WRITE_A])
        self.assertEqual(result, 1)
        self.assertIn("--pre-capture requires V2", output)


if __name__ == "__main__":
    unittest.main()
