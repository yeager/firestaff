#!/usr/bin/env python3
"""Verify the DM1 V1 ReDMCSB F0380 queue-pop eligibility blocker.

This is a source-plus-artifact verifier.  It deliberately does not promote on
breakpoint command echo or route keylogs: it checks the audited ReDMCSB source
branches that gate F0380 semantic dispatch and the pass386 runtime predicates.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
PASS386_MANIFEST = REPO / "parity-evidence/verification/pass386_dm1_v1_keyboard_vs_click_command_dispatch/manifest.json"
OUT = REPO / "parity-evidence/verification/pass387_dm1_v1_f0380_queue_pop_eligibility/manifest.json"


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(errors="replace")


def function_body(text: str, name: str) -> tuple[int, int, str]:
    m = re.search(rf"^void\s+{re.escape(name)}\s*\(", text, re.M)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing function body for {name}")
    depth = 0
    for i in range(brace, len(text)):
        ch = text[i]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                start_line = text.count("\n", 0, m.start()) + 1
                end_line = text.count("\n", 0, i) + 1
                return start_line, end_line, text[m.start() : i + 1]
    next_m = re.search(r"^void\s+F\d+_", text[m.start() + 1 :], re.M)
    if next_m:
        end = m.start() + 1 + next_m.start()
        start_line = text.count("\n", 0, m.start()) + 1
        end_line = text.count("\n", 0, end) + 1
        return start_line, end_line, text[m.start() : end]
    raise AssertionError(f"unterminated function body for {name}")


def require(body: str, needle: str, why: str) -> None:
    if needle not in body:
        raise AssertionError(f"missing {why}: {needle}")


def main() -> None:
    command_c = read(SOURCE_ROOT / "COMMAND.C")
    clikmenu_c = read(SOURCE_ROOT / "CLIKMENU.C")
    gameloop_c = read(SOURCE_ROOT / "GAMELOOP.C")
    pc_h = read(SOURCE_ROOT / "PC.H")

    f0380_start, f0380_end, f0380 = function_body(command_c, "F0380_COMMAND_ProcessQueue_CPSC")
    f0365_start, f0365_end, f0365 = function_body(clikmenu_c, "F0365_COMMAND_ProcessTypes1To2_TurnParty")
    f0366_start, f0366_end, f0366 = function_body(clikmenu_c, "F0366_COMMAND_ProcessTypes3To6_MoveParty")

    require(pc_h, "#define G2153_i_QueuedCommandsCount", "PC queue-count alias")
    require(f0380, "G0435_B_CommandQueueLocked = C1_TRUE;", "F0380 queue lock before eligibility")
    require(f0380, "if (G2153_i_QueuedCommandsCount == 0)", "F0380 PC34 empty-queue predicate")
    require(f0380, "goto T0380xxx;", "F0380 empty queue/movement-disabled bypass label")
    require(f0380, "G0435_B_CommandQueueLocked = C0_FALSE;\n                F0360_COMMAND_ProcessPendingClick();\n                goto T0380042;", "F0380 bypass before pop/dispatch")
    require(f0380, "G2153_i_QueuedCommandsCount--;", "F0380 queue pop decrement")
    require(f0380, "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;", "F0380 command load")
    require(f0380, "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0380 turn dispatch")
    require(f0380, "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);", "F0380 move dispatch")
    require(f0380, "G0310_i_DisabledMovementTicks", "F0380 movement-disabled predicate")
    require(f0365, "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "turn handler stop-wait write")
    require(f0366, "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "move handler stop-wait write")
    require(gameloop_c, "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());", "main loop keyboard-to-command processor")
    require(gameloop_c, "F0380_COMMAND_ProcessQueue_CPSC();", "main loop F0380 call")
    require(gameloop_c, "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);", "main loop wait predicate")

    pass386 = json.loads(read(PASS386_MANIFEST))
    predicates = pass386["proofPredicates"]
    expected = {
        "keyboardRouteRanAfterArm": True,
        "keyboardF0361Hit": True,
        "keyboardQueueCountChanged": False,
        "keyboardDispatchReached": False,
        "clickRouteRanAfterArm": True,
        "clickF0380Hit": True,
        "clickDispatchReached": False,
        "clickStopWaitWriteObserved": True,
        "clickF0128Observed": True,
    }
    for key, value in expected.items():
        if predicates.get(key) is not value:
            raise AssertionError(f"pass386 predicate mismatch for {key}: {predicates.get(key)!r} != {value!r}")

    result = {
        "status": "BLOCKED_PASS387_F0380_QUEUE_POP_ELIGIBILITY_PREDICATE_PROVEN",
        "sourceRoot": str(SOURCE_ROOT),
        "auditedFunctions": {
            "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": {"lines": [f0380_start, f0380_end]},
            "CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty": {"lines": [f0365_start, f0365_end]},
            "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": {"lines": [f0366_start, f0366_end]},
            "GAMELOOP.C:F0002_MAIN_GameLoop_CPSDF": "contains F0361 keyboard drain, F0380 call, and G0321/G0301 wait loop",
            "PC.H:G2153_i_QueuedCommandsCount": "PC symbol alias present",
        },
        "exactBlockingPredicate": "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC checks G2153_i_QueuedCommandsCount == 0 before loading/popping G0432_as_CommandQueue; that branch unlocks, processes pending click, and jumps to T0380042, bypassing F0365/F0366 dispatch.",
        "secondaryEligibilityPredicate": "Movement commands C003..C006 also bypass dispatch when G0310_i_DisabledMovementTicks is non-zero or projectile-disabled movement matches the attempted direction.",
        "pass386Predicates": predicates,
        "conclusion": "Given pass386 observed click entering F0380 but no F0365/F0366 and keyboard entering F0361 with no G2153 write, the remaining blocker is an empty/not-pop-eligible command queue at F0380, not bad F0365/F0366 anchors.",
        "notPromotedBy": pass386.get("notPromotedBy", []),
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps(result, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
