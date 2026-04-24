# Parity Matrix — DM1 V1 (Original-Compatible)

Last updated: 2026-04-22

Target: **Dungeon Master 1, PC DOS, VGA, English — 1:1 original-faithful behavior and presentation.**

This is the DM1 slice of the V1 parity ledger. CSB and DM2 will get their own matrices when reference data is acquired.
V2/V3 differences must never appear in this matrix as accepted parity.

---

## Status labels

| Label | Meaning |
|-------|---------|
| `MATCHED` | Verified against original/reference evidence. Source-backed. |
| `KNOWN_DIFF` | Observed difference from original. Tracked as active work. |
| `UNPROVEN` | Not yet compared against original. May be correct, may not. |
| `BLOCKED_ON_REFERENCE` | Cannot verify — missing original reference data or capture. |
| `N/A` | Not applicable for this game target. |

---

## 1. Screen composition and layout (W2)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| Main game screen (320×200 VGA) | `VIDEODRV.C` confirms Mode 0x0D (320×200, 16-color planar) | Firestaff renders at 320×200 | `UNPROVEN` — no pixel-level overlay comparison yet | Capture original via emulator, overlay against Firestaff |
| Viewport region bounds | DEFS.H: `C112_BYTE_WIDTH_VIEWPORT`=112 (224px at 4bpp), `C136_HEIGHT_VIEWPORT`=136. Graphic #0 (`C000_DERIVED_BITMAP_VIEWPORT`) is 224×136, confirming exact match. Viewport uses color indices 16–31 via `G8177_c_ViewportColorIndexOffset=0x10`. | `m11_game_view.c` viewport enum: `M11_VIEWPORT_X=12, M11_VIEWPORT_Y=24, M11_VIEWPORT_W=196, M11_VIEWPORT_H=118` — measured 196×118 on framebuffer. | `KNOWN_DIFF` — Firestaff viewport is **−28 px narrower and −18 px shorter** than DM1 original (24.1 % less pixel area). Horizon row 59 vs. source 68. See `parity-evidence/pass33_viewport_coordinate_overlay.md` §3. | Land a Firestaff-side renderer change to paint the full 224×136 viewport, or ship the width/height as an explicit V1 known-diff with rationale. Tracked for pass-37+. |
| Party/champion region | `DEFS.H` constants: portrait 32×29 (`G2078`/`G2079`), atlas addressing `M027_PORTRAIT_X`/`M028_PORTRAIT_Y`, champion status-box spacing 69 px (`C69_CHAMPION_STATUS_BOX_SPACING`). | Firestaff: `M11_PORTRAIT_W=32, M11_PORTRAIT_H=29` (matches); `M11_PARTY_SLOT_STEP=77` (drift **+8 px** per slot). Atlas indexing `(i & 7)*32, (i >> 3)*29` matches. | `KNOWN_DIFF` — portrait identity `MATCHED`, slot horizontal stride off by +8 px per slot. See `parity-evidence/pass34_sidepanel_rectangle_table.md` §3. | Adjust `M11_PARTY_SLOT_STEP` to 69 to match C69, or carry forward as known V1 diff with rationale. Tracked for pass-37+. |
| Inventory screen | ReDMCSB source available | Firestaff has inventory screen (see `verification-screens/03_ingame_start.png`) | `UNPROVEN` | Side-by-side with original capture |
| Spell panel | DEFS.H `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND` (87×25), `C011_GRAPHIC_MENU_SPELL_AREA_LINES` (14×39). | Firestaff `M11_DM_SPELL_AREA_W=87, M11_DM_SPELL_AREA_H=25` matches the backdrop graphic; graphic indices 9 and 11 used via `M11_GFX_SPELL_AREA_*`. Placement (x,y) = (224, 90). | `MATCHED` (asset identity 87×25 + index 9; lines graphic 11) / `BLOCKED_ON_REFERENCE` (placement overlay). See `parity-evidence/pass34_sidepanel_rectangle_table.md` §3. | Pixel-overlay against ReDMCSB capture when ZONES.H / PANEL.C pixel walk lands. |
| Map overlay | Unknown — may not exist in DM1 V1 scope | Unknown | `BLOCKED_ON_REFERENCE` | Confirm if DM1 has map overlay |
| Dialog/endgame overlays | `dialog_frontend_pc34_compat.{c,h}`, `endgame_frontend_pc34_compat.{c,h}` exist | Compat layers exist but not visually verified | `UNPROVEN` | Capture original dialog frames |
| Title screen composition | Composed backdrop path verified; `dialogGraphicIndex=1` contains title card; held state at graphic 313 is pixel-stable | Title hold verified via `--title-hold` mode, pixel-identical across repeated frames | `UNPROVEN` — no direct original screenshot comparison | Capture original title screen via emulator for overlay |
| Menu screen | M9 beta harness reaches menu state; submenu matrix exists | Menu interaction verified via M9 verify gate; submenu drift removed | `UNPROVEN` — layout not measured against original | Measure original menu layout |

