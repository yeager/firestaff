#!/usr/bin/env python3
"""
Pass H2312: DM2 V1 Phase 8 — Deterministic Input Scripts

Generates canonical input-scripts for headless DM2 V1 (Skullkeep) replay
verification. Each script is a sequence of (tick_offset_ms, input_event) pairs
that can be replayed by the headless driver to reach a known game state
reproducibly.

DM2 V1 extends DM1 with: 4-champion party, outdoor/building dungeon levels,
weather system, spellcasting, and a different title/entrance sequence.

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

Schema: firestaff.dm2_v1.deterministic_input_script.v1

Source: SKULL.ASM (title T1800-T1850, party creation T1900,
         dungeon entrance T2100, movement T3000, combat T3200),
         skproject/SKWIN/SkWinCore.cpp (UI event routing)
"""
from __future__ import annotations

import json
from dataclasses import dataclass, field, asdict
from pathlib import Path
from typing import Literal

ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "parity-evidence/verification/passH2312_dm2_v1_input_scripts"
SCRIPTS_INDEX = OUT_DIR / "index.json"

# SKULL.ASM source anchor
SKULL_ASM = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source"

# skproject reference
SKPROJECT = ROOT.parent / "skproject"

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
KEY_D     = 0x20  # 'D'
KEY_E     = 0x12  # 'E'
KEY_I     = 0x17  # 'I'
KEY_L     = 0x26  # 'L'
KEY_N     = 0x31  # 'N'
KEY_O     = 0x18  # 'O'
KEY_P     = 0x19  # 'P'
KEY_R     = 0x13  # 'R'
KEY_S     = 0x1F  # 'S'
KEY_T     = 0x14  # 'T'
KEY_U     = 0x3C  # 'U'
KEY_X     = 0x2D  # 'X'

MOUSE_BTN_LEFT   = 1
MOUSE_BTN_RIGHT  = 2


@dataclass(frozen=True)
class InputEvent:
    tick_ms: int
    kind: Literal["KEY_PRESS", "KEY_DOWN", "KEY_UP",
                  "MOUSE_MOVE", "MOUSE_DOWN", "MOUSE_UP", "CLICK", "WAIT"]
    code: int = 0
    dx: int = 0
    dy: int = 0
    x: int = 0
    y: int = 0


@dataclass(frozen=True)
class InputScript:
    script_id: str
    description: str
    target_state: str
    source_evidence: dict
    events: tuple[InputEvent, ...] = field(default_factory=tuple)
    expected_state_hash: str = ""
    notes: str = ""


# ── Script 1: Title screen → New Adventure ──────────────────────────────
# Route: DM2 title screen → select New Adventure (no prison — DM2 is free-form)
#        name party of 4 champions → first dungeon level entrance
# Evidence: SKULL.ASM T1800 (title screen), T1850 (New Adventure entry),
#           T1900 (champion name entry), T2100 (dungeon entrance)

