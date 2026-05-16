#!/usr/bin/env python3
"""Verify the DM1 V1 queued turn/step timing gate against ReDMCSB."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REDMCSB_SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
COMMAND_C = REDMCSB_SOURCE / "COMMAND.C"
CLIKMENU_C = REDMCSB_SOURCE / "CLIKMENU.C"
GAMELOOP_C = REDMCSB_SOURCE / "GAMELOOP.C"
IO_C = REDMCSB_SOURCE / "IO.C"
TIMING_C = ROOT / "src/dm1/dm1_v1_movement_timing_pc34_compat.c"
TIMING_H = ROOT / "include/dm1_v1_movement_timing_pc34_compat.h"
TEST_C = ROOT / "tests/test_dm1_v1_turn_step_timing_gate_pc34_compat.c"


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
    require("GAMELOOP.C:47-50", block(GAMELOOP_C, 47, 50), [
        "G0318_i_WaitForInputMaximumVerticalBlankCount = 10",
        "G0318_i_WaitForInputMaximumVerticalBlankCount = 12",
    ])
    citations.append("GAMELOOP.C:47-50 PC-34 input wait VBlank window is 12")

    require("GAMELOOP.C:150-155", block(GAMELOOP_C, 150, 155), [
        "G0310_i_DisabledMovementTicks", "G0310_i_DisabledMovementTicks--",
        "G0311_i_ProjectileDisabledMovementTicks", "G0311_i_ProjectileDisabledMovementTicks--",
    ])
    citations.append("GAMELOOP.C:150-155 movement cooldowns decrement once per game loop")

    require("GAMELOOP.C:164-219", block(GAMELOOP_C, 164, 219), [
        "while (M527_IsCharacterInKeyboardBuffer())",
        "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer())",
        "F0380_COMMAND_ProcessQueue_CPSC()",
        "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking)",
    ])
    citations.append("GAMELOOP.C:164-219 keyboard queue drains before queue processing inside wait loop")

    require("COMMAND.C:1734-1812", block(COMMAND_C, 1734, 1812), [
        "G0435_B_CommandQueueLocked = C1_TRUE",
        "G2153_i_QueuedCommandsCount < C5_UNKNOWN",
        "G0443_ps_PrimaryKeyboardInput",
        "G0444_ps_SecondaryKeyboardInput",
        "G0432_as_CommandQueue",
        "F0360_COMMAND_ProcessPendingClick",
    ])
    citations.append("COMMAND.C:1734-1812 keyboard checks primary then secondary and enqueues commands under the C5 cap")

    require("COMMAND.C:2095-2127", block(COMMAND_C, 2095, 2127), [
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command",
        "G0310_i_DisabledMovementTicks",
        "G0311_i_ProjectileDisabledMovementTicks",
        "F0360_COMMAND_ProcessPendingClick",
    ])
    citations.append("COMMAND.C:2095-2127 movement commands stay queued while cooldown gates are active")

    require("COMMAND.C:2150-2156", block(COMMAND_C, 2150, 2156), [
        "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty",
    ])
    citations.append("COMMAND.C:2150-2156 turns dispatch separately from step commands")

    require("CLIKMENU.C:156-173", block(CLIKMENU_C, 156, 173), [
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE",
        "F0284_CHAMPION_SetPartyDirection",
    ])
    citations.append("CLIKMENU.C:156-173 turn dispatch stops input waiting and changes direction")

    require("CLIKMENU.C:237-269", block(CLIKMENU_C, 237, 269), [
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE",
        "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
    ])
    citations.append("CLIKMENU.C:237-269 step dispatch stops input waiting and computes relative movement")

    require("CLIKMENU.C:330-346", block(CLIKMENU_C, 330, 346), [
        "AL1115_ui_Ticks = 1",
        "F0310_CHAMPION_GetMovementTicks",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks",
        "G0311_i_ProjectileDisabledMovementTicks = 0",
    ])
    citations.append("CLIKMENU.C:330-346 successful step installs movement cooldown ticks")

    require("IO.C:772-778", block(IO_C, 772, 778), [
        "G0317_i_WaitForInputVerticalBlankCount += G2163_",
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE",
    ])
    require("IO.C:1015-1019", block(IO_C, 1015, 1019), [
        "G0317_i_WaitForInputVerticalBlankCount",
        "G0318_i_WaitForInputMaximumVerticalBlankCount",
        "G0321_B_StopWaitingForPlayerInput",
    ])
    citations.append("IO.C:772-778 and 1015-1019 VBlank wait only advances input wait counter")
    return citations


def verify_firestaff() -> list[str]:
    c = TIMING_C.read_text()
    h = TIMING_H.read_text()
    t = TEST_C.read_text()
    for marker in [
        "GAMELOOP.C:47-50", "GAMELOOP.C:150-155", "GAMELOOP.C:164-219",
        "COMMAND.C:2095-2100", "CLIKMENU.C:142-174", "IO.C:772-778",
    ]:
        if marker not in c:
            raise AssertionError(f"timing source evidence missing {marker}")
    require_regex("include/dm1_v1_movement_timing_pc34_compat.h", h,
                  r"#define\s+DM1_V1_INPUT_WAIT_MAX_VBLANKS_PC34_COMPAT\s+12")
    require_regex("src/dm1/dm1_v1_movement_timing_pc34_compat.c", c,
                  r"InputWaitStopsAfterVBlanksPc34Compat.*>=\s*DM1_V1_INPUT_WAIT_MAX_VBLANKS_PC34_COMPAT")
    require_regex("src/dm1/dm1_v1_movement_timing_pc34_compat.c", c,
                  r"VBlankWaitDecrementsMovementCooldownPc34Compat\(void\).*return\s+0;")
    for needle in [
        "pc34 input wait window is 12 vblanks",
        "queued keyboard turn bypasses movement-disabled gate",
        "queued keyboard forward hits movement-disabled gate",
        "vblank wait left movement cooldown unchanged",
        "game loop tick decrements movement cooldown once",
        "cleared cooldown releases queued forward",
    ]:
        if needle not in t:
            raise AssertionError(f"test missing invariant label {needle!r}")
    return ["Firestaff gate pins PC-34 queued keyboard turn/step behavior and separates VBlank input wait from game-loop cooldown decrement"]


def main() -> None:
    print("probe=dm1_v1_turn_step_timing_gate_source_lock")
    print(f"sourceRoot={REDMCSB_SOURCE}")
    for item in verify_redmcsb() + verify_firestaff():
        print(f"- {item}")
    print("dm1V1TurnStepTimingGateSourceLockOk=1")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        sys.exit(1)
