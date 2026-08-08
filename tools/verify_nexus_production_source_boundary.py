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
if "#ifndef FIRESTAFF_NEXUS_PRODUCTION" in saturn_font:
    fail("Saturn font source still has the production glyph-renderer guard — should be removed")
if "nexus_v1_font_load_from_s2d" not in saturn_font:
    fail("Saturn font source missing nexus_v1_font_load_from_s2d")

runtime_match = re.search(
    r"set\(NEXUS_M11_RUNTIME_SOURCES(?P<body>.*?)\n\)", text, re.DOTALL
)
if not runtime_match:
    fail("could not locate NEXUS_M11_RUNTIME_SOURCES")
runtime_body = runtime_match.group("body")
if "src/nexus/nexus_v1_rasterizer.c" not in runtime_body:
    fail("production rasterizer is not the real implementation")
if "src/nexus/nexus_v1_rasterizer_runtime_noop.c" in runtime_body:
    fail("production rasterizer is still the no-op — should use real rasterizer")
if "src/nexus/nexus_v1_saturn_font_runtime_noop.c" in runtime_body:
    fail("production font runtime is still the no-op — should use real saturn_font.c")
for required_v2 in (
    "src/nexus/nexus_v2_hud_runtime.c",
    "src/nexus/nexus_v2_hud_overlay.c",
    "src/nexus/nexus_v2_lighting_runtime.c",
    "src/nexus/nexus_v2_lighting.c",
    "src/nexus/nexus_v2_smooth_movement_runtime.c",
    "src/nexus/nexus_v2_smooth_movement.c",
    "src/nexus/nexus_v2_touch_runtime.c",
    "src/nexus/nexus_v2_touch_controller_affordance.c",
):
    if required_v2 not in runtime_body:
        fail(f"V2 phase-gated runtime missing from production: {required_v2}")
for forbidden in (
    "src/nexus/nexus_v2_render_pipeline.c",
    "src/nexus/nexus_v2_particles.c",
    "src/nexus/nexus_v2_atmosphere.c",
):
    if forbidden in runtime_body:
        fail(f"procedural presentation source entered runtime list: {forbidden}")

if '"NEXUS ART"' in launcher_renderer:
    fail("procedural NEXUS ART card label remains in the launcher")
if "else if (slotIdx == 3)" not in launcher_renderer:
    fail("Nexus launcher card does not have an explicit capture-locked branch")
if '"CAPTURE LOCKED"' not in launcher_renderer:
    fail("Nexus launcher card does not expose its capture-locked status")

print(
    "nexus_production_source_boundary: PASS "
    "(retail presentation remains capture-gated; fixture/probe renderers excluded)"
)
