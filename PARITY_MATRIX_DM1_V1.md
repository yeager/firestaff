# Parity Matrix — DM1 V1 (Original-Compatible)

Last updated: 2026-04-25

Target: **Dungeon Master 1, PC DOS, VGA, English — 1:1 original-faithful behavior and presentation.**

This is the DM1 slice of the V1 parity ledger. CSB and DM2 will get their own matrices when reference data is acquired.
V2/V3 differences must never appear in this matrix as accepted parity.

Reference bank to consult actively when relevant:
- Greatstone / dmweb:
  - `http://greatstone.free.fr/dm/db_data/dm_pc_34/graphics.dat/graphics.dat.html`
  - `http://greatstone.free.fr/dm/db_data/dm_pc_34/dungeon.dat/dungeon.html`
  - `http://greatstone.free.fr/dm/db_data/dm_pc_34/title/title.html`
  - `http://greatstone.free.fr/dm/db_data/dm_pc_34/end/end.html`
  - `http://greatstone.free.fr/dm/db_data/dm_pc_34/song.dat/song.dat.html`
- ReDMCSB / dmweb:
  - `http://dmweb.free.fr/community/redmcsb/`
  - `http://dmweb.free.fr/Stuff/ReDMCSB_WIP20210206.7z`
  - `http://dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm`
- Local ReDMCSB unpack/analysis:
  - `/Users/bosse/.openclaw/workspace-main/ReDMCSB_WIP20210206`
  - `REDMCSB_LOCAL_ANALYSIS_2026-04-24.md`

