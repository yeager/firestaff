#!/usr/bin/env python3
"""Pass552: source-lock the DM1 V1 cooldown-gated movement queue loop."""
from __future__ import annotations

import json
import re
import subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass552_dm1_v1_movement_cooldown_queue_loop"
STATUS = "PASS552_DM1_V1_MOVEMENT_COOLDOWN_QUEUE_LOOP_LOCKED"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
REFS = {
    "redmcsb": RED,
    "greatstone": Path.home() / ".openclaw/data/firestaff-greatstone-atlas",
    "csbwin": Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin",
    "csb": Path.home() / ".openclaw/data/firestaff-csb-source/CSB",
    "original_dm": Path.home() / ".openclaw/data/firestaff-original-games/DM",
}
GAMELOOP = RED / "GAMELOOP.C"
COMMAND = RED / "COMMAND.C"
CLIKMENU = RED / "CLIKMENU.C"
QUEUE_C = ROOT / "dm1_v1_input_command_queue_pc34_compat.c"
TIMING_C = ROOT / "dm1_v1_movement_timing_pc34_compat.c"
TIMING_TEST = ROOT / "test_dm1_v1_movement_timing_pc34_compat.c"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"


def read(path: Path, encoding: str = "utf-8") -> str:
    return path.read_text(encoding=encoding)


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str, start: int = 0) -> int:
    pos = text.find(needle, start)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_order(text: str, needles: list[str], label: str) -> list[int]:
    offsets: list[int] = []
    last = -1
    for needle in needles:
        pos = require(text, needle, label, last + 1)
        offsets.append(pos)
        last = pos
    return offsets


def function_range(text: str, name: str, rettype: str = r"(?:STATICFUNCTION\s+)?(?:void|int|struct\s+\w+)") -> tuple[int, int, str]:
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
        next_match = re.search(r"\n(?:STATICFUNCTION\s+)?(?:void|int16_t|unsigned\s+int16_t|BOOLEAN|int|struct\s+\w+)\s+F\d+_", text[marker_pos + len(marker):])
        end = marker_pos + len(marker) + next_match.start() if next_match else len(text)
        return line_no(text, start), line_no(text, end), text[start:end]
    raise AssertionError(f"missing function body {name}")


def span(base_line: int, body: str, offsets: list[int]) -> str:
    return f"{base_line + line_no(body, min(offsets)) - 1}-{base_line + line_no(body, max(offsets)) - 1}"


def run(cmd: list[str]) -> str:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=180)
    if proc.returncode != 0:
        raise AssertionError(f"command failed {chr(32).join(cmd)}:\n{proc.stdout[-4000:]}")
    return proc.stdout.strip()


def git(*args: str) -> str:
    return run(["git", *args])


def find_test_exe() -> Path:
    candidates = [
        ROOT / "build-pass552" / "test_dm1_v1_movement_timing_pc34_compat",
        ROOT / "build" / "test_dm1_v1_movement_timing_pc34_compat",
    ]
    candidates.extend(sorted(ROOT.glob("build*/test_dm1_v1_movement_timing_pc34_compat")))
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise AssertionError("missing built test_dm1_v1_movement_timing_pc34_compat executable")


