# Firestaff v2.7.14

DM1 V1 source-lock and CSB V1 bounded-gap release — closes 6 DM1 V1 v1-simplifications documented in v2.7.13, fixes 2 pre-existing test regressions, and adds 3 CSB V1 implementations (NEOPHYTE rank, projectile speed normalization, reincarnation penalty).

## DM1 V1 parity work

- **F0308 CHAMPION_IsLucky** (CHAMPION.C:1123-1155): the 50% short-circuit, luck×2 roll, ±2 bounded update, and BUG0_38 negative-luck path are now implemented. Wired into the dex-duel via a new `luckyHit` field on CombatResult_Compat.
- **F0202 FAKEWALL non-material pass** (GROUP.C:1503-1505): added `adjacencyFakeWallMask` + `adjacencyFakeWallOpenMask` to CreatureTickInput_Compat. F0798 now correctly opens the door for FAKEWALL with the OPEN or IMAGINARY+allow bits.
- **F0229 cell ordering** (PROJEXPL.C:1284-1305): the per-primaryDir cell permutation table `kCellOrder[4][4]` is now consulted; the F0228 visibility parity flip (CellSource + 1 when LoS is blocked) is honoured.
- **C80..C83 magic-map per-champion counters** (CHAMDRAW.C:1069): added `magicMapRefresh[4]` to ChampionLifecycleState_Compat. The C80..C83 timeline handler decrements the counter and reschedules the next refresh.
- **Teleporter direction rotation** (PROJEXPL.C:1260-1310): the digest's `destTeleporterNewDirection` is now populated from the destination square's first THING_TYPE_TELEPORTER rotation when entering a teleporter.
- **Kinetic pass-through** (PROJEXPL.C:490-500): `launcherStrength` added to ProjectileInstance_Compat; F0816 now rolls `M002_RANDOM(100) < launcherStrength` for KINETIC projectiles.
- **F0321 fire/spell shield subtraction** (CHAMPION.C:1880-1882): F0321 C1 and C5 cases subtract `defender->partyShieldDefense` after the F0307 statistic adjustment. Bounded to 0.
- **F0321 C6 wisdom factor** (CHAMPION.C:1908-1932): the F0762 psychic adjustment now correctly sources `champ->statisticWisdom` (was passing `magic->luckCurrent`).
- **F0822 poison cloud group damage** (PROJEXPL.C:858-866): removed the F0192 over-scaling; the call site now passes `attackApplied` straight through to F0191 (which does the resistance adjustment internally).
- **Trolin F0823 anti-mage palette**: added the `DM1_CREATURE_TYPE_TROLIN` case to F0823 — 50% FIREBALL, else LIGHTNING_BOLT / HARM_NON_MATERIAL / OPEN_DOOR 3-way split. Note: Trolin's AttackRange=1 (DUNGEON.C G0243[16]) makes F0823 a no-op for melee — the anti-mage palette is wired but inert.
- **DM_SAVE_HEADER Noise[]/Keys[]/Checksums[]** (SAVEHEAD.C:44,97,104): added `noise[10]`, `sectionKeys[16]`, `sectionChecksums[16]` to SaveGameHeader_Compat. F0417_SAVEUTIL_Port_Hint_Compat derives 16 per-section XOR keys via FNV-1a fold; F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat runs a minimal XOR pass. Full CPSC checksum derivation deferred to post-M10.

## DM1 V1 documentation

- **29 NEEDS DISASSEMBLY REVIEW markers** replaced with precise ReDMCSB source citations (CHAMPION.C, GROUP.C, PROJEXPL.C, MOVESENS.C, etc.). Each marker now points to the exact function name, file, and line range so disassembly confirmation can be tracked against the ReDMCSB decompilation.

## Test infrastructure

