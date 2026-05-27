# CSB V1 parity matrix

Last updated: 2026-05-10

Scope: conservative CSB V1 definition-of-done matrix for the Atari ST v2.x lane. This matrix now recognizes the hash-matched M12 launch/profile boundary; rendering, gameplay, save compatibility, New Adventure, and original-overlay parity still require their own gates.

Primary references stay local on N2:

- ReDMCSB: `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`
- CSB lineage: `~/.openclaw/data/firestaff-csb-source/CSB/src/`
- CSBWin mirror: `~/.openclaw/data/firestaff-csbwin-source/CSBWin/`
- Original anchors: `~/.openclaw/data/firestaff-original-games/DM/_canonical/csb/`

## Completion criteria

| Criterion | Current score | Status | Acceptance boundary | Required next proof |
|---|---:|---|---|---|
| `reference_inventory` | 8/10 | `SOURCE_LOCKED_PARTIAL` | CSB-specific source and original payload identities must be fixed before launch/runtime work counts. | Fill remaining platform/version inventory gaps without substituting Amiga/Atari assets. |
| `definition_matrix` | 10/10 | `MATCHED_DEFINITION_ONLY` | This document plus `csb_v1_parity_surface_matrix` define the CSB V1 DoD surfaces and non-claims. | Keep the matrix verified whenever completion points change. |
| `launch_smoke` | 4/10 | `PROFILE_INTENT_READY` | A CSB launch/render smoke must prove CSB menu/config/load routing without falling through the old DM1-only startup gate. Current proof is a positive front-door render smoke plus source-locked CSB reference QuickPlay/load-route boundaries: matched CSB assets can now request a CSB M12 launch intent, but full playability and capture parity remain outside this row. | Next proof must add runtime/capture handling after the CSB profile handoff. |
| `core_input_movement` | 0/15 | `BLOCKED_RUNTIME` | CSB input must prove mode-specific mouse/keyboard routing from CSB state, including Utility/reincarnate/adventuring modes. | Add CSB state-backed input/movement fixtures. |
| `viewport_ui_render` | 0/20 | `BLOCKED_CAPTURE` | Viewport/HUD/UI parity must use stable CSB original capture/state anchors tied to the Atari ST v2.x renderer lane. | Build capture/overlay fixtures and compare against Firestaff output. |
| `gameplay_systems` | 0/15 | `BLOCKED_RUNTIME` | Prison/champion/new-adventure/combat/creature/item/save behavior cannot inherit DM1 points; it needs CSB source/runtime gates. | Land narrow CSB gameplay source/runtime gates. |
| `audio_timing` | 0/10 | `BLOCKED_RUNTIME` | CSB audio/timing must prove trigger cadence and overlap from CSB references. | Add CSB-specific audio/timing source and runtime evidence. |
| `original_overlay_regression` | 0/10 | `BLOCKED_CAPTURE` | Representative CSB original overlays are required before regression points count. | Produce original-vs-Firestaff overlay regression fixtures. |

## CSB front-door render smoke and launch blocker gate

- `csb_v1_launch_blocker_m12` forces a matched CSB Atari ST version in the M12 launcher, clicks into the CSB options view, verifies both the options and ready-message views render nonblank startup/menu pixels, then clicks the CSB launch row. It verifies `launchRequested == 1` plus `M12_StartupMenu_GetLaunchIntent(...).valid == 1` for the CSB game ID. This is a positive front-door render smoke for the launcher/profile boundary, not a full gameplay claim.
- `csb_v1_experimental_launch_intent_fixture` is the reviewed CSB launch-intent fixture. It consumes `csb_v1_atari_asset_pair_manifest.json`, requires the selected Atari ST `GRAPHICS.DAT`/`DUNGEON.DAT` pair plus `HCSB.DAT`, `HCSB.HTC`, and `MINI.DAT`, audits ReDMCSB primary CSB dungeon/new-adventure gates, and verifies `menu_startup_m12.c` includes CSB in the production launch-intent guard. It keeps full runtime/render/gameplay parity as non-claims.
- `csb_v1_quickplay_load_route_source_lock` source-locks the CSB/CSBWin reference QuickPlay route as replay playback only (`PlayfileIsOpen()` gates `OPT_QUICKPLAY`) and the shared load path as dungeon/signature/graphics/timer setup (`_ReadEntireGame`, `openGraphicsFile`, `HandleMouseEvents`). It also audits ReDMCSB primary CSB save/new-adventure routing so this launch evidence cannot be misread as production CSB runtime support in Firestaff.
- pass547_csb_v1_runtime_readiness_backfill source-locks the remaining runtime/capture readiness blocker: ReDMCSB requires a CSB-specific save/header/runtime lane while Firestaff still blocks CSB launch before the downstream DM1-shaped M11 title/entrance handoff, CSB.DAT built-in resolver, and DM1 capture fixture. It keeps CSB V1 at 20/100 and makes no launch/runtime/render claim.

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

- No CSB render, gameplay, save compatibility, New Adventure, or pixel parity is claimed by this matrix.
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
python3 tools/firestaff_completion_status.py
```

Evidence JSON: `parity-evidence/verification/csb_v1_completion_matrix.json`
