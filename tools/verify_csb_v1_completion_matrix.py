#!/usr/bin/env python3
"""Verify the CSB V1 parity completion/DoD matrix.

Evidence-only gate. It validates the CSB V1 definition matrix, the existing
CSB surface matrix, source anchors, and the conservative Firestaff completion
credit. It does not claim full CSB runtime parity.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT / "docs/parity/PARITY_MATRIX_CSB_V1.md"
OUT = ROOT / "parity-evidence/verification/csb_v1_completion_matrix.json"
SURFACE = ROOT / "parity-evidence/verification/csb_v1_parity_surface_matrix.json"
COMPLETION = ROOT / "parity-evidence/verification/firestaff_completion_matrix.json"
CMAKE = ROOT / "CMakeLists.txt"
HANDOFF_TEST = ROOT / "tests/test_csb_v1_boot_runtime_handoff.c"
ROUTE_TEST = ROOT / "tests/test_csb_v1_runtime_route_first_frame_movement_utility_gate.c"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CSB_SRC = Path.home() / ".openclaw/data/firestaff-csb-source/CSB/src"
ORIGINAL_CSB = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/csb"
GREATSTONE = Path.home() / ".openclaw/data/firestaff-greatstone-atlas/raw/greatstone.free.fr__dm__g_csb.html.html"
CSBWIN = Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin"

CRITERIA = {
    "reference_inventory": (9, "SOURCE_LOCKED_PARTIAL"),
    "definition_matrix": (10, "MATCHED_DEFINITION_ONLY"),
    "launch_smoke": (6, "RUNTIME_BOUNDARY_VERIFIED_PARTIAL"),
    "core_input_movement": (5, "RUNTIME_SLICE_VERIFIED_PARTIAL"),
    "viewport_ui_render": (7, "SOURCE_SLICE_VERIFIED_PARTIAL"),
    "gameplay_systems": (4, "RUNTIME_SLICE_VERIFIED_PARTIAL"),
    "audio_timing": (2, "DATA_DECODE_VERIFIED_PARTIAL"),
    "original_overlay_regression": (0, "BLOCKED_CAPTURE"),
}

ANCHORS = [
    (REDMCSB / "DEFS.H", "468-523", ["CSB_SAVE_HEADER", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME"]),
    (REDMCSB / "CEDTINC8.C", "101-118", ["M745_FILE_ID_SAVE_DMSAVE_DAT", "M746_FILE_ID_SAVE_CSBGAME_DAT", "C12_DUNGEON_CSB_PRISON"]),
    (REDMCSB / "CEDTINCH.C", "5-64", ["F7086_IsReadyToMakeNewAdventure", "F7272_IsDungeonValid", "C13_DUNGEON_CSB_GAME"]),
    (REDMCSB / "CEDTINCU.C", "5-77", ["F7272_IsDungeonValid", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME"]),
    (REDMCSB / "HINTLOAD.C", "11-18", ["0HCSB.HTC", "0HCSB.DAT", "1CSBGAME.DAT", "1CSBGAME.BAK"]),
    (REDMCSB / "HINTLOAD.C", "300-386", ["G3638_i_IORequestIndex_CSBGAME", "C4_CSBGAME_DAT", "C13_DUNGEON_CSB_GAME", "C1_PLATFORM_ATARI_ST"]),
    (REDMCSB / "FLOPPYST.C", "7-18", ["Support for two floppy disk drives", "A:\\\\CSBGAME.DAT", "A:\\\\CSBGAME.BAK"]),
    (REDMCSB / "DUNVIEW.C", "380-390", ["full dungeon view", "ThievesEye_ViewportVisibleArea"]),
    (REDMCSB / "DUNVIEW.C", "4547-5205", ["F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", "draw each object found", "Draw one creature", "Draw only projectiles", "Draw only explosions"]),
    (CSB_SRC / "README", "1-30", ["dungeon.dat", "hcsb.dat", "graphics.dat", "config.txt"]),
    (CSB_SRC / "Chaos.cpp", "500-625", ["CSBGAME", "CSBGAME2", "csbgame.dat", "csbgame.bak"]),
    (CSB_SRC / "Mouse.cpp", "1410-1425", ["keyboardMode == 2", "Reincarnate mode"]),
    (CSB_SRC / "Graphics.cpp", "1740-1915", ["graphics.dat", "Cannot find 'graphics.dat'", "CSBgraphics.dat"]),
    (CSB_SRC / "CSBwin.cpp", "502-508", ["IDC_QuickPlay", "PlayfileIsOpen", "OPT_QUICKPLAY"]),
    (CSB_SRC / "SaveGame.cpp", "1591-1607", ["GAMESTATE_ResumeSavedGame", "InsertDisk", "dungeonName", "Signature"]),
    (CSB_SRC / "SaveGame.cpp", "2046-2075", ["openGraphicsFile", "InitializeTimers", "HandleMouseEvents"]),
    (CSBWIN / "Game/readme.txt", "1-30", ["Enter the dungeon", "choose prison", "Make New Adventure"]),
    (CSBWIN / "SaveGame.cpp", "160-190", ["dungeonDatIndex", "NumWordsInTextArray"]),
    (CSBWIN / "Mouse.cpp", "1600-1668", ["HandleClickInViewport", "d.ClockRunning"]),
    (CSBWIN / "data.cpp", "1740-1755", ["keyboardMode=1", "adventuring"]),
]


REFERENCE_ANCHORS = [
    (ORIGINAL_CSB / "README.md", "1-34", ["atari-DUNGEON.DAT", "atari-GRAPHICS.DAT", "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba", "33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af"]),
    (GREATSTONE, "272-291", ["Chaos Strikes Back", "Atari", "2.0 (en) [hard-disk]", "dungeon.dat", "graphics.dat", "hcsb.dat", "hcsb.htc"]),
    (GREATSTONE, "306-323", ["Chaos Strikes Back", "Atari", "2.0 (en)", "mini.dat", "dungeon&nbsp;maps&nbsp;and&nbsp;savegame", "hcsb.dat", "hcsb.htc"]),
]

NON_CLAIMS = [
    "No full CSB runtime, render, gameplay, save compatibility, or pixel parity is claimed by this matrix.",
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

    cmake = CMAKE.read_text(encoding="utf-8", errors="replace") if CMAKE.exists() else ""
    handoff = HANDOFF_TEST.read_text(encoding="utf-8", errors="replace") if HANDOFF_TEST.exists() else ""
    runtime_spine_cmake_needles = [
        "firestaff_csb_v1_pc_real_asset_launch_probe",
        "firestaff_csb_v1_pc34_quickplay_dungeon_handle_probe",
        "firestaff_csb_v1_first_viewport_frame_probe",
        "test_csb_v1_boot_runtime_handoff",
        "test_csb_v1_runtime_route_first_frame_movement_utility_gate",
        "csb_v1_pc_real_asset_launch",
        "csb_v1_pc34_quickplay_dungeon_handle",
        "csb_v1_first_viewport_frame",
        "csb_v1_boot_runtime_handoff",
        "csb_v1_runtime_route_first_frame_movement_utility_gate",
        "src/dm1/dm1_v1_input_command_queue_pc34_compat.c",
        "src/dm1/dm1_v1_event_timer_pc34_compat.c",
        "src/shared/dungeon_decompressor_ftl.c",
        "verified profile -> runtime",
        "one deterministic tick -> Utility NEW_GAME handoff",
    ]
    forbidden_cmake_needles = [
        "target_link_libraries(firestaff_csb_v1_pc_real_asset_launch_probe\n    PRIVATE firestaff_m10)",
        "target_link_libraries(firestaff_csb_v1_pc34_quickplay_dungeon_handle_probe\n    PRIVATE firestaff_m10)",
        "target_link_libraries(firestaff_csb_v1_first_viewport_frame_probe\n    PRIVATE firestaff_m10)",
        "target_link_libraries(test_csb_v1_runtime_route_first_frame_movement_utility_gate\n    PRIVATE firestaff_m10)",
    ]
    runtime_spine_test_needles = [
        "test_enter_game_with_verified_profile_loads_dungeon",
        "test_enter_game_preserves_imported_party_and_switches_leader",
        "test_enter_game_rotate_party_aligns_champion_state",
        "test_utility_flow_new_game_handoff_preserves_leader_index",
        "dungeon_handle",
        "csb_v1_runtime_tick",
        "CSB_V1_UTIL_FLOW_NEW_GAME",
        "CHAMPION.C F0284",
    ]
    route = ROUTE_TEST.read_text(encoding="utf-8", errors="replace") if ROUTE_TEST.exists() else ""
    runtime_route_test_needles = [
        "Utility import -> NEW_GAME boundary",
        "csb_v1_boot_enter_game",
        "render_route_frame",
        "DM1_V1_COMMAND_MOVE_FORWARD",
        "csb_v1_movement_command_step_runtime_process_queue_pc34_compat",
        "post-movement viewport frame renders without panel bleed",
        "csb_v1_save_header_build",
        "csb_v1_save_game",
        "csb_v1_load_game",
        "bounded save prefix reloads multi-step route state",
        "not full CSB playability",
        "full save compatibility",
        "pixel parity",
    ]
    missing_cmake = [needle for needle in runtime_spine_cmake_needles if needle not in cmake]
    forbidden_cmake = [needle for needle in forbidden_cmake_needles if needle in cmake]
    missing_handoff = [needle for needle in runtime_spine_test_needles if needle not in handoff]
    missing_route = [needle for needle in runtime_route_test_needles if needle not in route]
    if missing_cmake:
        failures.append(f"CMake CSB boot/runtime handoff gate missing {missing_cmake}")
    if forbidden_cmake:
        failures.append("CSB fast runtime probes must not link firestaff_m10")
    if missing_handoff:
        failures.append(f"CSB boot/runtime handoff test missing {missing_handoff}")
    if missing_route:
        failures.append(f"CSB runtime route test missing {missing_route}")

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

    reference_rows = []
    for path, span, needles in REFERENCE_ANCHORS:
        haystack = line_window(path, span)
        missing = [needle for needle in needles if needle not in haystack]
        ok = path.exists() and not missing
        if not ok:
            failures.append(f"reference anchor {path}:{span} missing {missing} exists={path.exists()}")
        reference_rows.append({
            "path": str(path),
            "lines": span,
            "needles": needles,
            "missing": missing,
            "ok": ok,
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
        if csb.get("completionPercent") != 43 or csb.get("points") != 43:
            failures.append(f"CSB V1 completion should be 43/100 after current runtime-boundary evidence, got {csb.get('completionPercent')}/{csb.get('points')}")

    result = {
        "schema": "firestaff.csb_v1_completion_matrix.v1",
        "pass": not failures,
        "scope": "CSB V1 definition-of-done matrix; source/evidence boundary only, no full runtime parity claim.",
        "criteria": [{"id": k, "score": v[0], "status": v[1]} for k, v in CRITERIA.items()],
        "runtime_spine": {
            "cmake_targets": [
                "firestaff_csb_v1_pc_real_asset_launch_probe",
                "firestaff_csb_v1_pc34_quickplay_dungeon_handle_probe",
                "firestaff_csb_v1_first_viewport_frame_probe",
                "test_csb_v1_boot_runtime_handoff",
                "test_csb_v1_runtime_route_first_frame_movement_utility_gate",
            ],
            "ctest_names": [
                "csb_v1_pc_real_asset_launch",
                "csb_v1_pc34_quickplay_dungeon_handle",
                "csb_v1_first_viewport_frame",
                "csb_v1_boot_runtime_handoff",
                "csb_v1_runtime_route_first_frame_movement_utility_gate",
            ],
            "test_paths": [
                str(HANDOFF_TEST.relative_to(ROOT)),
                str(ROUTE_TEST.relative_to(ROOT)),
            ],
            "pass": not missing_cmake and not forbidden_cmake and not missing_handoff and not missing_route,
            "missing_cmake_needles": missing_cmake,
            "forbidden_cmake_needles": forbidden_cmake,
            "missing_test_needles": missing_handoff,
            "missing_route_test_needles": missing_route,
            "non_claim": "Fast PC/synthetic boot/runtime/viewport/bounded multi-step route gate plus save-prefix roundtrip only; not full CSB playability, original capture, full save compatibility, or pixel parity.",
        },
        "source_anchors": source_rows,
        "reference_anchors": reference_rows,
        "surface_matrix": {"path": str(SURFACE.relative_to(ROOT)), "pass": surface.get("pass"), "surface_count": surface.get("surface_count")},
        "completion_impact": {
            "target": "CSB V1",
            "current_percent": 43,
            "launch_smoke_score": 6,
            "launch_smoke_status": "RUNTIME_BOUNDARY_VERIFIED_PARTIAL",
            "core_input_movement_score": 5,
            "core_input_movement_status": "RUNTIME_SLICE_VERIFIED_PARTIAL",
            "gameplay_systems_score": 4,
            "gameplay_systems_status": "RUNTIME_SLICE_VERIFIED_PARTIAL",
            "viewport_ui_render_score": 7,
            "viewport_ui_render_status": "SOURCE_SLICE_VERIFIED_PARTIAL",
            "audio_timing_score": 2,
            "audio_timing_status": "DATA_DECODE_VERIFIED_PARTIAL",
        },
        "non_claims": NON_CLAIMS,
        "failures": failures,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(("PASS" if result["pass"] else "FAIL") + f" csb v1 completion matrix: criteria={len(CRITERIA)} anchors={len(source_rows)} reference_anchors={len(reference_rows)} completion=43/100")
    for failure in failures:
        print(f"- {failure}")
    return 0 if result["pass"] else 1

if __name__ == "__main__":
    raise SystemExit(main())
