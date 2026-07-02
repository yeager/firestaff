# CSB V1 parity matrix

Last updated: 2026-06-30

Scope: conservative CSB V1 definition-of-done matrix for the Atari ST v2.x lane. This matrix now recognizes the hash-matched M12 launch/profile boundary plus CTest-wired PC/synthetic boot/runtime/input/save/Utility/first-viewport/multi-step route slices, including a wall-blocked step and bounded save-prefix roundtrip after the final route state; broad real-data playability, CSBGAME.DAT save compatibility, New Adventure capture, original-overlay parity, and pixel parity still require their own gates.

Primary references stay local on N2:

- ReDMCSB: `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`
- CSB lineage: `~/.openclaw/data/firestaff-csb-source/CSB/src/`
- CSBWin mirror: `~/.openclaw/data/firestaff-csbwin-source/CSBWin/`
- Original anchors: `~/.openclaw/data/firestaff-original-games/DM/_canonical/csb/`

## Completion criteria

| Criterion | Current score | Status | Acceptance boundary | Required next proof |
|---|---:|---|---|---|
| `reference_inventory` | 9/10 | `SOURCE_LOCKED_PARTIAL` | CSB-specific source and original payload identities must be fixed before launch/runtime work counts. Current proof includes PC/Atari/Amiga references, source-lock manifests, archive/version inventories, and asset-pair manifests. | Fill the remaining platform/version inventory gap without substituting Amiga/Atari assets. |
| `definition_matrix` | 10/10 | `MATCHED_DEFINITION_ONLY` | This document plus `csb_v1_parity_surface_matrix` define the CSB V1 DoD surfaces and non-claims. | Keep the matrix verified whenever completion points change. |
| `launch_smoke` | 6/10 | `RUNTIME_BOUNDARY_VERIFIED_PARTIAL` | CSB launch/profile routing is source- and runtime-gated without falling through the old DM1-only startup path. Current proof includes a positive front-door render smoke, source-locked CSB reference QuickPlay/load-route boundaries, matched CSB assets requesting a CSB M12 launch intent, M11 handing CSB to `FS_GAME_CSB`, and the PC real-asset probe booting/ticking once. Full playability and capture parity remain outside this row. | Next proof must add CSB-specific capture/playability evidence beyond the PC boot/tick boundary. |
| `core_input_movement` | 5/15 | `RUNTIME_SLICE_VERIFIED_PARTIAL` | CSB input must prove mode-specific mouse/keyboard routing from CSB state. Current proof covers CTest-wired command-chain, input queue, movement step/rotation, runtime tick, reincarnation-adjacent gates, real/synthetic PC asset launch, quickplay dungeon-handle survival, composed boot/runtime handoff, and a bounded Utility NEW_GAME -> runtime -> repeated queued movement route with a wall-blocked step and save-prefix roundtrip. It is not yet broad adventuring playability. | Drive a wider real-data CSB route through Utility/reincarnate/adventuring input, interactions beyond movement, and CSBGAME.DAT save/reload. |
| `viewport_ui_render` | 7/20 | `SOURCE_SLICE_VERIFIED_PARTIAL` | Viewport/HUD/UI parity must use stable CSB original capture/state anchors tied to the Atari ST v2.x renderer lane. Current proof covers CSB-specific viewport source slices, CustomBackgrounds, D1/D2/D3 wall/door/ornament lanes, hidden item skip tables, portrait render handoff, first viewport-frame render entry, and a post-movement viewport render without panel bleed; real original-vs-Firestaff pixel captures remain open. | Build capture/overlay fixtures and compare original CSB frames against Firestaff output. |
| `gameplay_systems` | 4/15 | `RUNTIME_SLICE_VERIFIED_PARTIAL` | Prison/champion/new-adventure/combat/creature/item/save behavior cannot inherit DM1 points. Current proof covers dungeon loader/world slices, DSA trigger, save runtime boundary, Utility/CMP import, imported party handoff, leader/rotation state, and related Grey Lord/Zokathra/chaos slices; broad mechanics parity remains open. | Prove a real CSB prison/new-adventure/combat/creature/item/save route with representative source/runtime gates. |
| `audio_timing` | 2/10 | `DATA_DECODE_VERIFIED_PARTIAL` | CSB audio/timing must prove trigger cadence and overlap from CSB references. Current proof fixes AMG Utility Disk sound decoding for documented SND2 files; runtime playback/rate binding and timing proof remain open. | Add CSB-specific audio/timing source and runtime evidence. |
| `original_overlay_regression` | 0/10 | `BLOCKED_CAPTURE` | Representative CSB original overlays are required before regression points count. | Produce original-vs-Firestaff overlay regression fixtures. |

