#!/usr/bin/env python3
"""Pass566: source-lock DM1 V1 turn/cooldown gate boundary.

The concrete movement blocker here is command timing when movement is cooling
down: ReDMCSB gates only C003..C006 step commands in F0380. C001/C002 turns
must still dequeue and dispatch to F0365, while the front step command remains
queued until G0310/G0311 clear.
"""
from __future__ import annotations

import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS = "pass566_dm1_v1_turn_cooldown_gate_boundary"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"


def read(path: Path, encoding: str = "utf-8") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def require(text: str, needle: str, label: str) -> None:
    if compact(needle) not in compact(text):
        raise AssertionError(f"missing {label}: {needle!r}")


def require_absent(text: str, needle: str, label: str) -> None:
    if compact(needle) in compact(text):
        raise AssertionError(f"unexpected {label}: {needle!r}")


def require_order(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing ordered needle {needle!r}")
        pos = hit


def function_body(text: str, name: str) -> tuple[int, int, str]:
    m = re.search(rf"^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|unsigned\s+char|int)\s+{re.escape(name)}\s*\(", text, re.M)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text.count("\n", 0, m.start()) + 1, text.count("\n", 0, i) + 1, text[m.start():i + 1]
    next_m = re.search(r"^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|unsigned\s+char|int)\s+F\d+_[A-Za-z0-9_]+\s*\(", text[m.start() + 1:], re.M)
    if next_m:
        end = m.start() + 1 + next_m.start()
        return text.count("\n", 0, m.start()) + 1, text.count("\n", 0, end), text[m.start():end]
    raise AssertionError(f"unterminated function {name}")


def line(text: str, needle: str) -> int:
    i = text.find(needle)
    if i < 0:
        raise AssertionError(f"missing line needle {needle!r}")
    return text.count("\n", 0, i) + 1


def git_out(*args: str) -> str:
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True).strip()


def source_audit() -> dict:
    command = read(RED / "COMMAND.C", "latin-1")
    clik = read(RED / "CLIKMENU.C", "latin-1")
    gameloop = read(RED / "GAMELOOP.C", "latin-1")

    f0380_s, f0380_e, f0380 = function_body(command, "F0380_COMMAND_ProcessQueue_CPSC")
    f0365_s, f0365_e, f0365 = function_body(clik, "F0365_COMMAND_ProcessTypes1To2_TurnParty")

    require_order(gameloop, [
        "if (G0310_i_DisabledMovementTicks)",
        "G0310_i_DisabledMovementTicks--;",
        "if (G0311_i_ProjectileDisabledMovementTicks)",
        "G0311_i_ProjectileDisabledMovementTicks--;",
        "F0380_COMMAND_ProcessQueue_CPSC();",
    ], "GAMELOOP ages old movement cooldowns before queue processing")
    require_order(f0380, [
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT) && (G0310_i_DisabledMovementTicks",
        "goto T0380042;",
        "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
        "G2153_i_QueuedCommandsCount--;",
        "if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT))",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT))",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "F0380 gates only step commands before dequeue, then turns dispatch before moves")
    require_order(f0365, [
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "F0362_COMMAND_HighlightBoxEnable",
        "if (M034_SQUARE_TYPE",
        "F0364_COMMAND_TakeStairs",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval",
        "F0284_CHAMPION_SetPartyDirection",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval",
    ], "F0365 turn side effects")
    require_absent(f0365, "G0310_i_DisabledMovementTicks", "F0365 movement cooldown write")
    require_absent(f0365, "F0325_CHAMPION_DecrementStamina", "F0365 step stamina cost")
    return {
        "GAMELOOP.C:cooldown_decrement_before_F0380": [
            line(gameloop, "if (G0310_i_DisabledMovementTicks)"),
            line(gameloop, "F0380_COMMAND_ProcessQueue_CPSC();"),
        ],
        "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": [f0380_s, f0380_e],
        "COMMAND.C:F0380_step_only_gate": [
            line(command, "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;"),
            line(command, "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT) && (G0310_i_DisabledMovementTicks"),
            line(command, "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);"),
            line(command, "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"),
        ],
        "CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty": [f0365_s, f0365_e],
    }


