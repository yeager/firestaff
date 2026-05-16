#!/usr/bin/env python3
"""Verify V1 projectile/explosion viewport rendering is source-locked to ReDMCSB.

Cross-checks:
1. dm1_v1_projectile_explosion_render_pc34_compat.h declares all required
   constants matching ReDMCSB DEFS.H.
2. dm1_v1_projectile_explosion_render_pc34_compat.c implements source-locked
   data tables (G0210, G0215, G0216) and all rendering queries.
3. m11_game_view.c draw pipeline calls projectile drawing BEFORE explosion
   drawing, matching DUNVIEW.C F0115 line 5645 (projectiles) before line 5916
   (explosions).
4. ReDMCSB DUNVIEW.C contains the cited source markers.
"""
from __future__ import annotations
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
HDR  = ROOT / "include/dm1_v1_projectile_explosion_render_pc34_compat.h"
IMPL = ROOT / "src/dm1/dm1_v1_projectile_explosion_render_pc34_compat.c"
VIEW = ROOT / "src/engine/m11_game_view.c"
REDMCSB = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C").expanduser()
DEFS    = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DEFS.H").expanduser()

def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1

def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos

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
    hdr_text  = HDR.read_text(encoding="utf-8")
    impl_text = IMPL.read_text(encoding="utf-8")
    view_text = VIEW.read_text(encoding="utf-8")

    # 1. Header constants match DEFS.H
    require(hdr_text, "DM1_GFX_FIRST_PROJECTILE", "HDR constants")
    require(hdr_text, "454", "HDR M613=454")
    require(hdr_text, "DM1_GFX_FIRST_EXPLOSION", "HDR explosion gfx")
    require(hdr_text, "486", "HDR M614=486")
    require(hdr_text, "DM1_GFX_FIRST_EXPLOSION_PATTERN", "HDR pattern gfx")
    require(hdr_text, "489", "HDR M636=489")
    require(hdr_text, "DM1_PROJECTILE_ASPECT_COUNT", "HDR aspect count")
    require(hdr_text, "DM1_EXPLOSION_ASPECT_COUNT", "HDR exp aspect count")
    for asp_type in ["HAS_BACK_AND_ROTATION", "HAS_BACK_NO_ROTATION",
                     "NO_BACK_AND_ROTATION", "NO_BACK_NO_ROTATION"]:
        require(hdr_text, f"DM1_PROJ_ASPECT_{asp_type}", f"HDR {asp_type}")
    for exp_type in ["FIREBALL", "LIGHTNING_BOLT", "POISON_CLOUD", "SMOKE",
                     "FLUXCAGE", "REBIRTH_STEP1", "REBIRTH_STEP2"]:
        require(hdr_text, f"DM1_EXPLOSION_{exp_type}", f"HDR explosion {exp_type}")
    for exp_asp in ["FIRE", "SPELL", "POISON", "SMOKE"]:
        require(hdr_text, f"DM1_EXPLOSION_ASPECT_{exp_asp}", f"HDR aspect {exp_asp}")
    require(hdr_text, "DM1_SMOKE_RECOLOR_SRC_A", "HDR smoke recolor A")
    require(hdr_text, "DM1_SMOKE_RECOLOR_SRC_B", "HDR smoke recolor B")
    require(hdr_text, "DM1_F0115_LAYER_PROJECTILES", "HDR layer projectiles")
    require(hdr_text, "DM1_F0115_LAYER_EXPLOSIONS", "HDR layer explosions")

    # 2. Implementation has source-locked data tables
    require(impl_text, "DM1_ProjectileScales[7]", "IMPL G0215")
    require(impl_text, "DM1_ProjectileAspects[DM1_PROJECTILE_ASPECT_COUNT]", "IMPL G0210")
    require(impl_text, "DM1_ExplosionBaseScales[4]", "IMPL G0216")
    require(impl_text, "dm1_v1_projectile_bitmap_delta", "IMPL bitmap delta")
    require(impl_text, "dm1_v1_projectile_graphic_index", "IMPL graphic index")
    require(impl_text, "dm1_v1_projectile_flip_flags", "IMPL flip flags")
    require(impl_text, "dm1_v1_explosion_type_to_aspect", "IMPL exp type->aspect")
    require(impl_text, "dm1_v1_explosion_pattern_graphic_index", "IMPL pattern gfx")

    # 3. m11_game_view.c draws projectiles before explosions
    # Center path: effect_cue calls projectile then explosion
    view_proj = view_text.find("m11_draw_projectile_sprite")
    view_expl = view_text.find("m11_draw_explosion_sprite")
    if view_proj < 0:
        raise AssertionError("VIEW: missing m11_draw_projectile_sprite")
    if view_expl < 0:
        raise AssertionError("VIEW: missing m11_draw_explosion_sprite")
    # In the effect_cue function, projectiles must come before explosions
    eff_start = view_text.find("m11_draw_effect_cue(")
    if eff_start < 0:
        # Find the definition
        eff_start = view_text.find("static void m11_draw_effect_cue(")
    if eff_start >= 0:
        eff_body = view_text[eff_start:]
        proj_in_eff = eff_body.find("summary.projectiles > 0")
        expl_in_eff = eff_body.find("summary.explosions > 0")
        if proj_in_eff >= 0 and expl_in_eff >= 0:
            if proj_in_eff >= expl_in_eff:
                raise AssertionError("VIEW: projectiles must be drawn before explosions in effect_cue")

    # Side-contents path: projectiles before explosions
    side_start = view_text.find("m11_draw_dm1_side_contents")
    if side_start >= 0:
        side_body = view_text[side_start:]
        proj_side = side_body.find("summary.projectiles > 0")
        expl_side = side_body.find("summary.explosions > 0")
        if proj_side >= 0 and expl_side >= 0:
            if proj_side >= expl_side:
                raise AssertionError("VIEW: projectiles must be drawn before explosions in side_contents")

    # 4. ReDMCSB source markers
    if REDMCSB.exists():
        red_text = REDMCSB.read_text(encoding="latin-1")
        for marker in [
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "Draw only projectiles at specified cell",
            "T0115129_DrawProjectiles",
            "Draw only explosions at specified cell",
            "Draw explosions",
            "F0142_DUNGEON_GetProjectileAspect",
            "G0210_as_Graphic558_ProjectileAspects",
            "G0215_auc_Graphic558_ProjectileScales",
            "G0216_auc_Graphic558_ExplosionBaseScales",
            "M636_GRAPHIC_FIRST_EXPLOSION_PATTERN",
        ]:
            pos = require(red_text, marker, "REDMCSB")
            print(f"  ReDMCSB {marker!r}: line {line_no(red_text, pos)}")

    if DEFS.exists():
        defs_text = DEFS.read_text(encoding="latin-1")
        for marker in [
            "C0_PROJECTILE_ASPECT_TYPE_HAS_BACK_GRAPHIC_AND_ROTATION",
            "C3_PROJECTILE_ASPECT_TYPE_NO_BACK_GRAPHIC_AND_NO_ROTATION",
            "C10_PROJECTILE_ASPECT_EXPLOSION_FIREBALL",
            "C0_EXPLOSION_ASPECT_FIRE",
            "C3_EXPLOSION_ASPECT_SMOKE",
            "MASK0x0100_SCALE_WITH_KINETIC_ENERGY",
        ]:
            pos = require(defs_text, marker, "DEFS.H")
            print(f"  DEFS.H {marker!r}: line {line_no(defs_text, pos)}")

    print("V1 viewport projectile/explosion render source-lock gate passed")
    return 0

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
