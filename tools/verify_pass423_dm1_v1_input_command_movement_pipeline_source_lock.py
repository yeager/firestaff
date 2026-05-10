#!/usr/bin/env python3
"""Pass423: DM1 V1 input -> command -> movement pipeline source-lock gate.

This verifier audits ReDMCSB WIP20210206 source anchors for the PC-34 route
from raw input through command enqueue/dequeue into turn/move dispatch, then
checks Firestaff's compat seams and regressions cover the same route. It does
not claim original DOSBox runtime or pixel parity.
"""
from __future__ import annotations

from datetime import datetime, timezone
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass423_dm1_v1_input_command_movement_pipeline_source_lock"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
BUILD = ROOT / "build"

SOURCE_RANGES = [
    {"id": "raw_pc34_keyboard_read", "file": "IO2.C", "lines": "27-61", "claim": "PC-34 reads IODRV keyboard input and normalizes shifted extended arrows to command-table codes K/L/M/P.", "needles": ["IODRV_00_GetKeyboardInput", "MEDIA707_I34E_I34M", "switch (L2944_ui_ - 0x1248)", "L2944_ui_ = 'L'", "L2944_ui_ = 'P'", "L2944_ui_ = 'K'", "L2944_ui_ = 'M'", "return L2944_ui_"]},
    {"id": "pc34_keyboard_rows", "file": "COMMAND.C", "lines": "636-685", "claim": "PC-34 movement keyboard table maps normalized codes to C001/C002 turn and C003..C006 move commands.", "needles": ["G0459_as_Graphic561_SecondaryKeyboardInput_Movement", "MEDIA707_I34E_I34M", "C001_COMMAND_TURN_LEFT,     0x004B", "C003_COMMAND_MOVE_FORWARD,  0x004C", "C002_COMMAND_TURN_RIGHT,    0x004D", "C006_COMMAND_MOVE_LEFT,     0x004F", "C005_COMMAND_MOVE_BACKWARD, 0x0050", "C004_COMMAND_MOVE_RIGHT,    0x0051"]},
    {"id": "movement_mouse_rows", "file": "COMMAND.C", "lines": "106-121", "claim": "Movement-panel mouse rows map arrow boxes and viewport/right-click routes to movement/view/inventory commands.", "needles": ["G0448_as_Graphic561_SecondaryMouseInput_Movement", "C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C006_COMMAND_MOVE_LEFT", "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "C083_COMMAND_TOGGLE_INVENTORY_LEADER"]},
    {"id": "keyboard_enqueue", "file": "COMMAND.C", "lines": "1709-1813", "claim": "F0361 locks the queue, searches primary then secondary keyboard tables, writes a matched command, unlocks, then replays pending click.", "needles": ["F0361_COMMAND_ProcessKeyPress", "G0435_B_CommandQueueLocked = C1_TRUE", "G0443_ps_PrimaryKeyboardInput", "while (L1111_i_Command = L1112_ps_KeyboardInput->Command)", "P0728_KeyCode == L1112_ps_KeyboardInput->Code", "G0444_ps_SecondaryKeyboardInput", "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command", "G0435_B_CommandQueueLocked = C0_FALSE", "F0360_COMMAND_ProcessPendingClick();"]},
    {"id": "mouse_enqueue_pending", "file": "COMMAND.C", "lines": "1452-1662", "claim": "F0359 either records one pending click while locked or resolves primary/secondary mouse rows into a queued command.", "needles": ["F0359_COMMAND_ProcessClick_CPSC", "G0436_B_PendingClickPresent = C1_TRUE", "G0437_i_PendingClickX = P0725_i_X", "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput", "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput", "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command", "G0435_B_CommandQueueLocked = C0_FALSE"]},
    {"id": "dequeue_gate_dispatch", "file": "COMMAND.C", "lines": "2045-2156", "claim": "F0380 locks, tests empty/movement-disabled state before dequeue, then unlocks/replays and dispatches turn/move commands.", "needles": ["F0380_COMMAND_ProcessQueue_CPSC", "G0435_B_CommandQueueLocked = C1_TRUE", "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command", "G0310_i_DisabledMovementTicks", "G0311_i_ProjectileDisabledMovementTicks", "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X", "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)", "F0360_COMMAND_ProcessPendingClick();", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command)", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command)"]},
    {"id": "turn_handler", "file": "CLIKMENU.C", "lines": "142-174", "claim": "F0365 sets the input wait stop flag and changes party direction through the source sensor boundary.", "needles": ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE", "F0364_COMMAND_TakeStairs", "F0276_SENSOR_ProcessThingAdditionOrRemoval", "F0284_CHAMPION_SetPartyDirection"]},
    {"id": "move_handler", "file": "CLIKMENU.C", "lines": "180-347", "claim": "F0366 computes relative destination, blocks walls/doors/fakewalls/groups before side effects, discards input on block, and calls F0267 on success.", "needles": ["F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0465_ai_Graphic561_MovementArrowToStepForwardCount", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "L1117_B_MovementBlocked", "F0209_GROUP_ProcessEvents29to41", "F0357_COMMAND_DiscardAllInput();", "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;", "G0311_i_ProjectileDisabledMovementTicks = 0;"]},
    {"id": "game_loop_input_window", "file": "GAMELOOP.C", "lines": "150-219", "claim": "Main loop ages movement cooldowns before polling keyboard and processing exactly the queued command path.", "needles": ["G0310_i_DisabledMovementTicks--", "G0311_i_ProjectileDisabledMovementTicks--", "while (M527_IsCharacterInKeyboardBuffer())", "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer())", "F0380_COMMAND_ProcessQueue_CPSC();"]},
]

ORDER_CHECKS = [
    {"file": "COMMAND.C", "lines": "2045-2156", "claim": "F0380 movement gate happens before X/Y read and queue index advance, so a gated step remains queued.", "needles": ["G0435_B_CommandQueueLocked = C1_TRUE;", "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;", "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)", "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;", "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)", "G0435_B_CommandQueueLocked = C0_FALSE;", "F0360_COMMAND_ProcessPendingClick();", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"]},
    {"file": "CLIKMENU.C", "lines": "180-347", "claim": "F0366 blocked route returns before successful F0267/timing route.", "needles": ["F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "if (L1117_B_MovementBlocked)", "F0357_COMMAND_DiscardAllInput();", "G0321_B_StopWaitingForPlayerInput = C0_FALSE;", "return;", "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;"]},
]

FIRESTAFF_EVIDENCE = [
    {"file": "dm1_v1_input_command_queue_pc34_compat.c", "claim": "compat queue models PC-34 key normalization rows, mouse rows, lock/pending replay, movement gate retention, dequeue, and turn/move dispatch flags", "needles": ["IO2.C:27-61 F0540_INPUT_Crawcin", "case 0x004B:", "case 0x004C:", "case 0x0051:", "command_for_secondary_mouse", "queue->locked = 1;", "result.movementDisabledGate = 1;", "process_pending_click(queue);", "result.dispatchedTurn = 1;", "result.dispatchedMove = 1;"]},
    {"file": "dm1_v1_movement_command_core_pc34_compat.c", "claim": "command core consumes queue results into turn/move handling, blocks before successful movement side effects, and requests input discard/redraw at the F0366 boundary", "needles": ["DM1_V1_InputCommandQueue_ProcessOnePc34Compat", "dm1_v1_is_turn_command(outResult->queue.command)", "F0702_MOVEMENT_TryMove_Compat", "outResult->groupReactionPartyAdjacentRequested = 1;", "outResult->inputDiscardRequested = 1;", "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);", "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat", "outResult->viewportRedrawRequested = 1;"]},
    {"file": "test_dm1_v1_input_command_queue_pc34_compat.c", "claim": "regression covers PC-34 K/L/M/O/P/Q rows, IO2 shifted arrows, pending replay, movement-gate retention, and reserved queue slots", "needles": ["pc34 table up arrow moves forward", "pc34 io2 shifted up arrow normalizes to forward", "pc34 shifted backward arrow strafes right", "locked mouse becomes pending", "movement gate set", "movement not dequeued", "turn bypasses movement disabled gate", "redmcsb sixth command is dropped at C5 limit", "third reserved release is dropped at C7 limit"]},
    {"file": "test_dm1_v1_movement_command_core_pc34_compat.c", "claim": "focused command-core regression proves PC-34 queue output reaches turn, successful movement, cooldown clearing, and blocked-step queue discard", "needles": ["pc34 core left arrow dispatches turn", "pc34 core left arrow turn bypasses movement gate", "pc34 core up arrow dispatches move", "pc34 core up arrow applies step", "pc34 core up arrow clears projectile cooldown", "pc34 core blocked up arrow discards followup"]},
]

TEST_COMMANDS = [
    ["cmake", "--build", str(BUILD), "--target", "test_dm1_v1_input_command_queue_pc34_compat", "test_dm1_v1_movement_command_core_pc34_compat", "test_dm1_v1_command_movement_sensor_timing_pc34_compat", "-j2"],
    ["ctest", "--test-dir", str(BUILD), "--output-on-failure", "-R", "dm1_v1_input_command_queue_pc34_compat|dm1_v1_movement_command_core_pc34_compat|dm1_v1_command_movement_sensor_timing_pc34_compat"],
    ["git", "diff", "--check"],
]


def compact(text: str) -> str:
    return " ".join(text.split())


def parse_range(spec: str) -> tuple[int, int]:
    lo, hi = spec.split("-", 1)
    return int(lo), int(hi)


def source_block(file_name: str, spec: str) -> str:
    path = RED / file_name
    if not path.exists():
        raise AssertionError(f"missing ReDMCSB source file: {path}")
    lo, hi = parse_range(spec)
    lines = path.read_text(encoding="latin-1").splitlines()
    if lo < 1 or hi > len(lines):
        raise AssertionError(f"{file_name}:{spec} outside file length {len(lines)}")
    return "\n".join(lines[lo - 1:hi])


def require_needles(label: str, text: str, needles: list[str]) -> None:
    flat = compact(text)
    for needle in needles:
        if compact(needle) not in flat:
            raise AssertionError(f"{label} missing {needle!r}")


def verify_source_range(entry: dict) -> dict:
    require_needles(f"{entry['file']}:{entry['lines']}", source_block(entry["file"], entry["lines"]), entry["needles"])
    return {"id": entry["id"], "file": entry["file"], "lines": entry["lines"], "claim": entry["claim"]}


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
    return {"file": entry["file"], "lines": entry["lines"], "claim": entry["claim"]}


def verify_firestaff(entry: dict) -> dict:
    path = ROOT / entry["file"]
    if not path.exists():
        raise AssertionError(f"missing Firestaff file: {path}")
    require_needles(entry["file"], path.read_text(encoding="utf-8"), entry["needles"])
    return {"file": entry["file"], "claim": entry["claim"]}


def run(cmd: list[str], timeout: int = 900) -> dict:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    if proc.returncode != 0:
        raise AssertionError(f"command failed {' '.join(cmd)}:\n{proc.stdout[-4000:]}")
    return {"command": cmd, "returncode": proc.returncode, "tail": proc.stdout[-1200:]}


def git(cmd: list[str]) -> str:
    return subprocess.check_output(cmd, cwd=ROOT, text=True).strip()


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    source = [verify_source_range(entry) for entry in SOURCE_RANGES]
    order = [verify_order(entry) for entry in ORDER_CHECKS]
    firestaff = [verify_firestaff(entry) for entry in FIRESTAFF_EVIDENCE]
    tests = [run(cmd) for cmd in TEST_COMMANDS]
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": "PASS423_DM1_V1_INPUT_COMMAND_MOVEMENT_PIPELINE_SOURCE_LOCKED",
        "branch": git(["git", "branch", "--show-current"]),
        "head": git(["git", "rev-parse", "HEAD"]),
        "redmcsbRoot": str(RED),
        "scope": "PC-34 raw input -> command enqueue -> F0380 gate/dequeue -> F0365/F0366 turn/move dispatch",
        "sourceAudit": source,
        "orderChecks": order,
        "firestaffEvidence": firestaff,
        "tests": tests,
        "notClaimed": ["new original DOSBox runtime hit", "pixel parity", "full gameplay parity beyond this input-command-movement seam"],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    lines = ["# Pass423 DM1 V1 input → command → movement pipeline source lock", "", f"Status: **{manifest['status']}**", "", f"Scope: {manifest['scope']}.", "", "## ReDMCSB citations", ""]
    lines += [f"- `{item['file']}:{item['lines']}` — {item['claim']}" for item in source]
    lines += ["", "## Order checks", ""]
    lines += [f"- `{item['file']}:{item['lines']}` — {item['claim']}" for item in order]
    lines += ["", "## Firestaff evidence", ""]
    lines += [f"- `{item['file']}` — {item['claim']}" for item in firestaff]
    lines += ["", "## Gates", ""]
    lines += [f"- `{' '.join(item['command'])}` — rc {item['returncode']}" for item in tests]
    lines += ["", "## Not claimed", ""]
    lines += [f"- {item}" for item in manifest["notClaimed"]]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"{manifest['status']} manifest={MANIFEST}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
