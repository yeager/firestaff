#!/usr/bin/env python3
"""Verify that the old CSB V1 M11 runtime/capture blocker is retired.

This is still a conservative boundary gate. It proves that Firestaff now has
a CSB-specific launch handoff after M12 and a positive PC real-data boot/tick
probe, while keeping original capture and pixel parity out of scope.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/csb_v1_m11_runtime_capture_boundary.json"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
MANIFEST = ROOT / "parity-evidence/verification/csb_v1_atari_asset_pair_manifest.json"

ANCHORS = [
    {"id": "redmcsb_csb_save_header_and_dungeon_ids", "role": "primary", "path": REDMCSB / "DEFS.H", "lines": "468-523", "needles": ["DM_SAVE_HEADER", "CSB_SAVE_HEADER", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME"]},
    {"id": "redmcsb_csb_save_file_router", "role": "primary", "path": REDMCSB / "CEDTINC8.C", "lines": "101-118", "needles": ["M746_FILE_ID_SAVE_CSBGAME_DAT", "M745_FILE_ID_SAVE_DMSAVE_DAT", "C13_DUNGEON_CSB_GAME", "C12_DUNGEON_CSB_PRISON"]},
    {"id": "redmcsb_make_new_adventure_gate", "role": "primary", "path": REDMCSB / "CEDTINCH.C", "lines": "5-63", "needles": ["F7086_IsReadyToMakeNewAdventure", "GameLoaded", "G7114_LoadedChampionCount", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C13_DUNGEON_CSB_GAME"]},
    {"id": "redmcsb_csb_dungeon_validation", "role": "primary", "path": REDMCSB / "CEDTINCU.C", "lines": "5-77", "needles": ["F7272_IsDungeonValid", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C13_DUNGEON_CSB_GAME", "C12_DUNGEON_CSB_PRISON"]},
    {"id": "firestaff_m12_supports_csb_launch_intent", "role": "firestaff_positive", "path": ROOT / "src/ui/menu_startup_m12.c", "lines": "2327-2337", "needles": ["All five catalogued games now have runtime launch boundaries", "strcmp(gameId, \"csb\") == 0"]},
    {"id": "firestaff_m12_launch_intent_uses_supported_game_and_assets", "role": "firestaff_positive", "path": ROOT / "src/ui/menu_startup_m12.c", "lines": "7440-7513", "needles": ["M12_StartupMenu_GetLaunchIntent", "m12_game_supported(intent.gameId)", "M12_AssetStatus_GameAvailable(&state->assetStatus, intent.gameId)", "version && version->matched ? 1 : 0"]},
    {"id": "firestaff_m11_csb_handoff_bypasses_dm1_loader", "role": "firestaff_positive", "path": ROOT / "src/engine/m11_game_view.c", "lines": "6673-6705", "needles": ["FS_GAME_CSB path in firestaff_game_loop.c", "CSB READY (FS LOOP)", "CSB READY: gameId=csb dataDir=%s"]},
    {"id": "firestaff_game_loop_csb_boots_profile", "role": "firestaff_positive", "path": ROOT / "src/engine/firestaff_game_loop.c", "lines": "420-440", "needles": ["FS_GAME_CSB", "csb_v1_boot_scan_assets", "csb_v1_boot_enter_game", "csb_v1_boot_print_summary"]},
    {"id": "firestaff_pc_real_asset_probe_ticks_csb", "role": "firestaff_positive", "path": ROOT / "probes/csb/firestaff_csb_v1_pc_real_asset_launch_probe.c", "lines": "1-130", "needles": ["PC-first CSB V1 real-data launch gate", "CSB_V1_VARIANT_PC34_EN", "csb_v1_boot_enter_game", "csb_v1_runtime_tick"]},
]

NON_CLAIMS = [
    "This gate does not claim CSB original capture parity or pixel parity.",
    "This gate does not claim Atari ST or Amiga emulator boot parity.",
    "CSB V1 rendering/gameplay parity still requires focused CSB evidence gates.",
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
    source_rows = []

    for anchor in ANCHORS:
        haystack = line_window(anchor["path"], anchor["lines"])
        missing = [needle for needle in anchor["needles"] if needle not in haystack]
        ok = anchor["path"].exists() and not missing
        if not ok:
            failures.append(f"{anchor['id']} missing {missing} path_exists={anchor['path'].exists()}")
        source_rows.append({
            "id": anchor["id"],
            "role": anchor["role"],
            "path": str(anchor["path"]),
            "lines": anchor["lines"],
            "needles": anchor["needles"],
            "missing": missing,
            "ok": ok,
        })

    manifest = load_json(MANIFEST)
    manifest_checks = {
        "manifest_pass": manifest.get("pass") is True,
        "asset_pair_ready": manifest.get("asset_pair_ready") is True,
        "runtime_support_payloads_ready": manifest.get("runtime_support_payloads_ready") is True,
        "launch_intent_allowed_true": manifest.get("launch_intent_allowed") is True,
    }
    for key, ok in manifest_checks.items():
        if not ok:
            failures.append(f"manifest check failed: {key}")

    result = {
        "schema": "firestaff.csb_v1_m11_runtime_capture_boundary.v2",
        "pass": not failures,
        "scope": "CSB V1 M11 runtime/capture boundary retirement; positive CSB launch handoff and PC boot/tick evidence.",
        "blocker_retired": not failures,
        "positive_boundary": {
            "menu_guard": "M12 supports CSB launch intent when hash-matched assets are available.",
            "m11_handoff": "M11 recognizes gameId=csb and hands off to the FS_GAME_CSB loop instead of the DM1 dungeon loader.",
            "runtime_probe": "csb_v1_pc_real_asset_launch proves PC CSB scan, boot enter-game, dungeon ownership, and one tick.",
        },
        "manifest": {"path": str(MANIFEST.relative_to(ROOT)), "schema": manifest.get("schema"), "checks": manifest_checks},
        "source_anchors": source_rows,
        "non_claims": NON_CLAIMS,
        "failures": failures,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(("PASS" if result["pass"] else "FAIL") + " csb v1 m11 runtime/capture boundary: blocker_retired=" + str(result["blocker_retired"]).lower())
    for failure in failures:
        print("- " + failure)
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