## CSB front-door render smoke and launch blocker gate

- `csb_v1_launch_blocker_m12` forces a matched CSB Atari ST version in the M12 launcher, clicks into the CSB options view, verifies both the options and ready-message views render nonblank startup/menu pixels, then clicks the CSB launch row. It verifies `launchRequested == 1` plus `M12_StartupMenu_GetLaunchIntent(...).valid == 1` for the CSB game ID. This is a positive front-door render smoke for the launcher/profile boundary, not a full gameplay claim.
- `csb_v1_experimental_launch_intent_fixture` is the reviewed CSB launch-intent fixture. It consumes `csb_v1_atari_asset_pair_manifest.json`, requires the selected Atari ST `GRAPHICS.DAT`/`DUNGEON.DAT` pair plus `HCSB.DAT`, `HCSB.HTC`, and `MINI.DAT`, audits ReDMCSB primary CSB dungeon/new-adventure gates, and verifies `menu_startup_m12.c` includes CSB in the production launch-intent guard. It keeps full runtime/render/gameplay parity as non-claims.
- `csb_v1_quickplay_load_route_source_lock` source-locks the CSB/CSBWin reference QuickPlay route as replay playback only (`PlayfileIsOpen()` gates `OPT_QUICKPLAY`) and the shared load path as dungeon/signature/graphics/timer setup (`_ReadEntireGame`, `openGraphicsFile`, `HandleMouseEvents`). It also audits ReDMCSB primary CSB save/new-adventure routing so this launch evidence cannot be misread as full CSB runtime parity in Firestaff.
- pass547_csb_v1_runtime_readiness_backfill now verifies the retired launch-readiness blocker: M12 admits CSB launch intent for matched assets, M11 hands CSB to `FS_GAME_CSB`, and `csb_v1_pc_real_asset_launch` proves PC CSB scan, boot, dungeon ownership and one tick. It remains a readiness boundary, not a full runtime/render/gameplay/pixel-parity claim.
- `csb_v1_boot_runtime_handoff` is the composed CTest spine for the current partial runtime score. It proves verified profile -> runtime dungeon handle -> imported party -> leader/rotation runtime state -> one deterministic tick -> Utility `NEW_GAME` handoff. It remains synthetic-data evidence; it does not claim real-data CSB playability, original capture, or pixel parity.
- The current fast CSB runtime/viewport smoke suite is `csb_v1_pc_real_asset_launch`, `csb_v1_pc34_quickplay_dungeon_handle`, `csb_v1_first_viewport_frame`, `csb_v1_boot_runtime_handoff`, and `csb_v1_runtime_route_first_frame_movement_utility_gate`. These CTests are now self-contained instead of linking the broad `firestaff_m10` library. Together they prove skip-safe PC real-asset scan/boot/tick, verified dungeon-handle survival/rescan cleanup, first M11 viewport-frame render entry, composed runtime/Utility handoff, and a bounded Utility `NEW_GAME` -> runtime -> repeated queued movement route with a wall-blocked forward step, post-route viewport render, and save-prefix roundtrip. They still do not claim end-to-end CSB playability, original capture, full save compatibility, or pixel parity.

## Source-lock anchors audited by the verifier