Rules for this matrix:
- ReDMCSB is a structural reference, but DM1 PC 3.4 remains the target truth for this file.
- `BugsAndChanges.htm` must be checked before promoting a ReDMCSB-observed behavior to `MATCHED` when that behavior could be a ReDMCSB-only fix.
- For this PC/VGA/English target, use ReDMCSB label `I34E` unless a row explicitly says otherwise. `I34M` is multilingual comparison material only.
- Prefer `Reference/Original/I34E` and original runtime captures over rebuilt `Reference/ReDMCSB/I34E/FIRES` for binary/runtime truth; use ReDMCSB source to locate structure, functions, and conditionals.
- Treat `Documentation/Engine.htm` and `Documentation/BugsAndChanges.htm` as Atari-ST-scoped indexes. Do not mark PC 3.4 rows `MATCHED` from those docs alone; verify against `I34E` source/conditionals or original runtime.

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
| Viewport region bounds | DEFS.H: `C112_BYTE_WIDTH_VIEWPORT`=112 (224px at 4bpp), `C136_HEIGHT_VIEWPORT`=136. Graphic #0 (`C000_DERIVED_BITMAP_VIEWPORT`) is 224×136, confirming exact match. Viewport screen origin `G2067/G2068=(0, 33)` from COORD.C:81-82. Viewport uses color indices 16–31 via `G8177_c_ViewportColorIndexOffset=0x10`. | `m11_game_view.c` runtime viewport enum now binds `M11_VIEWPORT_*` directly to `M11_DM1_VIEWPORT_*`: `(0, 33, 224, 136)`. Phase 76–79 migrated probe gates to this source rectangle and removed the old prototype `PROBE_VIEWPORT_*` constants. | `MATCHED` for rectangle bounds — pass 40's old `KNOWN_DIFF` was superseded by the all-graphics viewport migration. Pixel/content parity remains a separate visual/rendering task. See `parity-evidence/dm1_all_graphics_phase76_probe_dm1_viewport_rect.md`, phase 77, phase 79, and phase 80. | Continue viewport content parity: source draw order, right-column/action UI polish, and original screenshot overlay. |
| Party/champion region | `DEFS.H` constants: portrait 32×29 (`G2078`/`G2079`), atlas addressing `M027_PORTRAIT_X`/`M028_PORTRAIT_Y`, champion status-box spacing 69 px (`C69_CHAMPION_STATUS_BOX_SPACING`), status-box frame graphic `C007_GRAPHIC_STATUS_BOX` 67×29. | Firestaff V1: `M11_PORTRAIT_W=32, M11_PORTRAIT_H=29` (matches); `M11_V1_PARTY_SLOT_STEP=69` + `M11_V1_PARTY_SLOT_W=67` (pass 41). Atlas indexing `(i & 7)*32, (i >> 3)*29` matches. V2 vertical-slice mode still uses the legacy `M11_PARTY_SLOT_STEP=77` / `M11_PARTY_SLOT_W=71` via `m11_party_slot_step()` / `m11_party_slot_w()` to keep the pre-baked 302×28 four-slot HUD sprite aligned. | `MATCHED` (V1 stride + slot width) — portrait identity + slot horizontal stride now both source-anchored. See `parity-evidence/pass41_status_box_stride.md` and `parity-evidence/overlays/pass41/`. Placement origin `(12, 160)` remains `BLOCKED_ON_REFERENCE` (ZONES.H not yet parsed; pass 47b). | Retire stride line.  Remaining sub-row: party-panel origin `BLOCKED_ON_REFERENCE`, tracked for pass 47b. |
| Inventory screen | ReDMCSB source available; slot-box object drawing is source-bound through `OBJECT.C:F0038_OBJECT_DrawIconInSlotBox` and slot graphics `C033`/`C034`/`C035` are identified as 18×18 slot-box variants. | Firestaff has an inventory panel and the deterministic capture fixture now emits `06_ingame_inventory_panel_latest` with a source dagger icon in the right hand. Phases 91, 93, and 95 prove inventory slot icons use source object icons and preserve source colour 12 without the action-area palette remap. | `KNOWN_DIFF` (narrowed) — slot icon rendering is source-backed and capture-gated, but full inventory layout/panel composition still lacks side-by-side original overlay. | Capture original inventory screen and overlay against deterministic `06_ingame_inventory_panel_latest`; then retire remaining layout/panel diffs. |
| Spell panel | DEFS.H `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND` (87×25), `C011_GRAPHIC_MENU_SPELL_AREA_LINES` (14×39). | Firestaff `M11_DM_SPELL_AREA_W=87, M11_DM_SPELL_AREA_H=25` matches the backdrop graphic; graphic indices 9 and 11 used via `M11_GFX_SPELL_AREA_*`. Placement (x,y) = (224, 90). | `MATCHED` (asset identity 87×25 + index 9; lines graphic 11) / `BLOCKED_ON_REFERENCE` (placement overlay). See `parity-evidence/pass34_sidepanel_rectangle_table.md` §3. | Pixel-overlay against ReDMCSB capture when ZONES.H / PANEL.C pixel walk lands. |
| Map overlay | Local ReDMCSB PC source has `NEWMAP.C`, but it is map-transition plumbing (`F0003_MAIN_ProcessNewPartyMap_CPSE`: music track, active groups, current-map graphics, dungeon-view palette), not a player automap/overlay renderer. No source-backed full-screen player map UI has been identified in the DM1 V1 runtime. | Firestaff still contains the invented `mapOverlayActive` / `m11_draw_fullscreen_map(...)` surface, but pass 103 makes it debug-only: default V1 chrome mode ignores `M12_MENU_INPUT_MAP_TOGGLE` unless `showDebugHUD=1`. Probes `INV_GV_181/181B/197` lock both the debug path and the default ignored path. | `KNOWN_DIFF` (debug-only) — retained as a Firestaff debug surface, no longer reachable in normal V1 parity play. | Keep debug-only unless contrary source/runtime evidence appears; do not count it as original UI. |
| Dialog/endgame overlays | ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` expands the original dialog-box graphic into the viewport, prints `V3.4` in `C450_ZONE_DIALOG_VERSION`, patches 1/2/4-choice layouts with dialog patch zones, centers up to two message strings, and uses source choice zones/colours. `ENDGAME.C:F0444_STARTEND_Endgame` has a separate source flow using `THE END`, champion mirror/portrait zones, champion text, and restart/quit controls. | Firestaff has compat stubs (`dialog_frontend_pc34_compat.*`, `endgame_frontend_pc34_compat.*`) and runtime flags. Passes 107–121 replaced the visible dialog placeholder path with source-backed V1 drawing: source C000 backdrop, C450 `V3.4`, C469/C471 message zones, source-width line splitting, C462–C467 choice text zones, choice hit flow, and M621/M622/M623 1/2/4-choice patch graphics. Passes 122–123 replaced the default V1 game-won panel with source endgame graphics: C006 `THE END`, C346 champion mirrors in C412–C415, restart/quit source boxes, and champion names at source coordinate x=87/y=14+48n. Remaining endgame composition is incomplete: champion portrait blits, title text, skill list, timing/music/restart loop, and original overlay comparison captures. | `KNOWN_DIFF` (narrowed) — dialog visual path is substantially source-backed; endgame is source-backed but incomplete. | Finish endgame portraits/title/skill list or document blockers, then capture original dialog/endgame frames for overlay comparison. |
| Title screen composition | Composed backdrop path verified; `dialogGraphicIndex=1` contains title card; held state at graphic 313 is pixel-stable | Title hold verified via `--title-hold` mode, pixel-identical across repeated frames | `UNPROVEN` — no direct original screenshot comparison | Capture original title screen via emulator for overlay |
| Menu screen | M9 beta harness reaches menu state; submenu matrix exists | Menu interaction verified via M9 verify gate; submenu drift removed | `UNPROVEN` — layout not measured against original | Measure original menu layout |

---

## 2. UI sprites and assets (W3)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| GRAPHICS.DAT decode coverage | 577/713 bitmaps decoded, 0 decode failures | Full extraction in `extracted-graphics-v1/` with manifest | `MATCHED` | — |
| Champion portraits | `G2078_C32_PortraitWidth=32`, `G2079_C29_PortraitHeight=29` (COORD.C), `M027_PORTRAIT_X`/`M028_PORTRAIT_Y` atlas math (DEFS.H). | Firestaff `M11_PORTRAIT_W=32, M11_PORTRAIT_H=29` and the exact same atlas math at m11_game_view.c:11670–11671. GRAPHICS.DAT entry `C026_GRAPHIC_CHAMPION_PORTRAITS`. | `MATCHED` — portrait pixel size and atlas addressing identical to source. See `parity-evidence/pass34_sidepanel_rectangle_table.md` §3. | — |
| Equipment/item icons | ReDMCSB `OBJECT.C` binds object icon selection through `F0033_OBJECT_GetIconIndex` and slot-box drawing through `F0038_OBJECT_DrawIconInSlotBox`; object-icon atlases are GRAPHICS.DAT graphics `42..48`, 32 icons per graphic, 16×16. Dynamic variants covered from source: empty hand `201`, lit torch `G0029` charge bucket, charged weapons `+1`, closed scroll, compass direction, charged water/Jewel Symal/Illumulet. | Phases 84–91 wire the source resolver into action-hand cells and inventory slot boxes. Action cells apply action-area `G0498` palette remap (`12 -> C04 cyan`) and preserve `ActionSetIndex==0` as plain cyan; inventory slots use direct source icons without that palette rewrite. Probes cover empty hand, ActionSet gating, palette remap, torch/charged variants, scroll/compass/junk variants, and inventory no-remap behavior. | `MATCHED` for icon selection/atlas extraction/palette distinction in current action + inventory slot surfaces; `UNPROVEN` for mouse-held/pointer or original screenshot pixel overlay. | Extend the same resolver to any remaining pointer/held-object surfaces, then side-by-side original inventory capture overlay. |
| Rune glyphs / spell label cells | DEFS.H identifies `C011_GRAPHIC_MENU_SPELL_AREA_LINES` as a 14×39 graphic split into three 14×13 cells; `SYMBOL.C:F0399` encodes runes as `0x60 + 6*row + col`. | Firestaff uses `M11_SPELL_LABEL_CELL_W/H = 14×13`, blits C011 available/selected cells for spell-panel rune labels, and encodes rune bytes with the source formula. Capture smoke now checks the selected-rune C011 cell colours in `04_ingame_spell_panel_latest`. | `MATCHED` for C011 cell identity, split, and rune encoding in the current spell-panel surface; `UNPROVEN` for exact original spell-panel placement/pixel overlay. | Original spell-panel screenshot overlay once capture reference is stable. |
| Panel backgrounds/ornaments | ReDMCSB/DUNVIEW.C ornament paths use per-map ornament index tables; local renderer resolves wall/floor/door ornament ordinals through the DUNGEON.DAT metadata cache. Known graphics ranges: wall ornaments start at `259` (`M615`), floor ornaments use regular sets from `247` plus special footprints `379..384`, and door ornaments resolve through per-map door ornament tables. | Firestaff parses per-map wall/floor/door ornament indices, renders floor ornaments below items/creatures/projectiles, renders wall/door ornaments on center and side panels, and has focused gates (`INV_GV_38I/J/K`, `INV_GV_114`, `INV_GV_234/235/238/248`) proving visible draw-paths, footprint special-case, depth scaling, side-pane path, and cache storage. | `KNOWN_DIFF` (narrowed) — ornament data paths and draw-paths are source-backed/probe-gated, but exact original panel placement/clipping/z-order still lacks screenshot overlay. | Capture original ornament-heavy views and overlay against focused Firestaff fixtures; then retire remaining placement/z-order diffs. |
| Title-side UI assets | DEFS.H defines: C001_GRAPHIC_TITLE(320×200), C002_ENTRANCE_LEFT_DOOR(105×161), C003_ENTRANCE_RIGHT_DOOR(128×161), C004_ENTRANCE(320×200), C005_CREDITS(320×200), C006_THE_END(80×14). All 6 extracted as BITMAP_SAFE with 0 decode failures. Symlinked in `by-category/title-ui/`. | Used in title boot sequence (M6/M7 verified). Asset-to-define mapping complete. | `MATCHED` (asset identification and extraction) / `UNPROVEN` (pixel-level rendering comparison) | Pixel-compare rendered title/entrance/credits against original emulator captures. See §E6. |
| TITLE animation bank | Greatstone DM PC 3.4 TITLE reference lists 59 items: `AN`, `BR`, `P8`, first `PL`, first `EN` + 36 `DL`, second `PL`, second `EN` + 15 `DL`, then `DO`; local original `TITLE` is 12002 bytes. Pass 56 parser/player probes verify 2 EN + 51 DL = 53 original 320×200 frames, palette break segmentation 37 + 16 frames, and `DO` stop semantics. Pass 57 decodes original TITLE IMG1 EN/DL payloads, composites DL transparent skips over the 320×200 V1 canvas, applies PL palettes, and matches all 53 rendered RGB frames byte-for-byte against local Greatstone source PNG references. Pass 58 wires those rendered original-resolution frames into the V1/M9 title frontend packed-4bpp screen path and verifies first/boundary/last-frame reachability plus M9 `--title-hold` publication. Pass 59 adds a deterministic finite presentation policy: source steps 1..53 render once, frame 53 is marked handoff-ready, and later implementation overrun steps hold frame 53 instead of wrapping. Pass 61 adds an opt-in frontend/harness handoff policy that preserves pass-59 hold semantics by default, but can switch caller-owned presentation to the menu surface on the first post-boundary step. Pass 62 adds a bounded DOSBox evidence note and capture harness; initial automation reached the original selector but could not cleanly enter the graphical TITLE runtime non-interactively. Pass 63 adds a local DOSBox Staging + `cliclick` selector-input wrapper that types the required graphics/sound/input choices and reaches the original graphical TITLE screen, but the resulting host screenshot is not clean enough for pixel/cadence parity because an unrelated macOS Keychain prompt overlays the window. Pass 64 replaces whole-desktop capture with clean targeted DOSBox-window capture by CGWindowID. Pass 65 deterministically crops the pass-64 clean window captures to 320×200 original evidence and compares the first clean still to the source-backed Firestaff/Greatstone TITLE render dump; best match is `frame_0037.ppm` with MAE `1.693047` and max delta `12`, but the two captures are byte-identical and untimestamped. Pass 66 adds a timestamped targeted-window sequence path; the local 16-still run produced 7 low-residual TITLE-like matches (best refs `frame_0039.ppm`, `frame_0046.ppm`, `frame_0037.ppm`) followed by high-residual transition/non-TITLE rows, proving the sequence bridge works but not final cadence. Pass 67 proves DOSBox Staging native raw screenshots work: a retained 18-still run produced 18/18 raw 320×200 framebuffer PNGs and 7 low-residual TITLE-like matches to `frame_0012.ppm` (`MAE 1.355042`, max delta `12`); a later-offset tuning run hit `frame_0030.ppm`, but the Cmd+F5 mapper path remains too blocking/sparse for monotonic cadence. | Loader/player plus bounded original-data renderer landed in `title_dat_loader_v1.[ch]`; V1 frontend adapter, finite sequence policy, and opt-in title-menu handoff decision landed in `title_frontend_v1.[ch]`; M9 title-hold uses original TITLE frames when present and preserves GRAPHICS.DAT graphic-313 fallback when missing. M9 `--title-menu-handoff` verifies 53 original TITLE frames followed by a deterministic menu-surface frame when requested. Low-level frame sampling remains deterministic implementation cadence only; original timing/cadence and emulator handoff timing are not claimed. Pass 62 documents that `DM VGA` still blocks at the original text selector, input redirection is not a clean handoff path, and direct VGA overlay execution is not a valid bypass. Pass 63 documents that host keystroke injection via `scripts/dosbox_dm1_title_input_pass63.sh` is the stable local selector-handoff path for future original-runtime captures. Pass 65 provides a reproducible still-frame crop/compare bridge from clean DOSBox window evidence. Pass 66 provides timestamped still-sequence evidence, but host capture is sparse/slow and the low-residual TITLE portion is not a robust monotonic source-frame timing trace. Pass 67 adds native raw screenshot still evidence, eliminating host crop/scaling uncertainty, but confirms the built-in screenshot mapper is still too sparse/blocking for cadence. | `KNOWN_DIFF` (narrowed) | Capture/source-prove title animation timing/cadence, palette-display timing, and handoff via per-presented-frame emulator framebuffer dump or PC-conditional source timing proof; then pixel-compare wired frontend output against original emulator capture before claiming full title animation parity. |
| Placeholder entries | 131 placeholders identified and cataloged | Correctly skipped during decode/render | `MATCHED` | — |
| Text-tag fallbacks | `TEXT.C`/`TEXT2.C`/`PANEL.C`/`CHAMPION.C`/`SPELDRAW.C` distinguish text-engine vs bar-graph vs graphic | Pass 43 landed source-faithful V1 HP/stamina/mana bar graphs (`CHAMDRAW.C` / recovered ZONES.H geometry, source color table `{7,11,8,14}`); pass 44 landed the native `C011_GRAPHIC_MENU_SPELL_AREA_LINES` 14×13 cell blits for available-rune and selected-rune spell-panel labels; pass 45 verified that the remaining generic text overlays now default to the original GRAPHICS.DAT font atlas instead of the custom builtin font. | `MATCHED` — text-vs-graphic routing for the currently visible V1 HUD surfaces is now source-faithful at the font/graphic layer. Remaining text-content questions stay tracked separately under over-labeling / string-parity rows. | Retire this row for the pass-45 scope; next medium blocker is the VGA palette swap in §3. |

---

## 3. VGA palette and color (W3 adjacent)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| Base palette (16 colors) | Verified from `VIDEODRV.C` source — custom VGA DAC, NOT EGA. 16 unique colors including Cyan(idx4), Brown(idx3), Tan(idx10), etc. | `vga_palette_pc34_compat.c` exposes the recovered `VIDEODRV.C` / `palette-recovery/recovered_palette.json` base VGA values through `G9010_auc_VgaPaletteBrightest_Compat` and `F9010_VGA_GetColorRgb_Compat`. | `MATCHED` — pass 46 probe-backed for the palette lookup seam. | Retire base-palette swap row. See `parity-evidence/pass46_vga_palette_lookup.md`. |
| Brightness levels (LIGHT0–5) | All 6 levels with per-color values extracted from `VIDEODRV.C` (G8151–G8156). NOT linearly attenuated — each level has independently tuned values. | `vga_palette_pc34_compat.c` exposes all six recovered per-level tables through `G9010_auc_VgaPaletteAll_Compat`; LIGHT5 retains 8 residual non-black colors. | `MATCHED` — pass 46 probe-backed for lookup data and sampled LIGHT1–LIGHT5 values. | Retire brightness lookup row. Runtime darkness/pixel comparisons remain separate visual work. |
| Creature palettes | 14 creature palettes (G8175_CREAT_PAL) extracted from `VIDEODRV.C` — 14 types × 6 replacement colors (indices 1–6) | In `recovered_palette.json` with full VGA6 data; pass 46 does not wire creature palette replacement/rendering support. | `MATCHED` (data) / `KNOWN_DIFF` (rendering — not integrated into compat layer) | Wire creature palettes from `recovered_palette.json` into rendering path. See §E5. |
| Cyan invariant (idx 4) | Confirmed invariant: VGA6 (0,54,54) = RGB8 (0,219,219) across all 6 brightness levels in `recovered_palette.json` (verified from VIDEODRV.C) | `F9010_VGA_GetColorRgb_Compat(4, level)` returns `(0,219,219)` for every level 0..5. | `MATCHED` — pass 46 probe-backed. | — |
| Special palettes (credits, entrance, swoosh) | CREDITS (`G8147_CREDITS`) and ENTRANCE (`G8148_ENTRANCE`) are source-backed in local ReDMCSB `DRAWVIEW.C:25-59`; table entries at `DRAWVIEW.C:420-421` select them as palette ids 6/7. | Pass 68 exposes converted RGB8 arrays `G9011_auc_VgaPaletteCredits_Compat` and `G9012_auc_VgaPaletteEntrance_Compat`, plus `F9011_VGA_GetSpecialColorRgb_Compat(...)` / indexed table `G9013_auc_VgaPaletteSpecial_Compat`. Frontend call-site timing/palette selection is still not proven. | `KNOWN_DIFF` (narrowed) — source-backed data + lookup seam landed; full entrance/credits runtime palette switching and pixel overlays still open. | Wire and verify exact entrance/credits frontend palette selection timing; then compare rendered screens against original capture. See `parity-evidence/pass68_special_palettes.md`. |
| Falsecolor vs. true-color | Current `ppm-falsecolor/` exports are inspection artifacts, not claimed final RGB | M10 VGA palette export uses the pass-46 source-backed palette lookup and still produces valid 320×200 PPM output. | `MATCHED` (palette export seam) / `UNPROVEN` (full pixel-overlay color parity) | Re-export comparison artifacts after viewport/layout capture paths are ready. |

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
| Over-labeling (invented strings in V1) | Original had minimal text | Firestaff invents: 82 `m11_set_status` strings (status lozenge surface), 68 `inspectTitle/inspectDetail` emissions (inspect overlay), 3 utility-strip captions (INSPECT/SAVE/LOAD), control + prompt chrome strips at y=165 | `KNOWN_DIFF` — enumerated in `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` (Pass 35) §2§2.2, §2.4, §2.5, §2.6 | Hide Firestaff-invented UI chrome in V1 mode; reroute relevant events to message log. Tracked for pass-37+. |
| Font rendering | Original font data in GRAPHICS.DAT, rendered via `TEXT.C`/`TEXT2.C` | Firestaff resolves the original interface-font entry through `font_m11.[ch]`, activates `g_activeOriginalFont` during `M11_GameView_Draw`, and defaults `m11_draw_text(...)` to that original font when original assets are present. Builtin 7-pixel glyphs remain only as the no-assets fallback. | `MATCHED` — pass 45 probe-backed. See `parity-evidence/pass45_font_bank_wiring.md` and `run_firestaff_m11_pass45_font_bank_probe.sh`. | Retire font-bank blocker; proceed to palette work (pass 46). |

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
| Sound trigger points | ReDMCSB I34E sound-index namespace mapped to SND3 item indices in Pass 52 (`DEFS.H`, `DATA.C`, `PASS52_AUDIO_FINDINGS.md`); Pass 55 audits remaining direct M11 marker calls | Sound-event → SND3 mapping table landed (`sound_event_snd3_map_v1.[ch]`); pass 53 routes `EMIT_SOUND_REQUEST` payloads through mapped event-index playback when original SND3 assets are present; pass 55 converts source-backed action cues (`WAR CRY`, `BLOW HORN`, `SHOOT`, `THROW`) to mapped event-index playback and documents four remaining direct-marker TODO buckets; runtime trigger cadence still not captured against original audio | `KNOWN_DIFF` (narrowed) | Capture cadence/overlap against original runtime and resolve the four documented direct-marker TODO buckets |
| Sound samples (content) — SONG.DAT music bank | `SONG.DAT` DM PC v3.4, 162482 bytes, DMCSB2, 10 items (1 SEQ2 + 9 SND8 at 11025 Hz DPCM); format verified (Pass 50, `DM1_SONG_DAT_FORMAT.md`, `parity-evidence/pass50_song_dat_header.txt`) | Format + decoder landed (`song_dat_loader_v1.[ch]`); pass 54 loads all 9 SND8 music parts when original `SONG.DAT` is present, linearly resamples signed 11025 Hz PCM to the fixed 22050 Hz SDL float stream, walks SEQ2 words up to the bit-15 marker, records loop target part 1 for `0x8001`, concatenates one pre-loop title phrase, and exposes `M11_Audio_PlayTitleMusic(...)` queueing with no-asset no-op fallback; probes PASS (7/7 + 6/6) | `KNOWN_DIFF` (narrowed) | Capture original title-music start/stop/continuous-loop cadence before claiming full music parity |
| Sound samples (content) — GRAPHICS.DAT SND3 SFX bank | 33 SND3 items (indices 671-675, 677-685, 687-693, 701-712) at 6000 Hz, per dmweb; Greatstone Sound 00..32 labels; real-file decode verified Pass 51 (`PASS51_AUDIO_FINDINGS.md`, `parity-evidence/pass51_v1_graphics_dat_snd3_probe.txt`); event mapping verified Pass 52 (`parity-evidence/pass52_v1_snd3_event_map_probe.txt`); runtime SND3 branch verified Pass 53 (`PASS53_AUDIO_FINDINGS.md`, `parity-evidence/pass53_v1_snd3_runtime_probe.txt`); direct-marker audit verified Pass 55 (`PASS55_AUDIO_FINDINGS.md`, `parity-evidence/pass55_m11_direct_audio_marker_audit.txt`) | Format + decoder landed (`graphics_dat_snd3_loader_v1.[ch]`); event mapping landed (`sound_event_snd3_map_v1.[ch]`); pass 53 loads all 35 mapped event-index buffers from original `GRAPHICS.DAT`, linearly resamples 6000 Hz unsigned PCM to the fixed 22050 Hz SDL float stream, queues mapped `EMIT_SOUND_REQUEST` buffers with procedural fallback preserved, and pass 55 converts four source-backed action cue sites to event-index playback; probes PASS (6/6 + 5/5 + 5/5 + pass55 audit) | `KNOWN_DIFF` (narrowed) | Capture original cadence/overlap and resolve remaining direct-marker TODO buckets before claiming full SFX parity |
| Sound cadence/overlap | No original capture | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator with audio |
| Music | `SONG.DAT` SEQ2 decoded; pass 54 runtime driver walks SEQ2 and queues one decoded/resampled pre-loop phrase | Asset-backed queue API exists; exact title frontend start/stop timing and continuous loop cadence are not captured | `KNOWN_DIFF` (narrowed) | Original runtime capture for title-music timing/looping, then bind frontend invocation timing if needed |

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

## Summary counts (after all-graphics Pass 97 matrix refresh)

| Status | Count |
|--------|-------|
| `MATCHED` | 13 — GRAPHICS.DAT decode coverage, placeholder cataloging, title-side UI asset mapping, Champion portraits (pass 34), Spell-panel asset identity (pass 34), M9 gate infra, M10 suite infra, M11 suite infra, Submenu matrix infra, Champion status-box stride + slot width V1 source-anchored (pass 41), viewport rectangle bounds (all-graphics phases 76–81), equipment/item icon resolver + palette split (passes 84–95), rune/C011 spell label cells (pass 96). |
| `KNOWN_DIFF` | 14 — prior palette + typography + Firestaff-invented UI chrome, the invented/convenience map overlay, placeholder dialog/endgame overlays, plus the now-narrowed inventory-screen and ornament rows. Viewport −28×−18 drift from pass 33/40 was superseded by all-graphics phases 76–81; champion status-box +8 px stride retired in pass 41. |
| `UNPROVEN` | ~33 — reduced from ~34 after dialog/endgame overlays moved from generic unknown to explicit `KNOWN_DIFF` placeholder status. |
| `BLOCKED_ON_REFERENCE` | ~11 — timing, audio, CSB data, DM2 data, plus pixel-overlay rows across §1–§2 that remain unblocked until a ReDMCSB headless rasteriser or emulator capture lands. |

**Bottom line (updated 2026-04-25, pass 124):** Firestaff has measured ownership progress (passes 29–32), measured visual drift (passes 33–34), a complete text-vs-graphics enumeration (pass 35), source-faithful champion status-box spacing (pass 41), source-matched DM1 viewport rectangle bounds via the all-graphics phases, source-bound action/inventory object icons plus C011 spell label cells, a narrowed inventory-screen row backed by deterministic capture evidence, narrowed ornament metadata/draw-path coverage, and the invented map overlay is explicitly `KNOWN_DIFF` but now debug-only and unreachable in normal V1 parity play. Dialog visuals are now source-backed across backdrop/version/message/choice/patch/input flow; endgame visuals are source-backed for `THE END`, champion mirrors, restart/quit boxes, and champion names, but still incomplete. **This still does not complete DM1/V1 parity**; remaining work is viewport content/draw-order parity, original screenshot overlays, audio/timing, remaining source endgame details, and non-current surfaces such as pointer/held-object icons and exact inventory/spell-panel/ornament placement overlays.

**Pass 1 key finding (2026-04-22):** The VGA palette compat layer (`vga_palette_pc34_compat.c`) uses an EGA-style palette — 14 of 16 colors are wrong. Tracked as V1 blocker #10 in `V1_BLOCKERS.md`.  See `parity-evidence/dm1_v1_pass1_palette_and_assets.md` for full evidence.
