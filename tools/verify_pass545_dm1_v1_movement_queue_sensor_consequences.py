#!/usr/bin/env python3
"""Pass545: DM1 V1 command queue lifecycle plus movement sensor consequences."""
from __future__ import annotations

import json
import re
import subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass545_dm1_v1_movement_queue_sensor_consequences"
STATUS = "PASS545_DM1_V1_MOVEMENT_QUEUE_SENSOR_CONSEQUENCES_LOCKED"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
COMMAND = RED / "COMMAND.C"
CLIKMENU = RED / "CLIKMENU.C"
MOVESENS = RED / "MOVESENS.C"
QUEUE_C = ROOT / "dm1_v1_input_command_queue_pc34_compat.c"
CORE_C = ROOT / "dm1_v1_movement_command_core_pc34_compat.c"
CORE_TEST = ROOT / "test_dm1_v1_movement_command_core_pc34_compat.c"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"


def read(path: Path, encoding: str = "utf-8") -> str:
    if encoding == "latin-1":
        return path.read_text(encoding=encoding)
    return path.read_text(encoding=encoding)


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str, start: int = 0) -> int:
    pos = text.find(needle, start)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_order(text: str, needles: list[str], label: str) -> list[int]:
    positions: list[int] = []
    last = -1
    for needle in needles:
        pos = require(text, needle, label, last + 1)
        positions.append(pos)
        last = pos
    return positions


def function_range(text: str, name: str, rettype: str = r"(?:STATICFUNCTION\s+)?(?:void|int|int16_t|unsigned\s+int16_t|BOOLEAN|const\s+char\*|struct\s+\w+)") -> tuple[int, int, str]:
    pattern = re.compile(r"\b(?:" + rettype + r")\s+" + re.escape(name) + r"\s*\(")
    for match in pattern.finditer(text):
        brace = text.find("{", match.end())
        semicolon = text.find(";", match.end(), brace if brace >= 0 else len(text))
        if brace < 0 or semicolon >= 0:
            continue
        depth = 0
        for pos in range(brace, len(text)):
            if text[pos] == "{":
                depth += 1
            elif text[pos] == "}":
                depth -= 1
                if depth == 0:
                    return line_no(text, match.start()), line_no(text, pos), text[match.start():pos + 1]
    marker = name + "("
    marker_pos = text.find(marker)
    if marker_pos >= 0:
        start = text.rfind("\n", 0, marker_pos) + 1
        next_match = re.search(r"\n(?:STATICFUNCTION\s+)?(?:void|int16_t|unsigned\s+int16_t|BOOLEAN|int)\s+F\d+_", text[marker_pos + len(marker):])
        if next_match:
            end = marker_pos + len(marker) + next_match.start()
            return line_no(text, start), line_no(text, end), text[start:end]
    raise AssertionError(f"missing function body {name}")


def span(base_line: int, body: str, offsets: list[int]) -> str:
    return f"{base_line + line_no(body, min(offsets)) - 1}-{base_line + line_no(body, max(offsets)) - 1}"


def run(cmd: list[str]) -> str:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=90)
    if proc.returncode != 0:
        raise AssertionError(f"command failed {' '.join(cmd)}:\n{proc.stdout[-4000:]}")
    return proc.stdout.strip()


def git(*args: str) -> str:
    return run(["git", *args])


def find_test_exe() -> Path:
    candidates = [
        ROOT / "build-pass545" / "test_dm1_v1_movement_command_core_pc34_compat",
        ROOT / "build" / "test_dm1_v1_movement_command_core_pc34_compat",
    ]
    candidates.extend(sorted(ROOT.glob("build*/test_dm1_v1_movement_command_core_pc34_compat")))
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise AssertionError("missing built test_dm1_v1_movement_command_core_pc34_compat executable")


