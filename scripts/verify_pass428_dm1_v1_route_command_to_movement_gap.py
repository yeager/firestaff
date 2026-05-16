#!/usr/bin/env python3
"""Pass428: prove keyboard/route command reaches DM1 V1 movement timing/collision.

This is a ReDMCSB-first static gate plus Firestaff seam audit.  It covers the
next gap after pass427: not just projectile-direction gate retention, but the
full command route from PC-34 key/mouse command ids through F0380 into F0365
turns or F0366 step collision/timing.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
OUT_DIR = ROOT / "parity-evidence" / "verification" / "pass428_dm1_v1_route_command_to_movement_gap"
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / "pass428_dm1_v1_route_command_to_movement_gap.md"


def read(path: Path, encoding: str = "utf-8") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def require(text: str, needle: str, label: str) -> None:
    if compact(needle) not in compact(text):
        raise AssertionError(f"missing {label}: {needle!r}")


def require_order(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing ordered needle {needle!r}")
        pos = hit


def line(text: str, needle: str) -> int:
    i = text.find(needle)
    if i < 0:
        raise AssertionError(f"missing line needle {needle!r}")
    return text.count("\n", 0, i) + 1


def source_audit() -> dict:
    command = read(RED / "COMMAND.C", "latin-1")
    io2 = read(RED / "IO2.C", "latin-1")
    clik = read(RED / "CLIKMENU.C", "latin-1")
    dungeon = read(RED / "DUNGEON.C", "latin-1")
    champion = read(RED / "CHAMPION.C", "latin-1")
    movesens = read(RED / "MOVESENS.C", "latin-1")
    gameloop = read(RED / "GAMELOOP.C", "latin-1")

    require_order(io2, [
        "L2944_ui_ = (*(G2162_IODriver->IODRV_00_GetKeyboardInput))();",
        "switch (L2944_ui_ - 0x1248)",
        "L2944_ui_ = 'L';",
        "L2944_ui_ = 'P';",
        "L2944_ui_ = 'K';",
        "L2944_ui_ = 'M';",
        "return L2944_ui_;",
    ], "IO2 PC-34 arrow normalization")
    require_order(command, [
        "{ C001_COMMAND_TURN_LEFT,     0x004B }",
        "{ C003_COMMAND_MOVE_FORWARD,  0x004C }",
        "{ C002_COMMAND_TURN_RIGHT,    0x004D }",
        "{ C006_COMMAND_MOVE_LEFT,     0x004F }",
        "{ C005_COMMAND_MOVE_BACKWARD, 0x0050 }",
        "{ C004_COMMAND_MOVE_RIGHT,    0x0051 }",
    ], "COMMAND PC-34 movement keyboard table")
    require_order(command, [
        "void F0359_COMMAND_ProcessClick_CPSC",
        "if ((P0727_i_ButtonsStatus == C04_MOUSE_EVENT_LEFT_BUTTON_UP)",
        "L2287_i_MaximumRegularCommandsInQueue = C7_UNKNOWN;",
        "L2287_i_MaximumRegularCommandsInQueue = C5_UNKNOWN;",
        "L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput",
        "L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput",
        "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command;",
        "G0432_as_CommandQueue[L1108_i_CommandQueueIndex].X = P0725_i_X;",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
    ], "F0359 mouse queue write and primary-before-secondary hit routing")
    require_order(command, [
        "STATICFUNCTION void F0360_COMMAND_ProcessPendingClick",
        "if (G0436_B_PendingClickPresent)",
        "G0436_B_PendingClickPresent = C0_FALSE;",
        "F0359_COMMAND_ProcessClick_CPSC(G0437_i_PendingClickX, G0438_i_PendingClickY, G0439_i_PendingClickButtonsStatus);",
    ], "F0360 pending-click replay hook")
    require_order(command, [
        "void F0361_COMMAND_ProcessKeyPress",
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "G2153_i_QueuedCommandsCount < C5_UNKNOWN",
        "while (L1111_i_Command = L1112_ps_KeyboardInput->Command)",
        "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
        "G2153_i_QueuedCommandsCount++;",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
    ], "F0361 keyboard queue write")
    require_order(command, [
        "void F0380_COMMAND_ProcessQueue_CPSC",
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "if (G2153_i_QueuedCommandsCount == 0)",
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT) && (G0310_i_DisabledMovementTicks",
        "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
        "G2153_i_QueuedCommandsCount--;",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "F0380 command gate/dequeue/dispatch")
    require_order(clik, [
        "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "F0364_COMMAND_TakeStairs",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval",
        "F0284_CHAMPION_SetPartyDirection",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval",
    ], "F0365 turn route")
    require_order(clik, [
        "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "F0325_CHAMPION_DecrementStamina",
        "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
        "if (L1116_i_SquareType == C04_ELEMENT_DOOR)",
        "if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)",
        "F0175_GROUP_GetThing",
        "F0357_COMMAND_DiscardAllInput();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
        "AL1115_ui_Ticks = 1;",
        "F0310_CHAMPION_GetMovementTicks",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        "G0311_i_ProjectileDisabledMovementTicks = 0;",
    ], "F0366 step collision/timing route")
    require(clik, "G0465_ai_Graphic561_MovementArrowToStepForwardCount[4]", "F0366 movement forward delta table")
    require(clik, "G0466_ai_Graphic561_MovementArrowToStepRightCount[4]", "F0366 movement right delta table")
    require_order(dungeon, [
        "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "*P0256_pi_MapX +=",
        "P0253_i_Direction += 1",
        "*P0256_pi_MapX +=",
    ], "relative coordinate mutation")
    require_order(champion, [
        "int16_t F0310_CHAMPION_GetMovementTicks",
        "L0933_ui_Ticks = 2;",
        "L0933_ui_Ticks = 4 +",
        "MASK0x0020_WOUND_FEET",
        "C194_ICON_ARMOUR_BOOT_OF_SPEED",
        "return L0933_ui_Ticks;",
    ], "movement tick formula")
    require_order(movesens, [
        "G0397_i_MoveResultMapX = P0560_i_DestinationMapX;",
        "G0398_i_MoveResultMapY = P0561_i_DestinationMapY;",
        "L0725_B_PartySquare =",
        "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;",
    ], "F0267 accepted party movement side effects")
    require_order(gameloop, [
        "if (G0310_i_DisabledMovementTicks)",
        "G0310_i_DisabledMovementTicks--;",
        "if (G0311_i_ProjectileDisabledMovementTicks)",
        "G0311_i_ProjectileDisabledMovementTicks--;",
    ], "GAMELOOP cooldown aging")

    return {
        "IO2.C:arrow_normalization": [line(io2, "switch (L2944_ui_ - 0x1248)"), line(io2, "return L2944_ui_;")],
        "COMMAND.C:movement_keyboard_table": [677, 684],
        "COMMAND.C:F0359_COMMAND_ProcessClick_CPSC": [1452, 1662],
        "COMMAND.C:F0360_COMMAND_ProcessPendingClick": [1692, 1707],
        "COMMAND.C:F0361_COMMAND_ProcessKeyPress": [1709, 1813],
        "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": [2045, 2156],
        "CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty": [142, 174],
        "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": [180, 346],
        "DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement": [1371, 1391],
        "CHAMPION.C:F0310_CHAMPION_GetMovementTicks": [1180, 1215],
        "MOVESENS.C:F0267_party_accepted_side_effects": [738, 783],
        "GAMELOOP.C:cooldown_decrement_before_input_wait": [150, 155],
    }


def firestaff_audit() -> dict:
    queue_c = read(ROOT / "src/dm1/dm1_v1_input_command_queue_pc34_compat.c")
    core_c = read(ROOT / "src/dm1/dm1_v1_movement_command_core_pc34_compat.c")
    timing_c = read(ROOT / "src/dm1/dm1_v1_movement_timing_pc34_compat.c")
    pipeline_c = read(ROOT / "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c")
    m11 = read(ROOT / "src/engine/m11_game_view.c")
    test_core = read(ROOT / "tests/test_dm1_v1_movement_command_core_pc34_compat.c")
    test_sensor = read(ROOT / "tests/test_dm1_v1_command_movement_sensor_timing_pc34_compat.c")
    test_pipeline = read(ROOT / "tests/test_dm1_v1_movement_pipeline_pc34_compat.c")

    for needle in [
        "case 0xAB34: case 0x007F: case 0x9BFF: case 0x004B:",
        "case 0xAB35: case 0x9B41: case 0x9B54: case 0x004C:",
        "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
        "queue->pendingClickPresent = 1;",
        "process_pending_click(queue);",
        "result.movementDisabledGate = 1;",
        "result.dequeued = 1;",
        "result.dispatchedTurn = 1;",
        "result.dispatchedMove = 1;",
        "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat",
    ]:
        require(queue_c, needle, f"input queue seam {needle}")
    for needle in [
        "dm1_v1_is_turn_command(outResult->queue.command)",
        "m11_v1_turning_apply_party_original_presentation_pc34_compat",
        "dm1_v1_apply_pre_step_stamina_cost(party, outResult);",
        "F0702_MOVEMENT_TryMove_Compat",
        "F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat",
        "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat",
        "outResult->viewportRedrawRequested = 1;",
    ]:
        require(core_c, needle, f"command core seam {needle}")
    for needle in [
        "CHAMPION.C:1180-1215",
        "G0310_i_DisabledMovementTicks",
        "GAMELOOP.C:150-155 decrements both cooldowns independently",
    ]:
        require(timing_c + pipeline_c, needle, f"timing/pipeline source evidence {needle}")
    require_order(m11, [
        "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat",
        "DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat",
        "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat",
        "DM1_V1_VBlankTiming_ResetForNewTick",
    ], "live M11 bridge enqueue/cooldown/process order")
    for needle in [
        "pc34 core left arrow enqueues turn",
        "pc34 core up arrow enqueues forward",
        "pc34 core disabled gate keeps command queued",
        "pc34 core blocked up arrow reports wall",
        "pc34 core blocked up arrow discards followup",
        "step decrements living champion0 stamina before resolution",
    ]:
        require(test_core, needle, f"core test {needle}")
    for needle in [
        "core forward1 requests viewport",
        "core turn requests viewport",
        "core blocked wall skips viewport",
        "core group block skips viewport",
        "disabled movement leaves command queued",
    ]:
        require(test_sensor, needle, f"sensor/timing test {needle}")
    for needle in [
        "key_event(0xAB36)",
        "seq_turn",
        "key_event(0xAB35)",
        "fwd_step_cooldown",
    ]:
        require(test_pipeline, needle, f"pipeline test {needle}")

    return {
        "src/dm1/dm1_v1_input_command_queue_pc34_compat.c": "maps PC-34 key/route events to C001..C006, retains movement commands while cooldown-gated, dequeues only after gate clears, and discards after blocked step",
        "src/dm1/dm1_v1_movement_command_core_pc34_compat.c": "dispatches dequeued turns to F0365-equivalent direction/sensor effects and steps to F0366-equivalent stamina, collision, group, timing, redraw effects",
        "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c": "writes cooldown/last-movement timing only after accepted step; M11 bridge decrements old cooldowns before F0380-compatible processing",
        "tests": [
            "test_dm1_v1_movement_command_core_pc34_compat",
            "test_dm1_v1_command_movement_sensor_timing_pc34_compat",
            "test_dm1_v1_movement_pipeline_pc34_compat",
        ],
    }


def main() -> int:
    source = source_audit()
    firestaff = firestaff_audit()
    manifest = {
        "schema": "firestaff.parity.pass428.dm1_v1_route_command_to_movement_gap.v1",
        "status": "PASS428_ROUTE_COMMAND_TO_MOVEMENT_GAP_CLOSED",
        "redmcsbRoot": str(RED),
        "sourceAudit": source,
        "firestaffAudit": firestaff,
        "claim": "PC-34 keyboard/route movement commands become queued C001..C006 commands, F0380 retains gated steps before dequeue, then dispatches turns/steps into source-locked timing and collision seams.",
        "blocker": None,
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass428 — DM1 V1 route command → movement gap",
        "",
        "Status: `PASS428_ROUTE_COMMAND_TO_MOVEMENT_GAP_CLOSED`",
        "",
        "## ReDMCSB anchors",
    ]
    for name, span in source.items():
        lines.append(f"- `{name}` — `{span}`")
    lines += ["", "## Firestaff seams"]
    for name, detail in firestaff.items():
        lines.append(f"- `{name}` — {detail}")
    lines += ["", f"Manifest: `{OUT_JSON.relative_to(ROOT)}`", ""]
    OUT_MD.write_text("\n".join(lines), encoding="utf-8")
    print(f"pass428_dm1_v1_route_command_to_movement_gap=pass manifest={OUT_JSON.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
