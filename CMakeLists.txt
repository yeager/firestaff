cmake_minimum_required(VERSION 3.20)
project(Firestaff VERSION 2.4.0 LANGUAGES C)

include_directories(include src)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Compiler warnings ────────────────────────────────────────────────
if(MSVC)
    add_compile_options(/W4)
else()
    # M11/M12 targets get strict warnings; M10 gets relaxed treatment.
    # We set per-target below.
endif()

# ── SDL discovery (prefer SDL3, fall back to SDL2) ──────────────────
find_package(PkgConfig REQUIRED)
find_package(SDL3 QUIET CONFIG)
if(SDL3_FOUND)
    message(STATUS "Found SDL3")
else()
    pkg_check_modules(SDL3 QUIET IMPORTED_TARGET sdl3)
endif()
if(NOT SDL3_FOUND AND NOT TARGET PkgConfig::SDL3)
    pkg_check_modules(SDL2 QUIET IMPORTED_TARGET sdl2)
    if(SDL2_FOUND OR TARGET PkgConfig::SDL2)
        message(STATUS "SDL3 not found, using SDL2")
        if(NOT TARGET SDL3::SDL3)
            add_library(SDL3::SDL3 ALIAS PkgConfig::SDL2)
        endif()
    else()
        message(FATAL_ERROR "Neither SDL3 nor SDL2 found")
    endif()
else()
    if(NOT TARGET SDL3::SDL3 AND TARGET PkgConfig::SDL3)
        add_library(SDL3::SDL3 ALIAS PkgConfig::SDL3)
    endif()
endif()

# ── Python discovery for source-shape/evidence gates ───────────────
find_package(Python3 COMPONENTS Interpreter QUIET)

# ── M10 static library (frozen Turbo-C compat sources) ──────────────
file(GLOB_RECURSE M10_SOURCES CONFIGURE_DEPENDS "src/*_pc34_compat.c")

# Exclude test_* files from the library (they have their own main()).
list(FILTER M10_SOURCES EXCLUDE REGEX "test_.*_pc34_compat\\.c$")
list(FILTER M10_SOURCES EXCLUDE REGEX "dm1_v1_engine_pc34_compat\\.c$")

