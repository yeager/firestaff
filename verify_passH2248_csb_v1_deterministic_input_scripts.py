#!/usr/bin/env python3
"""
Pass H2248: CSB V1 Phase 7 — Deterministic Input Scripts

Generates canonical input-scripts for headless CSB V1 replay verification.
Each script is a sequence of (tick_offset_ms, input_event) pairs that can be
replayed by the headless driver to reach a known game state reproducibly.

Input event types:
  KEY_PRESS   — keydown + keyup at same tick
  KEY_DOWN    — keydown only
  KEY_UP      — keyup only
  MOUSE_MOVE  — pointer move
  MOUSE_DOWN  — mouse button press
  MOUSE_UP    — mouse button release
  CLICK       — mouse down+up at same tick (convenience)
  WAIT        — no input, just advance clock

All tick offsets are in milliseconds from script start.
Scripts are named by the game state they navigate to.

Schema: firestaff.csb_v1.deterministic_input_script.v1
"""
from __future__ import annotations

import json
from dataclasses import dataclass, field, asdict
from pathlib import Path
from typing import Literal

ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "parity-evidence/verification/passH2248_csb_v1_input_scripts"
SCRIPTS_INDEX = OUT_DIR / "index.json"

# ReDMCSB source for keyboard/mouse routing evidence
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

# Input event constants (must match m11 input enum in the engine)
KEY_LEFT  = 0xCB  # arrow left
KEY_RIGHT = 0xCD  # arrow right
KEY_UP    = 0xC8  # arrow up
KEY_DOWN  = 0xD0  # arrow down
KEY_ENTER = 0x1C  # enter
KEY_ESC   = 0x01  # escape
KEY_A     = 0x1E  # 'A'
KEY_B     = 0x30  # 'B'
KEY_C     = 0x2E  # 'C'
KEY_E     = 0x12  # 'E'
KEY_I     = 0x17  # 'I'
KEY_L     = 0x26  # 'L'
KEY_O     = 0x18  # 'O'

MOUSE_BTN_LEFT   = 1
MOUSE_BTN_RIGHT  = 2


@dataclass(frozen=True)
class InputEvent:
    tick_ms: int
    kind: Literal["KEY_PRESS", "KEY_DOWN", "KEY_UP",
                  "MOUSE_MOVE", "MOUSE_DOWN", "MOUSE_UP", "CLICK", "WAIT"]
    code: int = 0
    dx: int = 0  # mouse delta x (for MOUSE_MOVE)
    dy: int = 0  # mouse delta y (for MOUSE_MOVE)
    x: int = 0   # mouse x (for CLICK, MOUSE_DOWN, MOUSE_UP)
    y: int = 0   # mouse y (for CLICK, MOUSE_DOWN, MOUSE_UP)


@dataclass(frozen=True)
class InputScript:
    script_id: str
    description: str
    target_state: str
    # Source evidence: what ReDMCSB / CSBWin source validates this input route
    source_evidence: dict
    # Events in tick order
    events: tuple[InputEvent, ...] = field(default_factory=tuple)
    # Expected state hash after script completes (SHA256 of serialized game state)
    expected_state_hash: str = ""
    # Notes
    notes: str = ""


# ── Script 1: Title screen → New Adventure ──────────────────────────────
# Route: M12 launcher selects CSB → CSB title screen
#        Enter → select Make New Adventure → name champions → start
# Evidence: CSBWin Mouse.cpp:1830-1952 (GAMESTATE_EnterPrison / adventuring mode)

