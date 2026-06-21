#!/usr/bin/env python3
"""Pass1071: DM1 V1 champion-panel pairing readiness verifier.

This is an artifact readiness verifier only. It fingerprints the existing
pass1053 original champion-candidate panel evidence and the existing Firestaff
V1 HUD PPM captures, then records the still-open blocker for a true original
four-champion HUD/status-panel pairing.

Honesty boundary:
- This does not compare Firestaff against original pixels.
- This does not claim champion-panel parity.
- The emitted ready-state status is intentionally BLOCKED until the missing
  original four-champion HUD and single-status-panel captures exist.
"""
from __future__ import annotations

import hashlib
import json
import struct
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "pass1071_dm1_v1_champion_panel_pairing_readiness"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

PASS1053_DIR = ROOT / "verification-screens/pass1053-dm1-original-champion-candidate-panel"
PASS1053_MANIFEST = PASS1053_DIR / "manifest.json"
PASS1053_REPORT = ROOT / "parity-evidence/pass1053_dm1_v1_original_champion_candidate_panel_capture.md"
FIRESTAFF_HUD_DIR = ROOT / "verification-m11/lane3-inventory-followup-20260428-0914"

BLOCKED_STATUS = "BLOCKED_ORIGINAL_FOUR_CHAMPION_HUD_AND_SINGLE_STATUS_PANEL_CAPTURE_MISSING"
EXPECTED_PASS1053_STATUS = "PASS1053_ORIGINAL_CHAMPION_CANDIDATE_PANEL_EVIDENCE_TRACKED"

EXPECTED_ORIGINAL_FRAMES = {
    "start_before_portrait_click": {
        "file": "start_before_portrait_click.png",
        "dims": [320, 200],
        "sha256": "50bead319e59bd42c9b5af6e4a39275e6cfc7a02fee96e6f6b766e575858fabc",
    },
    "candidate_select_after_click_111_82": {
        "file": "candidate_select_after_click_111_82.png",
        "dims": [320, 200],
        "sha256": "e4b373078be6aa0c27e793ccd476b6e886b34ef0c4b063c6d2274815351af53e",
    },
    "resurrect_terminal_hud_after_click_130_115": {
        "file": "resurrect_terminal_hud_after_click_130_115.png",
        "dims": [320, 200],
        "sha256": "7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f",
    },
}

EXPECTED_CROPS = {
    "start_before_portrait_click_candidate_buttons.png": {
        "dims": [155, 68],
        "sha256": "93b8ab0a0ff860ea59da92f3f5866247327dd93cfb128c75ba776cacb63d9eee",
    },
    "start_before_portrait_click_lower_panel.png": {
        "dims": [320, 64],
        "sha256": "91654a11f512c40242a61984f3830c1180e3629639b6a887f92819cca9255bb5",
    },
    "start_before_portrait_click_right_panel.png": {
        "dims": [96, 136],
        "sha256": "b770350704f1437e9cf193b3a96858dfbb4904abd4defc49907926a1d065ec64",
    },
    "start_before_portrait_click_viewport.png": {
        "dims": [224, 136],
        "sha256": "be3d78ea54a814a30894d58c24821a9de52454013093f541af163411a33134cc",
    },
    "candidate_select_after_click_111_82_candidate_buttons.png": {
        "dims": [155, 68],
        "sha256": "f7a509ef3c959d6f9ea46d5f51619c2f80c8e696aab7f196711eea1e58c8d3e4",
    },
    "candidate_select_after_click_111_82_lower_panel.png": {
        "dims": [320, 64],
        "sha256": "be87cafb3bd870e4f5d417cc288ac28e700fbe567863530f4d4120066ea25ba0",
    },
    "candidate_select_after_click_111_82_right_panel.png": {
        "dims": [96, 136],
        "sha256": "fab39b11602704accbdfddd5d284f60935562a106563fd05de359fd1a5e8d6c3",
    },
    "candidate_select_after_click_111_82_viewport.png": {
        "dims": [224, 136],
        "sha256": "b9160a2d3335bbc5bfcfacfd2ad007afadbc8184b289c17c155567785336d691",
    },
    "resurrect_terminal_hud_after_click_130_115_candidate_buttons.png": {
        "dims": [155, 68],
        "sha256": "ea0c7aa7abf9dfc77c33cff4bc7f1ddd0de30b80b1e610506976bdc2700cccce",
    },
    "resurrect_terminal_hud_after_click_130_115_lower_panel.png": {
        "dims": [320, 64],
        "sha256": "f0229dbb1c36b93830817715fb313cc5f2e8e6b7c921598b4cecf22dffb47b58",
    },
    "resurrect_terminal_hud_after_click_130_115_right_panel.png": {
        "dims": [96, 136],
        "sha256": "aed027a74a4762747be638909ab5aa99bf7f251281e9f56b608649677e64ee79",
    },
    "resurrect_terminal_hud_after_click_130_115_viewport.png": {
        "dims": [224, 136],
        "sha256": "b9160a2d3335bbc5bfcfacfd2ad007afadbc8184b289c17c155567785336d691",
    },
}

