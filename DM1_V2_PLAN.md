# DM1 V2.0 — Detaljerad implementationsplan & SDL-grafikinställningar

**Status:** Utkast 2026-05-26 (subagent på N2 / Firestaff-Worker-VM)
**Repo:** `/home/trv2/work/firestaff`
**Referenskällkod:** `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`
**Scope:** DM1 (V1 gameplay route + V2 presentation shell). CSB/DM2/Nexus berörs inte här.

---

## 0. Sammanfattning & lägesbild

### Vad README säger om V2.0
README.md (raderna 23–28) definierar grafiklägena så här:

| Mode | Beskrivning |
|------|-------------|
| **V1 Original** | Pixel-perfect 320×200, exakt som original |
| **V2.0 Filtered** | Original-grafik + CRT scanlines, palette correction, sharpening |
| **V2.1 Upscaled** | 10× AI-upscale som bevarar DM-estetiken |
| **V2.2 Modern** | Helt ny 3D-renderad 2D-konst |

Notera: `--scale-mode <n>` har kommandoradsalternativen `1=V1, 2=V2.1, 3=V2.2` — **V2.0 saknas i CLI-mappningen idag** (rad 101). Det är en lucka som ska stängas i denna plan.

### Befintlig V2-infrastruktur (redan på plats)

- `include/dm1_v2_presentation_profile_pc34.h` — `DM1_V2_PresentationMode { V1_ORIGINAL=0, V2_SHELL=1 }`, profil-struct, snapshot, tick.
- `include/dm1_v2_settings_pc34.h` — `DM1_V2_Settings` med `viewport_scale`, `use_epx`, `use_bilinear`, `palette_enhanced`, `aspectMode` m.fl.
- `include/config_m12.h` — `M12_Config.dm1V2*`-fält (`ScalePercent`, `SmoothingEnabled`, `DynamicLightingEnabled`, `AccessibilityTouchEnabled`, `AspectMode`).
- `src/dm1v2/dm1_v2_*.c` — ~46 V2-moduler (achievements, journal, minimap, particle, viewport_renderer, presentation_profile, settings, lighting_dynamic, viewport_renderer m.fl.).
- `src/ui/menu_startup_m12.c` (rad 102–168) — `m12_ext_settings[]` har redan **placeholder-rader** för V2.0 i GRAPHICS-fliken:
  - `"CRT Filter"` (V2.0, enabled=0)
  - `"Palette Correction"` (V2.0, enabled=0)
  - `"Dither Cleanup"` (V2.0, enabled=0)
  - `"Sharpening"` (V2.0, enabled=0)

Dessa rader är synliga men cycle-funktionen är `enabled=0`. **De ska bli `enabled=1` när V2.0 är klart.**

### Befintlig SDL-pipeline (M11)

- `src/engine/render_sdl_m11.c` (1191 rader) — single global `M11_RenderState`. Pipeline:
  1. Spelkod ritar i `framebuffer[]` (320×200, 1 byte/pixel: 4-bit palettindex + 4-bit per-pixel-level).
  2. `m11_framebuffer_to_rgba()` expanderar via `G9010_auc_VgaPaletteAll_Compat[level][idx]` → `presentBuffer[]` (RGBA8888, exakt fb-storlek).
  3. `SDL_UpdateTexture()` → streaming-textur (`SDL_PIXELFORMAT_RGBA32`).
  4. `SDL_RenderTexture(renderer, texture, src, dest)` med `destRect` beräknad av `M11_Render_ComputePresentationRect()` (4:3 / 16:9 / Content-aspect, integer scaling, fit/stretch).
  5. `SDL_SetTextureScaleMode(NEAREST | LINEAR)` styr GPU-filter.

Det här är vår enda hook-punkt för V2.0-filtren. Post-processing måste ske antingen mellan steg 2 och 3 (CPU-baserat, på `presentBuffer`) eller via en sekundär render-target (GPU-shader-väg).

### ReDMCSB-källkod — finns originalmotsvarighet?

- `PALETTE.C` (453 rader): hanterar VGA-DAC och dimningsnivåer (`G0010_aab_PalCh*`). **Ingen CRT-emulation, ingen sharpening, ingen scanline-logik.**
- `VIDEODRV.C` (4003 rader): VGA-drivrutin via inline-assembly mot CRTC-portar 0x3D4/0x3B4. Pratar med riktig hårdvara, inte med en emulerad CRT.
- `_MAIN.C`, `VDEOMAIN.C`, `VIDSET.C`: små shim-filer, inget post-process.

