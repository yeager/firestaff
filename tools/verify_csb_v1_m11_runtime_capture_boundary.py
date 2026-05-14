#!/usr/bin/env python3
"""Guard the current CSB V1 M11 runtime/capture blocker.

This is an evidence gate, not a launch enabler. It source-locks the next
boundary after the M12 CSB front door: any production CSB launch/capture must
replace the current DM1-shaped M11 path before m12_game_supported() can admit
CSB.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/csb_v1_m11_runtime_capture_boundary.json"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
MANIFEST = ROOT / "parity-evidence/verification/csb_v1_atari_asset_pair_manifest.json"

ANCHORS = [
    {"id": "redmcsb_csb_save_header_and_dungeon_ids", "role": "primary", "path": REDMCSB / "DEFS.H", "lines": "468-523", "needles": ["DM_SAVE_HEADER", "CSB_SAVE_HEADER", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME", "Value used in CSB MINI.DAT"]},
    {"id": "redmcsb_csb_save_file_router", "role": "primary", "path": REDMCSB / "CEDTINC8.C", "lines": "101-118", "needles": ["M746_FILE_ID_SAVE_CSBGAME_DAT", "M745_FILE_ID_SAVE_DMSAVE_DAT", "C13_DUNGEON_CSB_GAME", "C12_DUNGEON_CSB_PRISON"]},
    {"id": "redmcsb_make_new_adventure_gate", "role": "primary", "path": REDMCSB / "CEDTINCH.C", "lines": "5-63", "needles": ["F7086_IsReadyToMakeNewAdventure", "GameLoaded", "G7114_LoadedChampionCount", "F1996_", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C13_DUNGEON_CSB_GAME"]},
    {"id": "redmcsb_csb_dungeon_validation", "role": "primary", "path": REDMCSB / "CEDTINCU.C", "lines": "5-77", "needles": ["F7272_IsDungeonValid", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C13_DUNGEON_CSB_GAME", "C12_DUNGEON_CSB_PRISON"]},
    {"id": "redmcsb_atari_csb_payload_and_save_loader", "role": "primary", "path": REDMCSB / "HINTLOAD.C", "lines": "11-386", "needles": ["0HCSB.HTC", "0HCSB.DAT", "1CSBGAME.DAT", "1CSBGAME.BAK", "C4_CSBGAME_DAT", "C13_DUNGEON_CSB_GAME", "C1_PLATFORM_ATARI_ST"]},
    {"id": "redmcsb_atari_csb_save_filenames", "role": "primary", "path": REDMCSB / "FLOPPYST.C", "lines": "7-18", "needles": ["CSBGAME.DAT", "CSBGAME.BAK"]},
    {"id": "firestaff_m12_production_launch_guard", "role": "firestaff_guard", "path": ROOT / "menu_startup_m12.c", "lines": "1408-1414", "needles": ["Only DM1 is launch-supported", "return gameId && strcmp(gameId, \"dm1\") == 0;"]},
    {"id": "firestaff_m12_launch_intent_guard", "role": "firestaff_guard", "path": ROOT / "menu_startup_m12.c", "lines": "5313-5350", "needles": ["M12_StartupMenu_GetLaunchIntent", "intent.valid = m12_game_supported(intent.gameId)", "version && version->matched ? 1 : 0"]},
    {"id": "firestaff_m11_title_intro_is_dm1_only", "role": "firestaff_blocker", "path": ROOT / "main_loop_m11.c", "lines": "196-260", "needles": ["m11_find_title_dat_for_intro", "M12_AssetStatus_GetVersionCount(\"dm1\")", ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE"]},
    {"id": "firestaff_m11_launch_handoff_runs_title_before_open", "role": "firestaff_blocker", "path": ROOT / "main_loop_m11.c", "lines": "698-728", "needles": ["m11_open_requested_launch", "m11_play_redmcsb_title_intro_if_available", "M11_GameView_OpenSelectedMenuEntry", "m11_play_redmcsb_entrance_transition"]},
    {"id": "firestaff_m11_csb_builtin_path_mismatch", "role": "firestaff_blocker", "path": ROOT / "m11_game_view.c", "lines": "1003-1015", "needles": ["m11_resolve_builtin_dungeon_path", "strcmp(gameId, \"csb\") == 0", "\"CSB.DAT\""]},
    {"id": "firestaff_capture_fixture_is_dm1_only", "role": "firestaff_blocker", "path": ROOT / "verification-screens/capture_firestaff_ingame_series.c", "lines": "213-218", "needles": ["M12_StartupMenu_InitWithDataDir", "M11_GameView_OpenSelectedMenuEntry", "failed to open DM1 game view"]},
]

NON_CLAIMS = [
    "This gate does not enable CSB launch, runtime, capture, rendering, gameplay, save compatibility, or pixel parity.",
    "The current PASS means the blocker is still present and guarded.",
    "A future CSB runtime/capture implementation must update this gate before m12_game_supported() can admit CSB.",
]


def line_window(path: Path, span: str) -> str:
    start, end = [int(part) for part in span.split("-")]
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return ""
    return "\n".join(lines[start - 1:end])


def main() -> int:
    failures: list[str] = []
    source_rows = []

    for anchor in ANCHORS:
        haystack = line_window(anchor["path"], anchor["lines"])
        missing = [needle for needle in anchor["needles"] if needle not in haystack]
        ok = anchor["path"].exists() and not missing
        if not ok:
            failures.append(f"{anchor['id']} missing {missing} path_exists={anchor['path'].exists()}")
        source_rows.append({"id": anchor["id"], "role": anchor["role"], "path": str(anchor["path"]), "lines": anchor["lines"], "needles": anchor["needles"], "missing": missing, "ok": ok})

    try:
        manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    except Exception as exc:
        manifest = {}
        failures.append(f"cannot read CSB Atari asset manifest: {exc}")

    ok_asset_paths = [row.get("path", "") for row in manifest.get("asset_refs", []) if row.get("ok")]
    basenames = {Path(path).name.upper() for path in ok_asset_paths}
    asset_paths = "\n".join(ok_asset_paths)
    manifest_checks = {
        "manifest_pass": manifest.get("pass") is True,
        "asset_pair_ready": manifest.get("asset_pair_ready") is True,
        "runtime_support_payloads_ready": manifest.get("runtime_support_payloads_ready") is True,
        "launch_intent_allowed_false": manifest.get("launch_intent_allowed") is False,
        "source_locked_dungeon_dat_present": "DUNGEON.DAT" in basenames,
        "source_locked_csb_dat_absent": "CSB.DAT" not in basenames,
    }
    for key, ok in manifest_checks.items():
        if not ok:
            failures.append(f"manifest check failed: {key}")

    result = {
        "schema": "firestaff.csb_v1_m11_runtime_capture_boundary.v1",
        "pass": not failures,
        "scope": "CSB V1 next launch/runtime capture boundary detector; blocker-present gate.",
        "blocker_persists": not failures,
        "blocker": {
            "menu_guard": "M12 production launch remains DM1-only.",
            "runtime_handoff": "M11 V1 original handoff runs DM1 TITLE intro/entrance before opening the game view.",
            "runtime_path_mismatch": "M11 built-in CSB resolver still points at CSB.DAT while the source-locked Atari CSB lane is DUNGEON.DAT plus support/save payloads.",
            "capture_fixture": "The current deterministic ingame capture fixture is DM1-only.",
        },
        "manifest": {"path": str(MANIFEST.relative_to(ROOT)), "schema": manifest.get("schema"), "checks": manifest_checks},
        "source_anchors": source_rows,
        "non_claims": NON_CLAIMS,
        "failures": failures,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(("PASS" if result["pass"] else "FAIL") + " csb v1 m11 runtime/capture boundary: blocker_persists=" + str(result["blocker_persists"]).lower())
    for failure in failures:
        print("- " + failure)
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