def firestaff_audit() -> dict:
    queue_c = read(ROOT / "src/dm1/dm1_v1_input_command_queue_pc34_compat.c")
    core_c = read(ROOT / "src/dm1/dm1_v1_movement_command_core_pc34_compat.c")
    queue_test = read(ROOT / "tests/test_dm1_v1_input_command_queue_pc34_compat.c")
    core_test = read(ROOT / "tests/test_dm1_v1_movement_command_core_pc34_compat.c")
    cmake = read(ROOT / "CMakeLists.txt")

    require_order(queue_c, [
        "result.command = queue->commands[0].command;",
        "if (is_move_command(result.command) &&",
        "result.movementDisabledGate = 1;",
        "return result;",
        "queue->count--;",
        "if (result.command == DM1_V1_COMMAND_TURN_LEFT || result.command == DM1_V1_COMMAND_TURN_RIGHT)",
        "result.dispatchedTurn = 1;",
        "else if (is_move_command(result.command))",
        "result.dispatchedMove = 1;",
    ], "Firestaff input queue preserves ReDMCSB turn bypass versus step gate")
    require_order(core_c, [
        "if (dm1_v1_is_turn_command(outResult->queue.command))",
        "outResult->commandHandled = 1;",
        "m11_v1_turning_apply_party_original_presentation_pc34_compat",
        "outResult->turnApplied = 1;",
        "return 1;",
        "if (!dm1_v1_is_step_command(outResult->queue.command))",
        "dm1_v1_apply_pre_step_stamina_cost(party, outResult);",
    ], "Firestaff command core handles turns before step stamina/movement")
    for needle in [
        "turn bypasses movement disabled gate",
        "turn dequeued despite movement disabled",
        "turn dispatched despite movement disabled",
        "movement gate set",
        "movement not dequeued",
        "front still forward",
        "forward dequeued after gate clears",
    ]:
        require(queue_test, needle, f"queue focused probe {needle}")
    for needle in [
        "pc34 core left arrow turn bypasses movement gate",
        "pc34 core left arrow leaves no cooldown",
        "pc34 core left arrow dequeued",
        "pc34 core disabled gate keeps command queued",
        "pass542 cooldown gate did not dequeue held step",
        "pass542 replayed turn waits behind released step",
    ]:
        require(core_test, needle, f"command-core focused probe {needle}")
    require(cmake, "test_dm1_v1_input_command_queue_pc34_compat", "queue CTest target")
    require(cmake, "test_dm1_v1_movement_command_core_pc34_compat", "command-core CTest target")
    return {
        "queueCore": "DM1_V1_InputCommandQueue_ProcessOnePc34Compat gates only move commands, never C001/C002 turns",
        "commandCore": "DM1_V1_MovementCommandCore_ProcessOnePc34Compat returns from the turn branch before step stamina/collision/cooldown",
        "focusedTests": [
            "dm1_v1_input_command_queue_pc34_compat",
            "dm1_v1_movement_command_core_pc34_compat",
        ],
    }


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": "PASS566_DM1_V1_TURN_COOLDOWN_GATE_BOUNDARY_LOCKED",
        "branch": git_out("branch", "--show-current"),
        "redmcsbRoot": str(RED),
        "sourceAudit": source_audit(),
        "firestaffAudit": firestaff_audit(),
        "closedGap": "turn commands C001/C002 bypass G0310/G0311 movement-disabled gating and do not run step stamina/collision/cooldown, while C003..C006 step commands remain queued until the cooldown gate clears",
        "notClaimed": [
            "new original DOS runtime breakpoint transcript",
            "new pixel capture parity",
            "full creature AI reaction timing beyond command-gate boundary",
        ],
    }
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass566 - DM1 V1 turn/cooldown gate boundary",
        "",
        "Status: {}".format(manifest["status"]),
        "",
        "## ReDMCSB-first audit",
    ]
    for name, span in manifest["sourceAudit"].items():
        lines.append(f"- {name} - {span}")
    lines += [
        "",
        "## Firestaff audit",
        "- {}".format(manifest["firestaffAudit"]["queueCore"]),
        "- {}".format(manifest["firestaffAudit"]["commandCore"]),
        "",
        "## Closed gap",
        manifest["closedGap"],
        "",
        "## Not claimed",
    ]
    lines.extend(f"- {item}" for item in manifest["notClaimed"])
    lines.append("")
    OUT_MD.write_text("\n".join(lines), encoding="utf-8")
    print(manifest["status"])
    print(f"manifest={OUT_JSON.relative_to(ROOT)}")
    print(f"evidence={OUT_MD.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