**Slutsats: V2.0 är rent Firestaff-arbete.** Det finns ingen sourcelåst CRT/scanline/sharpening i originalkällkoden. Däremot är palette-korrigeringen (gamma-justering av VGA-paletten) en *tolkning* av VGA→sRGB och kan motiveras med referens till `G9010_auc_VgaPaletteAll_Compat[]` (våra egna palette-tabeller) plus välkänd VGA-gamma (~2.2 PC-CRT).

---

## DEL 1 — DM1 V2.0 Detaljerad Plan

### 1.1 Exakt definition av V2.0

V2.0 = **V1 gameplay-route + V1 pixelinnehåll + post-process-filterkedja**. Inget gameplay ändras, ingen ny grafik laddas. Endast `presentBuffer[]` (eller en sekundär render-target) modifieras före `SDL_RenderTexture`.

Filterkedja (i denna ordning, för att matcha CRT-fysik):

| Steg | Filter | Default | Sourcestatus |
|------|--------|---------|--------------|
| A | Palette correction (gamma 1.0 → 2.2-justering + brightness/contrast) | Off | Firestaff (motiveras av VGA-DAC ~6-bit RGB) |
| B | Dither cleanup (3×3 mode-filter på indexpixlar **före** RGBA-expansion) | Off | Firestaff |
| C | Sharpening (unsharp mask 3×3 på RGBA efter expansion) | Off | Firestaff |
| D | CRT scanlines (varannan rad multipliceras med 0.5–0.85) | Off | Firestaff |

(Vignette/bloom/temperatur sparas till V2.0.5 eller V2.1 — se DEL 2.)

### 1.2 Arkitektur — var i pipeline V2.0 sitter

```
spelkod ──► framebuffer (320×200, idx)
              │
              ▼  (steg B körs här om aktivt)
        [dither_cleanup_indexed()]
              │
              ▼
        framebuffer_to_rgba()  (steg A: använder palette_corrected[][] istället för Vga-tabell)
              │
              ▼  presentBuffer (320×200, RGBA8888)
              │
              ▼  (steg C körs här om aktivt)
        [unsharp_mask_rgba()]
              │
              ▼  (steg D körs här om aktivt)
        [crt_scanlines_rgba()]
              │
              ▼
        SDL_UpdateTexture ► SDL_RenderTexture (med NEAREST eller LINEAR)
```

**Designprinciper:**
- **Filterkedjan är CPU-baserad i V2.0.** Inga shaders — Firestaff stödjer SDL3/SDL2 och vi vill undvika GLSL/HLSL/MSL-versioner för en sak som körs ~60 FPS på 320×200 = 64k pixlar. Mätbar CPU-kostnad är försumbar.
- **Allt sker i `render_sdl_m11.c`** i en ny intern hjälpfunktion `m11_apply_v2_filters()` som anropas precis före `SDL_UpdateTexture`.
- **State läses från `M12_Config.dm1V2*`-flaggor** (nya bitar — se 1.3) via en single setter `M11_Render_SetV2Filters(...)`.
- **V1-pathen är opåverkad.** När `presentationMode == V1_ORIGINAL` hoppar `m11_apply_v2_filters()` ut tidigt.
- **Per-spel-flagga.** V2.0 ska kunna slås på/av per spel via `M12_Config.gameVersionIndex[]` × graphicsIndex; för nu räcker det med en global runtime-flagga som binds till DM1.

### 1.3 Nya config-fält (config_m12.h)

Lägg till efter rad 76 (`dm1V2AspectMode`):

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

Defaults sätts i `M12_Config_SetDefaults()` så att V2.0 startar med ALLA filter `0` (off) — exakt V1 utseende, så att A/B-test går rakt på.

### 1.4 Nya render_sdl_m11 API (render_sdl_m11.h)

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

Backas av nya fält i `M11_RenderState`:

```c
int v2_crt_enabled, v2_crt_strength;
int v2_palette_enabled, v2_palette_gamma100, v2_palette_brightness, v2_palette_contrast;
int v2_dither_enabled;
int v2_sharpen_enabled, v2_sharpen_strength;
unsigned char v2_palette_corrected[M11_PALETTE_LEVELS][16][3]; /* förberäknad LUT */
```

