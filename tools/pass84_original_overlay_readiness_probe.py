#!/usr/bin/env python3
"""Audit whether V1 original-vs-Firestaff overlay comparison is runnable.

This probe is intentionally evidence-only. It checks tracked/default inputs for
fullscreen overlay comparison and route capture safety; it does not generate or
promote screenshots and does not claim pixel parity.
"""
from __future__ import annotations

import json
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
PASS74_STATS = REPO / "parity-evidence" / "overlays" / "pass74" / "pass74_fullscreen_panel_compare_stats.json"
PASS78_DIR = REPO / "parity-evidence" / "overlays" / "pass78"
SHOT_LABEL_MANIFEST = REPO / "verification-screens" / "pass70-original-dm1-viewports" / "original_viewport_shot_labels.tsv"

SCENES = (
    ("01", "ingame_start"),
    ("02", "ingame_turn_right"),
    ("03", "ingame_move_forward"),
    ("04", "ingame_spell_panel"),
    ("05", "ingame_after_cast"),
    ("06", "ingame_inventory_panel"),
)

EXPECTED_FIRESTAFF_PPM = [
    REPO / "verification-screens" / f"{idx}_{scene}_latest.ppm"
    for idx, scene in SCENES
]
EXPECTED_FIRESTAFF_PNG = [
    REPO / "verification-screens" / f"{idx}_{scene}_latest.png"
    for idx, scene in SCENES
]
EXPECTED_ORIGINAL_RAW = [
    REPO / "verification-screens" / "pass70-original-dm1-viewports" / f"image{int(idx):04d}-raw.png"
    for idx, _scene in SCENES
]
EXPECTED_ROUTE_LABELS = [
    "party_hud",
    "",
    "",
    "spell_panel",
    "",
    "inventory_panel",
]
DIAGNOSTIC_ONLY_ROUTE_LABELS = {
    "title",
    "pre_enter_menu",
    "after_enter_click",
    "forward_1",
    "forward_2",
    "left_turn_probe",
}


def rel(path: Path) -> str:
    try:
        return str(path.relative_to(REPO))
    except ValueError:
        return str(path)


def file_state(paths: list[Path]) -> list[dict[str, object]]:
    return [
        {"path": rel(path), "exists": path.exists(), "bytes": path.stat().st_size if path.exists() else 0}
        for path in paths
    ]


def load_pass74() -> dict[str, object]:
    if not PASS74_STATS.exists():
        return {"exists": False}
    data = json.loads(PASS74_STATS.read_text())
    missing_referenced: list[str] = []
    for pair in data.get("pairings", []):
        for key in ("firestaff", "original", "mask"):
            value = pair.get(key)
            if isinstance(value, str) and not (REPO / value).exists():
                missing_referenced.append(value)
    return {
        "exists": True,
        "schema": data.get("schema"),
        "recorded_pass": data.get("pass"),
        "recorded_pairs": len(data.get("pairings", [])),
        "recorded_problems": data.get("problems", []),
        "missing_referenced_artifacts": sorted(set(missing_referenced)),
    }


def load_pass78_blockers() -> list[dict[str, object]]:
    blockers: list[dict[str, object]] = []
    for path in sorted(PASS78_DIR.glob("*.json")):
        data = json.loads(path.read_text())
        blockers.append({
            "path": rel(path),
            "captures": data.get("captures"),
            "dimensions_seen": data.get("dimensions_seen"),
            "all_gameplay_320x200": data.get("all_gameplay_320x200"),
        })
    return blockers


