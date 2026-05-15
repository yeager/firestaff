#!/usr/bin/env python3
"""Pass580: source-lock DM1 V1 forward collision/timing around queued moves.

This gate is intentionally evidence-only. It binds the Firestaff command core
to ReDMCSB for the narrow forward-step collision path: F0380 queue dispatch and
cooldown gating, F0366 pre-step stamina and blocked-square behavior, F0325
stamina side effects, and the invariant that a blocked step leaves party
position unchanged and never reaches successful-step timing.
"""
from __future__ import annotations

from datetime import datetime, timezone
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass580_dm1_v1_forward_collision_timing"
STATUS = "PASS580_DM1_V1_FORWARD_COLLISION_TIMING_LOCKED"
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
GAMELOOP = RED / "GAMELOOP.C"
CHAMPION = RED / "CHAMPION.C"
CORE_C = ROOT / "dm1_v1_movement_command_core_pc34_compat.c"
CORE_H = ROOT / "dm1_v1_movement_command_core_pc34_compat.h"
QUEUE_C = ROOT / "dm1_v1_input_command_queue_pc34_compat.c"
TIMING_C = ROOT / "dm1_v1_movement_timing_pc34_compat.c"
CORE_TEST = ROOT / "test_dm1_v1_movement_command_core_pc34_compat.c"
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


def order(text: str, needles: list[str], label: str) -> list[int]:
    offsets: list[int] = []
    last = -1
    for needle in needles:
        pos = require(text, needle, label, last + 1)
        offsets.append(pos)
        last = pos
    return offsets


def function_range(
    text: str,
    name: str,
    rettype: str = r"(?:STATICFUNCTION\s+)?(?:static\s+)?(?:void|int|int16_t|BOOLEAN|struct\s+\w+|const\s+char\*)",
) -> tuple[int, int, str]:
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
        next_match = re.search(
            r"\n(?:STATICFUNCTION\s+)?(?:void|int16_t|unsigned\s+int16_t|BOOLEAN|int|struct\s+\w+)\s+F\d+_",
            text[marker_pos + len(marker):],
        )
        end = marker_pos + len(marker) + next_match.start() if next_match else len(text)
        return line_no(text, start), line_no(text, end), text[start:end]
    raise AssertionError(f"missing function body {name}")


def span(base_line: int, body: str, offsets: list[int]) -> str:
    return f"{base_line + line_no(body, min(offsets)) - 1}-{base_line + line_no(body, max(offsets)) - 1}"


def run(cmd: list[str], timeout: int = 180) -> str:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    if proc.returncode != 0:
        raise AssertionError(f"command failed {' '.join(cmd)}:\n{proc.stdout[-4000:]}")
    return proc.stdout.strip()


def git(*args: str) -> str:
    return run(["git", *args], timeout=60)


def find_exe(name: str) -> Path:
    candidates = [ROOT / "build" / name]
    candidates.extend(sorted(ROOT.glob(f"build*/{name}")))
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise AssertionError(f"missing built executable {name}")


