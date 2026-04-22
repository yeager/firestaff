cmake_minimum_required(VERSION 3.20)
project(Firestaff VERSION 0.11.0 LANGUAGES C)

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

# ── SDL3 discovery ───────────────────────────────────────────────────
# Try CMake config first, then fall back to pkg-config.
find_package(SDL3 QUIET CONFIG)
if(NOT SDL3_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
    # Create an alias so the rest of the file can use SDL3::SDL3.
    if(NOT TARGET SDL3::SDL3)
        add_library(SDL3::SDL3 ALIAS PkgConfig::SDL3)
    endif()
endif()

# ── M10 static library (frozen Turbo-C compat sources) ──────────────
file(GLOB M10_SOURCES CONFIGURE_DEPENDS "*_pc34_compat.c")

# Exclude test_* files from the library (they have their own main()).
list(FILTER M10_SOURCES EXCLUDE REGEX "test_.*_pc34_compat\\.c$")

add_library(firestaff_m10 STATIC ${M10_SOURCES})
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

# ── M11 static library ──────────────────────────────────────────────
set(M11_SOURCES
    main_loop_m11.c
    m11_game_view.c
    render_sdl_m11.c
    audio_sdl_m11.c
    asset_loader_m11.c
    font_m11.c
    fs_portable_compat.c
)

add_library(firestaff_m11 STATIC ${M11_SOURCES})
target_include_directories(firestaff_m11 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(firestaff_m11 PUBLIC firestaff_m10 SDL3::SDL3 m)
if(NOT MSVC)
    target_compile_options(firestaff_m11 PRIVATE -Wall -Wextra -O2)
endif()

# ── M12 static library ──────────────────────────────────────────────
set(M12_SOURCES
    menu_startup_m12.c
    menu_startup_render_modern_m12.c
    menu_hit_m12.c
    config_m12.c
    asset_status_m12.c
    card_art_m12.c
    branding_logo_m12.c
    creature_art_m12.c
)

add_library(firestaff_m12 STATIC ${M12_SOURCES})
target_include_directories(firestaff_m12 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(firestaff_m12 PUBLIC firestaff_m11)
if(NOT MSVC)
    target_compile_options(firestaff_m12 PRIVATE -Wall -Wextra -O2)
endif()

# ── Main executable ──────────────────────────────────────────────────
add_executable(firestaff firestaff_main_m11.c)
target_link_libraries(firestaff PRIVATE firestaff_m12 firestaff_m11 firestaff_m10 SDL3::SDL3)
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
target_link_libraries(firestaff_m11_game_view_probe PRIVATE firestaff_m12 firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_game_view_probe PRIVATE -Wall -Wextra -O2)
endif()

# M11 Audio probe (fallback-safe audio marker pipeline)
add_executable(firestaff_m11_audio_probe probes/m11/firestaff_m11_audio_probe.c)
target_link_libraries(firestaff_m11_audio_probe PRIVATE firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_audio_probe PRIVATE -Wall -Wextra -O2)
endif()

# M11 FS Portable probe (cross-platform filesystem helpers)
add_executable(firestaff_m11_fs_portable_probe probes/m11/firestaff_m11_fs_portable_probe.c)
target_link_libraries(firestaff_m11_fs_portable_probe PRIVATE firestaff_m11 firestaff_m10)
if(NOT MSVC)
    target_compile_options(firestaff_m11_fs_portable_probe PRIVATE -Wall -Wextra -O2)
endif()

# M12 Startup Menu probe
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/probes/m12/firestaff_m12_startup_menu_probe.c")
    add_executable(firestaff_m12_startup_menu_probe probes/m12/firestaff_m12_startup_menu_probe.c)
    target_link_libraries(firestaff_m12_startup_menu_probe PRIVATE firestaff_m12 firestaff_m11 firestaff_m10 SDL3::SDL3)
    if(NOT MSVC)
        target_compile_options(firestaff_m12_startup_menu_probe PRIVATE -Wall -Wextra -O2)
    endif()
endif()

# ── CTest integration ────────────────────────────────────────────────
include(CTest)
if(BUILD_TESTING)
    # Phase A probe (headless, no game data needed)
    add_test(
        NAME m11_phase_a
        COMMAND firestaff_m11_phase_a_probe
    )
    set_tests_properties(m11_phase_a PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=dummy"
    )

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

    # Launcher smoke (needs game data + built firestaff binary)
    add_test(
        NAME m11_launcher_smoke
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_firestaff_m11_launcher_smoke.sh
    )
    set_tests_properties(m11_launcher_smoke PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;FIRESTAFF_DATA=$ENV{HOME}/.firestaff/data"
    )
endif()

# ── Install rules ────────────────────────────────────────────────────
install(TARGETS firestaff RUNTIME DESTINATION bin)

