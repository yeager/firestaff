# Firestaff v2.7.2

Patch release focused on game-data discovery, launch gating, real-asset runtime handoff, and queue regression cleanup after v2.7.1.

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
- Fixed M11 inventory scroll-panel and mouth-visual source-lock gates on local macOS data roots.
- Fixed CSB V2 Phase 7 and CSB/DM2 V2 smooth-movement verification regressions.
- Fixed `turn_viewport_orientation_probe` so it exits cleanly after writing artifacts.

## Verification

- GitHub Actions verify workflow passed on `main` before release.
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
- Fixed DM1 V1 pass515/519/560/565/570 verify probes stale line ranges.
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
