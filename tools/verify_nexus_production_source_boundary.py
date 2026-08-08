#!/usr/bin/env python3
"""Keep unproven Nexus presentation code out of the production library."""

from __future__ import annotations

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
text = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
launcher_renderer = (ROOT / "src/ui/menu_startup_render_modern_m12.c").read_text(
    encoding="utf-8"
)
launcher = (ROOT / "src/nexus/nexus_v1_launcher.c").read_text(encoding="utf-8")
m11_game_view = (ROOT / "src/engine/m11_game_view.c").read_text(
    encoding="utf-8"
)


def fail(message: str) -> None:
    print(f"nexus_production_source_boundary: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


match = re.search(
    r"file\(GLOB NEXUS_SOURCES.*?\n(?P<body>.*?)(?=\nset\(NEXUS_M11_RUNTIME_SOURCES)",
    text,
    re.DOTALL,
)
if not match:
    fail("could not locate NEXUS_SOURCES boundary")
body = match.group("body")

for exclusion in (
    r"nexus_v1_text\\.c$",
    r"nexus_v1_s2d_text_layout\\.c$",
    r"nexus_v1_s2d_glyph_decode\\.c$",
    r"nexus_v1_screen_text\\.c$",
    r"nexus_v1_rasterizer\\.c$",
    r"nexus_v1_item_ibs\\.c$",
    r"nexus_v1_title_cg\\.c$",
    r"nexus_v1_warning_dgt2_m11_presentation\\.c$",
    r"nexus_v1_mns\\.c$",
):
    if f'EXCLUDE REGEX "{exclusion}"' not in body:
        fail(f"missing production exclusion: {exclusion}")

# The SCR section-table parser is source-format admission code and must remain
# available to the production archive.  The same translation unit also holds
# the unproven flat glyph/framebuffer path; FIRESTAFF_NEXUS_PRODUCTION must
# compile that path out rather than excluding the parser or exporting a host
# renderer.
if 'target_compile_definitions(firestaff_nexus PRIVATE FIRESTAFF_NEXUS_PRODUCTION=1)' not in text:
    fail("production Nexus library does not define FIRESTAFF_NEXUS_PRODUCTION")
saturn_font = (ROOT / "src/nexus/nexus_v1_saturn_font.c").read_text(
    encoding="utf-8"
)
if "#ifndef FIRESTAFF_NEXUS_PRODUCTION" not in saturn_font:
    fail("Saturn font source lacks the production glyph-renderer guard")
if "#endif /* !FIRESTAFF_NEXUS_PRODUCTION */" not in saturn_font:
    fail("Saturn font source lacks the production glyph-renderer guard terminator")
if "nexus_v1_font_load_from_s2d" not in saturn_font:
    fail("Saturn font source missing nexus_v1_font_load_from_s2d")

runtime_match = re.search(
    r"set\(NEXUS_M11_RUNTIME_SOURCES(?P<body>.*?)\n\)", text, re.DOTALL
)
if not runtime_match:
    fail("could not locate NEXUS_M11_RUNTIME_SOURCES")
runtime_body = runtime_match.group("body")
for required_noop in (
    "src/nexus/nexus_v1_rasterizer_runtime_noop.c",
    "src/nexus/nexus_v1_saturn_font_runtime_noop.c",
    "src/nexus/nexus_v2_hud_runtime_noop.c",
    "src/nexus/nexus_v2_lighting_runtime_noop.c",
    "src/nexus/nexus_v2_smooth_movement_runtime_noop.c",
    "src/nexus/nexus_v2_touch_runtime_noop.c",
):
    if required_noop not in runtime_body:
        fail(f"capture-gated production adapter missing: {required_noop}")
for forbidden in (
    "src/nexus/nexus_v1_rasterizer.c",
    "src/nexus/nexus_v2_hud_runtime.c",
    "src/nexus/nexus_v2_hud_overlay.c",
    "src/nexus/nexus_v2_lighting_runtime.c",
    "src/nexus/nexus_v2_lighting.c",
    "src/nexus/nexus_v2_smooth_movement_runtime.c",
    "src/nexus/nexus_v2_smooth_movement.c",
    "src/nexus/nexus_v2_touch_runtime.c",
    "src/nexus/nexus_v2_touch_controller_affordance.c",
    "src/nexus/nexus_v2_render_pipeline.c",
    "src/nexus/nexus_v2_particles.c",
    "src/nexus/nexus_v2_atmosphere.c",
):
    if forbidden in runtime_body:
        fail(f"unbound presentation source entered production runtime list: {forbidden}")

if '"NEXUS ART"' in launcher_renderer:
    fail("procedural NEXUS ART card label remains in the launcher")
if "else if (slotIdx == 3)" not in launcher_renderer:
    fail("Nexus launcher card does not have an explicit capture-locked branch")
if '"CAPTURE LOCKED"' not in launcher_renderer:
    fail("Nexus launcher card does not expose its capture-locked status")

# The source loaders may retain authenticated retail bytes for later capture
# analysis, but the production startup handoff must not turn planner metadata
# into host pixels. Keep this check next to the CMake boundary: a future merge
# can otherwise restore a visually plausible menu without adding a Saturn
# VDP1/VDP2 consumer receipt.
for required in (
    "if (out_commands[read_index].kind != NEXUS_V1_STARTUP_DRAW_TEXT)",
    "out_receipt->hud_ready = 0;",
    "status = \"blocked-dgn-capture-required\"",
):
    if required not in launcher:
        fail(f"startup runtime handoff lost capture gate: {required}")

# These executors deliberately consume command metadata without writing the
# M11 framebuffer. The boot-title executor is the sole exception, and it must
# retain the complete authenticated transition receipt predicate.
noop_executors = (
    "m11_nexus_startup_exec_title_background",
    "m11_nexus_startup_exec_warning_background",
    "m11_nexus_startup_exec_fill_rect",
    "m11_nexus_startup_exec_outline_rect",
    "m11_nexus_startup_exec_text",
    "m11_nexus_startup_exec_portrait",
)
for function_name in noop_executors:
    function = re.search(
        rf"static void {function_name}\(.*?\n\}}",
        m11_game_view,
        re.DOTALL,
    )
    if not function:
        fail(f"M11 Nexus startup executor missing: {function_name}")
    body_text = function.group(0)
    if "(void)command;" not in body_text:
        fail(f"M11 startup executor is no longer explicit no-draw: {function_name}")

if "m11_nexus_startup_title_receipt_ready(context, command)" not in m11_game_view:
    fail("boot-title executor lost its authenticated transition-capture gate")

print(
    "nexus_production_source_boundary: PASS "
    "(retail presentation remains capture-gated; fixture/probe renderers excluded)"
)
