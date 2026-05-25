# Nexus V1 Known Regressions

## Overview

Nexus V1 implementation is not complete enough to have regression tests. What exists is scaffolding only (20 source files, no tests, no linked executable).

A "regression" in this context means: behavior that has regressed relative to the original Sega Saturn game, or relative to the stated Nexus plan.

---

## Phase Status: All NOT DONE

Every claimed feature in TODO.md for Nexus V1 has a regression relative to plan because nothing is wired to an executable yet.

---

## Known Implementation Gaps (Not Yet Regressions, But Will Be)

### 1. Nexus static library not linked into any binary
**Severity:** CRITICAL (blocking)
**Details:** libfirestaff_nexus.a compiles but nothing links against it. No firestaff --profile nexus exists.

### 2. No disc image or fixtures
**Severity:** CRITICAL (blocking)
**Details:** No Sega Saturn disc image is present in the repository. No tools/extract_nexus_iso.py has been tested against real data. Phase 0 (provenance gate) is not satisfied.

### 3. No game loop integration
**Severity:** HIGH
**Details:** nexus_v1_engine.c has a game loop but it is not integrated into firestaff_game_loop.c. The DM1 game loop is the active loop. Nexus V1 loop is dead code.

### 4. No asset loading integration
**Severity:** HIGH
**Details:** nexus_v1_dungeon.c has load functions but no DM1 dungeon loader integration. nexus_v1_iso_reader.c has ISO reading code but no production path.

### 5. No VDP1/VDP2 texture format implementation
**Severity:** HIGH
**Details:** nexus_v1_rasterizer.c exists but the actual VDP1 format parsing (4bpp/8bpp paletted, SH2 big-endian) is not implemented. Textures are not loaded.

### 6. No DMDF model format implementation
**Severity:** HIGH
**Details:** nexus_v1_dmdf_model.c exists but DMDF format is not documented or parsed. 3D models are not loaded.

### 7. No text encoding implementation
**Severity:** MEDIUM
**Details:** nexus_v1_text.c and nexus_v1_saturn_font.c exist but Shift-JIS decoding is not implemented. Japanese text will not render.

### 8. No CD audio integration
**Severity:** MEDIUM
**Details:** nexus_v1_game.c has nexus_v1_cd_track_for_level stub returning 2+(level/2), but no SDL_mixer integration. Tracks 2-9 Red Book Audio not played.

### 9. No save/load implementation
**Severity:** MEDIUM
**Details:** No save/load code in Nexus V1. Saturn save format unknown.

### 10. No V2 renderer scaffold wired
**Severity:** MEDIUM
**Details:** nexus_v2_*.c files (lighting, atmosphere, particles, render_pipeline, upscaler) exist but are not connected to nexus_v1_engine.c game loop.

---

## DM1 Parity Regressions (If Nexus Mechanics Deviate)

These are potential regressions if Nexus mechanics ever differ from DM1. Currently N/A since no Nexus mechanics run.

- **Combat**: If nexus_v1_combat.c uses different damage formulas than DM1 combat, combat is regressed
- **Movement**: If nexus_v1_game.c party movement differs from DM1, movement is regressed
- **Spells**: If nexus_v1_magic.c spell effects differ from DM1, magic is regressed
- **Champion stats**: If nexus_v1_champions.c stat changes differ from DM1, champion system is regressed
- **Creature AI**: If nexus_v1_creatures.c behavior differs from DM1, creature behavior is regressed

---

## Known BUG0 Issues (Same as DM1, Not Fixed in Nexus)

These bugs exist in DM1 and carry into Nexus (since Nexus is a DM1 remake):

| Bug | Severity | Status in Nexus |
|-----|----------|-----------------|
| BUG0_02 Timeline 24-bit overflow | CRITICAL | NOT FIXED -- game hangs after ~850 hours |
| BUG0_03 VBlank timing glitch | MINOR | NOT FIXED |
| BUG0_04 Creature colors | LOW | NOT FIXED |
| BUG0_05 Portrait sensor z-order | LOW | NOT FIXED |
| BUG0_06 Projectile blit left edge | LOW | NOT FIXED |
| BUG0_07 Explosion blit left edge | LOW | NOT FIXED |
| BUG0_64 Floor ornaments over pits | LOW | MAYBE PRESENT |
| BUG0_83 Thieves Eye hole animation | MEDIUM | NOT FIXED |
| BUG0_86 Champion portrait graphics | MEDIUM | NOT FIXED |

These are not Nexus-specific regressions -- they are inherited from the DM1 engine that Nexus re-implements.

---

## Rendering Regressions (When V1 Renderer Is Active)

When the Nexus V1 3D renderer is complete, these regressions are possible:

- **Z-fighting**: If depth buffer not correctly managed, polygons flicker
- **Draw order**: Wrong order (creature before wall, UI before floor) -- visual regression
- **Texture seam**: Cracks between polygons due to precision errors
- **Palette mismatch**: Colors differ from original Saturn VDP2 palette
- **EPX artifacts**: Upscaling artifacts at 640x448 (when V2.1 is wired)
- **Frame rate**: Below 60 FPS, game feels sluggish
- **Aspect ratio**: 4:3 vs 16:9 handling, pillarbox/letterbox issues

---

## Summary

There are no regressions **yet** because Nexus V1 is not running. The implementation is scaffolding.

The blocking regressions are:
1. No disc image (cannot test anything)
2. No linked executable (cannot run anything)
3. No integration into game loop (cannot play anything)

When Phase 1 (runtime profile split) is done, the first regression test should be: --profile nexus starts without loading DM1 assets. If it loads DM1 assets, that IS a regression.