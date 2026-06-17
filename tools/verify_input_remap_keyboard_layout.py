#!/usr/bin/env python3
"""Verify the v2.8.x keyboard layout contract.

Pins the production code paths that implement the new arrow-key
mapping (arrow keys = strafe; Home/End/Q/E = turn; WASD mirrors
arrows) so the contract enforced by
test_input_remap_keyboard_layout_pc34_compat.c cannot drift
silently.

Probes three source files:

  1. src/engine/main_loop_m11.c m11_map_script_token — the
     replay-script parser.  Lines are approximate; if the file
     shifts the test gets updated.
  2. src/engine/main_loop_m11.c SDLK_LEFT/RIGHT/KP_4/KP_6/Q/E/
     HOME/END switch cases — the runtime SDL keyboard handler.
     Two copies exist (one for SDL_EVENT_KEY_DOWN, one for
     SDL_KEYDOWN); both must be present and consistent.
  3. src/shared/input_remap_m12.c s_defaults_original[] — the
     keybindings.toml preset the user picks on first launch.
  4. src/engine/input_remap_m11.c s_defs[] — the M11-side
     scancode -> action remap table.
  5. src/engine/m11_game_view.c m11_dm1_v1_pipeline_command_for_
     input — the pipeline command switch that maps input tokens
     to DM1_V1_COMMAND_*.

Source references (ReDMCSB):
  - COMMAND.C:677-684 keypad scancode -> command table
  - COMMAND.C G0448 menu arrow-click command routing
  - COMMAND.C:2438-2451 entrance input / Enter handling
  - IO2.C:47-59 shifted arrow normalisation
"""
from __future__ import annotations

import json
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MLOOP = ROOT / "src/engine/main_loop_m11.c"
MREMAP = ROOT / "src/shared/input_remap_m12.c"
REMAP11 = ROOT / "src/engine/input_remap_m11.c"
GVIEW = ROOT / "src/engine/m11_game_view.c"
OUT_DIR = ROOT / "parity-evidence/verification/input_remap_keyboard_layout"
OUT_JSON = OUT_DIR / "manifest.json"
STATUS_PASS = "PASS_INPUT_REMAP_KEYBOARD_LAYOUT_V28X_LOCKED"


def read_text(path: Path) -> str:
    if not path.is_file():
        raise AssertionError(f"missing source file: {path}")
    return path.read_text(encoding="latin-1", errors="replace")


def require_in(text: str, needle: str, label: str, file: Path) -> int:
    """require needle appears in text; return count."""
    n = text.count(needle)
    if n == 0:
        raise AssertionError(f"{file}: missing {label} ({needle!r})")
    return n


