#!/usr/bin/env python3
"""Regression checks for authenticated Nexus FONT256 retail identities."""

from __future__ import annotations

import unittest

from analyze_nexus_vdp2_char_source_join import FONT256_SHA256
from fixtures.nexus_v1_disc_file_hashes import DISC_HASH


class Font256RetailIdentityTests(unittest.TestCase):
    def test_japanese_disc_font_hash_is_accepted(self) -> None:
        self.assertEqual(
            DISC_HASH["FONT256.S2D"],
            "b820d606b4de4fbaa21d4e32f1df56b4cce6898939fb04f73cb6f55f4ebd13af",
        )
        self.assertIn(DISC_HASH["FONT256.S2D"], FONT256_SHA256)

    def test_european_hash_remains_accepted_and_unknown_hash_is_rejected(self) -> None:
        self.assertIn(
            "764a2d6ce11b463817f5c1f2dfefbf55ff9221a1362cb5e4366998100d8ff3bb",
            FONT256_SHA256,
        )
        self.assertNotIn("0" * 64, FONT256_SHA256)


if __name__ == "__main__":
    unittest.main()
