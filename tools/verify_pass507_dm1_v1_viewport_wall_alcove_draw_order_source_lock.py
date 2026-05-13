#!/usr/bin/env python3
"""Pass 507 DM1 V1 viewport wall/alcove draw-order source lock.

This evidence gate locks the Lane B wall/alcove/draw-order surface to local
ReDMCSB. It deliberately checks metadata and tests only; it does not touch
movement, command routing, or pass435-owned code.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
RED_ROOT = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
).expanduser()
DUNVIEW = RED_ROOT / "DUNVIEW.C"
COMPAT = ROOT / "dm1_v1_viewport_3d_pc34_compat.c"
COMPAT_H = ROOT / "dm1_v1_viewport_3d_pc34_compat.h"
TEST = ROOT / "test_dm1_v1_viewport_3d_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
EVIDENCE = ROOT / "parity-evidence/pass507_dm1_v1_viewport_wall_alcove_draw_order_source_lock.md"

ALLOWED_REDMCSB = Path("~/.openclaw/data/firestaff-redmcsb-source").expanduser().resolve()


def read(path: Path, encoding: str = "utf-8") -> str:
    if path == DUNVIEW and not str(path.resolve()).startswith(str(ALLOWED_REDMCSB)):
        raise AssertionError(f"refusing non-local ReDMCSB path: {path}")
    return path.read_text(encoding=encoding, errors="replace")


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_in_order(text: str, needles: list[str], label: str) -> list[int]:
    positions: list[int] = []
    last = -1
    for needle in needles:
        pos = text.find(needle, last + 1)
        if pos < 0:
            raise AssertionError(f"{label}: missing {needle!r}")
        if pos <= last:
            raise AssertionError(f"{label}: {needle!r} is out of order")
        positions.append(pos)
        last = pos
    return positions


def line_slice(text: str, start: int, end: int) -> str:
    return "\n".join(text.splitlines()[start - 1:end])


def find_function(text: str, name: str) -> tuple[int, str]:
    match = re.search(r"\b" + re.escape(name) + r"\s*\(", text)
    if not match:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", match.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return match.start(), text[match.start():pos + 1]
    raise AssertionError(f"unterminated function {name}")


def main() -> int:
    dunview = read(DUNVIEW, "latin-1")
    compat = read(COMPAT)
    compat_h = read(COMPAT_H)
    test = read(TEST)
    cmake = read(CMAKE)
    evidence = read(EVIDENCE)

    report: list[tuple[str, Path, int]] = []

    f0107 = line_slice(dunview, 3502, 3940)
    require_in_order(f0107, [
        "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(",
        "F0106_DUNGEONVIEW_TestResetToStep1_CPSF",
        "G0286_B_FacingAlcove = L0096_B_IsAlcove;",
        "return L0096_B_IsAlcove;",
        "return C0_FALSE;",
    ], "ReDMCSB F0107 alcove classifier")
    report.append(("F0107 alcove classifier", DUNVIEW, 3502))

    f0115 = line_slice(dunview, 4547, 6100)
    require_in_order(f0115, [
        "Contains 4 nibbles processed from the least significant to the most significant nibble",
        "If the first nibble is 0, then the function call is to draw objects in an alcove on a wall square.",
        "L0135_B_DrawAlcoveObjects = !(L0130_ul_RemainingViewCellOrdinalsToProcess = P0146_ui_OrderedViewCellOrdinals);",
        "/* Draw objects */",
        "AL0126_i_ViewCell = C04_VIEW_CELL_ALCOVE; /* Index of coordinates to draw objects in alcoves */",
        "/* Draw creatures */",
        "T0115129_DrawProjectiles:",
        "/* Draw explosions */",
    ], "ReDMCSB F0115 cell/layer order")
    report.append(("F0115 packed cell and layer order", DUNVIEW, 4547))

    f0124 = line_slice(dunview, 7727, 7940)
    require_in_order(f0124, [
        "case C00_ELEMENT_WALL:",
        "if (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "case C17_ELEMENT_DOOR_FRONT:",
        "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
        "F0111_DUNGEONVIEW_DrawDoor",
        "L0217_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
        "T0124018:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
    ], "ReDMCSB F0124 alcove and door occlusion")
    report.append(("F0124 D1C wall alcove and door split", DUNVIEW, 7727))

    f0128 = line_slice(dunview, 8318, 8545)
    require_in_order(f0128, [
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
        "M598_VIEW_SQUARE_D4L",
        "M599_VIEW_SQUARE_D4R",
        "M597_VIEW_SQUARE_D4C",
        "F0676_DrawD3L2",
        "F0677_DrawD3R2",
        "F0116_DUNGEONVIEW_DrawSquareD3L",
        "F0117_DUNGEONVIEW_DrawSquareD3R",
        "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
        "F0678_DrawD2L2",
        "F0679_DrawD2R2",
        "F0119_DUNGEONVIEW_DrawSquareD2L",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
        "F0121_DUNGEONVIEW_DrawSquareD2C",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "F0124_DUNGEONVIEW_DrawSquareD1C",
        "F0125_DUNGEONVIEW_DrawSquareD0L",
        "F0126_DUNGEONVIEW_DrawSquareD0R",
        "F0127_DUNGEONVIEW_DrawSquareD0C",
    ], "ReDMCSB F0128 far-to-near replay")
    report.append(("F0128 viewport replay order", DUNVIEW, 8318))

    require_in_order(compat, [
        "static const DM1_ViewportDrawStep s_draw_order[] = {",
        "DM1_VIEW_SQUARE_D4L",
        "DM1_VIEW_SQUARE_D3L2",
        "DM1_VIEW_SQUARE_D2C",
        "DM1_VIEW_SQUARE_D1C",
        "DM1_VIEW_SQUARE_D0C",
        "static const DM1_ViewportThingLayerSpec s_thing_layers[] = {",
        "DM1_VIEWPORT_THING_LAYER_OBJECTS",
        "DM1_VIEWPORT_THING_LAYER_CREATURES",
        "DM1_VIEWPORT_THING_LAYER_PROJECTILES",
        "DM1_VIEWPORT_THING_LAYER_EXPLOSIONS",
        "static const DM1_ViewportDoorFrontOcclusionSpec s_door_front_occlusion_specs[] = {",
        "DUNVIEW.C:7874-7875 pass1 rear cells before frame",
        "DUNVIEW.C:7910-7937 pass2 front cells after door",
        "static const DM1_ViewportWallDrawSpec s_wall_draw_specs[] = {",
        "DUNVIEW.C:7842-7843 front alcove draws F0115; no side cells behind D1C",
    ], "Firestaff viewport wall metadata")
    report.append(("Firestaff wall metadata", COMPAT, line_no(compat, compat.find("static const DM1_ViewportDrawStep"))))

    for needle in [
        "test_redmcsb_f0128_draw_order",
        "test_f0115_cell_order_and_layer_z_order",
        "test_door_front_occlusion_split_passes",
        "PC34.wall_spec.%02zu.front_alcove",
        "F0115.cell_order.alcove",
    ]:
        require(test, needle, "Firestaff viewport tests")
    report.append(("Firestaff viewport tests", TEST, line_no(test, test.find("test_redmcsb_f0128_draw_order"))))

    for needle in [
        "DM1_ViewportDrawStep",
        "DM1_ViewportWallDrawSpec",
        "DM1_ViewportDoorFrontOcclusionSpec",
        "DM1_ViewportCellOrder",
    ]:
        require(compat_h, needle, "Firestaff viewport public contract")

    require(cmake, "NAME pass507_dm1_v1_viewport_wall_alcove_draw_order_source_lock", "CMake/CTest registration")
    require(evidence, "Pass 507 DM1 V1 viewport wall/alcove draw-order source lock", "parity evidence")

    print("Pass 507 DM1 V1 viewport wall/alcove draw-order source lock passed")
    for label, path, line in report:
        print(f"- {label}: {path.name}:{line}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
