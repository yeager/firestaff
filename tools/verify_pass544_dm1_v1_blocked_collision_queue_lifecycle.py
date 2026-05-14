#!/usr/bin/env python3
"""Pass544: source-lock blocked-collision queue lifecycle after pass542."""
from __future__ import annotations

import json
from pathlib import Path
import re
import subprocess
import sys
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass544_dm1_v1_blocked_collision_queue_lifecycle"
STATUS = "PASS544_DM1_V1_BLOCKED_COLLISION_QUEUE_LIFECYCLE_LOCKED"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
REFS = {
    "redmcsb": RED,
    "greatstone": Path.home() / ".openclaw/data/firestaff-greatstone-atlas",
    "csbwin": Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin",
    "csb": Path.home() / ".openclaw/data/firestaff-csb-source/CSB",
    "original_dm": Path.home() / ".openclaw/data/firestaff-original-games/DM",
}
COMMAND = RED / "COMMAND.C"
CLIKMENU = RED / "CLIKMENU.C"
QUEUE_C = ROOT / "dm1_v1_input_command_queue_pc34_compat.c"
CORE_C = ROOT / "dm1_v1_movement_command_core_pc34_compat.c"
CORE_TEST = ROOT / "test_dm1_v1_movement_command_core_pc34_compat.c"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str, start: int = 0) -> int:
    pos = text.find(needle, start)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def order(text: str, needles: list[str], label: str) -> list[int]:
    positions: list[int] = []
    last = -1
    for needle in needles:
        pos = require(text, needle, label, last + 1)
        positions.append(pos)
        last = pos
    return positions


def function_range(text: str, name: str, rettype: str = r"(?:STATICFUNCTION\s+)?(?:static\s+)?(?:void|int|const\s+char\*)") -> tuple[int, int, str]:
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
        next_match = re.search(r"\n(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|int)\s+F\d+_", text[marker_pos + len(marker):])
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
        ROOT / "build-pass544" / "test_dm1_v1_movement_command_core_pc34_compat",
        ROOT / "build" / "test_dm1_v1_movement_command_core_pc34_compat",
    ]
    candidates.extend(sorted(ROOT.glob("build*/test_dm1_v1_movement_command_core_pc34_compat")))
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise AssertionError("missing built test_dm1_v1_movement_command_core_pc34_compat executable")