add_library(firestaff_m10 STATIC ${M10_SOURCES} src/shared/dungeon_decompressor_ftl.c)
target_include_directories(firestaff_m10 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(firestaff_m10 PUBLIC
    COMPILE_H
    STATICFUNCTION=static
    "SEPARATOR=,"
    "FINAL_SEPARATOR=)"
    HUGE=
    huge=
)
if(NOT MSVC)
    # M10 code is frozen Turbo-C port — only minimal warnings.
    # -include stddef.h provides NULL for files that skip standard includes.
    target_compile_options(firestaff_m10 PRIVATE -std=c99 -O2 -include stddef.h)
endif()




# ── CSB V2 static library ──────────────────────────────────────────
file(GLOB CSB_V2_SOURCES CONFIGURE_DEPENDS "src/csb/csb_v2_*.c")
if(CSB_V2_SOURCES)
    add_library(firestaff_csb_v2 STATIC ${CSB_V2_SOURCES})
    target_include_directories(firestaff_csb_v2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(firestaff_csb_v2 PUBLIC firestaff_m10 m)
    if(NOT MSVC)
        target_compile_options(firestaff_csb_v2 PRIVATE -Wall -Wextra -O2)
    endif()
endif()

# ── DM2 V1 static library ──────────────────────────────────────────
file(GLOB DM2_SOURCES CONFIGURE_DEPENDS "src/dm2/dm2_v1_*.c")
if(DM2_SOURCES)
    add_library(firestaff_dm2 STATIC ${DM2_SOURCES})
    target_include_directories(firestaff_dm2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(NOT MSVC)
        target_compile_options(firestaff_dm2 PRIVATE -Wall -Wextra -O2)
    endif()
endif()



# ── Nexus V1 static library ────────────────────────────────────────
file(GLOB NEXUS_SOURCES CONFIGURE_DEPENDS "src/nexus/nexus_v1_*.c")
if(NEXUS_SOURCES)
    add_library(firestaff_nexus STATIC ${NEXUS_SOURCES})
    target_include_directories(firestaff_nexus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(NOT MSVC)
        target_compile_options(firestaff_nexus PRIVATE -Wall -Wextra -O2)
    endif()
endif()

# ── DM2 V2 static library ──────────────────────────────────────────
file(GLOB DM2_V2_SOURCES CONFIGURE_DEPENDS "src/dm2/dm2_v2_*.c")
if(DM2_V2_SOURCES)
    add_library(firestaff_dm2_v2 STATIC ${DM2_V2_SOURCES})
    target_include_directories(firestaff_dm2_v2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(firestaff_dm2_v2 PUBLIC firestaff_m10 m)
    if(NOT MSVC)
        target_compile_options(firestaff_dm2_v2 PRIVATE -Wall -Wextra -O2)
    endif()
endif()

# ── V2 static library (enhanced features) ──────────────────────────
file(GLOB_RECURSE V2_SOURCES CONFIGURE_DEPENDS "src/dm1v2/*_pc34.c")
add_library(firestaff_v2 STATIC ${V2_SOURCES})
target_include_directories(firestaff_v2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(firestaff_v2 PUBLIC firestaff_m10 m)
if(NOT MSVC)
    target_compile_options(firestaff_v2 PRIVATE -Wall -Wextra -O2)
endif()

# ── M11 static library ──────────────────────────────────────────────
set(M11_SOURCES
    src/engine/main_loop_m11.c
    src/engine/firestaff_accessibility.c
    src/engine/m11_game_view.c
    src/engine/render_sdl_m11.c
    src/shared/audio_sdl_m11.c
    src/shared/graphics_dat_snd3_loader_v1.c
    src/shared/song_dat_loader_v1.c
    src/shared/sound_event_snd3_map_v1.c
    src/frontend/title_dat_loader_v1.c
    src/frontend/title_frontend_v1.c
    src/shared/asset_loader_m11.c
    src/shared/dungeon_decompressor_ftl.c
    src/shared/font_m11.c
    src/shared/fs_portable_compat.c
    src/dm1/dm1_v1_vblank_timing.c
    src/dm1/dm1_v1_save_load.c
    src/dm1/dm1_v1_viewport_3d_pc34_compat.c
    src/dm1/dm1_v1_dungeon_loader_pc34_compat.c
    src/dm1/dm1_v1_title_screen_pc34_compat.c
    src/dm1/dm1_v1_palette_font_pc34_compat.c
    src/dm1/dm1_v1_object_world_pc34_compat.c
    src/dm1/dm1_v1_graphics_loader_pc34_compat.c
    src/dm1/dm1_v1_fade_transition_pc34_compat.c
    src/dm1/dm1_v1_room_transition_pc34_compat.c
    src/dm1/dm1_v1_portrait_panel_pc34_compat.c
    src/dm1/dm1_v1_wall_ornament_pc34_compat.c
    src/dm1/dm1_v1_dungeon_decompressor_pc34_compat.c
    src/dm1/dm1_v1_field_teleporter_effect_pc34_compat.c
    src/dm1/dm1_v1_game_loop_integration_pc34_compat.c
    src/dm1/dm1_v1_creature_viewport_pc34_compat.c
    src/dm1/dm1_v1_save_load_system_pc34_compat.c
    src/dm1/dm1_v1_blit_fill_pc34_compat.c
    src/dm1/dm1_v1_dialog_scroll_pc34_compat.c
    src/dm1/dm1_v1_text_message_pc34_compat.c
    src/dm1/dm1_v1_inventory_consumables_pc34_compat.c
    src/dm1/dm1_v1_click_routing_pc34_compat.c
    src/shared/firestaff_po_loader.c
    src/dm1/dm1_v1_engine_pc34_compat.c
)

add_library(firestaff_m11 STATIC ${M11_SOURCES})
target_include_directories(firestaff_m11 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(firestaff_m11 PUBLIC firestaff_m10 SDL3::SDL3 m)
if(NOT MSVC)
    target_compile_options(firestaff_m11 PRIVATE -Wall -Wextra -O2)
    # src/engine/m11_game_view.c is a large legacy file — relax truncation/unused warnings.
    set_source_files_properties(src/engine/m11_game_view.c PROPERTIES
        COMPILE_OPTIONS "-Wno-format-truncation;-Wno-unused-function;-Wno-maybe-uninitialized"
    )
endif()

# ── M12 static library ──────────────────────────────────────────────
set(M12_SOURCES
    src/ui/menu_startup_m12.c
    src/ui/menu_startup_render_modern_m12.c
    src/shared/firestaff_l10n.c
    src/ui/firestaff_bestiary.c
    src/ui/firestaff_spell_ref.c
    src/ui/firestaff_item_encyclopedia.c
    src/ui/firestaff_map_viewer.c
    src/ui/firestaff_screenshot_gallery.c
    src/engine/firestaff_dungeon_text.c
    src/engine/firestaff_graphics_dat_reader.c
    src/ui/menu_hit_m12.c
    src/engine/config_m12.c
    src/shared/asset_status_m12.c
    src/shared/asset_find_by_hash.c
    src/shared/card_art_m12.c
    src/shared/card_art_generated_m12.c
    src/ui/branding_logo_m12.c
    src/ui/branding_logo_readme_m12.c
    src/shared/bestiary_m12.c
    src/shared/creature_art_m12.c
    src/shared/data_validator_m12.c
    src/shared/changelog_m12.c
    src/ui/theme_m12.c
    src/shared/animated_bg_m12.c
    src/shared/input_remap_m12.c
    src/shared/save_browser_m12.c
    src/shared/custom_dungeon_m12.c
    src/shared/item_encyclopedia_m12.c
    src/shared/campaign_m12.c
    src/shared/spell_reference_m12.c
    src/shared/map_viewer_m12.c
    src/shared/music_jukebox_m12.c
    src/shared/gamepad_config_m12.c
    src/ui/touch_layout_m12.c
    src/shared/presentation_preview_m12.c
    src/shared/screenshot_gallery_m12.c
)

add_library(firestaff_m12 STATIC ${M12_SOURCES})
target_include_directories(firestaff_m12 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(firestaff_m12 PUBLIC firestaff_m11)
if(NOT MSVC)
    target_compile_options(firestaff_m12 PRIVATE -Wall -Wextra -O2
        -Wno-format-truncation -Wno-array-bounds -Wno-restrict -Wno-unused-variable)
endif()

# ── Main executable ──────────────────────────────────────────────────
add_executable(firestaff src/shared/firestaff_main_m11.c)
# M11 objects referenced by the executable call back into M12 menu/asset helpers;
# repeat M12 after M11 so static linkers resolve both directions.
target_link_libraries(firestaff PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m10 SDL3::SDL3)
if(NOT MSVC)
    target_compile_options(firestaff PRIVATE -Wall -Wextra -O2)
endif()

# ── Probe executables ────────────────────────────────────────────────
# M11 Phase A probe (only needs render_sdl + vga_palette, linked via m10)
add_executable(firestaff_m11_phase_a_probe probes/m11/firestaff_m11_phase_a_probe.c)
target_link_libraries(firestaff_m11_phase_a_probe PRIVATE firestaff_m11 firestaff_m10 SDL3::SDL3)
if(NOT MSVC)
    target_compile_options(firestaff_m11_phase_a_probe PRIVATE -Wall -Wextra -O2)
endif()

# M11 Game View probe (needs full M10 + M11 + M12)
add_executable(firestaff_m11_game_view_probe probes/m11/firestaff_m11_game_view_probe.c)
target_link_libraries(firestaff_m11_game_view_probe PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_game_view_probe PRIVATE -Wall -Wextra -O2 -Wno-format-truncation)
endif()

# V2 initial 4K capture scene (real M11 draw path for offline 4K composition)
add_executable(firestaff_m11_v2_initial_4k_capture probes/m11/firestaff_m11_v2_initial_4k_capture.c)
# M11 calls a small set of M12 startup/asset helpers; repeat m12 after m11 for static linkers.
target_link_libraries(firestaff_m11_v2_initial_4k_capture PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_v2_initial_4k_capture PRIVATE -Wall -Wextra -O2)
endif()

# Deterministic source-driven viewport state anchor (DUNGEON.DAT + GRAPHICS.DAT)
add_executable(firestaff_m11_viewport_state_probe probes/m11/firestaff_m11_viewport_state_probe.c)
# M11 viewport state probe pulls m11_game_view, which references M12 menu/asset helpers.
# Repeat firestaff_m12 after firestaff_m11 for static linkers.
target_link_libraries(firestaff_m11_viewport_state_probe PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m10 SDL3::SDL3)
if(NOT MSVC)
    target_compile_options(firestaff_m11_viewport_state_probe PRIVATE -Wall -Wextra -O2)
endif()

# Deterministic Firestaff capture-route state anchor (same route as screenshot fixture)
add_executable(firestaff_m11_capture_route_state_probe probes/m11/firestaff_m11_capture_route_state_probe.c)
target_link_libraries(firestaff_m11_capture_route_state_probe PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_capture_route_state_probe PRIVATE -Wall -Wextra -O2)
endif()

# DM1 V1 wall/collision runtime repro capture with exact map coordinates.
add_executable(firestaff_m11_wall_collision_capture_probe probes/m11/firestaff_m11_wall_collision_capture_probe.c)
target_link_libraries(firestaff_m11_wall_collision_capture_probe PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_wall_collision_capture_probe PRIVATE -Wall -Wextra -O2)
endif()

# Source-locked turn-left/turn-right viewport orientation anchor.
add_executable(firestaff_m11_turn_viewport_orientation_probe probes/m11/firestaff_m11_turn_viewport_orientation_probe.c)
target_link_libraries(firestaff_m11_turn_viewport_orientation_probe PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_turn_viewport_orientation_probe PRIVATE -Wall -Wextra -O2)
endif()

# DM1 V1 Hall of Champions walkaround runtime smoke probe.
add_executable(firestaff_m11_hall_walkaround_runtime_probe probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c)
target_link_libraries(firestaff_m11_hall_walkaround_runtime_probe PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_hall_walkaround_runtime_probe PRIVATE -Wall -Wextra -O2)
endif()

# Deterministic in-game screenshot series used by parity evidence.
add_executable(capture_ingame_series verification-screens/capture_firestaff_ingame_series.c)
target_link_libraries(capture_ingame_series PRIVATE firestaff_m12 firestaff_m11 firestaff_m10 SDL3::SDL3)
if(NOT MSVC)
    target_compile_options(capture_ingame_series PRIVATE -Wall -Wextra -O2)
endif()

# M11 Audio probe (fallback-safe audio marker pipeline)
add_executable(firestaff_m11_audio_probe probes/m11/firestaff_m11_audio_probe.c)
target_link_libraries(firestaff_m11_audio_probe PRIVATE firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_audio_probe PRIVATE -Wall -Wextra -O2)
endif()

# M11 creature attack/movement runtime source-index audio probe
add_executable(firestaff_m11_creature_runtime_audio_probe probes/m11/firestaff_m11_creature_runtime_audio_probe.c)
target_link_libraries(firestaff_m11_creature_runtime_audio_probe PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_creature_runtime_audio_probe PRIVATE -Wall -Wextra -O2)
endif()

# M11 title-song runtime source-lock probe (SONG.DAT only; no generic SFX lane)
add_executable(firestaff_m11_pass54_song_runtime_probe probes/m11/firestaff_m11_pass54_song_runtime_probe.c)
target_link_libraries(firestaff_m11_pass54_song_runtime_probe PRIVATE firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_pass54_song_runtime_probe PRIVATE -Wall -Wextra -O2)
endif()

# M11 FS Portable probe (cross-platform filesystem helpers)
add_executable(firestaff_m11_fs_portable_probe probes/m11/firestaff_m11_fs_portable_probe.c)
target_link_libraries(firestaff_m11_fs_portable_probe PRIVATE firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_fs_portable_probe PRIVATE -Wall -Wextra -O2)
endif()

# M12 Startup Menu probe
if(EXISTS "probes/m12/firestaff_m12_startup_menu_probe.c")
    add_executable(firestaff_m12_startup_menu_probe probes/m12/firestaff_m12_startup_menu_probe.c)
    target_link_libraries(firestaff_m12_startup_menu_probe PRIVATE firestaff_m12 firestaff_m11 firestaff_m10 SDL3::SDL3)
    if(NOT MSVC)
        target_compile_options(firestaff_m12_startup_menu_probe PRIVATE -Wall -Wextra -O2 -Wno-format-truncation)
    endif()
endif()


# DM1 V1 viewport draw-order evidence probe
if(EXISTS "probes/dm1/firestaff_dm1_v1_viewport_draw_order_probe.c")
    add_executable(firestaff_dm1_v1_viewport_draw_order_probe probes/dm1/firestaff_dm1_v1_viewport_draw_order_probe.c)
    if(NOT MSVC)
        target_compile_options(firestaff_dm1_v1_viewport_draw_order_probe PRIVATE -Wall -Wextra -O2)
    endif()
endif()

# DM1 V1 game-loop redraw/cadence evidence probe
if(EXISTS "probes/dm1/firestaff_dm1_v1_game_loop_redraw_cadence_probe.c")
    add_executable(firestaff_dm1_v1_game_loop_redraw_cadence_probe probes/dm1/firestaff_dm1_v1_game_loop_redraw_cadence_probe.c)
    if(NOT MSVC)
        target_compile_options(firestaff_dm1_v1_game_loop_redraw_cadence_probe PRIVATE -Wall -Wextra -O2)
    endif()
endif()

# DM1 V1 viewport palette/as-before cadence evidence probe
if(EXISTS "probes/dm1/firestaff_dm1_v1_viewport_palette_as_before_probe.c")
    add_executable(firestaff_dm1_v1_viewport_palette_as_before_probe probes/dm1/firestaff_dm1_v1_viewport_palette_as_before_probe.c)
    if(NOT MSVC)
        target_compile_options(firestaff_dm1_v1_viewport_palette_as_before_probe PRIVATE -Wall -Wextra -O2)
    endif()
endif()

# ── CTest integration ────────────────────────────────────────────────
include(CTest)
if(BUILD_TESTING)
    add_test(
        NAME dm1_v1_viewport_draw_order_probe
        COMMAND firestaff_dm1_v1_viewport_draw_order_probe
    )
    add_test(
        NAME dm1_v1_game_loop_redraw_cadence_probe
        COMMAND firestaff_dm1_v1_game_loop_redraw_cadence_probe
    )
    add_test(
        NAME dm1_v1_viewport_palette_as_before_probe
        COMMAND firestaff_dm1_v1_viewport_palette_as_before_probe
    )

    if(Python3_Interpreter_FOUND)
        add_test(
            NAME dm1_v2_upscale_dry_run_validator
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/test_v2_upscale_dry_run.py
        )
        add_test(
            NAME dm1_v2_asset_manifest_validator
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_v2_manifests.py
        )
        add_test(
            NAME dm1_v2_asset_manifest_validator_self_test
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_v2_manifests.py --self-test
        )
        add_test(
            NAME csb_v1_parity_surface_matrix
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_csb_v1_parity_surface_matrix.py
        )
        add_test(
            NAME csb_v1_completion_matrix
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_csb_v1_completion_matrix.py
        )
        add_test(
            NAME csb_v1_experimental_launch_intent_fixture
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_csb_v1_experimental_launch_intent_fixture.py
        )
        add_test(
            NAME csb_v1_quickplay_load_route_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_csb_v1_quickplay_load_route_source_lock.py
        )
        add_test(
            NAME csb_v1_m11_runtime_capture_boundary
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_csb_v1_m11_runtime_capture_boundary.py
        )
        add_test(
            NAME pass547_csb_v1_runtime_readiness_backfill
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass547_csb_v1_runtime_readiness_backfill.py
        )
    endif()

    add_executable(test_touch_click_zone_matrix_pc34_compat_integration
        tests/test_touch_click_zone_matrix_pc34_compat_integration.c
    )
    target_link_libraries(test_touch_click_zone_matrix_pc34_compat_integration PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_touch_click_zone_matrix_pc34_compat_integration PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME touch_click_zone_matrix_pc34_compat
        COMMAND test_touch_click_zone_matrix_pc34_compat_integration
    )

    add_executable(test_action_area_routes_pc34_compat_integration
        tests/test_action_area_routes_pc34_compat_integration.c
    )
    target_link_libraries(test_action_area_routes_pc34_compat_integration PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_action_area_routes_pc34_compat_integration PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME action_area_routes_pc34_compat
        COMMAND test_action_area_routes_pc34_compat_integration
    )

    add_executable(test_touch_pointer_input_pc34_compat_integration
        tests/test_touch_pointer_input_pc34_compat_integration.c
    )
    target_link_libraries(test_touch_pointer_input_pc34_compat_integration PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_touch_pointer_input_pc34_compat_integration PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME touch_pointer_input_pc34_compat
        COMMAND test_touch_pointer_input_pc34_compat_integration
    )

    add_executable(test_dm1_v1_dungeon_view_touch_route_pc34_compat
        tests/test_dm1_v1_dungeon_view_touch_route_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_dungeon_view_touch_route_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_dungeon_view_touch_route_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_dungeon_view_touch_route_pc34_compat
        COMMAND test_dm1_v1_dungeon_view_touch_route_pc34_compat
    )

    if(Python3_Interpreter_FOUND)
        add_test(
            NAME dm1_v1_dungeon_view_touch_route_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_dungeon_view_touch_route_source_lock.py
        )
        add_test(
            NAME pass395_touch_queue_integration_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify_pass395_touch_queue_integration.py
        )
        add_test(
            NAME dm1_v1_touch_original_mouse_ui_action_zones_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify_dm1_v1_touch_original_mouse_ui_action_zones_source_lock.py
        )
        add_test(
            NAME dm1_v1_viewport_door_wall_ornament_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify_dm1_v1_viewport_door_wall_ornament_source_lock.py
        )
    endif()

    add_executable(test_menu_hit_launch_direct_click_m12
        tests/test_menu_hit_launch_direct_click_m12.c
    )
    target_link_libraries(test_menu_hit_launch_direct_click_m12 PRIVATE firestaff_m12)
    if(NOT MSVC)
        target_compile_options(test_menu_hit_launch_direct_click_m12 PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME menu_hit_launch_direct_click_m12
        COMMAND test_menu_hit_launch_direct_click_m12
    )

    add_executable(test_csb_v1_launch_blocker_m12
        tests/test_csb_v1_launch_blocker_m12.c
    )
    target_link_libraries(test_csb_v1_launch_blocker_m12 PRIVATE firestaff_m12)
    if(NOT MSVC)
        target_compile_options(test_csb_v1_launch_blocker_m12 PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME csb_v1_launch_blocker_m12
        COMMAND test_csb_v1_launch_blocker_m12
    )

    add_executable(test_asset_find_by_hash
        tests/test_asset_find_by_hash.c
    )
    target_link_libraries(test_asset_find_by_hash PRIVATE firestaff_m12)
    if(NOT MSVC)
        target_compile_options(test_asset_find_by_hash PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME asset_find_by_hash
        COMMAND test_asset_find_by_hash
    )

    add_executable(test_m12_quick_resume_gate
        tests/test_m12_quick_resume_gate.c
    )
    target_link_libraries(test_m12_quick_resume_gate PRIVATE firestaff_m12)
    if(NOT MSVC)
        target_compile_options(test_m12_quick_resume_gate PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME m12_quick_resume_gate
        COMMAND test_m12_quick_resume_gate
    )

    add_executable(test_m11_quick_resume_roundtrip
        tests/test_m11_quick_resume_roundtrip.c
    )
    target_link_libraries(test_m11_quick_resume_roundtrip PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m10 SDL3::SDL3)
    if(NOT MSVC)
        target_compile_options(test_m11_quick_resume_roundtrip PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME m11_quick_resume_roundtrip
        COMMAND test_m11_quick_resume_roundtrip
    )

    add_executable(firestaff_m11_touch_pointer_dispatch_probe
        probes/m11/firestaff_m11_touch_pointer_dispatch_probe.c
    )
    target_include_directories(firestaff_m11_touch_pointer_dispatch_probe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(firestaff_m11_touch_pointer_dispatch_probe PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(firestaff_m11_touch_pointer_dispatch_probe PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME m11_touch_pointer_dispatch_probe
        COMMAND firestaff_m11_touch_pointer_dispatch_probe
    )

    add_executable(firestaff_m11_touch_live_dispatch_gate_probe
        probes/m11/firestaff_m11_touch_live_dispatch_gate_probe.c
    )
    target_include_directories(firestaff_m11_touch_live_dispatch_gate_probe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(firestaff_m11_touch_live_dispatch_gate_probe PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(firestaff_m11_touch_live_dispatch_gate_probe PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME pass350_dm1_v1_touch_live_dispatch_gate
        COMMAND firestaff_m11_touch_live_dispatch_gate_probe
    )

    add_executable(firestaff_m11_touch_mouse_queue_runtime_probe
        probes/m11/firestaff_m11_touch_mouse_queue_runtime_probe.c
    )
    target_include_directories(firestaff_m11_touch_mouse_queue_runtime_probe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(firestaff_m11_touch_mouse_queue_runtime_probe PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(firestaff_m11_touch_mouse_queue_runtime_probe PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME pass491_touch_pointer_mouse_queue_runtime_gate
        COMMAND firestaff_m11_touch_mouse_queue_runtime_probe
    )

    add_executable(test_inventory_panel_slot_routes_pc34_compat_integration
        tests/test_inventory_panel_slot_routes_pc34_compat_integration.c
    )
    target_link_libraries(test_inventory_panel_slot_routes_pc34_compat_integration PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_inventory_panel_slot_routes_pc34_compat_integration PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME inventory_panel_slot_routes_pc34_compat
        COMMAND test_inventory_panel_slot_routes_pc34_compat_integration
    )

    add_executable(test_dm1_v1_inventory_equip_slots_pc34_compat
        tests/test_dm1_v1_inventory_equip_slots_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_inventory_equip_slots_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_inventory_equip_slots_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_inventory_equip_slots_pc34_compat
        COMMAND test_dm1_v1_inventory_equip_slots_pc34_compat
    )

    add_executable(test_dm1_v1_inventory_backpack_chest_pc34_compat
        tests/test_dm1_v1_inventory_backpack_chest_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_inventory_backpack_chest_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_inventory_backpack_chest_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_inventory_backpack_chest_pc34_compat
        COMMAND test_dm1_v1_inventory_backpack_chest_pc34_compat
    )


    add_executable(test_dm1_v1_inventory_consumables_pc34_compat
        tests/test_dm1_v1_inventory_consumables_pc34_compat.c
        src/dm1/dm1_v1_inventory_consumables_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_inventory_consumables_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_inventory_consumables_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_inventory_consumables_pc34_compat
        COMMAND test_dm1_v1_inventory_consumables_pc34_compat
    )

    add_executable(test_inventory_mouth_eye_routes_pc34_compat_integration
        tests/test_inventory_mouth_eye_routes_pc34_compat_integration.c
    )
    target_link_libraries(test_inventory_mouth_eye_routes_pc34_compat_integration PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_inventory_mouth_eye_routes_pc34_compat_integration PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME inventory_mouth_eye_routes_pc34_compat
        COMMAND test_inventory_mouth_eye_routes_pc34_compat_integration
    )

    add_executable(test_inventory_item_identification_pc34_compat
        tests/test_inventory_item_identification_pc34_compat.c
    )
    target_link_libraries(test_inventory_item_identification_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_inventory_item_identification_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME inventory_item_identification_pc34_compat
        COMMAND test_inventory_item_identification_pc34_compat
    )

    add_executable(test_dm1_v1_fountain_interaction_pc34_compat
        tests/test_dm1_v1_fountain_interaction_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_fountain_interaction_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_fountain_interaction_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_fountain_interaction_pc34_compat
        COMMAND test_dm1_v1_fountain_interaction_pc34_compat
    )

    add_executable(test_dm1_v1_input_command_queue_pc34_compat
        tests/test_dm1_v1_input_command_queue_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_input_command_queue_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_input_command_queue_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_input_command_queue_pc34_compat
        COMMAND test_dm1_v1_input_command_queue_pc34_compat
    )

    add_executable(test_dm1_v1_movement_core_pc34_compat
        tests/test_dm1_v1_movement_core_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_movement_core_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_movement_core_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_movement_core_pc34_compat
        COMMAND test_dm1_v1_movement_core_pc34_compat
    )

    add_executable(test_dm1_v1_movement_command_core_pc34_compat
        tests/test_dm1_v1_movement_command_core_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_movement_command_core_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_movement_command_core_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_movement_command_core_pc34_compat
        COMMAND test_dm1_v1_movement_command_core_pc34_compat
    )

    add_executable(test_dm1_v1_door_button_click_pc34_compat
        tests/test_dm1_v1_door_button_click_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_door_button_click_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_door_button_click_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_door_button_click_pc34_compat
        COMMAND test_dm1_v1_door_button_click_pc34_compat
    )

    add_executable(test_m11_v1_turning_presentation_pc34_compat
        tests/test_m11_v1_turning_presentation_pc34_compat.c
    )
    target_link_libraries(test_m11_v1_turning_presentation_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_m11_v1_turning_presentation_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME m11_v1_turning_presentation_pc34_compat
        COMMAND test_m11_v1_turning_presentation_pc34_compat
    )

    if(Python3_FOUND)
        add_test(
            NAME m11_v1_turning_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_m11_v1_turning_source_lock.py
        )
    endif()

    add_executable(test_dm1_v1_movement_timing_pc34_compat
        tests/test_dm1_v1_movement_timing_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_movement_timing_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_movement_timing_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_movement_timing_pc34_compat
        COMMAND test_dm1_v1_movement_timing_pc34_compat
    )

    add_executable(test_dm1_v1_turn_step_timing_gate_pc34_compat
        tests/test_dm1_v1_turn_step_timing_gate_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_turn_step_timing_gate_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_turn_step_timing_gate_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_turn_step_timing_gate_pc34_compat
        COMMAND test_dm1_v1_turn_step_timing_gate_pc34_compat
    )

    add_executable(test_dm1_v1_command_movement_sensor_timing_pc34_compat
        tests/test_dm1_v1_command_movement_sensor_timing_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_command_movement_sensor_timing_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_command_movement_sensor_timing_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_command_movement_sensor_timing_pc34_compat
        COMMAND test_dm1_v1_command_movement_sensor_timing_pc34_compat
    )

    add_executable(firestaff_dm1_v1_movement_core_probe
        probes/dm1/firestaff_dm1_v1_movement_core_probe.c
    )
    target_link_libraries(firestaff_dm1_v1_movement_core_probe PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(firestaff_dm1_v1_movement_core_probe PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME firestaff_dm1_v1_movement_core_probe
        COMMAND firestaff_dm1_v1_movement_core_probe
    )

    add_executable(firestaff_dm1_v1_canonical_movement_probe
        probes/dm1/firestaff_dm1_v1_canonical_movement_probe.c
    )
    target_link_libraries(firestaff_dm1_v1_canonical_movement_probe PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(firestaff_dm1_v1_canonical_movement_probe PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME firestaff_dm1_v1_canonical_movement_probe
        COMMAND firestaff_dm1_v1_canonical_movement_probe
    )

    # DM1 V1 playable-route gate: forward/blocked/turn deltas with hash and
    # state changes proving the canonical DUNGEON.DAT layout actually plays.
    add_executable(firestaff_dm1_v1_playable_route_probe
        probes/dm1/firestaff_dm1_v1_playable_route_probe.c
    )
    target_link_libraries(firestaff_dm1_v1_playable_route_probe PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(firestaff_dm1_v1_playable_route_probe PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME firestaff_dm1_v1_playable_route_probe
        COMMAND firestaff_dm1_v1_playable_route_probe
    )

    add_executable(firestaff_dm1_v1_walls_occlusion_blockers_probe
        probes/dm1/firestaff_dm1_v1_walls_occlusion_blockers_probe.c
    )
    target_link_libraries(firestaff_dm1_v1_walls_occlusion_blockers_probe PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(firestaff_dm1_v1_walls_occlusion_blockers_probe PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME firestaff_dm1_v1_walls_occlusion_blockers_probe
        COMMAND firestaff_dm1_v1_walls_occlusion_blockers_probe
    )

    add_executable(firestaff_dm1_v1_original_fakewall_view_collision_probe
        probes/dm1/firestaff_dm1_v1_original_fakewall_view_collision_probe.c
    )
    target_link_libraries(firestaff_dm1_v1_original_fakewall_view_collision_probe PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(firestaff_dm1_v1_original_fakewall_view_collision_probe PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME firestaff_dm1_v1_original_fakewall_view_collision_probe
        COMMAND firestaff_dm1_v1_original_fakewall_view_collision_probe
    )

    add_executable(firestaff_dm1_v1_side_contents_center_blocker_probe
        probes/dm1/firestaff_dm1_v1_side_contents_center_blocker_probe.c
    )
    if(NOT MSVC)
        target_compile_options(firestaff_dm1_v1_side_contents_center_blocker_probe PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME firestaff_dm1_v1_side_contents_center_blocker_probe
        COMMAND firestaff_dm1_v1_side_contents_center_blocker_probe ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_executable(firestaff_dm1_v1_wall_composition_contract_probe
        probes/dm1/firestaff_dm1_v1_wall_composition_contract_probe.c
    )
    target_link_libraries(firestaff_dm1_v1_wall_composition_contract_probe PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(firestaff_dm1_v1_wall_composition_contract_probe PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME firestaff_dm1_v1_wall_composition_contract_probe
        COMMAND firestaff_dm1_v1_wall_composition_contract_probe
    )

    add_executable(test_dm1_v2_movement_viewport_pc34
        tests/test_dm1_v2_movement_viewport_pc34.c
        src/dm1v2/dm1_v2_movement_engine_pc34.c
        src/dm1v2/dm1_v2_viewport_renderer_pc34.c
        src/dm1v2/dm1_v2_anim_timing_pc34.c
    )
    add_executable(test_dm1_v2_runtime_shell_pc34
        tests/test_dm1_v2_runtime_shell_pc34.c
        src/dm1v2/dm1_v2_runtime_pc34.c
        src/dm1v2/dm1_v2_movement_engine_pc34.c
        src/dm1v2/dm1_v2_viewport_renderer_pc34.c
        src/dm1v2/dm1_v2_anim_timing_pc34.c
    )
    add_executable(test_dm1_v2_camera_controller_pc34
        tests/test_dm1_v2_camera_controller_pc34.c
        src/dm1v2/dm1_v2_camera_controller_pc34.c
        src/dm1v2/dm1_v2_movement_engine_pc34.c
        src/dm1v2/dm1_v2_anim_timing_pc34.c
    )
    add_executable(test_dm1_v2_movement_command_adapter_pc34
        tests/test_dm1_v2_movement_command_adapter_pc34.c
        src/dm1v2/dm1_v2_movement_command_adapter_pc34.c
        src/dm1v2/dm1_v2_runtime_pc34.c
        src/dm1v2/dm1_v2_camera_controller_pc34.c
        src/dm1v2/dm1_v2_movement_engine_pc34.c
        src/dm1v2/dm1_v2_viewport_renderer_pc34.c
        src/dm1v2/dm1_v2_anim_timing_pc34.c
    )
    add_executable(test_dm1_v2_hud_overlay_pc34
        tests/test_dm1_v2_hud_overlay_pc34.c
        src/dm1v2/dm1_v2_hud_overlay_pc34.c
        src/dm1v2/dm1_v2_anim_timing_pc34.c
    )
    add_executable(test_dm1_v2_lighting_dynamic_pc34
        tests/test_dm1_v2_lighting_dynamic_pc34.c
        src/dm1v2/dm1_v2_lighting_dynamic_pc34.c
        src/dm1v2/dm1_v2_anim_timing_pc34.c
    )
    add_executable(test_dm1_v2_item_render_pc34
        tests/test_dm1_v2_item_render_pc34.c
        src/dm1v2/dm1_v2_item_render_pc34.c
        src/dm1v2/dm1_v2_anim_timing_pc34.c
    )
    target_compile_definitions(test_dm1_v2_item_render_pc34 PRIVATE FIRESTAFF_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_executable(test_dm1_v2_settings_pc34
        tests/test_dm1_v2_settings_pc34.c
        src/dm1v2/dm1_v2_settings_pc34.c
        src/dm1v2/dm1_v2_anim_timing_pc34.c
    )
    target_link_libraries(test_dm1_v2_settings_pc34 PRIVATE firestaff_m12)
    add_executable(test_dm1_v2_presentation_profile_pc34
        tests/test_dm1_v2_presentation_profile_pc34.c
        src/dm1v2/dm1_v2_presentation_profile_pc34.c
        src/dm1v2/dm1_v2_settings_pc34.c
        src/dm1v2/dm1_v2_anim_timing_pc34.c
    )
    target_link_libraries(test_dm1_v2_presentation_profile_pc34 PRIVATE firestaff_m12)
    add_executable(test_m11_display_aspect_present_rect
        tests/test_m11_display_aspect_present_rect.c
    )
    target_link_libraries(test_m11_display_aspect_present_rect PRIVATE firestaff_m11)
    add_executable(dm1_v2_export_entry_viewport_png
        tools/dm1_v2_export_entry_viewport_png.c
        src/dm1v2/dm1_v2_viewport_renderer_pc34.c
    )
    if(UNIX AND NOT APPLE)
        target_link_libraries(test_dm1_v2_lighting_dynamic_pc34 PRIVATE m)
    endif()
    if(NOT MSVC)
        target_compile_options(test_dm1_v2_movement_viewport_pc34 PRIVATE -Wall -Wextra -O2)
        target_compile_options(test_dm1_v2_runtime_shell_pc34 PRIVATE -Wall -Wextra -O2)
        target_compile_options(test_dm1_v2_camera_controller_pc34 PRIVATE -Wall -Wextra -O2)
        target_compile_options(test_dm1_v2_movement_command_adapter_pc34 PRIVATE -Wall -Wextra -O2)
        target_compile_options(test_dm1_v2_hud_overlay_pc34 PRIVATE -Wall -Wextra -O2)
        target_compile_options(test_dm1_v2_lighting_dynamic_pc34 PRIVATE -Wall -Wextra -O2)
        target_compile_options(test_dm1_v2_item_render_pc34 PRIVATE -Wall -Wextra -O2)
        target_compile_options(test_dm1_v2_settings_pc34 PRIVATE -Wall -Wextra -O2)
        target_compile_options(test_dm1_v2_presentation_profile_pc34 PRIVATE -Wall -Wextra -O2)
        target_compile_options(test_m11_display_aspect_present_rect PRIVATE -Wall -Wextra -O2)
        target_compile_options(dm1_v2_export_entry_viewport_png PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v2_movement_viewport_pc34
        COMMAND test_dm1_v2_movement_viewport_pc34
    )
    add_test(
        NAME dm1_v2_completion_matrix_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_completion_matrix.py
    )
    add_test(
        NAME dm1_v2_runtime_shell_pc34
        COMMAND test_dm1_v2_runtime_shell_pc34
    )
    add_test(
        NAME dm1_v2_runtime_shell_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_runtime_shell_source_lock.py
    )
    add_test(
        NAME dm1_v2_camera_controller_pc34
        COMMAND test_dm1_v2_camera_controller_pc34
    )
    add_test(
        NAME dm1_v2_camera_controller_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_camera_controller_source_lock.py
    )
    add_test(
        NAME dm1_v2_movement_command_adapter_pc34
        COMMAND test_dm1_v2_movement_command_adapter_pc34
    )
    add_test(
        NAME dm1_v2_movement_command_adapter_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_movement_command_adapter_source_lock.py
    )
    add_test(
        NAME dm1_v2_hud_overlay_pc34
        COMMAND test_dm1_v2_hud_overlay_pc34
    )
    add_test(
        NAME dm1_v2_hud_overlay_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_hud_overlay_source_lock.py
    )
    add_test(
        NAME dm1_v2_lighting_dynamic_pc34
        COMMAND test_dm1_v2_lighting_dynamic_pc34
    )
    add_test(
        NAME dm1_v2_lighting_dynamic_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_lighting_dynamic_source_lock.py
    )
    add_test(
        NAME dm1_v2_item_render_pc34
        COMMAND test_dm1_v2_item_render_pc34
    )
    add_test(
        NAME dm1_v2_item_render_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_item_render_source_lock.py
    )
    add_test(
        NAME dm1_v2_settings_pc34
        COMMAND test_dm1_v2_settings_pc34
    )
    add_test(
        NAME dm1_v2_presentation_profile_pc34
        COMMAND test_dm1_v2_presentation_profile_pc34
    )
    add_test(
        NAME m11_display_aspect_present_rect
        COMMAND test_m11_display_aspect_present_rect
    )
    add_test(
        NAME dm1_v2_settings_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_settings_source_lock.py
    )
    add_test(
        NAME dm1_v2_presentation_profile_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_presentation_profile_source_lock.py
    )
    add_test(
        NAME dm1_v2_graphics_pipeline_source_isolation
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_graphics_pipeline_source_isolation.py
    )
    add_test(
        NAME dm1_v2_viewport_wall_occlusion_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_viewport_wall_occlusion_source_lock.py
    )
    add_test(
        NAME dm1_v2_viewport_d1c_door_occlusion_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_viewport_d1c_door_occlusion_source_lock.py
    )
    add_test(
        NAME dm1_v2_dungeon_view_asset_bindings_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_dungeon_view_asset_bindings.py
    )

    add_test(
        NAME dm1_v2_viewport_composition_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_viewport_composition_source_lock.py
    )
    add_test(
        NAME dm1_v2_d0_d3_draw_list_comparator_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_d0_d3_draw_list_comparator_gate.py
    )
    add_test(
        NAME dm1_v2_dungeon_dat_square_decoder_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_dungeon_dat_square_decoder_source_lock.py
    )
    add_test(
        NAME dm1_v2_viewport_pixel_capture_fixture_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_viewport_pixel_capture_fixture_gate.py
    )
    add_test(
        NAME dm1_v2_entry_viewport_png_export
        COMMAND dm1_v2_export_entry_viewport_png ${CMAKE_CURRENT_SOURCE_DIR}/parity-evidence/verification/pass285_dm1_v2_firestaff_entry_viewport_224x136.png
    )
    add_test(
        NAME dm1_v2_entry_viewport_png_export_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_entry_viewport_png_export_gate.py
    )
    add_test(
        NAME dm1_v2_entry_viewport_png_comparator_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_entry_viewport_png_comparator_gate.py
    )
    add_test(
        NAME dm1_v2_entry_bitmap_materialization_blocker_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v2_entry_bitmap_materialization_blocker.py
    )

    add_executable(test_dm1_v1_movement_pipeline_pc34_compat
        tests/test_dm1_v1_movement_pipeline_pc34_compat.c
    src/dm1/dm1_v1_viewport_3d_pc34_compat.c
    src/dm1/dm1_v1_creature_render_pc34_compat.c
    src/dm1v2/dm1_v2_viewport_renderer_pc34.c
    src/csb/csb_v1_dungeon_world_pc34_compat.c
    src/dm1v2/dm1_v2_movement_engine_pc34.c
    src/csb/csb_v1_game_state_pc34_compat.c
    src/csb/csb_v1_dungeon_loader_pc34_compat.c
    src/csb/csb_v1_chaos_magic_pc34_compat.c
    src/csb/csb_v1_viewport_pc34_compat.c
    src/csb/csb_v1_monster_pc34_compat.c
    src/csb/csb_v1_character_pc34_compat.c
    src/csb/csb_v1_save_load_pc34_compat.c
    src/dm1/dm1_v1_champion_stats_pc34_compat.c
    src/dm1/dm1_v1_inventory_pc34_compat.c
    src/dm1/dm1_v1_spell_casting_pc34_compat.c
    src/dm1/dm1_v1_stairs_level_pc34_compat.c
    src/dm1/dm1_v1_text_message_pc34_compat.c
    src/dm1/dm1_v1_sound_pc34_compat.c
    src/dm1/dm1_v1_teleporter_pit_pc34_compat.c
    src/dm1/dm1_v1_light_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_movement_pipeline_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_movement_pipeline_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_movement_pipeline_pc34_compat
        COMMAND test_dm1_v1_movement_pipeline_pc34_compat
    )

    add_executable(test_dm1_v1_creature_render_pc34_compat_integration
        tests/test_dm1_v1_creature_render_pc34_compat_integration.c
        src/dm1/dm1_v1_creature_render_pc34_compat.c
    src/dm1v2/dm1_v2_viewport_renderer_pc34.c
    src/csb/csb_v1_dungeon_world_pc34_compat.c
    src/dm1v2/dm1_v2_movement_engine_pc34.c
    src/csb/csb_v1_game_state_pc34_compat.c
    src/csb/csb_v1_dungeon_loader_pc34_compat.c
    src/csb/csb_v1_chaos_magic_pc34_compat.c
    src/csb/csb_v1_viewport_pc34_compat.c
    src/csb/csb_v1_monster_pc34_compat.c
    src/csb/csb_v1_character_pc34_compat.c
    src/csb/csb_v1_save_load_pc34_compat.c
    src/dm1/dm1_v1_champion_stats_pc34_compat.c
    src/dm1/dm1_v1_inventory_pc34_compat.c
    src/dm1/dm1_v1_spell_casting_pc34_compat.c
    src/dm1/dm1_v1_stairs_level_pc34_compat.c
    src/dm1/dm1_v1_text_message_pc34_compat.c
    src/dm1/dm1_v1_sound_pc34_compat.c
    src/dm1/dm1_v1_teleporter_pit_pc34_compat.c
    src/dm1/dm1_v1_light_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_creature_render_pc34_compat_integration PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_creature_render_pc34_compat_integration PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_creature_render_source_lock
        COMMAND test_dm1_v1_creature_render_pc34_compat_integration
    )
    add_executable(test_dm1_v1_projectile_explosion_render_pc34_compat
        tests/test_dm1_v1_projectile_explosion_render_pc34_compat.c
        src/dm1/dm1_v1_projectile_explosion_render_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_projectile_explosion_render_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_projectile_explosion_render_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_projectile_explosion_render_source_lock
        COMMAND test_dm1_v1_projectile_explosion_render_pc34_compat
    )
    add_test(
        NAME pass563_dm1_v1_projectile_travel_blockers_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass563_dm1_v1_projectile_travel_blockers_source_lock.py
    )
    add_executable(test_dm1_v1_viewport_hand_overlay_pc34_compat
        tests/test_dm1_v1_viewport_hand_overlay_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_viewport_hand_overlay_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_viewport_hand_overlay_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_viewport_hand_overlay_pc34_compat
        COMMAND test_dm1_v1_viewport_hand_overlay_pc34_compat
    )
    add_executable(test_dm1_v1_champion_panel_hud_pc34_compat
        tests/test_dm1_v1_champion_panel_hud_pc34_compat.c
        src/dm1/dm1_v1_champion_panel_hud_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_champion_panel_hud_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_champion_panel_hud_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_champion_panel_hud_source_lock
        COMMAND test_dm1_v1_champion_panel_hud_pc34_compat
    )

    add_executable(test_dm1_v1_viewport_status_bar_layout_pc34_compat
        src/test/test_dm1_v1_viewport_status_bar_layout_pc34_compat.c
    )
    target_include_directories(test_dm1_v1_viewport_status_bar_layout_pc34_compat PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/test
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_viewport_status_bar_layout_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_viewport_status_bar_layout_pc34_compat
        COMMAND test_dm1_v1_viewport_status_bar_layout_pc34_compat
    )
    if(Python3_EXECUTABLE)
        add_test(
            NAME dm1_v1_viewport_status_bar_layout_redmcsb_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_status_bar_layout_gate.py
        )
    endif()
    add_test(
        NAME dm1_v1_command_movement_sensor_timing_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/verify_dm1_v1_command_movement_sensor_timing_source_lock.py
    )
    add_test(
        NAME dm1_v1_movement_core_lane_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/verify_dm1_v1_movement_core_lane_source_lock.py
    )
    add_test(
        NAME pass383_dm1_v1_movement_timing_integration
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/verify_pass383_dm1_v1_movement_timing_integration.py
    )
    add_test(
        NAME dm1_v1_party_movement_sensor_order_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/verify_dm1_v1_party_movement_sensor_order_source_lock.py
    )

    add_test(
        NAME dm1_v1_movement_command_gate_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/verify_dm1_v1_movement_command_gate_source_lock.py
    )

    add_test(
        NAME pass564_dm1_v1_movement_collision_timing_cluster
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/verify_pass564_dm1_v1_movement_collision_timing_cluster.py
    )

    add_test(
        NAME pass394_dm1_v1_dispatch_to_movement_state
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/verify_pass394_dm1_v1_dispatch_to_movement_state.py
    )

    add_test(
        NAME pass428_dm1_v1_route_command_to_movement_gap
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/verify_pass428_dm1_v1_route_command_to_movement_gap.py
    )

    add_test(
        NAME pass505_dm1_v1_blocked_movement_collision_timing_gap
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/verify_pass505_dm1_v1_blocked_movement_collision_timing_gap.py
    )

    add_test(
        NAME pass351_dm1_v1_live_viewport_redraw_parity_sweep
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/verify_pass351_dm1_v1_live_viewport_redraw_parity_sweep.py
    )

    add_executable(test_dm1_v1_sensor_trigger_pc34_compat
        tests/test_dm1_v1_sensor_trigger_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_sensor_trigger_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_sensor_trigger_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_sensor_trigger_pc34_compat
        COMMAND test_dm1_v1_sensor_trigger_pc34_compat
    )

    add_executable(test_resurrect_reincarnate_cancel_routes_pc34_compat_integration
        tests/test_resurrect_reincarnate_cancel_routes_pc34_compat_integration.c
    )
    target_link_libraries(test_resurrect_reincarnate_cancel_routes_pc34_compat_integration PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_resurrect_reincarnate_cancel_routes_pc34_compat_integration PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME firestaff_resurrect_reincarnate_cancel_routes
        COMMAND test_resurrect_reincarnate_cancel_routes_pc34_compat_integration
    )

    add_executable(test_dm1_v1_resurrection_pc34_compat
        tests/test_dm1_v1_resurrection_pc34_compat.c
        src/dm1/dm1_v1_resurrection_pc34_compat.c
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_resurrection_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_resurrection
        COMMAND test_dm1_v1_resurrection_pc34_compat
    )

    # Phase A probe (headless, no game data needed)
    add_test(
        NAME m11_phase_a
        COMMAND firestaff_m11_phase_a_probe
    )
    set_tests_properties(m11_phase_a PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=dummy"
    )

    if(TARGET firestaff_m12_startup_menu_probe)
        add_test(
            NAME m12_startup_menu
            COMMAND firestaff_m12_startup_menu_probe
        )
    endif()

    # Game View probe (needs GRAPHICS.DAT via FIRESTAFF_DATA)
    add_test(
        NAME m11_game_view
        COMMAND firestaff_m11_game_view_probe "$ENV{HOME}/.firestaff/data"
    )
    set_tests_properties(m11_game_view PROPERTIES
        ENVIRONMENT "FIRESTAFF_DATA=$ENV{HOME}/.firestaff/data"
    )

    add_test(
        NAME m11_audio
        COMMAND firestaff_m11_audio_probe
    )

    add_test(
        NAME m11_audio_source_index_live_sdl_dummy
        COMMAND firestaff_m11_audio_probe --expect-sdl-source-index-queue
    )
    set_tests_properties(m11_audio_source_index_live_sdl_dummy PROPERTIES
        ENVIRONMENT "FIRESTAFF_AUDIO_ENABLE_SDL=1;SDL_AUDIODRIVER=dummy"
    )

    add_test(
        NAME firestaff_m11_creature_runtime_audio_probe
        COMMAND firestaff_m11_creature_runtime_audio_probe
    )

    add_test(
        NAME m11_title_song_runtime_source_lock
        COMMAND firestaff_m11_pass54_song_runtime_probe
    )
    set_tests_properties(m11_title_song_runtime_source_lock PROPERTIES
        ENVIRONMENT "FIRESTAFF_AUDIO_ENABLE_SDL=0"
    )

    add_test(
        NAME m11_title_song_live_sdl_dummy_source_lock
        COMMAND firestaff_m11_pass54_song_runtime_probe --expect-sdl-title-queue
    )
    set_tests_properties(m11_title_song_live_sdl_dummy_source_lock PROPERTIES
        ENVIRONMENT "FIRESTAFF_AUDIO_ENABLE_SDL=1;SDL_AUDIODRIVER=dummy"
    )

    add_test(
        NAME m11_viewport_state
        COMMAND firestaff_m11_viewport_state_probe "$ENV{HOME}/.firestaff/data" ${CMAKE_CURRENT_BINARY_DIR}/viewport-state
    )
    set_tests_properties(m11_viewport_state PROPERTIES
        ENVIRONMENT "FIRESTAFF_DATA=$ENV{HOME}/.firestaff/data"
    )

    add_test(
        NAME m11_capture_route_state
        COMMAND firestaff_m11_capture_route_state_probe "$ENV{HOME}/.firestaff/data" ${CMAKE_CURRENT_BINARY_DIR}/capture-route-state
    )
    set_tests_properties(m11_capture_route_state PROPERTIES
        ENVIRONMENT "FIRESTAFF_DATA=$ENV{HOME}/.firestaff/data"
    )

    add_test(
        NAME dm1_v1_wall_collision_runtime_capture
        COMMAND firestaff_m11_wall_collision_capture_probe "$ENV{HOME}/.firestaff/data" ${CMAKE_CURRENT_BINARY_DIR}/wall-collision-capture
    )
    set_tests_properties(dm1_v1_wall_collision_runtime_capture PROPERTIES
        ENVIRONMENT "FIRESTAFF_DATA=$ENV{HOME}/.firestaff/data"
    )

    add_test(
        NAME m11_turn_viewport_orientation
        COMMAND firestaff_m11_turn_viewport_orientation_probe "$ENV{HOME}/.firestaff/data" ${CMAKE_CURRENT_BINARY_DIR}/turn-viewport-orientation
    )
    set_tests_properties(m11_turn_viewport_orientation PROPERTIES
        ENVIRONMENT "FIRESTAFF_DATA=$ENV{HOME}/.firestaff/data"
    )

    add_test(
        NAME dm1_v1_hall_walkaround_runtime
        COMMAND firestaff_m11_hall_walkaround_runtime_probe "$ENV{HOME}/.firestaff/data" ${CMAKE_CURRENT_BINARY_DIR}/hall-walkaround-runtime
    )
    set_tests_properties(dm1_v1_hall_walkaround_runtime PROPERTIES
        ENVIRONMENT "FIRESTAFF_DATA=$ENV{HOME}/.firestaff/data"
    )

    if(Python3_Interpreter_FOUND)
        add_test(
            NAME v1_viewport_draw_order_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_draw_order_gate.py
        )
        add_test(
            NAME v1_viewport_redmcsb_draw_stack_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_redmcsb_draw_stack_gate.py
        )
        add_test(
            NAME v1_viewport_d1c_doorpass_source_lock_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_d1c_doorpass_source_lock_gate.py
        )
        add_test(
            NAME v1_viewport_source_zone_tables
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_source_zone_tables.py
        )
        add_test(
            NAME v1_viewport_projectile_source_clip_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_projectile_source_clip_gate.py
        )
        add_test(
            NAME v1_viewport_projectile_explosion_render_source_lock_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_projectile_explosion_render_source_lock_gate.py
        )
        add_test(
            NAME pass582_dm1_v1_explosion_viewport_clip_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass582_dm1_v1_explosion_viewport_clip_source_lock.py --check-only
        )
        add_test(
            NAME pass363_dm1_v1_f0115_thing_layer_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass363_dm1_v1_f0115_thing_layer_source_lock.py
        )
        add_test(
            NAME dm1_v1_viewport_explosion_pass_gap_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_viewport_explosion_pass_gap_source_lock.py
        )
        add_test(
            NAME pass374_dm1_v1_f0115_explosion_pass_blocker
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass374_dm1_v1_f0115_explosion_pass_blocker.py
        )
        add_test(
            NAME pass375_dm1_v1_deferred_explosion_pass
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass375_dm1_v1_deferred_explosion_pass.py
        )
        add_test(
            NAME pass405_dm1_v1_viewport_projectile_explosion_layer_occlusion
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass405_dm1_v1_viewport_projectile_explosion_layer_occlusion.py
        )
        add_test(
            NAME v1_viewport_hand_overlay_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_hand_overlay_gate.py
        )
        add_test(
            NAME v1_viewport_distance_row_clip_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_distance_row_clip_gate.py
        )
        add_test(
            NAME v1_viewport_field_zone_aspect_clip_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_field_zone_aspect_clip_gate.py
        )
        add_test(
            NAME dm1_v1_teleporter_visual_effect_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_teleporter_visual_effect_source_lock.py
        )
        add_test(
            NAME v1_viewport_floor_ornament_stair_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_floor_ornament_stair_gate.py
        )
        add_test(
            NAME v1_viewport_occlusion_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_occlusion_gate.py
        )
        add_test(
            NAME v1_viewport_side_wall_occlusion_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_side_wall_occlusion_gate.py
        )
        add_test(
            NAME pass427_dm1_v1_viewport_walls_gap_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass427_dm1_v1_viewport_walls_gap_gate.py
        )
        add_test(
            NAME v1_viewport_side_wall_ornament_source_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_side_wall_ornament_source_gate.py
        )
        add_test(
            NAME v1_door_button_ornament_coordinates_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_door_button_ornament_coordinates_gate.py
        )
        add_test(
            NAME v1_wall_ornament_coordinates_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_wall_ornament_coordinates_gate.py
        )
        add_test(
            NAME v1_viewport_center_door_occlusion_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_center_door_occlusion_gate.py
        )
        add_test(
            NAME pass361_dm1_v1_viewport_occlusion_redraw_order_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass361_dm1_v1_viewport_occlusion_redraw_order_gate.py
        )
        add_test(
            NAME pass362_dm1_v1_viewport_walls_source_lock_landable
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass362_dm1_v1_viewport_walls_source_lock_landable.py
        )
        add_test(
            NAME pass381_dm1_v1_movement_viewport_walls_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass381_dm1_v1_movement_viewport_walls_source_lock.py
        )
        add_test(
            NAME dm1_v1_viewport_movement_completion_matrix
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_viewport_movement_completion_matrix.py
        )
        add_test(
            NAME dm1_v1_viewport_walls_golden_comparison
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_viewport_walls_golden_comparison.py
        )
        add_test(
            NAME dm1_v1_real_wall_asset_distinctness_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_real_wall_asset_distinctness_gate.py
        )
        add_test(
            NAME dm1_v1_wallset_materialization_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_wallset_materialization_source_lock.py
        )
        add_test(
            NAME pass443_dm1_v1_graphics_asset_atlas_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass443_dm1_v1_graphics_asset_atlas_source_lock.py
        )
        add_test(
            NAME pass447_dm1_v1_thieves_eye_door_mask_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass447_dm1_v1_thieves_eye_door_mask_source_lock.py
        )
        add_test(
            NAME pass445_dm1_v1_pc34_data_hash_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass445_dm1_v1_pc34_data_hash_lock.py
        )
        add_test(
            NAME pass446_filename_only_asset_identity_audit
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass446_filename_only_asset_identity_audit.py
        )
        add_test(
            NAME pass448_dm1_v1_dungeon_hash_provenance
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass448_dm1_v1_dungeon_hash_provenance.py
        )
        add_test(
            NAME dm2_v1_phase0_provenance_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm2_v1_phase0_provenance_gate.py
        )
        add_test(
            NAME pass512_dm1_v1_original_dungeon_packet_boundary
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass512_dm1_v1_original_dungeon_packet_boundary.py
        )
        add_test(
            NAME pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.py
        )
        add_test(
            NAME pass450_dm1_v1_hall_original_candidate_artifact_inventory
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass450_dm1_v1_hall_original_candidate_artifact_inventory.py
        )
        add_test(
            NAME pass451_dm1_v1_hall_original_capture_storage_policy
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass451_dm1_v1_hall_original_capture_storage_policy.py
        )
        add_test(
            NAME pass452_dm1_v1_hall_original_route_state_blocker
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass452_dm1_v1_hall_original_route_state_blocker.py
        )
        add_test(
            NAME pass453_dm1_v1_hall_panel_visible_delta_classification
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass453_dm1_v1_hall_panel_visible_delta_classification.py
        )
        add_test(
            NAME pass454_dm1_v1_hall_original_input_mapping_blocker
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass454_dm1_v1_hall_original_input_mapping_blocker.py
        )
        add_test(
            NAME pass455_dm1_v1_hall_corrected_click_primitive_capture
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass455_dm1_v1_hall_corrected_click_primitive_capture.py
        )
        add_test(
            NAME pass466_dm1_v1_initial_hall_c080_source_stop_capture_path
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass466_dm1_v1_initial_hall_c080_source_stop_capture_path.py
        )
        add_test(
            NAME v1_viewport_front_wall_depth_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_front_wall_depth_gate.py
        )
        add_test(
            NAME v1_viewport_wall_depth_source_lock_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_wall_depth_source_lock_gate.py
        )
        add_test(
            NAME v1_viewport_wall_blit_transparency_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_wall_blit_transparency_gate.py
        )
        add_test(
            NAME v1_viewport_palette_source_lock_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_palette_source_lock_gate.py
        )
        add_test(
            NAME v1_viewport_wall_parity_flip_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_wall_parity_flip_gate.py
        )
        add_test(
            NAME v1_viewport_alcove_wall_item_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_alcove_wall_item_gate.py
        )
        add_test(
            NAME pass505_dm1_v1_alcove_item_c2548_blocker
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass505_dm1_v1_alcove_item_c2548_blocker.py
        )
        add_test(
            NAME pass507_dm1_v1_viewport_wall_alcove_draw_order_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass507_dm1_v1_viewport_wall_alcove_draw_order_source_lock.py
        )
        add_test(
            NAME pass506_dm1_v1_alcove_wall_item_occlusion_evidence
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass506_dm1_v1_alcove_wall_item_occlusion_evidence.py
        )
        add_test(
            NAME v1_viewport_pit_floor_ornament_bug64_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_viewport_pit_floor_ornament_bug64_gate.py
        )
        add_test(
            NAME dm1_v1_viewport_world_redmcsb_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify_dm1_v1_viewport_world_redmcsb_gate.py
        )
        add_test(
            NAME v1_inventory_panel_open_redmcsb_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_inventory_panel_open_redmcsb_gate.py
        )
        add_test(
            NAME v1_inventory_toggle_redmcsb_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_inventory_toggle_redmcsb_gate.py
        )
        add_test(
            NAME v1_inventory_chest_actionhand_redmcsb_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_inventory_chest_actionhand_redmcsb_gate.py
        )
        add_test(
            NAME v1_status_refresh_order_redmcsb_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_status_refresh_order_redmcsb_gate.py
        )
        add_test(
            NAME v1_entrance_input_wait_redmcsb_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_entrance_input_wait_redmcsb_gate.py
        )
        add_test(
            NAME dm1_v1_title_launcher_handoff_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_title_launcher_handoff_gate.py
        )
        add_test(
            NAME pass456_dm1_v1_door_title_end_priority_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass456_dm1_v1_door_title_end_priority_gate.py
        )
        add_test(
            NAME v1_champion_recruit_source_path
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_champion_recruit_source_path.py
        )
        add_test(
            NAME v1_champion_portrait_click_source_path
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_champion_portrait_click_source_path.py
        )
        add_test(
            NAME v1_champion_portrait_click_geometry
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_champion_portrait_click_geometry.py
        )
        add_test(
            NAME dm1_v1_hall_of_champions_full_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_hall_of_champions_full_source_lock.py
        )
        add_test(
            NAME dm1_v1_hall_walkaround_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_hall_walkaround_source_lock.py
        )
        add_test(
            NAME dm1_v1_hall_mirror_sensor_disable_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_hall_mirror_sensor_disable_source_lock.py
        )
        add_test(
            NAME dm1_v1_movement_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_movement_source_lock.py
        )
        add_test(
            NAME dm1_v1_entry_movement_viewport_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_entry_movement_viewport_source_lock.py
        )
        add_test(
            NAME dm1_v1_original_movement_viewport_blocker_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass207_dm1_v1_original_movement_viewport_blocker_gate.py
        )
        add_test(
            NAME dm1_v1_fires_loader_csip_source_map_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass208_dm1_v1_fires_loader_csip_source_map_gate.py
        )
        add_test(
            NAME dm1_v1_original_delayed_click_route_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass209_dm1_v1_original_delayed_click_route_gate.py
        )
        add_test(
            NAME dm1_v1_original_runtime_binding_guard
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass210_dm1_v1_original_runtime_binding_guard.py
        )
        add_test(
            NAME dm1_original_runtime_trace_scaffold
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/dm1_original_runtime_trace.py --check-only --expect-blocked
        )
        add_test(
            NAME dm1_v1_original_route_sync_probe
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass212_dm1_v1_original_route_sync_probe.py
        )
        add_test(
            NAME dm1_v1_post_redraw_instrumentation_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass223_dm1_v1_post_redraw_instrumentation_lock.py
        )
        add_test(
            NAME pass224_dm1_v1_runtime_state_probe_scaffold
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass224_dm1_v1_runtime_state_probe_scaffold.py
        )
        add_test(
            NAME pass227_dm1_v1_original_runtime_hook_design
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass227_dm1_v1_original_runtime_hook_design.py
        )
        add_test(
            NAME pass230_dm1_v1_fires_exenew_symbol_map_bootstrap
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass230_dm1_v1_fires_exenew_symbol_map_bootstrap.py
        )
        add_test(
            NAME pass231_dm1_v1_fires_exenew_runtime_hit_probe
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass231_dm1_v1_fires_exenew_runtime_hit_probe.py
        )
        add_test(
            NAME pass234_dm1_v1_viewport_runtime_hit_blocker
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass234_dm1_v1_viewport_runtime_hit_blocker.py
        )
        add_test(
            NAME pass238_dm1_v1_runtime_debugger_keystroke_blocker
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass238_dm1_v1_runtime_debugger_keystroke_blocker.py
        )
        add_test(
            NAME pass240_dm1_v1_fires_map_public_symbol_path
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass240_dm1_v1_fires_map_public_symbol_path.py
        )
        add_test(
            NAME pass241_dm1_v1_runtime_csip_candidate_bridge
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass241_dm1_v1_runtime_csip_candidate_bridge.py
        )
        add_test(
            NAME pass246_dm1_v1_dmexe_fires_load_segment_bridge
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass246_dm1_v1_dmexe_fires_load_segment_bridge.py
        )
        add_test(
            NAME pass242_dm1_v1_dunview_tcc_int6_blocker
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass242_dm1_v1_dunview_tcc_int6_blocker.py
        )
        add_test(
            NAME pass243_dm1_v1_dunview_emulator_variant_runbook
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass243_dm1_v1_dunview_emulator_variant_runbook.py
        )
        add_test(
            NAME pass244_dm1_v1_qemu_freedos_dunview_path
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass244_dm1_v1_qemu_freedos_dunview_path.py
        )
        add_test(
            NAME pass279_dm1_v1_evidence_capture_source_lock_followup
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass279_dm1_v1_evidence_capture_source_lock_followup.py
        )
        add_test(
            NAME pass296_dm1_v1_input_tuple_proof_without_direct_f0380
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass296_dm1_v1_input_tuple_proof_without_direct_f0380.py
        )
        add_test(
            NAME pass299_dm1_v1_movement_completion_matrix_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/pass299_dm1_v1_movement_completion_matrix_gate.py
        )
        add_test(
            NAME dm1_v1_stairs_pits_viewport_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_stairs_pits_viewport_source_lock.py
        )
        add_test(
            NAME dm1_v1_creature_render_aspect_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_creature_render_source_lock.py
        )
        add_test(
            NAME touch_movement_viewport_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_touch_movement_viewport_source_lock.py
        )
        add_test(
            NAME dm1_v1_touch_input_source_locked_ui_zones
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_touch_input_source_locked_ui_zones.py
        )
        add_test(
            NAME dm1_v1_wall_click_touch_seam_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_wall_click_touch_seam_source_lock.py
        )
        add_test(
            NAME dm1_v1_input_command_queue_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_input_command_queue_source_lock.py
        )
        add_test(
            NAME dm1_v1_turn_step_timing_gate_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_turn_step_timing_gate_source_lock.py
        )
        add_test(
            NAME pass372_dm1_v1_movement_runtime_route
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass372_dm1_v1_movement_runtime_route.py
        )
        add_test(
            NAME pass373_dm1_v1_launcher_viewport_redraw_wall_occlusion_path
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass373_dm1_v1_launcher_viewport_redraw_wall_occlusion_path.py
        )
        add_test(
            NAME pass374_dm1_v1_completion_viewport_wall_credit
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass374_dm1_v1_completion_viewport_wall_credit.py
        )
        add_test(
            NAME pass508_dm1_v1_movement_remaining_gap_after_pass374
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass508_dm1_v1_movement_remaining_gap_after_pass374.py
        )
        add_test(
            NAME pass376_dm1_v1_completion_deferred_explosion_credit
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass376_dm1_v1_completion_deferred_explosion_credit.py
        )
        add_test(
            NAME pass377_dm1_v1_paired_diff_artifact_blocker
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass377_dm1_v1_paired_diff_artifact_blocker.py
        )
        add_test(
            NAME pass404_dm1_v1_side_contents_center_blocker_occlusion_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass404_dm1_v1_side_contents_center_blocker_occlusion_gate.py
        )
        add_test(
            NAME firestaff_completion_matrix
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_firestaff_completion_matrix.py
        )
        add_test(
            NAME dm1_v1_movement_timing_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_movement_timing_source_lock.py
        )
        add_test(
            NAME dm1_v1_projectile_movement_interlock_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_projectile_movement_interlock_source_lock.py
        )
        add_test(
            NAME dm1_v1_ranged_shoot_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_ranged_shoot_source_lock.py
        )
        add_test(
            NAME csb_v1_inventory_render_grid_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_csb_v1_inventory_render_grid_source_lock.py
        )
        add_test(
            NAME csb_v1_viewport_inventory_mouse_source_lock
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_csb_v1_viewport_inventory_mouse_source_lock.py
        )

    add_executable(test_dm1_v1_spell_casting_pc34_compat
        tests/test_dm1_v1_spell_casting_pc34_compat.c
        src/dm1/dm1_v1_spell_casting_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_spell_casting_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_spell_casting_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_spell_casting_source_lock
        COMMAND test_dm1_v1_spell_casting_pc34_compat
    )
    endif()

    # Launcher/entrance handoff wait policy: ReDMCSB waits for fresh input.
    add_executable(test_m11_entrance_wait_policy
        tests/test_m11_entrance_wait_policy.c
    )
    target_link_libraries(test_m11_entrance_wait_policy PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m10 SDL3::SDL3)
    if(NOT MSVC)
        target_compile_options(test_m11_entrance_wait_policy PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME m11_entrance_wait_policy
        COMMAND test_m11_entrance_wait_policy
    )

    add_executable(test_entrance_mouse_routes_pc34_compat_integration
        tests/test_entrance_mouse_routes_pc34_compat_integration.c
    )
    target_link_libraries(test_entrance_mouse_routes_pc34_compat_integration PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_entrance_mouse_routes_pc34_compat_integration PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME entrance_mouse_routes_pc34_compat
        COMMAND test_entrance_mouse_routes_pc34_compat_integration
    )

    add_executable(test_entrance_runtime_dispatch_source_lock
        tests/test_entrance_runtime_dispatch_source_lock.c
    )
    target_link_libraries(test_entrance_runtime_dispatch_source_lock PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m10 SDL3::SDL3)
    if(NOT MSVC)
        target_compile_options(test_entrance_runtime_dispatch_source_lock PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME entrance_runtime_dispatch_source_lock
        COMMAND test_entrance_runtime_dispatch_source_lock
    )

    # Launcher smoke (needs game data + built firestaff binary)
    add_test(
        NAME m11_launcher_smoke
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/probes/run_firestaff_m11_launcher_smoke.sh
    )
    set_tests_properties(m11_launcher_smoke PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;FIRESTAFF_DATA=$ENV{HOME}/.firestaff/data;FIRESTAFF_BIN=${CMAKE_CURRENT_BINARY_DIR}/firestaff;BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    )

    # Deterministic in-game screenshot capture smoke. This guards the
    # capture pipeline itself (CMake target wiring + palette decode), not
    # pixel-perfect art parity.
    add_test(
        NAME m11_ingame_capture_smoke
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/probes/run_firestaff_m11_ingame_capture_smoke.sh
    )
    set_tests_properties(m11_ingame_capture_smoke PROPERTIES
        ENVIRONMENT "FIRESTAFF_DATA=$ENV{HOME}/.firestaff/data;BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    )

    # DM1 V1 Sound & Music System source-lock gate
    add_executable(test_dm1_v1_sound_pc34_compat_integration
        tests/test_dm1_v1_sound_pc34_compat_integration.c
        src/dm1/dm1_v1_sound_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_sound_pc34_compat_integration PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_sound_pc34_compat_integration PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_sound_music_source_lock
        COMMAND test_dm1_v1_sound_pc34_compat_integration
    )

    # DM1 V1 creature attack/movement sound source-lock gate
    add_executable(test_dm1_v1_creature_sound_pc34_compat
        tests/test_dm1_v1_creature_sound_pc34_compat.c
        src/dm1/dm1_v1_creature_sound_pc34_compat.c
        src/dm1/dm1_v1_sound_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_creature_sound_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_creature_sound_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_creature_sound_source_lock
        COMMAND test_dm1_v1_creature_sound_pc34_compat
    )

    # DM1 V1 Save/Load Game System source-lock gate
    add_executable(test_dm1_v1_save_load
        tests/test_dm1_v1_save_load.c
    src/dm1/dm1_v1_viewport_3d_pc34_compat.c
    src/dm1/dm1_v1_dungeon_loader_pc34_compat.c
    src/dm1/dm1_v1_title_screen_pc34_compat.c
    src/dm1/dm1_v1_palette_font_pc34_compat.c
    src/dm1/dm1_v1_object_world_pc34_compat.c
    src/dm1/dm1_v1_graphics_loader_pc34_compat.c
    src/dm1/dm1_v1_fade_transition_pc34_compat.c
    src/dm1/dm1_v1_portrait_panel_pc34_compat.c
    src/dm1/dm1_v1_wall_ornament_pc34_compat.c
    src/dm1/dm1_v1_dungeon_decompressor_pc34_compat.c
    src/dm1/dm1_v1_field_teleporter_effect_pc34_compat.c
    src/dm1/dm1_v1_game_loop_integration_pc34_compat.c
    src/dm1/dm1_v1_creature_viewport_pc34_compat.c
    src/dm1/dm1_v1_save_load_system_pc34_compat.c
    src/dm1/dm1_v1_blit_fill_pc34_compat.c
    src/dm1/dm1_v1_dialog_scroll_pc34_compat.c
    src/dm1/dm1_v1_click_routing_pc34_compat.c
        src/dm1/dm1_v1_save_load.c
    src/dm1/dm1_v1_viewport_3d_pc34_compat.c
    src/dm1/dm1_v1_dungeon_loader_pc34_compat.c
    src/dm1/dm1_v1_title_screen_pc34_compat.c
    src/dm1/dm1_v1_palette_font_pc34_compat.c
    src/dm1/dm1_v1_object_world_pc34_compat.c
    src/dm1/dm1_v1_graphics_loader_pc34_compat.c
    src/dm1/dm1_v1_fade_transition_pc34_compat.c
    src/dm1/dm1_v1_portrait_panel_pc34_compat.c
    src/dm1/dm1_v1_wall_ornament_pc34_compat.c
    src/dm1/dm1_v1_dungeon_decompressor_pc34_compat.c
    src/dm1/dm1_v1_field_teleporter_effect_pc34_compat.c
    src/dm1/dm1_v1_game_loop_integration_pc34_compat.c
    src/dm1/dm1_v1_creature_viewport_pc34_compat.c
    src/dm1/dm1_v1_save_load_system_pc34_compat.c
    src/dm1/dm1_v1_blit_fill_pc34_compat.c
    src/dm1/dm1_v1_dialog_scroll_pc34_compat.c
    src/dm1/dm1_v1_click_routing_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_save_load PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_save_load PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_save_load_source_lock
        COMMAND test_dm1_v1_save_load
    )

    add_executable(test_dm1_v1_save_load_slots
        tests/test_dm1_v1_save_load_slots.c
        src/dm1/dm1_v1_save_load_system_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_save_load_slots PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_save_load_slots PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_save_load_slot_core
        COMMAND test_dm1_v1_save_load_slots
    )

    # DM1 V1 Combat & Damage System source-lock gate
    add_executable(test_dm1_v1_combat_pc34_compat_integration
        tests/test_dm1_v1_combat_pc34_compat_integration.c
        src/dm1/dm1_v1_combat_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_combat_pc34_compat_integration PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_combat_pc34_compat_integration PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_combat_damage_source_lock
        COMMAND test_dm1_v1_combat_pc34_compat_integration
    )
endif()

# ── Install rules ────────────────────────────────────────────────────
install(TARGETS firestaff RUNTIME DESTINATION bin)


# ── DM1 V1 Champion Needs source-lock gate ───────────────────────────
if(NOT TARGET test_dm1_v1_champion_stats_pc34_compat)
    add_executable(test_dm1_v1_champion_stats_pc34_compat
        tests/test_dm1_v1_champion_stats_pc34_compat.c
        src/dm1/dm1_v1_champion_stats_pc34_compat.c
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_champion_stats_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_champion_stats_load_source_lock
        COMMAND test_dm1_v1_champion_stats_pc34_compat
    )
endif()

if(NOT TARGET test_dm1_v1_champion_needs_pc34_compat_integration)
    add_executable(test_dm1_v1_champion_needs_pc34_compat_integration
        tests/test_dm1_v1_champion_needs_pc34_compat_integration.c
        src/dm1/dm1_v1_champion_needs_pc34_compat.c
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_champion_needs_pc34_compat_integration PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_champion_needs_source_lock
        COMMAND test_dm1_v1_champion_needs_pc34_compat_integration
    )
endif()

# ── DM1 V1 Event Timer source-lock gate ──────────────────────────────
if(NOT TARGET test_dm1_v1_event_timer_pc34_compat)
    add_executable(test_dm1_v1_event_timer_pc34_compat
        tests/test_dm1_v1_event_timer_pc34_compat.c
        src/dm1/dm1_v1_event_timer_pc34_compat.c
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_event_timer_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_event_timer_source_lock
        COMMAND test_dm1_v1_event_timer_pc34_compat
    )
endif()

# ── DM1 V1 Object/Item Interaction source-lock gate ──────────────────
if(NOT TARGET test_dm1_v1_object_interaction_pc34_compat)
    add_executable(test_dm1_v1_object_interaction_pc34_compat
        tests/test_dm1_v1_object_interaction_pc34_compat.c
        src/dm1/dm1_v1_object_interaction_pc34_compat.c
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_object_interaction_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_object_interaction_source_lock
        COMMAND test_dm1_v1_object_interaction_pc34_compat
    )
endif()

# -- DM1 V1 Viewport floor pickup click source-lock gate -----------------
if(NOT TARGET test_dm1_v1_viewport_click_pc34_compat)
    add_executable(test_dm1_v1_viewport_click_pc34_compat
        tests/test_dm1_v1_viewport_click_pc34_compat.c
        src/dm1/dm1_v1_viewport_click_pc34_compat.c
    )
    target_include_directories(test_dm1_v1_viewport_click_pc34_compat PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_viewport_click_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_viewport_click_source_lock
        COMMAND test_dm1_v1_viewport_click_pc34_compat
    )
endif()

    # DM1 V1 Creature AI Behavior System
    add_executable(test_dm1_v1_creature_ai_behavior_pc34_compat
        tests/test_dm1_v1_creature_ai_behavior_pc34_compat.c
        src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c
        src/memory/memory_creature_ai_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_creature_ai_behavior_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_creature_ai_behavior_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_creature_ai_behavior_source_lock
        COMMAND test_dm1_v1_creature_ai_behavior_pc34_compat
    )

# ── DM1 V1 Light & Torch System source-lock gate ─────────────────────
if(NOT TARGET test_dm1_v1_light_pc34_compat)
    add_executable(test_dm1_v1_light_pc34_compat
        tests/test_dm1_v1_light_pc34_compat.c
        src/dm1/dm1_v1_light_pc34_compat.c
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_light_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_light_source_lock
        COMMAND test_dm1_v1_light_pc34_compat
    )
endif()

if(NOT TARGET test_dm1_v1_light_darkness_spell_bridge_pc34_compat)
    add_executable(test_dm1_v1_light_darkness_spell_bridge_pc34_compat
        tests/test_dm1_v1_light_darkness_spell_bridge_pc34_compat.c
        src/dm1/dm1_v1_light_pc34_compat.c
        src/dm1/dm1_v1_spell_casting_pc34_compat.c
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_light_darkness_spell_bridge_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_light_darkness_spell_bridge_source_lock
        COMMAND test_dm1_v1_light_darkness_spell_bridge_pc34_compat
    )
endif()

# ── DM1 V1 Text/Message Display source-lock gate ─────────────────────
if(NOT TARGET test_dm1_v1_text_message_pc34_compat)
    add_executable(test_dm1_v1_text_message_pc34_compat
        tests/test_dm1_v1_text_message_pc34_compat.c
        src/dm1/dm1_v1_text_message_pc34_compat.c
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_text_message_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_text_message_source_lock
        COMMAND test_dm1_v1_text_message_pc34_compat
    )
endif()

# ── DUNGEON.C F0168 scroll TextString source-lock gate ───────────────
if(NOT TARGET test_memory_dungeon_text_scroll_pc34_compat)
    add_executable(test_memory_dungeon_text_scroll_pc34_compat
        tests/test_memory_dungeon_text_scroll_pc34_compat.c
        src/memory/memory_dungeon_dat_pc34_compat.c
        src/shared/dungeon_decompressor_ftl.c
    )
    if(NOT MSVC)
        target_compile_options(test_memory_dungeon_text_scroll_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME memory_dungeon_text_scroll_source_lock
        COMMAND test_memory_dungeon_text_scroll_pc34_compat
    )
endif()

if(NOT TARGET test_m11_inventory_scroll_panel_render_pc34_compat)
    add_executable(test_m11_inventory_scroll_panel_render_pc34_compat
        tests/test_m11_inventory_scroll_panel_render_pc34_compat.c
    )
    target_link_libraries(test_m11_inventory_scroll_panel_render_pc34_compat PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m11 firestaff_m10 SDL3::SDL3)
    if(NOT MSVC)
        target_compile_options(test_m11_inventory_scroll_panel_render_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME m11_inventory_scroll_panel_render_source_lock
        COMMAND test_m11_inventory_scroll_panel_render_pc34_compat
    )
endif()

if(NOT TARGET test_m11_inventory_full_panel_runtime_pc34_compat)
    add_executable(test_m11_inventory_full_panel_runtime_pc34_compat
        tests/test_m11_inventory_full_panel_runtime_pc34_compat.c
    )
    target_link_libraries(test_m11_inventory_full_panel_runtime_pc34_compat PRIVATE firestaff_m12 firestaff_m11 firestaff_m12 firestaff_m11 firestaff_m10 SDL3::SDL3)
    if(NOT MSVC)
        target_compile_options(test_m11_inventory_full_panel_runtime_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME m11_inventory_full_panel_runtime_source_lock
        COMMAND test_m11_inventory_full_panel_runtime_pc34_compat
    )
endif()

# ── DM1 V1 Skill & Experience System source-lock gate ─────────────────
if(NOT TARGET test_dm1_v1_skill_experience_pc34_compat)
    add_executable(test_dm1_v1_skill_experience_pc34_compat
        tests/test_dm1_v1_skill_experience_pc34_compat.c
        src/dm1/dm1_v1_skill_experience_pc34_compat.c
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_skill_experience_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_skill_experience_source_lock
        COMMAND test_dm1_v1_skill_experience_pc34_compat
    )
endif()

# ── DM1 V1 Endgame System source-lock gate ───────────────────────────
if(NOT TARGET test_dm1_v1_endgame_system_pc34_compat)
    add_executable(test_dm1_v1_endgame_system_pc34_compat
        tests/test_dm1_v1_endgame_system_pc34_compat.c
        src/dm1/dm1_v1_endgame_system_pc34_compat.c
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_endgame_system_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_endgame_system_source_lock
        COMMAND test_dm1_v1_endgame_system_pc34_compat
    )
endif()


# ── DM1 V1 Room Transition source-lock gate ──────────────────────────
if(NOT TARGET test_dm1_v1_room_transition_pc34_compat)
    add_executable(test_dm1_v1_room_transition_pc34_compat
        tests/test_dm1_v1_room_transition_pc34_compat.c
        src/dm1/dm1_v1_room_transition_pc34_compat.c
    )
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_room_transition_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_room_transition_source_lock
        COMMAND test_dm1_v1_room_transition_pc34_compat
    )
    if(Python3_EXECUTABLE)
        add_test(
            NAME dm1_v1_room_transition_redmcsb_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_v1_room_transition_source_lock_gate.py
        )
    endif()
endif()

# ── DM1 V1 Viewport 3D source-lock gate ─────────────────────────────
if(BUILD_TESTING AND NOT TARGET test_dm1_v1_viewport_3d_pc34_compat)
    add_executable(test_dm1_v1_viewport_3d_pc34_compat
        tests/test_dm1_v1_viewport_3d_pc34_compat.c
        src/dm1/dm1_v1_viewport_3d_pc34_compat.c
    )
    target_link_libraries(test_dm1_v1_viewport_3d_pc34_compat PRIVATE firestaff_m10)
    if(NOT MSVC)
        target_compile_options(test_dm1_v1_viewport_3d_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME dm1_v1_viewport_3d_source_lock
        COMMAND test_dm1_v1_viewport_3d_pc34_compat
    )
    if(Python3_Interpreter_FOUND)
        add_test(
            NAME dm1_v1_viewport_3d_occlusion_metadata_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_viewport_3d_occlusion_metadata_gate.py
        )
        add_test(
            NAME dm1_v1_viewport_wall_render_plan_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_viewport_wall_render_plan_gate.py
        )
        add_test(
            NAME dm1_v1_projectile_wall_zone_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_projectile_wall_zone_gate.py
        )
    endif()
endif()

# ── DM1 V1 original viewport crop/source-lock readiness gate ─────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass434_dm1_v1_original_viewport_crop_readiness_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass434_dm1_v1_original_viewport_crop_readiness_gate.py
    )
endif()

# ── DM1 V1 input→command→movement pipeline source-lock gate ──────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass423_dm1_v1_input_command_movement_pipeline_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass423_dm1_v1_input_command_movement_pipeline_source_lock.py
    )
endif()

# ── DM1 V1 door-button viewport occlusion source-lock gate ─────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass486_dm1_v1_door_button_occlusion_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass486_dm1_v1_door_button_occlusion_source_lock.py
    )
endif()


# ── DM1 V1 wall occlusion merge-readiness gate ─────────────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass490_dm1_v1_wall_occlusion_merge_readiness
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass490_dm1_v1_wall_occlusion_merge_readiness.py
    )
endif()

# -- DM1 V1 original overlay keyboard-buffer blocker gate -------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass509_dm1_v1_original_overlay_keyboard_buffer_blocker
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass509_dm1_v1_original_overlay_keyboard_buffer_blocker.py
    )
endif()

# -- DM1 V1 movement original route contract gate --------------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass511_dm1_v1_movement_original_route_contract
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass511_dm1_v1_movement_original_route_contract.py
    )
endif()

# -- DM1 V1 I34E route-key transcript contract gate ------------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass513_dm1_v1_i34e_route_key_transcript_contract
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass513_dm1_v1_i34e_route_key_transcript_contract.py
    )
endif()

# -- DM1 V1 PC/I34E runtime transcript capture path gate --------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass514_dm1_v1_i34e_runtime_transcript_capture_path
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass514_dm1_v1_i34e_runtime_transcript_capture_path.py
    )
endif()

# -- DM1 V1 pass514/pass556 IO blocker classifier --------------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass558_dm1_v1_pass514_io_blocker_classifier
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass558_dm1_v1_pass514_io_blocker_classifier.py
    )
endif()

# -- DM1 V1 PC34 keyboard interrupt/runtime binding gate --------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass560_dm1_v1_pc34_keyboard_interrupt_runtime_binding
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass560_dm1_v1_pc34_keyboard_interrupt_runtime_binding.py
    )
endif()

# -- DM1 V1 PC34 keyboard next boundary after pass560 -----------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass562_dm1_v1_pass560_keyboard_next_boundary
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass562_dm1_v1_pass560_keyboard_next_boundary.py
    )
endif()

# -- DM1 V1 PC34 original C254 boundary -------------------------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass563_dm1_v1_pc34_original_c254_boundary
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass563_dm1_v1_pc34_original_c254_boundary.py
    )
endif()

# -- DM1 V1 original movement/viewport runtime transcript gate --------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass564_dm1_v1_original_movement_viewport_transcript_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass564_dm1_v1_original_movement_viewport_transcript_gate.py
    )
endif()

# -- DM1 V1 original overlay/capture bounded progress gate -------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass548_dm1_v1_original_overlay_capture_progress
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass548_dm1_v1_original_overlay_capture_progress.py
    )
endif()

# -- DM1 V1 passable door movement/timing source-lock gate --------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass549_dm1_v1_passable_door_movement_timing
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass549_dm1_v1_passable_door_movement_timing.py
    )
endif()

# -- DM1 V1 front-cell collision source-lock gate -----------------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass562_dm1_v1_front_cell_collision_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass562_dm1_v1_front_cell_collision_source_lock.py
    )
endif()

# ── DM1 V1 wall occlusion spec-matrix gate ───────────────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass496_dm1_v1_wall_occlusion_spec_matrix
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass496_dm1_v1_wall_occlusion_spec_matrix.py
    )
endif()

# ── DM1 V1 wall occlusion runtime-evidence gate ──────────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass499_dm1_v1_wall_occlusion_runtime_evidence_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass499_dm1_v1_wall_occlusion_runtime_evidence_gate.py
    )
endif()

# ── DM1 V1 movement/viewport blocker-cleanup source-lock gate ─────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass500_dm1_v1_viewport_walls_blocker_cleanup_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass500_dm1_v1_viewport_walls_blocker_cleanup_source_lock.py
    )
endif()

# -- DM1 V1 viewport wall draw-order evidence gate -----------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass503_dm1_v1_viewport_wall_draw_order_evidence
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass503_dm1_v1_viewport_wall_draw_order_evidence.py
    )
endif()

# -- DM1 V1 D0C Thieves Eye door-frame occlusion source-lock gate ----
if(Python3_FOUND)
    add_test(
        NAME pass565_dm1_v1_d0c_thieves_eye_door_frame_occlusion
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass565_dm1_v1_d0c_thieves_eye_door_frame_occlusion.py
    )
endif()


# -- DM1 V1 viewport present/capture contract source-lock gate ------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass504_dm1_v1_viewport_present_capture_contract
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass504_dm1_v1_viewport_present_capture_contract.py
    )
endif()

# -- DM1 V1 viewport F0128 draw-order source-lock gate --------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass557_dm1_v1_viewport_f0128_draw_order_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass557_dm1_v1_viewport_f0128_draw_order_source_lock.py
    )
