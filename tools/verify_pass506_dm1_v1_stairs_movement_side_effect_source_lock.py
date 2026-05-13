#!/usr/bin/env python3
"""Pass506: source-lock DM1 V1 stairs movement side-effect branches."""
from __future__ import annotations

import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass506_dm1_v1_stairs_movement_side_effect_source_lock"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CLIKMENU = RED / "CLIKMENU.C"
DUNGEON = RED / "DUNGEON.C"
COMMAND_CORE_C = ROOT / "dm1_v1_movement_command_core_pc34_compat.c"
COMMAND_CORE_H = ROOT / "dm1_v1_movement_command_core_pc34_compat.h"
PIPELINE_TEST = ROOT / "test_dm1_v1_movement_pipeline_pc34_compat.c"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str, start: int = 0) -> int:
    pos = text.find(needle, start)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_order(text: str, needles: list[str], label: str, start: int = 0) -> list[int]:
    positions: list[int] = []
    last = start - 1
    for needle in needles:
        pos = require(text, needle, label, last + 1)
        positions.append(pos)
        last = pos
    return positions


def function_range(
    text: str,
    name: str,
    rettype: str = r"(?:STATICFUNCTION\s+)?(?:void|BOOLEAN|int16_t|int)",
    next_anchor: str | None = None,
) -> tuple[int, int, str]:
    m = re.search(r"\b(?:" + rettype + r")\s+" + re.escape(name) + r"\s*\(", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return line_no(text, m.start()), line_no(text, pos), text[m.start():pos + 1]
    if next_anchor:
        end = text.find(next_anchor, brace)
        if end > brace:
            return line_no(text, m.start()), line_no(text, end) - 1, text[m.start():end]
    raise AssertionError(f"unterminated function {name}")


def span(base_line: int, body: str, offsets: list[int]) -> str:
    return f"{base_line + line_no(body, min(offsets)) - 1}-{base_line + line_no(body, max(offsets)) - 1}"


def run(cmd: list[str]) -> str:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
    if p.returncode != 0:
        raise AssertionError(f"command failed {' '.join(cmd)}:\n{p.stdout[-3000:]}")
    return p.stdout.strip()


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    clik = CLIKMENU.read_text(encoding="latin-1")
    dungeon = DUNGEON.read_text(encoding="latin-1")
    core_c = COMMAND_CORE_C.read_text(encoding="utf-8")
    core_h = COMMAND_CORE_H.read_text(encoding="utf-8")
    pipeline_test = PIPELINE_TEST.read_text(encoding="utf-8")

    f0364_start, f0364_end, f0364 = function_range(clik, "F0364_COMMAND_TakeStairs")
    f0365_start, f0365_end, f0365 = function_range(clik, "F0365_COMMAND_ProcessTypes1To2_TurnParty")
    f0366_start, f0366_end, f0366 = function_range(clik, "F0366_COMMAND_ProcessTypes3To6_MoveParty", next_anchor="#include \"CLIKCHAM.C\"")
    f0154_start, f0154_end, f0154 = function_range(dungeon, "F0154_DUNGEON_GetLocationAfterLevelChange", rettype=r"int16_t", next_anchor="int16_t F0155_DUNGEON_GetStairsExitDirection")
    f0155_start, f0155_end, f0155 = function_range(dungeon, "F0155_DUNGEON_GetStairsExitDirection", rettype=r"int16_t", next_anchor="unsigned char* F0156_DUNGEON_GetThingData")

    take_order = require_order(f0364, [
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, CM1_MAPX_NOT_ON_A_SQUARE, 0);",
        "F0154_DUNGEON_GetLocationAfterLevelChange",
        "F0173_DUNGEON_SetCurrentMap(G0327_i_NewPartyMapIndex);",
        "F0284_CHAMPION_SetPartyDirection(F0155_DUNGEON_GetStairsExitDirection",
        "F0173_DUNGEON_SetCurrentMap(G0309_i_PartyMapIndex);",
    ], "ReDMCSB F0364 stairs transition order")

    turn_stairs = require_order(f0365, [
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "if (M034_SQUARE_TYPE(L1114_ui_Square = F0151_DUNGEON_GetSquare",
        "F0364_COMMAND_TakeStairs(M007_GET(L1114_ui_Square, MASK0x0004_STAIRS_UP));",
        "return;",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, C1_TRUE, C0_FALSE);",
        "F0284_CHAMPION_SetPartyDirection",
    ], "ReDMCSB F0365 turn-on-stairs short-circuit")

    backward_stairs = require_order(f0366, [
        "L1123_B_StairsSquare = (M034_SQUARE_TYPE",
        "if (L1123_B_StairsSquare && (AL1118_ui_MovementArrowIndex == 2))",
        "F0364_COMMAND_TakeStairs(M007_GET(AL1115_ui_Square, MASK0x0004_STAIRS_UP));",
        "return;",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
    ], "ReDMCSB F0366 backward-on-stairs short-circuit")

    target_stairs = require_order(f0366, [
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "if (L1116_i_SquareType == C03_ELEMENT_STAIRS) {",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, CM1_MAPX_NOT_ON_A_SQUARE, 0);",
        "G0306_i_PartyMapX = L1121_i_MapX;",
        "G0307_i_PartyMapY = L1122_i_MapY;",
        "F0364_COMMAND_TakeStairs(M007_GET(AL1115_ui_Square, MASK0x0004_STAIRS_UP));",
        "return;",
        "L1117_B_MovementBlocked = C0_FALSE;",
    ], "ReDMCSB F0366 target-stairs consequence order")

    source_stairs = require_order(f0366, [
        "if (L1123_B_StairsSquare) {",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, CM1_MAPX_NOT_ON_A_SQUARE, 0, L1121_i_MapX, L1122_i_MapY);",
        "} else {",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
        "AL1115_ui_Ticks = 1;",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
    ], "ReDMCSB F0366 source-stairs normal-step order")

    require_order(f0154, [
        "L0250_i_NewMapX = L0254_ps_Map->OffsetMapX + *P0272_pi_MapX;",
        "L0251_i_NewMapY = L0254_ps_Map->OffsetMapY + *P0273_pi_MapY;",
        "L0252_i_NewLevel = L0254_ps_Map->A.Level + P0271_i_LevelDelta;",
        "*P0273_pi_MapY = L0251_i_NewMapY - L0253_i_Offset;",
        "*P0272_pi_MapX = L0250_i_NewMapX - L0254_ps_Map->OffsetMapX;",
        "return L0255_i_TargetMapIndex;",
    ], "ReDMCSB F0154 level/offset stairs target mapping")
    require_order(f0155, [
        "L0257_B_NorthSouthOrientedStairs = !M007_GET",
        "G0233_ai_Graphic559_DirectionToStepEastCount",
        "return ((((L0256_i_SquareType = M034_SQUARE_TYPE",
    ], "ReDMCSB F0155 stairs exit direction probe")

    for label, haystack, needles in [
        ("Firestaff result seam", core_h, ["sourceStairsWalkOffSkipped"]),
        ("Firestaff source-stairs helper", core_c, [
            "dm1_v1_party_source_square_is_stairs",
            "DUNGEON_ELEMENT_STAIRS",
            "sourceStairsWalkOffSkipped = 1",
            "SENSOR_EVENT_WALK_OFF",
            "SENSOR_EVENT_WALK_ON",
        ]),
        ("Firestaff target/backward stairs command core", core_c, [
            "F0705_MOVEMENT_ResolveStairsTransition_Compat",
            "stairSourceLeaveProcessed = 1",
            "stairTargetLeaveProcessed = 1",
            "return 1;",
        ]),
        ("Pipeline stairs regression assertions", pipeline_test, [
            "stairs_into_no_regular_step",
            "stairs_into_no_cooldown",
            "stairs_backward_no_target_walk_off",
            "stairs_backward_no_cooldown",
            "stairs_source_forward_skip_source_walk_off",
            "stairs_source_forward_sets_cooldown",
        ]),
    ]:
        missing = [needle for needle in needles if needle not in haystack]
        if missing:
            raise AssertionError(f"{label}: missing {missing!r}")

    test_out = run([str(ROOT / "build/test_dm1_v1_movement_pipeline_pc34_compat")])
    status = "PASS506_DM1_V1_STAIRS_MOVEMENT_SIDE_EFFECT_SOURCE_LOCK_PROVEN"

    manifest = {
        "schema": f"{PASS}.v1",
        "status": status,
        "branch": run(["git", "branch", "--show-current"]),
        "primarySources": {
            "CLIKMENU.C": {
                "F0364_COMMAND_TakeStairs": f"{f0364_start}-{f0364_end}",
                "F0365_COMMAND_ProcessTypes1To2_TurnParty": f"{f0365_start}-{f0365_end}",
                "F0366_COMMAND_ProcessTypes3To6_MoveParty": f"{f0366_start}-{f0366_end}",
                "takeStairsOrder": span(f0364_start, f0364, take_order),
                "turnOnStairsShortCircuit": span(f0365_start, f0365, turn_stairs),
                "backwardOnStairsShortCircuit": span(f0366_start, f0366, backward_stairs),
                "targetStairsConsequence": span(f0366_start, f0366, target_stairs),
                "sourceStairsNormalStep": span(f0366_start, f0366, source_stairs),
            },
            "DUNGEON.C": {
                "F0154_DUNGEON_GetLocationAfterLevelChange": f"{f0154_start}-{f0154_end}",
                "F0155_DUNGEON_GetStairsExitDirection": f"{f0155_start}-{f0155_end}",
            },
        },
        "firestaffGuards": {
            "commandCore": "dm1_v1_movement_command_core_pc34_compat.c",
            "commandCoreResult": "dm1_v1_movement_command_core_pc34_compat.h",
            "pipelineTest": "test_dm1_v1_movement_pipeline_pc34_compat.c",
            "runtimeExecutable": "build/test_dm1_v1_movement_pipeline_pc34_compat",
            "runtimeOutputLastLine": test_out.splitlines()[-1] if test_out else "",
        },
        "notClaimed": [
            "viewport/capture parity",
            "stairs artwork rendering",
            "new original DOS runtime trace",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    REPORT.write_text("\n".join([
        "# Pass506 - DM1 V1 stairs movement side-effect source lock",
        "",
        f"Status: {status}",
        "",
        "## ReDMCSB-first source audit",
        f"- CLIKMENU.C:{f0364_start}-{f0364_end} / F0364 takes stairs by calling F0267 with a non-square destination, resolving level/offset, setting the target map for exit-direction lookup, then restoring the current map.",
        f"- CLIKMENU.C:{f0365_start}-{f0365_end} / F0365 turns on a stairs square by taking stairs and returning before same-square turn leave/enter sensors.",
        f"- CLIKMENU.C:{f0366_start}-{f0366_end} / F0366 handles backward-on-stairs before relative stepping, target-stairs before normal blockers/cooldown, and source-stairs normal steps with a non-square source before normal cooldown.",
        f"- DUNGEON.C:{f0154_start}-{f0154_end} and DUNGEON.C:{f0155_start}-{f0155_end} bind stairs target map/coordinates and exit direction.",
        "",
        "## Firestaff guards",
        "- dm1_v1_movement_command_core_pc34_compat.c now records source-stairs walk-off suppression for normal steps while preserving destination walk-on and cooldown.",
        "- test_dm1_v1_movement_pipeline_pc34_compat.c covers target-stairs, backward-on-stairs, and source-stairs-to-corridor step semantics.",
        "",
        "## Scope guard",
        "- Movement-only evidence. No viewport, capture, or artwork lane changes.",
        "",
        f"Manifest: parity-evidence/verification/{PASS}/manifest.json",
    ]) + "\n")

    print(f"PASS {PASS}")
    print(f"- ReDMCSB CLIKMENU.C:{f0364_start}-{f0364_end}, {f0365_start}-{f0365_end}, {f0366_start}-{f0366_end}")
    print(f"- ReDMCSB DUNGEON.C:{f0154_start}-{f0154_end}, {f0155_start}-{f0155_end}")
    print(f"- report: {REPORT.relative_to(ROOT)}")
    print(f"- manifest: {MANIFEST.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, subprocess.SubprocessError) as exc:
        print(f"FAIL {PASS}: {exc}", file=sys.stderr)
        raise SystemExit(1)
