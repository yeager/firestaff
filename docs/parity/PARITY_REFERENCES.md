# Parity Reference Sources

Last updated: 2026-04-22

This file inventories all known local reference sources and artifacts available for 1:1 parity verification work.

---

## Product contract

**Version 1** targets 1:1 original-faithful parity with **all three original games**:
- **DM1** — Dungeon Master (PC DOS, VGA, English)
- **CSB** — Chaos Strikes Back
- **DM2** — Dungeon Master II: The Legend of Skullkeep

V2 (modern graphics/resolution) and V3 (modern 3D) are separate product tracks.
Nothing from V2 or V3 may contaminate V1 parity claims, evidence, or implementation.

---

## A. Original game data packages

### A1. DM1 DOS package
- **Location (canonical worker-local):** `~/.openclaw/data/firestaff-original-games/DM/Game,Dungeon_Master,DOS,Software.7z`
- **Extracted PC34 data set:** `~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/`
- **Historical/deprecated remote:** Burken `<deprecated-remote-dm-archive>` (do not use for worker-subagent provenance; retained only as old context)
- **Contents:**
  - `DUNGEON.DAT` — dungeon content data
  - `GRAPHICS.DAT` — all graphics (sprites, UI, viewport assets)
  - `SONG.DAT` — music/audio data
  - `DM.EXE` — main DOS loader
  - Overlay modules: `ANIM`, `EGA`, `FIRES`, `IBMIO`, `SELECTOR`, `SWOOSH`, `TITLE`, `VGA`
- **Copyright handling:** Not vendored into repo. Referenced for local verification only.
- **Good for:** Ground-truth data file formats, original asset presence, original executable behavior.

### A2. ReDMCSB WIP archive (dm7z-extract)
- **Local path:** `dm7z-extract/`
- **Source archive:** `ReDMCSB_WIP20210206.7z` (25,807,091 bytes)
- **Key contents:**
  - `Reference/Original/I34E/` — DM PC 3.4 English executables (11 LZEXE-compressed DOS files): `DM.EXE`, `FIRES` (94,779 bytes, main game), `VGA`, `ANIM`, `SWOOSH`, `SELECTOR`, etc.
  - `Reference/Original/G14ED/` — Amiga 1.4 English Demo: `DUNGEON.MASTER` (233,777 bytes), `START`
  - `Toolchains/Common/Source/VIDEODRV.C` (170,071 bytes) — **VGA video driver source with ALL palette data**
  - `Toolchains/Common/Source/DUNGEON.C` (181,374 bytes) — **Dungeon engine source**
  - `Toolchains/Common/Source/DEFS.H` (479,889 bytes) — Master struct definitions
- **Good for:** Structural reference for UI composition, component model, palette, gameplay engine logic, DM PC v3.4 faithful behavior, cross-check against Firestaff implementation.

### A3. CSB data — NOT YET ACQUIRED
- **Status:** `BLOCKED_ON_REFERENCE`
- **Needed:** Original CSB data files (DUNGEON.DAT, GRAPHICS.DAT equivalent) for CSB-specific parity work.

### A4. DM2 data — NOT YET ACQUIRED
- **Status:** `BLOCKED_ON_REFERENCE`
- **Needed:** Original DM2 data files for DM2-specific parity work.

---

## B. Extracted and processed artifacts (local)

### B1. GRAPHICS.DAT full extraction
- **Local path:** `extracted-graphics-v1/`
- **Source:** DM1 `GRAPHICS.DAT` (363,417 bytes, sha256: `2c3aa836...078ecf8e`)
- **Extraction summary:**
  - Total indexes: 713
  - Bitmap extracted: 577
  - Non-bitmap: 4
  - Placeholder: 131
  - Special: 1
  - Decode failed: 0
- **Sub-paths:**
  - `pgm/` — grayscale PGM per decoded bitmap
  - `ppm-falsecolor/` — palette-aware falsecolor PPM (inspection artifacts, not final RGB)
  - `non-bitmap/` — raw payload + metadata JSON for non-bitmap/placeholder/special entries
  - `by-category/` — symlink grouping: `title-ui` (0001..0006), `walls-ornate` (0303..0320), `unclassified` (rest)
  - `manifest.json` — machine-readable per-entry metadata (index, kind, width, height, status, exported_files, notes)