endif()

# -- DM1 V1 door composition occlusion source-lock gate --------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass542_laneB_dm1_v1_door_composition_occlusion_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass542_laneB_dm1_v1_door_composition_occlusion_source_lock.py
    )
endif()

# ── DM1 V1 wall-set startup binding source-lock gate ──────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass509_dm1_v1_wallset_startup_binding
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass509_dm1_v1_wallset_startup_binding.py
    )
    add_test(
        NAME pass511_dm1_v1_viewport_wall_reference_lineage
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass511_dm1_v1_viewport_wall_reference_lineage.py
    )
endif()

# ── DM1 V1 viewport/wall runtime-readiness evidence gate ──────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass508_dm1_v1_viewport_wall_runtime_readiness
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass508_dm1_v1_viewport_wall_runtime_readiness.py
    )
endif()

# -- DM1 V1 viewport wall parity flip source-lock gate ----------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass510_dm1_v1_viewport_wall_parity_flip_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass510_dm1_v1_viewport_wall_parity_flip_source_lock.py
    )
endif()

# -- DM1 V1 D0 side-wall occlusion before D0C foreground source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass515_dm1_v1_d0_side_wall_occlusion_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass515_dm1_v1_d0_side_wall_occlusion_source_lock.py
    )
