# Firestaff TODO - Open Work

This file tracks remaining work only. Completed work belongs in `DONE.md`.

## Maintenance Cadence

- 🔧 Update `TODO.md` and `DONE.md` at least twice per day while Firestaff work is active.
- 🔧 Keep `TODO.md` limited to things that still need fixing, building, verification, or release follow-up.
- 🔧 Keep `DONE.md` limited to finished, verified work.

## Legend

- ❌ Not started
- 🔧 In progress / partial
- 🐛 Known bug

## Current Priority Queue

- 🔧 Queue/refill priority order: DM1 V1 → Theron V1 → CSB V1 → DM1 V2 → CSB V2 → DM2 V1 → Theron V2 → Nexus V1 → Nexus V2. Worker refill focus should stay on `priority` unless intentionally pausing higher-priority lanes.
- 🔧 DM1 V1 original same-viewport capture: runbook §5b Output Manifest handoff (pass633) and runbook §5c Runtime Transcript handoff (this pass) both ship deterministic code paths; the next live DOSBox attempt must produce a ``transcript.json`` that satisfies the pass608 / pass625 binding contract and demote ``BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE`` to ``PROMOTED_STATUS = PASS608_DM1_V1_COMMAND_STATE_REDRAW_TRANSCRIPT_BOUND``.
- 🔧 Theron V1 Track 02 dungeon-bank promotion: replace the deterministic launch-room fallback with parsed real Track 02 dungeon/map banks for JP Rev 1 and US ISO variants.
- 🔧 Theron V1 runtime playability slice: prove movement, door/pit/teleporter interactions, altar/progression routing, combat, and save/load through at least one real Track 02 dungeon path.
- 🔧 Theron V1 real screenshot proof: capture tracked README-eligible in-game screenshots from actual Firestaff runtime, using only real Track 02 data and no generated/mock imagery.
- 🔧 Theron V1 scanner/runloop ergonomics: avoid expensive full default-root rescans on direct launch when a hash-verified Theron runtime path is already known.
  - 🔧 Add the boot-level `theron_v1_boot_load_verified_path()` entry point (regression: `test_theron_v1_direct_launch`); M12 catalog-level direct path already landed in `m12_try_match_direct_theron_request()`. Remaining: wire the boot entry point into `M11_GameView_StartTheron` so the two slices compose, and add a probe that exercises the full launch path with the rescan counter asserted.
- 🔧 Post-Theron handoff: when Theron V1 has real Track 02 dungeon-bank/runtime proof, shift active V1 work to CSB V1 runtime handoff, viewport integration, and end-to-end playability verification.
- 🔧 Data-directory picker polish: start menu can switch between configured/default roots; add a proper browse/manual path flow for arbitrary user-selected game-data folders.
- 🔧 Archive scanner performance: full default scan over the local real-asset collection with multiple Nexus ISO/ZIP images still needs indexing/prefilter work; isolated `--scan-data` smoke and archive regression tests pass.
- 🔧 Extend archive runtime handoff beyond DM1/CSB/DM2 if Theron Track 02 or future Nexus flows need cache materialization instead of their current direct image paths.
- 🔧 Extend the game-data scan report with machine-readable output if release/installer tooling needs it.
- 🔧 Add focused regression coverage for DM1/CSB/DM2 missing-DUNGEON launch blocking in addition to the existing CSB launcher gate.

## Dungeon Master (DM1)

### DM1 V1

