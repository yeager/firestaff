#!/usr/bin/env python3
from __future__ import annotations

import json
import re
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT_JSON = REPO / "parity-evidence/verification/dm1_v1_touch_input_source_locked_ui_zones.json"
OUT_MD = REPO / "parity-evidence/dm1_v1_touch_input_source_locked_ui_zones.md"


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding="latin-1", errors="replace")


def block(text: str, start: int, end: int) -> str:
    return "\n".join(text.splitlines()[start - 1 : end])


def require(text: str, needle: str, cite: str) -> None:
    if needle not in text:
        raise AssertionError(f"{cite} missing: {needle}")


MATRIX_RE = re.compile(
    r"\{\s*(\d+)u\s*,\s*(\d+)u\s*,\s*"
    r"(TOUCH_CLICK_COORD_[A-Z_]+_PC34_COMPAT)\s*,\s*"
    r"(TOUCH_CLICK_BUTTON_(?:LEFT|RIGHT)_PC34_COMPAT)\s*,\s*"
    r"(-?\d+)\s*,\s*(-?\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*\"([^\"]+)\"",
)


def parse_matrix() -> dict[str, dict[str, object]]:
    rows: dict[str, dict[str, object]] = {}
    for m in MATRIX_RE.finditer(read(REPO / "src/shared/touch_click_zone_matrix_pc34_compat.c")):
        command, zone, coord, button, x, y, w, h, name = m.groups()
        rows[name] = {
            "command": int(command),
            "zone": int(zone),
            "coord": coord,
            "button": button,
            "rect": [int(x), int(y), int(w), int(h)],
        }
    if len(rows) != 104:
        raise AssertionError(f"expected 104 source-locked touch matrix rows, got {len(rows)}")
    return rows


def require_row(rows: dict[str, dict[str, object]], name: str, command: int, zone: int, rect: list[int], button: str) -> None:
    expected = {
        "command": command,
        "zone": zone,
        "coord": "TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT",
        "button": button,
        "rect": rect,
    }
    if rows.get(name) != expected:
        raise AssertionError(f"{name} mismatch: expected {expected}, got {rows.get(name)}")