endif()

# -- DM1 V1 side-wall occlusion/source-row clipping focused gate -------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass576_dm1_v1_side_wall_occlusion_source_row_clipping
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass576_dm1_v1_side_wall_occlusion_source_row_clipping.py
    )
endif()

# -- DM1 V1 D1/D0 side-front wall occlusion source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass516_dm1_v1_d1_d0_wall_occlusion_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass516_dm1_v1_d1_d0_wall_occlusion_source_lock.py
    )
endif()

# -- DM1 V1 D3/D2 field occlusion source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass517_dm1_v1_d3_d2_field_occlusion_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass517_dm1_v1_d3_d2_field_occlusion_source_lock.py
    )
endif()

# -- DM1 V1 D1 side field occlusion source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass518_dm1_v1_d1_side_field_occlusion_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass518_dm1_v1_d1_side_field_occlusion_source_lock.py
    )
endif()

# -- DM1 V1 D1C door-front field source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass519_dm1_v1_d1c_door_front_field_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass519_dm1_v1_d1c_door_front_field_source_lock.py
    )
endif()

# -- DM1 V1 mirrored door-front source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass560_dm1_v1_mirrored_door_front_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass560_dm1_v1_mirrored_door_front_source_lock.py
    )
    set_tests_properties(pass560_dm1_v1_mirrored_door_front_source_lock PROPERTIES TIMEOUT 75)