---

## 2. UI sprites and assets (W3)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| GRAPHICS.DAT decode coverage | 577/713 bitmaps decoded, 0 decode failures | Full extraction in `extracted-graphics-v1/` with manifest | `MATCHED` | — |
| Champion portraits | `G2078_C32_PortraitWidth=32`, `G2079_C29_PortraitHeight=29` (COORD.C), `M027_PORTRAIT_X`/`M028_PORTRAIT_Y` atlas math (DEFS.H). | Firestaff `M11_PORTRAIT_W=32, M11_PORTRAIT_H=29` and the exact same atlas math at m11_game_view.c:11670–11671. GRAPHICS.DAT entry `C026_GRAPHIC_CHAMPION_PORTRAITS`. | `MATCHED` — portrait pixel size and atlas addressing identical to source. See `parity-evidence/pass34_sidepanel_rectangle_table.md` §3. | — |
| Equipment/item icons | Present in GRAPHICS.DAT extraction | Unknown mapping status | `UNPROVEN` | Build GRAPHICS.DAT → component usage map |
| Rune glyphs | Present in GRAPHICS.DAT extraction | Unknown mapping status | `UNPROVEN` | Map glyph indexes to spell panel usage |
| Panel backgrounds/ornaments | Front-lock assets identified: 303–320 (walls-ornate category) | `by-category/walls-ornate/` symlinks exist | `UNPROVEN` — usage not verified against original screen composition | Map ornament assets to panel placement |
| Title-side UI assets | DEFS.H defines: C001_GRAPHIC_TITLE(320×200), C002_ENTRANCE_LEFT_DOOR(105×161), C003_ENTRANCE_RIGHT_DOOR(128×161), C004_ENTRANCE(320×200), C005_CREDITS(320×200), C006_THE_END(80×14). All 6 extracted as BITMAP_SAFE with 0 decode failures. Symlinked in `by-category/title-ui/`. | Used in title boot sequence (M6/M7 verified). Asset-to-define mapping complete. | `MATCHED` (asset identification and extraction) / `UNPROVEN` (pixel-level rendering comparison) | Pixel-compare rendered title/entrance/credits against original emulator captures. See §E6. |
| Placeholder entries | 131 placeholders identified and cataloged | Correctly skipped during decode/render | `MATCHED` | — |
| Text-tag fallbacks | — | Unknown if V1 still uses text fallbacks where graphics should be | `UNPROVEN` | Audit all V1 screens for text-where-graphics-should-be |

---

## 3. VGA palette and color (W3 adjacent)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| Base palette (16 colors) | Verified from `VIDEODRV.C` source — custom VGA DAC, NOT EGA. 16 unique colors including Cyan(idx4), Brown(idx3), Tan(idx10), etc. | `palette-recovery/recovered_palette.json` has correct data; **`vga_palette_pc34_compat.c` uses wrong EGA-style palette** (14/16 colors differ). Used by `render_sdl_m11.c` and PPM export. | `MATCHED` (data extraction) / `KNOWN_DIFF` (rendering — EGA palette, not VGA) | Replace EGA palette in compat layer with VIDEODRV.C values from `recovered_palette.json`. See `parity-evidence/dm1_v1_pass1_palette_and_assets.md` §E1. |
| Brightness levels (LIGHT0–5) | All 6 levels with per-color values extracted from `VIDEODRV.C` (G8151–G8156). NOT linearly attenuated — each level has independently tuned values. | In `recovered_palette.json` (correct); **compat layer uses `rgb * (5-N)/5` linear model** (wrong). Level 5 is all-black in compat but original has 8 non-zero colors. | `MATCHED` (data) / `KNOWN_DIFF` (rendering — linear attenuation, not original lookup table) | Replace linear model with per-level lookup from `recovered_palette.json`. See §E2. |
| Creature palettes | 14 creature palettes (G8175_CREAT_PAL) extracted from `VIDEODRV.C` — 14 types × 6 replacement colors (indices 1–6) | In `recovered_palette.json` with full VGA6 data; **no creature palette support found in `vga_palette_pc34_compat.{c,h}`** | `MATCHED` (data) / `KNOWN_DIFF` (rendering — not integrated into compat layer) | Wire creature palettes from `recovered_palette.json` into rendering path. See §E5. |
| Cyan invariant (idx 4) | Confirmed invariant: VGA6 (0,54,54) = RGB8 (0,219,219) across all 6 brightness levels in `recovered_palette.json` (verified from VIDEODRV.C) | **Firestaff maps idx 4 to Dark Red (170,0,0)** — EGA palette error. Cyan color does not exist in compat layer. `test_vga_palette_pc34_compat.c` explicitly asserts idx 4 = (170,0,0). Linear brightness model also zeros it at level 5. | `KNOWN_DIFF` — cyan invariant completely violated | Fix base palette (idx 4 → Cyan), fix brightness model to preserve invariant at all levels. See §E3. |
| Special palettes (credits, entrance, swoosh) | CREDITS (G8147, 16 colors) and ENTRANCE (G8148, 16 colors) fully extracted from `VIDEODRV.C` lines 62–117 (VGA section). Custom warm/outdoor tones. | **Not implemented.** No references to G8147/G8148 or special palette switching in `vga_palette_pc34_compat.{c,h}`. Credits/entrance screens render with (wrong) base palette only. | `KNOWN_DIFF` — special palettes not implemented | Add CREDITS and ENTRANCE palette arrays to compat layer; wire palette switching for credits/entrance screens. See §E4. |
| Falsecolor vs. true-color | Current `ppm-falsecolor/` exports are inspection artifacts, not claimed final RGB | Firestaff PPM screenshots exist in `verification-screens/`. **Root cause identified:** compat layer palette is EGA-based, not VGA. Even "true-color" PPM output uses wrong colors. | `KNOWN_DIFF` — root cause is EGA palette in compat layer (see §E1) | Fix base palette first, then re-export and compare. |