### 1.5 Nya filer (src/dm1v2/)

| Fil | Innehåll | Source-evidence |
|-----|----------|------------------|
| `src/dm1v2/dm1_v2_filter_palette_correct.c` | `dm1_v2_filter_palette_build_lut(gamma100, bright, contrast, out_lut)` — bygger en `[levels][16][3]` LUT som ersätter `G9010_auc_VgaPaletteAll_Compat[]` i `framebuffer_to_rgba()` när aktiv. | Firestaff; refererar `G9010_auc_VgaPaletteAll_Compat`. |
| `src/dm1v2/dm1_v2_filter_dither_cleanup.c` | `dm1_v2_filter_dither_cleanup_indexed(uint8_t* fb, int w, int h)` — 3×3 mode-filter över indexbyte; sparar level-bits intakt; hoppar över överlay-/UI-regioner. | Firestaff. |
| `src/dm1v2/dm1_v2_filter_sharpen.c` | `dm1_v2_filter_sharpen_rgba(uint8_t* rgba, int w, int h, int strength_pct)` — 3×3 unsharp mask, separabel approximation. | Firestaff. |
| `src/dm1v2/dm1_v2_filter_crt_scanlines.c` | `dm1_v2_filter_crt_scanlines_rgba(uint8_t* rgba, int w, int h, int strength_pct)` — varannan rad multipliceras med (1 − s/100). | Firestaff. |
| `include/dm1v2/dm1_v2_filters.h` | Aggregat-header. | — |

### 1.6 Wiring i render_sdl_m11.c

Lägg till `m11_apply_v2_filters()` mellan `m11_framebuffer_to_rgba()` och `SDL_UpdateTexture()` i `M11_Render_Present()` (kring rad 670–705). Ordning:

1. Om `v2_dither_enabled`: kör `dm1_v2_filter_dither_cleanup_indexed()` på `framebuffer[]` *före* `framebuffer_to_rgba`.
2. `framebuffer_to_rgba()` — om `v2_palette_enabled`, använd `v2_palette_corrected[][]` istället för `G9010_auc_VgaPaletteAll_Compat[][]`.
3. Om `v2_sharpen_enabled`: kör `dm1_v2_filter_sharpen_rgba()` på `presentBuffer[]`.
4. Om `v2_crt_enabled`: kör `dm1_v2_filter_crt_scanlines_rgba()` på `presentBuffer[]`.

### 1.7 Wiring i menu_startup_m12.c

- Rad 121–124: ändra `enabled=0` → `enabled=1` för CRT Filter, Palette Correction, Dither Cleanup, Sharpening.
- Lägg till cycle-handlers i `m12_cycle_game_opt_with_mode()` / `m12_cycle_ext_setting()` som mappar mot de nya `dm1V2*`-config-fälten.
- Lås filterraderna när `presentationModeIndex == V1_ORIGINAL` via `M12_GameOptions_RowLockedByMode()` (samma mekanik som redan låser V2.1/V2.2-rader).

### 1.8 CLI-wiring

- `firestaff_cli.c`: `--scale-mode 2` är idag V2.1 enligt README. **Ändra mappningen:**
  - `1` = V1 Original
  - `2` = V2.0 Filtered (nytt — eller alias till V1+default-filter-preset)
  - `3` = V2.1 Upscaled (var tidigare 2)
  - `4` = V2.2 Modern (var tidigare 3)
- Uppdatera README rad 101 + 115 samtidigt (eller behåll `--scale-mode 2 = V2.1` och introducera `--scale-mode 5 = V2.0`; beslut tas vid implementationsstart för att inte bryta existerande dokumentation/skript).

### 1.9 Implementation milestones

🔲 **M1 — Config-skelett** (1–2 timmar)
- 🔲 Lägg till de 9 nya `dm1V2*`-fälten i `M12_Config`.
- 🔲 Uppdatera `M12_Config_SetDefaults()` med default-värden.
- 🔲 Verifiera att `M12_Config_Save()` skriver fälten och `M12_Config_Load()` läser dem (de använder generisk INI-loop, behöver oftast bara mappning).
- 🔲 Bygg + kör `tests/test_m12_config_*` så inga existerande config-tester går sönder.

