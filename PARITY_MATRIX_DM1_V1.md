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
| Viewport region bounds | ReDMCSB source: known viewport rectangle, backdrop composition path verified in M7 | Composed backdrop + viewport patch verified stable in `m7_reachability_b` | `UNPROVEN` — exact pixel bounds not measured | Build layout measurement sheet from ReDMCSB source |
| Party/champion region | ReDMCSB `DEFS.H` struct layout available | Firestaff has champion region | `UNPROVEN` | Extract coordinates from `DEFS.H`, compare |
| Inventory screen | ReDMCSB source available | Firestaff has inventory screen (see `verification-screens/03_ingame_start.png`) | `UNPROVEN` | Side-by-side with original capture |
| Spell panel | ReDMCSB source available | Firestaff has spell panel (see `verification-screens/04_ingame_spell_panel_latest.png`) | `UNPROVEN` | Side-by-side with original capture |
| Map overlay | Unknown — may not exist in DM1 V1 scope | Unknown | `BLOCKED_ON_REFERENCE` | Confirm if DM1 has map overlay |
| Dialog/endgame overlays | `dialog_frontend_pc34_compat.{c,h}`, `endgame_frontend_pc34_compat.{c,h}` exist | Compat layers exist but not visually verified | `UNPROVEN` | Capture original dialog frames |
| Title screen composition | Composed backdrop path verified; `dialogGraphicIndex=1` contains title card; held state at graphic 313 is pixel-stable | Title hold verified via `--title-hold` mode, pixel-identical across repeated frames | `UNPROVEN` — no direct original screenshot comparison | Capture original title screen via emulator for overlay |
| Menu screen | M9 beta harness reaches menu state; submenu matrix exists | Menu interaction verified via M9 verify gate; submenu drift removed | `UNPROVEN` — layout not measured against original | Measure original menu layout |

---

## 2. UI sprites and assets (W3)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| GRAPHICS.DAT decode coverage | 577/713 bitmaps decoded, 0 decode failures | Full extraction in `extracted-graphics-v1/` with manifest | `MATCHED` | — |
| Champion portraits | Indexed in GRAPHICS.DAT; not yet mapped to specific component usage | Unknown if Firestaff uses original portraits or placeholders | `UNPROVEN` | Map portrait indexes, compare rendered output |
| Equipment/item icons | Present in GRAPHICS.DAT extraction | Unknown mapping status | `UNPROVEN` | Build GRAPHICS.DAT → component usage map |
| Rune glyphs | Present in GRAPHICS.DAT extraction | Unknown mapping status | `UNPROVEN` | Map glyph indexes to spell panel usage |
| Panel backgrounds/ornaments | Front-lock assets identified: 303–320 (walls-ornate category) | `by-category/walls-ornate/` symlinks exist | `UNPROVEN` — usage not verified against original screen composition | Map ornament assets to panel placement |
| Title-side UI assets | `by-category/title-ui/` — indexes 0001..0006 | Used in title boot sequence (M6/M7 verified) | `UNPROVEN` — not pixel-compared to original | Direct comparison needed |
| Placeholder entries | 131 placeholders identified and cataloged | Correctly skipped during decode/render | `MATCHED` | — |
| Text-tag fallbacks | — | Unknown if V1 still uses text fallbacks where graphics should be | `UNPROVEN` | Audit all V1 screens for text-where-graphics-should-be |

---

## 3. VGA palette and color (W3 adjacent)

| Area | Original/Reference Evidence | Firestaff Current State | Status | Next Action |
|------|---------------------------|------------------------|--------|-------------|
| Base palette (16 colors) | Verified from `VIDEODRV.C` source — custom VGA DAC, NOT EGA | `palette-recovery/recovered_palette.json` has full data | `MATCHED` (palette data extraction) | Verify Firestaff rendering uses these exact values |
| Brightness levels (LIGHT0–5) | All 6 levels extracted from `VIDEODRV.C` | In `recovered_palette.json` | `MATCHED` (data) / `UNPROVEN` (rendering) | Verify depth-dependent palette switching in Firestaff viewport |
| Creature palettes | 14 creature palettes extracted from `VIDEODRV.C` | In `recovered_palette.json` | `MATCHED` (data) / `UNPROVEN` (rendering) | Verify creature color replacement in Firestaff |
| Cyan invariant (idx 4) | Confirmed invariant across all 6 brightness levels in source | Unknown if Firestaff preserves this | `UNPROVEN` | Add specific test |
| Special palettes (credits, entrance, swoosh) | Extracted from `VIDEODRV.C` | Unknown if implemented | `UNPROVEN` | Check against intro/credits rendering |
| Falsecolor vs. true-color | Current `ppm-falsecolor/` exports are inspection artifacts, not claimed final RGB | Firestaff PPM screenshots exist in `verification-screens/` | `KNOWN_DIFF` — falsecolor is deliberately NOT original palette | Apply `recovered_palette.json` to verify true-color output |

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
| `MATCHED` | 7 (mostly infrastructure/data extraction, not visual/behavioral parity) |
| `KNOWN_DIFF` | 3 (falsecolor palette, audio placeholders, no automated image comparison) |
| `UNPROVEN` | ~45 (the vast majority — most areas have implementation but no original-backed verification) |
| `BLOCKED_ON_REFERENCE` | ~12 (timing, audio, CSB data, DM2 data) |

**Bottom line:** Firestaff has substantial infrastructure and implementation, but almost nothing is source-backed-verified against the original yet. The parity gap is not primarily missing code — it is missing original reference captures and systematic comparison.