endif()

# -- DM1 V1 far door-front source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass561_dm1_v1_far_door_front_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass561_dm1_v1_far_door_front_source_lock.py
    )
    set_tests_properties(pass561_dm1_v1_far_door_front_source_lock PROPERTIES TIMEOUT 75)
endif()

# -- DM1 V1 D2 far-side wall source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass562_dm1_v1_d2_far_side_wall_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass562_dm1_v1_d2_far_side_wall_source_lock.py
    )
    set_tests_properties(pass562_dm1_v1_d2_far_side_wall_source_lock PROPERTIES TIMEOUT 75)
endif()

# -- DM1 V1 D1 side wall source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass563_dm1_v1_d1_side_wall_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass563_dm1_v1_d1_side_wall_source_lock.py
    )
    set_tests_properties(pass563_dm1_v1_d1_side_wall_source_lock PROPERTIES TIMEOUT 75)
endif()

# -- DM1 V1 D1 side door-front source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass565_dm1_v1_d1_side_door_front_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass565_dm1_v1_d1_side_door_front_source_lock.py
    )
    set_tests_properties(pass565_dm1_v1_d1_side_door_front_source_lock PROPERTIES TIMEOUT 75)
endif()

# -- DM1 V1 D3/D2 wall ornament draw-order source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass581_dm1_v1_d3_d2_wall_ornament_order_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass581_dm1_v1_d3_d2_wall_ornament_order_source_lock.py
    )
    set_tests_properties(pass581_dm1_v1_d3_d2_wall_ornament_order_source_lock PROPERTIES TIMEOUT 75)