🔲 **M2 — Filter-LUT-byggare (palette correction)** (2 timmar)
- 🔲 Skapa `src/dm1v2/dm1_v2_filter_palette_correct.c` med pure-C LUT-byggare.
- 🔲 Enhetstest: `tests/test_dm1_v2_filter_palette_correct.c` — kontrollera att gamma=2.20 lyfter mörka färger, gamma=1.00 är identitet inom ±1.
- 🔲 Source-evidence-kommentar refererande `G9010_auc_VgaPaletteAll_Compat`.

🔲 **M3 — Dither cleanup (indexerad mode-filter)** (2 timmar)
- 🔲 Skapa `dm1_v2_filter_dither_cleanup.c`. 3×3 mode-fönster på indexbyte. Skydda level-bits.
- 🔲 Enhetstest: bygg en 320×200 testbild med checkerboard-dither, verifiera att resultatet är dominerande färg.

🔲 **M4 — Sharpening (unsharp mask)** (2 timmar)
- 🔲 Skapa `dm1_v2_filter_sharpen.c`. Separabel 3×3 box-blur → original − blur × strength.
- 🔲 Enhetstest: en horisontell stegfunktion ska bli skarpare; en flat region oförändrad.

🔲 **M5 — CRT scanlines** (1 timme)
- 🔲 Skapa `dm1_v2_filter_crt_scanlines.c`. Varannan rad RGB *= (1 − s/100).
- 🔲 Enhetstest: udda rader oförändrade, jämna rader skalade enligt strength.

🔲 **M6 — Render-pipeline-integration** (3 timmar)
- 🔲 Lägg till `M11_Render_SetV2Filters()` / `GetV2Filters()` API i `render_sdl_m11.h/.c`.
- 🔲 Hooka in i `M11_Render_Present()` (steg-ordning enl. 1.6).
- 🔲 Bygg LUT vid set-anropet, inte per frame.
- 🔲 Smoke-test: kör `firestaff --scale-mode 2 --duration 5000` med och utan flaggor.

🔲 **M7 — Menu-wiring** (2 timmar)
- 🔲 Aktivera `enabled=1` på CRT/Palette/Dither/Sharpening i `m12_ext_settings[]`.
- 🔲 Lägg till cycle-handlers i `menu_startup_m12.c`.
- 🔲 Lås rader när V1-presentation är vald.
- 🔲 Verifiera via screenshot-test (jämför `verification-screens/` baseline).

🔲 **M8 — CLI + README** (1 timme)
- 🔲 Bestäm scale-mode-mappning (se 1.8).
- 🔲 Uppdatera `firestaff_cli.c`.
- 🔲 Uppdatera `README.md` rad 101/115/119–123.
- 🔲 Lägg till `RELEASE_2.5.0.md` (eller liknande).

🔲 **M9 — Live runtime toggle** (1 timme)
- 🔲 Säkerställ att alla 4 filter kan slås på/av utan restart (config → SetV2Filters → nästa frame visar effekt).

🔲 **M10 — Regressionsskydd & screenshots** (2 timmar)
- 🔲 Lägg in PNG-baselines i `verification-screens/dm1_v2_filters/`: off, CRT_only, palette_only, dither_only, sharpen_only, all.
- 🔲 Lägg till test som tar screenshot via `firestaff --duration 0 --script enter` och jämför mot baseline.

**Total uppskattning: ~16 timmar fokuserat arbete.**

### 1.10 Beroenden

| Beroende | Status | Vad behövs först |
|----------|--------|-------------------|
| V1 viewport (DM1) | ✅ Klart (entry-view, walls, doors, ESC-dialog) | — |
| V1 framebuffer-pipeline | ✅ Klart (M11 render path) | — |
| V1 palette (VgaPaletteAll_Compat) | ✅ Klart | — |
| M12_Config persistens | ✅ Klart | — |
| V2 presentation shell (V2_SHELL mode) | ✅ Klart (presentation_profile_pc34) | — |
| V2 viewport renderer | ⚠ Delvis (rendererar i egen `DM1_V2_ViewportState`, ej kopplad till V1 fb än) | Inte blockerande — V2.0 läser V1 fb direkt |