SCRIPT_TITLE_TO_ADVENTURE = InputScript(
    script_id="csb_v1_title_to_prison",
    description="CSB title screen → Make New Adventure → prison entrance",
    target_state="csb_prison_entrance",
    source_evidence={
        "title_enter": {
            "source": "ReDMCSB",
            "path": str(REDMCSB / "CEDTINCH.C"),
            "lines": "5-63",
            "needles": ["F7086_IsReadyToMakeNewAdventure", "C13_DUNGEON_CSB_GAME"],
            "claim": "New Adventure readiness requires valid CSB game dungeon source.",
        },
        "adventuring_mode": {
            "source": "CSB lineage",
            "path": "Mouse.cpp",
            "lines": "1830-1952",
            "needles": ["GAMESTATE_EnterPrison", "D6W == 80", "The Viewport while adventuring"],
            "claim": "Adventuring mode is reached after exiting prison entrance.",
        },
        "prison_entrance": {
            "source": "CSB lineage",
            "path": "Chaos.cpp",
            "lines": "507-623",
            "needles": ["CSBGAME", "csbgame.dat", "MINI.DAT"],
            "claim": "Prison entrance is the first dungeon state after Make New Adventure.",
        },
    },
    events=(
        # Enter from title to prison selection
        InputEvent(tick_ms=0,    kind="KEY_PRESS", code=KEY_ENTER),
        # Wait for menu transition (typically 500-1000ms)
        InputEvent(tick_ms=600,  kind="WAIT"),
        # Select first slot (Make New Adventure)
        InputEvent(tick_ms=620,  kind="KEY_PRESS", code=KEY_ENTER),
        # Wait for champion naming screen
        InputEvent(tick_ms=1200, kind="WAIT"),
        # Name first champion: 'A' 'L' 'I' 'C' 'E'
        InputEvent(tick_ms=1300, kind="KEY_PRESS", code=KEY_A),
        InputEvent(tick_ms=1350, kind="KEY_PRESS", code=KEY_L),
        InputEvent(tick_ms=1400, kind="KEY_PRESS", code=KEY_I),
        InputEvent(tick_ms=1450, kind="KEY_PRESS", code=KEY_C),
        InputEvent(tick_ms=1500, kind="KEY_PRESS", code=KEY_E),
        # Confirm name
        InputEvent(tick_ms=1700, kind="KEY_PRESS", code=KEY_ENTER),
        # Name second champion: 'B' 'O' 'B'
        InputEvent(tick_ms=2200, kind="KEY_PRESS", code=KEY_B),
        InputEvent(tick_ms=2250, kind="KEY_PRESS", code=KEY_O),
        InputEvent(tick_ms=2300, kind="KEY_PRESS", code=KEY_B),
        # Confirm name
        InputEvent(tick_ms=2500, kind="KEY_PRESS", code=KEY_ENTER),
        # Wait for dungeon to load and party to appear
        InputEvent(tick_ms=4000, kind="WAIT"),
        # Confirm ready to enter dungeon
        InputEvent(tick_ms=4100, kind="KEY_PRESS", code=KEY_ENTER),
        # Party now in prison entrance (first dungeon state)
        InputEvent(tick_ms=5000, kind="WAIT"),
    ),
    notes=(
        "This script targets the first reachable CSB V1 game state: party standing "
        "in the prison entrance after Make New Adventure. "
        "The state hash should match across runs on the same platform."
        "Timing is approximate; the WAIT events allow variable transition latency."
    ),
)


# ── Script 2: Move forward one square ───────────────────────────────────
# Route: From prison entrance, press Forward to move north
# Evidence: ReDMCSB MOVESENS.C, COMMAND.C movement command dispatch