- **Good for:** Asset identification, UI sprite mapping, GRAPHICS.DAT → component usage correlation, screenshot comparison source.

### B2. VGA palette recovery
- **Local path:** `palette-recovery/`
- **Source:** `VIDEODRV.C` (verified from source, not inferred)
- **Contents:**
  - `recovered_palette.json` — complete machine-readable palette (all 6 brightness levels, 14 creature palettes, special palettes)
  - `apply_palette.py` — palette application tool
  - `samples/` — sample outputs
- **Key findings:**
  - Custom 16-color VGA DAC palette (NOT EGA)
  - 32 DAC entries: 0-15 (UI/icons), 16-31 (viewport, OR'ed with 0x10)
  - 6 brightness levels (LIGHT0..LIGHT5) for dungeon depth
  - Cyan (idx 4) invariant across all brightness levels
  - Creature replacement palettes for 14 creature types
  - Special palettes: CREDITS, ENTRANCE, BLACK, BLUE, SWOOSH animation
- **Good for:** Correct color rendering, palette-based comparison, brightness/depth verification, creature rendering parity.

### B3. Visible exports (full and filtered)
- **Local paths:** `visible-exports/`, `visible-exports-filtered/`
- **Contents:** PGM exports for all 713 graphic indexes (full), and filtered late-range classified graphics
- **Good for:** Broad visual inspection of the entire GRAPHICS.DAT content, quick lookup by index.

---

## C. Firestaff capture artifacts

### C1. Verification screenshots (Firestaff current state)
- **Local path:** `verification-screens/`
- **Contents (11 PNG + PPM pairs):**
  - `01_start_menu` — launcher/start menu
  - `02_settings` — settings screen
  - `03_ingame_start` — in-game initial state
  - `04_ingame_turn_right` — in-game after turning
  - `05_ingame_move_forward` — in-game after movement
  - `*_latest` variants for ingame screens (newer captures)
  - `launcher-modern.png` — modern launcher appearance
- **Good for:** Regression baseline, before/after comparison against original, current Firestaff visual state documentation.

### C2. ReDMCSB boot/title/menu probe captures
- **Local paths (selected):**
  - `exports_2026-04-17_title_menu_best/` — best title/menu candidate captures with falsecolor probes
  - `exports_2026-04-17_title_menu_composed/` — composed title/menu frames (backdrop + viewport)
  - `exports_2026-04-17_title_menu_composed_backdrop/`, `*_m7/` — backdrop-aware composed frames
  - `exports_2026-04-17_backdrop_compare/` — backdrop vs. composed labeled comparison PNGs
  - `exports_2026-04-17_patch_candidate_match/` — patch evolution and candidate matching sheets
  - `exports_2026-04-17_dialog_probe/` — dialog graphic probe exports
  - `analysis_m4/` — M4 menu candidate analysis sheets (PNG)
- **Root-level PGM files:** ~31,000 PGM frames from boot plan scripts, runtime tail probes, M9 beta harness interaction sequences, submenu matrix runs, etc.
- **Good for:** Title/menu visual comparison, boot sequence fidelity, interaction state verification, runtime behavior documentation.

### C3. Verification suites
- **Local paths:**
  - `verification-m10/` — M10 semantic verification: champion-lifecycle, combat, creature-ai, dungeon-doors-sensors, dungeon-header, dungeon-monsters-items, dungeon-text, dungeon-tiles, internal-matrix, magic, movement-champions, projectile, roundtrip-matrix, runtime-dynamics, savegame, sensor-execution, submenu-matrix, tick-orchestrator, timeline, vga-palette
  - `verification-m11/` — M11 verification: audio, fs_portable, game-view, launcher-smoke, phase-a, screenshots
  - `verification-m12/` — M12 verification: settings-smoke, startup-menu
  - `submenu-matrix/` — submenu interaction matrix PGM captures
  - `roundtrip-matrix/` — roundtrip invariant PGM captures
  - `internal-matrix/` — internal invariant matrix (`internal_invariants.md`, `internal_matrix.md`, PGM captures)
- **Good for:** Behavioral parity verification, regression gating, M10 semantic protection.

---

## D. Source code references

### D1. ReDMCSB reconstructed source
- **Local path:** `dm7z-extract/Toolchains/Common/Source/`
- **Files:** `VIDEODRV.C`, `DUNGEON.C`, `DEFS.H`
- **Good for:** Authoritative structural reference for UI composition, component model, panel ownership, graphics slot use, event naming, gameplay mechanics, memory/cache behavior.

### D2. Firestaff compat layers (ReDMCSB PC 3.4 port)
- **Local path:** repo root — 270 `.c` files, 121 `.h` files
- **Key layers:**
  - `*_pc34_compat.{c,h}` — ported compat layer files (byteops, image backend, memory cache, GRAPHICS.DAT handling, etc.)
  - `*_frontend_pc34.{c,h}` — extracted frontend layers
  - `redmcsb_m9_beta_harness.c` — frozen M9 beta harness
  - `run_redmcsb_m9_verify.sh` — M9 verification gate
- **Good for:** Current implementation state, identifying where Firestaff diverges from original behavior.

### D3. Phase11/phase-ref source material
- **Local paths:** `phase11-fixtures/` (sensor catalog), `phase11-ref/` (MOVESENS.C), `shims/` (DOS compatibility headers)
- **Good for:** Sensor execution parity, DOS-layer compatibility verification.

---

## E. Probe and analysis scripts
- **Local path:** repo root — 136 `.py` files, 135 `.sh` files
- **Key categories:**
  - `build_redmcsb_*.py` — probe construction scripts for specific compat layers
  - `analyze_redmcsb_*.py` — analysis scripts for runtime tail behavior, branch metrics, title fidelity
  - `run_redmcsb_*.sh` — execution harnesses and verification gates
  - `capture_firestaff_*.c` — Firestaff screenshot capture programs
- **Good for:** Reproducible parity evidence generation, regression verification, automated comparison.

---

## F. Documentation and plans
- **Local paths:**
  - `MASTER_1TO1_PARITY_PLAN.md` — master parity plan (W1–W10 workstreams)
  - `MILESTONES.md` — milestone ladder (M0–M10)
  - `STATUS.md` — current project status
  - `NEXT_STEPS.md` — immediate work focus
  - `redmcsb_original_data_inventory_2026-04-16.md` — original data inventory
  - `dm7z_integration_2026-04-19.md` — dm7z archive integration report
  - `dungeon_format_notes_2026-04-19.md`, `dungeon_format_confirmation_2026-04-19.md` — dungeon file format
  - `BUGFIX_TOGGLE_SPEC.md` — bug-profile toggle specification
  - Various `redmcsb_*.md` — per-probe and per-analysis reports (200+ files)
- **Good for:** Decision audit trail, parity state tracking, understanding what has already been investigated.

---

## G. What is missing (known gaps)

| Gap | Impact | Status |
|-----|--------|--------|
| CSB original data files | Cannot begin CSB parity work | `BLOCKED_ON_REFERENCE` |
| DM2 original data files | Cannot begin DM2 parity work | `BLOCKED_ON_REFERENCE` |
| CSBwin source/build | Cross-check reference for CSB behavior | `BLOCKED_ON_REFERENCE` |
| Original DM1 runtime screenshots | No pixel-level original captures for direct overlay comparison yet | `UNPROVEN` — can be generated via emulator |
| Original audio samples | Only procedural placeholders exist in Firestaff | `BLOCKED_ON_REFERENCE` |
| DM2 ReDMCSB source equivalent | No reconstructed source for DM2 engine | `BLOCKED_ON_REFERENCE` |
| Original timing measurements | No frame/tick timing from original runtime | `UNPROVEN` — requires emulator instrumentation |
