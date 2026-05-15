#!/usr/bin/env python3
"""Pass559: source-lock gated movement pending-click replay order."""
from __future__ import annotations

import json
import re
import subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass559_dm1_v1_gated_movement_pending_click_queue_replay"
STATUS = "PASS559_DM1_V1_GATED_MOVEMENT_PENDING_CLICK_QUEUE_REPLAY_LOCKED"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
COMMAND = RED / "COMMAND.C"
QUEUE_C = ROOT / "dm1_v1_input_command_queue_pc34_compat.c"
TEST_C = ROOT / "test_dm1_v1_command_movement_sensor_timing_pc34_compat.c"
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
    raise AssertionError(f"missing function body {name}")


def span(base_line: int, body: str, offsets: list[int]) -> str:
    return f"{base_line + line_no(body, min(offsets)) - 1}-{base_line + line_no(body, max(offsets)) - 1}"


def run(cmd: list[str]) -> str:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=180)
    if proc.returncode != 0:
        raise AssertionError(f"command failed {' '.join(cmd)}:\n{proc.stdout[-4000:]}")
    return proc.stdout.strip()


def git(*args: str) -> str:
    return run(["git", *args])


def find_test_exe() -> Path:
    candidates = [
        ROOT / "build-pass559" / "test_dm1_v1_command_movement_sensor_timing_pc34_compat",
        ROOT / "build" / "test_dm1_v1_command_movement_sensor_timing_pc34_compat",
    ]
    candidates.extend(sorted(ROOT.glob("build*/test_dm1_v1_command_movement_sensor_timing_pc34_compat")))
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise AssertionError("missing built test_dm1_v1_command_movement_sensor_timing_pc34_compat executable")


def main() -> int:
    if not RED.exists():
        raise AssertionError(f"missing ReDMCSB source root: {RED}")

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    command = read(COMMAND, "latin-1")
    queue_c = read(QUEUE_C)
    test_c = read(TEST_C)

    f0380_start, f0380_end, f0380 = function_range(command, "F0380_COMMAND_ProcessQueue_CPSC")
    qproc_start, qproc_end, qproc = function_range(
        queue_c,
        "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
        rettype=r"struct\s+Dm1V1InputQueueProcessResultPc34Compat",
    )

    red_gate_replay = require_order(
        f0380,
        [
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD)",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
            "goto T0380042;",
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "G2153_i_QueuedCommandsCount--;",
        ],
        "ReDMCSB gated movement unlocks/replays pending click before any later dequeue path",
    )
    fire_gate_replay = require_order(
        qproc,
        [
            "result.command = queue->commands[0].command;",
            "result.movementDisabledGate = 1;",
            "queue->locked = 0;",
            "process_pending_click(queue);",
            "return result;",
            "queue->count--;",
            "result.dequeued = 1;",
        ],
        "Firestaff gated movement returns without dequeuing, after pending click replay",
    )

    for label in [
        "pass559 gated movement replays pending click",
        "pass559 gated movement not dequeued",
        "pass559 gated movement leaves original command first",
        "pass559 gated movement appends pending click behind it",
        "pass559 expired gate releases original movement first",
        "pass559 pending click dequeues after original movement",
    ]:
        require(test_c, label, f"runtime regression label {label}")

    test_exe = find_test_exe()
    test_out = run([str(test_exe)])
    if "dm1V1CommandMovementSensorTimingIntegrationOk=1" not in test_out:
        raise AssertionError("movement command runtime regression did not report invariant ok")

    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS,
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "branch": git("branch", "--show-current"),
        "head": git("rev-parse", "HEAD"),
        "worktree": str(ROOT),
        "scope": "DM1 V1 PC-34 movement cooldown gate plus pending-click replay queue order",
        "primarySourceAudit": {
            "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": f"COMMAND.C:{f0380_start}-{f0380_end}",
            "COMMAND.C:gatedMovementPendingClickReplayBeforeDequeue": f"COMMAND.C:{span(f0380_start, f0380, red_gate_replay)}",
        },
        "firestaffGuards": {
            "DM1_V1_InputCommandQueue_ProcessOnePc34Compat": f"dm1_v1_input_command_queue_pc34_compat.c:{qproc_start}-{qproc_end}",
            "gatedMovementReturnsAfterPendingReplay": f"dm1_v1_input_command_queue_pc34_compat.c:{span(qproc_start, qproc, fire_gate_replay)}",
            "runtimeRegression": "test_dm1_v1_command_movement_sensor_timing_pc34_compat.c:pass559 labels",
            "runtimeExecutable": str(test_exe.relative_to(ROOT)),
            "runtimeOutputLastLine": test_out.splitlines()[-1],
        },
        "notClaimed": [
            "new original DOSBox runtime capture",
            "viewport pixel parity",
            "new movement engine behavior beyond gated queue replay order",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass559 - DM1 V1 gated movement pending-click queue replay",
        "",
        f"Status: **{STATUS}**",
        "",
        "Scope: source-lock and runtime-guard the F0380 branch where a cooldown-gated movement command remains at the queue head while a pending click is replayed behind it.",
        "",
        "## ReDMCSB citations",
        "",
    ]
    for label, citation in manifest["primarySourceAudit"].items():
        lines.append(f"- {citation} - {label}")
    lines += ["", "## Firestaff guards", ""]
    for label, citation in manifest["firestaffGuards"].items():
        lines.append(f"- {citation} - {label}")
    lines += ["", "## Gates", "", f"- {test_exe} - reported dm1V1CommandMovementSensorTimingIntegrationOk=1", "", "## Not claimed", ""]
    lines += [f"- {item}" for item in manifest["notClaimed"]]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"{STATUS} manifest={MANIFEST}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
