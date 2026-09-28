#!/usr/bin/env python3
"""Regression tests for hash-authenticated regional VDP1 source scans."""

from __future__ import annotations

import unittest

from analyze_nexus_vdp1_source_join import (
    accepted_retail_hashes,
    aggregate_source_join_status,
    swapped_words,
)


class AcceptedRetailHashesTests(unittest.TestCase):
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
