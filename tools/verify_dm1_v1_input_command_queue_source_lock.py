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
REDMCSB_SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
COMMAND_C = REDMCSB_SOURCE / "COMMAND.C"
CLIKMENU_C = REDMCSB_SOURCE / "CLIKMENU.C"
MENUDRAW_C = REDMCSB_SOURCE / "MENUDRAW.C"
IO2_C = REDMCSB_SOURCE / "IO2.C"
DEFS_H = REDMCSB_SOURCE / "DEFS.H"
COMPAT_C = ROOT / "src/dm1/dm1_v1_input_command_queue_pc34_compat.c"
TEST_C = ROOT / "tests/test_dm1_v1_input_command_queue_pc34_compat.c"


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
    ]); citations.append("COMMAND.C:6 circular command queue storage declaration")
    require("DEFS.H:3263-3264", block(DEFS_H, 3263, 3264), [
        "MEDIA728_I34E_I34M_A36M_A35E_A35M",
        "M529_COMMAND_QUEUE_SIZE",
        "8",
    ]); citations.append("DEFS.H:3263-3264 PC34 command queue storage size")
    require("DEFS.H:3507", block(DEFS_H, 3507, 3507), [
        "C5_UNKNOWN", "5",
    ]); citations.append("DEFS.H:3507 PC34 regular-command cap constant")
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
    ]); citations.append("COMMAND.C:272-305 legacy keyboard movement rows")
    require("COMMAND.C:636-685", block(COMMAND_C, 636, 685), [
        "G0459_as_Graphic561_SecondaryKeyboardInput_Movement", "MEDIA707_I34E_I34M",
        "C001_COMMAND_TURN_LEFT,     0x004B", "C003_COMMAND_MOVE_FORWARD,  0x004C",
        "C002_COMMAND_TURN_RIGHT,    0x004D", "C006_COMMAND_MOVE_LEFT,     0x004F",
        "C005_COMMAND_MOVE_BACKWARD, 0x0050", "C004_COMMAND_MOVE_RIGHT,    0x0051",
    ]); citations.append("COMMAND.C:636-685 PC34 keyboard movement rows")
    require("IO2.C:27-61", block(IO2_C, 27, 61), [
        "MEDIA463_P20JA_P20JB_I34E_I34M_P31J", "IODRV_00_GetKeyboardInput",
        "MEDIA707_I34E_I34M", "switch (L2944_ui_ - 0x1248)",
        "0x48 = Scancode of Up arrow", "L2944_ui_ = 'L'",
        "0x50 = Scancode of Down arrow", "L2944_ui_ = 'P'",
        "0x4B = Scancode of Left arrow", "L2944_ui_ = 'K'",
        "0x4D = Scancode of Right arrow", "L2944_ui_ = 'M'",
    ]); citations.append("IO2.C:27-61 PC34 raw keyboard read and shifted-arrow normalization")
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
    require("COMMAND.C:1490-1493", block(COMMAND_C, 1490, 1493), [
        "G0436_B_PendingClickPresent = C1_TRUE",
        "G0437_i_PendingClickX = P0725_i_X",
        "G0438_i_PendingClickY = P0726_i_Y",
        "G0439_i_PendingClickButtonsStatus = P0727_i_ButtonsStatus",
    ]); citations.append("COMMAND.C:1490-1493 locked click overwrites one pending-click tuple")
    require("COMMAND.C:1692-1707", block(COMMAND_C, 1692, 1707), [
        "F0360_COMMAND_ProcessPendingClick", "if (G0436_B_PendingClickPresent)",
        "G0436_B_PendingClickPresent = C0_FALSE", "F0359_COMMAND_ProcessClick_CPSC(G0437_i_PendingClickX",
    ]); citations.append("COMMAND.C:1692-1707 pending click replay")
    require("COMMAND.C:2854-2857", block(COMMAND_C, 2854, 2857), [
        "G0436_B_PendingClickPresent = C1_TRUE",
        "G0437_i_PendingClickX = P0725_i_X",
        "G0438_i_PendingClickY = P0726_i_Y",
        "G0439_i_PendingClickButtonsStatus = P0727_i_ButtonsStatus",
    ]); citations.append("COMMAND.C:2854-2857 alternate locked-click path overwrites one pending-click tuple")
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
        "COMMAND.C:6", "COMMAND.C:106-121", "COMMAND.C:636-685", "DEFS.H:3263-3264", "IO2.C:27-61",
        "COMMAND.C:1379-1449", "COMMAND.C:1452-1661", "COMMAND.C:1692-1707",
        "COMMAND.C:1709-1813", "COMMAND.C:2045-2156",
        "CLIKMENU.C:142-174", "CLIKMENU.C:180-330", "MENUDRAW.C:5-19",
    ]:
        if marker not in c:
            raise AssertionError(f"compat source evidence missing {marker}")
    require_regex("src/dm1/dm1_v1_input_command_queue_pc34_compat.c", c, r"#define\s+DM1_V1_QUEUE_MAX_REGULAR\s+5u")
    require_regex("src/dm1/dm1_v1_input_command_queue_pc34_compat.c", c, r"0x9BFF.*0x004B.*DM1_V1_COMMAND_TURN_LEFT")
    require_regex("src/dm1/dm1_v1_input_command_queue_pc34_compat.c", c, r"0x9B6F.*0x004D.*DM1_V1_COMMAND_TURN_RIGHT")
    require_regex("src/dm1/dm1_v1_input_command_queue_pc34_compat.c", c, r"0x9B60.*0x0051.*DM1_V1_COMMAND_MOVE_RIGHT")
    require_regex("src/dm1/dm1_v1_input_command_queue_pc34_compat.c", c, r"0x9B54.*0x004C.*DM1_V1_COMMAND_MOVE_FORWARD")
    require_regex("src/dm1/dm1_v1_input_command_queue_pc34_compat.c", c, r"0x9B53.*0x0050.*DM1_V1_COMMAND_MOVE_BACKWARD")
    require_regex("src/dm1/dm1_v1_input_command_queue_pc34_compat.c", c, r"0x9B61.*0x004F.*DM1_V1_COMMAND_MOVE_LEFT")
    require_regex("src/dm1/dm1_v1_input_command_queue_pc34_compat.c", c, r"queue->pendingClickPresent\s*=\s*1;.*queue->pendingClickButtons.*queue->pendingClickCommand")
    require_regex("src/dm1/dm1_v1_input_command_queue_pc34_compat.c", c, r"process_pending_click\(queue\).*result\.movementDisabledGate\s*=\s*1")
    require_regex("src/dm1/dm1_v1_input_command_queue_pc34_compat.c", c, r"result\.dispatchedTurn\s*=\s*1;.*result\.dispatchedMove\s*=\s*1;")
    for needle in [
        "locked mouse becomes pending", "movement gate set", "movement not dequeued",
        "pending replayed", "forward dequeued after gate clears", "turn dispatched",
        "second locked pending overwrites with turn right", "pending overwrite replays once",
        "pending overwrite replay is latest",
        "turn bypasses movement disabled gate", "turn dequeued despite movement disabled",
        "locked direct touch inventory pending", "direct touch command",
        "pc34 table left arrow turns left", "pc34 table up arrow moves forward",
        "pc34 io2 shifted up arrow normalizes to forward", "pc34 shifted del turns left",
        "pc34 shifted help turns right", "pc34 shifted backward arrow strafes right",
        "redmcsb fifth command accepted before C5 limit", "redmcsb sixth command is dropped at C5 limit",
    ]:
        if needle not in t:
            raise AssertionError(f"test missing invariant label {needle!r}")
    notes.append("Firestaff compat queue model and regression probe cover pending replay, actual PC-34 movement keys, IO2 shifted-arrow normalization, legacy shifted movement keys, and C5 PC34/I34 regular-command queue capacity")
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
