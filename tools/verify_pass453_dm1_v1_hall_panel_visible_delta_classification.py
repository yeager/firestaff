#!/usr/bin/env python3
"""Gate the pass449 panel_visible comparator delta classification.

This verifier intentionally does not claim pixel parity. It locks the external
original DM1 PC34 data and the diagnostic artifact manifest, then verifies that
panel_visible is classified as a Firestaff-side state mismatch where the Hall
panel is not visible in the Firestaff frame.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
ARTIFACT_MANIFEST = Path(
    "/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/"
    "hall-comparator-diff-20260509/panel_visible_delta_manifest.json"
)
EXPECTED_CLASSIFICATION = "FIRESTAFF_FRAME_STATE_MISMATCH_PANEL_NOT_VISIBLE"
EXPECTED_HEAD = "9497681d4e2c92c79d20f4d643d4e7f4dcc316c0"
PASS455_MANIFEST = ROOT / "parity-evidence/verification/pass455_dm1_v1_hall_corrected_click_primitive_capture/manifest.json"
OUT_MD = ROOT / "parity-evidence/pass453_dm1_v1_hall_panel_visible_delta_classification.md"
EXTERNAL_HASH_LOCKS = {
    "dm1_pc34_english_graphics": {
        "path": Path(
            "/Volumes/Extern-disk/openclaw-data/firestaff/firestaff-original-games/"
            "DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT"
        ),
        "bytes": 363417,
        "sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    },
    "dm1_pc34_english_dungeon": {
        "path": Path(
            "/Volumes/Extern-disk/openclaw-data/firestaff/firestaff-original-games/"
            "DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT"
        ),
        "bytes": 33357,
        "sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    },
    "dm1_pc34_english_title": {
        "path": Path(
            "/Volumes/Extern-disk/openclaw-data/firestaff/firestaff-original-games/"
            "DM/_extracted/dm-pc34/DungeonMasterPC34/TITLE"
        ),
        "bytes": 12002,
        "sha256": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
    },
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def fail(message: str) -> int:
    print(f"FAIL pass453_dm1_v1_hall_panel_visible_delta_classification: {message}")
    return 1


def main() -> int:
    if not ARTIFACT_MANIFEST.exists():
        return fail(f"missing artifact manifest: {ARTIFACT_MANIFEST}")
    data = json.loads(ARTIFACT_MANIFEST.read_text())

    if data.get("classification") != EXPECTED_CLASSIFICATION:
        return fail(f"unexpected classification: {data.get('classification')}")
    if data.get("head") != EXPECTED_HEAD:
        return fail(f"unexpected evidence head: {data.get('head')}")

    observations = data.get("observations", {})
    if observations.get("firestaff_panel_visible_equals_candidate_select") is not True:
        return fail("Firestaff panel_visible is not hash-identical to candidate_select")

    panel_metrics = observations.get("panel_crop_metrics", {})
    if panel_metrics.get("dim") != [144, 73]:
        return fail(f"unexpected panel crop dimensions: {panel_metrics.get('dim')}")
    if float(panel_metrics.get("deltaPercent", 0.0)) < 80.0:
        return fail(f"panel crop delta too small for this blocker: {panel_metrics.get('deltaPercent')}")

    full_metrics = observations.get("fullframe_metrics", {})
    if full_metrics.get("dim") != [320, 200]:
        return fail(f"unexpected fullframe dimensions: {full_metrics.get('dim')}")
    if float(full_metrics.get("deltaPercent", 0.0)) < 45.0:
        return fail(f"fullframe delta too small for this blocker: {full_metrics.get('deltaPercent')}")

    near = observations.get("nearExpectedOffsetSearch_best10_meanAbsDeltaRgb_deltaPercent_dxdy_diffPixels", [])
    if not near or float(near[0].get("deltaPercent", 0.0)) < 80.0:
        return fail("small-offset search did not remain high-delta")

    artifact_locks = {entry.get("label"): entry for entry in data.get("originalDataHashLockExternal", [])}
    for label, expected in EXTERNAL_HASH_LOCKS.items():
        path = expected["path"]
        if not path.exists():
            return fail(f"missing external original data: {path}")
        if path.stat().st_size != expected["bytes"]:
            return fail(f"external original data byte mismatch for {label}")
        if sha256(path) != expected["sha256"]:
            return fail(f"external original data sha256 mismatch for {label}")
        locked = artifact_locks.get(label)
        if not locked or locked.get("sha256") != expected["sha256"] or locked.get("bytes") != expected["bytes"]:
            return fail(f"artifact manifest hash lock mismatch for {label}")

    for artifact in data.get("generatedArtifacts", []):
        if not Path(artifact).exists():
            return fail(f"missing generated diagnostic artifact: {artifact}")

    pass455 = json.loads(PASS455_MANIFEST.read_text(encoding="utf-8")) if PASS455_MANIFEST.exists() else {}
    superseded = pass455.get("status") == "PASS_PASS455_CORRECTED_CLICK_PRIMITIVE_AND_CANDIDATE_TRANSITION_PROVEN"
    status = "SUPERSEDED_BY_PASS455_CORRECTED_CANDIDATE_TRANSITION" if superseded else "DIAGNOSTIC_FIRESTAFF_PANEL_VISIBLE_FRAME_STATE_MISMATCH_NO_PIXEL_PARITY_CLAIM"
    OUT_MD.write_text(
        "# Pass453 DM1 V1 Hall panel-visible delta classification\n\n"
        f"Status: `{status}`.\n\n"
        "This remains a diagnostic classification only, not a pixel-parity claim. "
        "When pass455 is green, the old Firestaff-side stale panel-visible diagnostic is superseded by corrected initial-south candidate-transition evidence.\n\n"
        f"Evidence artifact: `{ARTIFACT_MANIFEST}`\n",
        encoding="utf-8",
    )
    print(f"PASS pass453_dm1_v1_hall_panel_visible_delta_classification: {status}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
