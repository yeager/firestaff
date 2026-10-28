#!/usr/bin/env python3
"""Verify the Japanese Nexus menu screenshot using only supplied retail media.

Set the FIRESTAFF_NEXUS_JP_* variables to the authentic CUE, retained
capture directory, and screenshot. Optionally set
FIRESTAFF_NEXUS_INCOMPLETE_BUNDLE to an authentic partial capture. No game or
capture bytes are generated or checked into the repository; tests skip when
the original inputs are absent.
"""

from __future__ import annotations

import contextlib
import io
import os
import unittest
from pathlib import Path

from verify_nexus_title_sprite_screenshot import same_capture_run, verify


def _path_from_env(name: str) -> Path | None:
    value = os.environ.get(name)
    return Path(value) if value else None


class AuthenticJapaneseContinueTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bundle = _path_from_env("FIRESTAFF_NEXUS_JP_CONTINUE_BUNDLE")
        cls.screenshot = _path_from_env("FIRESTAFF_NEXUS_JP_CONTINUE_SCREENSHOT")
        cls.cue = _path_from_env("FIRESTAFF_NEXUS_JP_CUE")
        if not all((cls.bundle, cls.screenshot, cls.cue)):
            raise unittest.SkipTest(
                "authentic Japanese CUE/BIN and retained Continue capture not supplied")
        for path in (cls.bundle, cls.screenshot, cls.cue):
            if not path.exists():
                raise unittest.SkipTest(f"authentic input is unavailable: {path.name}")

    def test_all_five_japanese_continue_draws_match_real_capture(self):
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            verify(self.bundle, self.screenshot, self.cue)
        report = output.getvalue()
        for expected in (
                "title_bin_offset=0x316 screenshot_matches=313/313",
                "title_bin_offset=0x1b36 screenshot_matches=186/186",
                "title_bin_offset=0x6a6 screenshot_matches=293/293",
                "title_bin_offset=0x7d6 screenshot_matches=463/463",
                "sprite_priority=5 ratio=31/32 lower_layer=VDP2_back_screen"):
            self.assertIn(expected, report)

    def test_english_retail_media_is_not_misreported_as_japanese_capture(self):
        english_cue = _path_from_env("FIRESTAFF_NEXUS_EN_CUE")
        if not english_cue or not english_cue.exists():
            self.skipTest("authentic English CUE/BIN not supplied")
        with self.assertRaisesRegex(ValueError, "only the authenticated Japanese"):
            verify(self.bundle, self.screenshot, english_cue)

    def test_rejects_authentic_partial_capture(self):
        incomplete_bundle = _path_from_env("FIRESTAFF_NEXUS_INCOMPLETE_BUNDLE")
        if not incomplete_bundle or not incomplete_bundle.exists():
            self.skipTest("authentic partial VDP capture not supplied")
        with self.assertRaises(FileNotFoundError):
            verify(incomplete_bundle, self.screenshot, self.cue)


class CaptureRunReceiptTests(unittest.TestCase):
    def test_run_log_and_receipt_report_same_run_without_frame_claim(self):
        import tempfile

        repo_root = Path(__file__).resolve().parents[1]
        scratch_root = repo_root / "build-pr153-capture-run-test"
        created_scratch_root = not scratch_root.exists()
        scratch_root.mkdir(exist_ok=True)
        try:
            with tempfile.TemporaryDirectory(
                    prefix="nexus-capture-run-", dir=scratch_root) as directory:
                root = Path(directory)
                bundle = root / "dumps"
                bundle.mkdir()
                screenshot = root / "screenshots" / "menu.png"
                screenshot.parent.mkdir()
                screenshot.write_bytes(b"test screenshot path")
                run_log = root / "run.log"
                run_log.write_text(
                    f"Screenshot saved to {screenshot}\n", encoding="utf-8")
                (bundle / "vdp1-capture-bundle.txt").write_text(
                    "format=ymir-vdp1-capture-bundle-v1\n"
                    "trace_file_complete=1\ntrace_dropped_total=0\n"
                    "capture_frame_index=8047\n", encoding="ascii")
                (bundle / "vdp-register-snapshot.txt").write_text(
                    "vdp1_frame_index=8047\n", encoding="ascii")

                status = same_capture_run(bundle, screenshot, run_log, 8047)
        finally:
            if created_scratch_root:
                scratch_root.rmdir()

        self.assertEqual(status, "same_capture_run")


if __name__ == "__main__":
    unittest.main()
