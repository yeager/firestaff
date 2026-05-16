#!/usr/bin/env python3
"""Verify the DM1 V1 touchscreen seam stays locked to original mouse/UI/action zones.

This guard is intentionally source-first: it audits the ReDMCSB mouse tables,
primary->secondary lookup order, action child-zone resolution, and raw mouse
button dispatch before accepting the local touch zone matrix. It does not
promote or change gameplay semantics.
"""
from __future__ import annotations

import json
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
).expanduser()
OUT_DIR = REPO / "parity-evidence/verification/dm1_v1_touch_original_mouse_ui_action_zones"
MANIFEST = OUT_DIR / "manifest.json"
EVIDENCE = OUT_DIR / "evidence.md"


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise AssertionError(f"missing {label}: {needle}")


def require_all(label: str, checks: list[tuple[str, str]]) -> None:
    for text, needle in checks:
        require(text, needle, label)


def main() -> None:
    command_c = read(SOURCE_ROOT / "COMMAND.C")
    input_c = read(SOURCE_ROOT / "INPUT.C")
    clikmenu_c = read(SOURCE_ROOT / "CLIKMENU.C")
    defs_h = read(SOURCE_ROOT / "DEFS.H")
    local_matrix_c = read(REPO / "src/shared/touch_click_zone_matrix_pc34_compat.c")
    local_pointer_c = read(REPO / "src/shared/touch_pointer_input_pc34_compat.c")
    local_pointer_test = read(REPO / "tests/test_touch_pointer_input_pc34_compat_integration.c")

    source_checks = {
        "primary_interface_mouse_rows": [
            (command_c, "MOUSE_INPUT G0447_as_Graphic561_PrimaryMouseInput_Interface"),
            (command_c, "{ C100_COMMAND_CLICK_IN_SPELL_AREA,                 233, 319,  42,  73, MASK0x0002_MOUSE_LEFT_BUTTON }"),
            (command_c, "{ C111_COMMAND_CLICK_IN_ACTION_AREA,                233, 319,  77, 121, MASK0x0002_MOUSE_LEFT_BUTTON }"),
            (command_c, "{ C147_COMMAND_FREEZE_GAME,                           0,   1, 198, 199, MASK0x0002_MOUSE_LEFT_BUTTON }"),
        ],
        "secondary_movement_mouse_rows": [
            (command_c, "MOUSE_INPUT G0448_as_Graphic561_SecondaryMouseInput_Movement"),
            (command_c, "{ C001_COMMAND_TURN_LEFT,             234, 261, 125, 145, MASK0x0002_MOUSE_LEFT_BUTTON }"),
            (command_c, "{ C003_COMMAND_MOVE_FORWARD,          263, 289, 125, 145, MASK0x0002_MOUSE_LEFT_BUTTON }"),
            (command_c, "{ C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   0, 223,  33, 168, MASK0x0002_MOUSE_LEFT_BUTTON }"),
            (command_c, "{ C083_COMMAND_TOGGLE_INVENTORY_LEADER, 0, 319,  33, 199, MASK0x0001_MOUSE_RIGHT_BUTTON }"),
        ],
        "action_child_mouse_rows_and_resolution": [
            (command_c, "MOUSE_INPUT G0452_as_Graphic561_MouseInput_ActionAreaNames"),
            (command_c, "{ C112_COMMAND_CLICK_IN_ACTION_AREA_PASS,     285, 318,  77,  83, MASK0x0002_MOUSE_LEFT_BUTTON }"),
            (command_c, "{ C115_COMMAND_CLICK_IN_ACTION_AREA_ACTION_2, 234, 318, 110, 120, MASK0x0002_MOUSE_LEFT_BUTTON }"),
            (command_c, "MOUSE_INPUT G0453_as_Graphic561_MouseInput_ActionAreaIcons"),
            (command_c, "{ C119_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_3_ACTION, 299, 318, 86, 120, MASK0x0002_MOUSE_LEFT_BUTTON }"),
            (clikmenu_c, "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0452_as_Graphic561_MouseInput_ActionAreaNames"),
            (clikmenu_c, "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0453_as_Graphic561_MouseInput_ActionAreaIcons"),
            (clikmenu_c, "G0321_B_StopWaitingForPlayerInput = F0391_MENUS_DidClickTriggerAction"),
        ],
        "source_lookup_and_raw_mouse_dispatch": [
            (command_c, "L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput"),
            (command_c, "L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput"),
            (input_c, "M008_SET(G0588_i_MouseButtonsStatus, MASK0x0002_MOUSE_LEFT_BUTTON);"),
            (input_c, "F0359_COMMAND_ProcessClick_CPSC(G1038_i_MouseX >> 1, G1039_i_MouseY >> 1, MASK0x0002_MOUSE_LEFT_BUTTON);"),
            (input_c, "M008_SET(G0588_i_MouseButtonsStatus, MASK0x0001_MOUSE_RIGHT_BUTTON);"),
            (input_c, "F0359_COMMAND_ProcessClick_CPSC(G1038_i_MouseX >> 1, G1039_i_MouseY >> 1, MASK0x0001_MOUSE_RIGHT_BUTTON);"),
        ],
        "zone_names_for_action_ui": [
            (defs_h, "#define C011_ZONE_ACTION_AREA"),
            (defs_h, "#define C013_ZONE_SPELL_AREA"),
            (defs_h, "#define C068_ZONE_TURN_LEFT"),
            (defs_h, "#define C070_ZONE_MOVE_FORWARD"),
            (defs_h, "#define C098_ZONE_ACTION_AREA_PASS"),
        ],
    }
    for label, checks in source_checks.items():
        require_all(label, checks)

    local_checks = {
        "local_source_ordered_touch_matrix": [
            (local_matrix_c, "kPrimaryInterfaceSourceOrder"),
            (local_matrix_c, "kSecondaryMovementSourceOrder"),
            (local_matrix_c, "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary"),
            (local_matrix_c, '"action.parent"'),
            (local_matrix_c, '"action.pass"'),
            (local_matrix_c, '"action.icon3"'),
            (local_matrix_c, '"movement.forward"'),
        ],
        "touch_pointer_uses_mouse_queue_not_keyboard": [
            (local_pointer_c, "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary"),
            (local_pointer_c, "DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat"),
            (local_pointer_c, "not synthesized keys"),
        ],
        "runtime_probe_covers_pointer_queue": [
            (local_pointer_test, "TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue"),
            (local_pointer_test, "processResult.dispatchedMove"),
            (local_pointer_test, "queue.pendingClickCommand != 71"),
        ],
    }
    for label, checks in local_checks.items():
        require_all(label, checks)

    result = {
        "status": "DM1_V1_TOUCH_ORIGINAL_MOUSE_UI_ACTION_ZONES_SOURCE_LOCK_VERIFIED",
        "repo": str(REPO),
        "sourceRoot": str(SOURCE_ROOT),
        "sourceAudits": sorted(source_checks.keys()),
        "localAudits": sorted(local_checks.keys()),
        "guardrail": "source-lock only: touch remains a mouse-coordinate producer feeding the existing command queue; no gameplay dispatch or movement semantics changed",
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    EVIDENCE.write_text(
        "# DM1 V1 touch original mouse/UI/action zones source lock\n\n"
        "Status: DM1_V1_TOUCH_ORIGINAL_MOUSE_UI_ACTION_ZONES_SOURCE_LOCK_VERIFIED\n\n"
        "- ReDMCSB audited: primary interface rows, secondary movement rows, action child rows, raw mouse button dispatch, and primary-before-secondary lookup order.\n"
        "- Local audited: touch matrix keeps source-order tables and touch pointer events enqueue through the mouse command queue seam.\n"
        "- Guardrail: no synthetic keyboard path, no new gameplay dispatcher, no movement parity change.\n",
    )
    print(json.dumps(result, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