def main() -> int:
    missing = [name for name, path in REFS.items() if not path.exists()]
    if missing:
        raise AssertionError(f"missing N2-local reference roots: {', '.join(missing)}")

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    command = COMMAND.read_text(encoding="latin-1")
    clikmenu = CLIKMENU.read_text(encoding="latin-1")
    queue_c = QUEUE_C.read_text(encoding="utf-8")
    core_c = CORE_C.read_text(encoding="utf-8")
    test_c = CORE_TEST.read_text(encoding="utf-8")

    f0380_start, f0380_end, f0380 = function_range(command, "F0380_COMMAND_ProcessQueue_CPSC")
    f0357_start, f0357_end, f0357 = function_range(command, "F0357_COMMAND_DiscardAllInput")
    if "C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL" not in f0357:
        for match in re.finditer(r"void\s+F0357_COMMAND_DiscardAllInput\s*\(", command):
            start = command.rfind("\n", 0, match.start()) + 1
            next_match = re.search(r"\n(?:void|int16_t|BOOLEAN|STATICFUNCTION\s+void)\s+F\d+_", command[match.end():])
            end = match.end() + next_match.start() if next_match else len(command)
            body = command[start:end]
            if "C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL" in body and "G0435_B_CommandQueueLocked = C1_TRUE;" in body:
                f0357_start, f0357_end, f0357 = line_no(command, start), line_no(command, end), body
                break
    f0366_start, f0366_end, f0366 = function_range(clikmenu, "F0366_COMMAND_ProcessTypes3To6_MoveParty")
    qproc_start, qproc_end, qproc = function_range(
        queue_c,
        "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
        rettype=r"struct\s+Dm1V1InputQueueProcessResultPc34Compat",
    )
    qdiscard_start, qdiscard_end, qdiscard = function_range(
        queue_c,
        "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat",
    )
    core_start, core_end, core = function_range(
        core_c,
        "DM1_V1_MovementCommandCore_ProcessOnePc34Compat",
    )

    f0380_dequeue_dispatch = order(f0380, [
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "G2153_i_QueuedCommandsCount--;",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "ReDMCSB F0380 dequeue/replay/dispatch order")
    f0366_blocked = order(f0366, [
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "L1117_B_MovementBlocked = C0_FALSE;",
        "if (L1117_B_MovementBlocked) {",
        "F0357_COMMAND_DiscardAllInput();",
        "F0693_WaitVerticalBlank();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "return;",
    ], "ReDMCSB F0366 blocked collision order")
    f0366_success_after_block = order(f0366, [
        "if (L1117_B_MovementBlocked) {",
        "return;",
        "F0267_MOVE_GetMoveResult_CPSCE",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
    ], "ReDMCSB blocked path returns before successful cooldown")
    f0357_preserve = order(f0357, [
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL",
        "G2153_i_QueuedCommandsCount++;",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
    ], "ReDMCSB F0357 reserved command preserve and pending replay")

    fire_qproc = order(qproc, [
        "result.command = queue->commands[0].command;",
        "queue->count--;",
        "queue->locked = 0;",
        "process_pending_click(queue);",
        "result.dispatchedMove = 1;",
    ], "Firestaff queue process pops and replays before move dispatch")
    fire_qdiscard = order(qdiscard, [
        "queue->locked = 1;",
        "if (is_reserved_release_command(queue->commands[readIndex].command))",
        "queue->count = writeIndex;",
        "queue->locked = 0;",
        "process_pending_click(queue);",
    ], "Firestaff discard preserves reserved commands and replays pending")
    fire_core_blocked = order(core, [
        "outResult->queue = DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
        "if (!F0702_MOVEMENT_TryMove_Compat",
        "outResult->movementBlocked = 1;",
        "outResult->inputDiscardRequested = 1;",
        "outResult->blockedMovementVblankWaitRequested = 1;",
        "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);",
        "return 1;",
        "outResult->timing = DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat",
    ], "Firestaff blocked collision returns before successful timing")

    for label in [
        "pass544 blocked collision front move queued",
        "pass544 blocked collision processed after cooldown clear",
        "pass544 blocked collision move dequeued before F0366",
        "pass544 blocked collision dispatched to move handler",
        "pass544 blocked collision one blocked vblank requested",
        "pass544 blocked collision no successful step cooldown",
        "pass544 blocked collision keeps input wait armed",
        "pass544 blocked collision pending replayed once",
        "pass544 blocked collision keeps only reserved commands",
        "pass544 blocked collision drops nonreserved trailing turn",
        "pass544 blocked collision preserves pending stop command",
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
        "worktree": str(ROOT),
        "primarySourceAudit": {
            "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": f"COMMAND.C:{f0380_start}-{f0380_end}",
            "COMMAND.C:F0357_COMMAND_DiscardAllInput": f"COMMAND.C:{f0357_start}-{f0357_end}",
            "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": f"CLIKMENU.C:{f0366_start}-{f0366_end}",
            "dequeuePendingReplayMoveDispatch": f"COMMAND.C:{span(f0380_start, f0380, f0380_dequeue_dispatch)}",
            "blockedCollisionDiscardVblankReturn": f"CLIKMENU.C:{span(f0366_start, f0366, f0366_blocked)}",
            "blockedReturnsBeforeSuccessfulCooldown": f"CLIKMENU.C:{span(f0366_start, f0366, f0366_success_after_block)}",
            "discardPreservesReservedAndReplaysPending": f"COMMAND.C:{span(f0357_start, f0357, f0357_preserve)}",
        },
        "firestaffGuards": {
            "queueProcessOne": f"dm1_v1_input_command_queue_pc34_compat.c:{qproc_start}-{qproc_end}",
            "queueProcessSpan": f"dm1_v1_input_command_queue_pc34_compat.c:{span(qproc_start, qproc, fire_qproc)}",
            "queueDiscard": f"dm1_v1_input_command_queue_pc34_compat.c:{qdiscard_start}-{qdiscard_end}",
            "queueDiscardSpan": f"dm1_v1_input_command_queue_pc34_compat.c:{span(qdiscard_start, qdiscard, fire_qdiscard)}",
            "movementCore": f"dm1_v1_movement_command_core_pc34_compat.c:{core_start}-{core_end}",
            "blockedCollisionSpan": f"dm1_v1_movement_command_core_pc34_compat.c:{span(core_start, core, fire_core_blocked)}",
            "runtimeExecutable": str(test_exe.relative_to(ROOT)),
            "runtimeOutputLastLine": test_out.splitlines()[-1],
        },
        "whyNotPass542Duplicate": "pass542 locks pre-dequeue cooldown/projectile gating where the movement command remains queued; pass544 locks the adjacent post-dequeue blocked collision path where ordinary queued/pending input is discarded, reserved release/stop survives, one blocked VBlank is requested, and successful-step cooldown is not assigned.",
        "notClaimed": [
            "new original DOS runtime capture",
            "viewport or wall pixel parity",
            "damage RNG/wound materialization beyond the recorded blocked-collision request",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    REPORT.write_text("\n".join([
        "# Pass544 - DM1 V1 blocked collision queue lifecycle",
        "",
        f"Status: {STATUS}",
        "",
        "## Why this is not pass542 again",
        "- Pass542 already locks the pre-dequeue movement-disabled gate: cooldown/projectile timing leaves the front movement command queued and replays pending click.",
        "- This pass locks the next boundary: cooldown is clear, the movement command is dequeued and dispatched to F0366, then wall/door collision blocks the step and F0357 discards ordinary queued/pending input while preserving reserved release/stop commands.",
        "",
        "## ReDMCSB-first source audit",
        f"- COMMAND.C:{f0380_start}-{f0380_end} / F0380 dequeues the front command, unlocks/replays pending click, then dispatches movement to F0366.",
        f"- CLIKMENU.C:{f0366_start}-{f0366_end} / F0366 detects blocked wall/door/fake-wall/group collision, calls F0357, waits one PC-34 VBlank, keeps input wait armed, and returns before successful-step timing/cooldown.",
        f"- COMMAND.C:{f0357_start}-{f0357_end} / F0357 flushes input, preserves C129/C254 reserved release/stop commands, unlocks, and replays one pending click.",
        "",
        "## Firestaff guards",
        f"- dm1_v1_input_command_queue_pc34_compat.c:{qproc_start}-{qproc_end} pops and replays pending input before move dispatch.",
        f"- dm1_v1_input_command_queue_pc34_compat.c:{qdiscard_start}-{qdiscard_end} preserves only reserved release/stop commands during discard and replays pending click after unlock.",
        f"- dm1_v1_movement_command_core_pc34_compat.c:{core_start}-{core_end} returns from blocked collision before successful movement timing/cooldown.",
        "- test_dm1_v1_movement_command_core_pc34_compat covers: front move dequeued after cooldown clears, wall collision blocks, trailing nonreserved turn is dropped, pre-existing reserved release and replayed pending stop survive, one blocked VBlank is requested, input wait remains armed, and no successful-step cooldown is assigned.",
        "",
        "## Scope guard",
        "- DM1 V1 movement/förflyttning only. No original runtime capture, viewport/walls, damage RNG, or pixel parity claim.",
        "",
        f"Manifest: parity-evidence/verification/{PASS}/manifest.json",
    ]) + "\n")

    print(f"PASS {PASS}")
    print(f"- ReDMCSB COMMAND.C:{f0380_start}-{f0380_end}, COMMAND.C:{f0357_start}-{f0357_end}, CLIKMENU.C:{f0366_start}-{f0366_end}")
    print(f"- Firestaff queue/core guards {qproc_start}-{qproc_end}, {qdiscard_start}-{qdiscard_end}, {core_start}-{core_end}")
    print(f"- report: {REPORT.relative_to(ROOT)}")
    print(f"- manifest: {MANIFEST.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, subprocess.SubprocessError) as exc:
        print(f"FAIL {PASS}: {exc}", file=sys.stderr)
        raise SystemExit(1)
