# DM1 V2.0 — Detailed implementation plan and SDL graphics settings

**Status:** Draft 2026-05-26 (subagent on N2 / Firestaff-Worker-VM)
**Repo:** `/home/trv2/work/firestaff`
**Reference source code:** `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`
**Scope:** DM1 (V1 gameplay route + V2 presentation shell). CSB/DM2/Nexus are not covered here.

---

## 0. Summary and current state

### What the README says about V2.0
README.md (lines 23–28) defines the graphics modes as follows:

| Mode | Description |
|------|-------------|
| **V1 Original** | Pixel-perfect 320×200, exactly as the original |
| **V2.0 Filtered** | Original graphics + CRT scanlines, palette correction, sharpening |
| **V2.1 Upscaled** | 10× AI upscale that preserves the DM aesthetic |
| **V2.2 Modern** | Completely new 3D-rendered 2D art |

Note: `--scale-mode <n>` has command-line choices `1=V1, 2=V2.1, 3=V2.2` —
**V2.0 is currently missing from the CLI mapping** (line 101). This plan closes that gap.

### Existing V2 infrastructure (already in place)

- `include/dm1_v2_presentation_profile_pc34.h` — `DM1_V2_PresentationMode { V1_ORIGINAL=0, V2_SHELL=1 }`, profile struct, snapshot, tick.
- `include/dm1_v2_settings_pc34.h` — `DM1_V2_Settings` with `viewport_scale`, `use_epx`, `use_bilinear`, `palette_enhanced`, `aspectMode`, and related fields.
- `include/config_m12.h` — `M12_Config.dm1V2*` fields (`ScalePercent`, `SmoothingEnabled`, `DynamicLightingEnabled`, `AccessibilityTouchEnabled`, `AspectMode`).
- `src/dm1v2/dm1_v2_*.c` — approximately 46 V2 modules (achievements, journal, minimap, particle, viewport_renderer, presentation_profile, settings, lighting_dynamic, viewport_renderer, and more).
- `src/ui/menu_startup_m12.c` (lines 102–168) — `m12_ext_settings[]` already has **placeholder rows** for V2.0 in the GRAPHICS tab:
  - `"CRT Filter"` (V2.0, enabled=0)
  - `"Palette Correction"` (V2.0, enabled=0)
  - `"Dither Cleanup"` (V2.0, enabled=0)
  - `"Sharpening"` (V2.0, enabled=0)

These rows are visible but their cycle function is `enabled=0`. **They must
become `enabled=1` when V2.0 is complete.**

### Existing SDL pipeline (M11)

- `src/engine/render_sdl_m11.c` (1,191 lines) — single global `M11_RenderState`. Pipeline:
  1. Game code draws to `framebuffer[]` (320×200, 1 byte/pixel: 4-bit palette index + 4-bit per-pixel level).
  2. `m11_framebuffer_to_rgba()` expands through `G9010_auc_VgaPaletteAll_Compat[level][idx]` → `presentBuffer[]` (RGBA8888, exact framebuffer size).
3. `SDL_UpdateTexture()` → streaming texture (`SDL_PIXELFORMAT_RGBA32`).
  4. `SDL_RenderTexture(renderer, texture, src, dest)` with `destRect` calculated by `M11_Render_ComputePresentationRect()` (4:3 / 16:9 / content aspect, integer scaling, fit/stretch).
  5. `SDL_SetTextureScaleMode(NEAREST | LINEAR)` controls GPU filtering.

This is the only hook point for V2.0 filters. Post-processing must happen
between steps 2 and 3 (CPU-based, on `presentBuffer`) or through a secondary
render target (the GPU-shader route).

### ReDMCSB source code — is there an original equivalent?

- `PALETTE.C` (453 lines): handles VGA DAC and dimming levels (`G0010_aab_PalCh*`). **No CRT emulation, sharpening, or scanline logic.**
- `VIDEODRV.C` (4,003 lines): VGA driver using inline assembly against CRTC ports 0x3D4/0x3B4. It talks to real hardware, not an emulated CRT.
- `_MAIN.C`, `VDEOMAIN.C`, `VIDSET.C`: small shim files; no post-processing.

