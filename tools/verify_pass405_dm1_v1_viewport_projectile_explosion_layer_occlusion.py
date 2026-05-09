#!/usr/bin/env python3
"""Verify DM1 V1 viewport projectile/explosion layer sequencing + occlusion metadata.

Focused source-lock for the Firestaff M11 split renderer:
* ReDMCSB F0115 draws objects, creatures, and projectiles while consuming each
  packed cell order; explosions restart only after every packed cell finishes.
* Door-front squares split F0115 around door frame/panel: rear cells first,
  then frame/door, then front cells.
* Firestaff keeps projectile drawing in the side/center contents passes, keeps
  explosions out of those passes, and routes explosions through the deferred
  after-all-cells pass with center/side occlusion guards.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C"
VIEW = ROOT / "m11_game_view.c"
META = ROOT / "dm1_v1_viewport_3d_pc34_compat.c"


def fail(message: str) -> int:
    print(f"status=FAIL_PASS405_DM1_V1_VIEWPORT_PROJECTILE_EXPLOSION_LAYER_OCCLUSION reason={message}")
    return 1


def line_slice(text: str, start: int, end: int) -> str:
    lines = text.splitlines()
    if end > len(lines):
        raise AssertionError(f"source has only {len(lines)} lines, need {end}")
    return "\n".join(lines[start - 1:end])


def compact(text: str) -> str:
    return " ".join(text.split())


def require_contains(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise AssertionError(f"{label}: missing {needle!r}")


def require_all(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    for needle in needles:
        if compact(needle) not in flat:
            raise AssertionError(f"{label}: missing {needle!r}")


def require_order(text: str, needles: list[str], label: str) -> None:
    pos = -1
    for needle in needles:
        at = text.find(needle)
        if at < 0:
            raise AssertionError(f"{label}: missing {needle!r}")
        if at <= pos:
            raise AssertionError(f"{label}: {needle!r} is out of order")
        pos = at


def function_body(text: str, signature: str) -> str:
    search_from = 0
    while True:
        start = text.find(signature, search_from)
        if start < 0:
            raise AssertionError(f"missing function signature {signature!r}")
        brace = text.find("{", start)
        if brace < 0:
            raise AssertionError(f"missing body for {signature!r}")
        semi = text.find(";", start, brace)
        if semi >= 0:
            search_from = semi + 1
            continue
        depth = 0
        for idx in range(brace, len(text)):
            ch = text[idx]
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return text[start:idx + 1]
        raise AssertionError(f"unterminated body for {signature!r}")


def verify_redmcsb(red: str) -> None:
    require_all(line_slice(red, 4547, 4582), [
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "Draw one creature at the cell being processed",
        "Draw only projectiles at specified cell",
        "Draw only explosions at specified cell",
    ], "ReDMCSB DUNVIEW.C:4547-4582 F0115 contract")
    require_all(line_slice(red, 4820, 4860), [
        "/* Draw objects */",
        "P0141_T_Thing = L0146_T_FirstThingToDraw;",
        "M011_CELL(P0141_T_Thing) == L0139_i_Cell",
    ], "ReDMCSB DUNVIEW.C:4820-4860 object pass")
    require_all(line_slice(red, 5195, 5202), [
        "/* Draw creatures */",
        "L0168_B_DrawingLastBackRowCell",
    ], "ReDMCSB DUNVIEW.C:5195-5202 creature pass")
    require_all(line_slice(red, 5679, 5683), [
        "Restart processing list of objects from the beginning",
        "C14_THING_TYPE_PROJECTILE",
        "M011_CELL(P0141_T_Thing) == L0139_i_Cell",
    ], "ReDMCSB DUNVIEW.C:5679-5683 projectile restart")
    require_all(line_slice(red, 5915, 5933), [
        "} while (L0130_ul_RemainingViewCellOrdinalsToProcess);",
        "/* Draw explosions */",
        "Restart processing list of things from the beginning",
        "C15_THING_TYPE_EXPLOSION",
    ], "ReDMCSB DUNVIEW.C:5915-5933 deferred explosion pass")

    door_cases = [
        ("D3L", 6443, 6459, "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT", "F0111_DUNGEONVIEW_DrawDoor", "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT"),
        ("D3C", 6722, 6746, "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT", "F0111_DUNGEONVIEW_DrawDoor", "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT"),
        ("D2C", 7314, 7341, "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT", "F0111_DUNGEONVIEW_DrawDoor", "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT"),
        ("D1C", 7874, 7937, "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT", "F0111_DUNGEONVIEW_DrawDoor", "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT"),
    ]
    for name, start, end, rear, door, front in door_cases:
        excerpt = line_slice(red, start, end)
        require_order(excerpt, [rear, door, front], f"ReDMCSB {name} door-front split {start}-{end}")


def verify_metadata(meta: str) -> None:
    require_order(meta, [
        "DM1_VIEWPORT_THING_LAYER_OBJECTS",
        "DM1_VIEWPORT_THING_LAYER_CREATURES",
        "DM1_VIEWPORT_THING_LAYER_PROJECTILES",
        "DM1_VIEWPORT_THING_LAYER_EXPLOSIONS",
    ], "Firestaff F0115 layer metadata order")
    for needle in [
        "{ DM1_VIEWPORT_THING_LAYER_PROJECTILES, \"projectiles\", \"DUNVIEW.C:4575-4577,5681-5883\", true,  false }",
        "{ DM1_VIEWPORT_THING_LAYER_EXPLOSIONS,  \"explosions\",  \"DUNVIEW.C:4579-4581,5915-5933\", false, true  }",
        "DM1_VIEW_SQUARE_D3L, 0x0218, 0x0349",
        "DM1_VIEW_SQUARE_D3C, 0x0218, 0x0349",
        "DM1_VIEW_SQUARE_D2C, 0x0218, 0x0349",
        "DM1_VIEW_SQUARE_D1C, 0x0218, 0x0349",
        "DUNVIEW.C:7874-7875 pass1 rear cells before frame",
        "DUNVIEW.C:7910-7937 pass2 front cells after door",
    ]:
        require_contains(meta, needle, "Firestaff occlusion/layer metadata")


def verify_renderer(view: str) -> None:
    wall = function_body(view, "static void m11_draw_wall_contents(")
    require_order(wall, [
        "Layer 0: Floor ornaments",
        "Layer 1: Floor items",
        "Layer 2: Creatures",
        "m11_draw_effect_cue(",
    ], "center contents object/creature/projectile stack")

    effect = function_body(view, "static void m11_draw_effect_cue(")
    require_contains(effect, "cell->summary.projectiles > 0", "center effect cue")
    require_contains(effect, "m11_draw_projectile_sprite", "center effect cue")
    require_contains(effect, "Explosions are intentionally not drawn here", "center effect cue")
    if "m11_draw_explosion_sprite" in effect or "cell->summary.explosions > 0" in effect:
        raise AssertionError("center effect cue must not draw explosions inline")

    side = function_body(view, "static void m11_draw_dm1_side_contents(")
    require_order(side, [
        "m11_draw_floor_ornament(",
        "cell->floorItemCount > 0",
        "cell->creatureGroupCount > 0",
        "cell->summary.projectiles > 0",
        "Explosions are deferred to m11_draw_dm1_deferred_explosion_pass()",
    ], "side contents object/creature/projectile/defer stack")
    if "m11_draw_explosion_sprite" in side or "m11_draw_explosion_cue" in side:
        raise AssertionError("side contents must not draw explosions inline")

    deferred = function_body(view, "static void m11_draw_dm1_deferred_explosion_pass(")
    require_all(deferred, [
        "DUNVIEW.C:5915 exits the packed-cell",
        "DUNVIEW.C:5916-5933 starts",
        "m11_dm1_nearest_blocking_center_depth_index(cells)",
        "m11_dm1_center_line_clear_before_depth(cells, depth)",
        "m11_draw_dm1_deferred_center_explosion",
        "blockingCenterDepth >= 0 && depth >= blockingCenterDepth",
        "m11_dm1_side_lane_clear_before_depth(cells, depth, sideIndex)",
        "m11_draw_dm1_deferred_side_explosion",
    ], "deferred explosion pass occlusion guards")

    viewport = function_body(view, "static void m11_draw_viewport(")
    require_order(viewport, [
        "m11_draw_dm1_center_doors(",
        "m11_draw_dm1_side_contents(",
        "m11_draw_wall_contents(framebuffer",
        "m11_draw_dm1_deferred_explosion_pass(",
    ], "viewport wall/door -> projectile contents -> deferred explosions order")


def main() -> int:
    try:
        red = REDMCSB.read_text(encoding="latin-1")
        view = VIEW.read_text(encoding="utf-8")
        meta = META.read_text(encoding="utf-8")
        verify_redmcsb(red)
        verify_metadata(meta)
        verify_renderer(view)
        print("status=PASS_PASS405_DM1_V1_VIEWPORT_PROJECTILE_EXPLOSION_LAYER_OCCLUSION")
        print("source=DUNVIEW.C:4547-4582,4820-4860,5195-5202,5679-5683,5915-5933")
        print("door_occlusion=DUNVIEW.C:6443-6459,6722-6746,7314-7341,7874-7937")
        print("firestaff=m11_draw_wall_contents/m11_draw_dm1_side_contents projectiles before deferred m11_draw_dm1_deferred_explosion_pass")
        return 0
    except (AssertionError, OSError) as exc:
        return fail(str(exc))


if __name__ == "__main__":
    sys.exit(main())
