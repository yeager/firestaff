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
}

CATEGORY_RULES = [
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
    modules = sorted(p.name for p in ROOT.glob("dm1_v2_*_pc34.c"))
    headers = {p.name for p in ROOT.glob("dm1_v2_*_pc34.h")}
    tests = sorted(p.name for p in ROOT.glob("test_dm1_v2*.c"))
    tools = sorted(p.name for p in (ROOT / "tools").glob("*v2*"))
    manifests = sorted(p.name for p in (ROOT / "assets-v2/manifests").glob("firestaff-v2-*.manifest.json"))
    names = ctest_names()

    errors: list[str] = []
    matrix = []
    for module in modules:
        header = module[:-2] + ".h"
        category = REQUIRED_MODULES.get(module, classify(module))
        if header not in headers:
            errors.append(f"missing header for {module}: {header}")
        matrix.append({"module": module, "header": header, "category": category})

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