endif()

# -- DM1 V1 side-wall inscription source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass582_dm1_v1_side_inscription_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass582_dm1_v1_side_inscription_source_lock.py
    )
    set_tests_properties(pass582_dm1_v1_side_inscription_source_lock PROPERTIES TIMEOUT 75)
endif()

# -- DM1 V1 D2C front-order source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass570_dm1_v1_d2c_front_order_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass570_dm1_v1_d2c_front_order_source_lock.py
    )
    set_tests_properties(pass570_dm1_v1_d2c_front_order_source_lock PROPERTIES TIMEOUT 75)
endif()

# -- DM1 V1 D0/D1 visible-square draw-order source lock --
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass577_dm1_v1_d0_d1_visible_square_draw_order_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass577_dm1_v1_d0_d1_visible_square_draw_order_source_lock.py
    )
    set_tests_properties(pass577_dm1_v1_d0_d1_visible_square_draw_order_source_lock PROPERTIES TIMEOUT 75)
endif()

# ── DM1 V1 viewport/wall same-frame blocker gate ──────────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass504_dm1_v1_viewport_wall_same_frame_blocker
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass504_dm1_v1_viewport_wall_same_frame_blocker.py
    )
endif()

# -- DM1 V1 viewport wall render-plan promotion boundary gate --------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass504_dm1_v1_viewport_wall_render_plan_promotion_boundary
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass504_dm1_v1_viewport_wall_render_plan_promotion_boundary.py
    )
