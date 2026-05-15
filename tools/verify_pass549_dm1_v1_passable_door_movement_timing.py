#!/usr/bin/env python3
"""Pass549: source-lock DM1 V1 passable door movement/timing states."""
from __future__ import annotations

import json
import re
import subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass549_dm1_v1_passable_door_movement_timing"
STATUS = "PASS549_DM1_V1_PASSABLE_DOOR_MOVEMENT_TIMING_LOCKED"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CLIKMENU = RED / "CLIKMENU.C"
DEFS = RED / "DEFS.H"
MOVE_C = ROOT / "memory_movement_pc34_compat.c"
MOVE_H = ROOT / "memory_movement_pc34_compat.h"
CORE_C = ROOT / "dm1_v1_movement_command_core_pc34_compat.c"
CORE_TEST = ROOT / "test_dm1_v1_movement_command_core_pc34_compat.c"
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
    positions: list[int] = []
    last = -1
    for needle in needles:
        pos = require(text, needle, label, last + 1)
        positions.append(pos)
        last = pos
    return positions


def function_range(
    text: str,
    name: str,
    rettype: str = r"(?:STATICFUNCTION\s+)?(?:static\s+)?(?:void|int|int16_t|unsigned\s+int16_t|BOOLEAN|const\s+char\*)",
) -> tuple[int, int, str]:
    pattern = re.compile(r"\b(?:" + rettype + r")\s+" + re.escape(name) + r"\s*\(")
    for match in pattern.finditer(text):
        brace = text.find("{", match.end())
        if brace < 0:
            continue
        depth = 0
        for pos in range(brace, len(text)):
            if text[pos] == "{":
                depth += 1
            elif text[pos] == "}":
                depth -= 1
                if depth == 0:
                    return line_no(text, match.start()), line_no(text, pos), text[match.start() : pos + 1]
    marker = name + "("
    marker_pos = text.find(marker)
    if marker_pos >= 0:
        start = text.rfind("\n", 0, marker_pos) + 1
        next_match = re.search(
            r"\n(?:STATICFUNCTION\s+)?(?:void|int16_t|unsigned\s+int16_t|BOOLEAN|int)\s+F\d+_",
            text[marker_pos + len(marker) :],
        )
        end = marker_pos + len(marker) + next_match.start() if next_match else len(text)
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
        ROOT / "build-pass549" / "test_dm1_v1_movement_command_core_pc34_compat",
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

    clik = read(CLIKMENU, "latin-1")
    defs = read(DEFS, "latin-1")
    move_c = read(MOVE_C)
    move_h = read(MOVE_H)
    core_c = read(CORE_C)
    test_c = read(CORE_TEST)

    f0366_start, f0366_end, f0366 = function_range(clik, "F0366_COMMAND_ProcessTypes3To6_MoveParty")
    f0702_start, f0702_end, f0702 = function_range(move_c, "F0702_MOVEMENT_TryMove_Compat")
    core_start, core_end, core = function_range(core_c, "DM1_V1_MovementCommandCore_ProcessOnePc34Compat")

    red_door = require_order(
        f0366,
        [
            "if (L1116_i_SquareType == C04_ELEMENT_DOOR) {",
            "L1117_B_MovementBlocked = M036_DOOR_STATE(AL1115_ui_Square);",
            "L1117_B_MovementBlocked = (L1117_B_MovementBlocked != C0_DOOR_STATE_OPEN) && (L1117_B_MovementBlocked != C1_DOOR_STATE_CLOSED_ONE_FOURTH) && (L1117_B_MovementBlocked != C5_DOOR_STATE_DESTROYED);",
            "if (G0305_ui_PartyChampionCount == 0)",
            "if (L1117_B_MovementBlocked) {",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
            "G0311_i_ProjectileDisabledMovementTicks = 0;",
        ],
        "ReDMCSB F0366 passable door states continue to successful movement timing",
    )
    red_defs = require_order(
        defs,
        [
            "#define C0_DOOR_STATE_OPEN",
            "#define C1_DOOR_STATE_CLOSED_ONE_FOURTH",
            "#define C5_DOOR_STATE_DESTROYED",
        ],
        "ReDMCSB door state constants",
    )

    fire_move = require_order(
        f0702,
        [
            "if (elementType == DUNGEON_ELEMENT_DOOR) {",
            "doorState = squareByte & 0x07;",
            "if (doorState != 0 && doorState != 1 && doorState != 5) {",
            "outResult->resultCode = MOVE_BLOCKED_DOOR;",
            "return 0;",
            "outResult->newMapX = nx;",
            "outResult->newMapY = ny;",
            "outResult->resultCode = MOVE_OK;",
        ],
        "Firestaff F0702 passable door states fall through to MOVE_OK",
    )
    fire_core = require_order(
        core,
        [
            "if (!F0702_MOVEMENT_TryMove_Compat",
            "outResult->movementBlocked = 1;",
            "return 1;",
            "party->mapIndex = outResult->movement.newMapIndex;",
            "party->mapX = outResult->movement.newMapX;",
            "party->mapY = outResult->movement.newMapY;",
            "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat",
            "outResult->stepApplied = 1;",
            "outResult->stopWaitingForPlayerInput = 1;",
            "outResult->viewportRedrawRequested = 1;",
        ],
        "Firestaff command core accepts F0702 MOVE_OK door states into timing/redraw path",
    )

    for needle in [
        "Pass549: passable door states are accepted movement",
        "M036_DOOR_STATE is neither C0_OPEN, C1_CLOSED_ONE_FOURTH, nor",
        "pass549 one-fourth door is accepted movement",
        "pass549 one-fourth door not blocked",
        "pass549 one-fourth door keeps trailing queued input",
        "pass549 one-fourth door destination sensor",
        "pass549 one-fourth door sets cooldown",
        "pass549 destroyed door is accepted movement",
        "pass549 destroyed door not blocked",
        "pass549 destroyed door destination sensor",
        "pass549 destroyed door sets cooldown",
    ]:
        haystack = move_h if needle.startswith("doors are") or "closed one-fourth" in needle else test_c
        require(haystack, needle, f"Firestaff guard {needle}")

    test_exe = find_test_exe()
    test_out = run([str(test_exe)])
    if "dm1V1MovementCommandCoreInvariantOk=1" not in test_out:
        raise AssertionError("runtime movement command core test did not report invariant ok")

    previous_manifest = {}
    if MANIFEST.exists():
        try:
            previous_manifest = json.loads(MANIFEST.read_text())
        except Exception:
            previous_manifest = {}

    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS,
        "timestampUtc": previous_manifest.get("timestampUtc") or datetime.now(timezone.utc).isoformat(),
        "branch": git("branch", "--show-current"),
        "head": previous_manifest.get("head") or git("rev-parse", "HEAD"),
        "worktree": str(ROOT),
        "primarySourceAudit": {
            "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": f"CLIKMENU.C:{f0366_start}-{f0366_end}",
            "doorPassabilityToSuccessPath": f"CLIKMENU.C:{span(f0366_start, f0366, red_door)}",
            "DEFS.H door constants": f"DEFS.H:{span(1, defs, red_defs)}",
        },
        "firestaffGuards": {
            "F0702_MOVEMENT_TryMove_Compat": f"memory_movement_pc34_compat.c:{f0702_start}-{f0702_end}",
            "doorFallthroughToMoveOk": f"memory_movement_pc34_compat.c:{span(f0702_start, f0702, fire_move)}",
            "DM1_V1_MovementCommandCore_ProcessOnePc34Compat": f"dm1_v1_movement_command_core_pc34_compat.c:{core_start}-{core_end}",
            "acceptedMovementToTiming": f"dm1_v1_movement_command_core_pc34_compat.c:{span(core_start, core, fire_core)}",
            "runtimeExecutable": str(test_exe.relative_to(ROOT)),
            "runtimeOutputLastLine": test_out.splitlines()[-1],
        },
        "whyNotPass547Duplicate": "pass547 locks blocked closed-door/fake-wall/group convergence. Pass549 locks the opposite door-state branch: one-fourth and destroyed doors are accepted movement, keep queued follow-up input, process destination sensors, set movement cooldown, clear projectile cooldown, and request viewport redraw.",
        "notClaimed": [
            "new original DOS runtime capture",
            "pass514/pass388 capture follow-up",
            "door animation or pixel parity",
            "creature/projectile door behavior",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    report = "\n".join(
        [
            "# Pass549 - DM1 V1 passable door movement/timing",
            "",
            f"Status: {STATUS}",
            "",
            "## Source audit",
            "",
            f"- CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty {manifest['primarySourceAudit']['doorPassabilityToSuccessPath']} checks door state and only blocks states other than open, closed-one-fourth, and destroyed before falling through to F0267 and cooldown assignment.",
            f"- DEFS.H {manifest['primarySourceAudit']['DEFS.H door constants']} defines the door states used by the condition.",
            "",
            "## Firestaff lock",
            "",
            f"- memory_movement_pc34_compat.c {manifest['firestaffGuards']['doorFallthroughToMoveOk']} preserves doorState != 0 && != 1 && != 5 as the only door block branch, so states 1 and 5 fall through to MOVE_OK.",
            f"- dm1_v1_movement_command_core_pc34_compat.c {manifest['firestaffGuards']['acceptedMovementToTiming']} takes that MOVE_OK into party position mutation, destination sensors, successful-step timing, input-wait release, and viewport redraw.",
            "- test_dm1_v1_movement_command_core_pc34_compat.c now covers one-fourth and destroyed door front-key commands through accepted movement/timing behavior.",
            "",
            "## Boundary",
            "",
            "This is deliberately not a pass514/pass388 capture follow-up. It adds no original runtime capture claim and no pixel/door-animation parity claim.",
        ]
    )
    REPORT.write_text(report + "\n")
    print(STATUS)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