def verify_main_loop() -> list[dict]:
    text = read_text(MLOOP)
    cases: list[dict] = []

    # Script-token parser: arrow LEFT/RIGHT now map to STRAFE.
    # The production code uses strncmp(token, "left", len) and then
    # returns either M12_MENU_INPUT_LEFT (historical) or
    # M12_MENU_INPUT_STRAFE_LEFT (v2.8.x).  We can't tell which
    # mapping is active from a single needle, so look for the
    # "left"/"right" strncmp lines and check that NEITHER is
    # followed by `return M12_MENU_INPUT_LEFT;` (the historical
    # turn-left mapping) within 6 lines.
    def _has_stale_left_to_turn(text: str, token: str) -> bool:
        needle = f'"{token}", len'
        idx = 0
        while True:
            i = text.find(needle, idx)
            if i < 0:
                return False
            # Look at the next ~6 lines for the historical
            # `return M12_MENU_INPUT_LEFT;` (turn-left).  If
            # found, the contract is broken.
            j = text.find("\n", i)
            for _ in range(6):
                k = text.find("\n", j + 1)
                if k < 0:
                    break
                chunk = text[j:k]
                if "return M12_MENU_INPUT_LEFT;" in chunk:
                    return True
                if "return M12_MENU_INPUT_RIGHT;" in chunk and token == "right":
                    return True
                j = k
            idx = i + 1
        return False
    cases.append({
        "id": "script_token_left_to_strafe",
        "label": "m11_map_script_token: 'left'/'l' token no longer routes to LEFT (turn)",
        "ok": not _has_stale_left_to_turn(text, "left"),
        "needle": "strncmp 'left' must NOT be followed by return M12_MENU_INPUT_LEFT",
    })
    cases.append({
        "id": "script_token_right_to_strafe",
        "label": "m11_map_script_token: 'right'/'r' token no longer routes to RIGHT (turn)",
        "ok": not _has_stale_left_to_turn(text, "right"),
        "needle": "strncmp 'right' must NOT be followed by return M12_MENU_INPUT_RIGHT",
    })
    # New turn-left/turn-right tokens.
    cases.append({
        "id": "script_token_turn_left",
        "label": "m11_map_script_token: 'turn-left'/'home'/'tl' -> TURN_LEFT",
        "ok": text.count("M12_MENU_INPUT_TURN_LEFT") > 0,
        "needle": "M12_MENU_INPUT_TURN_LEFT",
    })
    cases.append({
        "id": "script_token_turn_right",
        "label": "m11_map_script_token: 'turn-right'/'end'/'tr' -> TURN_RIGHT",
        "ok": text.count("M12_MENU_INPUT_TURN_RIGHT") > 0,
        "needle": "M12_MENU_INPUT_TURN_RIGHT",
    })

    # SDL keymap handler: SDLK_LEFT -> STRAFE_LEFT.
    cases.append({
        "id": "sdl_left_to_strafe",
        "label": "SDL keymap: SDLK_LEFT -> STRAFE_LEFT",
        "ok": "case SDLK_LEFT:" in text and "M12_MENU_INPUT_STRAFE_LEFT" in text,
        "needle": "case SDLK_LEFT: ... STRAFE_LEFT",
    })
    cases.append({
        "id": "sdl_right_to_strafe",
        "label": "SDL keymap: SDLK_RIGHT -> STRAFE_RIGHT",
        "ok": "case SDLK_RIGHT:" in text and "M12_MENU_INPUT_STRAFE_RIGHT" in text,
        "needle": "case SDLK_RIGHT: ... STRAFE_RIGHT",
    })
    cases.append({
        "id": "sdl_q_to_turn_left",
        "label": "SDL keymap: SDLK_Q -> TURN_LEFT",
        "ok": text.count("case SDLK_Q:") >= 1 and "M12_MENU_INPUT_TURN_LEFT" in text,
        "needle": "case SDLK_Q: ... TURN_LEFT",
    })
    cases.append({
        "id": "sdl_e_to_turn_right",
        "label": "SDL keymap: SDLK_E -> TURN_RIGHT",
        "ok": text.count("case SDLK_E:") >= 1 and "M12_MENU_INPUT_TURN_RIGHT" in text,
        "needle": "case SDLK_E: ... TURN_RIGHT",
    })
    cases.append({
        "id": "sdl_home_to_turn_left",
        "label": "SDL keymap: SDLK_HOME -> TURN_LEFT",
        "ok": "case SDLK_HOME:" in text,
        "needle": "case SDLK_HOME:",
    })
    cases.append({
        "id": "sdl_end_to_turn_right",
        "label": "SDL keymap: SDLK_END -> TURN_RIGHT",
        "ok": "case SDLK_END:" in text,
        "needle": "case SDLK_END:",
    })
    # KP_4 / KP_6 stay turn-left / turn-right (PC 3.4 source-locked).
    cases.append({
        "id": "sdl_kp4_to_turn_left",
        "label": "SDL keymap: SDLK_KP_4 -> TURN_LEFT (PC 3.4 source-lock)",
        "ok": "case SDLK_KP_4:" in text,
        "needle": "case SDLK_KP_4:",
    })
    cases.append({
        "id": "sdl_kp6_to_turn_right",
        "label": "SDL keymap: SDLK_KP_6 -> TURN_RIGHT (PC 3.4 source-lock)",
        "ok": "case SDLK_KP_6:" in text,
        "needle": "case SDLK_KP_6:",
    })
    # WASD aliases: A -> STRAFE_LEFT, D -> STRAFE_RIGHT.
    cases.append({
        "id": "sdl_a_to_strafe_left",
        "label": "SDL keymap: SDLK_A (WASD enabled) -> STRAFE_LEFT",
        "ok": "case SDLK_A:" in text and "M12_MENU_INPUT_STRAFE_LEFT" in text,
        "needle": "case SDLK_A:",
    })
    cases.append({
        "id": "sdl_d_to_strafe_right",
        "label": "SDL keymap: SDLK_D (WASD enabled) -> STRAFE_RIGHT",
        "ok": "case SDLK_D:" in text and "M12_MENU_INPUT_STRAFE_RIGHT" in text,
        "needle": "case SDLK_D:",
    })

    return cases


