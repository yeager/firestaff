# Parity Matrix — DM1 V1 (Original-Compatible)

Last updated: 2026-04-24

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
| Viewport region bounds | DEFS.H: `C112_BYTE_WIDTH_VIEWPORT`=112 (224px at 4bpp), `C136_HEIGHT_VIEWPORT`=136. Graphic #0 (`C000_DERIVED_BITMAP_VIEWPORT`) is 224×136, confirming exact match. Viewport screen origin `G2067/G2068=(0, 33)` from COORD.C:81-82. Viewport uses color indices 16–31 via `G8177_c_ViewportColorIndexOffset=0x10`. | `m11_game_view.c` runtime viewport enum: `M11_VIEWPORT_X=12, M11_VIEWPORT_Y=24, M11_VIEWPORT_W=196, M11_VIEWPORT_H=118` — measured 196×118 on framebuffer. DM1 source anchor `(0, 33, 224, 136)` is encoded in the adjacent `M11_DM1_VIEWPORT_*` enum (pass 40) but not yet active. | `KNOWN_DIFF` — **locked (pass 40)**. Firestaff viewport is **−28 px narrower and −18 px shorter** than DM1 original (24.1 % less pixel area). Horizon row 59 vs. source 68. See `parity-evidence/pass33_viewport_coordinate_overlay.md` §3 and `parity-evidence/pass40_viewport_lock.md` (source anchor + 2 914 px² structural overlap with Firestaff-invented chrome that pass 42 must clear). | Bind the renderer to `M11_DM1_VIEWPORT_*` as part of pass 42 (invented-chrome reroute, V1_BLOCKERS §6). Independent landing at pass 40 is blocked by 2 914 px² of HUD overlap; evidence lives in `firestaff_m11_pass40_viewport_lock_probe.c`. |
| Party/champion region | `DEFS.H` constants: portrait 32×29 (`G2078`/`G2079`), atlas addressing `M027_PORTRAIT_X`/`M028_PORTRAIT_Y`, champion status-box spacing 69 px (`C69_CHAMPION_STATUS_BOX_SPACING`), status-box frame graphic `C007_GRAPHIC_STATUS_BOX` 67×29. | Firestaff V1: `M11_PORTRAIT_W=32, M11_PORTRAIT_H=29` (matches); `M11_V1_PARTY_SLOT_STEP=69` + `M11_V1_PARTY_SLOT_W=67` (pass 41). Atlas indexing `(i & 7)*32, (i >> 3)*29` matches. V2 vertical-slice mode still uses the legacy `M11_PARTY_SLOT_STEP=77` / `M11_PARTY_SLOT_W=71` via `m11_party_slot_step()` / `m11_party_slot_w()` to keep the pre-baked 302×28 four-slot HUD sprite aligned. | `MATCHED` (V1 stride + slot width) — portrait identity + slot horizontal stride now both source-anchored. See `parity-evidence/pass41_status_box_stride.md` and `parity-evidence/overlays/pass41/`. Placement origin `(12, 160)` remains `BLOCKED_ON_REFERENCE` (ZONES.H not yet parsed; pass 47b). | Retire stride line.  Remaining sub-row: party-panel origin `BLOCKED_ON_REFERENCE`, tracked for pass 47b. |
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
| TITLE animation bank | Greatstone DM PC 3.4 TITLE reference lists 59 items: `AN`, `BR`, `P8`, first `PL`, first `EN` + 36 `DL`, second `PL`, second `EN` + 15 `DL`, then `DO`; local original `TITLE` is 12002 bytes. Pass 56 parser/player probes verify 2 EN + 51 DL = 53 original 320×200 frames, palette break segmentation 37 + 16 frames, and `DO` stop semantics. Pass 57 decodes original TITLE IMG1 EN/DL payloads, composites DL transparent skips over the 320×200 V1 canvas, applies PL palettes, and matches all 53 rendered RGB frames byte-for-byte against local Greatstone source PNG references. Pass 58 wires those rendered original-resolution frames into the V1/M9 title frontend packed-4bpp screen path and verifies first/boundary/last-frame reachability plus M9 `--title-hold` publication. Pass 59 adds a deterministic finite presentation policy: source steps 1..53 render once, frame 53 is marked handoff-ready, and later implementation overrun steps hold frame 53 instead of wrapping. Pass 61 adds an opt-in frontend/harness handoff policy that preserves pass-59 hold semantics by default, but can switch caller-owned presentation to the menu surface on the first post-boundary step. Pass 62 adds a bounded DOSBox evidence note and capture harness; initial automation reached the original selector but could not cleanly enter the graphical TITLE runtime non-interactively. Pass 63 adds a local DOSBox Staging + `cliclick` selector-input wrapper that types the required graphics/sound/input choices and reaches the original graphical TITLE screen, but the resulting host screenshot is not clean enough for pixel/cadence parity because an unrelated macOS Keychain prompt overlays the window. Pass 64 replaces whole-desktop capture with clean targeted DOSBox-window capture by CGWindowID. Pass 65 deterministically crops the pass-64 clean window captures to 320×200 original evidence and compares the first clean still to the source-backed Firestaff/Greatstone TITLE render dump; best match is `frame_0037.ppm` with MAE `1.693047` and max delta `12`, but the two captures are byte-identical and untimestamped. Pass 66 adds a timestamped targeted-window sequence path; the local 16-still run produced 7 low-residual TITLE-like matches (best refs `frame_0039.ppm`, `frame_0046.ppm`, `frame_0037.ppm`) followed by high-residual transition/non-TITLE rows, proving the sequence bridge works but not final cadence. | Loader/player plus bounded original-data renderer landed in `title_dat_loader_v1.[ch]`; V1 frontend adapter, finite sequence policy, and opt-in title-menu handoff decision landed in `title_frontend_v1.[ch]`; M9 title-hold uses original TITLE frames when present and preserves GRAPHICS.DAT graphic-313 fallback when missing. M9 `--title-menu-handoff` verifies 53 original TITLE frames followed by a deterministic menu-surface frame when requested. Low-level frame sampling remains deterministic implementation cadence only; original timing/cadence and emulator handoff timing are not claimed. Pass 62 documents that `DM VGA` still blocks at the original text selector, input redirection is not a clean handoff path, and direct VGA overlay execution is not a valid bypass. Pass 63 documents that host keystroke injection via `scripts/dosbox_dm1_title_input_pass63.sh` is the stable local selector-handoff path for future original-runtime captures. Pass 65 provides a reproducible still-frame crop/compare bridge from clean DOSBox window evidence. Pass 66 provides timestamped still-sequence evidence, but host capture is sparse/slow and the low-residual TITLE portion is not a robust monotonic source-frame timing trace. | `KNOWN_DIFF` (narrowed) | Capture/source-prove title animation timing/cadence, palette-display timing, and handoff using native/raw DOSBox screenshots or a faster monotonic targeted-window sequence; then pixel-compare wired frontend output against original emulator capture before claiming full title animation parity. |
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
| Special palettes (credits, entrance, swoosh) | CREDITS (G8147, 16 colors) and ENTRANCE (G8148, 16 colors) fully extracted from `VIDEODRV.C` lines 62–117 (VGA section). Custom warm/outdoor tones. | **Not implemented.** No references to G8147/G8148 or special palette switching in `vga_palette_pc34_compat.{c,h}`. Credits/entrance screens render with (wrong) base palette only. | `KNOWN_DIFF` — special palettes not implemented | Add CREDITS and ENTRANCE palette arrays to compat layer; wire palette switching for credits/entrance screens. See §E4. |
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