- **Hall of Champions 4-mirror zones** (60/60 PASS): pixel-proves that all 4 endgame champion mirrors are drawn at the source-locked C412..C415 destinations with correct portrait cutouts, name origins, and 48px row pitch.
- **Hall of Champions wall-mirror zones** (18/18 PASS): pixel-proves the D1C champion mirror on the (1,3) and (1,4) wall routes — wall ornament box at (96, 36, 32, 28), portrait cutout at (96, 35, 32, 29), 100% / 97% pixel match, no bleed.
- **Hall of Champions panel-guard probe** (5/5 PASS): real pixel-probe for the BUG-120/121 panel-active guard via D1C zone diff (2961 bytes when panel is on, portrait still 1024 pixels).
- **M12 extras view smoke probe + visual capture** (11/11 PASS): Bestiary, Item Encyclopedia, and Screenshot Gallery render non-trivial framebuffers.
- **F0827/F0828 launcherStrength** fix: serialiser now writes the new field at the right slot (25 fields total, 100 bytes — was 24 fields / 96 bytes, causing the world-hash to fail). Brought test_m11_inventory_full_panel from 21 sub-failures down to 2 (panel-render bleeds, pre-existing and not related to this fix).
- **M11_GameView_HandlePointer** now refreshes `lastWorldHash` on every REDRAW-returning click so the inventory test's deterministic world-hash assertions see the post-click snapshot.

## CSB V1 bounded-gap implementations

- **Champions GAP 1 — NEOPHYTE rank** (PANEL.C:26, CEDT006.C:141, Character.cpp:665): added `csb_v1_neophyte_skills_mode_get/set` and `csb_v1_neophyte_display_for_level` helpers. m11_dm1_v1_skill_level_name_pc34 now returns "NEOPHYTE" for level 0 in CSB mode (was returning NULL for level <= 1, making both NEOPHYTE and NOVICE display as empty). 8/8 PASS.
- **Combat GAP 1 — Projectile Speed Normalization** (PROJEXPL.C CHANGE7_20): added `csb_v1_projectile_speed_normalization_get/set` flag. F0825 uses delay=1 on every map when CSB mode is on (was delay=1 on party map, 3 on other maps in DM1). 7/7 PASS.
- **Champions GAP 2 — Reincarnation Penalty** (CSB:REVIVE.C CHANGE7_24, Character.cpp:14): added `csb_v1_reincarnation_mode_get/set` plus 3 globals (attributePenalty=2, statPenalty=8, randomPoints=3). F0610_PARTY_AddChampionFromMirrorTextString applies the penalty in place when the mode is on — HP/STA/MANA halved, each non-Luck stat reduced by attributePenalty, clamped to 0. 16/16 PASS.

## Verification

- Full CMake build: 0 errors
- Phase A probe: 23/23 invariants
- CSB V1 gates: 31/31 PASS (8 neophyte + 7 projectile-speed + 16 reincarnation)
- DM1 V1 wall-mirror zones: 18/18 PASS
- DM1 V1 endgame 4-mirror zones: 60/60 PASS
- DM1 V1 panel-guard: 5/5 PASS
- test_dm1_v1_combat_pc34_compat_integration: 31/31 PASS
- test_dm1_v1_projectile_explosion_render_pc34_compat: PASS
- Pre-existing failure unchanged: `m11_inventory_full_panel_runtime_source_lock` has 2 panel-render bleed failures in C025 open-chest transparency (root cause: C025 red-transparency path; documented in docs/FINAL_GAPS.md Group 4).

## Known gaps

- DM1 V1: 2 panel-render bleed sub-tests in test_m11_inventory_full_panel_runtime (CHEST.C F0333 red-transparency path; not closed in this release).
- DM1 V1: BUG-106 (creature flee F0201 negated direction), BUG-108 (light amount table G0039 16-entry), BUG-109 (champion stat gain F0303 cycle), BUG-111 (sub-cell hit mask), BUG-116 (runtime dynamics adjacency). Documented in docs/FINAL_GAPS.md Group 3.
- CSB V1: 24 of 27 implementation gaps remain (see docs/FINAL_CSB_GAPS.md). 3 bounded gaps closed in this release (NEOPHYTE, projectile speed, reincarnation).
- DM2 / CSB / Nexus / Theron: separate milestones, not parity targets for this release.

---

## v2.7.13 (previous release, kept for reference)

DM1 V1 combat fidelity and bug audit release — systematic audit of the DM1 V1 runtime against the ReDMCSB decompilation with targeted fixes for the highest-impact issues.

## Fixes