def main() -> int:
    missing = [name for name, path in REFS.items() if not path.exists()]
    if missing:
        raise AssertionError(f"missing N2-local reference roots: {(chr(44)+chr(32)).join(missing)}")

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    gameloop = read(GAMELOOP, "latin-1")
    command = read(COMMAND, "latin-1")
    clikmenu = read(CLIKMENU, "latin-1")
    queue_c = read(QUEUE_C)
    timing_c = read(TIMING_C)
    timing_test = read(TIMING_TEST)

    f0380_start, f0380_end, f0380 = function_range(command, "F0380_COMMAND_ProcessQueue_CPSC")
    f0366_start, f0366_end, f0366 = function_range(clikmenu, "F0366_COMMAND_ProcessTypes3To6_MoveParty")
    qproc_start, qproc_end, qproc = function_range(queue_c, "DM1_V1_InputCommandQueue_ProcessOnePc34Compat", rettype=r"struct\s+Dm1V1InputQueueProcessResultPc34Compat")
    dec_start, dec_end, decrement = function_range(timing_c, "DM1_V1_MovementTiming_DecrementCooldownsPc34Compat")

    gameloop_order = require_order(gameloop, [
        "if (G0310_i_DisabledMovementTicks) {",
        "G0310_i_DisabledMovementTicks--;",
        "if (G0311_i_ProjectileDisabledMovementTicks) {",
        "G0311_i_ProjectileDisabledMovementTicks--;",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "while (M527_IsCharacterInKeyboardBuffer()) {",
        "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
        "F0380_COMMAND_ProcessQueue_CPSC();",
        "if (!G0321_B_StopWaitingForPlayerInput) {",
        "F0363_COMMAND_HighlightBoxDisable();",
        "} while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
    ], "ReDMCSB GAMELOOP cooldown/input/F0380 loop order")
    f0380_gate = require_order(f0380, [
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD)",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
        "goto T0380042;",
        "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
        "G2153_i_QueuedCommandsCount--;",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "ReDMCSB F0380 disabled move remains queued until gate clears")
    f0366_timing = require_order(f0366, [
        "F0267_MOVE_GetMoveResult_CPSCE",
        "AL1115_ui_Ticks = 1;",
        "F0310_CHAMPION_GetMovementTicks",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        "G0311_i_ProjectileDisabledMovementTicks = 0;",
    ], "ReDMCSB successful step arms movement cooldown and clears projectile gate")
    fire_queue = require_order(qproc, [
        "result.command = queue->commands[0].command;",
        "result.movementDisabledGate = 1;",
        "queue->locked = 0;",
        "process_pending_click(queue);",
        "return result;",
        "queue->count--;",
        "result.dequeued = 1;",
    ], "Firestaff queue keeps gated movement at the front")
    fire_decrement = require_order(decrement, [
        "if (disabledMovementTicks && *disabledMovementTicks > 0) {",
        "--*disabledMovementTicks;",
        "if (projectileDisabledMovementTicks && *projectileDisabledMovementTicks > 0) {",
        "--*projectileDisabledMovementTicks;",
    ], "Firestaff timing decrements both movement cooldowns once per loop")

    for label in [
        "game loop decrements movement cooldown independently",
        "game loop decrements projectile movement cooldown independently",
        "projectile cooldown still blocks matching movement after one tick",
        "projectile cooldown leaves matching movement queued",
        "movement cooldown saturates at zero",
        "projectile movement cooldown reaches zero after four ticks",
        "expired projectile cooldown releases queued movement",
        "released queued movement dequeued",
    ]:
        require(timing_test, label, f"timing regression label {label}")

    test_exe = find_test_exe()
    test_out = run([str(test_exe)])
    if "dm1V1MovementTimingInvariantOk=1" not in test_out:
        raise AssertionError("movement timing regression did not report invariant ok")

    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS,
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "branch": git("branch", "--show-current"),
        "head": git("rev-parse", "HEAD"),
        "worktree": str(ROOT),
        "scope": "DM1 V1 PC-34 movement cooldown tick order and queued-step release",
        "primarySourceAudit": {
            "GAMELOOP.C:cooldownInputQueueLoop": f"GAMELOOP.C:{span(1, gameloop, gameloop_order)}",
            "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": f"COMMAND.C:{f0380_start}-{f0380_end}",
            "COMMAND.C:disabledMoveQueueRetention": f"COMMAND.C:{span(f0380_start, f0380, f0380_gate)}",
            "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": f"CLIKMENU.C:{f0366_start}-{f0366_end}",
            "CLIKMENU.C:successfulStepCooldown": f"CLIKMENU.C:{span(f0366_start, f0366, f0366_timing)}",
        },
        "firestaffGuards": {
            "queueProcessOne": f"dm1_v1_input_command_queue_pc34_compat.c:{qproc_start}-{qproc_end}",
            "queueRetentionSpan": f"dm1_v1_input_command_queue_pc34_compat.c:{span(qproc_start, qproc, fire_queue)}",
            "cooldownDecrement": f"dm1_v1_movement_timing_pc34_compat.c:{dec_start}-{dec_end}",
            "cooldownDecrementSpan": f"dm1_v1_movement_timing_pc34_compat.c:{span(dec_start, decrement, fire_decrement)}",
            "regression": "test_dm1_v1_movement_timing_pc34_compat",
        },
        "runtimeProbe": {"command": str(test_exe), "statusNeedle": "dm1V1MovementTimingInvariantOk=1"},
        "notClaimed": ["new original DOSBox runtime capture", "viewport or wall pixel parity", "new movement engine behavior beyond the cooldown queue loop evidence"],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    lines = ["# Pass552 DM1 V1 movement cooldown queue loop", "", f"Status: **{STATUS}**", "", "Scope: source-lock the loop that decrements movement cooldowns before raw input and F0380 queue processing, then prove a gated movement command stays queued until that gate clears.", "", "## ReDMCSB citations", ""]
    for label, citation in manifest["primarySourceAudit"].items():
        lines.append(f"- {citation} - {label}")
    lines += ["", "## Firestaff guards", ""]
    for label, citation in manifest["firestaffGuards"].items():
        lines.append(f"- {citation} - {label}")
    lines += ["", "## Gates", "", f"- {test_exe} - reported dm1V1MovementTimingInvariantOk=1", "", "## Not claimed", ""]
    lines += [f"- {item}" for item in manifest["notClaimed"]]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"{STATUS} manifest={MANIFEST}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
