#!/usr/bin/env python3
"""Verify the CSB V1 parity completion/DoD matrix.

Evidence-only gate. It validates the CSB V1 definition matrix, the existing
CSB surface matrix, source anchors, and the conservative Firestaff completion
credit. It does not enable or claim CSB launch/runtime parity.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT / "docs/parity/PARITY_MATRIX_CSB_V1.md"
OUT = ROOT / "parity-evidence/verification/csb_v1_completion_matrix.json"
SURFACE = ROOT / "parity-evidence/verification/csb_v1_parity_surface_matrix.json"
COMPLETION = ROOT / "parity-evidence/verification/firestaff_completion_matrix.json"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CSB_SRC = Path.home() / ".openclaw/data/firestaff-csb-source/CSB/src"
CSBWIN = Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin"

CRITERIA = {
    "reference_inventory": (8, "SOURCE_LOCKED_PARTIAL"),
    "definition_matrix": (10, "MATCHED_DEFINITION_ONLY"),
    "launch_smoke": (1, "POSITIVE_BLOCKER_RENDER_SMOKE"),
    "core_input_movement": (0, "BLOCKED_RUNTIME"),
    "viewport_ui_render": (0, "BLOCKED_CAPTURE"),
    "gameplay_systems": (0, "BLOCKED_RUNTIME"),
    "audio_timing": (0, "BLOCKED_RUNTIME"),
    "original_overlay_regression": (0, "BLOCKED_CAPTURE"),
}

ANCHORS = [
    (REDMCSB / "DEFS.H", "468-523", ["CSB_SAVE_HEADER", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME"]),
    (REDMCSB / "CEDTINC8.C", "101-118", ["M745_FILE_ID_SAVE_DMSAVE_DAT", "M746_FILE_ID_SAVE_CSBGAME_DAT", "C12_DUNGEON_CSB_PRISON"]),
    (REDMCSB / "CEDTINCH.C", "5-64", ["F7086_IsReadyToMakeNewAdventure", "F7272_IsDungeonValid", "C13_DUNGEON_CSB_GAME"]),
    (REDMCSB / "DUNVIEW.C", "380-390", ["full dungeon view", "ThievesEye_ViewportVisibleArea"]),
    (REDMCSB / "DUNVIEW.C", "4547-5205", ["F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", "/* Draw objects */", "/* Draw creatures */"]),
    (CSB_SRC / "README", "1-30", ["dungeon.dat", "hcsb.dat", "graphics.dat", "config.txt"]),
    (CSB_SRC / "Chaos.cpp", "500-625", ["CSBGAME", "CSBGAME2", "csbgame.dat", "csbgame.bak"]),
    (CSB_SRC / "Mouse.cpp", "1410-1425", ["keyboardMode == 2", "Reincarnate mode"]),
    (CSB_SRC / "Graphics.cpp", "1740-1915", ["graphics.dat", "Cannot find 'graphics.dat'", "CSBgraphics.dat"]),
    (CSBWIN / "Game/readme.txt", "1-30", ["Enter the dungeon", "choose prison", "Make New Adventure"]),
    (CSBWIN / "SaveGame.cpp", "160-190", ["dungeonDatIndex", "NumWordsInTextArray"]),
    (CSBWIN / "Mouse.cpp", "1600-1668", ["HandleClickInViewport", "d.ClockRunning"]),
    (CSBWIN / "data.cpp", "1740-1755", ["keyboardMode=1", "adventuring"]),
]

NON_CLAIMS = [
    "No CSB runtime, launch, render, gameplay, save compatibility, or pixel parity is claimed by this matrix.",
    "No Firestaff runtime code is modified by this matrix.",
    "DM1 gates cannot be counted as CSB V1 completion without CSB-specific evidence.",
]

def line_window(path: Path, span: str) -> str:
    start, end = [int(x) for x in span.split("-")]
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return ""
    return "\n".join(lines[start - 1:end])

def main() -> int:
    failures: list[str] = []
    doc = DOC.read_text(encoding="utf-8") if DOC.exists() else ""
    for criterion, (score, status) in CRITERIA.items():
        row_needles = [f"`{criterion}`", f"{score}/", f"`{status}`"]
        missing = [needle for needle in row_needles if needle not in doc]
        if missing:
            failures.append(f"doc missing CSB V1 criterion row details for {criterion}: {missing}")
    for text in NON_CLAIMS:
        if text not in doc:
            failures.append(f"doc missing non-claim: {text}")

    source_rows = []
    default_optional_roots_missing = not CSB_SRC.exists() or not CSBWIN.exists()
    for path, span, needles in ANCHORS:
        optional_missing = default_optional_roots_missing and (path.is_relative_to(CSB_SRC) or path.is_relative_to(CSBWIN))
        haystack = line_window(path, span)
        missing = [needle for needle in needles if needle not in haystack]
        ok = optional_missing or (path.exists() and not missing)
        if not ok:
            failures.append(f"anchor {path}:{span} missing {missing} exists={path.exists()}")
        source_rows.append({
            "path": str(path),
            "lines": span,
            "needles": needles,
            "missing": [] if optional_missing else missing,
            "ok": ok,
            "skippedHostMissingOptionalRoot": optional_missing,
        })

    surface = json.loads(SURFACE.read_text(encoding="utf-8")) if SURFACE.exists() else {}
    if surface.get("schema") != "firestaff.csb_v1_parity_surface_matrix.v1" or not surface.get("pass"):
        failures.append("CSB V1 surface matrix evidence is missing or failing")
    if surface.get("surface_count", 0) < 6:
        failures.append("CSB V1 surface matrix must define at least six parity surfaces")

    completion = json.loads(COMPLETION.read_text(encoding="utf-8")) if COMPLETION.exists() else {}
    rows = {row.get("target"): row for row in completion.get("rows", []) if isinstance(row, dict)}
    csb = rows.get("CSB V1")
    if not csb:
        failures.append("Firestaff completion matrix missing CSB V1 row")
    else:
        score = csb.get("scores", {}).get("definition_matrix")
        if score is None or score[0] != 10:
            failures.append(f"CSB V1 definition_matrix completion credit must be 10/10, got {score}")
        if csb.get("completionPercent") != 19 or csb.get("points") != 19:
            failures.append(f"CSB V1 completion should be 19/100 after launch blocker gate, got {csb.get('completionPercent')}/{csb.get('points')}")

    result = {
        "schema": "firestaff.csb_v1_completion_matrix.v1",
        "pass": not failures,
        "scope": "CSB V1 definition-of-done matrix; source/evidence boundary only, no runtime parity claim.",
        "criteria": [{"id": k, "score": v[0], "status": v[1]} for k, v in CRITERIA.items()],
        "source_anchors": source_rows,
        "surface_matrix": {"path": str(SURFACE.relative_to(ROOT)), "pass": surface.get("pass"), "surface_count": surface.get("surface_count")},
        "completion_impact": {"target": "CSB V1", "before_percent": 18, "after_percent": 19, "launch_blocker_delta": 1, "launch_smoke_status": "POSITIVE_BLOCKER_RENDER_SMOKE"},
        "non_claims": NON_CLAIMS,
        "failures": failures,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(("PASS" if result["pass"] else "FAIL") + f" csb v1 completion matrix: criteria={len(CRITERIA)} anchors={len(source_rows)} impact=+1 point")
    for failure in failures:
        print(f"- {failure}")
    return 0 if result["pass"] else 1

if __name__ == "__main__":
    raise SystemExit(main())