def load_shot_label_manifest() -> dict[str, object]:
    if not SHOT_LABEL_MANIFEST.exists():
        return {"exists": False, "path": rel(SHOT_LABEL_MANIFEST)}
    lines = SHOT_LABEL_MANIFEST.read_text().splitlines()
    rows: list[dict[str, object]] = []
    problems: list[str] = []
    if not lines or lines[0].split("\t") != ["index", "filename", "route_label", "route_token"]:
        problems.append("unexpected shot-label manifest header")
    for line in lines[1:]:
        parts = line.split("\t")
        if len(parts) != 4:
            problems.append(f"malformed shot-label manifest row: {line!r}")
            continue
        idx_text, filename, route_label, route_token = parts
        try:
            idx = int(idx_text)
        except ValueError:
            problems.append(f"non-numeric shot-label index: {idx_text!r}")
            idx = -1
        rows.append({
            "index": idx,
            "filename": filename,
            "route_label": route_label,
            "route_token": route_token,
        })
    labels = [str(row["route_label"]) for row in rows]
    diagnostic_labels = [label for label in labels if label in DIAGNOSTIC_ONLY_ROUTE_LABELS]
    if diagnostic_labels:
        problems.append(
            "diagnostic-only pass94 route labels are not overlay labels: "
            + ", ".join(diagnostic_labels)
        )
    if len(rows) != len(EXPECTED_ROUTE_LABELS):
        problems.append(f"expected {len(EXPECTED_ROUTE_LABELS)} shot-label rows, found {len(rows)}")
    elif labels != EXPECTED_ROUTE_LABELS:
        problems.append(f"shot labels do not match expected semantic checkpoints: {labels!r}")
    return {
        "exists": True,
        "path": rel(SHOT_LABEL_MANIFEST),
        "expected_route_labels": EXPECTED_ROUTE_LABELS,
        "diagnostic_only_route_labels": sorted(DIAGNOSTIC_ONLY_ROUTE_LABELS),
        "rows": rows,
        "problems": problems,
        "pass": not problems,
    }


def main() -> int:
    firestaff_ppm = file_state(EXPECTED_FIRESTAFF_PPM)
    firestaff_png = file_state(EXPECTED_FIRESTAFF_PNG)
    original_raw = file_state(EXPECTED_ORIGINAL_RAW)
    pass74 = load_pass74()
    pass78 = load_pass78_blockers()
    shot_labels = load_shot_label_manifest()

    missing_ppm = [row["path"] for row in firestaff_ppm if not row["exists"]]
    missing_original = [row["path"] for row in original_raw if not row["exists"]]
    missing_png = [row["path"] for row in firestaff_png if not row["exists"]]
    text_mode_blockers = [
        row for row in pass78
        if row.get("all_gameplay_320x200") is False
    ]

    blockers: list[str] = []
    if missing_ppm:
        blockers.append("default pass74 compare inputs are missing Firestaff PPM full-frame captures")
    if missing_original:
        blockers.append("default pass74 compare inputs are missing original raw 320x200 screenshots")
    if pass74.get("missing_referenced_artifacts"):
        blockers.append("recorded pass74 stats reference artifacts absent from the fresh worktree")
    if text_mode_blockers:
        blockers.append("pass78 route attempts remained text-mode/prompt captures, not gameplay")
    if missing_png:
        blockers.append("review PNGs for the six Firestaff scenes are incomplete")
    if not shot_labels.get("exists"):
        blockers.append("original shot-label manifest is absent; semantic route checkpoints are not auditable")
    elif shot_labels.get("problems"):
        blockers.append("original shot-label manifest is incomplete or does not match expected route checkpoints")

    report = {
        "schema": "pass84_original_overlay_readiness_probe.v1",
        "honesty": "Evidence/readiness only; no pixel-parity or semantic-route parity is claimed.",
        "firestaff_ppm_inputs": firestaff_ppm,
        "firestaff_png_reviews": firestaff_png,
        "original_raw_inputs": original_raw,
        "pass74_stats": pass74,
        "pass78_route_attempts": pass78,
        "shot_label_manifest": shot_labels,
        "champion_inventory_spell_route_readiness": {
            "required_scenes": [f"{idx}_{scene}" for idx, scene in SCENES],
            "explicit_champion_keys_supported_by_route_script": True,
            "serialized_original_frame_clicks_supported_by_route_script": True,
            "route_shape_dry_run_supported": True,
            "semantically_locked_original_runtime_route": False,
        },
        "ready_for_overlay_comparison": not blockers,
        "blockers": blockers,
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