**Conclusion: V2.0 is purely Firestaff work.** The original source has no source-locked CRT/scanline/sharpening counterpart. Palette correction (gamma adjustment of the VGA palette) is an *interpretation* of VGA→sRGB and can be motivated by reference to `G9010_auc_VgaPaletteAll_Compat[]` (our own palette tables) plus the well-known VGA gamma (~2.2 PC CRT).

---

## PART 1 — DM1 V2.0 Detailed Plan

### 1.1 Exact definition of V2.0

V2.0 = **V1 gameplay route + V1 pixel content + post-processing filter chain**. No gameplay changes and no new graphics are loaded. Only `presentBuffer[]` (or a secondary render target) is modified before `SDL_RenderTexture`.

Filter chain (in this order, to match CRT physics):

| Step | Filter | Default | Source status |
|------|--------|---------|--------------|
| A | Palette correction (gamma 1.0 → 2.2 adjustment + brightness/contrast) | Off | Firestaff (motivated by VGA DAC ~6-bit RGB) |
| B | Dither cleanup (3×3 mode filter on indexed pixels **before** RGBA expansion) | Off | Firestaff |
| C | Sharpening (3×3 unsharp mask on RGBA after expansion) | Off | Firestaff |
| D | CRT scanlines (every other row multiplied by 0.5–0.85) | Off | Firestaff |

(Vignette/bloom/temperature are deferred to V2.0.5 or V2.1 — see PART 2.)

### 1.2 Architecture — where V2.0 sits in the pipeline

```
game code ──► framebuffer (320×200, idx)
              │
              ▼  (step B runs here when enabled)
        [dither_cleanup_indexed()]
              │
              ▼
        framebuffer_to_rgba()  (step A: uses palette_corrected[][] instead of VGA table)
              │
              ▼  presentBuffer (320×200, RGBA8888)
              │
              ▼  (step C runs here when enabled)
        [unsharp_mask_rgba()]
              │
              ▼  (step D runs here when enabled)
        [crt_scanlines_rgba()]
              │
              ▼
        SDL_UpdateTexture ► SDL_RenderTexture (with NEAREST or LINEAR)
```

**Designprinciper:**
- **The filter chain is CPU-based in V2.0.** No shaders — Firestaff supports SDL3/SDL2, and we want to avoid GLSL/HLSL/MSL variants for something that runs at ~60 FPS on 320×200 = 64k pixels. The measurable CPU cost is negligible.
- **Everything happens in `render_sdl_m11.c`** in a new internal helper function, `m11_apply_v2_filters()`, called immediately before `SDL_UpdateTexture`.
- **State is read from `M12_Config.dm1V2*` flags** (new bits — see 1.3) through a single setter, `M11_Render_SetV2Filters(...)`.
- **The V1 path is unaffected.** When `presentationMode == V1_ORIGINAL`, `m11_apply_v2_filters()` returns early.
- **Per-game flag.** V2.0 should be switchable per game through `M12_Config.gameVersionIndex[]` × graphicsIndex; for now, a global runtime flag bound to DM1 is sufficient.

### 1.3 New configuration fields (`config_m12.h`)

Add after line 76 (`dm1V2AspectMode`):

```c
/* DM1 V2.0 filter chain (V2-only; V1 launch path ignores these) */
int dm1V2CrtScanlinesEnabled;        /* 0 = off, 1 = on (50% darken even rows) */
int dm1V2CrtScanlineStrength;        /* 0-100, percent darken; default 35 */
int dm1V2PaletteCorrectionEnabled;   /* 0 = off, 1 = on */
int dm1V2PaletteGamma;               /* 80-260 (= 0.80..2.60 ×100); default 220 */
int dm1V2PaletteBrightness;          /* -50..+50 percent; default 0 */
int dm1V2PaletteContrast;            /* -50..+50 percent; default 0 */
int dm1V2DitherCleanupEnabled;       /* 0 = off, 1 = on (3×3 mode filter on indexed) */
int dm1V2SharpeningEnabled;          /* 0 = off, 1 = on */
int dm1V2SharpeningStrength;         /* 0-100, percent; default 30 */
```

Set defaults in `M12_Config_SetDefaults()` so V2.0 starts with ALL filters at `0` (off) — an exact V1 appearance, enabling direct A/B testing.

### 1.4 New `render_sdl_m11` API (`render_sdl_m11.h`)

