#!/usr/bin/env python3
"""Verify pass374 DM1 V1 F0115 explosion-pass blocker evidence.

This is intentionally a blocker-narrowing source-lock gate, not a renderer
implementation. ReDMCSB F0115 draws objects/creatures/projectiles per packed
view cell, then restarts the thing list once to draw explosions after all packed
cells. Current M11 still draws projectiles and explosions together inside the
per-open-cell effect cue; this verifier keeps that mismatch explicit until a
safe deferred explosion pass lands.
"""
from __future__ import annotations

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
VIEW = ROOT / "src/engine/m11_game_view.c"
EVIDENCE = ROOT / "parity-evidence/pass374_dm1_v1_f0115_explosion_pass_blocker.md"
REDMCSB = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C").expanduser()


def fail(message: str) -> int:
    print(f"status=FAIL_PASS374_DM1_V1_F0115_EXPLOSION_PASS_BLOCKER reason={message}")
    return 1


def line_no(text: str, needle: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"missing {needle!r}")
    return text.count("\n", 0, pos) + 1


def function_body_span(text: str, signature: str) -> tuple[int, str]:
    search_from = 0
    while True:
        start = text.find(signature, search_from)
        if start < 0:
            raise AssertionError(f"missing function body {signature!r}")
        brace = text.find("{", start)
        semicolon = text.find(";", start)
        if brace >= 0 and (semicolon < 0 or brace < semicolon):
            break
        search_from = start + len(signature)
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


def body_for_function(text: str, signature: str) -> str:
    return function_body_span(text, signature)[1]


def line_for_offset(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


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
    if ("m11_draw_dm1_deferred_explosion_pass" in view and
            "Explosions are intentionally not drawn here" in view and
            "m11_draw_effect_cue" in view):
        print("status=PASS_PASS374_DM1_V1_F0115_EXPLOSION_PASS_BLOCKER_SUPERSEDED")
        print("blocker=superseded_by_pass375_deferred_after_all_packed_cells_explosion_pass")
        return 0
    anchors = {
        "f0115_signature": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "packed_cell_contract": "The remaining nibbles contain ordinals of square view cells to draw",
        "projectile_contract": "Draw only projectiles at specified cell",
        "per_cell_loop_end": "} while (L0130_ul_RemainingViewCellOrdinalsToProcess);",
        "explosion_phase": "/* Draw explosions */",
        "explosion_restart": "P0141_T_Thing = L0146_T_FirstThingToDraw; /* Restart processing list of things from the beginning. The next loop draws only explosion things among the list */",
        "explosion_type": "C15_THING_TYPE_EXPLOSION",
    }
    lines = {name: line_no(red, needle) for name, needle in anchors.items()}
    require_order(red, anchors["projectile_contract"], anchors["per_cell_loop_end"], "ReDMCSB projectile before end of packed-cell loop")
    require_order(red, anchors["per_cell_loop_end"], anchors["explosion_phase"], "ReDMCSB explosions after all packed cells")
    require_order(red, anchors["explosion_phase"], anchors["explosion_restart"], "ReDMCSB explosion restart")

    effect_start, effect_body = function_body_span(view, "static void m11_draw_effect_cue(")
    wall_start, wall_body = function_body_span(view, "static void m11_draw_wall_contents(")
    effect_line = line_for_offset(view, effect_start)
    wall_line = line_for_offset(view, wall_start)
    call_line = line_no(view, "m11_draw_effect_cue(framebuffer, framebufferWidth, framebufferHeight,")

    for needle in [
        "cell->summary.projectiles > 0",
        "m11_draw_projectile_sprite",
        "cell->summary.explosions > 0",
        "m11_draw_explosion_sprite",
    ]:
        if needle not in effect_body:
            raise AssertionError(f"M11 effect cue no longer has expected blocker marker {needle!r}")
    require_order(effect_body, "cell->summary.projectiles > 0", "cell->summary.explosions > 0", "M11 coalesced cue order")
    if "m11_draw_effect_cue(framebuffer" not in wall_body:
        raise AssertionError("M11 wall contents no longer calls the coalesced per-cell effect cue")

    for required in [
        "DUNVIEW.C:4547-4582",
        "DUNVIEW.C:5645-5683",
        "DUNVIEW.C:5915-5933",
        "m11_draw_effect_cue()",
        "coalesces projectiles and explosions per open cell",
        "remaining blocker",
    ]:
        if required not in evidence:
            raise AssertionError(f"evidence missing {required!r}")
    print("status=PASS_PASS374_DM1_V1_F0115_EXPLOSION_PASS_BLOCKER")
    print(f"redmcsb_f0115_signature_line={lines['f0115_signature']}")
    print(f"redmcsb_projectile_contract_line={lines['projectile_contract']}")
    print(f"redmcsb_packed_cell_loop_end_line={lines['per_cell_loop_end']}")
    print(f"redmcsb_explosion_phase_line={lines['explosion_phase']}")
    print(f"redmcsb_explosion_restart_line={lines['explosion_restart']}")
    print(f"m11_effect_cue_line={effect_line}")
    print(f"m11_wall_contents_line={wall_line}")
    print(f"m11_effect_call_line={call_line}")
    print("blocker=M11 effect cue still coalesces projectiles/explosions per open cell; safe fix needs a deferred after-all-packed-cells explosion pass matching F0115")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        raise SystemExit(fail(str(exc)))