- 🔧 Original DOS capture parity: five specific paired evidence sets are blocked. Details and honest status labels at `docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md`. Minimum runbook at `docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`.
  - Viewport: original pass94 captures exist (2026-04-28) but are impaired — frames 03–06 have duplicate SHA256, pass80 classifier reclassifies them as `entrance_menu`/`wall_closeup` instead of `dungeon_gameplay`. DOSBox input route failed to enter dungeon. New capture session with working dungeon-entry sequence required.
    - The `docs/parity/tools/dosbox_state_detector.py` and `docs/parity/tools/dosbox_capture_session.py` regression gates now run in CI via `dm1_v1_original_capture_state_detector_self_test` and `dm1_v1_original_capture_session_dry_run`.  Post-fix the pass94 frames reclassify as 03-04 `entrance_menu` and 05-06 `dungeon_gameplay` (the runbook's "wall_closeup" label for 05-06 came from the same broken 0.70/0.10 envelope this script used to use, not from real densities — the corrected ground truth is captured in the detector's `PASS94_GROUND_TRUTH` table).  The next live attempt must still be run to actually pair the original captures with Firestaff output, but the classifier bug is no longer a blocker for diagnosing the route state.
    - The `docs/parity/tools/dosbox_capture_preflight.py` regression gate now runs in CI via `dm1_v1_original_capture_preflight_self_test`.  The preflight pins the runbook §2 "machine=svga_s3 is non-negotiable" requirement (and the related `memsize=16`, `cpu core=dynamic`, `cycles=max`, `frameskip=0`, `windowresolution=1024x768`, `output=opengl` settings) and refuses to write a conf that contains the historical pass94 failure-mode values (`svga_paradise`, `memsize=4`, `core=normal`, `cycles=3000`); it also performs the §1 SHA256 checks and records a JSON receipt.  The preflight is the upstream gate for the runbook's "wrong machine type" failure mode — a future live attempt that runs the preflight before launching DOSBox cannot reproduce the pass94 conf shape.
    - The `docs/parity/tools/dosbox_capture_manifest_writer.py` regression gate now runs in CI via `dm1_v1_original_capture_manifest_writer_self_test`, and the runbook-consistency probe at `tools/test_dm1_v1_capture_runbook_consistency.py` now fails if the runbook stops pointing operators at the on-disk manifest writer.  The writer turns the preflight receipt + a per-capture classifier output TSV into the runbook's Output Manifest Template with real SHA256s, a live `git rev-parse HEAD`, the receipt's pinned `launch_command`/`render_settings`, and the classifier's verdict for every capture; it refuses to emit a manifest when the preflight pin checks are not all PASS, when a recorded SHA does not match the file on disk, or when a classification is outside the runbook's documented state list.  The next live attempt that runs the manifest writer cannot ship a hand-filled manifest with a stale placeholder hash the way a previous draft of the runbook did.
    - The `docs/parity/tools/dosbox_capture_transcript_writer.py` regression gate now runs in CI via `dm1_v1_original_capture_transcript_writer_self_test`, and the runbook-consistency probe at `tools/test_dm1_v1_capture_runbook_consistency.py` now fails if the runbook stops pointing operators at the on-disk transcript writer.  The writer turns the preflight receipt + a per-capture events TSV + the pass623 canonical input-capture fixture + the Firestaff capture-manifest viewport-hash set into the `transcript.json` the pass608 same-viewport capture blocker and the pass625 preflight demand.  The writer refuses to emit a transcript when the preflight pin checks are not all PASS, when an input token has no `sourceCommandId` mapping in the pass623 fixture, when a row's command id is a TURN and `dispatch.handler` is the MOVE handler (or vice versa), when `partyAfter` does not match the F0128 redraw tuple, when `firestaffFrame.viewportSha256` is not in the canonical Firestaff `viewport_224x136` hash set, when a recorded `originalFrame.rawSha256`/`cropSha256` does not match the bytes on disk, or when the events TSV header is re-ordered (the 41-column order is part of the public contract).  The next live attempt that runs the transcript writer cannot ship a hand-built transcript with a stale ReDMCSB function name the way a previous draft of the same-viewport route did.
    - The live session runner now has an executable `--live` path and the current canonical data passes preflight, but the latest host attempt produced all-black screen captures from both normal and Bridge-routed capture.  Follow-up diagnostics proved the live launch should use the extracted `DungeonMasterPC34` runtime layout (`DM.EXE` + `DATA/`) while preflight keeps proving the canonical hash root; the runner now supports that split through `--runtime-dir`.  Remaining capture work is a GUI/input lane that can focus DOSBox or trigger its internal `rawshot`/`rendshot` mapper actions, then rerun the same state-machine route until it reaches `dungeon_gameplay` and emits paired manifest rows.
  - Wall: no paired original wall screenshot exists. Wall composition is source-locked only.
  - Collision: no paired original collision transcript exists. Collision logic is source-locked only.
  - Creature-chain: no paired original creature screenshot exists. Creature render is source-locked only.
  - Champion-panel: Firestaff V1 captures exist (party_hud_four_champions_vga.ppm, party_hud_statusbox_gfx_vga.ppm), `dm1_v1_champion_panel_pixels_runtime` covers a deterministic Firestaff-side real-asset HUD/status-box pixel slice, `dm1_v1_champion_panel_status_states_runtime` extends it with secondary status-box states, the HUD source-lock test now covers the inventory mouth/eye warning-border, action-hand slot-priority, champion-icon redraw, shield-border stack routes, and the F0354 inventory portrait box/blit route, and `dm1_v1_champion_panel_recompute_runtime` now locks the contract-only status-recompute slice's food/water/poison panel visible-delta spec against the M11 draw stack with real `GRAPHICS.DAT` assets; no paired original DM1 PC 3.4 champion panel screenshot exists.
  Canonical game data verified: DUNGEON.DAT SHA256 `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`, GRAPHICS.DAT SHA256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`. Firestaff-side gates, source locks, and runtime routing are complete.
- 🔧 Viewport rendering hardening: broad wall/floor/ceiling/door/ornament/pit/stair/source routes are source-locked, and D3C, D3L/D3R, D3L2/D3R2, D3R2 teleporter-field pixels, D2C/D2C opaque center-wall/D2L/D2R, D2L2/D2R2, D1C/D1R, D1C center-field, D1L no-write clipping, D0L, D0L/D0R parity, D0L2/D0R2 F0115 thing pass, D2L2/D2R2 F0115 thing pass, F0098 floor/ceiling fallback pixels, F0099 row-local flip parity, F0108 floor-ornament metadata/open-pit ordering, F0107 wall-ornament ordinal/coordinateSet/alcove path, the D0L/D0R row-composition source lock (DRAWVIEW.C F0097, VIEWPORT.C F0564/F0565/F0566, DUNVIEW.C F0125/F0126/F0128, DEFS.H C716/C717), and the D3C stairs/pit dispatch source-lock (DUNVIEW.C F0118:6642-6763 D3C dispatch, F0115 thing-pass follow-up, F0128 post-D3C wall-followup writes) now have concrete gates; remaining work is broader capture-backed bug closure and any remaining real-asset viewport pixel-polish evidence.
- 🔧 Inventory/chest polish beyond source-locked routes: core slot/body/chest/backpack/source routes, compact-close edge cases, occupied-slot swap, hand swap with an open chest, pouch/quiver/backpack hand-to-belt mask swaps, inventory champion-switch hand-carry preservation, empty-slot no-op, late empty-slot placement, incompatible item rejection including zero AllowedSlots masks, mixed-type chest link-order pickup, overfull C544 replacement close rewrite, C537/C538/C539/C540/C541/C542/C543/C544 real-asset icon pixels, the per-container base-weight + champion encumbrance adjustment on close, the chest pickup stack failover (CHEST.C F0333:30-32 early-return vs. F0333:53-67 dispatch vs. F0334:117-132 close-rewrite, OBJECT.C F0032/F0033:121-212, AMMO.C F0294:54-79, BLITMASK.C F0133:30-33), the backpack↔chest round-trip routing (CHEST.C F0333:53-76, F0334:113-132, CHAMPION.C F0297/F0298:243-298, F0300/F0301:511-515,606-614), and the mid-close hand-swap with open chest runtime regression (CHEST.C F0333:43-67 + F0334:113-132, CHAMPION.C F0297/F0298/F0300/F0301/F0302:243-298,511-515,606-614,688-710, OBJECT.C F0033:147-212, BLITMASK.C F0133:30-33) are implemented; remaining work is broader chest runtime detail coverage and pixel-polish evidence.
- 🔧 Inventory panel hardening: core item/inventory routes are source-locked; status hand slot box 0..7 routing (CHAMPION.C F0302:677-684) and the status hand + open chest state interaction (F0333:43-67 + F0334:112-133 + F0291:621-630) now have contract-only synthetic gates; remaining work is broader chest runtime detail coverage and pixel-polish evidence.
- 🐛 P1 visual bugs needing capture/repro: missing or incorrect viewport walls and champion Z-order/floating. The known Hall champion-mirror visibility route is covered by `dm1_v1_champion_mirror_visibility_runtime`, north/south/east Hall D1C plus west-facing no-floating slices are covered by `dm1_v1_champion_mirror_zorder_runtime`, the sequential in-place 4-direction Z-order re-blt invariant (no stale ordinal pixels left in the portrait rect after a turn) is covered by `dm1_v1_champion_mirror_zorder_reblt_runtime`, the cross-cell forward-walk re-blt invariant (no stale ordinal pixels left in the D1C front-wall box when the front cell changes from (1,3) ordinal 1 to (2,3) no-portrait to (3,3) ordinal 19 and back) is covered by `dm1_v1_champion_mirror_walkpath_runtime`, the C040 resurrect/reincarnate panel pixel/overlap/select-miss/reincarnate-click/rest-guard/inventory-toggle/spell-rune/portrait-reselect/status-box-click/save-input/C159 champion-icon/double-open-close guard routes are covered by `dm1_v1_champion_mirror_candidate_panel_runtime` and focused regression gates, the mirror-candidate pending hand queue routing (CHEST.C F0333:30-32 + F0334:113-132 + COMMAND.C F0359:1985-1990 + REVIVE.C F0280:124-132) is covered by `dm1_v1_mirror_candidate_pending_hand_queue_pc34_compat`, and the C040 candidate full-chain (CHAMDRAW.C F0293:1117-1143 champion-state redraw + CHAMPION.C F0284:93-131 party direction rotate + CHAMPION.C F0297:243-268 leader-hand put + COMMAND.C F0359:1985-1990 M568/C040 dispatch + REVIVE.C F0282:744-806 candidate clear) is covered by `dm1_v1_mirror_candidate_full_chain_pc34_compat`; keep broader mirror/Z-order/candidate capture evidence open until more routes are pixel-proved.

### DM1 V2.0 / V2.1 / V2.2

- 🔧 Phase 3 - Modern UI overlay hardening: HUD/action route gates exist; remaining work is optional inventory, champion, rune, and action-panel polish without bypassing V1 command routes or inventory transactions.
- 🔧 Phase 4 - Lighting and visual effects hardening: palette/projectile metadata gates and field/projectile VFX binding gates exist; remaining work is full enhanced lighting, shadows, broader field effects, and deterministic fallback coverage.
- 🔧 Phase 5 - Smooth movement presentation hardening: runtime bridge/gates and optional Custom/V2 smooth turn-pan camera backend exist; remaining work is broader interpolation coverage and launcher UI polish while preserving V1 cooldowns, collision, sensors, creature timing, and redraw cadence.
- 🔧 Phase 6 - Touch/controller ergonomics hardening: route gates exist; remaining work is broader V2-only gesture/controller affordances with V1 touch/click parity preserved.
- 🔧 Phase 7 - V2 verification suite hardening: presentation-disabled state-hash gate exists; remaining work is full side-by-side V1/V2 deterministic input scripts plus screenshot/pixel gates for enhanced presentation.

## Chaos Strikes Back (CSB)

### CSB V1

- 🔧 Phase 2 - Dungeon data model: synthetic CSB dungeon loader/model probe exists and the loader now ingests the hash-verified FTL-compressed CSB PC 3.4 DUNGEON.DAT; remaining work is DSA/runtime structure parity beyond the current real-asset load gate.
- 🔧 Phase 3 - Rendering parity hardening: D3/D2 wall tables, bitmap selection, grid routing, CSB-only D3L2/D3R2 and D2L2/D2R2 draw-order/frame gates, D3L2/D3R2 and D2L2/D2R2 wall gates, F0107 back-wall ornament routing/blit/side-effect gates, F0108/F0111 route gates, F0108 floor-blit metadata and footprint-recursion source-lock gates, F0111 door panel zone/clip/partially-open-zone/runtime bitmap/destroyed-mask blit gates, the D2C F0111 door source-lock gate (DUNVIEW.C F0111:4218-4337 + F0121:7244-7389 + F0128:8508-8533 + DUNGEON.C F0163/F0164:1769-1840), F0115 projectile metadata/runtime synthetic blit, creature/item/explosion metadata gates, teleporter-field route gates, `CustomBackgrounds` room-slot/bitmap-application source-lock gates, the `CustomBackgrounds` viewport backdrop mask/enumeration source-lock gate, the `CustomBackgrounds` first backdrop source-lock gate (room 0 / rel fwd=3 side=-2, non-overlap with second backdrop), and initial viewport gates exist; remaining work is broader capture-backed bug closure and real-asset viewport integration evidence.
- 🐛 Runtime handoff: the M12 launch/profile intent is valid for hash-matched CSB assets and the boot→runtime handoff now actually loads the verified DUNGEON.DAT into `runtime.dungeon_handle` (heap-allocated, owned by the profile); remaining work is title/intro/import path, CSB-specific viewport integration, and end-to-end playability verification.

### CSB V2.0 / V2.1 / V2.2

- ❌ Phase 2 - Enhanced asset pipeline.
- 🔧 Phase 3 - Enhanced UI overlays: scaffolded (HUD compass/depth/gold/champion bars/action strip/chaos indicator, csb_v2_hud_overlay_pc34.h/.c, build+probe pass).
- ❌ Phase 4 - Enhanced lighting and magic effects.
- 🔧 Phase 5 - Smooth movement and viewport interpolation: `firestaff_csb_v2_smooth_movement_probe` passes 58/58 and `firestaff_dm2_v2_smooth_movement_probe` passes 54/54; remaining work is broader CSB V1 runtime binding and side-by-side presentation evidence.
- ❌ Phase 6 - Touch/controller ergonomics.
- 🔧 Phase 7 - V2 verification suite: scaffolded and current deterministic gate is green; remaining work is full side-by-side V1/V2 screenshot/pixel gates.

## Dungeon Master II: Skullkeep (DM2)

### DM2 V1

- 🔧 Phase 2 - Dungeon/world data model: DM2 map, object, tile, and world-state ingestion probes pass against the local hash-verified PC English DUNGEON.DAT; remaining work is broader runtime parity and any still-stale CTest-era dungeon fixtures.
- 🔧 Phase 3 - Rendering pipeline: viewport, UI chrome, items, outdoor/indoor, and palette behavior remain active; the real PC English GRAPHICS.DAT asset-loader probe is green again.
- ❌ Phase 4 - Mechanics parity: movement, interactions, shops/NPCs, doors, pressure plates, triggers, combat, magic, and timeline.
- ❌ Phase 5 - Creature/combat parity: complete DM2 creature AI, projectile, damage, death/drop, and sound behavior.
- 🔧 Phase 7 - Save/import compatibility verification: focused `test_dm2_v1_save_load` now covers invalid DM2 slot headers and corrupt-primary backup recovery; remaining work is broader real-runtime compatibility coverage.

## Dungeon Master Nexus

### Nexus V1

- 🔧 Runtime handoff/playability proof: V1 phases 0-7 are implemented/source-locked; remaining work is launcher/game-loop handoff and real Saturn asset-path proof.
- 🔧 Mechanics parity hardening: movement, click routes, item usage, doors, pits, teleporters, triggers, combat, AI, and sound are implemented; remaining work is broader runtime/probe coverage beyond compile/save-load gates.
- 🔧 Phase 5 - Mechanics parity hardening: runtime bridge and probe coverage for movement, click routes, item usage, doors, pits, teleporters, triggers, combat, AI, and sound.

### Nexus V2.0 / V2.1 / V2.2

- ❌ Phase 0 - V1 compatibility lock before V2 work.
- ❌ Phase 1 - V2 launch/profile separation.
- ❌ Phase 5 - Smooth movement and viewport interpolation.
- ❌ Phase 7 - V2 verification suite.

## Theron's Quest

### Theron V1

- 🔧 Runtime handoff/playability hardening: hash-verified JP Rev 1 and US Track 02 ISO images now route into the M11 Theron boot/world/viewport path; remaining work is promotion of exact Track 02 dungeon-bank offsets beyond the deterministic launch room.
- 🔧 Track 02 bank map: identify and document JP Rev 1 and US ISO offsets for dungeon descriptors, map grids, object tables, champion/party seed data, text strings, palette/tile banks, and audio markers. Keep each offset behind a regression probe before claiming parity.
- 🔧 Real dungeon loader: replace synthetic/fallback launch-room construction with parsed Track 02 room grids, square flags, stairs/pits/doors/teleporters, and object/creature placements for dungeon 1 first, then all seven Theron dungeons.
- 🔧 Runtime input/playability proof: run a deterministic script from direct launch through movement, turning, wall blocking, door interaction, pit/stair routing, altar/progression state, combat contact, and save/load resume using hash-verified Track 02 data.
- 🔧 Rendering proof with real assets: bind real Track 02 tile/palette data into the Theron viewport where available, keep deterministic fallback only for unknown banks, and add pixel/screenshot gates for wall/floor/UI/champion panel output.
- 🔧 README-eligible screenshots: capture real Firestaff Theron V1 runtime screenshots from verified Track 02 data and store only genuine in-game captures in `verification-screens/` or `docs/compare/`.
- 🔧 Availability/direct-launch coverage: add focused tests for JP Rev 1 ISO and US ISO version fallback selection, runtime data-dir selection, isolated `--data-dir` scans, and `FIRESTAFF_EXIT_AFTER_LAUNCH=1` direct launch.
- 🔧 Scanner performance around Theron: direct hash-verified Track 02 file scans now skip root-wide rescans; remaining work is profiling and reusing startup/menu scan state across repeated launcher availability refreshes.
- 🔧 Source/provenance documentation: keep Theron docs honest about current state: Track 02 images are hash-verified and launchable, but exact dungeon-bank parity remains active work until real-offset probes pass.

### Theron V2.0 / V2.1 / V2.2

- ❌ Phase 0 - V1 compatibility lock before V2 work.
- ❌ Phase 1 - V2 launch/profile separation.
- ❌ Phase 2 - Enhanced asset pipeline.
- ❌ Phase 3 - Enhanced UI overlays.
- ❌ Phase 4 - Enhanced lighting/effects.
- ❌ Phase 5 - Smooth movement and viewport interpolation.
- ❌ Phase 6 - Touch/controller ergonomics.
- ❌ Phase 7 - V2 verification suite.

## Cross-Cutting

### Launcher and Settings

- 🔧 Start-menu feature hardening: first-pass persistence exists for quick resume, minimap, automap, combat log, soundtrack, ambient audio, UI scale, streamer mode, custom music, custom dungeon, screenshot path, and all five per-game option slots; remaining work is polished UI flow, runtime handoff for every option, save export/import, session timer, manual/docs launcher, cloud sync, and Custom/V2 smooth-turn-pan toggles.

### Touch and Controller Support

- ❌ Gesture navigation for runtime movement and turning.
- ❌ UI scaling and touch-target audit across launcher and game views.

### Accessibility

- ❌ Screen reader integration for launcher and game-critical state.
- 🔧 High-contrast presentation hardening: launcher output is remapped to a restricted high-contrast palette; remaining work is in-game overlay coverage.
- 🔧 Configurable font sizing hardening: launcher `fontScale` affects M12 text rendering; remaining work is in-game overlays and UI-fit coverage.

## Known Bugs

- 🐛 Viewport/collision reports without capture manifests must stay as bugs until paired original PC 3.4 evidence or a reproducible local probe exists.