**Inga blockare.** Hela V2.0 kan börja idag.

### 1.11 ReDMCSB-källlåsning

**V2.0 är rent Firestaff-arbete.** Inga sourcelåsta element. Detta måste dokumenteras tydligt i varje ny fil:

```c
/* Source: Firestaff V2.0 filtered presentation.
 * No ReDMCSB original equivalent — ReDMCSB targets raw VGA DAC output
 * via real-hardware CRTC port programming (VIDEODRV.C). V2.0 emulates
 * the perceived CRT look on modern flat-panel displays. */
```

Detta är viktigt: AGENTS.md säger "source-locked" är ett kärnvärde. V2.0 är opt-in och flaggad som ren presentationsskikt, V1 gameplay-route är fortsatt 100 % source-locked.

### 1.12 Testplan

🔲 **Build-verifiering**
```bash
cd /home/trv2/work/firestaff
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```
🔲 **Enhetstester per filter** — separata `tests/test_dm1_v2_filter_*.c`.
🔲 **Smoke-test V1 oförändrad** — kör `firestaff --scale-mode 1` och jämför mot baseline-screenshot.
🔲 **Smoke-test V2.0 off-by-default** — kör `firestaff --scale-mode 2` utan filter-flaggor → identiskt med V1.
🔲 **Filter-on/off-toggling i runtime** — använd `--script` för att tabba in i GRAPHICS-menyn och toggla.
🔲 **Performance-mätning** — `--fps` flag på 320×200; samtliga filter aktiva får inte sänka under 60 FPS på Steam Deck-klass-hårdvara.
🔲 **Pixel-jämförelse** — `parity-evidence/dm1_v2_filters/` med PNG-diff för varje filter solo.

Verifieringskommando:
```bash
cmake --build /home/trv2/work/firestaff/build --parallel && \
  /home/trv2/work/firestaff/build/firestaff --scale-mode 1 --duration 2000 && \
  /home/trv2/work/firestaff/build/firestaff --scale-mode 2 --duration 2000
```

---

## DEL 2 — SDL-grafikinställningar (nya features)

### 2.1 Förslag på inställningar

| # | Inställning | Var | Typ | Implementation |
|---|-------------|-----|-----|----------------|
| 1 | CRT Scanlines (V2.0) | M12_Config + GUI | On/Off + 0–100 strength | CPU `dm1_v2_filter_crt_scanlines.c` |
| 2 | Palette Correction (V2.0) | M12_Config + GUI | On/Off + gamma/bright/contrast | CPU LUT |
| 3 | Dither Cleanup (V2.0) | M12_Config + GUI | On/Off | CPU 3×3 mode-filter |
| 4 | Sharpening (V2.0) | M12_Config + GUI | On/Off + 0–100 strength | CPU 3×3 unsharp |
| 5 | Interpolation Filter | M12_Config (finns) + GUI | Nearest / Linear (SDL har båda) | `SDL_SetTextureScaleMode` (redan implementerad) |
| 6 | Vignette | M12_Config + GUI | On/Off + radius | CPU enkel — `dm1_v2_filter_vignette.c` |
| 7 | Bloom / Glow | M12_Config + GUI | On/Off + threshold/strength | CPU — bright-pass + 5×5 blur; kostsam |
| 8 | Color Temperature | M12_Config + GUI | Slider −100..+100 (cool↔warm) | Inkluderas i palette LUT |
| 9 | Integer Scale | M12_Config (finns) | On/Off | Redan i `render_sdl_m11.c` |
| 10 | Aspect Ratio | M12_Config (finns) | 4:3 / 16:9 / Content | Redan i `render_sdl_m11.c` |
| 11 | CRT Curvature | M12_Config + GUI | On/Off + curve | **Shader-only** (skjuts till V2.x eller framtida shader-väg) |
| 12 | NTSC Composite Artifact | M12_Config + GUI | On/Off | **Shader-only** (skjuts) |

### 2.2 GUI-design