```c
int M11_Render_SetV2Filters(int crtScanlines,
                            int crtStrength,
                            int paletteCorrection,
                            int paletteGamma,
                            int paletteBrightness,
                            int paletteContrast,
                            int ditherCleanup,
                            int sharpening,
                            int sharpeningStrength);
int M11_Render_GetV2Filters(/* out params */);
```

Backed by new fields in `M11_RenderState`:

```c
int v2_crt_enabled, v2_crt_strength;
int v2_palette_enabled, v2_palette_gamma100, v2_palette_brightness, v2_palette_contrast;
int v2_dither_enabled;
int v2_sharpen_enabled, v2_sharpen_strength;
unsigned char v2_palette_corrected[M11_PALETTE_LEVELS][16][3]; /* precomputed LUT */
```

### 1.5 New files (`src/dm1v2/`)

| File | Contents | Source evidence |
|-----|----------|------------------|
| `src/dm1v2/dm1_v2_filter_palette_correct.c` | `dm1_v2_filter_palette_build_lut(gamma100, bright, contrast, out_lut)` — builds a `[levels][16][3]` LUT that replaces `G9010_auc_VgaPaletteAll_Compat[]` in `framebuffer_to_rgba()` when enabled. | Firestaff; references `G9010_auc_VgaPaletteAll_Compat`. |
| `src/dm1v2/dm1_v2_filter_dither_cleanup.c` | `dm1_v2_filter_dither_cleanup_indexed(uint8_t* fb, int w, int h)` — 3×3 mode filter over index bytes; preserves level bits; skips overlay/UI regions. | Firestaff. |
| `src/dm1v2/dm1_v2_filter_sharpen.c` | `dm1_v2_filter_sharpen_rgba(uint8_t* rgba, int w, int h, int strength_pct)` — 3×3 unsharp mask, separable approximation. | Firestaff. |
| `src/dm1v2/dm1_v2_filter_crt_scanlines.c` | `dm1_v2_filter_crt_scanlines_rgba(uint8_t* rgba, int w, int h, int strength_pct)` — every other row is multiplied by (1 − s/100). | Firestaff. |
| `include/dm1v2/dm1_v2_filters.h` | Aggregate header. | — |

### 1.6 Wiring i render_sdl_m11.c

Add `m11_apply_v2_filters()` between `m11_framebuffer_to_rgba()` and `SDL_UpdateTexture()` in `M11_Render_Present()` (around lines 670–705). Order:

1. When `v2_dither_enabled`: run `dm1_v2_filter_dither_cleanup_indexed()` on `framebuffer[]` *before* `framebuffer_to_rgba`.
2. `framebuffer_to_rgba()` — when `v2_palette_enabled`, use `v2_palette_corrected[][]` instead of `G9010_auc_VgaPaletteAll_Compat[][]`.
3. When `v2_sharpen_enabled`: run `dm1_v2_filter_sharpen_rgba()` on `presentBuffer[]`.
4. When `v2_crt_enabled`: run `dm1_v2_filter_crt_scanlines_rgba()` on `presentBuffer[]`.

### 1.7 Wiring in `menu_startup_m12.c`

- Lines 121–124: change `enabled=0` → `enabled=1` for CRT Filter, Palette Correction, Dither Cleanup, and Sharpening.
- Add cycle handlers in `m12_cycle_game_opt_with_mode()` / `m12_cycle_ext_setting()` that map to the new `dm1V2*` configuration fields.
- Lock filter rows when `presentationModeIndex == V1_ORIGINAL` through `M12_GameOptions_RowLockedByMode()` (the same mechanism that already locks V2.1/V2.2 rows).

### 1.8 CLI-wiring

- `firestaff_cli.c`: `--scale-mode 2` is currently V2.1 according to the README. **Change the mapping:**
  - `1` = V1 Original
  - `2` = V2.0 Filtered (new — or an alias for V1 + default filter preset)
  - `3` = V2.1 Upscaled (previously 2)
  - `4` = V2.2 Modern (previously 3)
- Update README lines 101 and 115 at the same time (or retain `--scale-mode 2 = V2.1` and introduce `--scale-mode 5 = V2.0`; decide at implementation start to avoid breaking existing documentation/scripts).

### 1.9 Implementation milestones