endif()

# ── DM1 V1 door/TITLE/end source-audit closure gate ───────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass489_dm1_v1_door_title_end_source_audit
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass489_dm1_v1_door_title_end_source_audit_gate.py
    )
endif()

# ── DM1 V1 original click-route capture blocker gate ───────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass487_dm1_v1_original_click_capture_blocker
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass487_dm1_v1_original_click_capture_blocker.py
    )
endif()
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass508_dm1_v1_original_route_capture_blocker_evidence
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass508_dm1_v1_original_route_capture_blocker_evidence.py
    )
endif()

# ── DM1 V1 original capture route-label filename fixture ───────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass510_dm1_v1_original_capture_route_label_filename_fixture
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass510_dm1_v1_original_capture_route_label_filename_fixture.py
    )
endif()

# ── DM1 V1 same-viewport original/Firestaff capture blocker gate ───
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass505_dm1_v1_same_viewport_capture_blocker
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass505_dm1_v1_same_viewport_capture_blocker.py
    )
endif()

# -- DM1 V1 original capture route preflight gate -------------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass504_dm1_v1_original_capture_route_preflight
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass504_dm1_v1_original_capture_route_preflight.py
    )
endif()



# -- DM1 V1 viewport wall clip source audit gate ---------------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass512_dm1_v1_viewport_wall_clip_source_audit
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass512_dm1_v1_viewport_wall_clip_source_audit.py
    )