I `m12_ext_settings[]` på GRAPHICS-fliken, lägg V2.0-rader **direkt efter** `Palette Mode` (rad 119):

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
{"Bloom",                "Off",   0, M12_SETTINGS_TAB_GRAPHICS},  /* V2.0.5 — kostsam */
```

UI-pattern:
- **Toggle** (On/Off): vänster/höger pil cyklar.
- **Slider** (procent/gamma): vänster/höger pil ändrar i steg om 5 (snabb-step med shift).
- **Sub-menu**: nej. Slider-rader är lika lätta att förstå inline och konsistenta med existerande Volume-rader.
- **Lock when V1 vald**: rader med kommentar `/* V2.0 */` låses (gråtonas) när `presentationModeIndex == V1_ORIGINAL`, via samma mekanism som idag låser V2.2-rader.

### 2.3 Vilka kräver shader vs CPU

| Filter | CPU OK? | Shader bättre? | Beslut för V2.0 |
|--------|---------|---------------|-----------------|
| CRT scanlines | ✅ trivial | ✅ snyggare med gauss-pulse | CPU |
| Palette correction | ✅ LUT en gång | gradient i shader | CPU |
| Dither cleanup | ✅ måste vara CPU (indexerad) | nej | CPU |
| Sharpening | ✅ 3×3 räcker | ✅ för 5×5+ | CPU |
| Vignette | ✅ trivial | ✅ snyggare med radial gauss | CPU |
| Bloom | ⚠ tungt på CPU | ✅ shader | SKJUTS till V2.0.5 |
| CRT curvature | nej | ✅ kräver shader | SKJUTS |
| NTSC composite | ⚠ tungt | ✅ shader | SKJUTS |
| Color temperature | ✅ ingår i palette-LUT | ja | CPU |
| Interpolation | redan SDL | redan SDL | klart |
| Integer scale | redan M11 | — | klart |
| Aspect | redan M11 | — | klart |

### 2.4 Shader-vägen (för V2.0.5+, ej för V2.0)

Om Firestaff senare vill ha CRT curvature/NTSC/bloom så krävs:
- Sekundär render-target (RT) via `SDL_CreateTexture(SDL_TEXTUREACCESS_TARGET)`.
- Egen shader-pipeline. SDL3 har inte inbyggt shader-API på samma sätt som SDL_gpu eller bgfx — typiskt ramverk är:
  - `SDL_RenderGeometry` med custom textures + flera passes.
  - Eller backe SDL3 + GLSL via `SDL_GPUDevice` (SDL3 v3.2+ har detta).
- Beslut: **inte i V2.0**. Markera som framtida arbete i `TODO.md`.

### 2.5 Config-fält för shader-vägen (förberedande)

Stub:t i config men `enabled=0` i GUI tills shader-pipeline finns:

```c
int dm1V2VignetteEnabled;
int dm1V2VignetteStrength;     /* 0–100, default 25 */
int dm1V2ColorTemperature;     /* −100..+100, default 0 */
int dm1V2BloomEnabled;         /* off i V2.0; framtida */
int dm1V2CrtCurvatureEnabled;  /* shader-only; framtida */
```

---

## DEL 3 — ReDMCSB source-låsning för V2

### 3.1 Genomgång av relevanta källkodsfiler

| Fil | Innehåll | Relevans för V2.0 |
|-----|----------|-------------------|
| `PALETTE.C` (453 rader) | VGA-DAC-skrivning, palette-tabeller, dimningsnivåer | Bas för `G9010_auc_VgaPaletteAll_Compat` (redan i Firestaff). Ingen post-process. |
| `VIDEODRV.C` (4003 rader) | VGA-port-driver (inline ASM mot 0x3D4/0x3B4) | **Inget post-process** — pratar direkt med riktig hårdvara. Endast en CRT-relaterad träff: kommentar om VGA CRTC controller port. |
| `STARTEND.C` | Saknas i denna ReDMCSB-snapshot | — |
| `_MAIN.C`, `VDEOMAIN.C`, `VIDSET.C` | Små shim-filer | Ingen filter-hook. |
| `FILTERS.C` | Finns inte | — |
| `EVENT.C` | Hittas inte i grep — kan finnas men inga filter-träffar | — |

### 3.2 Vad ReDMCSB gör med bit-depth/palette

ReDMCSB skriver 4-bit/pixel till VGA Mode 13h linear buffer (eller motsvarande). Paletten skrivs som **6-bit per kanal** till VGA DAC. Det här hanterar Firestaff redan via `G9010_auc_VgaPaletteAll_Compat[level][idx]` (16 paletter à 16 färger, RGB888-utvidgade). Ingen ytterligare bit-depth-konvertering behövs för V2.0.

### 3.3 Slutsats: source-låsning

- **V1 gameplay/render path:** 100 % source-locked mot ReDMCSB. Inget ändras.
- **V2.0 filter chain:** **Firestaff-originellt arbete.** Ingen ReDMCSB-motsvarighet finns. Detta dokumenteras i varje ny `dm1_v2_filter_*.c` med source-evidence-kommentar (se 1.11).
- **V2-presentation profile** (`presentation_profile_pc34.c`): redan på plats, `presentationMode == V2_SHELL` är hooken där V2.0 aktiveras.

Det här är samma policy som redan gäller för V2.1 (AI upscale) och V2.2 (modern art): **gameplay = source-locked, presentation = opt-in nytt arbete.**

---

## 4. Prioritetsordning (vad göra först)

1. 🔲 **M1 Config** — alla nio nya fält + defaults. Inget annat kan börja förrän M12_Config kan persistera.
2. 🔲 **M2 Palette LUT** — enskilt mest synbar effekt; lätt att verifiera med PNG.
3. 🔲 **M5 CRT Scanlines** — trivialt att implementera, ger "wow"-effekt direkt.
4. 🔲 **M4 Sharpening** — bygger förtroende för pipeline-flödet.
5. 🔲 **M3 Dither Cleanup** — kräver mer omsorg pga indexerad data.
6. 🔲 **M6 Pipeline Integration** — hookar ihop allt; här blir det "live".
7. 🔲 **M7 Menu Wiring** — gör features upptäckbara.
8. 🔲 **M8 CLI + README** — dokumentation.
9. 🔲 **M9 Runtime Toggle** — säkerställ no-restart.
10. 🔲 **M10 Regression Screenshots** — låser klart-läget.

---

## 5. Verifieringskommando

```bash
cd /home/trv2/work/firestaff && \
  cmake -B build -DCMAKE_BUILD_TYPE=Release && \
  cmake --build build --parallel && \
  ./build/firestaff --scale-mode 1 --duration 2000 && \
  ./build/firestaff --scale-mode 2 --duration 2000