## Summary counts (after Pass 36 honesty lock)

| Status | Count |
|--------|-------|
| `MATCHED` | 10 — GRAPHICS.DAT decode coverage, placeholder cataloging, title-side UI asset mapping, Champion portraits (pass 34), Spell-panel asset identity (pass 34), M9 gate infra, M10 suite infra, M11 suite infra, Submenu matrix infra, Champion status-box stride + slot width V1 source-anchored (pass 41). |
| `KNOWN_DIFF` | 11 — prior palette + typography + Firestaff-invented UI chrome + viewport −28×−18 drift (pass 33, locked pass 40). Champion status-box +8 px stride retired (pass 41). |
| `UNPROVEN` | ~38 — reduced from ~41 after pass 33 (viewport), pass 34 (portraits + spell panel), pass 35 (text-tag fallbacks, over-labeling, font rendering) promotions. |
| `BLOCKED_ON_REFERENCE` | ~12 — timing, audio, CSB data, DM2 data, plus pixel-overlay rows across §1–§2 that remain unblocked until a ReDMCSB headless rasteriser or emulator capture lands. |

**Bottom line (after passes 29–36):** Firestaff has measured ownership progress (passes 29–32), measured visual drift (passes 33–34), and a complete text-vs-graphics enumeration (pass 35).  The remaining V1 gap is now a numbered ledger in `V1_BLOCKERS.md` rather than vague rows in this file.  **Executing passes 29–36 did not complete DM1/V1 parity**; it converted Firestaff's state from 'many reconstructed seams with under-measured visuals and mixed ownership' into 'source-backed ownership for movement/doors/env, measured visuals for viewport and side-panel, and an honest ledger of what remains' — which is exactly what `PASSLIST_29_36.md` said this program would do.

**Pass 1 key finding (2026-04-22):** The VGA palette compat layer (`vga_palette_pc34_compat.c`) uses an EGA-style palette — 14 of 16 colors are wrong. Tracked as V1 blocker #10 in `V1_BLOCKERS.md`.  See `parity-evidence/dm1_v1_pass1_palette_and_assets.md` for full evidence.
