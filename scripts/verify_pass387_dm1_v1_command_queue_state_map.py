#!/usr/bin/env python3
"""Verify pass387 DM1 V1 command queue evidence anchors against ReDMCSB."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

DEFAULT_SOURCE = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
).expanduser()

CHECKS: list[dict[str, Any]] = [
    {
        "id": "defs-command-payload-and-buttons",
        "file": "DEFS.H",
        "range": "213-244",
        "needles": [
            "#define MASK0x0001_MOUSE_RIGHT_BUTTON",
            "#define MASK0x0002_MOUSE_LEFT_BUTTON",
            "#define MASK0x0004_MOUSE_LEFT_BUTTON_UP",
            "#define MASK0x0008_MOUSE_RIGHT_BUTTON_UP",
            "#define MASK0x0010_MOUSE_BONUS_DUNGEON",
            "typedef struct {",
            "int16_t X;",
            "int16_t Y;",
            "int16_t Command;",
            "#define CM1_COMMAND_NONE",
            "#define C000_COMMAND_NONE",
            "#define C001_COMMAND_TURN_LEFT",
            "#define C002_COMMAND_TURN_RIGHT",
            "#define C003_COMMAND_MOVE_FORWARD",
            "#define C004_COMMAND_MOVE_RIGHT",
            "#define C005_COMMAND_MOVE_BACKWARD",
            "#define C006_COMMAND_MOVE_LEFT",
        ],
    },
    {
        "id": "defs-i34e-queue-size",
        "file": "DEFS.H",
        "range": "3260-3265",
        "needles": [
            "#define M529_COMMAND_QUEUE_SIZE                                          4",
            "MEDIA728_I34E_I34M_A36M_A35E_A35M",
            "#define M529_COMMAND_QUEUE_SIZE                                          8",
        ],
    },
    {
        "id": "command-ring-state",
        "file": "COMMAND.C",
        "range": "1-16",
        "needles": [
            "COMMAND G0432_as_CommandQueue[M529_COMMAND_QUEUE_SIZE + 1];",
            "int16_t G2153_i_QueuedCommandsCount;",
            "int16_t G0433_i_CommandQueueFirstIndex;",
            "int16_t G0434_i_CommandQueueLastIndex = M529_COMMAND_QUEUE_SIZE;",
            "BOOLEAN G0435_B_CommandQueueLocked = C1_TRUE;",
            "BOOLEAN G0436_B_PendingClickPresent;",
            "int16_t G0437_i_PendingClickX;",
            "int16_t G0438_i_PendingClickY;",
            "int16_t G0439_i_PendingClickButtonsStatus;",
        ],
    },
    {
        "id": "startup-active-input-producers",
        "file": "STARTUP2.C",
        "range": "1179-1182",
        "needles": [
            "G0441_ps_PrimaryMouseInput = G0447_as_Graphic561_PrimaryMouseInput_Interface;",
            "G0442_ps_SecondaryMouseInput = G0448_as_Graphic561_SecondaryMouseInput_Movement;",
            "G0443_ps_PrimaryKeyboardInput = G0458_as_Graphic561_PrimaryKeyboardInput_Interface;",
            "G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;",
        ],
    },
    {
        "id": "mouse-and-keyboard-movement-producers",
        "file": "COMMAND.C",
        "range": "375-644",
        "needles": [
            "G0447_as_Graphic561_PrimaryMouseInput_Interface",
            "G0448_as_Graphic561_SecondaryMouseInput_Movement",
            "C001_COMMAND_TURN_LEFT",
            "C002_COMMAND_TURN_RIGHT",
            "C003_COMMAND_MOVE_FORWARD",
            "C004_COMMAND_MOVE_RIGHT",
            "C005_COMMAND_MOVE_BACKWARD",
            "C006_COMMAND_MOVE_LEFT",
            "C080_COMMAND_CLICK_IN_DUNGEON_VIEW",
            "C083_COMMAND_TOGGLE_INVENTORY_LEADER",
            "G0459_as_Graphic561_SecondaryKeyboardInput_Movement",
        ],
    },
    {
        "id": "mouse-table-resolution-order",
        "file": "COMMAND.C",
        "range": "1379-1449",
        "needles": [
            "int16_t F0358_COMMAND_GetCommandFromMouseInput_CPSC(",
            "return C000_COMMAND_NONE;",
            "while (L1107_Command = P0721_ps_MouseInput->Command)",
            "P0724_i_ButtonsStatus & P0721_ps_MouseInput->Button",
            "P0721_ps_MouseInput++;",
            "return L1107_Command;",
        ],
    },
    {
        "id": "click-enqueue-primary-secondary-and-count",
        "file": "COMMAND.C",
        "range": "1631-1662",
        "needles": [
            "L1109_i_Command = C129_COMMAND_RELEASE_CHAMPION_ICON;",
            "L1109_i_Command = C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL;",
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput",
            "if (L1109_i_Command == C000_COMMAND_NONE)",
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput",
            "G2153_i_QueuedCommandsCount++;",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command;",
            "G0432_as_CommandQueue[L1108_i_CommandQueueIndex].X = P0725_i_X;",
            "G0432_as_CommandQueue[L1108_i_CommandQueueIndex].Y = P0726_i_Y;",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
        ],
    },
    {
        "id": "pending-click-replay",
        "file": "COMMAND.C",
        "range": "1692-1707",
        "needles": [
            "F0360_COMMAND_ProcessPendingClick",
            "if (G0436_B_PendingClickPresent)",
            "G0436_B_PendingClickPresent = C0_FALSE;",
            "F0359_COMMAND_ProcessClick_CPSC(G0437_i_PendingClickX, G0438_i_PendingClickY, G0439_i_PendingClickButtonsStatus);",
        ],
    },
    {
        "id": "f0380-dequeue-eligibility-dispatch",
        "file": "COMMAND.C",
        "range": "2045-2156",
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC(",
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "AL1159_i_CommandQueueIndex = G0434_i_CommandQueueLastIndex + 1",
            "if (G2153_i_QueuedCommandsCount == 0)",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G0310_i_DisabledMovementTicks",
            "G0311_i_ProjectileDisabledMovementTicks",
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "L1162_i_CommandY = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Y;",
            "G2153_i_QueuedCommandsCount--;",
            "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)",
            "F0360_COMMAND_ProcessPendingClick();",
            "if (L1160_i_Command == C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL)",
            "if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT))",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT))",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
    },
    {
        "id": "f0365-f0366-stopwaiting",
        "file": "CLIKMENU.C",
        "range": "142-323",
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty(",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0362_COMMAND_HighlightBoxEnable((P0734_i_Command == C001_COMMAND_TURN_LEFT) ? C068_ZONE_TURN_LEFT : C069_ZONE_TURN_RIGHT);",
            "F0284_CHAMPION_SetPartyDirection",
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty(",
            "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;",
            "F0362_COMMAND_HighlightBoxEnable(C070_ZONE_MOVE_FORWARD + AL1118_ui_MovementArrowIndex);",
            "if (L1117_B_MovementBlocked)",
            "F0357_COMMAND_DiscardAllInput();",
            "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        ],
    },
    {
        "id": "gameloop-wait-loop-and-f0380",
        "file": "GAMELOOP.C",
        "range": "164-219",
        "needles": [
            "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
            "while (M527_IsCharacterInKeyboardBuffer())",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "if (!G0321_B_StopWaitingForPlayerInput)",
            "F0363_COMMAND_HighlightBoxDisable();",
            "} while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        ],
    },
    {
        "id": "viewport-redraw-paths",
        "file": "DUNVIEW.C",
        "range": "8604-8611",
        "needles": [
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
    },
    {
        "id": "rest-freeze-explicit-viewport-draws",
        "file": "COMMAND.C",
        "range": "2340-2415",
        "needles": [
            "F0097_DUNGEONVIEW_DrawViewport(C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME);",
            "G0441_ps_PrimaryMouseInput = G0450_as_Graphic561_PrimaryMouseInput_PartyResting;",
            "F0357_COMMAND_DiscardAllInput();",
            "G0301_B_GameTimeTicking = C0_FALSE;",
            "G0441_ps_PrimaryMouseInput = G0451_as_Graphic561_PrimaryMouseInput_FrozenGame;",
        ],
    },
]


def slice_text(path: Path, line_range: str) -> str:
    start_s, end_s = line_range.split("-", 1)
    start = int(start_s)
    end = int(end_s)
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(lines[start - 1 : end])


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--json", type=Path, default=None)
    args = parser.parse_args()

    failures: list[dict[str, Any]] = []
    passed: list[str] = []
    for check in CHECKS:
        path = args.source / check["file"]
        if not path.exists():
            failures.append({"id": check["id"], "missing_file": str(path)})
            continue
        haystack = slice_text(path, check["range"])
        missing = [needle for needle in check["needles"] if needle not in haystack]
        if missing:
            failures.append({"id": check["id"], "file": check["file"], "range": check["range"], "missing": missing})
        else:
            passed.append(check["id"])

    result = {"source": str(args.source), "passed": passed, "failures": failures}
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    if failures:
        print(json.dumps(result, indent=2, sort_keys=True))
        return 1
    print(f"verified {len(passed)} pass387 command queue source anchors in {args.source}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
