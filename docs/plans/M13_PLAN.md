# Firestaff M13 — CSB Integration, Replay UI, and Completion Polish

**Status:** PLAN (not started)
**Created:** 2026-04-21
**Depends on:** M11 (first playable, ~70%), M12 (launcher/config, ~30%)
**Goal:** Expand Firestaff from a DM1-only player into a multi-game platform with replay capability and preservation content.

---

## Table of Contents

1. [§1 Scope](#1-scope)
2. [§2 Prerequisites — What Is Already Built](#2-prerequisites--what-is-already-built)
3. [§3 Ordered Slices](#3-ordered-slices)
4. [§4 Verification Strategy](#4-verification-strategy)
5. [§5 Risk Register](#5-risk-register)
6. [§6 Open Questions](#6-open-questions)

---

## §1 Scope

### 1.1 What M13 Is

M13 is the **multi-game and replay milestone**. It extends Firestaff beyond a DM1-only engine into a platform that can host Chaos Strikes Back (CSB), exposes the M10 tick-stream recording infrastructure through a user-facing replay UI, and adds preservation/credits content.

When M13 is done:
- CSB (Atari ST 2.1 EN baseline) is playable through the same launcher as DM1
- The replay system lets players record, save, load, and play back tick-stream files
- A credits/lore section exists in the launcher
- M11 remaining work (audio, CMake, CI, packaging) is complete
- M12 remaining work (bug-toggle UI, i18n engine, credits screen) is complete

### 1.2 In Scope

| Area | Deliverable |
|------|-------------|
| **CSB data layer** | CSB dungeon loader, CSB-specific creature roster, CSB item/spell deltas |
| **CSB presentation** | CSB GRAPHICS.DAT support, CSB-specific viewport assets, CSB title screen |
| **CSB launcher integration** | Game picker enables CSB when assets are validated, CSB-specific bug toggles |
| **Replay recording** | Wire `TickStreamRecord_Compat` into game view; auto-record during play |
| **Replay file I/O** | Save/load `.fsr` (Firestaff Replay) files; header with version, flags, world-hash |
| **Replay playback UI** | Load replay, play/pause/step, speed control, seek-to-tick |
| **Lore Museum** | Credits + FTL history section in launcher; static content, no external dependencies |
| **M11 completion** | Audio (SDL3_mixer), CMake build, CI on all 3 platforms, packaging |
| **M12 completion** | Bug-toggle UI (v1: 20 representative entries), i18n engine + en/sv .po, credits screen |

### 1.3 Out of Scope (Deferred)

| Deferral | Target |
|----------|--------|
| DM2 (Dungeon Master II: The Legend of Skullkeep) | M14+ |
| Custom dungeon editor / loading | M14+ |
| Full 201-entry bug-toggle database | M14 |
| French/German translations | M14 |
| Online leaderboards / replay sharing | Far future |
| Co-op / networking | Far future |
| Version 2 (hi-res graphics) | V2 milestone |
| Version 3 (3D) | V3 milestone |

---

## §2 Prerequisites — What Is Already Built

### From M10 (complete, 20/20 phases, 500+ invariants)

| Prerequisite | Files | Status |
|--------------|-------|--------|
| Deterministic game loop | `memory_tick_orchestrator_pc34_compat.{c,h}` | ✅ Complete |
| Tick-stream record struct | `TickStreamRecord_Compat` (serialize/deserialize) | ✅ Complete |
| Save/load with CRC32 | `memory_savegame_pc34_compat.{c,h}` | ✅ Complete |
| Combat system (45 invariants) | Phase 13 | ✅ Complete |
| Magic system (45 invariants) | Phase 14 | ✅ Complete |
| Creature AI (3 types) | Phase 16 | ✅ Complete |
| Projectile flight | Phase 17 | ✅ Complete |
| Champion lifecycle | Phase 18 | ✅ Complete |
| Runtime dynamics | Phase 19 | ✅ Complete |
| Bug-flags bit-mask | `bug_flags_compat` in M10 tick orchestrator | ✅ Exists |

### From M11 (~70% complete)

| Prerequisite | Status | Notes |
|--------------|--------|-------|
| SDL3 window + framebuffer | ✅ Done | 18/18 Phase-A invariants |
| GRAPHICS.DAT asset loader | ✅ Done | Full pipeline |
| VGA palette | ✅ Done | `vga_palette_pc34_compat.{h,c}` |
| Dungeon viewport + depth rendering | ✅ Done | Real textures, walls, doors, stairs, creatures |
| Movement, combat, spells, inventory | ✅ Done | 115/115 game-view invariants |
| Audio (SDL3_mixer) | ❌ Not started | Needed for M13 completion |
| CMake build system | ❌ Not started | Needed for M13 completion |
| CI on Linux/Windows | ❌ Not started | Needed for M13 completion |
| Packaging (DMG/AppImage/NSIS) | ❌ Not started | Needed for M13 completion |

### From M12 (~30% complete)

| Prerequisite | Status | Notes |
|--------------|--------|-------|
| Launcher game picker (3 slots) | ✅ Done | DM1 active, CSB/DM2 greyed |
| MD5 asset validation | ✅ Done | `asset_status_m12.{c,h}` |
| Config persistence (TOML) | ✅ Done | `config_m12.{c,h}` |
| Settings screen | ✅ Done | Keyboard navigation works |
| Bug-toggle UI | ❌ Not started | Needed for M13 completion |
| i18n engine | ❌ Not started | Needed for M13 completion |
| Credits screen | ❌ Not started | Needed for M13 completion |

---

## §3 Ordered Slices

### Slice 1: M11 Completion — Audio, Build, CI, Packaging

**Rationale:** Must be done before M13 new features; CSB and replay work depend on a stable cross-platform build.

#### 1A: Audio Integration (SDL3_mixer)

**Target files:**
- New: `audio_sdl_m11.c`, `audio_sdl_m11.h`
- Edit: `main_loop_m11.c` (wire audio tick), `m11_game_view.c` (emit sound events)

**Implementation notes:**
- Load WAV/VOC audio assets from DUNGEON.DAT via the existing asset loader pipeline
- Map M10 sound emission markers (footstep, door, combat, creature, spell) to SDL3_mixer channels
- Volume mixer: master / SFX / music / UI, driven by `config_m12` values
- Spatial audio hint: attenuate by distance from party to source cell
- If SDL3_mixer is unavailable, fall back gracefully (no audio, no crash)

**Invariants to add:**
- `INV_M11_AUDIO_01`: Audio subsystem initializes without crash when SDL3_mixer is present
- `INV_M11_AUDIO_02`: Sound emission from M10 tick maps to non-zero SDL channel play
- `INV_M11_AUDIO_03`: Volume change via config reflects in mixer state
- `INV_M11_AUDIO_04`: Graceful fallback when SDL3_mixer is absent

**Verification:**
```bash
run_firestaff_m11_audio_probe.sh
# Must print: 4/4 invariants passed
```

**Risks:** SDL3_mixer API may differ from SDL2_mixer; check availability on CI runners.

**Dependencies:** None beyond existing M11 baseline.

#### 1B: CMake Build System

**Target files:**
- New: `CMakeLists.txt` (top-level), `cmake/FindSDL3.cmake`, `cmake/FindSDL3_mixer.cmake`

**Implementation notes:**
- Replace shell-script compilation with CMake targets
- Targets: `firestaff` (main executable), `firestaff_m10_verify` (M10 probe suite), `firestaff_m11_verify` (M11 probes), `firestaff_m12_verify` (M12 probes)
- SDL3 detection with SDL2 fallback (`find_package(SDL3 QUIET)`, then `find_package(SDL2)`)
- Platform conditionals for macOS frameworks, Linux pkg-config, Windows vcpkg
- Out-of-source build: `mkdir build && cd build && cmake .. && make -j$(nproc)`
- Preserve existing shell scripts as wrappers around CMake targets for backward compat

**Invariants to add:**
- `INV_M11_CMAKE_01`: `cmake --build` succeeds on current platform
- `INV_M11_CMAKE_02`: `firestaff_m10_verify` target runs and passes
- `INV_M11_CMAKE_03`: `firestaff_m11_verify` target runs and passes

**Verification:**
```bash
mkdir -p build && cd build && cmake .. && cmake --build . --target firestaff_m10_verify
# Must exit 0
```

**Risks:** SDL3 may not be in system package managers yet on all CI runners; bundle SDL3 headers or use FetchContent.

#### 1C: CI (GitHub Actions)

**Target files:**
- New: `.github/workflows/build.yml`

**Implementation notes:**
- Matrix: `{os: [macos-latest, ubuntu-latest, windows-latest]}`
- Steps: install SDL3 (brew / apt / vcpkg), cmake configure, build, run M10 verify, run M11 verify
- Cross-platform hash check: run M10 tick orchestrator probe on all 3, compare world-hash output
- Cache SDL3 builds to speed up CI

**Invariants:**
- `INV_CI_01`: All three platforms produce identical M10 world-hash for the same 1000-tick run

**Risks:** Windows SDL3 setup may require manual vcpkg bootstrap.

#### 1D: Packaging

**Target files:**
- New: `packaging/macos/`, `packaging/linux/`, `packaging/windows/`

**Implementation notes:**
- macOS: CMake `install(TARGETS)` + `cpack -G DragNDrop` for DMG
- Linux: AppImage via `linuxdeploy`, `.deb` via `cpack -G DEB`
- Windows: NSIS installer via `cpack -G NSIS`, portable ZIP via `cpack -G ZIP`
- All packages include `LICENSE`, `README.md`, branding assets

**Invariants:**
- `INV_PKG_01`: macOS DMG contains signed `Firestaff.app`
- `INV_PKG_02`: Linux AppImage launches and renders first frame
- `INV_PKG_03`: Windows installer creates start-menu entry

**Risks:** Code signing requires Apple Developer ID and Windows Authenticode cert. Can ship unsigned for beta.

**Dependencies:** CMake (1B) must land first.

---

### Slice 2: M12 Completion — Bug Toggles, i18n, Credits

**Rationale:** Launcher must be feature-complete before CSB integration adds a second active game.

#### 2A: Bug-Toggle UI (v1)

**Target files:**
- New: `bug_toggle_m12.c`, `bug_toggle_m12.h`, `bug_database_m12.json`
- Edit: `menu_startup_m12.c` (add bug-toggle view), `config_m12.c` (persist toggle state)

**Implementation notes:**
- Start with 20 representative bug entries from Fontanel's 201-entry database
- Include the highest-impact DM1 bugs: BUG0_02 (time overflow), BUG0_03 (darkness glitch), BUG0_08 (thing pool), BUG0_09 (discard sensor), BUG0_38 (cursed items)
- Version presets: "DM 1.0a Purist", "CSB 2.1 Latest", "Modern (all fixes)"
- Each toggle maps to a bit in `bug_flags_compat` consumed by M10's tick orchestrator
- Save files embed the active flag-set; loading warns on mismatch
- Per-game gating: DM1 bugs shown only when DM1 selected

**Invariants to add:**
- `INV_M12_BT_01`: Default preset produces expected flag bitmask
- `INV_M12_BT_02`: Custom toggle change reflects in config and persists
- `INV_M12_BT_03`: Version preset switch updates all flags atomically
- `INV_M12_BT_04`: Per-game gating hides CSB-only bugs when DM1 is selected
- `INV_M12_BT_05`: Save file embeds active flags; load detects mismatch

**Verification:**
```bash
run_firestaff_m12_bug_toggle_probe.sh
# Must print: 5/5 invariants passed
```

**Dependencies:** `config_m12` (already done), `menu_startup_m12` (already done).

#### 2B: i18n Engine + en/sv .po Files

**Target files:**
- New: `i18n_m12.c`, `i18n_m12.h`, `locale/en.po`, `locale/sv.po`
- Edit: `menu_startup_m12.c` (replace hardcoded strings), `m11_game_view.c` (UI text)

**Implementation notes:**
- Minimal .po parser: read `msgid`/`msgstr` pairs, store in hash map
- `i18n_get(key)` returns translated string or falls back to English
- Runtime language switch from settings screen (no restart required)
- Start with ~100 UI strings (menu labels, settings, error messages)
- Swedish translations by Daniel Nylander

**Invariants to add:**
- `INV_M12_I18N_01`: English lookup returns non-empty for all registered keys
- `INV_M12_I18N_02`: Swedish lookup returns non-empty for ≥90% of keys
- `INV_M12_I18N_03`: Language switch mid-session updates rendered text
- `INV_M12_I18N_04`: Missing key falls back to English without crash

**Verification:**
```bash
run_firestaff_m12_i18n_probe.sh
# Must print: 4/4 invariants passed
```

**Dependencies:** None beyond existing M12 baseline.

#### 2C: Credits / About Screen

**Target files:**
- Edit: `menu_startup_m12.c` (add credits view)

**Implementation notes:**
- Static text screen accessible from launcher settings
- Content: Firestaff version, M10 engine version, FTL Games credits (Doug Bell, Andy Jaros, Dennis Walker, Wayne Holder), Christophe Fontanel (ReDMCSB), open-source license
- Render using existing M11 text/font pipeline
- Scrollable if content exceeds viewport

**Invariants to add:**
- `INV_M12_CREDITS_01`: Credits view opens and renders non-empty text
- `INV_M12_CREDITS_02`: Escape returns to previous view

**Verification:**
```bash
run_firestaff_m12_credits_probe.sh
# Must print: 2/2 invariants passed
```

**Dependencies:** `menu_startup_m12.c` (already done).

---

### Slice 3: CSB Data Layer

**Rationale:** CSB shares ~85% of DM1's engine but has dungeon format differences, new creatures, new items, and new spells. The data layer must be extended before CSB can render.

#### 3A: CSB Dungeon Loader

**Target files:**
- New: `dungeon_csb_loader_pc34_compat.c`, `dungeon_csb_loader_pc34_compat.h`
- Edit: `memory_dungeon_header_pc34_compat.c` (detect CSB format via header magic/size)

**Implementation notes:**
- CSB DUNGEON.DAT has a different header: 24 levels (vs 14), different creature table offsets, and an extended item table
- Reuse M10's `DungeonHeader_Compat` parser but add CSB-specific field offsets via a `GameVariant_Compat` enum (`GAME_DM1_PC34`, `GAME_CSB_ST21`)
- Level count, creature type count, and item type count are variant-driven rather than hardcoded
- CSB has `CHAOS.DAT` as an additional data file alongside `DUNGEON.DAT`

**Invariants to add:**
- `INV_M13_CSB_01`: CSB header parses without error when CSB DUNGEON.DAT is present
- `INV_M13_CSB_02`: Level count reads as 24 for CSB vs 14 for DM1
- `INV_M13_CSB_03`: CSB creature table offset differs from DM1 and reads valid entries
- `INV_M13_CSB_04`: Variant enum round-trips through save/load
- `INV_M13_CSB_05`: DM1 dungeon still loads identically with variant-aware code (regression guard)

**Verification:**
```bash
run_firestaff_m13_csb_dungeon_probe.sh
# Must print: 5/5 invariants passed
# Requires: CSB DUNGEON.DAT in $FIRESTAFF_DATA/csb/
```

**Risks:**
- CSB data files may not be available on CI; probe must skip gracefully when absent
- CSB header exact byte layout needs verification against a real CSB data file; cross-check with dmweb.free.fr documentation

**Dependencies:** M10 dungeon phases (complete).

#### 3B: CSB Creature / Item / Spell Deltas

**Target files:**
- New: `creature_csb_delta_pc34_compat.c`, `creature_csb_delta_pc34_compat.h`
- New: `item_csb_delta_pc34_compat.c`, `item_csb_delta_pc34_compat.h`
- Edit: `memory_combat_pc34_compat.c` (variant-gated creature stats), `memory_magic_pc34_compat.c` (CSB spell variants)

**Implementation notes:**
- CSB adds ~10 new creature types beyond DM1's roster
- CSB changes some spell power values and adds the "ZO KATH RA" fireball variant
- CSB items include new weapons and armor with different stat blocks
- All deltas are gated by `GameVariant_Compat`; DM1 behavior is untouched when variant is DM1
- New creatures beyond M10 Phase 16's 3 types: use the same creature-tick framework, add stats tables

**Invariants to add:**
- `INV_M13_CSB_06`: CSB creature type count exceeds DM1 count
- `INV_M13_CSB_07`: DM1 creature stats unchanged when variant = DM1 (regression)
- `INV_M13_CSB_08`: CSB-specific creature resolves valid attack/defense/HP
- `INV_M13_CSB_09`: CSB spell variant resolves different power for shared spell names
- `INV_M13_CSB_10`: CSB-specific item resolves valid weight/damage/defense

**Verification:**
```bash
run_firestaff_m13_csb_delta_probe.sh
# Must print: 5/5 invariants passed
```

**Dependencies:** Slice 3A (CSB dungeon loader).

---

### Slice 4: CSB Presentation and Launcher Integration

#### 4A: CSB GRAPHICS.DAT Support

**Target files:**
- Edit: `asset_loader_m11.c` (variant-aware GRAPHICS.DAT path selection)
- Edit: `memory_graphics_dat_header_pc34_compat.c` (CSB header differences if any)
- New: `csb_asset_map_m13.c`, `csb_asset_map_m13.h` (CSB-specific graphic index mapping)

**Implementation notes:**
- CSB GRAPHICS.DAT has a different entry count and different graphic indices for shared concepts (walls, creatures, UI)
- Create a variant-driven asset map that translates logical asset names (e.g., `ASSET_WALL_STONE_FRONT`) to variant-specific graphic indices
- Title screen: CSB uses a different title graphic than DM1; load from CSB GRAPHICS.DAT index
- Shared code path: `asset_loader_m11` already handles DM1 GRAPHICS.DAT; extend with CSB path from config

**Invariants to add:**
- `INV_M13_CSB_11`: CSB GRAPHICS.DAT header parses and reports different entry count from DM1
- `INV_M13_CSB_12`: Logical asset lookup returns valid graphic for CSB variant
- `INV_M13_CSB_13`: DM1 logical asset lookup unchanged (regression)
- `INV_M13_CSB_14`: CSB title screen graphic loads and exports non-empty PGM

**Verification:**
```bash
run_firestaff_m13_csb_assets_probe.sh
# Must print: 4/4 invariants passed
```

**Risks:** CSB GRAPHICS.DAT might use a slightly different compression scheme; needs binary comparison.

**Dependencies:** M11 asset loader (done), Slice 3A.

#### 4B: CSB Launcher Integration

**Target files:**
- Edit: `menu_startup_m12.c` (enable CSB slot when assets valid)
- Edit: `asset_status_m12.c` (add CSB MD5 checksums)
- Edit: `asset_validator_checksums_m12.json` (add CSB entries)
- Edit: `config_m12.c` (CSB data path, CSB-specific settings)

**Implementation notes:**
- When CSB DUNGEON.DAT + GRAPHICS.DAT pass MD5 validation, the CSB card in the game picker becomes active (not greyed out)
- CSB-specific bug toggles: BUG7_xx entries only visible when CSB is selected
- Game launch from CSB card sets `GameVariant_Compat = GAME_CSB_ST21` and passes CSB data paths
- Card art: provide a built-in CSB card art alongside DM1's existing card art

**Invariants to add:**
- `INV_M13_CSB_15`: CSB card enabled when CSB assets present
- `INV_M13_CSB_16`: CSB card greyed when CSB assets absent
- `INV_M13_CSB_17`: CSB launch sets correct variant enum
- `INV_M13_CSB_18`: Bug-toggle view shows CSB-specific entries only for CSB

**Verification:**
```bash
run_firestaff_m13_csb_launcher_probe.sh
# Must print: 4/4 invariants passed
```

**Dependencies:** Slice 2A (bug toggles), Slice 3A, 4A.

---

### Slice 5: Replay System

#### 5A: Replay Recording

**Target files:**
- New: `replay_record_m13.c`, `replay_record_m13.h`
- Edit: `main_loop_m11.c` (hook tick-stream recording into game loop)
- Edit: `m11_game_view.c` (start/stop recording on key or auto-start)

**Implementation notes:**
- M10's `TickStreamRecord_Compat` already contains per-tick input + world-hash
- Recording: allocate a growable buffer of `TickStreamRecord_Compat`; append after each tick
- Auto-record by default; manual toggle via F9 key
- Recording metadata: game variant, bug-flag bitmask, initial save-state hash, timestamp
- Memory budget: cap at 100,000 ticks (~28 hours at 1 tick/sec), then ring-buffer oldest

**Invariants to add:**
- `INV_M13_REPLAY_01`: Recording starts on game-view entry and appends records per tick
- `INV_M13_REPLAY_02`: Record count matches tick count after N ticks
- `INV_M13_REPLAY_03`: Each record's world-hash matches M10's deterministic computation
- `INV_M13_REPLAY_04`: Ring-buffer wraps without crash at capacity

**Verification:**
```bash
run_firestaff_m13_replay_record_probe.sh
# Must print: 4/4 invariants passed
```

**Dependencies:** M10 tick orchestrator (done), M11 game loop (done).

#### 5B: Replay File I/O

**Target files:**
- New: `replay_io_m13.c`, `replay_io_m13.h`

**Implementation notes:**
- File format `.fsr` (Firestaff Replay):
  - Header: magic bytes `FSR1`, version u16, variant enum u8, bug-flags u128, initial world-hash u32, tick-count u32, timestamp u64
  - Body: array of serialized `TickStreamRecord_Compat` (already has serialize/deserialize in M10)
  - Footer: CRC32 over header + body
- Atomic write: write to `.fsr.tmp`, fsync, rename
- Load: validate CRC32, check version compatibility, deserialize records

**Invariants to add:**
- `INV_M13_REPLAY_05`: Write + read round-trip produces identical records
- `INV_M13_REPLAY_06`: CRC32 mismatch detected on corrupted file
- `INV_M13_REPLAY_07`: Version mismatch produces clear error, not crash
- `INV_M13_REPLAY_08`: File with 10,000 records round-trips in <1 second

**Verification:**
```bash
run_firestaff_m13_replay_io_probe.sh
# Must print: 4/4 invariants passed
```

**Dependencies:** Slice 5A.

#### 5C: Replay Playback UI

**Target files:**
- New: `replay_playback_m13.c`, `replay_playback_m13.h`
- Edit: `main_loop_m11.c` (add replay playback mode)
- Edit: `menu_startup_m12.c` (add "Load Replay" option)

**Implementation notes:**
- Playback mode: load `.fsr`, create initial world state from save, feed recorded inputs tick-by-tick
- Controls: Play / Pause (Space), Step forward (Right), Speed 1x/2x/4x/8x (1-4 keys), Seek to tick (type number + Enter)
- HUD overlay: current tick / total ticks, speed indicator, "REPLAY" badge
- Verification mode: compare world-hash at each tick against recorded hash; halt on divergence with diagnostic
- Exit replay: Escape returns to launcher

**Invariants to add:**
- `INV_M13_REPLAY_09`: Playback of 100-tick recording produces identical world-hash sequence
- `INV_M13_REPLAY_10`: Pause halts tick advancement; unpause resumes
- `INV_M13_REPLAY_11`: Speed change affects tick rate proportionally
- `INV_M13_REPLAY_12`: Hash divergence halts playback with diagnostic message

**Verification:**
```bash
run_firestaff_m13_replay_playback_probe.sh
# Must print: 4/4 invariants passed
```

**Dependencies:** Slice 5A, 5B, M11 game view (done).

---

### Slice 6: Lore Museum / Preservation Section

**Target files:**
- New: `lore_museum_m13.c`, `lore_museum_m13.h`
- Edit: `menu_startup_m12.c` (add "Museum" entry in launcher)

**Implementation notes:**
- Static content section in the launcher, keyboard-navigable
- Pages:
  1. **FTL Games History** — founding, the DM series, key people (Doug Bell, Andy Jaros, Dennis Walker, Wayne Holder)
  2. **The Making of DM** — original development timeline, Atari ST origins, PC port, CSB
  3. **Christophe Fontanel & ReDMCSB** — the reverse-engineering effort, the documentation, the community
  4. **Firestaff Credits** — contributors, license, links
- Text content embedded as C string arrays (no external file dependency)
- Simple scrollable text renderer using M11's font pipeline
- Optional: embed 2-3 low-res historical images (box art thumbnails) as C arrays if licensing allows

**Invariants to add:**
- `INV_M13_MUSEUM_01`: Museum view opens and renders non-empty content
- `INV_M13_MUSEUM_02`: Page navigation cycles through all pages
- `INV_M13_MUSEUM_03`: Escape returns to launcher

**Verification:**
```bash
run_firestaff_m13_museum_probe.sh
# Must print: 3/3 invariants passed
```

**Dependencies:** M12 launcher (done).

---

## §4 Verification Strategy

### Gate structure

Each slice has its own probe script following the established pattern:
1. Shell script compiles probe + dependencies
2. Runs binary, captures log
3. Checks `# summary: N/N invariants passed`
4. Exit 1 on any failure

### Top-level M13 verification gate

```bash
run_firestaff_m13_verify.sh
```

This script runs all M13 probe scripts in order and reports aggregate pass/fail. It also re-runs `run_firestaff_m10_verify.sh`, `run_firestaff_m11_verify.sh`, and `run_firestaff_m12_verify.sh` as regression guards.

### Invariant summary

| Slice | Invariants | Prefix |
|-------|-----------|--------|
| 1A Audio | 4 | INV_M11_AUDIO |
| 1B CMake | 3 | INV_M11_CMAKE |
| 1C CI | 1 | INV_CI |
| 1D Packaging | 3 | INV_PKG |
| 2A Bug Toggles | 5 | INV_M12_BT |
| 2B i18n | 4 | INV_M12_I18N |
| 2C Credits | 2 | INV_M12_CREDITS |
| 3A CSB Dungeon | 5 | INV_M13_CSB |
| 3B CSB Deltas | 5 | INV_M13_CSB |
| 4A CSB Assets | 4 | INV_M13_CSB |
| 4B CSB Launcher | 4 | INV_M13_CSB |
| 5A Replay Record | 4 | INV_M13_REPLAY |
| 5B Replay I/O | 4 | INV_M13_REPLAY |
| 5C Replay Playback | 4 | INV_M13_REPLAY |
| 6 Lore Museum | 3 | INV_M13_MUSEUM |
| **Total** | **55** | |

---

## §5 Risk Register

| Risk | Severity | Mitigation |
|------|----------|------------|
| CSB data files unavailable for CI | Medium | Probes skip gracefully; CSB tests require `$FIRESTAFF_DATA/csb/` |
| CSB dungeon format undocumented differences | High | Cross-reference dmweb.free.fr, Fontanel's ReDMCSB source, and Greatstone's documentation |
| SDL3 not in CI package managers | Medium | Use FetchContent or pre-built SDL3 cache |
| CSB creature/item balance not verified | Medium | Port Fontanel's known CSB stat tables directly |
| Replay file size for long sessions | Low | Ring-buffer + compression; `.fsr` header stores tick range |
| Code signing for packaging | Low | Ship unsigned for beta; signing is a release-time task |
| i18n .po parser edge cases | Low | Test with malformed files; keep parser minimal |
| CSB GRAPHICS.DAT compression variant | Medium | Binary compare first 100 entries; same IMG3 expander likely works |

---

## §6 Open Questions

1. **CSB exact header layout:** Needs binary verification against a real CSB DUNGEON.DAT. The M10 header parser assumes DM1 PC 3.4 offsets. CSB Atari ST 2.1 offsets must be documented.

2. **CSB creature roster completeness:** M10 Phase 16 implements 3 creature types. CSB needs ~13 more. How many must be implemented for M13 vs deferred to M14?
   - **Proposed answer:** Implement the 5-7 most common CSB creatures for M13; defer rare/boss creatures to M14.

3. **Replay initial state:** The `.fsr` header stores an initial world-hash. Should it also embed the full save-state, or reference an external `.dat` file?
   - **Proposed answer:** Reference external save file by hash; embedding the full state bloats replays unnecessarily.

4. **Bug-toggle scope for CSB:** Fontanel documents CSB-specific bugs (BUG7_xx). How many should be in the v1 toggle set?
   - **Proposed answer:** Add 5 representative CSB entries in M13; expand to full set in M14.

5. **M11 audio priority:** Is audio a hard gate for M13, or can CSB/replay work proceed with silent audio?
   - **Proposed answer:** Audio is a completion criterion but not a blocker for CSB/replay implementation. Slices can overlap.

---

## Execution Order Summary

```
Slice 1A (Audio) ──────────────────┐
Slice 1B (CMake) ──────────────────┤
                                    ├── Slice 1C (CI) ── Slice 1D (Packaging)
Slice 2A (Bug Toggles) ───────────┤
Slice 2B (i18n) ───────────────────┤
Slice 2C (Credits) ────────────────┘
                                    
Slice 3A (CSB Dungeon) ── Slice 3B (CSB Deltas) ── Slice 4A (CSB Assets)
                                                           │
                                                    Slice 4B (CSB Launcher)

Slice 5A (Replay Record) ── Slice 5B (Replay I/O) ── Slice 5C (Replay Playback)

Slice 6 (Lore Museum) ── independent, can run any time after M12 launcher exists
```

**Parallelism:**
- Slices 1-2 (M11/M12 completion) can run in parallel with Slices 3-5 (new M13 features)
- Slice 6 is fully independent
- Slice 4B depends on both Slice 2A and Slice 4A
- Slice 5C depends on 5A + 5B
- CI (1C) depends on CMake (1B)
- Packaging (1D) depends on CI (1C)

---

*End of M13 Plan. Version 1.0, 2026-04-21.*
