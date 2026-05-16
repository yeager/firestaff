#!/usr/bin/env python3
from __future__ import annotations
import json, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT = ROOT / "parity-evidence/verification/pass564_dm1_v1_movement_collision_timing_cluster"

SOURCE = [
    ("COMMAND.C", "2045-2156", "F0380 queue lock, movement cooldown gate, dequeue, pending replay, turn/step dispatch", [
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "G0310_i_DisabledMovementTicks",
        "G0311_i_ProjectileDisabledMovementTicks",
        "G0312_i_LastProjectileDisabledMovementDirection == (M021_NORMALIZE(G0308_i_PartyDirection + L1160_i_Command - C003_COMMAND_MOVE_FORWARD))",
        "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
        "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ]),
    ("CLIKMENU.C", "142-174", "F0365 turn/stairs/current-square sensor boundary", [
        "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "F0364_COMMAND_TakeStairs",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, C1_TRUE, C0_FALSE);",
        "F0284_CHAMPION_SetPartyDirection",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, C1_TRUE, C1_TRUE);",
    ]),
    ("CLIKMENU.C", "180-347", "F0366 stamina, relative step, blockers, discard, F0267, cooldown install", [
        "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "G0465_ai_Graphic561_MovementArrowToStepForwardCount",
        "G0466_ai_Graphic561_MovementArrowToStepRightCount",
        "F0325_CHAMPION_DecrementStamina",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "L1117_B_MovementBlocked = C1_TRUE;",
        "L1117_B_MovementBlocked = M036_DOOR_STATE(AL1115_ui_Square);",
        "MASK0x0004_FAKEWALL_OPEN",
        "F0175_GROUP_GetThing(L1121_i_MapX, L1122_i_MapY) != C0xFFFE_THING_ENDOFLIST",
        "F0209_GROUP_ProcessEvents29to41",
        "F0357_COMMAND_DiscardAllInput();",
        "F0693_WaitVerticalBlank();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "F0267_MOVE_GetMoveResult_CPSCE",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        "G0311_i_ProjectileDisabledMovementTicks = 0;",
    ]),
    ("DUNGEON.C", "1371-1391", "F0150 relative forward/right coordinate math", [
        "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount",
        "P0253_i_Direction += 1, P0253_i_Direction &= 3;",
        "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0255_i_StepsRightCount",
    ]),
    ("MOVESENS.C", "438-444", "F0267 immediate party coordinate update", [
        "if (P0557_T_Thing == C0xFFFF_THING_PARTY)",
        "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
        "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
    ]),
    ("MOVESENS.C", "738-783", "F0267 move-result globals, scent, G0362 timing", [
        "G0397_i_MoveResultMapX = P0560_i_DestinationMapX;",
        "G0398_i_MoveResultMapY = P0561_i_DestinationMapY;",
        "G0399_ui_MoveResultMapIndex = L0715_ui_MapIndexDestination;",
        "L0725_B_PartySquare =",
        "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;",
        "G0407_s_Party.Scents[AL0708_i_ScentIndex].Location.MapX = P0560_i_DestinationMapX;",
    ]),
    ("MOVESENS.C", "799-822", "F0267 source leave, destination group deletion, destination enter/defer", [
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);",
        "F0175_GROUP_GetThing(G0306_i_PartyMapX, G0307_i_PartyMapY)",
        "F0189_GROUP_Delete(G0306_i_PartyMapX, G0307_i_PartyMapY);",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);",
        "G0327_i_NewPartyMapIndex = L0715_ui_MapIndexDestination;",
    ]),
    ("CHAMPION.C", "1180-1215", "F0310 movement ticks formula", [
        "int16_t F0310_CHAMPION_GetMovementTicks",
        "F0309_CHAMPION_GetMaximumLoad",
        "MASK0x0020_WOUND_FEET",
        "C194_ICON_ARMOUR_BOOT_OF_SPEED",
        "return L0933_ui_Ticks;",
    ]),
    ("GAMELOOP.C", "150-155", "G0310/G0311 independent per-loop decrement", [
        "if (G0310_i_DisabledMovementTicks)",
        "G0310_i_DisabledMovementTicks--;",
        "if (G0311_i_ProjectileDisabledMovementTicks)",
        "G0311_i_ProjectileDisabledMovementTicks--;",
    ]),
]

