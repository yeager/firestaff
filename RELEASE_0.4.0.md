## Firestaff v0.4.0

Major milestone: DM1 V2.1 (Upscaled) + V2.2 (Enhanced) + full ReDMCSB code parity.

### DM1 V2.1 — Upscaled (Original-Faithful at Higher Resolution)

- **EPX/Scale2x upscaler**: Pixel art-preserving doubler for indexed 320×200 V1 frames. No blurry bilinear on pixel art — EPX detects edges and expands cleanly.
- **Palette-aware RGBA pipeline**: V1 indexed framebuffer → EPX 2× → VGA palette lookup → RGBA output. The original 256-color VGA palette is preserved exactly.
- **V2.1 viewport renderer**: The V1 draw pipeline renders at native 224×136 resolution, then V2.1 upscales to display. Pixel-identical to V1 at the logical level.
- **Audio mixer**: V1 PC-34 sound events mapped through SDL_mixer for modern audio output.
- **HUD panel upscaling**: Champion status bars, action icons, compass — all EPX-upscaled.
- **V2.1 settings**: Scale factor (2×/4×), EPX toggle, bilinear override, palette mode, vsync.
- **V2.1 runtime loop**: V1 game tick → V1 render → EPX upscale → palette → SDL present.

### DM1 V2.2 — Enhanced (New Features)

- **Smooth movement**: Interpolated walk/turn/stairs transitions with ease-out cubic easing. Game state updates instantly (V1-compatible); only the camera smoothly transitions.
- **Camera shake**: Trauma-based screen shake on damage/explosions (quadratic falloff).
- **Minimap**: Explored dungeon overview with colored square types (wall/corridor/door/stairs/pit).
- **Floating damage numbers**: Spawn on hit, drift upward, fade out. Red/yellow/green by type.
- **Weather FX**: Atmospheric overlays per dungeon zone (rain, fog, embers, water drip).
- **Particle presets**: Torch flame/smoke, spell effects, blood, water, dust, magic sparkle.

### ReDMCSB Code Parity

- **40/40 gameplay source files at 100% function citation**: Every function in COMMAND.C, CHAMPION.C, GROUP.C, DUNGEON.C, DUNVIEW.C, MEMORY.C, TIMELINE.C, OBJECT.C, IO.C, COORD.C, ANIM.C, SOUND.C, PANEL.C, STATS.C, and 26 more files is now documented in Firestaff source with exact ReDMCSB line numbers.
- **pass593**: 5,767+ lines of movement/viewport/walls source-locked code merged.
- **pass601**: Combat (F0309-F0321), spell, inventory, creature AI, memory/cache system.
- **pass602**: 396 previously uncited functions documented.
- **35/35 V2 modules complete**.

### Test Results

- 305 tests, 303 pass (99%).
- 2 pre-existing blocked tests (pass372, pass377).
- 0 new regressions.

### Cleanup

- 471 → 1 local worktrees (freed 123 GB disk on N2).
- 784 stale local branches pruned.
- 47 stale GitHub remote branches deleted.
