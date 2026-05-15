#!/usr/bin/env python3
"""Verify pass560 DM1 V1 touch UI zone source-lock evidence.

This is intentionally a source/evidence gate. It does not exercise movement
logic and it does not require original game assets.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path


REPO = Path(__file__).resolve().parents[1]
REDMCSB = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8", errors="replace")
    except OSError as exc:
        raise AssertionError(f"could not read {path}: {exc}") from exc


def require_contains(name: str, haystack: str, needle: str) -> None:
    if needle not in haystack:
        raise AssertionError(f"{name} missing source anchor: {needle}")


def count_matrix_entries(source: str) -> int:
    match = re.search(
        r"static const TouchClickZonePc34Compat kTouchClickZones\[\] = \{(?P<body>.*?)\n\};",
        source,
        flags=re.S,
    )
    if not match:
        raise AssertionError("could not locate kTouchClickZones initializer")
    return len(re.findall(r"^\s*\{\s*\d+u,", match.group("body"), flags=re.M))


def main() -> int:
    command = read(REDMCSB / "COMMAND.C")
    defs = read(REDMCSB / "DEFS.H")
    coord = read(REDMCSB / "COORD.C")
    matrix = read(REPO / "touch_click_zone_matrix_pc34_compat.c")
    pointer = read(REPO / "touch_pointer_input_pc34_compat.c")
    matrix_test = read(REPO / "test_touch_click_zone_matrix_pc34_compat_integration.c")
    pointer_test = read(REPO / "test_touch_pointer_input_pc34_compat_integration.c")
    manifest = read(REPO / "parity-evidence/pass560_dm1_v1_touch_ui_zone_source_lock.md")

    command_anchors = [
        "MOUSE_INPUT G0447_as_Graphic561_PrimaryMouseInput_Interface[20]",
        "MOUSE_INPUT G0448_as_Graphic561_SecondaryMouseInput_Movement[9]",
        "C080_COMMAND_CLICK_IN_DUNGEON_VIEW",
        "C083_COMMAND_TOGGLE_INVENTORY_LEADER",
        "C070_COMMAND_CLICK_ON_MOUTH",
        "C071_COMMAND_CLICK_ON_EYE",
        "F0358_COMMAND_GetCommandFromMouseInput_CPSC",
        "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput",
        "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput",
        "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command",
        "F0377_COMMAND_ProcessType80_ClickInDungeonView",
    ]
    for anchor in command_anchors:
        require_contains("COMMAND.C", command, anchor)

    defs_anchors = [
        "typedef struct {\n        int16_t Command;",
        "#define MASK0x0001_MOUSE_RIGHT_BUTTON    0x0001",
        "#define MASK0x0002_MOUSE_LEFT_BUTTON     0x0002",
        "#define C007_ZONE_VIEWPORT",
        "#define C011_ZONE_ACTION_AREA",
        "#define C013_ZONE_SPELL_AREA",
        "#define C068_ZONE_TURN_LEFT",
        "#define C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS",
        "#define C507_ZONE_SLOT_BOX_08_INVENTORY_READY_HAND",
        "#define C545_ZONE_MOUTH",
        "#define C546_ZONE_EYE",
        "#define M664_ZONE_RESURRECT",
    ]
    for anchor in defs_anchors:
        require_contains("DEFS.H", defs, anchor)

    coord_anchors = [
        "int16_t G2071_C320_ScreenPixelWidth = 320;",
        "int16_t G2072_C200_ScreenPixelHeight = 200;",
        "int16_t G2073_C224_ViewportPixelWidth = 224;",
        "int16_t G2074_C136_ViewportHeight = 136;",
        "F0798_COMMAND_IsPointInZone",
    ]
    for anchor in coord_anchors:
        require_contains("COORD.C", coord, anchor)

    if count_matrix_entries(matrix) != 104:
        raise AssertionError("touch matrix entry count is not 104")

    matrix_tokens = [
        '"movement.turn_left"',
        '"movement.forward"',
        '"viewport.dungeon"',
        '"inventory.toggle_leader"',
        '"champion0.status_box"',
        '"action.parent"',
        '"spell.symbol1"',
        '"inventory.ready_hand"',
        '"inventory.eye"',
        '"panel.resurrect"',
        '"system.freeze_game"',
        "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary",
        "kPrimaryInterfaceSourceOrder",
        "kSecondaryMovementSourceOrder",
        "*outX = 0;",
        "*outY = 33;",
        "*outW = 224;",
        "*outH = 136;",
    ]
    for token in matrix_tokens:
        require_contains("touch_click_zone_matrix_pc34_compat.c", matrix, token)

    pointer_tokens = [
        "TOUCHPOINTER_Compat_TranslateEvent",
        "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary",
        "DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat",
        "not synthesized keys",
    ]
    for token in pointer_tokens:
        require_contains("touch_pointer_input_pc34_compat.c", pointer, token)

    test_tokens = [
        "TOUCHCLICK_Compat_GetZoneCount() != 104u",
        "TOUCHCLICK_Compat_GetSourceViewportRect",
        "TOUCH_POINTER_SPACE_SCALED_SCREEN_PC34_COMPAT",
        "TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue",
    ]
    combined_tests = matrix_test + "\n" + pointer_test
    for token in test_tokens:
        require_contains("touch tests", combined_tests, token)

    require_contains(
        "pass560 manifest",
        manifest,
        "No movement implementation or runtime route code is changed",
    )
    require_contains("pass560 manifest", manifest, "PASS_DM1_V1_TOUCH_UI_ZONE_SOURCE_LOCK")

    print("PASS_DM1_V1_TOUCH_UI_ZONE_SOURCE_LOCK")
    print("matrixEntries=104")
    print("sourceViewportRect=0,33,224,136")
    print("movementCodeTouched=0")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"FAIL_DM1_V1_TOUCH_UI_ZONE_SOURCE_LOCK: {exc}", file=sys.stderr)
        raise SystemExit(1)
