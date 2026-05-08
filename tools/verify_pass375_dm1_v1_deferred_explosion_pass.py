#!/usr/bin/env python3
"""Verify DM1 V1 deferred after-all-packed-cells explosion pass.

ReDMCSB F0115 draws objects/creatures/projectiles while consuming packed
view-cell ordinals, exits that loop at DUNVIEW.C:5915, then restarts the
thing list to draw explosions at DUNVIEW.C:5916-5933.  This gate keeps the
Firestaff M11 viewport implementation locked to that separation: no explosion
bitmap/cue inside per-cell effect drawing; all explosions route through the
single deferred pass called after side and center contents.
"""
from __future__ import annotations

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
VIEW = ROOT / "m11_game_view.c"
EVIDENCE = ROOT / "parity-evidence/pass375_dm1_v1_deferred_explosion_pass.md"
REDMCSB = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C").expanduser()


def fail(message: str) -> int:
    print(f"status=FAIL_PASS375_DM1_V1_DEFERRED_EXPLOSION_PASS reason={message}")
    return 1


def line_no(text: str, needle: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"missing {needle!r}")
    return text.count("\n", 0, pos) + 1


def body_span(text: str, signature: str) -> tuple[int, str]:
    start = text.find(signature)
    if start < 0:
        raise AssertionError(f"missing function {signature!r}")
    brace = text.find("{", start)
    if brace < 0:
        raise AssertionError(f"missing body for {signature!r}")
    depth = 0
    for idx in range(brace, len(text)):
        ch = text[idx]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return start, text[start:idx + 1]
    raise AssertionError(f"unterminated body for {signature!r}")


def require_order(text: str, first: str, second: str, label: str) -> None:
    p1 = text.find(first)
    p2 = text.find(second)
    if p1 < 0:
        raise AssertionError(f"{label}: missing {first!r}")
    if p2 < 0:
        raise AssertionError(f"{label}: missing {second!r}")
    if p1 >= p2:
        raise AssertionError(f"{label}: {first!r} must appear before {second!r}")


def main() -> int:
    if not REDMCSB.exists():
        return fail(f"missing ReDMCSB DUNVIEW.C at {REDMCSB}")
    red = REDMCSB.read_text(encoding="latin-1")
    view = VIEW.read_text(encoding="utf-8")
    evidence = EVIDENCE.read_text(encoding="utf-8")

    anchors = {
        "f0115_signature": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "projectile_contract": "Draw only projectiles at specified cell",
        "per_cell_loop_end": "} while (L0130_ul_RemainingViewCellOrdinalsToProcess);",
        "explosion_phase": "/* Draw explosions */",
        "explosion_restart": "P0141_T_Thing = L0146_T_FirstThingToDraw; /* Restart processing list of things from the beginning. The next loop draws only explosion things among the list */",
        "explosion_type": "C15_THING_TYPE_EXPLOSION",
    }
    lines = {name: line_no(red, needle) for name, needle in anchors.items()}
    require_order(red, anchors["projectile_contract"], anchors["per_cell_loop_end"], "ReDMCSB projectile before packed-cell loop end")
    require_order(red, anchors["per_cell_loop_end"], anchors["explosion_phase"], "ReDMCSB explosion after packed-cell loop")
    require_order(red, anchors["explosion_phase"], anchors["explosion_restart"], "ReDMCSB restarts thing list for explosion pass")
    restart_pos = red.find(anchors["explosion_restart"])
    type_after_restart = red.find(anchors["explosion_type"], restart_pos)
    if restart_pos < 0 or type_after_restart < 0:
        raise AssertionError("ReDMCSB explosion-only thing filter missing after restart")

    _, effect_body = body_span(view, "static void m11_draw_effect_cue(")
    _, side_body = body_span(view, "static void m11_draw_dm1_side_contents(")
    _, deferred_body = body_span(view, "static void m11_draw_dm1_deferred_explosion_pass(")
    _, viewport_body = body_span(view, "static void m11_draw_viewport(")
    _, explosion_cue_body = body_span(view, "static void m11_draw_explosion_cue(")

    if "cell->summary.projectiles > 0" not in effect_body:
        raise AssertionError("per-cell effect cue lost projectile path")
    for forbidden in ["m11_draw_explosion_sprite", "cell->summary.explosions > 0) {"]:
        if forbidden in effect_body:
            raise AssertionError(f"per-cell effect cue still draws explosions via {forbidden!r}")
    if "Explosions are intentionally not drawn here" not in effect_body:
        raise AssertionError("per-cell effect cue lacks explicit ReDMCSB deferred-pass marker")

    projectile_pos = side_body.find("cell->summary.projectiles > 0")
    deferred_comment_pos = side_body.find("Explosions are deferred to m11_draw_dm1_deferred_explosion_pass")
    if projectile_pos < 0 or deferred_comment_pos < 0 or projectile_pos >= deferred_comment_pos:
        raise AssertionError("side contents must draw projectiles before deferring explosions")
    if "m11_draw_explosion_sprite" in side_body or "m11_draw_explosion_cue" in side_body:
        raise AssertionError("side contents still draws explosions inline")

    for required in [
        "DUNVIEW.C:5915",
        "DUNVIEW.C:5916-5933",
        "m11_draw_dm1_deferred_center_explosion",
        "m11_draw_dm1_deferred_side_explosion",
        "m11_draw_explosion_cue",
    ]:
        if required not in deferred_body and required not in view:
            raise AssertionError(f"deferred pass missing {required!r}")
    if "m11_draw_explosion_sprite" not in explosion_cue_body:
        raise AssertionError("deferred explosion cue must keep source-backed bitmap path")
    require_order(viewport_body, "m11_draw_dm1_side_contents(", "m11_draw_dm1_deferred_explosion_pass(", "viewport side contents before deferred explosions")
    require_order(viewport_body, "m11_draw_wall_contents(framebuffer", "m11_draw_dm1_deferred_explosion_pass(", "viewport center contents before deferred explosions")

    for required in [
        "DUNVIEW.C:5915-5933",
        "after-all-packed-cells explosion pass",
        "m11_draw_dm1_deferred_explosion_pass()",
        "m11_draw_effect_cue() no longer draws explosions",
    ]:
        if required not in evidence:
            raise AssertionError(f"evidence missing {required!r}")

    print("status=PASS_PASS375_DM1_V1_DEFERRED_EXPLOSION_PASS")
    print(f"redmcsb_f0115_signature_line={lines['f0115_signature']}")
    print(f"redmcsb_projectile_contract_line={lines['projectile_contract']}")
    print(f"redmcsb_packed_cell_loop_end_line={lines['per_cell_loop_end']}")
    print(f"redmcsb_explosion_phase_line={lines['explosion_phase']}")
    print(f"redmcsb_explosion_restart_line={lines['explosion_restart']}")
    print("implementation=m11_draw_dm1_deferred_explosion_pass after side+center contents")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        raise SystemExit(fail(str(exc)))