def main() -> int:
    missing = [name for name, path in REFS.items() if not path.exists()]
    if missing:
        raise AssertionError(f"missing N2-local reference roots: {', '.join(missing)}")

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    command = read(COMMAND, "latin-1")
    clik = read(CLIKMENU, "latin-1")
    gameloop = read(GAMELOOP, "latin-1")
    champion = read(CHAMPION, "latin-1")
    core_c = read(CORE_C)
    core_h = read(CORE_H)
    queue_c = read(QUEUE_C)
    timing_c = read(TIMING_C)
    core_test = read(CORE_TEST)
    timing_test = read(TIMING_TEST)

    f0380_start, f0380_end, f0380 = function_range(command, "F0380_COMMAND_ProcessQueue_CPSC")
    f0366_start, f0366_end, f0366 = function_range(clik, "F0366_COMMAND_ProcessTypes3To6_MoveParty")
    f0325_start, f0325_end, f0325 = function_range(champion, "F0325_CHAMPION_DecrementStamina")
    core_start, core_end, core = function_range(core_c, "DM1_V1_MovementCommandCore_ProcessOnePc34Compat")
    qproc_start, qproc_end, qproc = function_range(
        queue_c,
        "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
        rettype=r"struct\s+Dm1V1InputQueueProcessResultPc34Compat",
    )
    qdiscard_start, qdiscard_end, qdiscard = function_range(queue_c, "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat")
    timing_start, timing_end, timing = function_range(timing_c, "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat")

    gameloop_tick = order(gameloop, [
        "if (G0310_i_DisabledMovementTicks) {",
        "G0310_i_DisabledMovementTicks--;",
        "if (G0311_i_ProjectileDisabledMovementTicks) {",
        "G0311_i_ProjectileDisabledMovementTicks--;",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "F0361_COMMAND_ProcessKeyPress",
        "F0380_COMMAND_ProcessQueue_CPSC();",
    ], "ReDMCSB gameloop decrements cooldowns before queue processing")
    f0380_gate = order(f0380, [
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD)",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
        "goto T0380042;",
        "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
        "G2153_i_QueuedCommandsCount--;",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "ReDMCSB F0380 gated move remains queued; ungated move dequeues and dispatches")
    f0366_forward_collision = order(f0366, [
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "F0325_CHAMPION_DecrementStamina",
        "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
        "if (L1117_B_MovementBlocked) {",
        "F0357_COMMAND_DiscardAllInput();",
        "F0693_WaitVerticalBlank();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "return;",
        "F0267_MOVE_GetMoveResult_CPSCE",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
    ], "ReDMCSB F0366 blocked forward collision returns before accepted move/timing")
    f0325_effects = order(f0325, [
        "L0989_ps_Champion->CurrentStamina -= P0673_i_Decrement",
        "L0989_ps_Champion->CurrentStamina = 0;",
        "F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage",
        "L0989_ps_Champion->CurrentStamina = L0989_ps_Champion->MaximumStamina;",
        "M008_SET(L0989_ps_Champion->Attributes, MASK0x0200_LOAD | MASK0x0100_STATISTICS);",
    ], "ReDMCSB F0325 stamina and non-stamina side effects")

    fire_qproc = order(qproc, [
        "result.command = queue->commands[0].command;",
        "result.movementDisabledGate = 1;",
        "process_pending_click(queue);",
        "return result;",
        "queue->count--;",
        "result.dequeued = 1;",
    ], "Firestaff queue gate keeps movement queued until cooldown clears")
    fire_qdiscard = order(qdiscard, [
        "queue->locked = 1;",
        "if (is_reserved_release_command(queue->commands[readIndex].command))",
        "queue->count = writeIndex;",
        "queue->locked = 0;",
        "process_pending_click(queue);",
    ], "Firestaff blocked discard preserves reserved releases and replays pending")
    fire_core = order(core, [
        "outResult->sourceMapX = party->mapX;",
        "outResult->sourceMapY = party->mapY;",
        "outResult->queue = DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
        "dm1_v1_apply_pre_step_stamina_cost(party, outResult);",
        "if (!F0702_MOVEMENT_TryMove_Compat(dungeon, party, action, &outResult->movement)) {",
        "outResult->movementBlocked = 1;",
        "outResult->inputDiscardRequested = 1;",
        "outResult->blockedMovementVblankWaitRequested = 1;",
        "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);",
        "return 1;",
        "party->mapX = outResult->movement.newMapX;",
        "outResult->timing = DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat",
    ], "Firestaff blocked forward returns before party position commit and successful timing")
    fire_stamina = order(core_c, [
        "cost = dm1_v1_compute_step_stamina_cost(champion);",
        "champion->stamina.current = 0;",
        "outResult->staminaDamage[i] = damage;",
        "champion->hp.current = (champion->hp.current > damage)",
        "champion->stamina.current = champion->stamina.maximum;",
    ], "Firestaff stamina side-effect guard")
    fire_timing = order(timing, [
        "result.disabledMovementTicks = DM1_V1_MovementTiming_ComputePartyStepTicksPc34Compat",
        "result.projectileDisabledMovementTicks = 0;",
        "result.lastPartyMovementTime = previousLastPartyMovementTime;",
        "result.scentRecorded = 1;",
        "result.lastPartyMovementTime = currentGameTick;",
    ], "Firestaff successful movement timing path")

    for needle in [
        "COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC",
        "CLIKMENU.C:180-347 F0366",
    ]:
        require(core_h, needle, f"public source evidence {needle}")

    for needle in [
        "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:237-255",
        "CHAMPION.C:F0325_CHAMPION_DecrementStamina:2025-2048",
        "269-323 relative step/block/self-damage-and-wounds request/discard/group-adjacent reaction/one PC-34 blocked-movement VBlank/keep input wait armed",
    ]:
        require(core_c, needle, f"runtime source evidence {needle}")

    for label in [
        "pc34 core disabled gate keeps command queued",
        "pc34 core projectile gate keeps command queued",
        "pc34 core cooldown expiry applies step",
        "pc34 core blocked up arrow dequeued",
        "pc34 core blocked up arrow reports wall",
        "pc34 core blocked up arrow discards followup",
        "pc34 core blocked up arrow keeps y",
        "blocked movement leaves x",
        "blocked movement leaves y",
        "blocked movement still decrements stamina",
        "blocked command does not release wait",
        "blocked command does not redraw viewport",
        "blocked movement flushes queued input",
        "stamina underflow records damage",
        "stamina underflow applies damage",
        "dead champion skipped by stamina window",
        "stamina clamps down to maximum after decrement",
        "pass544 blocked collision no successful step cooldown",
        "pass544 blocked collision keeps input wait armed",
    ]:
        require(core_test, label, f"movement command core runtime assertion {label}")

    for label in [
        "projectile cooldown leaves matching movement queued",
        "expired projectile cooldown releases queued movement",
        "released queued movement dequeued",
    ]:
        require(timing_test, label, f"timing runtime assertion {label}")

    core_exe = find_exe("test_dm1_v1_movement_command_core_pc34_compat")
    timing_exe = find_exe("test_dm1_v1_movement_timing_pc34_compat")
    core_out = run([str(core_exe)])
    timing_out = run([str(timing_exe)])
    if "dm1V1MovementCommandCoreInvariantOk=1" not in core_out:
        raise AssertionError("command core invariant did not pass")
    if "dm1V1MovementTimingInvariantOk=1" not in timing_out:
        raise AssertionError("movement timing invariant did not pass")

    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS,
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "branch": git("branch", "--show-current"),
        "head": git("rev-parse", "HEAD"),
        "worktree": str(ROOT),
        "scope": "DM1 V1 movement pipeline only: queued forward command dispatch, cooldown gating, blocked-square behavior, stamina and non-stamina stamina side effects, and unchanged party position on blocked movement.",
        "primarySourceAudit": {
            "GAMELOOP.C:cooldownBeforeInputAndQueue": f"GAMELOOP.C:{span(1, gameloop, gameloop_tick)}",
            "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": f"COMMAND.C:{f0380_start}-{f0380_end}",
            "COMMAND.C:queuedDispatchCooldownGate": f"COMMAND.C:{span(f0380_start, f0380, f0380_gate)}",
            "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": f"CLIKMENU.C:{f0366_start}-{f0366_end}",
            "CLIKMENU.C:forwardCollisionTiming": f"CLIKMENU.C:{span(f0366_start, f0366, f0366_forward_collision)}",
            "CHAMPION.C:F0325_CHAMPION_DecrementStamina": f"CHAMPION.C:{f0325_start}-{f0325_end}",
            "CHAMPION.C:staminaAndNonStaminaEffects": f"CHAMPION.C:{span(f0325_start, f0325, f0325_effects)}",
        },
        "firestaffGuards": {
            "queueProcessOne": f"dm1_v1_input_command_queue_pc34_compat.c:{qproc_start}-{qproc_end}",
            "queueGateSpan": f"dm1_v1_input_command_queue_pc34_compat.c:{span(qproc_start, qproc, fire_qproc)}",
            "queueDiscard": f"dm1_v1_input_command_queue_pc34_compat.c:{qdiscard_start}-{qdiscard_end}",
            "queueDiscardSpan": f"dm1_v1_input_command_queue_pc34_compat.c:{span(qdiscard_start, qdiscard, fire_qdiscard)}",
            "movementCommandCore": f"dm1_v1_movement_command_core_pc34_compat.c:{core_start}-{core_end}",
            "blockedBeforePartyCommit": f"dm1_v1_movement_command_core_pc34_compat.c:{span(core_start, core, fire_core)}",
            "staminaSideEffects": f"dm1_v1_movement_command_core_pc34_compat.c:{span(1, core_c, fire_stamina)}",
            "successfulMovementTiming": f"dm1_v1_movement_timing_pc34_compat.c:{timing_start}-{timing_end}",
            "successfulMovementTimingSpan": f"dm1_v1_movement_timing_pc34_compat.c:{span(timing_start, timing, fire_timing)}",
        },
        "runtimeGates": {
            str(core_exe.relative_to(ROOT)): core_out.splitlines()[-1],
            str(timing_exe.relative_to(ROOT)): timing_out.splitlines()[-1],
        },
        "notClaimed": [
            "new original DOSBox capture",
            "viewport/touch/rendering parity",
            "combat RNG or final wound materialization beyond the F0325/F0366 request seams",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    report = [
        "# Pass580 - DM1 V1 forward collision/timing source lock",
        "",
        f"Status: {STATUS}",
        "",
        "## ReDMCSB audit anchors",
    ]
    for label, citation in manifest["primarySourceAudit"].items():
        report.append(f"- {citation} - {label}")
    report += ["", "## Firestaff guards"]
    for label, citation in manifest["firestaffGuards"].items():
        report.append(f"- {citation} - {label}")
    report += ["", "## Verification"]
    for command_name, last_line in manifest["runtimeGates"].items():
        report.append(f"- {command_name}: {last_line}")
    report += ["", "## Scope guard"]
    report += [f"- {item}" for item in manifest["notClaimed"]]
    REPORT.write_text("\n".join(report) + "\n", encoding="utf-8")

    print(f"{STATUS} manifest={MANIFEST.relative_to(ROOT)} report={REPORT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, subprocess.SubprocessError) as exc:
        print(f"FAIL {PASS}: {exc}", file=sys.stderr)
        raise SystemExit(1)