def main() -> int:
    if not RED.exists():
        raise AssertionError(f"missing ReDMCSB source root: {RED}")
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    command = read(COMMAND, "latin-1")
    clikmenu = read(CLIKMENU, "latin-1")
    movesens = read(MOVESENS, "latin-1")
    queue_c = read(QUEUE_C)
    core_c = read(CORE_C)
    test_c = read(CORE_TEST)

    f0380_start, f0380_end, f0380 = function_range(command, "F0380_COMMAND_ProcessQueue_CPSC")
    f0357_start, f0357_end, f0357 = function_range(command, "F0357_COMMAND_DiscardAllInput")
    if "C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL" not in f0357:
        for match in re.finditer(r"void\s+F0357_COMMAND_DiscardAllInput\s*\(", command):
            start = command.rfind("\n", 0, match.start()) + 1
            next_match = re.search(r"\n(?:void|int16_t|BOOLEAN|STATICFUNCTION\s+void)\s+F\d+_", command[match.end():])
            end = match.end() + next_match.start() if next_match else len(command)
            body = command[start:end]
            if "C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL" in body:
                f0357_start, f0357_end, f0357 = line_no(command, start), line_no(command, end), body
                break
    f0366_start, f0366_end, f0366 = function_range(clikmenu, "F0366_COMMAND_ProcessTypes3To6_MoveParty")
    f0267_start, f0267_end, f0267 = function_range(movesens, "F0267_MOVE_GetMoveResult_CPSCE")
    f0276_start, f0276_end, f0276 = function_range(movesens, "F0276_SENSOR_ProcessThingAdditionOrRemoval")
    qproc_start, qproc_end, qproc = function_range(queue_c, "DM1_V1_InputCommandQueue_ProcessOnePc34Compat", rettype=r"struct\s+Dm1V1InputQueueProcessResultPc34Compat")
    qdiscard_start, qdiscard_end, qdiscard = function_range(queue_c, "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat")
    core_start, core_end, core = function_range(core_c, "DM1_V1_MovementCommandCore_ProcessOnePc34Compat")

    f0380_queue = require_order(f0380, [
        "if (G2153_i_QueuedCommandsCount == 0)",
        "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD)",
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "G2153_i_QueuedCommandsCount--;",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "F0380 retains disabled moves and pops before F0366")
    f0366_paths = require_order(f0366, [
        "if (L1117_B_MovementBlocked) {",
        "F0357_COMMAND_DiscardAllInput();",
        "return;",
        "F0267_MOVE_GetMoveResult_CPSCE",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
    ], "F0366 blocked path discards and returns before successful move consequences")
    f0357_discard = require_order(f0357, [
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL",
        "G0434_i_CommandQueueLastIndex = L2285_i_DestinationCommandQueueIndex;",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
    ], "F0357 keeps only reserved release/stop commands and replays pending click")
    f0267_sensors = require_order(f0267, [
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY",
    ], "F0267 source leave before destination enter sensor processing")
    f0276_defer = require_order(f0276, [
        "F0272_SENSOR_TriggerEffect",
        "L0766_T_Thing = F0159_DUNGEON_GetNextThing(L0766_T_Thing);",
        "F0271_SENSOR_ProcessRotationEffect();",
    ], "F0276 processes sensor list before deferred rotation consequence")

    require_order(qproc, [
        "result.command = queue->commands[0].command;",
        "result.movementDisabledGate = 1;",
        "return result;",
        "queue->count--;",
        "result.dequeued = 1;",
    ], "Firestaff queue retains disabled move and otherwise dequeues exactly one")
    require_order(qdiscard, [
        "if (is_reserved_release_command(queue->commands[readIndex].command))",
        "queue->count = writeIndex;",
        "process_pending_click(queue);",
    ], "Firestaff discard keeps reserved commands and pending replay")
    require_order(core, [
        "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
        "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);",
        "return 1;",
        "F0718_SENSOR_ProcessPartyEnterLeave_Compat(",
        "party->mapIndex = outResult->movement.newMapIndex;",
        "F0718_SENSOR_ProcessPartyEnterLeave_Compat(",
        "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat",
    ], "Firestaff blocked path exits before successful source/destination sensor side effects")

    for label in [
        "pass545 successful movement retains trailing command",
        "pass545 successful movement destination sensor fires once",
        "pass545 successful movement destination sensor text index",
        "pass545 successful movement sets cooldown after sensors",
        "pass545 blocked movement discards trailing command",
        "pass545 blocked movement leaves sensor effects empty",
        "pass545 blocked movement destination sensor not fired",
        "pass545 blocked movement no cooldown after blocked",
    ]:
        require(test_c, label, f"runtime assertion label {label}")

    test_exe = find_test_exe()
    test_out = run([str(test_exe)])
    if "dm1V1MovementCommandCoreInvariantOk=1" not in test_out:
        raise AssertionError("runtime test did not report invariant ok")

    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS,
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "branch": git("branch", "--show-current"),
        "head": git("rev-parse", "HEAD"),
        "sourceAudit": {
            "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": f"COMMAND.C:{f0380_start}-{f0380_end}",
            "COMMAND.C:F0357_COMMAND_DiscardAllInput": f"COMMAND.C:{f0357_start}-{f0357_end}",
            "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": f"CLIKMENU.C:{f0366_start}-{f0366_end}",
            "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE": f"MOVESENS.C:{f0267_start}-{f0267_end}",
            "MOVESENS.C:F0276_SENSOR_ProcessThingAdditionOrRemoval": f"MOVESENS.C:{f0276_start}-{f0276_end}",
            "queueRetainPopDispatchSpan": f"COMMAND.C:{span(f0380_start, f0380, f0380_queue)}",
            "blockedDiscardSuccessBoundarySpan": f"CLIKMENU.C:{span(f0366_start, f0366, f0366_paths)}",
            "discardReservedPendingSpan": f"COMMAND.C:{span(f0357_start, f0357, f0357_discard)}",
            "moveSensorLeaveEnterSpan": f"MOVESENS.C:{span(f0267_start, f0267, f0267_sensors)}",
            "sensorRotationDeferSpan": f"MOVESENS.C:{span(f0276_start, f0276, f0276_defer)}",
        },
        "firestaffGuards": {
            "queueProcessOne": f"dm1_v1_input_command_queue_pc34_compat.c:{qproc_start}-{qproc_end}",
            "queueDiscard": f"dm1_v1_input_command_queue_pc34_compat.c:{qdiscard_start}-{qdiscard_end}",
            "movementCore": f"dm1_v1_movement_command_core_pc34_compat.c:{core_start}-{core_end}",
            "runtimeExecutable": str(test_exe.relative_to(ROOT)),
            "runtimeOutputLastLine": test_out.splitlines()[-1],
        },
        "closedGap": "successful movement dequeues one command, retains later queued input, runs destination sensor consequences, then applies cooldown; blocked movement dequeues the attempted move, discards nonreserved queued input, runs no movement sensor consequences, and does not assign successful-step cooldown.",
        "notClaimed": [
            "viewport wall drawing changes",
            "new DOS runtime capture",
            "full mutation of deferred sensor-list rotation beyond existing pass510 metadata",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass545 - DM1 V1 movement queue and sensor consequences",
        "",
        f"Status: {STATUS}",
        "",
        "## ReDMCSB Evidence",
    ]
    for key, value in manifest["sourceAudit"].items():
        lines.append(f"- {key}: {value}")
    lines += [
        "",
        "## Closed Gap",
        manifest["closedGap"],
        "",
        "## Local Gate",
        "- CTest: pass545_dm1_v1_movement_queue_sensor_consequences",
        "- Runtime: test_dm1_v1_movement_command_core_pc34_compat",
        "",
        "## Not Claimed",
    ]
    lines.extend(f"- {item}" for item in manifest["notClaimed"])
    lines.append("")
    REPORT.write_text("\n".join(lines), encoding="utf-8")
    print(STATUS)
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
