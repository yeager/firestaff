#!/usr/bin/env python3
"""Pass507: Lane A DM1 V1 movement/stairs/group/timing source-lock gate."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
OUT_DIR = ROOT / "parity-evidence/verification/pass507_dm1_v1_movement_stairs_group_timing_source_lock"
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass507_dm1_v1_movement_stairs_group_timing_source_lock.md"


def read(path: Path, enc: str = "utf-8") -> str:
    return path.read_text(encoding=enc, errors="replace")


def line(text: str, pos: int) -> int:
    return text.count("\n", 0, pos) + 1


def after(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def ordered(text: str, markers: list[str], label: str, start: int = 0) -> str:
    positions = []
    cursor = start
    for marker in markers:
        pos = text.find(marker, cursor)
        if pos < 0:
            raise AssertionError(f"{label}: missing {marker!r}")
        positions.append(pos)
        cursor = pos + len(marker)
    return f"{line(text, positions[0])}-{line(text, positions[-1])}"


def body(text: str, name: str) -> str:
    start = after(text, name, name)
    brace = text.find("{", start)
    depth = 0
    for idx in range(brace, len(text)):
        if text[idx] == "{":
            depth += 1
        elif text[idx] == "}":
            depth -= 1
            if depth == 0:
                return text[start:idx + 1]
    raise AssertionError(f"unterminated {name}")


def require_all(text: str, needles: list[str], label: str) -> None:
    missing = [n for n in needles if n not in text]
    if missing:
        raise AssertionError(f"{label}: missing {missing!r}")


def src_rows() -> list[dict[str, str]]:
    command = read(RED / "COMMAND.C", "latin-1")
    clik = read(RED / "CLIKMENU.C", "latin-1")
    moves = read(RED / "MOVESENS.C", "latin-1")
    champion = read(RED / "CHAMPION.C", "latin-1")
    gameloop = read(RED / "GAMELOOP.C", "latin-1")
    dungeon = read(RED / "DUNGEON.C", "latin-1")
    rows = []

    f0380 = after(command, "void F0380_COMMAND_ProcessQueue_CPSC", "F0380")
    rows.append({"source": "COMMAND.C:" + ordered(command, [
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "G0310_i_DisabledMovementTicks || (G0311_i_ProjectileDisabledMovementTicks",
        "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "queue/dequeue/dispatch", f0380), "claim": "F0380 locks queue, gates movement before dequeue, replays pending click, then dispatches turn/move handlers."})

    f0366 = after(clik, "void F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0366")
    rows.append({"source": "CLIKMENU.C:" + ordered(clik, [
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "if (L1119_ps_Champion->CurrentHealth)",
        "F0325_CHAMPION_DecrementStamina",
        "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;",
    ], "stamina before movement", f0366), "claim": "F0366 applies living-champion stamina before arrow/stair/blocker handling."})
    rows.append({"source": "CLIKMENU.C:" + ordered(clik, [
        "static int16_t G0465_ai_Graphic561_MovementArrowToStepForwardCount[4] = {",
        "1,   /* Forward */",
        "-1,  /* Backward */",
        "static int16_t G0466_ai_Graphic561_MovementArrowToStepRightCount[4] = {",
        "1,    /* Right */",
        "-1 }; /* Left */",
    ], "relative step tables"), "claim": "Movement arrow tables source-lock forward/right/back/left deltas."})
    rows.append({"source": "DUNGEON.C:" + ordered(dungeon, [
        "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount",
        "P0253_i_Direction += 1, P0253_i_Direction &= 3; /* Simulate turning right */",
        "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0255_i_StepsRightCount",
    ], "relative coordinate math", after(dungeon, "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0150")), "claim": "Relative movement applies forward deltas, then simulated-right-turn strafe deltas."})
    rows.append({"source": "CLIKMENU.C:" + ordered(clik, [
        "L1123_B_StairsSquare = (M034_SQUARE_TYPE",
        "if (L1123_B_StairsSquare && (AL1118_ui_MovementArrowIndex == 2))",
        "F0364_COMMAND_TakeStairs",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "if (L1116_i_SquareType == C03_ELEMENT_STAIRS)",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, CM1_MAPX_NOT_ON_A_SQUARE, 0);",
        "F0364_COMMAND_TakeStairs",
    ], "stairs handling", f0366), "claim": "Current-square and target-square stairs route through F0364/F0267 before ordinary blocker flow."})
    rows.append({"source": "CLIKMENU.C:" + ordered(clik, [
        "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
        "L1117_B_MovementBlocked = C1_TRUE;",
        "if (L1116_i_SquareType == C04_ELEMENT_DOOR)",
        "L1117_B_MovementBlocked = (L1117_B_MovementBlocked != C0_DOOR_STATE_OPEN)",
        "if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)",
        "L1117_B_MovementBlocked = (!M007_GET(AL1115_ui_Square, MASK0x0004_FAKEWALL_OPEN)",
    ], "tile blockers", f0366), "claim": "Wall, closed-door and closed-real-fakewall blockers happen before accepted movement."})
    rows.append({"source": "CLIKMENU.C:" + ordered(clik, [
        "if (G0305_ui_PartyChampionCount == 0) { /* BUG0_85",
        "if (L1117_B_MovementBlocked)",
        "if (L1117_B_MovementBlocked = (F0175_GROUP_GetThing(L1121_i_MapX, L1122_i_MapY) != C0xFFFE_THING_ENDOFLIST))",
        "F0209_GROUP_ProcessEvents29to41",
    ], "group collision", f0366), "claim": "Empty-party group collision is skipped; non-empty parties block on F0175_GROUP_GetThing and trigger reaction."})
    rows.append({"source": "CLIKMENU.C:" + ordered(clik, [
        "if (L1117_B_MovementBlocked)",
        "F0357_COMMAND_DiscardAllInput();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "return;",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY",
    ], "blocked before accepted move", f0366), "claim": "Blocked movement discards input and returns before F0267/timing/sensor side effects."})
    rows.append({"source": "CLIKMENU.C:" + ordered(clik, [
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
        "AL1115_ui_Ticks = 1;",
        "if (L1119_ps_Champion->CurrentHealth)",
        "AL1115_ui_Ticks = F0025_MAIN_GetMaximumValue(AL1115_ui_Ticks, F0310_CHAMPION_GetMovementTicks(L1119_ps_Champion));",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        "G0311_i_ProjectileDisabledMovementTicks = 0;",
    ], "successful timing", f0366), "claim": "Successful movement sets disabled ticks from the slowest living champion and clears projectile movement cooldown."})
    rows.append({"source": "CHAMPION.C:" + ordered(champion, [
        "int16_t F0310_CHAMPION_GetMovementTicks",
        "F0309_CHAMPION_GetMaximumLoad",
        "L0933_ui_Ticks = 2;",
        "L0933_ui_Ticks = 4 +",
        "MASK0x0020_WOUND_FEET",
        "C194_ICON_ARMOUR_BOOT_OF_SPEED",
        "return L0933_ui_Ticks;",
    ], "movement ticks", after(champion, "int16_t F0310_CHAMPION_GetMovementTicks", "F0310")), "claim": "Champion movement cadence depends on load/maxLoad, feet wounds, and Boots of Speed."})

    f0267 = after(moves, "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE", "F0267")
    rows.append({"source": "MOVESENS.C:" + ordered(moves, [
        "G0397_i_MoveResultMapX = P0560_i_DestinationMapX;",
        "G0398_i_MoveResultMapY = P0561_i_DestinationMapY;",
        "L0725_B_PartySquare = (L0715_ui_MapIndexDestination == L0714_ui_MapIndexSource)",
        "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;",
        "G0407_s_Party.Scents[AL0708_i_ScentIndex].Location.MapX = P0560_i_DestinationMapX;",
    ], "result/time/scent", f0267), "claim": "F0267 records result coordinates and last-movement/scent only for real party square changes with champions."})
    rows.append({"source": "MOVESENS.C:" + ordered(moves, [
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);",
        "if (P0560_i_DestinationMapX >= 0)",
        "if (P0557_T_Thing == C0xFFFF_THING_PARTY)",
        "if ((P0557_T_Thing = F0175_GROUP_GetThing(G0306_i_PartyMapX, G0307_i_PartyMapY)) != C0xFFFE_THING_ENDOFLIST)",
        "F0189_GROUP_Delete(G0306_i_PartyMapX, G0307_i_PartyMapY);",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);",
    ], "sensor order", f0267), "claim": "Accepted party movement processes source leave before destination enter; party-on-group deletes the group."})
    rows.append({"source": "MOVESENS.C:" + ordered(moves, [
        "if (L0710_i_ThingType == C04_THING_TYPE_GROUP)",
        "if (((L0715_ui_MapIndexDestination == G0309_i_PartyMapIndex)",
        "F0175_GROUP_GetThing(P0560_i_DestinationMapX, P0561_i_DestinationMapY) != C0xFFFE_THING_ENDOFLIST",
        "F0265_MOVE_CreateEvent60To61_MoveGroup",
        "return C1_TRUE; /* The specified group thing cannot be moved",
    ], "group move collision", f0267), "claim": "Group moves cannot enter party/occupied group squares; a later group-move event is scheduled."})
    rows.append({"source": "GAMELOOP.C:" + ordered(gameloop, [
        "if (G0310_i_DisabledMovementTicks)",
        "G0310_i_DisabledMovementTicks--;",
        "if (G0311_i_ProjectileDisabledMovementTicks)",
        "G0311_i_ProjectileDisabledMovementTicks--;",
        "F0380_COMMAND_ProcessQueue_CPSC();",
    ], "cooldown before queue"), "claim": "Main loop ages movement/projectile cooldown before processing queued commands."})
    return rows


def firestaff_rows() -> list[dict[str, str]]:
    files = {
        "queue": read(ROOT / "dm1_v1_input_command_queue_pc34_compat.c"),
        "move": read(ROOT / "memory_movement_pc34_compat.c"),
        "core": read(ROOT / "dm1_v1_movement_command_core_pc34_compat.c"),
        "timing": read(ROOT / "dm1_v1_movement_timing_pc34_compat.c"),
        "sensor": read(ROOT / "memory_sensor_execution_pc34_compat.c"),
        "test_core": read(ROOT / "test_dm1_v1_movement_core_pc34_compat.c"),
        "test_int": read(ROOT / "test_dm1_v1_command_movement_sensor_timing_pc34_compat.c"),
    }
    checks = [
        ("dm1_v1_input_command_queue_pc34_compat.c", "DM1_V1_InputCommandQueue_ProcessOnePc34Compat", files["queue"], ["queue->locked = 1;", "is_move_command(result.command)", "disabledMovementTicks", "projectileDisabledMovementTicks", "process_pending_click(queue)", "result.dequeued = 1", "result.dispatchedMove = 1"], "F0380 move gate/replay/dispatch seam."),
        ("memory_movement_pc34_compat.c", "F0701_MOVEMENT_GetStepDelta_Compat", files["move"], ["case MOVE_FORWARD:  stepDir = direction; break;", "case MOVE_RIGHT:    stepDir = (direction + 1) & 3; break;", "case MOVE_BACKWARD: stepDir = (direction + 2) & 3; break;", "case MOVE_LEFT:     stepDir = (direction + 3) & 3; break;"], "relative step delta seam."),
        ("memory_movement_pc34_compat.c", "F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat", files["move"], ["party->championCount <= 0", "DUNGEON_SQUARE_MASK_THING_LIST", "THING_GET_TYPE(thing) == THING_TYPE_GROUP", "return 1;"], "empty-party exception and group block seam."),
        ("dm1_v1_movement_command_core_pc34_compat.c", "DM1_V1_MovementCommandCore_ProcessOnePc34Compat", files["core"], ["dm1_v1_apply_pre_step_stamina_cost", "F0705_MOVEMENT_ResolveStairsTransition_Compat", "F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat", "inputDiscardRequested = 1", "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat"], "stamina/stairs/group/input-discard/timing command seam."),
        ("dm1_v1_movement_timing_pc34_compat.c", "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat", files["timing"], ["DM1_V1_MovementTiming_ComputePartyStepTicksPc34Compat", "projectileDisabledMovementTicks = 0", "scentRecorded = 1", "lastPartyMovementTime = currentGameTick"], "successful-step timing seam."),
        ("memory_sensor_execution_pc34_compat.c", "F0718_SENSOR_ProcessPartyEnterLeave_Compat", files["sensor"], ["F0717_SENSOR_EnumerateOnSquare_Compat", "F0710_SENSOR_Execute_Compat", "outList->effects[outList->count++]"], "source-ordered enter/leave sensor walking."),
    ]
    rows = []
    for file_name, func, text, needles, claim in checks:
        require_all(body(text, func), needles, f"{file_name}:{func}")
        rows.append({"source": f"{file_name}:{line(text, after(text, func, func))}", "claim": claim})
    require_all(files["test_core"], ["movement gate leaves queued command", "wall block skips accepted-move side effects", "closed door state blocks forward", "closed real fakewall blocks forward", "pit square passable by movement dispatch", "empty party preserves source group-collision bug"], "movement core probe")
    rows.append({"source": "test_dm1_v1_movement_core_pc34_compat.c", "claim": "covers queue gates, tile blockers, pits, and empty-party group bug."})
    require_all(files["test_int"], ["blocked movement skips enter/leave sensors", "blocked movement skips timing update", "core group block clears queued followup", "empty-party group collision bug", "successful step cadence from slowest living champion", "successful step clears projectile cadence"], "movement integration probe")
    rows.append({"source": "test_dm1_v1_command_movement_sensor_timing_pc34_compat.c", "claim": "covers blocked side-effect suppression, group collision, empty-party bug, and timing cooldowns."})
    return rows


def static_gates() -> list[dict[str, object]]:
    gates = [
        ("dm1_v1_command_movement_sensor_timing_source_lock", [sys.executable, "tools/verify_dm1_v1_command_movement_sensor_timing_source_lock.py"]),
        ("dm1_v1_party_movement_sensor_order_source_lock", [sys.executable, "tools/verify_dm1_v1_party_movement_sensor_order_source_lock.py"]),
        ("dm1_v1_movement_timing_source_lock", [sys.executable, "tools/verify_dm1_v1_movement_timing_source_lock.py"]),
        ("v1_movement_legality_source_lock", [sys.executable, "tools/verify_v1_movement_legality_source_lock.py"]),
    ]
    out = []
    for name, cmd in gates:
        proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        out.append({"id": name, "cmd": cmd, "returncode": proc.returncode, "summary": proc.stdout.splitlines()[:8]})
        if proc.returncode != 0:
            raise AssertionError(f"{name} failed rc={proc.returncode}: {proc.stdout[-1000:]}")
    return out


def main() -> int:
    if not RED.is_dir():
        raise AssertionError(f"missing ReDMCSB source root: {RED}")
    red = src_rows()
    fire = firestaff_rows()
    gates = static_gates()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = {
        "schema": "firestaff.parity.pass507.dm1_v1_movement_stairs_group_timing_source_lock.v1",
        "status": "PASS507_DM1_V1_MOVEMENT_STAIRS_GROUP_TIMING_SOURCE_LOCKED",
        "generatedAt": datetime.now(timezone.utc).isoformat(),
        "sourceRoot": str(RED),
        "lane": "Lane A movement/stairs/group-collision/timing only; no viewport/pass435 dependency",
        "redmcsbEvidence": red,
        "firestaffEvidence": fire,
        "chainedStaticGates": gates,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = ["# Pass507 - DM1 V1 movement/stairs/group/timing source lock", "", "Status: PASS507_DM1_V1_MOVEMENT_STAIRS_GROUP_TIMING_SOURCE_LOCKED", "", "Lane: movement-related verifiers, parity evidence, and CTest wiring only. No viewport/pass435 dependency.", "", "## ReDMCSB source audit"]
    lines.extend("- {source} - {claim}".format(**row) for row in red)
    lines.extend(["", "## Firestaff coverage"])
    lines.extend("- {source} - {claim}".format(**row) for row in fire)
    lines.extend(["", "## Chained static gates"])
    lines.extend("- {id} rc={returncode}".format(**gate) for gate in gates)
    lines.extend(["", "Manifest: parity-evidence/verification/pass507_dm1_v1_movement_stairs_group_timing_source_lock/manifest.json", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")
    print("pass507_dm1_v1_movement_stairs_group_timing_source_lock=PASS")
    print(f"redmcsbCitations={len(red)} firestaffEvidence={len(fire)} chainedGates={len(gates)}")
    print(f"manifest={MANIFEST}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"FAIL pass507_dm1_v1_movement_stairs_group_timing_source_lock: {exc}", file=sys.stderr)
        raise SystemExit(1)
