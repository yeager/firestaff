#!/usr/bin/env python3
"""Keep unowned DM2 placeholder/transcript modules out of product archives.

The DM2 source tree intentionally retains several narrow compatibility studies
and V2 fixture readers for direct regression work.  They are not runtime
owners: their inputs are caller-authored, fixture-only, or lack the original
GAME_LOAD/GDAT transaction.  CMake's broad globs must therefore continue to
exclude them until a source-backed replacement is wired into M11.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path


EXPECTED_REMOVALS = {
    "M10_SOURCES": {
        "dm2_v1_1c9a_pc34_compat.c",
        "dm2_v1_0aaf_pc34_compat.c",
        "dm2_v1_runtime_narrow_pc34_compat.c",
        "dm2_v1_runtime_parity_pc34_compat.c",
        "dm2_v1_load_orchestrator_pc34_compat.c",
        "dm2_v1_save_orchestrator_pc34_compat.c",
        "dm2_v1_save_load_extra_dungeon_data_pc34_compat.c",
        "dm2_v1_save_store_extra_dungeon_data_pc34_compat.c",
        "dm2_v1_save_read_record_checkcode_pc34_compat.c",
        "dm2_v1_save_write_record_checkcode_pc34_compat.c",
        "dm2_v1_gui_draw_pc34_compat.c",
        "dm2_v1_gui_vp_pc34_compat.c",
        "dm2_v1_querydb_pc34_compat.c",
        "dm2_v1_gdatfile_pc34_compat.c",
        "dm2_v1_sfx_pc34_compat.c",
        "dm2_v1_ccm.c",
    },
    "DM2_SOURCES": {
        "dm2_v1_hud_panel_routing.c",
        "dm2_v1_hud_survey_helpers.c",
        "dm2_v1_combat.c",
        "dm2_v1_tech_magic.c",
        "dm2_v1_record_name_helper.c",
        "dm2_v1_source_name_helpers.c",
        "dm2_v1_ui_event_name_helper.c",
        "dm2_v1_champion_hud_helpers.c",
        "dm2_v1_food_water_bridge.c",
        "dm2_v1_outdoor_renderer.c",
        "dm2_v1_1c9a_pc34_compat.c",
        "dm2_v1_0aaf_pc34_compat.c",
        "dm2_v1_runtime_narrow_pc34_compat.c",
        "dm2_v1_runtime_parity_pc34_compat.c",
        "dm2_v1_load_orchestrator_pc34_compat.c",
        "dm2_v1_save_orchestrator_pc34_compat.c",
        "dm2_v1_save_load_extra_dungeon_data_pc34_compat.c",
        "dm2_v1_save_store_extra_dungeon_data_pc34_compat.c",
        "dm2_v1_save_read_record_checkcode_pc34_compat.c",
        "dm2_v1_save_write_record_checkcode_pc34_compat.c",
        "dm2_v1_gui_draw_pc34_compat.c",
        "dm2_v1_gui_vp_pc34_compat.c",
        "dm2_v1_querydb_pc34_compat.c",
        "dm2_v1_gdatfile_pc34_compat.c",
        "dm2_v1_sfx_pc34_compat.c",
        "dm2_v1_ccm.c",
        "dm2_v1_world_state.c",
        "dm2_v1_object_model.c",
        "dm2_v1_gfx_str_pc34_compat.c",
        "dm2_v1_graphics_data_open.c",
        "dm2_v1_creature_attacks_player_pc34_compat.c",
        "dm2_v1_creature_combat_pc34_compat.c",
        "dm2_v1_creature_ai_spec_pc34_compat.c",
        "dm2_v1_anim_chunk_pc34_compat.c",
        "dm2_v1_projectile_pc34_compat.c",
        "dm2_v1_projectile_step_pc34_compat.c",
        "dm2_v1_projectile_creature_collision_pc34_compat.c",
        "dm2_v1_anim_bootstrap.c",
        "dm2_v1_fmtowns_disc.c",
        "dm2_v1_shop.c",
        "dm2_v1_shop_npc_pc34_compat.c",
        "dm2_v1_inventory_panel.c",
        "dm2_v1_tim_proc_pc34_compat.c",
        "dm2_v1_timer_ops_pc34_compat.c",
        "dm2_v1_timer_dispatch_wiring_pc34_compat.c",
    },
    "DM2_V2_SOURCES": {
        "dm2_v2_hud_widget_bitmap_blit.c",
        "dm2_v2_hud_widget_assets.c",
        "dm2_v2_hud_overlay.c",
        "dm2_v2_touch_runtime.c",
        "dm2_v2_touch_controller_affordance.c",
        "dm2_v2_companion_ui.c",
        "dm2_v2_tech_crafting.c",
        "dm2_v2_asset_pipeline.c",
        "dm2_v2_interaction_feedback.c",
        "dm2_v2_lighting.c",
        "dm2_v2_lighting_runtime.c",
        "dm2_v2_outdoor_enhanced.c",
        "dm2_v2_runtime.c",
        "dm2_v2_smooth_movement.c",
        "dm2_v2_viewport_renderer.c",
        "dm2_v22_finished_art_material_gate_pc34.c",
        "dm2_v22_inplace_draw_pc34.c",
        "dm2_v22_modern_assets_pc34.c",
        "dm2_v22_shape_cache_pc34.c",
        "dm2_v22_viewport_swap_pc34.c",
    },
}


# These are observability fields, not a renderer escape hatch.  The live
# viewport clears them at the beginning of a frame and its material gates must
# either consume a source-owned bitmap or report the blocked material.  A
# future increment or non-zero assignment would mean an unowned draw path has
# been reintroduced while callers still rely on a zero total as their
# no-placeholder receipt.
VIEWPORT_FALLBACK_COUNTERS = (
    "fallback_floor_ceiling_drawn_count",
    "fallback_wall_drawn_count",
    "fallback_wall_ornament_drawn_count",
    "fallback_door_drawn_count",
    "fallback_creature_drawn_count",
    "fallback_item_drawn_count",
    "fallback_creature_possession_item_drawn_count",
    "fallback_carried_item_drawn_count",
    "fallback_projectile_drawn_count",
    "fallback_hud_core_drawn_count",
    "fallback_hud_portrait_drawn_count",
)


def removed_files(cmake: str, variable: str) -> set[str]:
    """Extract all quoted source names from list(REMOVE_ITEM <variable> ...)."""
    found: set[str] = set()
    marker = f"list(REMOVE_ITEM {variable}"
    cursor = 0
    while (start := cmake.find(marker, cursor)) >= 0:
        depth = 0
        end = start
        for end in range(start, len(cmake)):
            if cmake[end] == "(":
                depth += 1
            elif cmake[end] == ")":
                depth -= 1
                if depth == 0:
                    break
        if depth != 0:
            raise ValueError(f"unterminated {marker} block")
        found.update(re.findall(r"src/dm2/([^\"/]+\.c)", cmake[start:end + 1]))
        cursor = end + 1
    return found


def verify(repo: Path) -> list[str]:
    errors: list[str] = []
    cmake_path = repo / "CMakeLists.txt"
    if not cmake_path.exists():
        return [f"missing {cmake_path}"]
    cmake = cmake_path.read_text(encoding="utf-8")
    for variable, expected in EXPECTED_REMOVALS.items():
        actual = removed_files(cmake, variable)
        missing = sorted(expected - actual)
        if missing:
            errors.append(
                f"{variable} no longer excludes: {', '.join(missing)}")
        for filename in sorted(expected):
            if not (repo / "src/dm2" / filename).exists():
                errors.append(f"inventory source missing: src/dm2/{filename}")
    required_guards = (
        'state->presentationMode = M12_PRESENTATION_V21_UPSCALED;',
        'dm2_v1_boot_startup_launch_alloc_with_language(',
    )
    m11 = (repo / "src/engine/m11_game_view.c").read_text(encoding="utf-8")
    for guard in required_guards:
        if guard not in m11:
            errors.append(f"M11 source gate missing: {guard}")

    viewport_path = repo / "src/dm2/dm2_v1_viewport_renderer.c"
    if not viewport_path.exists():
        errors.append(f"missing {viewport_path}")
        return errors
    viewport = viewport_path.read_text(encoding="utf-8")
    if "dm2_v1_block_source_material" not in viewport:
        errors.append("viewport source-material block gate missing")
    for forbidden in (
            "dm2_populate_view_squares",
            "alternate wall sets for visual variety",
            "s->source_materials_required ? 0u : 15u",
    ):
        if forbidden in viewport:
            errors.append(f"viewport retains synthetic world fallback: {forbidden}")
    for counter in VIEWPORT_FALLBACK_COUNTERS:
        assignments = re.findall(
            rf"\bs->\s*{counter}\s*(?:=|\+=|-=|\+\+|--)", viewport)
        reset = re.findall(
            rf"\bs->\s*{counter}\s*=\s*0\s*;", viewport)
        if len(assignments) != 1 or len(reset) != 1:
            errors.append(
                f"viewport fallback counter is not reset-only: {counter}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path,
                        default=Path(__file__).resolve().parents[1])
    args = parser.parse_args()
    errors = verify(args.repo)
    if errors:
        for error in errors:
            print(f"FAIL: {error}")
        return 1
    print("PASS: DM2 production archives exclude every inventoried placeholder/transcript module")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