🔲 **M1 — Configuration skeleton** (1–2 hours)
- 🔲 Add the 9 new `dm1V2*` fields to `M12_Config`.
- 🔲 Update `M12_Config_SetDefaults()` with default values.
- 🔲 Verify that `M12_Config_Save()` writes the fields and `M12_Config_Load()` reads them (they use a generic INI loop, so usually only mapping is needed).
- 🔲 Build and run `tests/test_m12_config_*` so no existing configuration tests break.

🔲 **M2 — Filter LUT builder (palette correction)** (2 hours)
- 🔲 Create `src/dm1v2/dm1_v2_filter_palette_correct.c` with a pure-C LUT builder.
- 🔲 Unit test: `tests/test_dm1_v2_filter_palette_correct.c` — check that gamma=2.20 lifts dark colors and gamma=1.00 is identity within ±1.
- 🔲 Source-evidence comment referencing `G9010_auc_VgaPaletteAll_Compat`.

🔲 **M3 — Dither cleanup (indexed mode filter)** (2 hours)
- 🔲 Create `dm1_v2_filter_dither_cleanup.c`. Use a 3×3 mode window on index bytes. Protect level bits.
- 🔲 Unit test: build a 320×200 test image with checkerboard dithering and verify that the result is the dominant color.

🔲 **M4 — Sharpening (unsharp mask)** (2 hours)
- 🔲 Create `dm1_v2_filter_sharpen.c`. Separable 3×3 box blur → original − blur × strength.
- 🔲 Unit test: a horizontal step function should become sharper; a flat region remains unchanged.

🔲 **M5 — CRT scanlines** (1 hour)
- 🔲 Create `dm1_v2_filter_crt_scanlines.c`. Every other row: RGB *= (1 − s/100).
- 🔲 Unit test: odd rows unchanged, even rows scaled according to strength.

🔲 **M6 — Render pipeline integration** (3 hours)
- 🔲 Add the `M11_Render_SetV2Filters()` / `GetV2Filters()` API in `render_sdl_m11.h/.c`.
- 🔲 Hook it into `M11_Render_Present()` (step order as in 1.6).
- 🔲 Build the LUT at the set call, not per frame.
- 🔲 Smoke test: run `firestaff --scale-mode 2 --duration 5000` with and without flags.

🔲 **M7 — Menu wiring** (2 hours)
- 🔲 Enable `enabled=1` for CRT/Palette/Dither/Sharpening in `m12_ext_settings[]`.
- 🔲 Add cycle handlers in `menu_startup_m12.c`.
- 🔲 Lock rows when V1 presentation is selected.
- 🔲 Verify through a screenshot test (compare the `verification-screens/` baseline).

🔲 **M8 — CLI + README** (1 hour)
- 🔲 Decide the scale-mode mapping (see 1.8).
- 🔲 Update `firestaff_cli.c`.
- 🔲 Update README.md lines 101/115/119–123.
- 🔲 Add `RELEASE_2.5.0.md` (or similar).

🔲 **M9 — Live runtime toggle** (1 hour)
- 🔲 Ensure all four filters can be enabled/disabled without restart (configuration → SetV2Filters → the next frame shows the effect).

🔲 **M10 — Regression protection and screenshots** (2 hours)
- 🔲 Add PNG baselines in `verification-screens/dm1_v2_filters/`: off, CRT_only, palette_only, dither_only, sharpen_only, all.
- 🔲 Add a test that takes a screenshot through `firestaff --duration 0 --script enter` and compares it with the baseline.

**Total uppskattning: ~16 timmar fokuserat arbete.**

### 1.10 Dependencies

| Dependency | Status | What is needed first |
|----------|--------|-------------------|
| V1 viewport (DM1) | ✅ Complete (entry view, walls, doors, ESC dialog) | — |
| V1 framebuffer pipeline | ✅ Complete (M11 render path) | — |
| V1 palette (VgaPaletteAll_Compat) | ✅ Complete | — |
| M12_Config persistence | ✅ Complete | — |
| V2 presentation shell (V2_SHELL mode) | ✅ Complete (presentation_profile_pc34) | — |
| V2 viewport renderer | ⚠ Partial (renders in its own `DM1_V2_ViewportState`, not yet connected to the V1 framebuffer) | Not blocking — V2.0 reads the V1 framebuffer directly |

**No blockers.** Work on the full V2.0 scope can begin today.

### 1.11 ReDMCSB source locking

**V2.0 is entirely Firestaff work.** It has no source-locked elements. This must be documented clearly in every new file:

