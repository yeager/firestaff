#!/usr/bin/env python3
"""Pass475: DM1 V1 input -> F0365/F0366 runtime follow-up closure.

Closes the Firestaff-side blocker that the live M11 route might bypass the
source-owned COMMAND.C F0380 -> CLIKMENU.C F0365/F0366 boundary.  The gate is
source-first: audit ReDMCSB PC34/I34E input/queue/dispatch anchors from the
local N2 source tree, then prove current Firestaff runtime seams and focused
regressions exercise the same turn/step dispatch.  It does not promote original
DOSBox/FIRES pixel or debugger parity.
"""
from __future__ import annotations

from datetime import datetime, timezone
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass475_dm1_v1_f0365_f0366_runtime_followup"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
BUILD = Path(os.environ.get("FIRESTAFF_BUILD_DIR") or (Path.cwd() if (Path.cwd() / "CMakeCache.txt").exists() else ROOT / "build"))
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"

SOURCE_RANGES = [
    {
        "id": "game_loop_drains_input_then_f0380",
        "file": "GAMELOOP.C",
        "lines": "150-219",
        "claim": "PC34 game loop drains keyboard input into F0361, then processes the command queue with F0380 until the input/tick gate is satisfied.",
        "needles": [
            "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
            "while (M527_IsCharacterInKeyboardBuffer())",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer())",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "if (!G0321_B_StopWaitingForPlayerInput)",
        ],
    },
    {
        "id": "pc34_keyboard_rows_to_movement_commands",
        "file": "COMMAND.C",
        "lines": "636-685",
        "claim": "I34E/I34M secondary keyboard rows map PC34 movement key codes to C001/C002 turns and C003..C006 movement commands.",
        "needles": [
            "G0459_as_Graphic561_SecondaryKeyboardInput_Movement",
            "MEDIA707_I34E_I34M",
            "C001_COMMAND_TURN_LEFT,     0x004B",
            "C003_COMMAND_MOVE_FORWARD,  0x004C",
            "C002_COMMAND_TURN_RIGHT,    0x004D",
            "C006_COMMAND_MOVE_LEFT,     0x004F",
            "C005_COMMAND_MOVE_BACKWARD, 0x0050",
            "C004_COMMAND_MOVE_RIGHT,    0x0051",
        ],
    },
    {
        "id": "f0361_keyboard_enqueue",
        "file": "COMMAND.C",
        "lines": "1709-1813",
        "claim": "F0361 resolves primary/secondary keyboard input, writes the matched command to G0432, unlocks, and replays a pending click.",
        "needles": [
            "F0361_COMMAND_ProcessKeyPress",
            "G0435_B_CommandQueueLocked = C1_TRUE",
            "G0443_ps_PrimaryKeyboardInput",
            "G0444_ps_SecondaryKeyboardInput",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command",
            "G0435_B_CommandQueueLocked = C0_FALSE",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
    },
    {
        "id": "f0380_dequeue_gate_then_dispatch",
        "file": "COMMAND.C",
        "lines": "2045-2156",
        "claim": "F0380 locks the queue, keeps movement queued while cooldown/projectile gates apply, dequeues exactly one command, then dispatches turns to F0365 and steps to F0366.",
        "needles": [
            "F0380_COMMAND_ProcessQueue_CPSC",
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G0310_i_DisabledMovementTicks",
            "G0311_i_ProjectileDisabledMovementTicks",
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
    },
    {
        "id": "f0365_turn_handler",
        "file": "CLIKMENU.C",
        "lines": "142-174",
        "claim": "F0365 is the source turn boundary: stop-wait flag, stair special case, sensor removal/addition, and direction update.",
        "needles": [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0364_COMMAND_TakeStairs",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval",
            "F0284_CHAMPION_SetPartyDirection",
        ],
    },
    {
        "id": "f0366_move_handler",
        "file": "CLIKMENU.C",
        "lines": "180-347",
        "claim": "F0366 is the source step/strafe boundary: stamina cost, relative destination, wall/door/fakewall/group blocking, blocked-input discard, F0267 commit, and cooldown side effects.",
        "needles": [
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0325_CHAMPION_DecrementStamina",
            "G0465_ai_Graphic561_MovementArrowToStepForwardCount",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "L1117_B_MovementBlocked = C0_FALSE;",
            "F0357_COMMAND_DiscardAllInput();",
            "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
            "G0311_i_ProjectileDisabledMovementTicks = 0;",
        ],
    },
]

ORDER_CHECKS = [
    {
        "id": "f0380_gate_before_dequeue_and_dispatch",
        "file": "COMMAND.C",
        "lines": "2045-2156",
        "claim": "The movement-disabled gate is tested before X/Y read and queue index advance; dispatch to F0365/F0366 occurs only after unlock/pending-click replay.",
        "needles": [
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)",
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
    },
    {
        "id": "f0366_blocked_route_returns_before_commit",
        "file": "CLIKMENU.C",
        "lines": "180-347",
        "claim": "F0366 discards input and returns on blocked movement before F0267/cooldown side effects; accepted movement reaches F0267 and then arms cooldown.",
        "needles": [
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "if (L1117_B_MovementBlocked)",
            "F0357_COMMAND_DiscardAllInput();",
            "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
            "return;",
            "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        ],
    },
]

FIRESTAFF_LOCKS = [
    {
        "file": "m11_game_view.c",
        "claim": "product runtime maps live M12 movement inputs to source command ids and feeds the DM1 V1 pipeline instead of moving directly",
        "needles": [
            "static int m11_dm1_v1_pipeline_command_for_input",
            "return DM1_V1_COMMAND_TURN_LEFT;",
            "return DM1_V1_COMMAND_MOVE_FORWARD;",
            "return DM1_V1_COMMAND_TURN_RIGHT;",
            "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat",
            "DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat",
            "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat",
            "lastDm1V1MovementPipelineResult.viewportDirty",
        ],
    },
    {
        "file": "dm1_v1_movement_pipeline_pc34_compat.c",
        "claim": "pipeline API accepts either input events or command ids and processes one F0380/F0365/F0366-compatible tick",
        "needles": [
            "DM1_V1_MovementPipeline_EnqueueInputPc34Compat",
            "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat",
            "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat",
            "DM1_V1_MovementCommandCore_ProcessOnePc34Compat",
            "viewportDirty",
        ],
    },
    {
        "file": "dm1_v1_movement_command_core_pc34_compat.c",
        "claim": "command core consumes the queued command result into turn or step handling, including blocked-input discard and accepted-step side effects",
        "needles": [
            "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
            "dm1_v1_is_turn_command(outResult->queue.command)",
            "F0718_SENSOR_ProcessPartyEnterLeave_Compat",
            "F0702_MOVEMENT_TryMove_Compat",
            "outResult->inputDiscardRequested = 1;",
            "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);",
            "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat",
        ],
    },
    {
        "file": "main_loop_m11.c",
        "claim": "runtime probe exports dequeued command, turn/step flags, movement/turn occurrence, and viewportDirty for live verification",
        "needles": [
            "lastDm1V1MovementPipelineResult.core.queue.dequeued",
            "lastDm1V1MovementPipelineResult.core.queue.command",
            "lastDm1V1MovementPipelineResult.core.turnApplied",
            "lastDm1V1MovementPipelineResult.core.stepApplied",
            "lastDm1V1MovementPipelineResult.anyMovementOccurred",
            "lastDm1V1MovementPipelineResult.anyTurnOccurred",
            "lastDm1V1MovementPipelineResult.viewportDirty",
            "FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON",
        ],
    },
]

TEST_COMMANDS = [
    ["cmake", "--build", str(BUILD), "--target", "test_dm1_v1_input_command_queue_pc34_compat", "test_dm1_v1_movement_command_core_pc34_compat", "test_dm1_v1_movement_pipeline_pc34_compat", "test_dm1_v1_command_movement_sensor_timing_pc34_compat", "-j2"],
    ["ctest", "--test-dir", str(BUILD), "--output-on-failure", "-R", "dm1_v1_input_command_queue_pc34_compat|dm1_v1_movement_command_core_pc34_compat|dm1_v1_movement_pipeline_pc34_compat|dm1_v1_command_movement_sensor_timing_pc34_compat"],
    ["git", "diff", "--check"],
]


def compact(text: str) -> str:
    return " ".join(text.split())


def source_block(file_name: str, line_range: str) -> str:
    lo_s, hi_s = line_range.split("-", 1)
    lo, hi = int(lo_s), int(hi_s)
    path = RED / file_name
    if not path.exists():
        raise AssertionError(f"missing ReDMCSB source: {path}")
    lines = path.read_text(encoding="latin-1").splitlines()
    if lo < 1 or hi > len(lines):
        raise AssertionError(f"{file_name}:{line_range} outside file length {len(lines)}")
    return "\n".join(lines[lo - 1:hi])


def require_needles(label: str, text: str, needles: list[str]) -> None:
    flat = compact(text)
    for needle in needles:
        if compact(needle) not in flat:
            raise AssertionError(f"{label} missing {needle!r}")


def verify_source(entry: dict) -> dict:
    require_needles(f"{entry['file']}:{entry['lines']}", source_block(entry["file"], entry["lines"]), entry["needles"])
    return {k: entry[k] for k in ("id", "file", "lines", "claim")}


def verify_order(entry: dict) -> dict:
    flat = compact(source_block(entry["file"], entry["lines"]))
    pos = -1
    for needle in entry["needles"]:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{entry['file']}:{entry['lines']} order missing {needle!r}")
        if hit <= pos:
            raise AssertionError(f"{entry['file']}:{entry['lines']} order failed at {needle!r}")
        pos = hit
    return {k: entry[k] for k in ("id", "file", "lines", "claim")}


def verify_firestaff(entry: dict) -> dict:
    path = ROOT / entry["file"]
    if not path.exists():
        raise AssertionError(f"missing Firestaff file: {path}")
    require_needles(entry["file"], path.read_text(encoding="utf-8", errors="replace"), entry["needles"])
    return {"file": entry["file"], "claim": entry["claim"]}


def run(cmd: list[str], timeout: int = 900) -> dict:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    if proc.returncode != 0:
        raise AssertionError(f"command failed {' '.join(cmd)}:\n{proc.stdout[-4000:]}")
    return {"command": cmd, "returncode": proc.returncode, "tail": proc.stdout[-1200:]}


def git(args: list[str]) -> str:
    return subprocess.check_output(args, cwd=ROOT, text=True).strip()


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    source = [verify_source(entry) for entry in SOURCE_RANGES]
    order = [verify_order(entry) for entry in ORDER_CHECKS]
    firestaff = [verify_firestaff(entry) for entry in FIRESTAFF_LOCKS]
    tests = [run(cmd) for cmd in TEST_COMMANDS]
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": "PASS475_FIRESTAFF_DM1_V1_INPUT_TO_F0365_F0366_ROUTE_CLOSED",
        "branch": git(["git", "branch", "--show-current"]),
        "head": git(["git", "rev-parse", "HEAD"]),
        "redmcsbRoot": str(RED),
        "closedBlocker": "Firestaff live M11 movement input now has a source-locked and regression-tested route through command ids into the F0380 -> F0365/F0366 compat boundary; no direct-move bypass remains in this seam.",
        "sourceAudit": source,
        "orderChecks": order,
        "firestaffLocks": firestaff,
        "tests": tests,
        "remainingBlocker": "Original stock FIRES keyboard-buffer/debugger hit and semantically party-control-ready original overlay/pixel parity are still unclaimed; they require a separate DOSBox/FIRES capture or symbol-address run, not more Firestaff route plumbing.",
    }
    OUT_JSON.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    lines = [
        "# Pass475 — DM1 V1 input → F0365/F0366 runtime follow-up",
        "",
        f"Status: **{manifest['status']}**",
        "",
        f"Closed blocker: {manifest['closedBlocker']}",
        "",
        "## ReDMCSB source audit",
        "",
    ]
    lines += [f"- `{item['file']}:{item['lines']}` — {item['claim']}" for item in source]
    lines += ["", "## Ordering locks", ""]
    lines += [f"- `{item['file']}:{item['lines']}` — {item['claim']}" for item in order]
    lines += ["", "## Firestaff runtime locks", ""]
    lines += [f"- `{item['file']}` — {item['claim']}" for item in firestaff]
    lines += ["", "## Gates", ""]
    lines += [f"- `{' '.join(item['command'])}` — rc {item['returncode']}" for item in tests]
    lines += ["", "## Still not claimed", "", f"- {manifest['remainingBlocker']}"]
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"{manifest['status']} manifest={OUT_JSON}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
