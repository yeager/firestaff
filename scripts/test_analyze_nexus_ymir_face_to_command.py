#!/usr/bin/env python3
"""Focused regressions for winding-preserving DGN face row matching."""

from __future__ import annotations

import unittest

import analyze_nexus_ymir_face_to_command as analyzer


class CyclicFaceMatchTests(unittest.TestCase):
    def test_accepts_rotation_without_changing_winding(self) -> None:
        self.assertTrue(analyzer.face_indices_match_cyclically(
            (107, 104, 105, 106), (105, 106, 107, 104)))

    def test_rejects_reversed_winding(self) -> None:
        self.assertFalse(analyzer.face_indices_match_cyclically(
            (1, 2, 3, 4), (1, 4, 3, 2)))

    def test_rejects_non_quadrilateral_rows(self) -> None:
        self.assertFalse(analyzer.face_indices_match_cyclically((1, 2, 3), (1, 2, 3)))


if __name__ == "__main__":
    unittest.main()