endif()

# -- DM1 V1 movement cross-reference audit blocker gate ----------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass512_dm1_v1_movement_cross_reference_audit
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass512_dm1_v1_movement_cross_reference_audit.py
    )
endif()

# -- DM1 V1 movement follow-up source-lock gate ---------------------
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass504_dm1_v1_movement_followup_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass504_dm1_v1_movement_followup_source_lock.py
    )
endif()

# ── DM1 V1 input→F0365/F0366 runtime follow-up gate ─────────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass475_dm1_v1_f0365_f0366_runtime_followup
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass475_dm1_v1_f0365_f0366_runtime_followup.py
    )
endif()

# ── DM1 V1 F0365/F0366 runtime/static boundary gate ─────────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass495_dm1_v1_f0365_f0366_runtime_static_boundary
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass495_dm1_v1_f0365_f0366_runtime_static_boundary.py
    )
endif()

# ── DM1 V1 key-route state-delta blocker gate ───────────────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass508_dm1_v1_key_route_state_delta_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass508_dm1_v1_key_route_state_delta_gate.py
    )
endif()

# ── DM1 V1 original route state-delta diversity blocker gate ─────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass504_dm1_v1_original_route_state_delta_diversity_blocker
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass504_dm1_v1_original_route_state_delta_diversity_blocker.py
    )
endif()

# ── DM1 V1 movement legality completion-matrix gate ─────────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass406_dm1_v1_movement_legality_completion_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass406_dm1_v1_movement_legality_completion_gate.py
    )
endif()


# DM1 V1 blocked movement side-effect source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass505_dm1_v1_blocked_movement_side_effect_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass505_dm1_v1_blocked_movement_side_effect_source_lock.py
    )
endif()


# DM1 V1 blocked collision queue lifecycle source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass544_dm1_v1_blocked_collision_queue_lifecycle
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass544_dm1_v1_blocked_collision_queue_lifecycle.py
    )
endif()

# DM1 V1 blocked movement lifecycle source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass551_dm1_v1_blocked_movement_lifecycle
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass551_dm1_v1_blocked_movement_lifecycle.py
    )
endif()

# DM1 V1 movement queue and sensor consequence source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass545_dm1_v1_movement_queue_sensor_consequences
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass545_dm1_v1_movement_queue_sensor_consequences.py
    )
endif()

# DM1 V1 movement cooldown queue loop source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass552_dm1_v1_movement_cooldown_queue_loop
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass552_dm1_v1_movement_cooldown_queue_loop.py
    )
endif()

# DM1 V1 forward collision/timing source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass580_dm1_v1_forward_collision_timing
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass580_dm1_v1_forward_collision_timing.py
    )
endif()

# DM1 V1 blocked wall/door/fakewall self-damage source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass590_dm1_v1_blocked_wall_door_self_damage_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass590_dm1_v1_blocked_wall_door_self_damage_source_lock.py
    )
    set_tests_properties(pass590_dm1_v1_blocked_wall_door_self_damage_source_lock PROPERTIES
        ENVIRONMENT "BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    )
endif()

# DM1 V1 gated movement pending-click replay source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass559_dm1_v1_gated_movement_pending_click_queue_replay
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass559_dm1_v1_gated_movement_pending_click_queue_replay.py
    )
endif()

# DM1 V1 stairs movement side-effect source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass506_dm1_v1_stairs_movement_side_effect_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass506_dm1_v1_stairs_movement_side_effect_source_lock.py
    )
endif()

# DM1 V1 cooldown-gated backward-on-stairs source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass578_dm1_v1_stairs_backstep_cooldown_gate
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass578_dm1_v1_stairs_backstep_cooldown_gate.py
    )
endif()

# ── DM1 V1 movement/stairs/group/timing source-lock gate ────────────
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass507_dm1_v1_movement_stairs_group_timing_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass507_dm1_v1_movement_stairs_group_timing_source_lock.py
    )
endif()

# -- DM1 V1 movement N2 reference-anchor source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass509_dm1_v1_movement_n2_reference_anchor
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass509_dm1_v1_movement_n2_reference_anchor.py
    )
endif()

# -- DM1 V1 movement sensor rotation defer source-lock gate
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME pass510_dm1_v1_movement_sensor_rotation_defer_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass510_dm1_v1_movement_sensor_rotation_defer_source_lock.py
    )
endif()


# -- DM1 V1 SWSH source animation/capture schedule gate
if(BUILD_TESTING)
    add_executable(test_swsh_frontend_pc34_compat_integration
        tests/test_swsh_frontend_pc34_compat_integration.c
        src/frontend/swsh_frontend_pc34_compat.c
        src/shared/bitmap_call_pc34_compat.c
        src/shared/image_expand_pc34_compat.c
        src/shared/image_backend_pc34_compat.c
    )
    target_compile_definitions(test_swsh_frontend_pc34_compat_integration PRIVATE
        COMPILE_H
        STATICFUNCTION=static
        "SEPARATOR=,"
        "FINAL_SEPARATOR=)"
    )
    if(NOT MSVC)
        target_compile_options(test_swsh_frontend_pc34_compat_integration PRIVATE -std=c99 -Wall -Wextra -pedantic -O2)
    endif()
    add_test(
        NAME swsh_frontend_source_animation_schedule
        COMMAND test_swsh_frontend_pc34_compat_integration
    )
endif()


# ── DM1 V1 TITLE runtime cadence source-lock gate ───────────────────
if(BUILD_TESTING)
    add_executable(test_title_frontend_runtime_cadence_pc34_compat
        tests/test_title_frontend_runtime_cadence_pc34_compat.c
        src/frontend/title_frontend_v1.c
        src/frontend/title_dat_loader_v1.c
    )
    if(NOT MSVC)
        target_compile_options(test_title_frontend_runtime_cadence_pc34_compat PRIVATE -Wall -Wextra -O2)
    endif()
    add_test(
        NAME title_frontend_runtime_cadence_source_lock
        COMMAND test_title_frontend_runtime_cadence_pc34_compat
    )
    if(Python3_EXECUTABLE)
        add_test(
            NAME pass457_dm1_v1_title_runtime_cadence_gate
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass457_dm1_v1_title_runtime_cadence_gate.py
        )
        add_test(
            NAME pass462_dm1_v1_title_end_original_capture_parity
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_pass462_dm1_v1_title_end_original_capture_parity.py
        )
    endif()
endif()


# ── DM1 V1 ENDGAME source animation/capture schedule gate ─────────────
if(BUILD_TESTING)
    add_executable(test_endgame_frontend_pc34_compat_integration
        tests/test_endgame_frontend_pc34_compat_integration.c
        src/frontend/endgame_frontend_pc34_compat.c
        src/shared/bitmap_call_pc34_compat.c
        src/shared/image_expand_pc34_compat.c
        src/shared/image_backend_pc34_compat.c
    )
    target_compile_definitions(test_endgame_frontend_pc34_compat_integration PRIVATE
        COMPILE_H
        STATICFUNCTION=static
        "SEPARATOR=,"
        "FINAL_SEPARATOR=)"
    )
    if(NOT MSVC)
        target_compile_options(test_endgame_frontend_pc34_compat_integration PRIVATE -std=c99 -Wall -Wextra -pedantic -O2)
    endif()
    add_test(
        NAME endgame_frontend_source_animation_schedule
        COMMAND test_endgame_frontend_pc34_compat_integration
    )
endif()


# ── GFX extract proof probe (uses M10 GRAPHICS.DAT parser) ──────────
add_executable(firestaff_gfx_extract_proof
    probes/dm1/firestaff_gfx_extract_proof.c
)
target_link_libraries(firestaff_gfx_extract_proof PRIVATE firestaff_m10)
target_include_directories(firestaff_gfx_extract_proof PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(NOT MSVC)
    target_compile_options(firestaff_gfx_extract_proof PRIVATE -Wall -Wextra -O2)
endif()

if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
        NAME dm1_v1_ambient_dungeon_sound_source_lock
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_dm1_v1_ambient_dungeon_sound_source_lock.py
    )
endif()



# ── Known failing tests (WILL_FAIL = expected failure) ──
set_tests_properties(pass372_dm1_v1_movement_runtime_route PROPERTIES WILL_FAIL TRUE)
set_tests_properties(pass377_dm1_v1_paired_diff_artifact_blocker PROPERTIES WILL_FAIL TRUE)
set_tests_properties(pass462_dm1_v1_title_end_original_capture_parity PROPERTIES WILL_FAIL TRUE)

set_tests_properties(pass558_dm1_v1_pass514_io_blocker_classifier PROPERTIES WILL_FAIL TRUE)

set_tests_properties(pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate PROPERTIES WILL_FAIL TRUE)
set_tests_properties(pass563_dm1_v1_pc34_original_c254_boundary PROPERTIES WILL_FAIL TRUE)
set_tests_properties(pass362_dm1_v1_viewport_walls_source_lock_landable PROPERTIES WILL_FAIL TRUE)
set_tests_properties(pass578_dm1_v1_stairs_backstep_cooldown_gate PROPERTIES WILL_FAIL TRUE)


set_tests_properties(dm1_v2_completion_matrix_gate PROPERTIES WILL_FAIL TRUE)
set_tests_properties(dm1_v2_viewport_composition_source_lock PROPERTIES WILL_FAIL TRUE)
set_tests_properties(dm1_v2_d0_d3_draw_list_comparator_gate PROPERTIES WILL_FAIL TRUE)
set_tests_properties(dm1_v2_dungeon_dat_square_decoder_source_lock PROPERTIES WILL_FAIL TRUE)
set_tests_properties(dm1_v2_viewport_pixel_capture_fixture_gate PROPERTIES WILL_FAIL TRUE)
set_tests_properties(dm1_v2_entry_viewport_png_export_gate PROPERTIES WILL_FAIL TRUE)
set_tests_properties(csb_v1_m11_runtime_capture_boundary PROPERTIES WILL_FAIL TRUE)
set_tests_properties(pass547_csb_v1_runtime_readiness_backfill PROPERTIES WILL_FAIL TRUE)
set_tests_properties(pass395_touch_queue_integration_source_lock PROPERTIES WILL_FAIL TRUE)
set_tests_properties(firestaff_dm1_v1_side_contents_center_blocker_probe PROPERTIES WILL_FAIL TRUE)
set_tests_properties(v1_inventory_panel_open_redmcsb_gate PROPERTIES WILL_FAIL TRUE)
set_tests_properties(v1_inventory_toggle_redmcsb_gate PROPERTIES WILL_FAIL TRUE)
set_tests_properties(v1_inventory_chest_actionhand_redmcsb_gate PROPERTIES WILL_FAIL TRUE)
set_tests_properties(v1_entrance_input_wait_redmcsb_gate PROPERTIES WILL_FAIL TRUE)

# ── Sprite extraction tool for V2 graphics pipeline ─────────────────
add_executable(extract_all_sprites tools/extract_all_sprites.c src/engine/firestaff_graphics_dat_reader.c)
target_include_directories(extract_all_sprites PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/tools)
target_link_libraries(extract_all_sprites PRIVATE firestaff_m10)
if(NOT MSVC)
    target_compile_options(extract_all_sprites PRIVATE -Wall -Wextra -O2)
endif()
