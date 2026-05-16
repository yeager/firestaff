#!/usr/bin/env python3
"""Verify pass343 DM1 V1 touch input source-lock design artifacts."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT / "parity-evidence" / "pass343_dm1_v1_touch_input_source_lock_design.md"
MANIFEST = ROOT / "parity-evidence" / "verification" / "pass343_dm1_v1_touch_input_source_lock_design" / "manifest.json"
REQUIRED_DOC_MARKERS = [
    "STARTUP2.C:1179-1182",
    "COMMAND.C:375-395",
    "COMMAND.C:396-405",
    "COMMAND.C:412-419",
    "COMMAND.C:461-511",
    "COMMAND.C:1379-1449",
    "COMMAND.C:1452-1661",
    "COMMAND.C:1692-1707",
    "COMMAND.C:2045-2156",
    "COMMAND.C:2296-2324",
    "CLIKMENU.C:142-174",
    "CLIKMENU.C:180-330",
    "CLIKMENU.C:519-585",
    "COORD.C:1693-1722",
    "COORD.C:1915-1920",
    "touch_click_zone_matrix_pc34_compat.{h,c}",
    "touch_pointer_input_pc34_compat.{h,c}",
    "dm1_v1_input_command_queue_pc34_compat.{h,c}",
    "touch must not synthesize keyboard movement",
    "Primary-interface mouse input is searched before secondary movement mouse input",
]
REQUIRED_MANIFEST_KEYS = {
    "pass",
    "date",
    "branch",
    "evidence_file",
    "verifier",
    "redmcsb_source_root",
    "redmcsb_anchors",
    "firestaff_seams",
    "gates",
}


def main() -> int:
    errors: list[str] = []
    if not DOC.exists():
        errors.append(f"missing document: {DOC.relative_to(ROOT)}")
        doc_text = ""
    else:
        doc_text = DOC.read_text(encoding="utf-8")
        for marker in REQUIRED_DOC_MARKERS:
            if marker not in doc_text:
                errors.append(f"document missing marker: {marker}")
        if "```" in doc_text:
            errors.append("document must not contain raw source/code blocks")

    if not MANIFEST.exists():
        errors.append(f"missing manifest: {MANIFEST.relative_to(ROOT)}")
    else:
        try:
            manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
        except json.JSONDecodeError as exc:
            errors.append(f"manifest is not valid JSON: {exc}")
            manifest = {}
        missing = REQUIRED_MANIFEST_KEYS.difference(manifest)
        if missing:
            errors.append(f"manifest missing keys: {sorted(missing)}")
        anchors = manifest.get("redmcsb_anchors", [])
        if not isinstance(anchors, list) or len(anchors) < 10:
            errors.append("manifest redmcsb_anchors must list at least 10 anchors")
        seams = manifest.get("firestaff_seams", [])
        for seam in [
            "src/shared/touch_click_zone_matrix_pc34_compat.c",
            "src/shared/touch_pointer_input_pc34_compat.c",
            "src/dm1/dm1_v1_input_command_queue_pc34_compat.c",
        ]:
            if seam not in seams:
                errors.append(f"manifest missing Firestaff seam: {seam}")

    for path in [
        ROOT / "src/shared/touch_click_zone_matrix_pc34_compat.c",
        ROOT / "src/shared/touch_pointer_input_pc34_compat.c",
        ROOT / "src/dm1/dm1_v1_input_command_queue_pc34_compat.c",
    ]:
        if not path.exists():
            errors.append(f"missing existing seam file: {path.relative_to(ROOT)}")

    if errors:
        for error in errors:
            print(f"FAIL: {error}")
        return 1
    print("pass343 touch input source-lock design verification: OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
