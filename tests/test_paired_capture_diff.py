#!/usr/bin/env python3
"""Unit tests for state-bound, cross-game paired capture comparison."""
from __future__ import annotations

import hashlib
import json
import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools.paired_capture_diff import ManifestError, compare_manifest


class PairedCaptureDiffTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory(prefix="firestaff-paired-capture-")
        self.root = Path(self.temp.name)
        self.reference = self.root / "reference.png"
        self.firestaff = self.root / "firestaff.png"
        self.reference_state = self.root / "reference-state.json"
        self.firestaff_state = self.root / "firestaff-state.json"
        Image.new("RGB", (320, 200), (0, 0, 0)).save(self.reference)
        frame = Image.new("RGB", (320, 200), (0, 0, 0))
        frame.putpixel((1, 34), (40, 20, 10))
        frame.save(self.firestaff)
        evidence = {"map": 2, "position": {"x": 5, "y": 7}, "facing": "north"}
        self.reference_state.write_text(json.dumps(evidence), encoding="utf-8")
        self.firestaff_state.write_text(json.dumps(evidence), encoding="utf-8")
        self.manifest = self.root / "pair.json"
        self.report_dir = self.root / "result"
        self.data = {
            "schema": "firestaff.parity.paired_capture.v1",
            "gameId": "dm1",
            "platformId": "dos-pc34",
            "referenceBuildId": "dosbox-x-test-build",
            "firestaffBuildId": "test-build",
            "frameSize": [320, 200],
            "comparisonStateKeys": ["map", "position.x", "position.y", "facing"],
            "regions": [
                {"name": "viewport", "x": 0, "y": 33, "width": 224, "height": 136},
                {"name": "hud", "x": 224, "y": 0, "width": 96, "height": 200},
            ],
            "captures": [{
                "id": "hoc-entry",
                "reference": {
                    "path": "reference.png",
                    "sha256": self._sha(self.reference),
                    "state": {"map": 2, "position": {"x": 5, "y": 7}, "facing": "north"},
                    "stateEvidence": {
                        "path": "reference-state.json",
                        "sha256": self._sha(self.reference_state),
                        "bindings": {
                            "map": "/map", "position.x": "/position/x",
                            "position.y": "/position/y", "facing": "/facing",
                        },
                    },
                },
                "firestaff": {
                    "path": "firestaff.png",
                    "sha256": self._sha(self.firestaff),
                    "state": {"map": 2, "position": {"x": 5, "y": 7}, "facing": "north"},
                    "stateEvidence": {
                        "path": "firestaff-state.json",
                        "sha256": self._sha(self.firestaff_state),
                        "bindings": {
                            "map": "/map", "position.x": "/position/x",
                            "position.y": "/position/y", "facing": "/facing",
                        },
                    },
                },
            }],
        }
        self._write_manifest()

    def tearDown(self) -> None:
        self.temp.cleanup()

    @staticmethod
    def _sha(path: Path) -> str:
        return hashlib.sha256(path.read_bytes()).hexdigest()

    def _write_manifest(self) -> None:
        self.manifest.write_text(json.dumps(self.data), encoding="utf-8")

    def test_emits_provenance_and_per_region_pixel_metrics(self) -> None:
        report = compare_manifest(self.manifest, self.report_dir)
        self.assertEqual(report["status"], "PAIR_READY_METRIC_ONLY")
        self.assertIn("does not claim", report["claim"])
        capture = report["captures"][0]
        self.assertEqual(capture["state"]["position.x"], 5)
        viewport, hud = capture["regions"]
        self.assertEqual(viewport["differingPixels"], 1)
        self.assertEqual(hud["differingPixels"], 0)
        self.assertTrue(Path(viewport["maskPath"]).is_file())
        self.assertTrue(Path(report["reportPath"]).is_file())

    def test_uses_one_manifest_contract_for_all_five_games(self) -> None:
        for game_id in ("dm1", "csb", "dm2", "theron", "nexus"):
            with self.subTest(game_id=game_id):
                self.data["gameId"] = game_id
                self._write_manifest()
                report = compare_manifest(
                    self.manifest, self.root / f"result-{game_id}"
                )
                self.assertEqual(report["gameId"], game_id)
                self.assertEqual(report["captures"][0]["state"]["map"], 2)

    def test_rejects_a_different_source_state(self) -> None:
        self.data["captures"][0]["firestaff"]["state"]["position"]["x"] = 6
        evidence = {"map": 2, "position": {"x": 6, "y": 7}, "facing": "north"}
        self.firestaff_state.write_text(json.dumps(evidence), encoding="utf-8")
        self.data["captures"][0]["firestaff"]["stateEvidence"]["sha256"] = self._sha(
            self.firestaff_state
        )
        self._write_manifest()
        with self.assertRaisesRegex(ManifestError, "source-state mismatch at position.x"):
            compare_manifest(self.manifest, self.report_dir)

    def test_rejects_a_stale_image_hash(self) -> None:
        self.data["captures"][0]["reference"]["sha256"] = "0" * 64
        self._write_manifest()
        with self.assertRaisesRegex(ManifestError, "SHA-256 mismatch"):
            compare_manifest(self.manifest, self.report_dir)

    def test_rejects_state_that_does_not_match_its_evidence(self) -> None:
        self.data["captures"][0]["firestaff"]["state"]["position"]["x"] = 6
        self._write_manifest()
        with self.assertRaisesRegex(ManifestError, "state evidence mismatch at position.x"):
            compare_manifest(self.manifest, self.report_dir)

    def test_rejects_noncanonical_dimensions(self) -> None:
        Image.new("RGB", (640, 400), (0, 0, 0)).save(self.reference)
        self.data["captures"][0]["reference"]["sha256"] = self._sha(self.reference)
        self._write_manifest()
        with self.assertRaisesRegex(ManifestError, "must match frameSize"):
            compare_manifest(self.manifest, self.report_dir)

    def test_accepts_platform_native_frame_geometry(self) -> None:
        Image.new("RGB", (256, 224), (0, 0, 0)).save(self.reference)
        frame = Image.new("RGB", (256, 224), (0, 0, 0))
        frame.putpixel((10, 10), (80, 40, 20))
        frame.save(self.firestaff)
        self.data["platformId"] = "theron-pc-engine"
        self.data["frameSize"] = [256, 224]
        self.data["regions"] = [
            {"name": "viewport", "x": 0, "y": 0, "width": 256, "height": 224}
        ]
        self.data["captures"][0]["reference"]["sha256"] = self._sha(self.reference)
        self.data["captures"][0]["firestaff"]["sha256"] = self._sha(self.firestaff)
        self._write_manifest()
        report = compare_manifest(self.manifest, self.report_dir)
        self.assertEqual(report["canonicalFrameSize"], [256, 224])
        self.assertEqual(report["captures"][0]["regions"][0]["differingPixels"], 1)

    def test_refuses_to_write_original_image_diffs_into_tracked_tree(self) -> None:
        repo = Path(__file__).resolve().parents[1]
        unsafe_output = repo / "tools" / "paired-capture-output-test"
        with self.assertRaisesRegex(ManifestError, "not ignored"):
            compare_manifest(self.manifest, unsafe_output)


if __name__ == "__main__":
    unittest.main()
