#!/usr/bin/env python3
"""Build and verify the DM1 V2 completion matrix.

This gate is deliberately practical: it inventories every top-level dm1_v2_*.c
module, classifies ownership, checks that the current critical V2 gates are wired
into CTest, and writes a JSON evidence file for the next implementation passes.
It does not claim V2 is done; it prevents blind V2 work by making the gaps
explicit and versioned.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v2_completion_matrix.json"

REQUIRED_MODULES = {
    "src/dm1v2/dm1_v2_runtime_pc34.c": "runtime/shell",
    "src/dm1v2/dm1_v2_movement_engine_pc34.c": "runtime/movement",
    "src/dm1v2/dm1_v2_viewport_renderer_pc34.c": "runtime/viewport",
    "src/dm1v2/dm1_v2_texture_upscale_pc34.c": "asset/upscale",
    "src/dm1v2/dm1_v2_hud_overlay_pc34.c": "ui/hud",
    "src/dm1v2/dm1_v2_lighting_dynamic_pc34.c": "visual/lighting",
    "src/dm1v2/dm1_v2_particle_system_pc34.c": "visual/effects",
}

REQUIRED_CTEST_NAMES = {
    "dm1_v2_upscale_dry_run_validator",
    "dm1_v2_asset_manifest_validator",
    "dm1_v2_asset_manifest_validator_self_test",
    "dm1_v2_movement_viewport_pc34",
    "dm1_v2_runtime_shell_pc34",
    "dm1_v2_runtime_shell_source_lock",
    "dm1_v2_launch_smoke_pc34",
    "dm1_v2_launch_smoke_source_lock",
    "dm1_v2_viewport_wall_occlusion_source_lock",
    "dm1_v2_viewport_d1c_door_occlusion_source_lock",
    "dm1_v2_dungeon_view_asset_bindings_source_lock",
    "dm1_v2_viewport_composition_source_lock",
    "dm1_v2_d0_d3_draw_list_comparator_gate",
    "dm1_v2_viewport_pixel_capture_fixture_gate",
    "dm1_v2_entry_viewport_png_export",
    "dm1_v2_entry_viewport_png_export_gate",
    "dm1_v2_entry_viewport_png_comparator_gate",
    "dm1_v2_entry_bitmap_materialization_blocker_gate",
    "dm1_v2_hud_overlay_pc34",
    "dm1_v2_hud_overlay_source_lock",
    "dm1_v2_lighting_dynamic_pc34",
    "dm1_v2_lighting_dynamic_source_lock",
    "dm1_v2_settings_pc34",
    "dm1_v2_settings_source_lock",
    "dm1_v2_graphics_pipeline_source_isolation",
    "dm1_v2_item_render_pc34",
    "dm1_v2_item_render_source_lock",
    "dm1_v2_runtime_presentation_smoke",
    "dm1_v2_camera_controller_pc34",
    "dm1_v2_camera_turn_edge_cases_pc34",
    "dm1_v2_camera_controller_source_lock",
    "dm1_v2_movement_camera_pc34",
    "dm1_v2_smooth_movement_source_lock",
    "dm1_v2_movement_command_adapter_pc34",
    "dm1_v2_movement_command_adapter_source_lock",
    "dm1_v2_touch_controller_affordance_pc34",
    "dm1_v2_touch_controller_affordance_source_lock",
    "dm1_v2_source_route_state_hash_pc34",
    "dm1_v2_side_by_side_seed_pc34",
    "dm1_v2_v1_v2_side_by_side_seed_pc34",
    "dm1_v2_hud_interaction_pc34",
    "dm1_v2_hud_interaction_source_lock",
    "dm1_v2_selected_resolution_input_mapping_pc34",
    "dm1_v2_4k_input_zone_mapping_pc34",
    "dm1_v2_ui_overlay_affordance_routes_source_lock",
    "dm1_v2_enhanced_effects_runtime_pc34",
    "dm1_v2_field_projectile_effect_metadata_pc34",
    "dm1_v2_field_projectile_vfx_pc34",
    "dm1_v2_extended_field_vfx_pc34",
    "dm1_v2_anim_timing_pc34",
    "dm1_v2_creature_render_pc34",
    "dm1_v2_spell_effect_pc34",
    "dm1_v2_field_projectile_effect_metadata_source_lock",
    "dm1_v2_presentation_profile_pc34",
    "dm1_v2_presentation_mode_pc34",
    "dm1_v2_phase_gate_pc34",
    "dm1_v2_phase5_runtime_bridge_pc34",
    "dm1_v2_shape_runtime_pc34",
    "dm1_v2_per_mode_material_signatures_pc34",
    "dm1_v22_modern_resolution_matrix_pc34",
    "dm1_v22_verification",
    "dm1_v22_asset_pipeline",
    "dm1_v22_upscaled_asset_selection_pc34",
    "m11_v22_shape_cache_pc34",
    "m11_v22_inplace_draw_pc34",
    "m11_v22_render_overlay_pc34",
    "dm1_v22_real_asset_material_gate_pc34",
    "dm1_v22_finished_art_material_gate_pc34",
    "firestaff_dm1_v22_finished_art_material_gate_probe",
    "dm1_v22_finished_pack_receipt_pc34",
    "firestaff_dm1_v22_finished_pack_receipt_probe",
    "dm1_v22_m11_inplace_handoff_source_lock",
}

CATEGORY_RULES = [
    ("anim_timing", "runtime/shell"),
    ("asset_pipeline", "asset/upscale"),
    ("modern_assets", "asset/upscale"),
    ("field_projectile", "visual/effects"),
    ("extended_field", "visual/effects"),
    ("phase", "runtime/shell"),
    ("presentation", "runtime/shell"),
    ("settings", "runtime/shell"),
    ("side_by_side", "support/tooling"),
    ("touch_controller", "ui/input"),
    ("filter", "visual/effects"),
    ("runtime", "runtime/shell"),
    ("movement", "runtime/movement"),
    ("smooth_movement", "runtime/movement"),
    ("viewport", "runtime/viewport"),
    ("texture_upscale", "asset/upscale"),
    ("hud", "ui/hud"),
    ("champion", "ui/champion"),
    ("input", "ui/input"),
    ("tooltip", "ui/input"),
    ("inventory", "ui/inventory"),
    ("item_render", "visual/item"),
    ("minimap", "ui/minimap"),
    ("journal", "ui/journal"),
    ("message_log", "ui/message-log"),
    ("creature", "visual/creature"),
    ("particle", "visual/effects"),
    ("spell", "visual/effects"),
    ("weather", "visual/effects"),
    ("damage", "visual/effects"),
    ("camera", "visual/camera"),
    ("lighting", "visual/lighting"),
    ("screen_transition", "visual/transition"),
    ("level_transition", "runtime/transition"),
    ("audio", "support/audio"),
    ("footstep", "support/audio"),
    ("auto_save", "support/persistence"),
    ("screenshot", "support/tooling"),
    ("pathfinding", "runtime/ai"),
    ("stat_tracker", "support/stats"),
    ("achievements", "support/achievements"),
]

INTERNAL_MODULES_WITHOUT_PUBLIC_HEADER = {
    "src/dm1v2/dm1_v2_anim_timing_pc34.c",
    "src/dm1v2/dm1_v2_filter_crt_scanlines_pc34.c",
    "src/dm1v2/dm1_v2_filter_dither_cleanup_pc34.c",
    "src/dm1v2/dm1_v2_filter_palette_correct_pc34.c",
    "src/dm1v2/dm1_v2_filter_palette_interpolate_pc34.c",
    "src/dm1v2/dm1_v2_filter_sharpen_pc34.c",
    "src/dm1v2/dm1_v2_modern_assets_pc34.c",
    "src/dm1v2/dm1_v2_particle_tick_pc34.c",
}


def classify(name: str) -> str:
    stem = name.removeprefix("dm1_v2_").removesuffix("_pc34.c")
    for needle, category in CATEGORY_RULES:
        if needle in stem:
            return category
    return "orphan"


def ctest_names() -> set[str]:
    text = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8", errors="replace")
    return set(re.findall(r"NAME\s+([A-Za-z0-9_]+)", text))


def main() -> int:
    modules = sorted(str(p.relative_to(ROOT)) for p in (ROOT / "src/dm1v2").glob("dm1_v2_*_pc34.c"))
    headers = {str(p.relative_to(ROOT)) for p in (ROOT / "include").glob("dm1_v2_*_pc34.h")}
    tests = sorted(str(p.relative_to(ROOT)) for p in (ROOT / "tests").glob("test_dm1_v2*.c"))
    tools = sorted(p.name for p in (ROOT / "tools").glob("*v2*"))
    manifests = sorted(p.name for p in (ROOT / "assets-v2/manifests").glob("firestaff-v2-*.manifest.json"))
    names = ctest_names()

    errors: list[str] = []
    matrix = []
    for module in modules:
        stem = Path(module).stem
        header = f"include/{stem}.h"
        category = REQUIRED_MODULES.get(module, classify(Path(module).name))
        has_header = header in headers
        if (module in REQUIRED_MODULES or module not in INTERNAL_MODULES_WITHOUT_PUBLIC_HEADER) and not has_header:
            errors.append(f"missing public header for {module}: {header}")
        matrix.append({"module": module, "header": header if has_header else None, "category": category})

    for module, category in sorted(REQUIRED_MODULES.items()):
        if module not in modules:
            errors.append(f"missing required V2 module {module} ({category})")

    for test_name in sorted(REQUIRED_CTEST_NAMES):
        if test_name not in names:
            errors.append(f"missing required CTest V2 gate {test_name}")

    if "verify_v2_viewport_asset_source_lock.py" not in tools:
        errors.append("missing V2 viewport asset source-lock gate")
    if "test_v2_upscale_dry_run.py" not in tools:
        errors.append("missing V2 upscale dry-run test tool")
    if not manifests:
        errors.append("no V2 manifests found")

    summary: dict[str, int] = {}
    for row in matrix:
        summary[row["category"]] = summary.get(row["category"], 0) + 1

    result = {
        "status": "failed" if errors else "passed",
        "moduleCount": len(modules),
        "testCount": len(tests),
        "toolCount": len(tools),
        "manifestCount": len(manifests),
        "categories": dict(sorted(summary.items())),
        "requiredCTestNames": sorted(REQUIRED_CTEST_NAMES),
        "currentCTestNamesPresent": sorted(REQUIRED_CTEST_NAMES & names),
        "matrix": matrix,
        "errors": errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    print(f"dm1_v2_completion_matrix: modules={len(modules)} tests={len(tests)} tools={len(tools)} manifests={len(manifests)}")
    for category, count in sorted(summary.items()):
        print(f"  {category}: {count}")
    if errors:
        for error in errors:
            print(f"error: {error}")
        return 1
    print(f"evidence={EVIDENCE.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