```

Förväntat: båda runs avslutas med exit code 0, ingen segfault, ingen visuell regression i V1, V2.0 visar identisk bild som V1 när alla filter är `Off` (default).

---

## 6. Risker & öppna frågor

- **Scale-mode-mappning:** `--scale-mode 2` betyder idag V2.1. Om vi mappar 2→V2.0 bryter vi exempel i README. Förslag: behåll 2=V2.1, lägg V2.0 som `--scale-mode 5` ELLER introducera `--graphics v1|v2.0|v2.1|v2.2`-namngiven flagga. **Beslut tas vid M8-start.**
- **CRT-emulation kvalitetsförväntan:** Daniel kan ha en specifik referensbild (CRT Royale, Reshade m.fl.) — be om referens innan M5 spikas.
- **Performance på ARM64 (Steam Deck):** alla fyra filter aktiva på 320×200 är ~64k pixlar × 4 RGBA passes = försumbart. Borde inte sänka FPS. Mätning sker i M10.
- **Per-spel-flagga:** V2.0 är DM1-specifikt i denna plan. CSB/DM2/Nexus får egna V2.0 senare. Configfält ska heta `dm1V2*` och inte återanvändas av andra spel.

---

## 7. Filmanifest — vad ändras

**Nya filer (10):**
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

**Ändrade filer (6):**
- `include/config_m12.h` (+9 fält)
- `src/config_m12.c` (defaults, load/save mapping)
- `include/render_sdl_m11.h` (+ SetV2Filters/GetV2Filters API)
- `src/engine/render_sdl_m11.c` (+ state, + apply_v2_filters hook)
- `src/ui/menu_startup_m12.c` (+ rader 121–124 enabled=1, + nya rader, + cycle handlers)
- `src/firestaff_cli.c` (+ scale-mode mappning eller --graphics flagga)
- `CMakeLists.txt` (+ nya .c-filer)
- `README.md` (+ V2.0-rad och CLI-uppdatering)

---

*Plan skriven av subagent på N2 / Firestaff-Worker-VM 2026-05-26. Inga ändringar i `src/` gjorda i denna pass — endast denna plan-fil.*