def main() -> None:
    command_c = read(SOURCE_ROOT / "COMMAND.C")
    coord_c = read(SOURCE_ROOT / "COORD.C")
    clikmenu_c = read(SOURCE_ROOT / "CLIKMENU.C")
    clikview_c = read(SOURCE_ROOT / "CLIKVIEW.C")
    entrance_c = read(SOURCE_ROOT / "ENTRANCE.C")
    anchors = {
        "primary_interface": "COMMAND.C:375-395",
        "secondary_movement": "COMMAND.C:396-405",
        "action_spell_champion_children": "COMMAND.C:461-497",
        "mouse_hit_test": "COMMAND.C:1379-1449",
        "primary_before_secondary": "COMMAND.C:1641-1644",
        "inclusive_zone_test": "COORD.C:1915-1920",
        "action_area_child_resolution": "CLIKMENU.C:529-582",
        "viewport_click_handler": "CLIKVIEW.C:365-510",
        "entrance_mouse_table_install": "ENTRANCE.C:717-740",
    }

    checks = [
        (command_c, 375, 395, ["G0447_as_Graphic561_PrimaryMouseInput_Interface", "C013_ZONE_SPELL_AREA", "C011_ZONE_ACTION_AREA", "C147_COMMAND_FREEZE_GAME"], anchors["primary_interface"]),
        (command_c, 396, 405, ["C068_ZONE_TURN_LEFT", "C070_ZONE_MOVE_FORWARD", "C007_ZONE_VIEWPORT", "C083_COMMAND_TOGGLE_INVENTORY_LEADER", "MASK0x0001_MOUSE_RIGHT_BUTTON"], anchors["secondary_movement"]),
        (command_c, 461, 497, ["G0452_as_Graphic561_MouseInput_ActionAreaNames", "C098_ZONE_ACTION_AREA_PASS", "G0453_as_Graphic561_MouseInput_ActionAreaIcons", "C092_ZONE_ACTION_AREA_CHAMPION_3_ACTION", "G0454_as_Graphic561_MouseInput_SpellArea", "C254_ZONE_SPELL_AREA_RECANT_SYMBOL", "G0455_as_Graphic561_MouseInput_ChampionNamesHands", "C218_ZONE_SLOT_BOX_07_CHAMPION_3_STATUS_BOX_ACTION_HAND"], anchors["action_spell_champion_children"]),
        (command_c, 1379, 1449, ["while (L1107_Command = P0721_ps_MouseInput->Command)", "F0798_COMMAND_IsPointInZone", "P0724_i_ButtonsStatus & P0721_ps_MouseInput->Button"], anchors["mouse_hit_test"]),
        (command_c, 1641, 1644, ["G0441_ps_PrimaryMouseInput", "G0442_ps_SecondaryMouseInput"], anchors["primary_before_secondary"]),
        (coord_c, 1915, 1920, ["P0750_i_X >= M704_ZONE_LEFT"], anchors["inclusive_zone_test"]),
        (clikmenu_c, 529, 582, ["G0452_as_Graphic561_MouseInput_ActionAreaNames", "G0453_as_Graphic561_MouseInput_ActionAreaIcons"], anchors["action_area_child_resolution"]),
        (clikview_c, 365, 510, ["C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT", "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor"], anchors["viewport_click_handler"]),
        (entrance_c, 717, 740, ["G0441_ps_PrimaryMouseInput = G0445_as_Graphic561_PrimaryMouseInput_Entrance"], anchors["entrance_mouse_table_install"]),
    ]
    for text, start, end, needles, cite in checks:
        segment = block(text, start, end)
        for needle in needles:
            require(segment, needle, cite)

    rows = parse_matrix()
    left = "TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT"
    right = "TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT"
    expected_rows = [
        ("viewport.dungeon", 80, 7, [0, 33, 224, 136], left),
        ("movement.turn_left", 1, 68, [234, 125, 28, 21], left),
        ("movement.forward", 3, 70, [263, 125, 27, 21], left),
        ("movement.turn_right", 2, 69, [291, 125, 28, 21], left),
        ("movement.left", 6, 73, [234, 147, 28, 21], left),
        ("movement.backward", 5, 72, [263, 147, 27, 21], left),
        ("movement.right", 4, 71, [291, 147, 28, 21], left),
        ("inventory.toggle_leader", 83, 2, [0, 0, 320, 200], right),
        ("spell.parent", 100, 13, [233, 42, 87, 33], left),
        ("action.parent", 111, 11, [233, 77, 87, 45], left),
        ("action.pass", 112, 98, [285, 77, 35, 7], left),
        ("action.row2", 115, 84, [234, 110, 85, 11], left),
        ("action.icon3", 119, 92, [299, 86, 20, 35], left),
        ("system.freeze_game", 147, 0, [0, 198, 2, 2], left),
    ]
    for args in expected_rows:
        require_row(rows, *args)

    result = {
        "status": "DM1_V1_TOUCH_INPUT_SOURCE_LOCKED_UI_ZONES_VERIFIED",
        "source_root": str(SOURCE_ROOT),
        "anchors": anchors,
        "matrix_rows_checked": [name for name, *_ in expected_rows],
        "matrix_row_count": len(rows),
        "parity_behavior_changed": False,
        "guardrail": "Evidence gate only; touch zones remain mouse-command coordinate evidence and are not coupled to keyboard behavior.",
        "hash_locked_original_data": [],
    }
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    OUT_MD.write_text(
        "# DM1 V1 touch input source-locked UI zones\n\n"
        "Status: DM1_V1_TOUCH_INPUT_SOURCE_LOCKED_UI_ZONES_VERIFIED\n\n"
        "Scope: evidence-only gate for viewport, movement controls, action/spell/menu parents, action child rows/icons, and the entrance mouse-table install. No runtime input behavior changes are made by this pass.\n\n"
        "ReDMCSB anchors:\n"
        "- COMMAND.C:375-395 primary in-game interface table.\n"
        "- COMMAND.C:396-405 secondary movement/viewport/right-button leader-inventory table.\n"
        "- COMMAND.C:461-497 action, spell, and champion-name/hand child tables.\n"
        "- COMMAND.C:1379-1449 mouse table scan and button/zone hit test.\n"
        "- COMMAND.C:1641-1644 primary-before-secondary lookup order.\n"
        "- COORD.C:1915-1920 inclusive zone bounds check.\n"
        "- CLIKMENU.C:529-582 action-area child resolution.\n"
        "- CLIKVIEW.C:365-510 viewport click handler path.\n"
        "- ENTRANCE.C:717-740 entrance mouse table install.\n\n"
        "Guardrail: no parity behavior changed; this only verifies source-locked evidence in the existing touch click matrix. No DUNGEON.DAT or GRAPHICS.DAT variant was read by this verifier.\n",
        encoding="utf-8",
    )
    print(json.dumps(result, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