EXPECTED_FIRESTAFF_PPMS = {
    "party_hud_four_champions_vga.ppm": {
        "dims": [320, 200],
        "sha256": "d995c4991e5b973ea98c7eedd2d13bdca0e624061983da99131ea5110ddd17a9",
    },
    "party_hud_statusbox_gfx_vga.ppm": {
        "dims": [320, 200],
        "sha256": "860bc022785b2567eedfa552a99e103d02c573b432446c679a273d96d0a18363",
    },
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def png_dims(path: Path) -> list[int] | None:
    try:
        data = path.read_bytes()[:24]
    except OSError:
        return None
    if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        return None
    width, height = struct.unpack(">II", data[16:24])
    return [width, height]


def ppm_dims(path: Path) -> list[int] | None:
    try:
        data = path.read_bytes()[:256]
    except OSError:
        return None
    if not data.startswith(b"P6"):
        return None
    tokens: list[bytes] = []
    i = 0
    while i < len(data) and len(tokens) < 4:
        while i < len(data) and data[i:i + 1].isspace():
            i += 1
        if i < len(data) and data[i:i + 1] == b"#":
            while i < len(data) and data[i:i + 1] not in (b"\n", b"\r"):
                i += 1
            continue
        start = i
        while i < len(data) and not data[i:i + 1].isspace():
            i += 1
        if start < i:
            tokens.append(data[start:i])
    if len(tokens) < 4:
        return None
    try:
        return [int(tokens[1]), int(tokens[2])]
    except ValueError:
        return None


def load_pass1053_manifest() -> dict[str, Any]:
    if not PASS1053_MANIFEST.exists():
        return {}
    return json.loads(PASS1053_MANIFEST.read_text(encoding="utf-8"))


def check_png_set(base: Path, expected: dict[str, dict[str, Any]]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for name, expectation in expected.items():
        path = base / expectation["file"] if "file" in expectation else base / name
        dims = png_dims(path) if path.exists() else None
        digest = sha256(path) if path.exists() else None
        rows.append({
            "name": name,
            "path": str(path.relative_to(ROOT)),
            "exists": path.exists(),
            "dims": dims,
            "expected_dims": expectation["dims"],
            "sha256": digest,
            "expected_sha256": expectation["sha256"],
            "ok": path.exists() and dims == expectation["dims"] and digest == expectation["sha256"],
        })
    return rows


def check_firestaff_ppms() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for name, expectation in EXPECTED_FIRESTAFF_PPMS.items():
        path = FIRESTAFF_HUD_DIR / name
        dims = ppm_dims(path) if path.exists() else None
        digest = sha256(path) if path.exists() else None
        rows.append({
            "name": name,
            "path": str(path.relative_to(ROOT)),
            "exists": path.exists(),
            "dims": dims,
            "expected_dims": expectation["dims"],
            "sha256": digest,
            "expected_sha256": expectation["sha256"],
            "ok": path.exists() and dims == expectation["dims"] and digest == expectation["sha256"],
        })
    return rows


def check_manifest(pass1053: dict[str, Any]) -> dict[str, Any]:
    frames = {frame.get("label"): frame for frame in pass1053.get("frames", [])}
    frame_rows: list[dict[str, Any]] = []
    for label, expectation in EXPECTED_ORIGINAL_FRAMES.items():
        frame = frames.get(label, {})
        frame_rows.append({
            "label": label,
            "manifest_file": frame.get("file"),
            "manifest_dims": [frame.get("width"), frame.get("height")],
            "manifest_sha256": frame.get("sha256"),
            "expected_file": expectation["file"],
            "expected_dims": expectation["dims"],
            "expected_sha256": expectation["sha256"],
            "ok": frame.get("file") == expectation["file"]
            and [frame.get("width"), frame.get("height")] == expectation["dims"]
            and frame.get("sha256") == expectation["sha256"],
        })
    non_claims = pass1053.get("non_claims", [])
    return {
        "path": str(PASS1053_MANIFEST.relative_to(ROOT)),
        "exists": PASS1053_MANIFEST.exists(),
        "report_exists": PASS1053_REPORT.exists(),
        "status": pass1053.get("status"),
        "expected_status": EXPECTED_PASS1053_STATUS,
        "status_ok": pass1053.get("status") == EXPECTED_PASS1053_STATUS,
        "route_contains_hud_status_after": "shot:hud_status_after" in pass1053.get("route", []),
        "non_claim_boundary_ok": any(
            "not a full four-champion party HUD capture" in claim for claim in non_claims
        ),
        "frames": frame_rows,
        "ok": PASS1053_MANIFEST.exists()
        and PASS1053_REPORT.exists()
        and pass1053.get("status") == EXPECTED_PASS1053_STATUS
        and "shot:hud_status_after" in pass1053.get("route", [])
        and any("not a full four-champion party HUD capture" in claim for claim in non_claims)
        and all(row["ok"] for row in frame_rows),
    }


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass1071 DM1 V1 champion-panel pairing readiness",
        "",
        f"Status: `{result['status']}`",
        "",
        "This verifier fingerprints the already-tracked pass1053 original",
        "champion-candidate panel artifacts and the existing Firestaff V1 HUD PPMs.",
        "It is a readiness/blocker record only and explicitly makes no parity claim.",
        "",
        "## Inputs",
        "",
        f"- pass1053 manifest: `{PASS1053_MANIFEST.relative_to(ROOT)}`",
        f"- pass1053 report: `{PASS1053_REPORT.relative_to(ROOT)}`",
        f"- Firestaff HUD PPM directory: `{FIRESTAFF_HUD_DIR.relative_to(ROOT)}`",
        "",
        "## Fingerprint Result",
        "",
        f"- pass1053 manifest status OK: `{result['pass1053_manifest']['status_ok']}`",
        f"- pass1053 frame files OK: `{result['original_frames_ok']}`",
        f"- pass1053 crop files OK: `{result['original_crops_ok']}`",
        f"- Firestaff V1 HUD PPMs OK: `{result['firestaff_ppms_ok']}`",
        f"- Overall fingerprint OK: `{result['fingerprints_ok']}`",
        "",
        "## Firestaff PPMs",
        "",
        "| File | Dimensions | SHA256 | Status |",
        "|---|---:|---|---|",
    ]
    for row in result["firestaff_ppms"]:
        lines.append(
            f"| `{row['path']}` | `{row['dims']}` | `{row['sha256']}` | "
            f"{'OK' if row['ok'] else 'FAIL'} |"
        )
    lines += [
        "",
        "## Original Frames",
        "",
        "| Label | Dimensions | SHA256 | Status |",
        "|---|---:|---|---|",
    ]
    for row in result["original_frames"]:
        lines.append(
            f"| `{row['name']}` | `{row['dims']}` | `{row['sha256']}` | "
            f"{'OK' if row['ok'] else 'FAIL'} |"
        )
    lines += [
        "",
        "## Blocker",
        "",
        "- Missing original four-champion HUD capture for same-state pairing.",
        "- Missing original single-status-panel capture for same-state pairing.",
        "- Therefore this pass is readiness evidence only, not parity evidence.",
        "",
        "## Non-Claims",
        "",
        "- No Firestaff-vs-original pixel diff is performed.",
        "- No same-state champion-panel parity row is promoted.",
        "- No game code is changed or exercised by this verifier.",
        "",
        f"Manifest: `{MANIFEST.relative_to(ROOT)}`",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    pass1053 = load_pass1053_manifest()
    manifest = check_manifest(pass1053)
    original_frames = check_png_set(PASS1053_DIR, EXPECTED_ORIGINAL_FRAMES)
    original_crops = check_png_set(PASS1053_DIR / "crops", EXPECTED_CROPS)
    firestaff_ppms = check_firestaff_ppms()
    original_frames_ok = all(row["ok"] for row in original_frames)
    original_crops_ok = all(row["ok"] for row in original_crops)
    firestaff_ppms_ok = all(row["ok"] for row in firestaff_ppms)
    fingerprints_ok = manifest["ok"] and original_frames_ok and original_crops_ok and firestaff_ppms_ok
    result: dict[str, Any] = {
        "schema": "firestaff.parity.pass1071_dm1_v1_champion_panel_pairing_readiness.v1",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": BLOCKED_STATUS if fingerprints_ok else "FAIL_FINGERPRINT_DRIFT",
        "parity_claim": "none",
        "claim_parity": False,
        "scope": (
            "Readiness fingerprint for existing pass1053 original candidate-panel "
            "artifacts and existing Firestaff V1 champion HUD PPMs only."
        ),
        "blocked_on": [
            "original four-champion HUD capture",
            "original single-status-panel capture",
        ],
        "pass1053_manifest": manifest,
        "original_frames_ok": original_frames_ok,
        "original_frames": original_frames,
        "original_crops_ok": original_crops_ok,
        "original_crops": original_crops,
        "firestaff_ppms_ok": firestaff_ppms_ok,
        "firestaff_ppms": firestaff_ppms,
        "fingerprints_ok": fingerprints_ok,
    }
    write_outputs(result)
    if fingerprints_ok:
        print(f"PASS pass1071 readiness fingerprints; status={BLOCKED_STATUS}")
        print(f"manifest: {MANIFEST.relative_to(ROOT)}")
        return 0
    print("FAIL pass1071 readiness fingerprints drifted")
    print(f"manifest: {MANIFEST.relative_to(ROOT)}")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