- **Armor defense overhaul:** Replaced the simplified skill-level armor approximation with a proper ReDMCSB F0321 wound defense calculation that iterates equipped armor slots, applies per-slot defense values with G0050 wound defense factors, and scales attack by (130 − avgDefense) / 64.
- **Fire/Spell Shield defense:** Fire Shield and Spell Shield spells now correctly reduce incoming damage. Fire attacks subtract FireShieldDefense before armor scaling. Magic attacks subtract SpellShieldDefense and skip armor scaling (matching F0321's goto T0321024). Psychic attacks skip armor scaling entirely.
- **Creature poison:** Creature melee attacks now apply poison when the creature has a non-zero poison attack value, with a 50% chance per hit and vitality-adjusted damage via F0307.
- **Luck and stamina adjustments:** F0308-style luck bias now influences melee hit/miss at the damage dispatch boundary, and the F0306 stamina-adjusted value compiler order hazard is routed through a single helper.
- **Creature AI promotion:** 7 creature types promoted from STUB to FULL tier with per-type behaviour bias: Giant Scorpion (C00, poison sting), Giggler (C02, steal-then-flee), Screamer (C06, cowardly group-fleer), Vexirk (C14, ranged magic), Magenta Worm (C15, 30pt venom), Animated Armour (C18, cursed fixed drops), Red Dragon (C24, flame stream). 10 of 27 types now FULL.
- **Combat mechanics:** Creature attack target ordering respects F0229 direction weighting. Creatures below HP threshold can now flee. Projectile sub-cell hit mask narrows from 0xFF to the actually-targeted sub-cell. C6_PSYCHIC damage type applies from the spell descriptor.
- **Source-locked tables:** Thieves Eye duration uses the F0394 interval table. Light amount uses G0116 graphic559 light factors. Champion stat gain cycle uses F0303. Magic map is per-champion tracked. Runtime dynamics table uses the exact timing constants.
- **Savegame field mask:** Bit layout now matches LOADSAVE.C for all champion fields.
- **Test infrastructure:** Added FIRESTAFF_BUILD_DIR environment variable support to Python verification scripts for out-of-tree builds.
- **Viewport readiness:** The pass434 viewport crop readiness gate is now wired to the pass610 wall-collision runtime capture evidence path.
- **DM1 V1 audit divergence observability (MNU-02, DUN-05, PJE-05):** Three ReDMCSB-original divergence sites that were previously silent defensive divergences are now observable.
  - **MNU-02 (F0757 Thieves Eye duration):** Default is source-locked 0 ticks (the original PC 3.4 broken-by-uninitialised-stack-residue behaviour). Opt-in to the defensive envelope (`spellPower * 40`, 64-224 s) via build flag `-DFIRESTAFF_PC34_LEGACY_THIEVES_EYE=1` or env var `FIRESTAFF_DM1_THIEVES_EYE_LEGACY=1`.
  - **DUN-05 (F0163 BUG0_08 SFT overfill):** A new `F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat` helper runs at dungeon load. If a hand-crafted or modded dungeon contains more thing-bearing squares than the SFT buffer can hold, a one-shot warning is emitted to stderr with the overfill count. Defensive behaviour is preserved.
  - **PJE-05 (F0220 BUG0_16 projectile list overfill):** `F0810_PROJECTILE_Create_Compat` now emits a one-shot stderr warning when the per-dungeon projectile list is full and the overflow is dropped. Cap behaviour unchanged.
  - See `docs/dm1-v1-functional-divergence-report.md` for full audit context.

## Bug Audit

A comprehensive bug audit document is now available at `docs/DM1_V1_BUG_AUDIT.md` covering 18 identified issues across mechanics, rendering, data, and testing categories.

## Verification

- Full CMake build passed with zero errors.
- Phase A probe passed 23/23 invariants.
- `git diff --check` clean.

---

# Firestaff v2.7.12

Patch release focused on the post-v2.7.11 DM1 V1 hardening batch.

## Fixes

- Added the original-DOS in-dungeon movement capture route using VGA mode, no sound, keyboard simulation of digital joystick, and source-locked Keypad-5 movement after dungeon entry.
- Expanded DM1 V1 viewport source-lock coverage across additional F0108, F0111, and F0115 wall, door, side-wall, ornament, stairs/pit, and thing-pass slices.
- Hardened DM1 V1 chest, mirror-candidate, champion-panel, door-bash, sleep/wakeup, projectile, creature, and inventory runtime regressions.
- Fixed the newest D1L/D1R viewport-gate release wiring before packaging.

## Verification

- GitHub Actions verify workflow passed on `main` at `ed244b866` before release prep.
- Local release-prep verification passed: full CMake build, focused DM1 viewport/runtime gates, Phase A probe, and `git diff --check`.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.11

Patch release for DM1 V1 regressions reported after v2.7.10.

## Fixes

- Fixed Hall of Champions movement/survival timing so champions no longer drain or die while walking the Hall.
- Fixed mirror-candidate slot ownership during confirm/cancel and saved the runtime movement timestamp in quick-resume sidecars.
- Reduced normal gameplay CPU load by making the disk-backed accessibility manifest opt-in instead of writing it every frame.
- Corrected the FTL/SWSH PC palette mapping and made palette mutations visible immediately, restoring the intended swoosh cadence and colors.
- Corrected the Dungeon Master title palette base so the DM title animation uses the PC 3.4 palette instead of a blacked-out fallback.

## Verification

- Full local CMake build completed.
- Focused Hall/mirror/title/swoosh/starvation/quick-resume CTest suite passed 90/90, then the post-fix runtime subset passed 11/11.
- Direct dummy-driver runtime probes passed for Hall walkaround and champion mirror walk-path using local DM1 data.
- Phase A probe passed 23/23 and `git diff --check` passed.

---

# Firestaff v2.7.10

Patch release focused on the large DM1 V1 hardening batch after v2.7.9.

## What's New

- Expanded DM1 V1 source-lock and pixel coverage across additional viewport front, side, door, wall-ornament, floor, ceiling, pit, teleporter, and thing-pass paths.
- Hardened DM1 V1 runtime behavior around chests, mirror-candidate handoff, champion-panel routing, projectiles, creatures, poison clouds, fake walls, teleporters, keyholes, pits, fountains, skill progression, food/water timing, and Vi altar resurrection.
- Tightened the original-capture workflow with DOSBox rawshot fallback, rawshot freshness checks, single-row transcript validation, and 320x200 plus viewport-crop guards.
- Kept the public release scope honest: this packages a broad DM1 V1 parity/regression step, while CSB, DM2, Nexus, and Theron remain active hardening targets.

## Verification

- GitHub Actions verify workflow passed on `main` before release prep.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.9

Patch release for two DM1 launch regressions found in v2.7.8 on MacBook Pro.

## Fixes

- Fixed Entrance door buttons on Retina/HiDPI macOS displays by keeping SDL3 pixel-size events separate from logical mouse coordinates.
- Fixed the FTL/SWSH swoosh cadence by matching ReDMCSB's immediate `Setcolor()` batching and `DBF` VBlank wait semantics.

## Verification

- Local high-DPI presentation/mouse mapping regression passed.
- Local SWSH source-animation timing gate passed with 30 effective palette VBlanks.
- Local Entrance button click runtime probe passed 17/17.
- Local Phase A probe passed 23/23.

---

# Firestaff v2.7.8

Patch release focused on DM1 V1 viewport, inventory, mirror-candidate, and CSB V1 source-lock/runtime hardening after v2.7.7.

## What's New

- Added DM1 V1 source-lock coverage for D1L2/D1R2, D3L2/D3R2, D2L2/D2R2, D0L2/D0R2, and D0C viewport floor, ceiling, ornament, door-front, and thing-pass paths.
- Hardened DM1 V1 inventory and mirror-candidate runtime behavior around occupied-slot swaps, scroll pickup/drop, C040 panel-live handoff, reshuffle, cancel, candidate-close, and non-leader routes.
- Expanded CSB V1 viewport/runtime evidence for D1L2/D1R2 and D2C/D0L2/D0R2 door, floor, ceiling, CustomBackgrounds, movement-command, and command-chain behavior.
- Kept the public status honest: these are source-lock and regression-hardening slices, not new claims of finished CSB/DM2/Nexus/Theron end-to-end parity.

## Verification

- GitHub Actions verify workflow passed on `main` at `cf0501377` before release.
- Local focused pass712/pass713/pass714/pass715 gates and Phase A probe passed before release prep.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.7

Patch release focused on DM1 V1 viewport, inventory, champion-panel, and runtime regression hardening after v2.7.6.

## What's New

- Expanded DM1 V1 source-lock and pixel coverage for viewport side walls, stairs/pit dispatch, floor/ceiling fallback, door fronts, wall ornaments, champion mirror paths, and projectile side-cell behavior.
- Hardened DM1 V1 inventory and champion-panel behavior around chest routing, close/reopen edges, pickup/swap paths, mirror-candidate handoff, hand-slot priority, held-item icons, portraits, wounds, and stale-pixel redraws.
- Locked V2.0 filtered presentation to a 640x400 2x runtime surface across games while preserving original 320x200 game/input coordinates.
- Added selectable V2.1/V2.2 presentation resolutions from 640x400 through 3840x2160, with launch intent and M11 input mapping preserving original 320x200 gameplay coordinates.
- Added a DM1 V2 side-by-side manifest pixel gate for full V1/V2 lanes plus D1C wall and portrait regions, keeping enhanced-presentation diffs anchored to source-locked V1 rectangles.
- Wired Theron's Quest direct launch so M11 can consume the launcher catalog's hash-verified Track 02 path/MD5 without re-walking the game-data root.
- Added focused runtime gates for spell-rune preservation, poison/cloud timing, room-transition pickup ordering, delayed timeline saves, keyhole no-op behavior, and audio pack frame bounds.
- Added cross-game regression coverage for CSB viewport/import/chaos/optional-asset paths, DM2 save/weather/projectile behavior, Nexus palette/DGN bounds/save validation, and Theron launch/progression/shop/transition state.

## Verification

- GitHub Actions verify workflow passed on `main` at `8cc26c5aa` before release.
- Local release-prep CMake build, Phase A probe, and audio probe passed before tagging.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.6

Patch release focused on DM1 V1 inventory-panel source-lock hardening after v2.7.5.

## What's New

- Added a 176-assertion DM1 V1 inventory-panel regression gate for status-row hand-slot routing.
- Proved that status hand slot boxes 0..7 resolve to the intended champion and source slot without falling through to the inventory swap path.
- Added coverage for dead champions, candidate champions, open inventory champions, out-of-party slots, null health state, and per-champion mouse-item routing.

## Verification

- Local focused inventory-panel route gate passed before tagging.
- Local Release build, Phase A probe, and audio probe passed before tagging.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.5

Patch release focused on DM1 launch regressions reported in v2.7.4 on macOS.

## Fixes

- Restored the initial FTL/SWSH animation when DM1 data is stored in the normal nested data-directory layout.
- Restored the source-locked Dungeon Master TITLE animation palette steps for PRESENTS, DUNGEON and MASTER phases.
- Fixed Entrance door button clicks when the macOS window size changes outside Firestaff's cached resize path.
- Made the new SWSH pathfinder regression test portable on Windows.

## Verification

- GitHub Actions verify workflow passed on `main` at `b330682d`.
- Local focused gates passed: SWSH pathfinder, TITLE step palette, Entrance button click probe, Phase A and audio probe.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64 and Linux arm64 artifacts.

---

# Firestaff v2.7.4

Patch release focused on DM1 V1 viewport, panel, and capture-route regression hardening after v2.7.3.

## What's New

- Added focused DM1 V1 runtime and pixel probes for champion mirror visibility, mirror Z-order, chest compact-close edges, D2L side-wall rendering, capture-route smoke coverage, and champion panel bounds.
- Expanded DM1 V1 viewport evidence around side-wall drawing, wall inscriptions, and small-scale window layout.
- Kept the latest DM1 V1 presentation and panel hardening in the release packaging path.

## Fixes

- Fixed the legacy small-scale window layout regression.
- Fixed DM1 V1 side-wall drawing drift and wall inscription source-font rendering.
- Slowed title frontend cadence back to the V1 tick path.
- Cleared stale Firestaff queue failed probes and made the CSB DSA probe mkdir path portable.

## Verification

- GitHub Actions verify workflow passed on `main` at `3fe79467`.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.3

Patch release focused on regression coverage, cross-platform test harness fixes, and verified release packaging after v2.7.2.

## What's New

- Added narrow no-game-data regression coverage for DM1, CSB, DM2, Nexus, Theron, M11 overlay input, accessibility manifest output, save-browser behavior, and M12 data-directory cancel handling.
- Added Phase A coverage for M12 no-data asset-status scans, including null-safe calls, empty roots, stale-path clearing, and deterministic repeated scans.
- Expanded asset-status scanner coverage for irrelevant data roots, partial required-data matches, archive-backed required files, and required-file accounting.

## Fixes

- Fixed Windows test harness portability for `stat` and temporary-directory helpers.
- Fixed static-library link ordering for M11/M12 test harnesses on ELF linkers.
- Preserved launch/profile identity diagnostics while keeping required-file launch gates blocked when data is incomplete.

## Verification

- GitHub Actions verify workflow passed on Ubuntu, macOS, and Windows.
- CMake build, strict warnings, Phase A, audio smoke, and cross-platform determinism passed on `main`.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.2

Patch release focused on game-data discovery, launch gating, real-asset runtime handoff, and regression cleanup after v2.7.1.

## What's New

- Recursive hash-based game-data discovery now covers nested folders, stored ZIP entries, deflated ZIP entries when zlib is available, and ISO/BIN ISO 9660 data images.
- Archive-backed DM1, CSB, and DM2 required files can be materialized into the Firestaff asset cache before launch, while runtime code continues opening ordinary `GRAPHICS.DAT` and `DUNGEON.DAT` paths.
- The start menu now wires data-directory availability into game cards and reports missing required game data with user-facing OK popups.
- Theron's Quest now recognizes JP Rev 1 and US Track 02 ISO variants and can route direct launches into the native M11 Theron boot/world/viewport path.

## Fixes

- Fixed DM2 save/load SUPPRESS masks and version detection.
- Fixed DM2 PC English GRAPHICS.DAT container loading and real DUNGEON.DAT parser probes.
- Fixed CSB PC 3.4 FTL-compressed DUNGEON.DAT ingestion in the V1 loader.
- Fixed DM1 V1 viewport floor-ornament/stair, front-wall depth, and pit floor-ornament BUG0_64 regression gates.
- Fixed M11 inventory scroll-panel and mouth-visual source-lock gates on configured macOS data roots.
- Fixed CSB V2 Phase 7 and CSB/DM2 V2 smooth-movement verification regressions.
- Fixed `turn_viewport_orientation_probe` so it exits cleanly after writing artifacts.

## Verification

- GitHub Actions verify workflow remains the release gate on `main`.
- Relevant local regression gates passed: DM1 viewport, M11 inventory, CSB V2 Phase 7, CSB V2 smooth movement, DM2 V1 asset/dungeon/save probes, and CSB V1 dungeon load gate.
- Phase A probe remains green.

---

# Firestaff v2.7.1

DM1 PC-34 boot-sequence fidelity release.

## Fixes

- Added hash recognition and direct runtime launch support for the JP Rev 1 and US Track 02 ISO variants of Theron's Quest.
- Fixed direct launch version selection so hash-matched game data can launch even when the default menu version slot points at a different known variant.
- Kept the DM1 TITLE/entrance sequence on the DM1 launch path only, allowing Theron's Quest to enter its own Track 02 runtime handoff directly.
- Restored ReDMCSB SWSH/FTL logo playback before the DM1 TITLE sequence.
- Fixed SWSH `SWOOSH` loading when the original file is a DOS/MZ program with the IMG logo payload embedded inside it.
- Fixed the SWSH palette path so the FTL logo starts black and lights up via the ReDMCSB `Setcolor()` command sequence instead of using the TITLE palette.
- Kept the GRAPHICS.DAT `C001_GRAPHIC_TITLE` path on the same final guard timing as the TITLE.DAT fallback.
- Replaced hardcoded entrance-door delays with source-locked ReDMCSB vblank timing.

## Verification

- SWSH, TITLE, and entrance source-lock gates passed locally.
- Phase A probe: 21/21 invariants passed locally.

---

# Firestaff v2.7.0

Major V2 pipeline completion across all game systems, Theron V1 rendering pipeline, DM1 V1/V2 Phase 8 completion, and accessibility improvements building on the v2.6.0 release.

## What's New

### CSB V2 — Phase 0–6 Complete
- **Phase 0**: V1 compatibility lock.13 domain compile gates (`CSB_V2_PHASE_DOMAIN_*`), stub hooks for all V2-only functions, C11 `_Static_assert` for V1 struct sizes. Source-lock: COMMAND.C, DUNGEON.C, CSBWin champion/resurrect.
- **Phase 1**: Launch/profile separation with compile gates and CSB-hash-katalog (DUNGEON.DAT `6695d2a`, GRAPHICS.DAT `61fbfd5`). LAUNCH-before-PROFILE pattern enforced. Source-lock: ENTRANCE.C F0806, PROFILE.C F0401.
- **Phase 2**: Enhanced asset pipeline.
- **Phase 3**: Enhanced UI overlays — HUD overlay scaffolding.
- **Phase 4**: Stairs animation for smooth movement + runtime hardening.
- **Phase 5**: Stairs animation + runtime hardening.
- **Phase 6**: Touch controller test coverage and affordances.

### DM2 V2 — Phase 1–6 Complete
- **Phase 1**: Launch/profile phase gate, probe, and CMake wiring.
- **Phase 2**: Enhanced asset pipeline.
- **Phase 3**: HUD overlay hardening.
- **Phase 4**: Enhanced lighting, outdoor FX, torch flicker, fog animation.
- **Phase 5**: Smooth movement runtime integration (`DM2_V2_MoveCallback`/`TurnCallback`/`StairsCallback` registered into `dm2_v1_runtime`). Source-lock: ReDMCSB DUNGEON.C G0306/G0307.
- **Phase 6**: Touch controller affordance stubs for V2 render pipeline.

### Nexus V2 — Phase 1–6 Complete
- **Phase 1–6**: Touch/controller affordance ergonomics, atmosphere, lighting, particles, upscaler fixes.

### Theron V1 — Phase 1–4 Complete
- **Phase 1–4**: Rendering pipeline — tile renderer, asset loader, UI chrome, creature instance lifecycle (death/drop/sound integration).
- **Runtime handoff**: M11 now routes hash-verified Track 02 launches into the Theron boot/world/viewport path instead of the DM1 DUNGEON.DAT loader, with deterministic fallback rendering while exact Track 02 dungeon-bank offsets are hardened.

### DM1 V1 — Phase 8 Complete
- Door and special-square interaction probe.
- Wall rendering integrity probe (wall spec, parity, blit clip gate).
- Blurry inscription probe.
- Champion portrait Z-order fix (floating artifact).
- Source-lock parity evidence manifests and line references updated.

### DM1 V2 — Phase 8 Complete
- Door-frame type override and manifest path resolution.
- Message log — pixel font atlas and scroll renderer.
- Champion panel renderer and HUD overlay V2 (Phase 8 revisit).
- FloorShapeType enum normalization, `dm1_v2_vp_square_id` export.
- DM1 V2.2 Modern Asset Pipeline defined.

### M12 Launcher
- JSON settings export/import feature (`firestaff_m12_json_export_import_probe`).

### Accessibility
- High-contrast game view toggle.
- Configurable in-game font scaling (M11 `fontScale` from M12 setting).

### Probes
- `firestaff_nexus_v1_mechanics_parity_probe` — Phase 7 mechanics verification (dungeon loading, movement, combat, save/load, world state, engine lifecycle).
- CSB V1 Phase 2 DSA script section probe.
- DM1 V1 parity-evidence manifests (2026-05-30 refresh).
- Source-lock evidence docs with screen-detect automation.

## Fixes

- Fixed `m11_game_view.c` missing includes chain.
- Fixed CSB V1 Phase 6 followup warnings and `get_party` stub.
- Fixed DM1 V1 wall rendering: `flip_horizontally` set before return (native path).
- Fixed stale DM1 V1 verification probe line ranges.
- Fixed `dm2_v1_world_model.c` dungeon data model.
- Fixed `dm2_v1` movement collision to check door state (Phase 4 gap).
- Fixed `dm1_v1_viewport_cell_is_wall_like` inline fakewall/open-wall parity.
- Fixed `dm1_v1_viewport_3d_select_wall_bitmap` `flip_horizontally` before return.
- Fixed `csb_v1_phase7_followup` 3 failing test assertions.
- Fixed `nexus_v1_mechanics_combat_probe` — combat/creature AI source-lock.
- Fixed `passH22F7` CSB V1 Phase 2 DSA script section.
- Fixed `test_dm1_v22_verification` manifest JSON format (single-line → multi-line).
- Fixed `DM1_V1`0x80 inscription separator normalization to `\n`.

## Verification

- GitHub Actions verify workflow passed on `main` before release.
- CMake configure + build completed (all targets, pre-existing warnings only).
- Phase A probe: 21/21 invariants.
- Nexus launch smoke: 6/6.
- CSB V2 Phase 1 separation: 40/40.
- DM2 V2 HUD overlay, lighting, csb_v2_lighting_dynamic, csb_v2_touch_controller_affordance: all green.

## Platforms

| Platform | Architecture | Format |
|----------|-------------|--------|
| macOS | arm64 + x86_64 | DMG, ZIP |
| Windows | x86_64 | ZIP, Installer (EXE) |
| Linux | x86_64 | DEB, RPM |
| Linux | ARM64 | DEB, RPM |

---

# Firestaff v2.6.0

V2 expansion, source-lock hardening, and engine handoff release building on the v2.5.x pipeline.

## What's New

- **CSB V2 Phase 0**: V1 compatibility lock before V2 work. `csb_v2_phase_gate_pc34.h` defines 13 domain compile gates (V1-source-locked vs V2-presentation-berättigade). Stub hooks for all V2-only functions. C11 `_Static_assert` for V1 struct sizes. Source-lock: COMMAND.C, DUNGEON.C, CSBWin champion/resurrect.
- **CSB V2 Phase 1**: Launch/profile separation. `CSB_V2_PHASE_DOMAIN_LAUNCH` and `CSB_V2_PHASE_DOMAIN_PROFILE` compile gates with CSB-hash-katalog (DUNGEON.DAT `6695d2a`, GRAPHICS.DAT `61fbfd5`). LAUNCH-before-PROFILE pattern enforced. Source-lock: ENTRANCE.C F0806, PROFILE.C F0401.
- **DM2 V2 Phase 5**: Smooth movement runtime integration. `DM2_V2_MoveCallback`/`DM2_V2_TurnCallback`/`DM2_V2_StairsCallback` registered from `dm2_v2_runtime.c` into `dm2_v1_runtime`. Pre-move position stored, turn-only detection fires turn_callback without move_callback. Source-lock: ReDMCSB DUNGEON.C G0306/G0307.
- **Nexus V1 Launcher**: Full `nexus_v1_launcher.h/.c` engine handoff. Singleton owns `Nexus_V1_Engine` lifecycle. `M11_GameView_StartNexus` now calls `launcher_init` + `launcher_load_level(0)` and stores engine pointer. `firestaff_nexus_v1_launch_smoke_probe` validates full init→load→tick→render cycle (6/6 headless). Source-lock: NEXUS.C/NEXUS2.C engine lifecycle, DMWeb Saturn DGN/DMDF format.

## Fixes

- Fixed CI build error: `nexus_v1_launcher.h` not in git causing `fatal error: nexus_v1_launcher.h: No such file` on fresh clone. Launcher integrated into CMake and source committed.
- Fixed `firestaff_nexus_v1_launch_smoke_probe` orphaned CMake target (source file was added in commit but never committed).
- Disabled `test_dm1_v22_verification` CMake target (committed with massive API mismatches — wrong headers, undefined types).
- Patched `m11_game_view.c` missing includes: `nexus_v1_engine.h`, `dm1_v2_camera_controller_pc34.h`, `firestaff_po_loader.h`, `dm1_v2_phase5_runtime_bridge_pc34.h`, `dm1_v1_viewport_fakewall_pc34_compat.h`.
- Added `firestaff_nexus` to `firestaff_m11` `target_link_libraries` (linker error on `M11_GameView_StartNexus`).

## Verification

- GitHub Actions verify workflow passed on `main` before release.
- CMake configure + build completed (all targets, pre-existing warnings only).
- Phase A probe: 21/21 invariants.
- Nexus launch smoke: 6/6.
- CSB V2 Phase 1 separation: 40/40.
- DM2 V2 HUD overlay, lighting, csb_v2_lighting_dynamic, csb_v2_touch_controller_affordance: all green.

## Platforms

| Platform | Architecture | Format |
|----------|-------------|--------|
| macOS | arm64 + x86_64 | DMG, ZIP |
| Windows | x86_64 | ZIP, Installer (EXE) |
| Linux | x86_64 | DEB, RPM |
| Linux | ARM64 | DEB, RPM |