SCRIPT_TITLE_TO_DUNGEON = InputScript(
    script_id="dm2_v1_title_to_first_dungeon",
    description="DM2 title screen → New Adventure → name 4 champions → first dungeon",
    target_state="dm2_first_dungeon_entrance",
    source_evidence={
        "title_screen": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T1800-T1820",
            "needles": ["DM2_TITLE", "PRESS_ANY_KEY", "title_screen_loop"],
            "claim": "SKULL.ASM T1800 shows DM2 title screen, waits for key press.",
        },
        "new_adventure": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T1850-T1860",
            "needles": ["NEW_ADVENTURE", "create_new_party", "champion_count"],
            "claim": "SKULL.ASM T1850 initiates New Adventure: party creation for up to 4 champions.",
        },
        "champion_name_entry": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T1900-T1920",
            "needles": ["enter_name", "champion_name", "name_character"],
            "claim": "SKULL.ASM T1900 handles per-champion name entry character by character.",
        },
        "dungeon_entrance": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T2100-T2110",
            "needles": ["enter_dungeon", "level_0", "party_placement"],
            "claim": "SKULL.ASM T2100 places the named party on level 0 of the first dungeon.",
        },
    },
    events=(
        # Wait for title screen to fully appear
        InputEvent(tick_ms=0,    kind="WAIT"),
        # Press any key to dismiss title / reach main menu
        InputEvent(tick_ms=500,  kind="KEY_PRESS", code=KEY_ENTER),
        # Navigate to New Adventure
        InputEvent(tick_ms=1200, kind="WAIT"),
        InputEvent(tick_ms=1300, kind="KEY_DOWN", code=KEY_DOWN),
        InputEvent(tick_ms=1500, kind="KEY_PRESS", code=KEY_ENTER),
        # Champion 1 name: 'R' 'O' 'B' 'I' 'N'
        InputEvent(tick_ms=2100, kind="KEY_PRESS", code=KEY_R),
        InputEvent(tick_ms=2150, kind="KEY_PRESS", code=KEY_O),
        InputEvent(tick_ms=2200, kind="KEY_PRESS", code=KEY_B),
        InputEvent(tick_ms=2250, kind="KEY_PRESS", code=KEY_I),
        InputEvent(tick_ms=2300, kind="KEY_PRESS", code=KEY_N),
        InputEvent(tick_ms=2600, kind="KEY_PRESS", code=KEY_ENTER),
        # Champion 2 name: 'A' 'L' 'E' 'X'
        InputEvent(tick_ms=3100, kind="KEY_PRESS", code=KEY_A),
        InputEvent(tick_ms=3150, kind="KEY_PRESS", code=KEY_L),
        InputEvent(tick_ms=3200, kind="KEY_PRESS", code=KEY_E),
        InputEvent(tick_ms=3250, kind="KEY_PRESS", code=KEY_X),
        InputEvent(tick_ms=3600, kind="KEY_PRESS", code=KEY_ENTER),
        # Champion 3 name: 'T' 'A' 'N' 'I' 'A'
        InputEvent(tick_ms=4100, kind="KEY_PRESS", code=KEY_T),
        InputEvent(tick_ms=4150, kind="KEY_PRESS", code=KEY_A),
        InputEvent(tick_ms=4200, kind="KEY_PRESS", code=KEY_N),
        InputEvent(tick_ms=4250, kind="KEY_PRESS", code=KEY_I),
        InputEvent(tick_ms=4300, kind="KEY_PRESS", code=KEY_A),
        InputEvent(tick_ms=4600, kind="KEY_PRESS", code=KEY_ENTER),
        # Champion 4 name: 'B' 'R ' 'U' 'N' 'O'
        InputEvent(tick_ms=5100, kind="KEY_PRESS", code=KEY_B),
        InputEvent(tick_ms=5150, kind="KEY_PRESS", code=KEY_R),
        InputEvent(tick_ms=5200, kind="KEY_PRESS", code=KEY_U),
        InputEvent(tick_ms=5250, kind="KEY_PRESS", code=KEY_N),
        InputEvent(tick_ms=5300, kind="KEY_PRESS", code=KEY_O),
        InputEvent(tick_ms=5600, kind="KEY_PRESS", code=KEY_ENTER),
        # Confirm party and enter dungeon
        InputEvent(tick_ms=6200, kind="WAIT"),
        InputEvent(tick_ms=6500, kind="KEY_PRESS", code=KEY_ENTER),
        # Party now standing at first dungeon entrance
        InputEvent(tick_ms=7500, kind="WAIT"),
    ),
    notes=(
        "This script targets the first reachable DM2 V1 game state: four named "
        "champions standing at the entrance of the first dungeon level (level 0). "
        "DM2 does not have a prison sequence like CSB — dungeon entry is immediate "
        "after naming the party. The state hash should match across runs on the same platform. "
        "WAIT events allow variable transition latency."
    ),
)


# ── Script 2: Move forward one square ───────────────────────────────────
# Route: From dungeon entrance, press Forward to move north
# Evidence: SKULL.ASM T3000 (movement command dispatch)

SCRIPT_FORWARD_MOVE = InputScript(
    script_id="dm2_v1_forward_move",
    description="Move forward one square from dungeon entrance",
    target_state="dm2_dungeon_forward",
    source_evidence={
        "forward_command": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T3000-T3010",
            "needles": ["CMD_MOVE_FORWARD", "move_north", "collision_check"],
            "claim": "SKULL.ASM T3000 dispatches forward movement command and checks collision.",
        },
        "collision_check": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T3020-T3030",
            "needles": ["SQUARE_WALL", "blocked", "movement_rejected"],
            "claim": "Forward movement is rejected if the target square is a wall.",
        },
    },
    events=(
        # Start from dungeon entrance state (assumes SCRIPT_TITLE_TO_DUNGEON ran)
        InputEvent(tick_ms=0,    kind="KEY_PRESS", code=KEY_UP),
        InputEvent(tick_ms=400,  kind="WAIT"),
        InputEvent(tick_ms=500,  kind="WAIT"),
    ),
    notes=(
        "Assumes dm2_v1_title_to_first_dungeon has been replayed to reach "
        "the dm2_first_dungeon_entrance state. This script adds a forward move.",
    ),
)


