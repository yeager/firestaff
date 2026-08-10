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
        "dm2_v1_champion_lifecycle_pc34_compat.c",
        "dm2_v1_load_orchestrator_pc34_compat.c",
        "dm2_v1_save_orchestrator_pc34_compat.c",
        "dm2_v1_save_store_extra_dungeon_data_pc34_compat.c",
        "dm2_v1_save_write_record_checkcode_pc34_compat.c",
        "dm2_v1_gui_draw_pc34_compat.c",
        "dm2_v1_gui_vp_pc34_compat.c",
        "dm2_v1_querydb_pc34_compat.c",
        "dm2_v1_gdatfile_pc34_compat.c",
        "dm2_v1_sfx_pc34_compat.c",
        "dm2_v1_ccm.c",
        "dm2_v1_projectile_pc34_compat.c",
        "dm2_v1_projectile_step_pc34_compat.c",
        "dm2_v1_projectile_creature_collision_pc34_compat.c",
        "dm2_v1_perform_move_exec_pc34_compat.c",
    },
    "DM2_SOURCES": {
        "dm2_v1_hud_panel_routing.c",
        "dm2_v1_hud_survey_helpers.c",
        "dm2_v1_combat.c",
        "dm2_v1_tech_magic.c",
        "dm2_v1_record_name_helper.c",
        "dm2_v1_source_name_helpers.c",
        "dm2_v1_ui_event_name_helper.c",
        "dm2_v1_food_water_bridge.c",
        "dm2_v1_outdoor_renderer.c",
        "dm2_v1_1c9a_pc34_compat.c",
        "dm2_v1_0aaf_pc34_compat.c",
        "dm2_v1_runtime_narrow_pc34_compat.c",
        "dm2_v1_runtime_parity_pc34_compat.c",
        "dm2_v1_load_orchestrator_pc34_compat.c",
        "dm2_v1_save_orchestrator_pc34_compat.c",
        "dm2_v1_save_store_extra_dungeon_data_pc34_compat.c",
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
        "dm2_v1_perform_move_exec_pc34_compat.c",
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

    # FM Towns English keeps the Japanese CD as the native owner and admits
    # the PC-English companion only through dm2_v1_boot's explicit callback.
    # The source-shaped GUI/query modules below remain direct-regression
    # studies and are removed from product archives. Reject a future M11
    # call site that bypasses the authenticated companion boundary.
    if "dm2_v1_boot_dialogue_open_panel_host_command(" not in m11:
        errors.append("M11 DM2 dialogue owner no longer uses the boot text boundary")
    # Keep the live-owner census explicit.  At this stage only the save
    # dialogue panel has a source-owned M11 route; admitting a second text
    # consumer without binding its c_dialog/c_gfx_str owner would reintroduce
    # host-authored or caller-authored text by accident.
    dialogue_owner_calls = m11.count(
        "dm2_v1_boot_dialogue_open_panel_host_command(")
    if dialogue_owner_calls != 1:
        errors.append(
            "M11 DM2 dialogue owner census changed: expected one boot-panel "
            f"call, found {dialogue_owner_calls}")
    if "m11_draw_dm2_save_dialogue_panel(" not in m11:
        errors.append("M11 DM2 save-dialogue consumer is missing")
    if m11.count("m11_draw_dm2_save_dialogue_panel(") != 2:
        errors.append(
            "M11 DM2 save-dialogue census changed: expected one definition "
            "and one render call")
    boot_path = repo / "src/dm2/dm2_v1_boot.c"
    if boot_path.exists():
        boot = boot_path.read_text(encoding="utf-8")
        if ("dm2_v1_boot_fmtowns_english_dialogue_text" not in boot or
                "dm2_v1_runtime_query_gdat_text_override" not in boot):
            errors.append("FM Towns English dialogue callback no longer reaches the runtime companion")
    else:
        errors.append(f"missing {boot_path}")
    legacy_text_call = re.compile(
        r"\bdm2_v1_(?:query_gdat_text|gfx_str_query_gdat_text|"
        r"0aaf_|draw_dialogue_)\w*\s*\(")
    for source_path in sorted((repo / "src/engine").rglob("*.c")):
        if legacy_text_call.search(source_path.read_text(encoding="utf-8")):
            errors.append(
                "M11 calls an unbound DM2 compatibility text owner: "
                f"{source_path.relative_to(repo)}")

    # SOUND1..9's historical local queue accepts caller-authored state and
    # music maps.  It is useful to its direct contract test but is not an
    # original GAME_LOAD/GDAT sound owner; the product source must compile it
    # only behind its explicit fixture definition.
    sound_path = repo / "src/dm2/dm2_v1_sound.c"
    if not sound_path.exists():
        errors.append(f"missing {sound_path}")
    else:
        sound = sound_path.read_text(encoding="utf-8")
        if ("#ifdef FIRESTAFF_DM2_SKPROJECT_SOUND_FIXTURE" not in sound or
                "#endif /* FIRESTAFF_DM2_SKPROJECT_SOUND_FIXTURE */" not in sound):
            errors.append("DM2 caller-authored SOUND1..9 fixture is not compile-gated")
    if "FIRESTAFF_DM2_SKPROJECT_SOUND_FIXTURE=1" not in cmake:
        errors.append("DM2 SOUND1..9 source-contract target lacks its fixture definition")

    # Position and outdoor compatibility setters are retained for narrow
    # source-study targets.  They accept caller-authored coordinates, so M11
    # must never use them to promote a parsed File_header pose into a party.
    # The real route begins only after the still-missing atomic GAME_LOAD
    # handoff restores the party, record pools, timers and environment.
    legacy_pose_setter = re.compile(
        r"\bdm2_v1_runtime_set_(?:position|outdoor)\s*\(")
    for source_path in sorted((repo / "src/engine").rglob("*.c")):
        if legacy_pose_setter.search(source_path.read_text(encoding="utf-8")):
            errors.append(
                "M11 calls a caller-authored DM2 party/environment setter: "
                f"{source_path.relative_to(repo)}")

    runtime_path = repo / "src/dm2/dm2_v1_runtime.c"
    if not runtime_path.exists():
        errors.append(f"missing {runtime_path}")
        return errors
    runtime = runtime_path.read_text(encoding="utf-8")
    for forbidden in (
            "dm2_runtime_process_3d_timer",
            "dm2_runtime_actuate_pitfall",
            "dm2_runtime_actuate_door",
            "dm2_runtime_invoke_message",
            "dm2_runtime_invoke_actuator",
            "dm2_runtime_tick_generator_timer",
            "dm2_runtime_ornate_animator_timer",
            "dm2_runtime_ornate_noise_timer",
            "dm2_runtime_move_record_rotate_timer",
            "dm2_runtime_actuate_wall_mecha",
            "dm2_runtime_actuate_teleporter",
            "dm2_runtime_actuate_floor_mecha",
            "dm2_runtime_actuate_trickwall",
            "dm2_runtime_delete_creature_full",
            "dm2_v1_caii_set_delete_creature_full_fn",
    ):
        if forbidden in runtime:
            errors.append(f"runtime retains timer-byte mutation study: {forbidden}")

    # The bounded DELETE_CREATURE_RECORD source study lacks the original
    # shared c_map/3CE7D/DB-allocation/timer owner.  It may be compiled by its
    # focused test targets, never by the broad DM2 product archive.
    delete_full_source = (
        '"${CMAKE_CURRENT_SOURCE_DIR}/src/dm2/'
        'dm2_v1_delete_creature_full_pc34_compat.c"')
    m10_remove_start = cmake.find("list(REMOVE_ITEM M10_SOURCES")
    dm2_sources_start = cmake.find("# ── DM2 V1 static library")
    if (m10_remove_start < 0 or dm2_sources_start < 0 or
            delete_full_source not in cmake[m10_remove_start:dm2_sources_start]):
        errors.append(
            "bounded DM2 DELETE_CREATURE_RECORD study is no longer excluded "
            "from the production M10 archive")

    dm2_remove_start = cmake.find("list(REMOVE_ITEM DM2_SOURCES")
    dm2_remove_end = cmake.find(")\nif(DM2_SOURCES)", dm2_remove_start)
    if (dm2_remove_start < 0 or dm2_remove_end < 0 or
            delete_full_source not in cmake[dm2_remove_start:dm2_remove_end]):
        errors.append(
            "bounded DM2 DELETE_CREATURE_RECORD study is no longer excluded "
            "from the production archive")

    # These two compatibility studies accept caller-owned timer/map/pool
    # state.  SKProject c_moverec.cpp and c_tim_proc.cpp only reach those
    # mutations through one live GAME_LOAD transaction, which Firestaff does
    # not publish yet.  Their focused tests compile them explicitly; the
    # broad DM2 archive must not export an alternate execution route.
    for legacy_mutator in (
            'dm2_v1_move_record_to_pc34_compat.c',
            'dm2_v1_actuator_event_pc34_compat.c'):
        source_entry = ('"${CMAKE_CURRENT_SOURCE_DIR}/src/dm2/' +
                        legacy_mutator + '"')
        if source_entry not in cmake[dm2_remove_start:dm2_remove_end]:
            errors.append(
                "caller-owned DM2 mutation study is no longer excluded "
                f"from the production archive: {legacy_mutator}")

    caii_alloc_path = repo / "src/dm2/dm2_v1_caii_alloc_pc34_compat.c"
    if not caii_alloc_path.exists():
        errors.append(f"missing {caii_alloc_path}")
        return errors
    caii_alloc = caii_alloc_path.read_text(encoding="utf-8")
    if "dm2_v1_caii_set_delete_creature_full_fn" in caii_alloc:
        errors.append(
            "CAII retains a callback seam for partial "
            "DELETE_CREATURE_RECORD mutation")

    # dm2_v1_sound.c still carries a compact SKProject transcription model for
    # its direct regression.  Its state is caller-authored (including made-up
    # MIDI handles and queue capacity), so it must never become a second live
    # sound owner beside the GDAT/DYN4/SDL and FM Towns CDDA paths.  Keep the
    # implementation available to its narrow test while rejecting every
    # product-side call site.  The definition file itself is intentionally
    # excluded from this search.
    legacy_sound_prefix = re.compile(
        r"\bdm2_v1_skproject_(?:sound\w*|process_sound|"
        r"query_snd_entry_index|get_music_index_from_modlist)\s*\(")
    legacy_sound_definition = repo / "src/dm2/dm2_v1_sound.c"
    for source_path in sorted((repo / "src").rglob("*.c")):
        if source_path == legacy_sound_definition:
            continue
        if legacy_sound_prefix.search(source_path.read_text(encoding="utf-8")):
            errors.append(
                "product source calls legacy caller-authored SKProject sound "
                f"model: {source_path.relative_to(repo)}")

    dungeon_loader_path = repo / "src/dm2/dm2_v1_dungeon_loader.c"
    if not dungeon_loader_path.exists():
        errors.append(f"missing {dungeon_loader_path}")
        return errors
    dungeon_loader = dungeon_loader_path.read_text(encoding="utf-8")
    required_fixture_guard = (
        "#if !defined(FIRESTAFF_DM2_SYNTHETIC_DUNGEON_FIXTURES)",
        "return -1;",
        "#else",
        "#endif",
    )
    if not all(fragment in dungeon_loader for fragment in required_fixture_guard):
        errors.append(
            "DM2 dungeon loader no longer excludes the word-square fixture "
            "parser from product builds")
    # The historic word-square reader is compiled only by the one direct
    # regression target.  A mere substring check is not enough here: a
    # future developer could add the same definition to firestaff_dm2 or M10
    # and leave this check green while production once again accepted
    # caller-authored dungeon bytes.  Keep the definition singular and bind
    # it to the test target explicitly.
    synthetic_fixture_definitions = re.findall(
        r"target_compile_definitions\(\s*([^\s)]+)(.*?)\)",
        cmake, flags=re.DOTALL)
    synthetic_fixture_targets = [
        target for target, definitions in synthetic_fixture_definitions
        if "FIRESTAFF_DM2_SYNTHETIC_DUNGEON_FIXTURES=1" in definitions
    ]
    if synthetic_fixture_targets != ["test_dm2_v1_dungeon_loader_first_map_gate"]:
        errors.append(
            "DM2 word-square fixture definition must occur exactly once and "
            "only on test_dm2_v1_dungeon_loader_first_map_gate; found: " +
            (", ".join(synthetic_fixture_targets) or "none"))

    creature_path = repo / "src/dm2/dm2_v1_creature.c"
    if not creature_path.exists():
        errors.append(f"missing {creature_path}")
        return errors
    creature = creature_path.read_text(encoding="utf-8")
    for forbidden in (
            "dm2_v1_creature_make_ccm_args",
            "dm2_v1_creature_imported_ccm_op",
            "dm2_v1_creature_door_blocks_creature",
    ):
        if forbidden in creature:
            errors.append(
                f"creature retains reduced-state CCM mutation study: {forbidden}")
    if ("void dm2_v1_creature_tick(void) {\n"
            "#ifndef FIRESTAFF_DM2_CREATURE_TESTING\n" not in creature or
            "admitted only through dm2_v1_runtime's source-timer/DB4 route."
            not in creature):
        errors.append(
            "production creature tick no longer closes the standalone fixture pool")

    actuator_path = repo / "src/dm2/dm2_v1_actuator_event_pc34_compat.c"
    if not actuator_path.exists():
        errors.append(f"missing {actuator_path}")
        return errors
    actuator = actuator_path.read_text(encoding="utf-8")
    for forbidden in (
            "DM2_V1_TIMER_SHOOT_ITEM",
            "dm2_v1_record_pool_cut_from_tile(pool_set, dungeon",
            "dm2_v1_alloc_new_dbitem(pool_set, actu_data)",
    ):
        if forbidden in actuator:
            errors.append(
                f"actuator retains reduced-state shooter mutation study: {forbidden}")

    combat_path = repo / "src/dm2/dm2_v1_combat.c"
    if not combat_path.exists():
        errors.append(f"missing {combat_path}")
        return errors
    combat = combat_path.read_text(encoding="utf-8")
    if "receipt.damage = dm2_v1_combat_resolve_attack_full(" in combat:
        errors.append("combat retains the incomplete creature-damage bridge")

    main_loop_path = repo / "src/engine/main_loop_m11.c"
    if not main_loop_path.exists():
        errors.append(f"missing {main_loop_path}")
    else:
        main_loop = main_loop_path.read_text(encoding="utf-8")
        if main_loop.count("m11_dm2_host_save_log_allowed(") < 5:
            errors.append(
                "M11 DM2 quick-save/load text is not guarded by the source-GUI boundary")
        if "gameView->sourceKind != M11_GAME_SOURCE_DM2_BOOT" not in main_loop:
            errors.append("M11 DM2 quick-save/load source guard is missing")

    # The source-shaped player-attack and wound receipts are direct-regression
    # seams. Their caller-authored champion, item, target and RNG words must
    # not become a product damage path merely because a future source glob
    # links them. `dm2_v1_attack_party` is intentionally not included here:
    # creature-ops has a separate callback API with the same C identifier.
    legacy_combat_prefix = re.compile(
        r"\bdm2_v1_(?:calc_player_attack_damage_receipt|"
        r"wound_player_receipt)\s*\(")
    legacy_combat_definitions = {
        repo / "src/dm2/dm2_v1_combat_damage_pc34_compat.c",
    }
    for source_path in sorted((repo / "src").rglob("*.c")):
        if source_path in legacy_combat_definitions:
            continue
        if legacy_combat_prefix.search(source_path.read_text(encoding="utf-8")):
            errors.append(
                "product source calls caller-authored DM2 combat damage "
                f"seam: {source_path.relative_to(repo)}")

    hud_path = repo / "src/dm2/dm2_v1_gdat_hud_m11_command.c"
    if not hud_path.exists():
        errors.append(f"missing {hud_path}")
        return errors
    hud = hud_path.read_text(encoding="utf-8")
    if "DM2_V1_GDAT_IMAGE_FALLBACK" in hud:
        errors.append("HUD mislabels SKProject's portrait source-default as fallback")
    for source_default in (
            "DM2_V1_GDAT_CHAMPION_PORTRAIT_DEFAULT_CATEGORY",
            "DM2_V1_GDAT_CHAMPION_PORTRAIT_DEFAULT_INDEX",
            "DM2_V1_GDAT_CHAMPION_PORTRAIT_DEFAULT_FIELD",
    ):
        if source_default not in hud:
            errors.append(f"HUD portrait source-default binding missing: {source_default}")

    world_model_path = repo / "src/dm2/dm2_v1_world_model.c"
    if not world_model_path.exists():
        errors.append(f"missing {world_model_path}")
        return errors
    world_model = world_model_path.read_text(encoding="utf-8")
    for forbidden in (
            "dm2_parse_header((const dm2_dungeon_header_t *)decoded, world)",
            "dm2_parse_tile(raw)",
    ):
        if forbidden in world_model:
            errors.append(
                "world model retains the retired inferred 16-bit dungeon fallback")

    object_model_path = repo / "src/dm2/dm2_v1_object_model.c"
    if not object_model_path.exists():
        errors.append(f"missing {object_model_path}")
        return errors
    object_model = object_model_path.read_text(encoding="utf-8")
    for forbidden in (
            "size_t thing_data_start = header_size + map_desc_total + tile_total;",
            "pool_offset_abs += (size_t)world->thing_pool_counts[t]",
    ):
        if forbidden in object_model:
            errors.append("object model retains the inferred sequential-pool fallback")

    modern_assets_path = repo / "src/dm2/dm2_v22_modern_assets_pc34.c"
    if not modern_assets_path.exists():
        errors.append(f"missing {modern_assets_path}")
        return errors
    modern_assets = modern_assets_path.read_text(encoding="utf-8")
    if "found_critical[0] && found_critical[1] && found_critical[2]" in modern_assets:
        errors.append("modern-assets availability retains the local-manifest admission study")

    v2_hud_path = repo / "src/dm2/dm2_v2_hud_overlay.c"
    if not v2_hud_path.exists():
        errors.append(f"missing {v2_hud_path}")
        return errors
    v2_hud = v2_hud_path.read_text(encoding="utf-8")
    for forbidden in (
            "static void hud_plot(",
            "static const uint8_t g_digit_bits",
            "static const char *g_action_icon_labels",
    ):
        if forbidden in v2_hud:
            errors.append(f"V2 HUD retains generated-pixel fallback: {forbidden}")

    weather_path = repo / "src/dm2/dm2_v1_weather_gdat.c"
    if not weather_path.exists():
        errors.append(f"missing {weather_path}")
        return errors
    weather = weather_path.read_text(encoding="utf-8")
    if "else if (bpp == 4u || bpp == 8u)" in weather:
        errors.append(
            "IMG3 metadata lets a fixture w4 word override compressed-source depth")
    for required in (
            "else if (offset_y == -32)",
            "if (bpp != 4u && bpp != 8u)",
    ):
        if required not in weather:
            errors.append(f"IMG3 Getpf source-depth guard missing: {required}")

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
            "dm2_blit_bitmap",
            "dm2_v1_blit_tiled_bitmap",
            "dm2_v1_blit_scaled_bitmap",
            "dm2_resolve_blit_clip",
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
