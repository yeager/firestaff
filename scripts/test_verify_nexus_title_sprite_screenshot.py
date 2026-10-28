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

from verify_nexus_title_sprite_screenshot import verify


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
        lines = report.splitlines()
        for source_offset, palette, matches in (
                ("0x316", "0x2f6", "313/313"),
                ("0x1b36", "0x1b16", "186/186"),
                ("0x6a6", "0x2f6", "293/293"),
                ("0x7d6", "0x2f6", "463/463")):
            self.assertTrue(any(
                f"title_bin_offset={source_offset}" in line and
                f"palette_title_bin_offsets={palette}" in line and
                f"screenshot_matches={matches}" in line
                for line in lines),
                f"missing source/palette/pixel receipt for {source_offset}: {report}")
        self.assertEqual(sum("title_bin_offset=" in line for line in lines), 5)
        self.assertIn("menu_state=japanese-continue", report)
        self.assertIn(
            "sprite_priority=5 ratio=31/32 lower_layer=VDP2_back_screen", report)

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


if __name__ == "__main__":
    unittest.main()