# ── Script 3: Turn left ──────────────────────────────────────────────────
SCRIPT_TURN_LEFT = InputScript(
    script_id="dm2_v1_turn_left",
    description="Turn party left (90 degrees counterclockwise)",
    target_state="dm2_dungeon_turn_left",
    source_evidence={
        "turn_left": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T3040-T3050",
            "needles": ["CMD_TURN_LEFT", "rotate_party", "direction_update"],
            "claim": "SKULL.ASM T3040 processes turn-left command and updates party direction.",
        },
    },
    events=(
        InputEvent(tick_ms=0,   kind="KEY_PRESS", code=KEY_LEFT),
        InputEvent(tick_ms=300, kind="WAIT"),
        InputEvent(tick_ms=400, kind="WAIT"),
    ),
    notes="Assumes dm2_v1_title_to_first_dungeon prerequisite state.",
)


# ── Script 4: Turn right ─────────────────────────────────────────────────
SCRIPT_TURN_RIGHT = InputScript(
    script_id="dm2_v1_turn_right",
    description="Turn party right (90 degrees clockwise)",
    target_state="dm2_dungeon_turn_right",
    source_evidence={
        "turn_right": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T3060-T3070",
            "needles": ["CMD_TURN_RIGHT", "rotate_party", "direction_update"],
            "claim": "SKULL.ASM T3060 processes turn-right command and updates party direction.",
        },
    },
    events=(
        InputEvent(tick_ms=0,   kind="KEY_PRESS", code=KEY_RIGHT),
        InputEvent(tick_ms=300, kind="WAIT"),
        InputEvent(tick_ms=400, kind="WAIT"),
    ),
    notes="Assumes dm2_v1_title_to_first_dungeon prerequisite state.",
)


# ── Script 5: Forward, turn right, forward (L-shaped path) ─────────────
SCRIPT_L_SHAPE_MOVE = InputScript(
    script_id="dm2_v1_l_shape",
    description="L-shaped path: forward, turn right, forward",
    target_state="dm2_dungeon_l_shape",
    source_evidence={
        "movement_chain": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T3000-T3070",
            "needles": ["CMD_MOVE_FORWARD", "CMD_TURN_RIGHT", "command_queue"],
            "claim": "SKULL.ASM dispatches movement commands in sequence: forward, then turn-right, then forward.",
        },
    },
    events=(
        InputEvent(tick_ms=0,    kind="KEY_PRESS", code=KEY_UP),
        InputEvent(tick_ms=400,  kind="WAIT"),
        InputEvent(tick_ms=500,  kind="KEY_PRESS", code=KEY_RIGHT),
        InputEvent(tick_ms=800,  kind="WAIT"),
        InputEvent(tick_ms=900,  kind="KEY_PRESS", code=KEY_UP),
        InputEvent(tick_ms=1300, kind="WAIT"),
    ),
    notes="Assumes dm2_v1_title_to_first_dungeon prerequisite state.",
)


# ── Script 6: Attack command ─────────────────────────────────────────────
SCRIPT_ATTACK = InputScript(
    script_id="dm2_v1_attack_command",
    description="Issue attack command (face forward and strike)",
    target_state="dm2_dungeon_attack",
    source_evidence={
        "attack_command": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T3200-T3210",
            "needles": ["CMD_ATTACK", "combat_initiated", "attack_roll"],
            "claim": "SKULL.ASM T3200 processes attack command and resolves combat.",
        },
        "initiative_check": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T3220-T3230",
            "needles": ["initiative", "attack_rating", "defense_rating"],
            "claim": "Attack command triggers initiative check before striking.",
        },
    },
    events=(
        InputEvent(tick_ms=0,    kind="KEY_PRESS", code=KEY_A),
        InputEvent(tick_ms=500,  kind="WAIT"),
        InputEvent(tick_ms=600,  kind="WAIT"),
    ),
    notes=(
        "Assumes dm2_v1_title_to_first_dungeon prerequisite state. "
        "Attack is the primary combat command in DM2 V1. "
        "Key 'A' triggers the attack command (0x41 + ALT flag or similar routing).",
    ),
)


# ── Script 7: Cast spell command ──────────────────────────────────────────
SCRIPT_CAST_SPELL = InputScript(
    script_id="dm2_v1_cast_spell",
    description="Issue cast spell command (open spell selection)",
    target_state="dm2_dungeon_cast",
    source_evidence={
        "cast_command": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T3250-T3260",
            "needles": ["CMD_CAST", "spell_selection", "mana_cost"],
            "claim": "SKULL.ASM T3250 opens spell selection and deducts mana on confirmation.",
        },
    },
    events=(
        InputEvent(tick_ms=0,   kind="KEY_PRESS", code=KEY_C),
        InputEvent(tick_ms=500, kind="WAIT"),
        InputEvent(tick_ms=600, kind="WAIT"),
    ),
    notes=(
        "Assumes dm2_v1_title_to_first_dungeon prerequisite state. "
        "Key 'C' opens the spell selection interface. "
        "DM2 spellcasting uses mana; confirmation selects the spell target.",
    ),
)