```c
/* Source: Firestaff V2.0 filtered presentation.
 * No ReDMCSB original equivalent — ReDMCSB targets raw VGA DAC output
 * via real-hardware CRTC port programming (VIDEODRV.C). V2.0 emulates
 * the perceived CRT look on modern flat-panel displays. */
```

This is important: AGENTS.md identifies "source-locked" as a core value. V2.0 is opt-in and designated as a pure presentation layer; the V1 gameplay route remains 100% source-locked.

### 1.12 Testplan

🔲 **Build verification**
```bash
cd /home/trv2/work/firestaff
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```
🔲 **Unit tests per filter** — separate `tests/test_dm1_v2_filter_*.c` files.
🔲 **V1 unchanged smoke test** — run `firestaff --scale-mode 1` and compare against the baseline screenshot.
🔲 **V2.0 off-by-default smoke test** — run `firestaff --scale-mode 2` with no filter flags → identical to V1.
🔲 **Runtime filter on/off toggling** — use `--script` to tab into the GRAPHICS menu and toggle.
🔲 **Performance measurement** — use `--fps` at 320×200; all active filters must not reduce performance below 60 FPS on Steam Deck-class hardware.
🔲 **Pixel comparison** — `parity-evidence/dm1_v2_filters/` with a PNG diff for each solo filter.

Verification command:
```bash
cmake --build /home/trv2/work/firestaff/build --parallel && \
  /home/trv2/work/firestaff/build/firestaff --scale-mode 1 --duration 2000 && \
  /home/trv2/work/firestaff/build/firestaff --scale-mode 2 --duration 2000
```

---

## PART 2 — SDL graphics settings (new features)

### 2.1 Proposed settings

| # | Setting | Where | Type | Implementation |
|---|-------------|-----|-----|----------------|
| 1 | CRT Scanlines (V2.0) | M12_Config + GUI | On/Off + 0–100 strength | CPU `dm1_v2_filter_crt_scanlines.c` |
| 2 | Palette Correction (V2.0) | M12_Config + GUI | On/Off + gamma/bright/contrast | CPU LUT |
| 3 | Dither Cleanup (V2.0) | M12_Config + GUI | On/Off | CPU 3×3 mode-filter |
| 4 | Sharpening (V2.0) | M12_Config + GUI | On/Off + 0–100 strength | CPU 3×3 unsharp |
| 5 | Interpolation Filter | M12_Config (exists) + GUI | Nearest / Linear (SDL supports both) | `SDL_SetTextureScaleMode` (already implemented) |
| 6 | Vignette | M12_Config + GUI | On/Off + radius | Simple CPU — `dm1_v2_filter_vignette.c` |
| 7 | Bloom / Glow | M12_Config + GUI | On/Off + threshold/strength | CPU — bright pass + 5×5 blur; costly |
| 8 | Color Temperature | M12_Config + GUI | Slider −100..+100 (cool↔warm) | Included in palette LUT |
| 9 | Integer Scale | M12_Config (exists) | On/Off | Already in `render_sdl_m11.c` |
| 10 | Aspect Ratio | M12_Config (exists) | 4:3 / 16:9 / Content | Already in `render_sdl_m11.c` |
| 11 | CRT Curvature | M12_Config + GUI | On/Off + curve | **Shader-only** (deferred to V2.x or a future shader path) |
| 12 | NTSC Composite Artifact | M12_Config + GUI | On/Off | **Shader-only** (deferred) |

### 2.2 GUI-design

In `m12_ext_settings[]` on the GRAPHICS tab, add V2.0 rows **immediately after** `Palette Mode` (line 119):

```c
{"CRT Filter",           "Off",   1, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0 — toggle */
{"CRT Strength",         "35%",   1, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0 — 0–100 */
{"Palette Correction",   "Off",   1, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0 — toggle */
{"Palette Gamma",        "2.20",  1, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0 — 0.80–2.60 */
{"Palette Brightness",   "0",     1, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0 — −50..+50 */
{"Palette Contrast",     "0",     1, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0 — −50..+50 */
{"Color Temperature",    "0",     1, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0 — cool↔warm */
{"Dither Cleanup",       "Off",   1, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0 — toggle */
{"Sharpening",           "Off",   1, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0 — toggle */
{"Sharpening Strength",  "30%",   1, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0 — 0–100 */
{"Vignette",             "Off",   1, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0 — toggle */
{"Bloom",                "Off",   0, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0.5 — costly */
```