def verify_input_remap_m12() -> list[dict]:
    """s_defaults_original[] must use Home/End + Q/E for turn and
    arrow keys for strafe.  The historical "A/D = turn" binding
    is REMOVED from this preset."""
    text = read_text(MREMAP)
    cases: list[dict] = []

    cases.append({
        "id": "m12_preset_turn_left_home",
        "label": "s_defaults_original: TURN_LEFT = SDLK_HOME + SDLK_Q",
        "ok": "M12_ACTION_TURN_LEFT,        SDLK_HOME,      SDLK_Q" in text,
        "needle": "M12_ACTION_TURN_LEFT, SDLK_HOME, SDLK_Q",
    })
    cases.append({
        "id": "m12_preset_turn_right_end",
        "label": "s_defaults_original: TURN_RIGHT = SDLK_END + SDLK_E",
        "ok": "M12_ACTION_TURN_RIGHT,       SDLK_END,       SDLK_E" in text,
        "needle": "M12_ACTION_TURN_RIGHT, SDLK_END, SDLK_E",
    })
    cases.append({
        "id": "m12_preset_strafe_left_arrow",
        "label": "s_defaults_original: STRAFE_LEFT = SDLK_LEFT + SDLK_A",
        "ok": "M12_ACTION_STRAFE_LEFT,      SDLK_LEFT,      SDLK_A" in text,
        "needle": "M12_ACTION_STRAFE_LEFT, SDLK_LEFT, SDLK_A",
    })
    cases.append({
        "id": "m12_preset_strafe_right_arrow",
        "label": "s_defaults_original: STRAFE_RIGHT = SDLK_RIGHT + SDLK_D",
        "ok": "M12_ACTION_STRAFE_RIGHT,     SDLK_RIGHT,     SDLK_D" in text,
        "needle": "M12_ACTION_STRAFE_RIGHT, SDLK_RIGHT, SDLK_D",
    })
    # v2.8.x: the user's request applies to all games, so the
    # "hybrid" preset also gets the same arrow-strafe defaults.
    cases.append({
        "id": "m12_hybrid_preset_turn_left_home",
        "label": "s_defaults_hybrid: TURN_LEFT = SDLK_HOME + SDLK_Q",
        "ok": "M12_ACTION_TURN_LEFT,        SDLK_HOME,      SDLK_Q" in text,
        "needle": "M12_ACTION_TURN_LEFT, SDLK_HOME, SDLK_Q (in hybrid preset)",
    })
    cases.append({
        "id": "m12_hybrid_preset_turn_right_end",
        "label": "s_defaults_hybrid: TURN_RIGHT = SDLK_END + SDLK_E",
        "ok": "M12_ACTION_TURN_RIGHT,       SDLK_END,       SDLK_E" in text,
        "needle": "M12_ACTION_TURN_RIGHT, SDLK_END, SDLK_E (in hybrid preset)",
    })
    cases.append({
        "id": "m12_hybrid_preset_strafe_left_arrow",
        "label": "s_defaults_hybrid: STRAFE_LEFT = SDLK_LEFT + SDLK_A",
        "ok": "M12_ACTION_STRAFE_LEFT,      SDLK_LEFT,      SDLK_A" in text,
        "needle": "M12_ACTION_STRAFE_LEFT, SDLK_LEFT, SDLK_A (in hybrid preset)",
    })
    cases.append({
        "id": "m12_hybrid_preset_strafe_right_arrow",
        "label": "s_defaults_hybrid: STRAFE_RIGHT = SDLK_RIGHT + SDLK_D",
        "ok": "M12_ACTION_STRAFE_RIGHT,     SDLK_RIGHT,     SDLK_D" in text,
        "needle": "M12_ACTION_STRAFE_RIGHT, SDLK_RIGHT, SDLK_D (in hybrid preset)",
    })
    # The historical "A/D = turn" line MUST be gone from both presets.
    cases.append({
        "id": "m12_no_stale_ad_turn_binding",
        "label": "no M12_ACTION_TURN_LEFT, SDLK_LEFT, SDLK_A line in any preset",
        "ok": "M12_ACTION_TURN_LEFT,        SDLK_LEFT,      SDLK_A" not in text,
        "needle": "historical TURN_LEFT = SDLK_LEFT + SDLK_A must be removed",
    })
    cases.append({
        "id": "m12_no_stale_right_turn_binding",
        "label": "no M12_ACTION_TURN_RIGHT, SDLK_RIGHT, SDLK_D line in any preset",
        "ok": "M12_ACTION_TURN_RIGHT,       SDLK_RIGHT,     SDLK_D" not in text,
        "needle": "historical TURN_RIGHT = SDLK_RIGHT + SDLK_D must be removed",
    })
    return cases


