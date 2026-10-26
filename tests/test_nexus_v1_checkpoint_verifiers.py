#!/usr/bin/env python3
"""Regression checks for strict Nexus checkpoint provenance validators."""

from __future__ import annotations

import hashlib
import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def load_script(name: str):
    spec = importlib.util.spec_from_file_location(name, ROOT / "scripts" / f"{name}.py")
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {name}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


sh2 = load_script("verify_nexus_sh2_decompilation")
checkpoint = load_script("verify_nexus_ymir_title_state_checkpoint")


class SH2ExportHeaderTests(unittest.TestCase):
    def write_export(self, path: Path, hashes: list[str], languages: list[str]) -> None:
        path.write_text(
            "".join(f"/* input_sha256={value} */\n" for value in hashes)
            + "".join(f"/* language={value} */\n" for value in languages)
            + "/* requested=06010000 entry=06010000 name=entry bytes=4 complete=true */\n",
            encoding="utf-8",
        )

    def test_consistent_duplicate_headers_are_accepted(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "export.txt"
            sha = "a" * 64
            self.write_export(path, [sha, sha], ["Japan", "Japan"])
            self.assertEqual(sh2.read_export(path)[:2], (sha, "Japan"))

    def test_conflicting_hash_header_inside_one_export_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "export.txt"
            self.write_export(path, ["a" * 64, "b" * 64], ["Japan"])
            with self.assertRaisesRegex(ValueError, "conflicting.*input_sha256"):
                sh2.read_export(path)

    def test_conflicting_language_header_inside_one_export_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "export.txt"
            self.write_export(path, ["a" * 64], ["Japan", "Europe"])
            with self.assertRaisesRegex(ValueError, "conflicting.*language"):
                sh2.read_export(path)


class YmirCheckpointTests(unittest.TestCase):
    def test_every_cue_track_is_hashed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            track = root / "retail-track.bin"
            track.write_bytes(b"track bytes for verifier test")
            cue = root / "retail.cue"
            cue.write_text('FILE "retail-track.bin" BINARY\n  TRACK 01 MODE1/2352\n', encoding="utf-8")
            original = checkpoint.EXPECTED_TRACKS
            checkpoint.EXPECTED_TRACKS = {
                track.name: hashlib.sha256(track.read_bytes()).hexdigest()
            }
            try:
                self.assertEqual(checkpoint.cue_track_paths(cue), [track.resolve()])
                track.write_bytes(b"changed bytes")
                with self.assertRaisesRegex(ValueError, "SHA-256 mismatch"):
                    checkpoint.cue_track_paths(cue)
            finally:
                checkpoint.EXPECTED_TRACKS = original

    def test_unpinned_or_missing_cue_track_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            cue = Path(directory) / "retail.cue"
            cue.write_text('FILE "unlisted.bin" BINARY\n', encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "unpinned retail track"):
                checkpoint.cue_track_paths(cue)

    def test_authentic_snapshot_schema_is_validated(self) -> None:
        snapshot = ROOT / "parity-evidence/verification/nexus_v1_ymir_title_menu_vdp_bundle_20261025/vdp-register-snapshot.txt"
        fields = checkpoint.validate_register_snapshot(snapshot)
        self.assertEqual(fields["register_byte_order"], "big_endian")

    def test_bad_snapshot_counts_and_frame_fields_are_rejected(self) -> None:
        snapshot = ROOT / "parity-evidence/verification/nexus_v1_ymir_title_menu_vdp_bundle_20261025/vdp-register-snapshot.txt"
        original = snapshot.read_text(encoding="utf-8")
        for old, new, message in (
            ("register_byte_order=big_endian", "register_byte_order=little_endian", "register_byte_order"),
            ("vdp2_register_bytes=512", "vdp2_register_bytes=32", "vdp2_register_bytes"),
            ("vdp1_frame_index=497", "vdp1_frame_index=-1", "vdp1_frame_index"),
            ("vdp1_next_command_index=178", "vdp1_next_command_index=bad", "vdp1_next_command_index"),
        ):
            with self.subTest(field=message), tempfile.TemporaryDirectory() as directory:
                altered = Path(directory) / "snapshot.txt"
                self.assertIn(old, original)
                altered.write_text(original.replace(old, new, 1), encoding="utf-8")
                with self.assertRaisesRegex(ValueError, message):
                    checkpoint.validate_register_snapshot(altered)

    def test_unpinned_vdp1_bytes_are_not_claimed_as_checkpoint_evidence(self) -> None:
        self.assertNotIn("vdp1-registers.bin", checkpoint.EXPECTED_SIZES)


if __name__ == "__main__":
    unittest.main()
