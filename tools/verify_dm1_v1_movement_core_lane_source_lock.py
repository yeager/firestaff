#!/usr/bin/env python3
"""Focused DM1 V1 movement-core lane source lock.

This intentionally avoids viewport/wall rendering ownership. It proves the
ReDMCSB source path from input rows through command dequeue, turning, collision,
successful movement timing, and sensor side effects, then ties that path to the
Firestaff movement-core seams and tests.
"""
from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
).expanduser()
OUT_JSON = ROOT / "parity-evidence" / "verification" / "dm1_v1_movement_core_lane_source_lock.json"
OUT_MD = ROOT / "parity-evidence" / "verification" / "dm1_v1_movement_core_lane_source_lock.md"


@dataclass(frozen=True)
class SourceRange:
    file: str
    lines: str
    claim: str
    needles: tuple[str, ...]


@dataclass(frozen=True)
class FirestaffEvidence:
    file: str
    claim: str
    needles: tuple[str, ...]


RED_RANGES = (
    SourceRange("COMMAND.C", "106-121", "mouse movement rows map visible arrows and dungeon viewport to command ids", ("G0448_as_Graphic561_SecondaryMouseInput_Movement", "C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C006_COMMAND_MOVE_LEFT", "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW")),
    SourceRange("COMMAND.C", "677-685", "PC-34 keyboard movement rows map keypad/arrow codes to turn and step commands", ("MEDIA707_I34E_I34M", "C001_COMMAND_TURN_LEFT,     0x004B", "C003_COMMAND_MOVE_FORWARD,  0x004C", "C002_COMMAND_TURN_RIGHT,    0x004D", "C006_COMMAND_MOVE_LEFT,     0x004F", "C005_COMMAND_MOVE_BACKWARD, 0x0050", "C004_COMMAND_MOVE_RIGHT,    0x0051")),
    SourceRange("IO2.C", "5-61", "PC input normalizes shifted extended movement keys before command-table lookup", ("F0540_INPUT_Crawcin", "switch (L2944_ui_ - 0x1248)", "L2944_ui_ = 'L'", "L2944_ui_ = 'P'", "L2944_ui_ = 'K'", "L2944_ui_ = 'M'")),
    SourceRange("COMMAND.C", "2045-2156", "F0380 locks the queue, keeps gated movement queued, dequeues one command, then dispatches turns and steps", ("void F0380_COMMAND_ProcessQueue_CPSC", "G0435_B_CommandQueueLocked = C1_TRUE;", "G0310_i_DisabledMovementTicks", "G0311_i_ProjectileDisabledMovementTicks", "G0435_B_CommandQueueLocked = C0_FALSE;", "F0360_COMMAND_ProcessPendingClick();", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);")),
    SourceRange("CLIKMENU.C", "142-174", "F0365 turning sets stop-wait, handles stairs as a special case, otherwise applies direction change with sensor boundaries", ("void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0364_COMMAND_TakeStairs", "F0276_SENSOR_ProcessThingAdditionOrRemoval", "F0284_CHAMPION_SetPartyDirection")),
    SourceRange("CLIKMENU.C", "180-347", "F0366 steps charge stamina before resolution, blocks walls/doors/fakewalls/groups before F0267, and applies cooldown only after accepted movement", ("void F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0465_ai_Graphic561_MovementArrowToStepForwardCount", "F0325_CHAMPION_DecrementStamina", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "C00_ELEMENT_WALL", "C04_ELEMENT_DOOR", "C06_ELEMENT_FAKEWALL", "F0175_GROUP_GetThing", "F0357_COMMAND_DiscardAllInput();", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;", "G0311_i_ProjectileDisabledMovementTicks = 0;")),
    SourceRange("DUNGEON.C", "1371-1440", "relative forward/right deltas are applied from direction tables before current-map square lookup", ("void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "G0233_ai_Graphic559_DirectionToStepEastCount", "P0253_i_Direction += 1, P0253_i_Direction &= 3", "unsigned char F0151_DUNGEON_GetSquare", "G0271_ppuc_CurrentMapData")),
    SourceRange("CHAMPION.C", "1180-1215", "movement cadence is derived from load, feet wounds, and Boots of Speed", ("int16_t F0310_CHAMPION_GetMovementTicks", "F0309_CHAMPION_GetMaximumLoad", "MASK0x0020_WOUND_FEET", "C194_ICON_ARMOUR_BOOT_OF_SPEED", "return L0933_ui_Ticks;")),
    SourceRange("MOVESENS.C", "316-443", "F0267 owns the source/destination move-result contract and projectile-impact precheck", ("BOOLEAN F0267_MOVE_GetMoveResult_CPSCE", "P0558_i_SourceMapX", "P0560_i_DestinationMapX", "F0266_MOVE_IsKilledByProjectileImpact", "return C1_TRUE")),
    SourceRange("MOVESENS.C", "738-818", "accepted party moves record result/timing/scent then run source leave and destination enter sensor passes", ("G0397_i_MoveResultMapX = P0560_i_DestinationMapX;", "G0398_i_MoveResultMapY = P0561_i_DestinationMapY;", "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;", "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX", "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX")),
)

FIRESTAFF_EVIDENCE = (
    FirestaffEvidence("dm1_v1_input_command_queue_pc34_compat.c", "PC-34 input rows, queue lock, gated movement retention, pending-click replay, and blocked-input discard", ("DM1_V1_InputCommandQueue_ProcessOnePc34Compat", "disabledMovementTicks", "projectileDisabledMovementTicks", "movementDisabledGate", "process_pending_click(queue)", "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat")),
    FirestaffEvidence("dm1_v1_movement_command_core_pc34_compat.c", "F0380-to-F0365/F0366 seam: turns bypass movement gates, steps apply stamina/collision/group blocking/timing", ("DM1_V1_MovementCommandCore_ProcessOnePc34Compat", "dm1_v1_is_turn_command", "dm1_v1_apply_pre_step_stamina_cost", "F0702_MOVEMENT_TryMove_Compat", "movementBlocked", "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat")),
    FirestaffEvidence("memory_movement_pc34_compat.c", "pure movement legality and target-square result semantics behind F0366", ("F0700_MOVEMENT_TurnDirection_Compat", "F0701_MOVEMENT_GetStepDelta_Compat", "F0702_MOVEMENT_TryMove_Compat", "MOVE_BLOCKED_WALL", "MOVE_BLOCKED_DOOR")),
    FirestaffEvidence("dm1_v1_movement_timing_pc34_compat.c", "post-step cadence mirrors F0310/F0366 timing side effects", ("DM1_V1_MovementTiming_ComputeChampionTicksPc34Compat", "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat", "disabledMovementTicks", "projectileDisabledMovementTicks", "lastPartyMovementTime")),
    FirestaffEvidence("test_dm1_v1_movement_command_core_pc34_compat.c", "behavior coverage for queued input through turn/step dispatch, collision, stamina, timing, and sensor order", ("turn does not set movement cooldown", "blocked movement reported", "stamina cost", "step records scent/last movement time", "step updates last movement time")),
    FirestaffEvidence("test_dm1_v1_command_movement_sensor_timing_pc34_compat.c", "integration coverage for PC-34 movement queue, collision, turn, sensor, and timing edges", ("projectile same-direction movement gate reported", "turn current-square sensor", "mouse forward command dequeued", "core forward1 destination sensors", "core turn bypasses movement gates")),
)


def read(path: Path, encoding: str = "latin-1") -> str:
    return path.read_text(encoding=encoding)


def slice_lines(text: str, line_range: str) -> str:
    start_s, end_s = line_range.split("-", 1)
    start = int(start_s)
    end = int(end_s)
    lines = text.splitlines()
    return "\n".join(lines[start - 1 : end])


def require_needles(label: str, haystack: str, needles: tuple[str, ...]) -> None:
    missing = [needle for needle in needles if needle not in haystack]
    if missing:
        raise AssertionError(f"{label}: missing {missing!r}")


def main() -> int:
    red_citations = []
    firestaff_citations = []

    for source in RED_RANGES:
        full_text = read(RED / source.file)
        window = slice_lines(full_text, source.lines)
        require_needles(f"{source.file}:{source.lines}", window, source.needles)
        red_citations.append({"file": source.file, "lines": source.lines, "claim": source.claim})

    for evidence in FIRESTAFF_EVIDENCE:
        text = read(ROOT / evidence.file, "utf-8")
        require_needles(evidence.file, text, evidence.needles)
        firestaff_citations.append({"file": evidence.file, "claim": evidence.claim})

    result = {
        "status": "PASS",
        "scope": "DM1 V1 movement core lane: input -> command -> turning/collision -> timing/sensors",
        "redmcsbRoot": str(RED),
        "redmcsbCitations": red_citations,
        "firestaffEvidence": firestaff_citations,
        "nonScope": "viewport/wall rendering code intentionally excluded",
        "requiredRuntimeGates": [
            "dm1_v1_input_command_queue_pc34_compat",
            "dm1_v1_movement_command_core_pc34_compat",
            "dm1_v1_movement_timing_pc34_compat",
            "dm1_v1_command_movement_sensor_timing_pc34_compat",
            "dm1_v1_movement_command_gate_source_lock",
            "dm1_v1_command_movement_sensor_timing_source_lock",
        ],
    }

    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")

    lines = [
        "# DM1 V1 movement core lane source lock",
        "",
        "Status: **PASS**",
        "",
        "Scope: input -> command queue -> turning/collision -> timing/sensors. Viewport/wall rendering is intentionally out of scope.",
        "",
        "## ReDMCSB citations",
        "",
    ]
    lines += [f"- {entry['file']}:{entry['lines']} - {entry['claim']}" for entry in red_citations]
    lines += ["", "## Firestaff evidence", ""]
    lines += [f"- {entry['file']} - {entry['claim']}" for entry in firestaff_citations]
    lines += [
        "",
        "## Required gates",
        "",
        "- dm1_v1_input_command_queue_pc34_compat",
        "- dm1_v1_movement_command_core_pc34_compat",
        "- dm1_v1_movement_timing_pc34_compat",
        "- dm1_v1_command_movement_sensor_timing_pc34_compat",
        "- dm1_v1_movement_command_gate_source_lock",
        "- dm1_v1_command_movement_sensor_timing_source_lock",
    ]
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"dm1_v1_movement_core_lane_source_lock=PASS output={OUT_JSON}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