UI pattern:
- **Toggle** (On/Off): left/right arrow cycles.
- **Slider** (percent/gamma): left/right arrow changes in steps of 5 (quick step with Shift).
- **Sub-menu**: no. Slider rows are as easy to understand inline and consistent with existing Volume rows.
- **Lock when V1 is selected**: rows with the `/* V2.0 */` comment are locked (grayed out) when `presentationModeIndex == V1_ORIGINAL`, through the same mechanism that currently locks V2.2 rows.

### 2.3 Which filters require a shader vs. CPU

| Filter | CPU suitable? | Shader better? | V2.0 decision |
|--------|---------|---------------|-----------------|
| CRT scanlines | ✅ trivial | ✅ more attractive with a Gaussian pulse | CPU |
| Palette correction | ✅ LUT once | gradient in shader | CPU |
| Dither cleanup | ✅ must be CPU (indexed) | no | CPU |
| Sharpening | ✅ 3×3 is sufficient | ✅ for 5×5+ | CPU |
| Vignette | ✅ trivial | ✅ more attractive with radial Gaussian | CPU |
| Bloom | ⚠ CPU-intensive | ✅ shader | DEFER to V2.0.5 |
| CRT curvature | no | ✅ requires a shader | DEFER |
| NTSC composite | ⚠ intensive | ✅ shader | DEFER |
| Color temperature | ✅ included in palette LUT | yes | CPU |
| Interpolation | already SDL | already SDL | complete |
| Integer scale | already M11 | — | complete |
| Aspect | already M11 | — | complete |

### 2.4 Shader path (for V2.0.5+, not V2.0)

If Firestaff later wants CRT curvature/NTSC/bloom, it requires:
- A secondary render target (RT) through `SDL_CreateTexture(SDL_TEXTUREACCESS_TARGET)`.
- A custom shader pipeline. SDL3 does not have a built-in shader API in the same way as SDL_gpu or bgfx — a typical framework is:
  - `SDL_RenderGeometry` with custom textures + multiple passes.
  - Or SDL3 + GLSL through `SDL_GPUDevice` (SDL3 v3.2+ supports this).
- Decision: **not in V2.0**. Mark it as future work in `TODO.md`.

### 2.5 Configuration fields for the shader path (preparatory)

Stub in configuration, but `enabled=0` in the GUI until the shader pipeline exists:

```c
int dm1V2VignetteEnabled;
int dm1V2VignetteStrength;     /* 0–100, default 25 */
int dm1V2ColorTemperature;     /* −100..+100, default 0 */
int dm1V2BloomEnabled;         /* off in V2.0; future */
int dm1V2CrtCurvatureEnabled;  /* shader-only; future */
```

---

## PART 3 — ReDMCSB source locking for V2

### 3.1 Review of relevant source-code files

| File | Contents | Relevance to V2.0 |
|-----|----------|-------------------|
| `PALETTE.C` (453 lines) | VGA DAC writes, palette tables, dimming levels | Basis for `G9010_auc_VgaPaletteAll_Compat` (already in Firestaff). No post-processing. |
| `VIDEODRV.C` (4003 lines) | VGA-port driver (inline ASM to 0x3D4/0x3B4) | **No post-processing** — talks directly to real hardware. Only one CRT-related match: a comment about the VGA CRTC controller port. |
| `STARTEND.C` | Missing from this ReDMCSB snapshot | — |
| `_MAIN.C`, `VDEOMAIN.C`, `VIDSET.C` | Small shim files | No filter hook. |
| `FILTERS.C` | Does not exist | — |
| `EVENT.C` | Not found by grep — may exist, but has no filter matches | — |

### 3.2 What ReDMCSB does with bit depth/palette

ReDMCSB writes 4-bit/pixel data to the VGA Mode 13h linear buffer (or equivalent). The palette is written as **6 bits per channel** to the VGA DAC. Firestaff already handles this through `G9010_auc_VgaPaletteAll_Compat[level][idx]` (16 palettes of 16 colors, expanded to RGB888). No additional bit-depth conversion is needed for V2.0.

### 3.3 Conclusion: source locking

