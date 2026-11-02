#!/usr/bin/env python3
"""Evidence-classification regressions for the Ymir face/material join."""

from __future__ import annotations

import unittest

from analyze_nexus_ymir_face_material_cross_join import relation_for_face_material


class FaceMaterialRelationshipTests(unittest.TestCase):
    def test_indexed_material_requires_exact_source_face_owner(self) -> None:
        face = (1, 8, 42)
        self.assertEqual(
            relation_for_face_material(1, face, {face}),
            "indexed_face_selector_owner",
        )

    def test_other_selector_owners_do_not_bind_runtime_face(self) -> None:
        self.assertEqual(
            relation_for_face_material(1, (1, 8, 42), {(1, 8, 43)}),
            "indexed_selector_has_other_face_owners",
        )

    def test_direct_colour_does_not_infer_face_material_owner(self) -> None:
        self.assertEqual(
            relation_for_face_material(5, (1, 8, 42), {(1, 8, 42)}),
            "direct_colour_face_material_link_unproven",
        )

    def test_unresolved_runtime_face_stays_unresolved(self) -> None:
        self.assertEqual(
            relation_for_face_material(1, None, {(1, 8, 42)}),
            "runtime_face_unresolved",
        )


if __name__ == "__main__":
    unittest.main()