- ReDMCSB `DEFS.H` lines 468-523: CSB save header format and CSB dungeon identifiers.
- ReDMCSB `CEDTINC8.C` lines 101-118: save-file routing separates `DMSAVE.DAT` and `CSBGAME.DAT`.
- ReDMCSB `CEDTINCH.C` lines 5-64: Make-New-Adventure gate requires a valid CSB game dungeon.
- ReDMCSB `CEDTINCU.C` lines 5-77: `F7272_IsDungeonValid` switches on save-header format and accepts CSB prison/game IDs only through CSB-aware validation criteria.
- ReDMCSB `HINTLOAD.C` lines 11-18 and 300-386: Atari CSB hint/runtime loader names `HCSB.HTC`, `HCSB.DAT`, `CSBGAME.DAT`, `CSBGAME.BAK`, opens `CSBGAME.DAT`, and requires `C13_DUNGEON_CSB_GAME` plus `C1_PLATFORM_ATARI_ST`.
- ReDMCSB `FLOPPYST.C` lines 7-18: Atari CSB save filenames are `A:\CSBGAME.DAT` and `A:\CSBGAME.BAK`, separate from DM save naming.
- ReDMCSB `DUNVIEW.C` lines 380-390 and 4547-5205: viewport boxes and object/creature/projectile/explosion draw stack inherited by the CSB parity surface contract.
- CSB lineage `Chaos.cpp`, `Mouse.cpp`, `Graphics.cpp`, and `README`: Utility flow, mode routing, graphics payload boundaries, and required runtime files.
- CSB lineage/CSBWin `CSBwin.cpp`, `SaveGame.cpp`, and `CSB.h`: QuickPlay playback-only gate, load/game-state constants, dungeon signature/open path, graphics signature check, timer initialization, and mouse initialization.
- CSBWin `Game/readme.txt`, `SaveGame.cpp`, `Mouse.cpp`, and `data.cpp`: play workflow, dungeon index usage, viewport/inventory mouse partition, and keyboard modes.
- Reference anchors: original N2 canonical CSB README locks `atari-DUNGEON.DAT` / `atari-GRAPHICS.DAT` hashes; Greatstone `g_csb.html` lines 272-323 confirm Atari CSB 2.0/2.1 entries separate `dungeon.dat`, `graphics.dat`, `mini.dat`, `hcsb.dat`, and `hcsb.htc`. These are supporting references only; ReDMCSB remains primary.

## Non-claims

- No full CSB runtime, render, gameplay, save compatibility, or pixel parity is claimed by this matrix.
- No Firestaff runtime code is modified by this matrix.
- DM1 gates cannot be counted as CSB V1 completion without CSB-specific evidence.

## Verification

```sh
python3 -m py_compile tools/verify_csb_v1_completion_matrix.py
python3 tools/verify_csb_v1_completion_matrix.py
python3 -m py_compile tools/verify_csb_v1_experimental_launch_intent_fixture.py
python3 tools/verify_csb_v1_experimental_launch_intent_fixture.py
python3 tools/verify_firestaff_completion_matrix.py
python3 -m py_compile tools/verify_csb_v1_quickplay_load_route_source_lock.py
python3 tools/verify_csb_v1_quickplay_load_route_source_lock.py
python3 -m py_compile tools/verify_pass547_csb_v1_runtime_readiness_backfill.py
python3 tools/verify_pass547_csb_v1_runtime_readiness_backfill.py
cmake --build /tmp/firestaff-cmake-probe --target firestaff_csb_v1_pc_real_asset_launch_probe firestaff_csb_v1_pc34_quickplay_dungeon_handle_probe firestaff_csb_v1_first_viewport_frame_probe test_csb_v1_boot_runtime_handoff test_csb_v1_runtime_route_first_frame_movement_utility_gate
ctest --test-dir /tmp/firestaff-cmake-probe -R "csb_v1_(pc_real_asset_launch|pc34_quickplay_dungeon_handle|first_viewport_frame|boot_runtime_handoff|runtime_route_first_frame_movement_utility_gate)" --output-on-failure
python3 tools/firestaff_completion_status.py
```

Evidence JSON: `parity-evidence/verification/csb_v1_completion_matrix.json`
