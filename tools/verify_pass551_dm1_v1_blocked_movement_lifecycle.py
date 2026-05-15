#!/usr/bin/env python3
"""Pass551 DM1 V1 blocked movement lifecycle source-lock gate."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="latin-1")
    except FileNotFoundError:
        print(f"missing: {path}", file=sys.stderr)
        sys.exit(1)


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        print(f"FAIL {label}: missing {needle!r}", file=sys.stderr)
        sys.exit(1)
    print(f"PASS {label}")


def main() -> int:
    command = read(SRC / "COMMAND.C")
    clickmenu = read(SRC / "CLIKMENU.C")
    movesens = read(SRC / "MOVESENS.C")
    queue = read(ROOT / "dm1_v1_input_command_queue_pc34_compat.c")
    core = read(ROOT / "dm1_v1_movement_command_core_pc34_compat.c")
    test = read(ROOT / "test_dm1_v1_command_movement_sensor_timing_pc34_compat.c")

    require(command, "G0435_B_CommandQueueLocked = C1_TRUE;", "F0380/F0357 lock command queue")
    require(command, "if (G2153_i_QueuedCommandsCount == 0)", "F0380 empty queue check before dequeue")
    require(command, "G2153_i_QueuedCommandsCount--;", "F0380 dequeue decrements queued count")
    require(command, "F0360_COMMAND_ProcessPendingClick();", "F0380/F0357 pending-click replay hook")
    require(command, "C129_COMMAND_RELEASE_CHAMPION_ICON) || (G0432_as_CommandQueue[L2284_i_SourceCommandQueueIndex].Command == C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL)", "F0357 preserves release/stop commands only")

    require(clickmenu, "F0325_CHAMPION_DecrementStamina", "F0366 applies living champion stamina before block resolution")
    require(clickmenu, "F0357_COMMAND_DiscardAllInput();", "F0366 blocked movement discards queued input")
    require(clickmenu, "F0693_WaitVerticalBlank();", "F0366 PC34 blocked movement waits one VBlank")
    require(clickmenu, "G0321_B_StopWaitingForPlayerInput = C0_FALSE;", "F0366 blocked movement keeps input wait armed")
    require(clickmenu, "F0209_GROUP_ProcessEvents29to41", "F0366 group block requests adjacent reaction")
    require(clickmenu, "F0267_MOVE_GetMoveResult_CPSCE", "F0366 successful path enters move/sensor core after blockers")

    require(movesens, "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX", "F0267 source leave sensor path")
    require(movesens, "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX", "F0267 destination enter sensor path")

    require(queue, "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat", "Firestaff queue exposes discard lifecycle")
    require(queue, "DM1_V1_COMMAND_RELEASE_CHAMPION_ICON", "Firestaff discard preserves release command")
    require(queue, "DM1_V1_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL", "Firestaff discard preserves stop-pressing command")

    require(core, "blockedMovementVblankWaitRequested = 1;", "Firestaff blocked movement records VBlank wait")
    require(core, "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);", "Firestaff blocked movement discards input")
    require(core, "groupReactionPartyAdjacentRequested = 1;", "Firestaff group block records adjacent reaction")
    require(core, "dm1_v1_record_blocked_wall_or_door_damage_request", "Firestaff wall/door block records damage request")

    require(test, "core blocked wall keeps input wait armed", "wall lifecycle asserts stop-wait remains armed")
    require(test, "core blocked door preserves release/stop only", "door lifecycle asserts reserved queue compaction")
    require(test, "core group block requests blocked vblank", "group lifecycle asserts blocked VBlank")
    require(test, "core group block does not request wall damage", "group lifecycle separates reaction from wall damage")

    print("pass551_dm1_v1_blocked_movement_lifecycle=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