---

## 4. Typography and text (W4)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| Player-visible strings | Not yet inventoried from original | Not yet inventoried from Firestaff | `UNPROVEN` | Extract all strings from both, compare |
| Status text labels | ReDMCSB source available | Unknown parity | `UNPROVEN` | Cross-reference with source |
| Spell labels | ReDMCSB source available | Unknown parity | `UNPROVEN` | Cross-reference with source |
| Inventory labels | ReDMCSB source available | Unknown parity | `UNPROVEN` | Cross-reference with source |
| Message log text | ReDMCSB source available | Unknown parity | `UNPROVEN` | Cross-reference with source |
| Dialog/plaque text | `DUNGEON.C` source available | Unknown parity | `UNPROVEN` | Extract original strings from `DUNGEON.C` |
| Over-labeling (invented strings in V1) | Original had minimal text | Unknown if Firestaff adds helper/debug text | `UNPROVEN` | Audit all V1 screens |
| Font rendering | Original font data in GRAPHICS.DAT | Unknown parity | `UNPROVEN` | Compare rendered glyphs |

---

## 5. Gameplay behavior (W5)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| Movement and turning | `DUNGEON.C` source available | `verification-m10/movement-champions/` suite exists | `UNPROVEN` — suite tests internal consistency, not original-match | Add original-behavior-backed test cases |
| Collision and doors | `DUNGEON.C` source available | `verification-m10/dungeon-doors-sensors/` suite exists | `UNPROVEN` | Add original-backed cases |
| Combat and attack timing | `DUNGEON.C` source available | `verification-m10/combat/` suite exists | `UNPROVEN` | Add original-backed cases |
| Creature AI/movement | `DUNGEON.C` source available | `verification-m10/creature-ai/` suite exists | `UNPROVEN` | Add original-backed cases |
| Item pickup/drop/use | `DUNGEON.C` source available | `verification-m10/dungeon-monsters-items/` suite exists | `UNPROVEN` | Add original-backed cases |
| Spell input and effects | `DUNGEON.C` source available | `verification-m10/magic/` suite exists | `UNPROVEN` | Add original-backed cases |
| Pits, teleporters, stairs | `DUNGEON.C` source available | `verification-m10/dungeon-tiles/` suite exists | `UNPROVEN` | Add original-backed cases |
| Food/water/rest/death/victory | `DUNGEON.C` source available | `verification-m10/champion-lifecycle/` suite exists | `UNPROVEN` | Add original-backed cases |
| Save/load state | `DUNGEON.C` source available | `verification-m10/savegame/` suite exists | `UNPROVEN` | Roundtrip test with original save files |
| Sensor execution | `MOVESENS.C` in `phase11-ref/`, sensor catalog in `phase11-fixtures/` | `verification-m10/sensor-execution/` suite exists | `UNPROVEN` | Cross-reference with original source |
| Projectile behavior | `DUNGEON.C` source available | `verification-m10/projectile/` suite exists | `UNPROVEN` | Add original-backed cases |
| Tick orchestration | `DUNGEON.C` source available | `verification-m10/tick-orchestrator/` suite exists | `UNPROVEN` | Compare tick cadence with original |
| Dungeon text rendering | `DUNGEON.C` source available | `verification-m10/dungeon-text/` suite exists | `UNPROVEN` | Compare with original text display |