- **V1 gameplay/render path:** 100% source-locked to ReDMCSB. Nothing changes.
- **V2.0 filter chain:** **Firestaff-original work.** No ReDMCSB equivalent exists. This is documented in every new `dm1_v2_filter_*.c` with a source-evidence comment (see 1.11).
- **V2 presentation profile** (`presentation_profile_pc34.c`): already in place; `presentationMode == V2_SHELL` is the hook where V2.0 is enabled.

This is the same policy already used for V2.1 (AI upscale) and V2.2 (modern art): **gameplay = source-locked, presentation = opt-in new work.**

---

## 4. Priority order (what to do first)

1. 🔲 **M1 Configuration** — all nine new fields plus defaults. Nothing else can begin until M12_Config can persist them.
2. 🔲 **M2 Palette LUT** — the single most visible effect; easy to verify with PNG.
3. 🔲 **M5 CRT Scanlines** — trivial to implement, immediately provides a "wow" effect.
4. 🔲 **M4 Sharpening** — builds confidence in the pipeline flow.
5. 🔲 **M3 Dither Cleanup** — requires more care because of indexed data.
6. 🔲 **M6 Pipeline Integration** — connects everything; this is where it becomes "live".
7. 🔲 **M7 Menu Wiring** — makes features discoverable.
8. 🔲 **M8 CLI + README** — documentation.
9. 🔲 **M9 Runtime Toggle** — ensure no restart is required.
10. 🔲 **M10 Regression Screenshots** — locks down the finished state.

---

## 5. Verification command

```bash
cd /home/trv2/work/firestaff && \
  cmake -B build -DCMAKE_BUILD_TYPE=Release && \
  cmake --build build --parallel && \
  ./build/firestaff --scale-mode 1 --duration 2000 && \
  ./build/firestaff --scale-mode 2 --duration 2000
```

Expected: both runs end with exit code 0, no segmentation fault, no visual regression in V1, and V2.0 displays an identical image to V1 when all filters are `Off` (the default).

---

## 6. Risks and open questions

- **Scale-mode mapping:** `--scale-mode 2` currently means V2.1. Mapping 2→V2.0 would break README examples. Proposal: retain 2=V2.1, assign V2.0 to `--scale-mode 5`, OR introduce a named `--graphics v1|v2.0|v2.1|v2.2` flag. **Decide at the start of M8.**
- **Expected CRT-emulation quality:** Daniel may have a specific reference image (CRT Royale, ReShade, etc.) — request a reference before M5 is finalized.
- **ARM64 performance (Steam Deck):** all four filters active at 320×200 are ~64k pixels × 4 RGBA passes = negligible. They should not reduce FPS. Measure in M10.
- **Per-game flag:** V2.0 is DM1-specific in this plan. CSB/DM2/Nexus get their own V2.0 later. Configuration fields must be named `dm1V2*` and not reused by other games.

---

## 7. File manifest — what changes

**New files (10):**
- `include/dm1v2/dm1_v2_filters.h`
- `src/dm1v2/dm1_v2_filter_palette_correct.c`
- `src/dm1v2/dm1_v2_filter_dither_cleanup.c`
- `src/dm1v2/dm1_v2_filter_sharpen.c`
- `src/dm1v2/dm1_v2_filter_crt_scanlines.c`
- `tests/test_dm1_v2_filter_palette_correct.c`
- `tests/test_dm1_v2_filter_dither_cleanup.c`
- `tests/test_dm1_v2_filter_sharpen.c`
- `tests/test_dm1_v2_filter_crt_scanlines.c`
- `verification-screens/dm1_v2_filters/README.md` + 6 PNG-baselines

**Changed files (6):**
- `include/config_m12.h` (+9 fields)
- `src/config_m12.c` (defaults, load/save mapping)
- `include/render_sdl_m11.h` (+ SetV2Filters/GetV2Filters API)
- `src/engine/render_sdl_m11.c` (+ state, + apply_v2_filters hook)
- `src/ui/menu_startup_m12.c` (+ lines 121–124 `enabled=1`, + new rows, + cycle handlers)
- `src/firestaff_cli.c` (+ scale-mode mapping or `--graphics` flag)
- `CMakeLists.txt` (+ new .c files)
- `README.md` (+ V2.0 row and CLI update)

---

*Plan written by a subagent on N2 / Firestaff-Worker-VM, 2026-05-26. No changes in `src/` were made in this pass — only this plan file.*