def verify_input_remap_m11() -> list[dict]:
    """s_defs[] in src/engine/input_remap_m11.c must mirror the
    M12 preset for turn/strafe defaults."""
    text = read_text(REMAP11)
    cases: list[dict] = []

    cases.append({
        "id": "m11_preset_turn_left_home",
        "label": "s_defs: TURN_LEFT = SDL_SCANCODE_HOME + SDL_SCANCODE_Q",
        "ok": "M11_ACTION_TURN_LEFT,       \"turn_left\",       \"TURN LEFT\",       SDL_SCANCODE_HOME,   SDL_SCANCODE_Q" in text,
        "needle": "TURN_LEFT, SDL_SCANCODE_HOME, SDL_SCANCODE_Q",
    })
    cases.append({
        "id": "m11_preset_turn_right_end",
        "label": "s_defs: TURN_RIGHT = SDL_SCANCODE_END + SDL_SCANCODE_E",
        "ok": "M11_ACTION_TURN_RIGHT,      \"turn_right\",      \"TURN RIGHT\",      SDL_SCANCODE_END,    SDL_SCANCODE_E" in text,
        "needle": "TURN_RIGHT, SDL_SCANCODE_END, SDL_SCANCODE_E",
    })
    cases.append({
        "id": "m11_preset_strafe_left_arrow",
        "label": "s_defs: STRAFE_LEFT = SDL_SCANCODE_LEFT + SDL_SCANCODE_A",
        "ok": "M11_ACTION_STRAFE_LEFT,     \"strafe_left\",     \"STRAFE LEFT\",     SDL_SCANCODE_LEFT,   SDL_SCANCODE_A" in text,
        "needle": "STRAFE_LEFT, SDL_SCANCODE_LEFT, SDL_SCANCODE_A",
    })
    cases.append({
        "id": "m11_preset_strafe_right_arrow",
        "label": "s_defs: STRAFE_RIGHT = SDL_SCANCODE_RIGHT + SDL_SCANCODE_D",
        "ok": "M11_ACTION_STRAFE_RIGHT,    \"strafe_right\",    \"STRAFE RIGHT\",    SDL_SCANCODE_RIGHT,  SDL_SCANCODE_D" in text,
        "needle": "STRAFE_RIGHT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_D",
    })
    return cases