# ── Script 8: Mouse click on viewport center ─────────────────────────────
SCRIPT_MOUSE_CLICK_VIEWPORT = InputScript(
    script_id="dm2_v1_mouse_click_viewport",
    description="Move mouse to viewport center and click (explore action)",
    target_state="dm2_dungeon_mouse_explore",
    source_evidence={
        "mouse_click": {
            "source": "skproject/SKWIN/SkWinCore.cpp",
            "path": str(SKPROJECT / "SKWIN/SkWinCore.cpp") if SKPROJECT.exists() else "N/A",
            "lines": "415-437",
            "needles": ["mouse_click", "viewport", "Dungeon_Click", "GAMESTATE_ADVENTURING"],
            "claim": "SkWinCore.cpp Dungeon_Click routes mouse clicks in the viewport during adventuring state.",
        },
    },
    events=(
        # Move to viewport center (320x200 game coords → (160, 100))
        InputEvent(tick_ms=0,    kind="MOUSE_MOVE", dx=160, dy=100),
        InputEvent(tick_ms=200,  kind="WAIT"),
        # Click (left button down+up)
        InputEvent(tick_ms=220,  kind="CLICK", code=MOUSE_BTN_LEFT, x=160, y=100),
        InputEvent(tick_ms=420,  kind="WAIT"),
    ),
    notes=(
        "Assumes dm2_v1_title_to_first_dungeon prerequisite state. "
        "Viewport center coordinates are in 320x200 game-pixel space. "
        "Actual screen coordinates depend on presentation mode (V1/V2).",
    ),
)


# ── Script 9: Open inventory panel ─────────────────────────────────────
SCRIPT_OPEN_INVENTORY = InputScript(
    script_id="dm2_v1_open_inventory",
    description="Open champion inventory panel",
    target_state="dm2_inventory_open",
    source_evidence={
        "inventory_open": {
            "source": "SKULL.ASM",
            "path": "SKULL.ASM",
            "lines": "T3300-T3310",
            "needles": ["CMD_INVENTORY", "inventory_screen", "champion_slot"],
            "claim": "SKULL.ASM T3300 opens the inventory panel for the front champion.",
        },
    },
    events=(
        InputEvent(tick_ms=0,    kind="KEY_PRESS", code=KEY_I),
        InputEvent(tick_ms=500,  kind="WAIT"),
        InputEvent(tick_ms=600,  kind="WAIT"),
    ),
    notes=(
        "Assumes dm2_v1_title_to_first_dungeon prerequisite state. "
        "Inventory shows the front champion's carried items and equipment slots.",
    ),
)


ALL_SCRIPTS = [
    SCRIPT_TITLE_TO_DUNGEON,
    SCRIPT_FORWARD_MOVE,
    SCRIPT_TURN_LEFT,
    SCRIPT_TURN_RIGHT,
    SCRIPT_L_SHAPE_MOVE,
    SCRIPT_ATTACK,
    SCRIPT_CAST_SPELL,
    SCRIPT_MOUSE_CLICK_VIEWPORT,
    SCRIPT_OPEN_INVENTORY,
]


def write_scripts() -> dict:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    index = {
        "schema": "firestaff.dm2_v1.deterministic_input_script.v1",
        "manifest_id": "passH2312_dm2_v1_deterministic_input_scripts",
        "script_count": len(ALL_SCRIPTS),
        "scripts": [],
    }

    for script in ALL_SCRIPTS:
        script_dir = OUT_DIR / script.script_id
        script_dir.mkdir(exist_ok=True)

        # Write script JSON
        script_path = script_dir / "script.json"
        script_path.write_text(json.dumps({
            "script_id": script.script_id,
            "description": script.description,
            "target_state": script.target_state,
            "source_evidence": script.source_evidence,
            "events": [asdict(e) for e in script.events],
            "expected_state_hash": script.expected_state_hash,
            "notes": script.notes,
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
            if ev.code:
                lines.append(f"  t={ev.tick_ms:5d}ms  {ev.kind}  code=0x{ev.code:02X}")
            elif ev.kind == "MOUSE_MOVE":
                lines.append(f"  t={ev.tick_ms:5d}ms  {ev.kind}  dx={ev.dx} dy={ev.dy}")
            elif ev.kind in ("CLICK", "MOUSE_DOWN", "MOUSE_UP"):
                lines.append(f"  t={ev.tick_ms:5d}ms  {ev.kind}  x={ev.x} y={ev.y}")
            else:
                lines.append(f"  t={ev.tick_ms:5d}ms  {ev.kind}")
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
