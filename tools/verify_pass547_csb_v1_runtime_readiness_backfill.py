#!/usr/bin/env python3
"""Source-lock the CSB V1 runtime readiness blocker."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/pass547_csb_v1_runtime_readiness_backfill.json"
DOC = ROOT / "parity-evidence/pass547_csb_v1_runtime_readiness_backfill.md"
CSB_MATRIX = ROOT / "docs/parity/PARITY_MATRIX_CSB_V1.md"
COMPLETION = ROOT / "parity-evidence/verification/firestaff_completion_matrix.json"
ASSET_MANIFEST = ROOT / "parity-evidence/verification/csb_v1_atari_asset_pair_manifest.json"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

ANCHORS = [
    {"id": "redmcsb_csb_header_identity", "role": "primary_source", "path": REDMCSB / "DEFS.H", "lines": "482-523", "needles": ["CSB_SAVE_HEADER", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C1_PLATFORM_ATARI_ST", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME"]},
    {"id": "redmcsb_atari_csb_payload_names", "role": "primary_source", "path": REDMCSB / "HINTLOAD.C", "lines": "11-18", "needles": ["0HCSB.HTC", "0HCSB.DAT", "1CSBGAME.DAT", "1CSBGAME.BAK"]},
    {"id": "redmcsb_atari_csb_saved_game_load", "role": "primary_source", "path": REDMCSB / "HINTLOAD.C", "lines": "300-390", "needles": ["G3638_i_IORequestIndex_CSBGAME", "G3637_apc_FileNames[C4_CSBGAME_DAT]", "F1914_LoadAndDeobfuscateSavedGameHeader", "C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER", "C13_DUNGEON_CSB_GAME", "C1_PLATFORM_ATARI_ST", "GameLoaded = C1_TRUE"]},
    {"id": "redmcsb_make_new_adventure_runtime_gate", "role": "primary_source", "path": REDMCSB / "CEDTINCH.C", "lines": "5-63", "needles": ["F7086_IsReadyToMakeNewAdventure", "GameLoaded", "G7114_LoadedChampionCount", "F1996_", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C13_DUNGEON_CSB_GAME"]},
    {"id": "redmcsb_csb_dungeon_validation_switch", "role": "primary_source", "path": REDMCSB / "CEDTINCU.C", "lines": "5-77", "needles": ["F7272_IsDungeonValid", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME"]},
    {"id": "firestaff_m12_launch_guard_still_blocks_csb", "role": "firestaff_guard", "path": ROOT / "src/ui/menu_startup_m12.c", "lines": "1408-1414", "needles": ["Only DM1 is launch-supported", "strcmp(gameId, \"dm1\") == 0"]},
    {"id": "firestaff_m11_original_handoff_is_dm1_title_first", "role": "firestaff_blocker", "path": ROOT / "src/engine/main_loop_m11.c", "lines": "196-270", "needles": ["m11_find_title_dat_for_intro", "M12_AssetStatus_GetVersionCount(\"dm1\")", "_canonical/dm1/TITLE"]},
    {"id": "firestaff_m11_runtime_open_runs_original_dm1_handoff", "role": "firestaff_blocker", "path": ROOT / "src/engine/main_loop_m11.c", "lines": "698-728", "needles": ["m11_open_requested_launch", "m11_play_redmcsb_title_intro_if_available", "M11_GameView_OpenSelectedMenuEntry", "m11_play_redmcsb_entrance_transition"]},
    {"id": "firestaff_m11_builtin_csb_path_is_not_atari_csb_manifest", "role": "firestaff_blocker", "path": ROOT / "src/engine/m11_game_view.c", "lines": "1003-1015", "needles": ["m11_resolve_builtin_dungeon_path", "strcmp(gameId, \"csb\") == 0", "\"CSB.DAT\""]},
    {"id": "firestaff_current_ingame_capture_fixture_is_dm1", "role": "firestaff_blocker", "path": ROOT / "verification-screens/capture_firestaff_ingame_series.c", "lines": "213-218", "needles": ["M12_StartupMenu_InitWithDataDir", "M11_GameView_OpenSelectedMenuEntry", "failed to open DM1 game view"]},
]

NON_CLAIMS = [
    "This readiness backfill does not enable CSB launch, runtime, capture, rendering, gameplay, save compatibility, or pixel parity.",
    "The PASS condition means the runtime blocker is precisely documented and still present.",
    "CSB V1 completion remains 20/100 until CSB-specific runtime/capture evidence replaces the guarded DM1-shaped path.",
]

def line_window(path: Path, span: str) -> str:
    start, end = [int(part) for part in span.split("-")]
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return ""
    return "\n".join(lines[start - 1:end])

def load_json(path: Path) -> dict:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return {}

def main() -> int:
    failures: list[str] = []
    rows: list[dict] = []
    for anchor in ANCHORS:
        haystack = line_window(anchor["path"], anchor["lines"])
        missing = [needle for needle in anchor["needles"] if needle not in haystack]
        ok = anchor["path"].exists() and not missing
        if not ok:
            failures.append(f"{anchor['id']} missing {missing} path_exists={anchor['path'].exists()}")
        rows.append({"id": anchor["id"], "role": anchor["role"], "path": str(anchor["path"]), "lines": anchor["lines"], "needles": anchor["needles"], "missing": missing, "ok": ok})

    asset_manifest = load_json(ASSET_MANIFEST)
    asset_refs = [row.get("path", "") for row in asset_manifest.get("asset_refs", []) if isinstance(row, dict) and row.get("ok")]
    asset_basenames = {Path(path).name.upper() for path in asset_refs}
    manifest_checks = {
        "manifest_pass": asset_manifest.get("pass") is True,
        "asset_pair_ready": asset_manifest.get("asset_pair_ready") is True,
        "runtime_support_payloads_ready": asset_manifest.get("runtime_support_payloads_ready") is True,
        "launch_intent_allowed_false": asset_manifest.get("launch_intent_allowed") is False,
        "atari_dungeon_dat_present": "DUNGEON.DAT" in asset_basenames,
        "firestaff_builtin_csb_dat_not_source_locked_asset": "CSB.DAT" not in asset_basenames,
    }
    for key, ok in manifest_checks.items():
        if not ok:
            failures.append(f"asset manifest check failed: {key}")

    completion = load_json(COMPLETION)
    completion_rows = {row.get("target"): row for row in completion.get("rows", []) if isinstance(row, dict)}
    csb = completion_rows.get("CSB V1", {})
    completion_checks = {
        "csb_v1_still_20_percent": csb.get("completionPercent") == 20,
        "csb_v1_still_20_points": csb.get("points") == 20,
        "runtime_criteria_zero": all(csb.get("scores", {}).get(key, [None])[0] == 0 for key in ("core_input_movement", "viewport_ui_render", "gameplay_systems", "audio_timing", "original_overlay_regression")),
    }
    for key, ok in completion_checks.items():
        if not ok:
            failures.append(f"completion check failed: {key}")

    matrix_text = CSB_MATRIX.read_text(encoding="utf-8") if CSB_MATRIX.exists() else ""
    doc_text = DOC.read_text(encoding="utf-8") if DOC.exists() else ""
    doc_checks = {
        "matrix_names_readiness_gate": "pass547_csb_v1_runtime_readiness_backfill" in matrix_text,
        "matrix_keeps_runtime_blocked": "core_input_movement" in matrix_text and "BLOCKED_RUNTIME" in matrix_text,
        "evidence_doc_names_blocker": "The runtime blocker is not just missing code" in doc_text,
        "evidence_doc_names_non_claims": "No CSB runtime or capture launch is enabled." in doc_text,
    }
    for key, ok in doc_checks.items():
        if not ok:
            failures.append(f"doc check failed: {key}")

    result = {"schema": "firestaff.pass547_csb_v1_runtime_readiness_backfill.v1", "pass": not failures, "scope": "CSB V1 source-lock/runtime readiness backfill; blocker-present gate.", "blocker_persists": not failures, "source_anchors": rows, "asset_manifest": {"path": str(ASSET_MANIFEST.relative_to(ROOT)), "checks": manifest_checks}, "completion": {"path": str(COMPLETION.relative_to(ROOT)), "checks": completion_checks}, "required_runtime_replacements": ["CSB-specific launch handoff after M12 instead of the DM1 TITLE/entrance path.", "CSB Atari source-locked DUNGEON.DAT/GRAPHICS.DAT plus HCSB/MINI/save payload handling instead of the current CSB.DAT built-in branch.", "CSB-specific deterministic capture fixture before viewport/UI/render or overlay points can move from 0."], "non_claims": NON_CLAIMS, "failures": failures}
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(("PASS" if result["pass"] else "FAIL") + " pass547 csb v1 runtime readiness backfill: blocker_persists=" + str(result["blocker_persists"]).lower())
    for failure in failures:
        print("- " + failure)
    return 0 if result["pass"] else 1

if __name__ == "__main__":
    raise SystemExit(main())