SCRIPT_FORWARD_MOVE = InputScript(
    script_id="csb_v1_forward_move",
    description="Move forward one square in prison",
    target_state="csb_prison_forward",
    source_evidence={
        "forward_command": {
            "source": "ReDMCSB",
            "path": str(REDMCSB / "COMMAND.C"),
            "lines": "1-60",
            "needles": ["C0x41_CMD_MOVE_FORWARD", "F0201_COMMAND_ProcessMoveForward"],
            "claim": "Forward movement is command 0x41, processed by F0201.",
        },
        "collision_check": {
            "source": "ReDMCSB",
            "path": str(REDMCSB / "MOVESENS.C"),
            "lines": "1-50",
            "needles": ["F0230_MOVE_CheckBlocked", "C0x01_SQUARE_WALL"],
            "claim": "Forward movement checks wall collision before allowing step.",
        },
    },
    events=(
        # Start from prison entrance state (assumes SCRIPT_TITLE_TO_ADVENTURE ran)
        InputEvent(tick_ms=0,    kind="KEY_PRESS", code=KEY_UP),    # forward
        InputEvent(tick_ms=400,  kind="WAIT"),   # wait for movement to complete
        InputEvent(tick_ms=500,  kind="WAIT"),   # extra settle time
    ),
    notes=(
        "Assumes script csb_v1_title_to_prison has been replayed to reach "
        "the prison_entrance state. This script adds a forward move. "
        "The expected state hash should match regardless of whether it is "
        "replayed standalone after the prerequisite or as part of a combined script.",
    ),
)


# ── Script 3: Turn left ──────────────────────────────────────────────────
SCRIPT_TURN_LEFT = InputScript(
    script_id="csb_v1_turn_left",
    description="Turn party left (90 degrees counterclockwise)",
    target_state="csb_prison_turn_left",
    source_evidence={
        "turn_command": {
            "source": "ReDMCSB",
            "path": str(REDMCSB / "COMMAND.C"),
            "lines": "1-60",
            "needles": ["C0x42_CMD_TURN_LEFT", "F0202_COMMAND_ProcessTurnLeft"],
            "claim": "Turn left is command 0x42, processed by F0202.",
        },
    },
    events=(
        InputEvent(tick_ms=0,   kind="KEY_PRESS", code=KEY_LEFT),   # turn left
        InputEvent(tick_ms=300, kind="WAIT"),
        InputEvent(tick_ms=400, kind="WAIT"),
    ),
    notes="Assumes csb_v1_title_to_prison prerequisite state.",
)


# ── Script 4: Turn right ─────────────────────────────────────────────────
SCRIPT_TURN_RIGHT = InputScript(
    script_id="csb_v1_turn_right",
    description="Turn party right (90 degrees clockwise)",
    target_state="csb_prison_turn_right",
    source_evidence={
        "turn_command": {
            "source": "ReDMCSB",
            "path": str(REDMCSB / "COMMAND.C"),
            "lines": "1-60",
            "needles": ["C0x43_CMD_TURN_RIGHT", "F0203_COMMAND_ProcessTurnRight"],
            "claim": "Turn right is command 0x43, processed by F0203.",
        },
    },
    events=(
        InputEvent(tick_ms=0,   kind="KEY_PRESS", code=KEY_RIGHT),  # turn right
        InputEvent(tick_ms=300, kind="WAIT"),
        InputEvent(tick_ms=400, kind="WAIT"),
    ),
    notes="Assumes csb_v1_title_to_prison prerequisite state.",
)


# ── Script 5: Move forward, turn right, move forward ───────────────────
SCRIPT_L_SHAPE_MOVE = InputScript(
    script_id="csb_v1_l_shape",
    description="L-shaped path: forward, turn right, forward",
    target_state="csb_prison_l_shape",
    source_evidence={
        "movement_chain": {
            "source": "ReDMCSB",
            "path": str(REDMCSB / "COMMAND.C"),
            "lines": "1-80",
            "needles": ["C0x41_CMD_MOVE_FORWARD", "C0x43_CMD_TURN_RIGHT"],
            "claim": "Forward and turn-right are sequential commands in a movement chain.",
        },
    },
    events=(
        InputEvent(tick_ms=0,    kind="KEY_PRESS", code=KEY_UP),     # forward
        InputEvent(tick_ms=400,  kind="WAIT"),
        InputEvent(tick_ms=500,  kind="KEY_PRESS", code=KEY_RIGHT),  # turn right
        InputEvent(tick_ms=800,  kind="WAIT"),
        InputEvent(tick_ms=900,  kind="KEY_PRESS", code=KEY_UP),     # forward
        InputEvent(tick_ms=1300, kind="WAIT"),
    ),
    notes="Assumes csb_v1_title_to_prison prerequisite state.",
)


