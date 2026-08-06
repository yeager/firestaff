#!/usr/bin/env python3
"""Keep unproven Nexus presentation code out of the production library."""

from __future__ import annotations

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
text = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")


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
    r"nexus_v1_saturn_font\\.c$",
    r"nexus_v1_warning_dgt2_m11_presentation\\.c$",
    r"nexus_v1_mns\\.c$",
    r"nexus_v1_spell_effects\\.c$",
):
    if f'EXCLUDE REGEX "{exclusion}"' not in body:
        fail(f"missing production exclusion: {exclusion}")

runtime_match = re.search(
    r"set\(NEXUS_M11_RUNTIME_SOURCES(?P<body>.*?)\n\)", text, re.DOTALL
)
if not runtime_match:
    fail("could not locate NEXUS_M11_RUNTIME_SOURCES")
runtime_body = runtime_match.group("body")
if "src/nexus/nexus_v2_hud_runtime_noop.c" not in runtime_body:
    fail("production HUD runtime is not the no-op implementation")
for forbidden in (
    "src/nexus/nexus_v2_hud_runtime.c",
    "src/nexus/nexus_v2_hud_overlay.c",
    "src/nexus/nexus_v2_render_pipeline.c",
    "src/nexus/nexus_v2_particles.c",
    "src/nexus/nexus_v2_atmosphere.c",
):
    if forbidden in runtime_body:
        fail(f"procedural presentation source entered runtime list: {forbidden}")

print(
    "nexus_production_source_boundary: PASS "
    "(retail presentation remains capture-gated; fixture/probe renderers excluded)"
)
