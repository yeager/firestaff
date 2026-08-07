# Firestaff DM1 V2 Code Review — Complete

## Files Reviewed: 40 (4838 lines total)

## Bugs Found

### V2-BUG-001: [minor] Movement engine collision check uses row-major layout
- File: src/dm1v2/dm1_v2_movement_engine_pc34.c, function dm1_v2_collides_at
- Problem: `int index = py * mapW + px` is row-major, but DM1 uses column-major (px * mapH + py per ReDMCSB DUNGEON.C F0151 and BUG-008/BUG-036)
- Fix: Change to `int index = px * mapH + py`
- Impact: Medium — V2 movement collision would check wrong tiles

### V2-BUG-002: [minor] HUD digit font data is placeholder (all digits render as rectangles)
- File: src/dm1v2/dm1_v2_hud_overlay_pc34.c, g_v2_digits[10][5]
- Problem: All 10 digit entries have the same pattern {0x7E, 0x41, 0x41, 0x41, 0x7E} (rectangle). Only digit 0 and 1 have separate entries but 1 is all-zeros. Digits 2-9 all render as "0".
- Fix: Replace with proper 5x5 digit bitmaps (the damage_numbers module already has correct g_digit_font)
- Impact: Low — HUD digits are unreadable but this is scaffolding

### V2-BUG-003: [minor] Weather drop spawn rate can produce negative spawn_rate
- File: src/dm1v2/dm1_v2_weather_fx_pc34.c, v2_weather_update
- Problem: `spawn_rate = 1.0f / (state->intensity * 200.0f)` — if intensity is very close to 0 but non-zero, spawn_rate can be enormous, causing spawn_timer to never trigger. Not really a bug but inefficient. The real issue: the `while` loop can spam 512 drops in one frame if dt is large.
- Fix: Cap drops spawned per frame to ~20 to prevent frame stutter

### V2-BUG-004: [minor] Smooth movement uses global state (not per-instance)
- File: src/dm1v2/dm1_v2_smooth_movement_pc34.c
- Problem: g_move, g_rot, g_smooth are file-static globals. Only one movement/rotation can be interpolated at a time. If V2 ever needs multiple simultaneous smooth transitions (e.g., camera + creature), this breaks.
- Fix: Future — make state per-instance when V2 supports multiple animated entities

## Verified Correct (No Bugs)
- **Viewport renderer** (1067 lines) — well-structured draw command pipeline, correct DUNVIEW.C source references, proper depth-order traversal, EPX upscaler, inscription overlay
- **Runtime** — clean V2 shell with V1-compatible tick semantics
- **Camera controller** — correct lerp interpolation, presentation-only (doesn't mutate game state)
- **Movement command adapter** — correctly translates V1 command IDs to V2 runtime
- **Animation timing** — 7 easing functions, V1-tick-synced clock, proper ping-pong loops
- **Lighting dynamic** — inverse-square falloff, flicker, ambient per-tile propagation
- **Pathfinding** — A* with proper open/closed sets, Manhattan heuristic
- **Particle system + presets** — clean emitter/particle lifecycle
- **Spell effect overlay** — correct DM1 explosion thing→VFX mapping
- **HUD interaction** — properly consumes V1 PC34 touch route matrix
- **Damage numbers** — correct floating number rendering with proper digit font
- **Tooltip, minimap, journal, achievements, stat tracker** — clean scaffolding
- **Auto-save, input remap, settings** — clean configuration wrappers
- **Screenshot, screen transition, level transition** — simple utilities
- **Audio mixer, footstep audio** — clean audio state management
- **Creature animation** — proper pose/frame cycling
- **Champion select, inventory sort** — clean UI helpers
- **Message log** — scrollable buffer with proper ring buffer

## Code Quality Assessment
V2 code is presentation-layer scaffolding that intentionally does NOT modify V1 gameplay state. Every file that bridges V1 data properly cites ReDMCSB source anchors. The architecture is sound: V1 game logic runs identically, V2 adds visual overlays. Only V2-BUG-001 (collision layout) is functionally significant.

## Summary
- 40 files reviewed
- 4 bugs found (1 medium, 3 minor)
- V2-BUG-001 should be fixed before V2 movement is enabled