# ── Script 6: Click on viewport center (mouse move) ────────────────────
SCRIPT_MOUSE_CLICK_VIEWPORT = InputScript(
    script_id="csb_v1_mouse_click_viewport",
    description="Move mouse to viewport center and click (explore action)",
    target_state="csb_prison_mouse_explore",
    source_evidence={
        "mouse_input": {
            "source": "CSB lineage",
            "path": "Mouse.cpp",
            "lines": "1830-1952",
            "needles": ["D6W == 80", "GAMESTATE_EnterPrison", "Viewport click"],
            "claim": "Mouse clicks in the viewport (D6W==80) trigger explore/proceed actions.",
        },
    },
    events=(
        # Move to viewport center (640x200 game coords → screen coords depend on mode)
        # Using 320x200 viewport center = (160, 100)
        InputEvent(tick_ms=0,    kind="MOUSE_MOVE", dx=160, dy=100),
        InputEvent(tick_ms=200,  kind="WAIT"),
        # Click (left button down+up)
        InputEvent(tick_ms=220,  kind="CLICK", code=MOUSE_BTN_LEFT, x=160, y=100),
        InputEvent(tick_ms=420,  kind="WAIT"),
    ),
    notes=(
        "Assumes csb_v1_title_to_prison prerequisite state. "
        "Viewport center coordinates are in 320x200 game-pixel space. "
        "Actual screen coordinates depend on the presentation mode (V1/V2).",
    ),
)


ALL_SCRIPTS = [
    SCRIPT_TITLE_TO_ADVENTURE,
    SCRIPT_FORWARD_MOVE,
    SCRIPT_TURN_LEFT,
    SCRIPT_TURN_RIGHT,
    SCRIPT_L_SHAPE_MOVE,
    SCRIPT_MOUSE_CLICK_VIEWPORT,
]


def write_scripts() -> dict:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    index = {
        "schema": "firestaff.csb_v1.deterministic_input_script.v1",
        "manifest_id": "passH2248_csb_v1_deterministic_input_scripts",
        "script_count": len(ALL_SCRIPTS),
        "scripts": [],
    }

    for script in ALL_SCRIPTS:
        script_dir = OUT_DIR / script.script_id
        script_dir.mkdir(exist_ok=True)

        # Write script JSON
        script_path = script_dir / "script.json"
        obj = asdict(script)
        # Convert events tuple to list for JSON
        obj["events"] = [asdict(e) for e in script.events]
        obj["events"] = tuple(obj["events"])  # keep as tuple for hash stability
        script_path.write_text(json.dumps({
            **obj,
            "events": [asdict(e) for e in script.events],
        }, indent=2), encoding="utf-8")

        # Write script as plain text for human review
        txt_path = script_dir / "script.txt"
        lines = [
            f"Script: {script.script_id}",
            f"Description: {script.description}",
            f"Target state: {script.target_state}",
            f"Events ({len(script.events)} total):",
            "",
        ]
        for ev in script.events:
            lines.append(f"  t={ev.tick_ms:5d}ms  {ev.kind}  "
                         f"code=0x{ev.code:02X}" if ev.code else
                         f"  t={ev.tick_ms:5d}ms  {ev.kind}")
        lines.append("")
        lines.append(f"Notes: {script.notes}")
        txt_path.write_text("\n".join(lines), encoding="utf-8")

        index["scripts"].append({
            "script_id": script.script_id,
            "path": str(script_path.relative_to(OUT_DIR.parent)),
            "target_state": script.target_state,
        })

    SCRIPTS_INDEX.write_text(json.dumps(index, indent=2), encoding="utf-8")
    return index


def main():
    idx = write_scripts()
    print(f"Wrote {idx['script_count']} input scripts to {OUT_DIR}")
    print(f"Index: {SCRIPTS_INDEX}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())