FIRESTAFF = [
    ("dm1_v1_input_command_queue_pc34_compat.c", "queued-command gate and pending replay", ["result.movementDisabledGate = 1;", "process_pending_click(queue);", "result.dispatchedMove = 1"]),
    ("dm1_v1_movement_command_core_pc34_compat.c", "turn/step dispatch, blockers, coordinate update, cooldown", ["dm1_v1_apply_pre_step_stamina_cost", "F0702_MOVEMENT_TryMove_Compat", "dm1_v1_record_blocked_wall_or_door_damage_request", "F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat", "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);", "party->mapX = outResult->movement.newMapX;", "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat"]),
    ("dm1_v1_movement_timing_pc34_compat.c", "movement ticks, G0362 update, cooldown decrement", ["DM1_V1_MovementTiming_ComputeChampionTicksPc34Compat", "result.lastPartyMovementTime = currentGameTick;", "DM1_V1_MovementTiming_DecrementCooldownsPc34Compat"]),
    ("test_dm1_v1_movement_command_core_pc34_compat.c", "focused command core coverage", ["pc34 core disabled gate keeps command queued", "pc34 core projectile nonmatching direction processes move", "forward y decremented", "blocked movement flushes queued input", "pass547 closed door reports door block", "pass547 group reaction requested", "pass549 one-fourth door sets cooldown"]),
    ("test_dm1_v1_movement_pipeline_pc34_compat.c", "pipeline cooldown decrement/release coverage", ["test_movement_cooldown_gate", "DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat(&pipeline)", "gate_drained_step", "cmd_move_view.evidence.gameloop_cooldown"]),
]

def compact(s: str) -> str:
    return " ".join(s.split())

def read(path: Path, enc="latin-1") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=enc)

def block(path: Path, spec: str) -> str:
    a, b = [int(x) for x in spec.split("-")]
    lines = read(path).splitlines()
    if b > len(lines):
        raise AssertionError(f"{path.name}:{spec} exceeds file length {len(lines)}")
    return "\n".join(lines[a-1:b])

def require(flat: str, needle: str, where: str) -> None:
    if compact(needle) not in flat:
        raise AssertionError(f"{where} missing {needle!r}")

def main() -> int:
    source_rows = []
    for fn, spec, claim, needles in SOURCE:
        flat = compact(block(RED / fn, spec))
        for n in needles:
            require(flat, n, f"{fn}:{spec}")
        source_rows.append({"file": fn, "lines": spec, "claim": claim})
    firestaff_rows = []
    for rel, claim, needles in FIRESTAFF:
        flat = compact(read(ROOT / rel, "utf-8"))
        for n in needles:
            require(flat, n, rel)
        firestaff_rows.append({"file": rel, "claim": claim})
    cmake = read(ROOT / "CMakeLists.txt", "utf-8")
    ctests = ["dm1_v1_movement_command_core_pc34_compat", "dm1_v1_movement_pipeline_pc34_compat", "pass564_dm1_v1_movement_collision_timing_cluster"]
    for name in ctests:
        require(cmake, name, "CMakeLists.txt")
    OUT.mkdir(parents=True, exist_ok=True)
    manifest = {
        "schema": "firestaff.pass564.dm1_v1_movement_collision_timing_cluster.v1",
        "status": "pass",
        "redmcsbRoot": str(RED),
        "scope": "relative movement, collision blockers, party coordinate update, cooldown install/decrement, queued command behavior",
        "sourceAnchors": source_rows,
        "firestaffAnchors": firestaff_rows,
        "ctests": ctests,
        "dataFilesRead": [],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    md = ["# Pass564 - DM1 V1 Movement Collision Timing Cluster", "", "Status: pass", "", "No DUNGEON.DAT/GRAPHICS.DAT variants are read by this verifier.", "", "## ReDMCSB Anchors", ""]
    md += [f"- {r['file']}:{r['lines']} - {r['claim']}" for r in source_rows]
    md += ["", "## Firestaff Anchors", ""]
    md += [f"- {r['file']} - {r['claim']}" for r in firestaff_rows]
    md += ["", "## Registered CTests", ""] + [f"- {x}" for x in ctests]
    (OUT / "report.md").write_text("\n".join(md) + "\n", encoding="utf-8")
    print(f"pass564_dm1_v1_movement_collision_timing_cluster=pass anchors={len(source_rows)} evidence={OUT / 'manifest.json'}")
    return 0

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"pass564_dm1_v1_movement_collision_timing_cluster=fail {exc}", file=sys.stderr)
        raise