---

## 6. Timing, animation, sequencing (W6)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| Idle animation cadence | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture original via emulator with frame timing |
| Attack cue duration | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
| Damage flash duration | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
| Message timing | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
| Input responsiveness | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
| Spell sequence pacing | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
| Door open/close sequencing | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
| Title/menu boot sequence timing | M9 boot sequence verified structurally; no wall-clock timing comparison | Boot reaches held title state reliably | `UNPROVEN` | Compare boot sequence duration |

---

## 7. Audio (W7)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| Sound trigger points | No original capture | `verification-m11/audio/` suite exists | `UNPROVEN` | Map original sound events |
| Sound samples (content) | `SONG.DAT` exists in original package; extraction not attempted | Procedural placeholders only | `KNOWN_DIFF` | Investigate SONG.DAT format and extraction |
| Sound cadence/overlap | No original capture | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator with audio |
| Music | `SONG.DAT` exists | Unknown if Firestaff has music | `UNPROVEN` | Investigate |

---

## 8. Bug-profile and version fidelity (W8)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| Target version profile | DM PC 3.4 English is the target | `BUGFIX_TOGGLE_SPEC.md` exists | `UNPROVEN` — no specific bugs cataloged yet | Build parity bug ledger from ReDMCSB/CSBwin notes |
| Original bugs preserved | Not yet cataloged | Unknown | `UNPROVEN` | Catalog known DM1 bugs |
| Patched behavior options | Not yet cataloged | `BUGFIX_TOGGLE_SPEC.md` framework exists | `UNPROVEN` | Populate toggle spec with real entries |

---

## 9. Regression infrastructure (W9)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| M9 verification gate | — | `run_redmcsb_m9_verify.sh` exists and passes | `MATCHED` (infrastructure exists) | Expand coverage |
| M10 semantic suites | — | 20 sub-suites in `verification-m10/` | `MATCHED` (infrastructure exists) | Add original-backed test cases |
| M11 verification | — | 6 sub-suites in `verification-m11/` | `MATCHED` (infrastructure exists) | Expand |
| Screenshot regression baseline | — | 11 captures in `verification-screens/` | `UNPROVEN` — not compared to original | Need golden originals |
| Submenu/interaction matrix | — | `submenu-matrix/`, `roundtrip-matrix/`, `internal-matrix/` with PGM captures and invariant docs | `MATCHED` (infrastructure) | Gate parity claims on these |
| Automated image comparison | — | Not yet built | `KNOWN_DIFF` | Build tolerance-aware pixel comparison tool |

---

## 10. CSB and DM2 readiness (W10)

| Area | Status | Notes |
|------|--------|-------|
| CSB original data acquired | `BLOCKED_ON_REFERENCE` | Must acquire before any CSB parity work |
| CSB parity matrix created | Not started | Depends on reference data |
| DM2 original data acquired | `BLOCKED_ON_REFERENCE` | Must acquire before any DM2 parity work |
| DM2 parity matrix created | Not started | Depends on reference data |
| DM1 assumptions leaking into CSB/DM2 | No leakage — CSB/DM2 work has not started | Monitor when work begins |

---

## Summary counts

| Status | Count |
|--------|-------|
| `MATCHED` | 8 (7 prior + title-side UI asset mapping; mostly infrastructure/data, not visual parity) |
| `KNOWN_DIFF` | 8 (prior 3 + base palette rendering, brightness rendering, creature palette rendering, cyan invariant, special palettes) |
| `UNPROVEN` | ~41 (reduced from ~45: 4 rows reclassified to KNOWN_DIFF, 1 partially advanced to MATCHED/UNPROVEN) |
| `BLOCKED_ON_REFERENCE` | ~12 (timing, audio, CSB data, DM2 data) |

**Bottom line:** Firestaff has substantial infrastructure and implementation, but almost nothing is source-backed-verified against the original yet. The parity gap is not primarily missing code — it is missing original reference captures and systematic comparison.

**Pass 1 key finding (2026-04-22):** The VGA palette compat layer (`vga_palette_pc34_compat.c`) uses an EGA-style palette — 14 of 16 colors are wrong. The correct values exist in `recovered_palette.json` (from VIDEODRV.C) but are not wired into the rendering path. This affects every rendered frame. See `parity-evidence/dm1_v1_pass1_palette_and_assets.md` for full evidence.
