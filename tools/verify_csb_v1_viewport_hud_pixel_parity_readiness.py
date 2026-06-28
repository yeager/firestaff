#!/usr/bin/env python3
"""CSB V1 viewport/HUD pixel-parity readiness gate.

This is an evidence-shape gate, not a capture probe. It keeps the existing
CSB viewport/HUD fixture manifest ready for a later original-vs-Firestaff
pairing pass while requiring the broad parity status to remain open until real
captures and hashes are supplied.
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
SOURCE_MANIFEST = ROOT / "parity-evidence/verification/passH2248_csb_v1_viewport_pixel_gate.json"
PASS = "csb_v1_viewport_hud_pixel_parity_readiness"
VERIFY_DIR = ROOT / "parity-evidence/verification" / PASS
OUT = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

EXPECTED_REGIONS = {
    "viewport_full": (0, 0, 320, 200),
    "viewport_center": (48, 28, 224, 136),
    "status_bar": (0, 0, 320, 28),
    "chrome_bottom": (0, 152, 320, 48),
    "panel_right": (240, 28, 80, 172),
}

HUD_REGIONS = {"status_bar", "chrome_bottom", "panel_right"}
REQUIRED_STATE_LABELS = {"csb_prison_entrance", "csb_prison_forward"}
PROMOTION_REQUIRED_FIELDS = (
    "original_artifact",
    "original_sha256",
    "firestaff_artifact",
    "firestaff_sha256",
    "diff_artifact",
    "diff_sha256",
    "changed_pixels",
    "mae",
    "max_delta",
)


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def check_fixture(row: dict[str, Any]) -> tuple[dict[str, Any], list[str]]:
    problems: list[str] = []
    fixture_id = str(row.get("fixture_id", ""))
    region = str(row.get("capture_region", ""))
    label = str(row.get("game_state_label", ""))
    expected_geom = EXPECTED_REGIONS.get(region)
    observed_geom = (
        row.get("x"),
        row.get("y"),
        row.get("w"),
        row.get("h"),
    )

    if not fixture_id.startswith("csb_v1_"):
        problems.append(f"{fixture_id or '<missing>'}: fixture_id must be CSB-scoped")
    if region not in EXPECTED_REGIONS:
        problems.append(f"{fixture_id}: unknown capture_region {region!r}")
    elif observed_geom != expected_geom:
        problems.append(f"{fixture_id}: geometry {observed_geom} != {expected_geom}")
    if label not in REQUIRED_STATE_LABELS:
        problems.append(f"{fixture_id}: unexpected state label {label!r}")
    if row.get("expected_sha256", None) != "":
        problems.append(f"{fixture_id}: expected_sha256 must stay empty until paired evidence lands")
    if not row.get("source_evidence"):
        problems.append(f"{fixture_id}: missing source_evidence block")

    return {
        "fixture_id": fixture_id,
        "game_state_label": label,
        "capture_region": region,
        "geometry": {
            "x": row.get("x"),
            "y": row.get("y"),
            "w": row.get("w"),
            "h": row.get("h"),
        },
        "covers_hud": region in HUD_REGIONS,
        "covers_viewport": region in {"viewport_full", "viewport_center"},
        "ready_for_pairing": not problems,
        "promotion_fields_required": list(PROMOTION_REQUIRED_FIELDS),
        "current_pairing": {
            "original_artifact": None,
            "original_sha256": None,
            "firestaff_artifact": None,
            "firestaff_sha256": None,
            "diff_artifact": None,
            "diff_sha256": None,
            "changed_pixels": None,
            "mae": None,
            "max_delta": None,
        },
    }, problems


def build_result() -> dict[str, Any]:
    problems: list[str] = []
    if not SOURCE_MANIFEST.exists():
        problems.append(f"missing source manifest: {SOURCE_MANIFEST.relative_to(ROOT)}")
        fixtures: list[dict[str, Any]] = []
        source_schema = None
    else:
        source = load_json(SOURCE_MANIFEST)
        source_schema = source.get("schema")
        fixtures = list(source.get("fixtures", []))
        if source_schema != "firestaff.csb_v1.viewport_pixel_gate.v1":
            problems.append(f"unexpected source manifest schema: {source_schema!r}")
        if source.get("fixture_count") != len(fixtures):
            problems.append("source fixture_count does not match fixtures length")

    rows: list[dict[str, Any]] = []
    for fixture in fixtures:
        checked, fixture_problems = check_fixture(fixture)
        rows.append(checked)
        problems.extend(fixture_problems)

    regions = {row["capture_region"] for row in rows}
    states = {row["game_state_label"] for row in rows}
    if len(rows) < 6:
        problems.append(f"expected at least 6 CSB viewport/HUD fixtures, got {len(rows)}")
    for region in EXPECTED_REGIONS:
        if region not in regions:
            problems.append(f"missing required region: {region}")
    for label in REQUIRED_STATE_LABELS:
        if label not in states:
            problems.append(f"missing required state label: {label}")
    if not any(row["covers_hud"] for row in rows):
        problems.append("no HUD/chrome regions covered")
    if not any(row["covers_viewport"] for row in rows):
        problems.append("no viewport regions covered")

    all_pair_hashes_present = all(
        all(row["current_pairing"][field] is not None for field in PROMOTION_REQUIRED_FIELDS)
        for row in rows
    )
    broad_parity_status = "READY_TO_PROMOTE" if all_pair_hashes_present else "OPEN_UNPAIRED"
    if broad_parity_status != "OPEN_UNPAIRED":
        problems.append("broad parity must remain OPEN_UNPAIRED in this readiness-only gate")

    return {
        "schema": "firestaff.parity.csb_v1_viewport_hud_pixel_parity_readiness.v1",
        "status": "PASS_OPEN" if not problems else "FAIL",
        "broad_parity_status": broad_parity_status,
        "source_manifest": str(SOURCE_MANIFEST.relative_to(ROOT)),
        "source_schema": source_schema,
        "fixture_count": len(rows),
        "regions_required": {
            name: {"x": geom[0], "y": geom[1], "w": geom[2], "h": geom[3]}
            for name, geom in EXPECTED_REGIONS.items()
        },
        "required_state_labels": sorted(REQUIRED_STATE_LABELS),
        "future_promotion_rule": (
            "Promote CSB viewport/HUD pixel parity only after every fixture has "
            "paired original and Firestaff artifacts, SHA256 values for both, "
            "a diff artifact/hash, and explicit changed_pixels/mae/max_delta metrics."
        ),
        "fixtures": rows,
        "non_claims": [
            "no original CSB frame is captured by this gate",
            "no Firestaff runtime frame is captured by this gate",
            "no original-vs-Firestaff pixel parity is promoted",
            "broader CSB viewport/HUD parity remains OPEN_UNPAIRED until paired evidence lands",
            "no user-supplied game data is committed",
        ],
        "problems": problems,
    }


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# CSB V1 viewport/HUD pixel parity readiness",
        "",
        f"Status: `{result['status']}`",
        f"Broad parity: `{result['broad_parity_status']}`",
        "",
        "This gate keeps the CSB viewport/HUD fixture shape ready for a later",
        "paired original-vs-Firestaff hash pass. It deliberately does not run a",
        "capture or promote parity while all future pairing fields are empty.",
        "",
        "## Fixture Surface",
        "",
        "| Fixture | State | Region | Geometry | Surface |",
        "|---|---|---|---|---|",
    ]
    for row in result["fixtures"]:
        surface = "HUD" if row["covers_hud"] else "Viewport"
        geom = row["geometry"]
        lines.append(
            f"| `{row['fixture_id']}` | `{row['game_state_label']}` | "
            f"`{row['capture_region']}` | "
            f"`{geom['x']},{geom['y']} {geom['w']}x{geom['h']}` | {surface} |"
        )
    lines += [
        "",
        "## Promotion Rule",
        "",
        result["future_promotion_rule"],
        "",
        "## Non-claims",
        "",
    ]
    lines.extend(f"- {item}" for item in result["non_claims"])
    if result["problems"]:
        lines += ["", "## Problems", ""]
        lines.extend(f"- {problem}" for problem in result["problems"])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    result = build_result()
    write_outputs(result)
    if result["status"] == "PASS_OPEN":
        print(
            "PASS csb_v1_viewport_hud_pixel_parity_readiness "
            f"fixtures={result['fixture_count']} broad_parity={result['broad_parity_status']}"
        )
        return 0
    print("FAIL csb_v1_viewport_hud_pixel_parity_readiness")
    for problem in result["problems"]:
        print("- " + problem)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
