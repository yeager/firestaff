#!/usr/bin/env python3
"""Verify DM1 V1 viewport hand/cursor carried-item overlay is source-locked."""
from __future__ import annotations

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
HDR = ROOT / "include/dm1_v1_viewport_hand_overlay_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_viewport_hand_overlay_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
REDMCSB_IO = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/IO.C"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_order(text: str, first: str, second: str, label: str) -> None:
    p1 = require(text, first, label)
    p2 = require(text, second, label)
    if p1 >= p2:
        raise AssertionError(f"{label}: {first!r} must precede {second!r}")


def main() -> int:
    hdr = HDR.read_text(encoding="utf-8")
    test = TEST.read_text(encoding="utf-8")
    cmake = CMAKE.read_text(encoding="utf-8")

    for needle in [
        "DM1_V1_POINTER_ARROW       0",
        "DM1_V1_POINTER_HAND        1",
        "DM1_V1_POINTER_OBJECT_ICON 2",
        "DM1_V1_CURSOR_VIEWPORT_RIGHT_EXCLUSIVE 224",
        "DM1_V1_CURSOR_TOP_PANEL_BOTTOM          28",
        "DM1_V1_CURSOR_MENU_LEFT                224",
        "DM1_V1_CURSOR_CHAMPION_ICON_LEFT       274",
        "DM1_V1_CURSOR_MESSAGE_TOP              169",
        "DM1_V1_CURSOR_OBJECT_HOTSPOT_X           8",
        "DM1_V1_CURSOR_OBJECT_HOTSPOT_Y           8",
        "DM1_V1_CURSOR_OBJECT_SCREEN_Y_OFFSET    19",
        "DM1_V1_CURSOR_OBJECT_LAST_LINE_INDEX    17",
        "DM1_V1_ViewportHandOverlay_RegionPointerPc34Compat",
        "DM1_V1_ViewportHandOverlay_DecidePc34Compat",
        "useObjectAsPointer && regionPointerType == DM1_V1_POINTER_HAND",
    ]:
        require(hdr, needle, "header")

    require_order(hdr, "if (useObjectAsPointer) {", "} else {\n            out.pointerType = DM1_V1_POINTER_HAND;", "object priority")
    for needle in [
        "object priority over hand in viewport",
        "viewport lower-right inclusive object",
        "menu area arrow",
        "message area arrow",
        "top status name arrow",
    ]:
        require(test, needle, "unit test")

    require(cmake, "test_dm1_v1_viewport_hand_overlay_pc34_compat", "cmake")
    require(cmake, "v1_viewport_hand_overlay_gate", "cmake")

    red = REDMCSB_IO.read_text(encoding="latin-1")
    markers = [
        "/* Determine mouse pointer type */",
        "G0600_B_UseObjectAsMousePointerBitmap == C1_TRUE",
        "return C2_POINTER_OBJECT_ICON;",
        "cmpi.w  #169,L0060_ul_Y",
        "cmpi.w  #274,L0059_ul_X",
        "cmpi.w  #28,L0060_ul_Y",
        "cmpi.w  #224,L0059_ul_X",
        "G0605_i_MousePointerBitmapHotspotX = 8;",
        "G0606_i_MousePointerBitmapHotspotY = 8;",
        "moveq   #18,L0061_i_Height",
        "moveq   #17,L0062_i_PixelWidth",
        "subq.w  #8,L0059_ul_X",
        "subi.w  #19,L0060_ul_Y",
        "T0073011_MousePointerAcrossPaletteAreas",
    ]
    for marker in markers:
        pos = require(red, marker, "ReDMCSB IO.C")
        print(f"- ReDMCSB IO.C:{line_no(red, pos)} {marker}")

    print("V1 viewport hand/cursor carried-item overlay gate passed")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
