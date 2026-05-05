#!/usr/bin/env python3
"""Verify the DM1 V1 input->command-queue source lock against ReDMCSB.

This is intentionally narrow and non-visual: it locks the mouse/keyboard input
rows, pending click replay, queue enqueue/dequeue, movement-disabled gate, and
turn/move dispatch boundaries used by the Firestaff compat queue probe.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
COMMAND_C = REDMCSB_SOURCE / "COMMAND.C"
CLIKMENU_C = REDMCSB_SOURCE / "CLIKMENU.C"
MENUDRAW_C = REDMCSB_SOURCE / "MENUDRAW.C"
COMPAT_C = ROOT / "dm1_v1_input_command_queue_pc34_compat.c"
TEST_C = ROOT / "test_dm1_v1_input_command_queue_pc34_compat.c"


def lines(path: Path) -> list[str]:
    try:
        return path.read_text(encoding="latin-1").splitlines()
    except FileNotFoundError as exc:
        raise AssertionError(f"missing required file: {path}") from exc


def block(path: Path, start: int, end: int) -> str:
    ls = lines(path)
    return "\n".join(ls[start - 1:end])


def compact(text: str) -> str:
    return " ".join(text.split())


def require(cite: str, text: str, needles: list[str]) -> None:
    hay = compact(text)
    for needle in needles:
        if compact(needle) not in hay:
            raise AssertionError(f"{cite} missing expected text: {needle}")


def require_regex(cite: str, text: str, pattern: str) -> None:
    if re.search(pattern, text, re.MULTILINE | re.DOTALL) is None:
        raise AssertionError(f"{cite} missing pattern: {pattern}")


def verify_redmcsb() -> list[str]:
    citations: list[str] = []
    require("COMMAND.C:6", block(COMMAND_C, 6, 6), [
        "G0432_as_CommandQueue[M529_COMMAND_QUEUE_SIZE + 1]",
        "Can only contain up to 4 actual commands at a time",
    ]); citations.append("COMMAND.C:6 circular command queue capacity")
    require("COMMAND.C:106-121", block(COMMAND_C, 106, 121), [
        "C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT",
        "C006_COMMAND_MOVE_LEFT", "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT",
        "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "C083_COMMAND_TOGGLE_INVENTORY_LEADER",
    ]); citations.append("COMMAND.C:106-121 mouse movement/viewport rows")
    require("COMMAND.C:272-305", block(COMMAND_C, 272, 305), [
        "G0459_as_Graphic561_SecondaryKeyboardInput_Movement", "C001_COMMAND_TURN_LEFT",
        "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C006_COMMAND_MOVE_LEFT",
        "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT", "0xAB34", "0x9B41",
        "0x9BFF", "0x9B6F", "0x9B60",
    ]); citations.append("COMMAND.C:272-305 keyboard movement rows")
    require("COMMAND.C:1379-1449", block(COMMAND_C, 1379, 1449), [
        "F0358_COMMAND_GetCommandFromMouseInput_CPSC", "while (L1107_Command = P0721_ps_MouseInput->Command)",
        "P0724_i_ButtonsStatus & P0721_ps_MouseInput->Button", "F0798_COMMAND_IsPointInZone", "return L1107_Command",
    ]); citations.append("COMMAND.C:1379-1449 mouse hit matcher")
    require("COMMAND.C:1452-1661", block(COMMAND_C, 1452, 1661), [
        "F0359_COMMAND_ProcessClick_CPSC", "G0436_B_PendingClickPresent = C1_TRUE",
        "G0437_i_PendingClickX = P0725_i_X", "G0439_i_PendingClickButtonsStatus = P0727_i_ButtonsStatus",
        "if (L1108_i_CommandQueueIndex == G0433_i_CommandQueueFirstIndex)",
        "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput",
        "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput",
        "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command",
    ]); citations.append("COMMAND.C:1452-1661 mouse enqueue/pending")
    require("COMMAND.C:1692-1707", block(COMMAND_C, 1692, 1707), [
        "F0360_COMMAND_ProcessPendingClick", "if (G0436_B_PendingClickPresent)",
        "G0436_B_PendingClickPresent = C0_FALSE", "F0359_COMMAND_ProcessClick_CPSC(G0437_i_PendingClickX",
    ]); citations.append("COMMAND.C:1692-1707 pending click replay")
    require("COMMAND.C:1709-1813", block(COMMAND_C, 1709, 1813), [
        "F0361_COMMAND_ProcessKeyPress", "G0443_ps_PrimaryKeyboardInput", "G0444_ps_SecondaryKeyboardInput",
        "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command",
        "F0360_COMMAND_ProcessPendingClick",
    ]); citations.append("COMMAND.C:1709-1813 keyboard enqueue")
    require("COMMAND.C:2045-2156", block(COMMAND_C, 2045, 2156), [
        "F0380_COMMAND_ProcessQueue_CPSC", "G0435_B_CommandQueueLocked = C1_TRUE",
        "G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command",
        "G0310_i_DisabledMovementTicks", "G0311_i_ProjectileDisabledMovementTicks",
        "G0433_i_CommandQueueFirstIndex", "F0360_COMMAND_ProcessPendingClick",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty",
    ]); citations.append("COMMAND.C:2045-2156 queue dequeue/gate/dispatch")
    require("CLIKMENU.C:142-174", block(CLIKMENU_C, 142, 174), [
        "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection",
    ]); citations.append("CLIKMENU.C:142-174 turn handler")
    require("CLIKMENU.C:180-330", block(CLIKMENU_C, 180, 330), [
        "F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0465_ai_Graphic561_MovementArrowToStepForwardCount",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0357_COMMAND_DiscardAllInput",
    ]); citations.append("CLIKMENU.C:180-330 move handler")
    require("MENUDRAW.C:5-19", block(MENUDRAW_C, 5, 19), [
        "F0395_MENUS_DrawMovementArrows", "C013_GRAPHIC_MOVEMENT_ARROWS",
    ]); citations.append("MENUDRAW.C:5-19 arrow draw baseline")
    return citations


def verify_firestaff() -> list[str]:
    c = COMPAT_C.read_text()
    t = TEST_C.read_text()
    notes: list[str] = []
    for marker in [
        "COMMAND.C:6", "COMMAND.C:106-121", "COMMAND.C:1379-1449", "COMMAND.C:1452-1661",
        "COMMAND.C:1692-1707", "COMMAND.C:1709-1813", "COMMAND.C:2045-2156",
        "CLIKMENU.C:142-174", "CLIKMENU.C:180-330", "MENUDRAW.C:5-19",
    ]:
        if marker not in c:
            raise AssertionError(f"compat source evidence missing {marker}")
    require_regex("dm1_v1_input_command_queue_pc34_compat.c", c, r"#define\s+DM1_V1_QUEUE_MAX_REGULAR\s+4u")
    require_regex("dm1_v1_input_command_queue_pc34_compat.c", c, r"0x9BFF.*DM1_V1_COMMAND_TURN_LEFT")
    require_regex("dm1_v1_input_command_queue_pc34_compat.c", c, r"0x9B6F.*DM1_V1_COMMAND_TURN_RIGHT")
    require_regex("dm1_v1_input_command_queue_pc34_compat.c", c, r"0x9B60.*DM1_V1_COMMAND_MOVE_RIGHT")
    require_regex("dm1_v1_input_command_queue_pc34_compat.c", c, r"queue->pendingClickPresent\s*=\s*1;.*queue->pendingClickButtons.*queue->pendingClickCommand")
    require_regex("dm1_v1_input_command_queue_pc34_compat.c", c, r"process_pending_click\(queue\).*result\.movementDisabledGate\s*=\s*1")
    require_regex("dm1_v1_input_command_queue_pc34_compat.c", c, r"result\.dispatchedTurn\s*=\s*1;.*result\.dispatchedMove\s*=\s*1;")
    for needle in [
        "locked mouse becomes pending", "movement gate set", "movement not dequeued",
        "pending replayed", "forward dequeued after gate clears", "turn dispatched",
        "turn bypasses movement disabled gate", "turn dequeued despite movement disabled",
        "locked direct touch inventory pending", "direct touch command",
        "pc34 shifted del turns left", "pc34 shifted help turns right",
        "pc34 shifted backward arrow strafes right", "redmcsb fifth command is dropped",
    ]:
        if needle not in t:
            raise AssertionError(f"test missing invariant label {needle!r}")
    notes.append("Firestaff compat queue model and regression probe cover pending replay, PC-34 shifted movement keys, and four-command queue capacity")
    return notes


def main() -> None:
    print("probe=dm1_v1_input_command_queue_source_lock")
    print(f"sourceRoot={REDMCSB_SOURCE}")
    for item in verify_redmcsb() + verify_firestaff():
        print(f"- {item}")
    print("dm1V1InputCommandQueueSourceLockOk=1")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        sys.exit(1)
