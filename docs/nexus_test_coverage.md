# Nexus V1 Test Coverage Report

## Summary

Nexus V1 implementation: 20 source files in src/nexus/, static library libfirestaff_nexus.a.
Nexus V1 tests: **0 tests**. No CTest entries. No test binaries. No test fixtures.

The test suite has 387 tests but none are for Nexus. All existing tests are DM1 V1/V2 and memory subsystem tests.

---

## Source Files and Test Coverage

| Source File | Tests | Coverage |
|-------------|-------|----------|
| nexus_v1_engine.c | 0 | NONE |
| nexus_v1_game.c | 0 | NONE |
| nexus_v1_dungeon.c | 0 | NONE |
| nexus_v1_champions.c | 0 | NONE |
| nexus_v1_combat.c | 0 | NONE |
| nexus_v1_creatures.c | 0 | NONE |
| nexus_v1_magic.c | 0 | NONE |
| nexus_v1_iso_reader.c | 0 | NONE |
| nexus_v1_rasterizer.c | 0 | NONE |
| nexus_v1_viewport.c | 0 | NONE |
| nexus_v1_text.c | 0 | NONE |
| nexus_v1_saturn_font.c | 0 | NONE |
| nexus_v1_math3d.c | 0 | NONE |
| nexus_v1_dmdf_model.c | 0 | NONE |
| nexus_v2_config.c | 0 | NONE |
| nexus_v2_lighting.c | 0 | NONE |
| nexus_v2_atmosphere.c | 0 | NONE |
| nexus_v2_particles.c | 0 | NONE |
| nexus_v2_render_pipeline.c | 0 | NONE |
| nexus_v2_upscaler.c | 0 | NONE |

**20 source files, 0 tests. 0% test coverage.**

---

## What's Well-Tested (DM1, not Nexus)

The well-tested areas are all DM1 V1, not Nexus V1. This is relevant because DM1 parity serves as a proxy for Nexus mechanics:

**Well-tested (DM1 V1):**
- Movement/collision: 7 tests (movement_core, movement_command, movement_timing, etc.)
- Viewport rendering: 5 tests (viewport_3d, viewport_click, viewport_hand_overlay, etc.)
- Inventory system: 3 tests (equip_slots, backpack_chest, consumables)
- Champion system: 4 tests (stats, panel_hud, needs, resurrection)
- Combat: 5 tests (combat, creature_ai, creature_render, creature_sound, projectile)
- Dungeon loading: strong memory subsystem tests (graphics_dat tests)
- Save/load: 2 tests
- Room transitions: covered
- Spells: 2 tests

**Not tested (DM1 V1, gaps that apply to Nexus by proxy):**
- BUG0_02 timeline 24-bit overflow (CRITICAL -- no test for 850+ hour sessions)
- BUG0_03 VBlank timing
- BUG0_04 creature colors
- BUG0_05 portrait sensor z-order
- BUG0_06/07 projectile/explosion blit on left viewport edge
- BUG0_83 Thieves Eye hole animation
- BUG0_86 champion portrait memory limitation on custom dungeons

---

## Coverage Gaps for Nexus V1

### Critical Gaps

1. **No ISO parsing tests** -- nexus_v1_iso_reader.c has no tests. The tools/extract_nexus_iso.py script exists but is not tested against a real disc image.

2. **No data format tests** -- dungeon loading (nexus_v1_dungeon.c), texture loading (nexus_v1_rasterizer.c), model loading (nexus_v1_dmdf_model.c), text extraction (nexus_v1_text.c) -- none have tests.

3. **No game state tests** -- nexus_v1_game.c init/load_level/cd_track have no tests.

4. **No renderer tests** -- nexus_v1_rasterizer.c, nexus_v1_viewport.c, nexus_v1_dmdf_model.c -- no tests for 3D rendering output.

5. **No mechanics tests** -- nexus_v1_combat.c, nexus_v1_champions.c, nexus_v1_creatures.c, nexus_v1_magic.c -- no tests for any game mechanics.

### No Cross-Reference Tests

- DM1 V1 tests (387 tests) serve as proxy for Nexus mechanics, but no test explicitly references Nexus
- No test verifies that Nexus uses the same dungeon loading code path as DM1
- No test verifies that Nexus combat calls the same functions as DM1 combat

---

## Fixture Availability

| Fixture Type | DM1 | Nexus |
|-------------|-----|-------|
| Dungeon files | YES (canonical) | NO -- needs Saturn disc |
| Graphics assets | YES (canonical) | NO -- needs Saturn disc |
| Save files | YES | NO |
| Input scripts | YES | NO |
| Viewport baselines | YES | NO |
| World state hashes | YES | NO |

**Nexus fixtures: NONE. Cannot write integration tests without the disc image.**

---

## Probes

Nexus has no probes. DM1 has probes in probes/dm1/:
- dm1_v1_viewport_draw_order_probe
- dm1_v1_game_loop_redraw_cadence_probe
- dm1_v1_viewport_palette_as_before_probe

These probes exist but are not built (Could not find executable in CTest).

No equivalent probes exist in probes/nexus/.

---

## What Would Give Good Nexus Test Coverage

1. **ISO extractor test**: Run extract_nexus_iso.py against a real disc image, verify file count and directory structure
2. **Dungeon parser test**: Load parsed dungeon data, verify level count, map dimensions, square types against known values
3. **Texture decoder test**: Verify VDP1 texture decodes to valid bitmap (no mojibake, correct palette)
4. **Model decoder test**: Verify DMDF model loads, vertex count, face count
5. **Text extraction test**: Verify Shift-JIS to UTF-8 produces valid Japanese text (or English if translated)
6. **Render smoke test**: Render a single frame at 320x224, verify output buffer has non-zero content
7. **Game init test**: Initialize game state, verify party position, direction, level
8. **CD audio track mapping test**: Verify level -> track mapping (Track 2-9 for levels 0-15)
9. **Deterministic hash test**: Run game for 100 ticks, hash state, compare across platforms

None of these can be written without the Sega Saturn disc image.

---

## Build Status

- libfirestaff_nexus.a compiles cleanly: YES (CMake build succeeds)
- Nexus source files: 20
- Nexus header files: 10+ in include/
- Warnings in Nexus build: NONE (target_compile_options: -Wall -Wextra -O2)
- Linked into any executable: NO

The library exists but nothing uses it yet.