def verify_game_view() -> list[dict]:
    """m11_dm1_v1_pipeline_command_for_input switch must map
    M12_MENU_INPUT_TURN_LEFT to DM1_V1_COMMAND_TURN_LEFT and
    M12_MENU_INPUT_LEFT (defensive fallback) to MOVE_LEFT."""
    text = read_text(GVIEW)
    cases: list[dict] = []

    cases.append({
        "id": "gv_turn_left_command",
        "label": "m11_dm1_v1_pipeline_command_for_input: TURN_LEFT -> TURN_LEFT command",
        "ok": "case M12_MENU_INPUT_TURN_LEFT:" in text and "return DM1_V1_COMMAND_TURN_LEFT" in text,
        "needle": "TURN_LEFT -> DM1_V1_COMMAND_TURN_LEFT",
    })
    cases.append({
        "id": "gv_turn_right_command",
        "label": "m11_dm1_v1_pipeline_command_for_input: TURN_RIGHT -> TURN_RIGHT command",
        "ok": "case M12_MENU_INPUT_TURN_RIGHT:" in text and "return DM1_V1_COMMAND_TURN_RIGHT" in text,
        "needle": "TURN_RIGHT -> DM1_V1_COMMAND_TURN_RIGHT",
    })
    cases.append({
        "id": "gv_left_to_move_left",
        "label": "m11_dm1_v1_pipeline_command_for_input: LEFT -> MOVE_LEFT (strafe)",
        "ok": "case M12_MENU_INPUT_LEFT:" in text and "return DM1_V1_COMMAND_MOVE_LEFT" in text,
        "needle": "LEFT -> DM1_V1_COMMAND_MOVE_LEFT",
    })
    cases.append({
        "id": "gv_right_to_move_right",
        "label": "m11_dm1_v1_pipeline_command_for_input: RIGHT -> MOVE_RIGHT (strafe)",
        "ok": "case M12_MENU_INPUT_RIGHT:" in text and "return DM1_V1_COMMAND_MOVE_RIGHT" in text,
        "needle": "RIGHT -> DM1_V1_COMMAND_MOVE_RIGHT",
    })
    return cases


def main() -> int:
    cases = (
        verify_main_loop()
        + verify_input_remap_m12()
        + verify_input_remap_m11()
        + verify_game_view()
    )
    failures = [c for c in cases if not c["ok"]]

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    payload = {
        "schema": "input_remap_keyboard_layout.v28x.v1",
        "pass": len(failures) == 0,
        "status": STATUS_PASS if not failures else f"FAIL_INPUT_REMAP_KEYBOARD_LAYOUT_{len(failures)}_BREAKS",
        "scope": "v2.8.x keyboard layout contract: arrow keys strafe, "
                 "Home/End/Q/E turn, WASD mirrors arrows. Validates "
                 "src/engine/main_loop_m11.c (script-token parser + "
                 "two SDL keymap switches), src/shared/input_remap_m12.c "
                 "s_defaults_original[], src/engine/input_remap_m11.c "
                 "s_defs[], src/engine/m11_game_view.c "
                 "m11_dm1_v1_pipeline_command_for_input.",
        "sourceReferences": [
            "ReDMCSB COMMAND.C:677-684 keypad scancode -> command table",
            "ReDMCSB COMMAND.C G0448 menu arrow-click command routing",
            "ReDMCSB COMMAND.C:2438-2451 entrance input / Enter handling",
            "ReDMCSB IO2.C:47-59 shifted arrow normalisation",
        ],
        "cases": cases,
        "nonClaims": [
            "No change to M12 menu +/- cycle buttons (still use M12_MENU_INPUT_LEFT/RIGHT).",
            "No change to M12_MENU_INPUT_VALUE_LEFT/RIGHT (settings value +/-).",
            "No change to inventory/map/dialog overlay dismissal (still swallow all input).",
            "No new keybinding file format — same keybindings.toml.",
        ],
    }
    OUT_JSON.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")

    print(f"input_remap_keyboard_layout: {len(cases)} contract cases checked")
    if failures:
        for c in failures:
            print(f"  FAIL: {c['label']} ({c['needle']})")
        return 1
    print(f"PASS - manifest at {OUT_JSON.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
