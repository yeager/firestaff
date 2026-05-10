# CSB V1 parity matrix

Last updated: 2026-05-09

Scope: conservative CSB V1 definition-of-done matrix for the Atari ST v2.x lane. This matrix is a source/evidence contract only; it does not enable CSB launch, rendering, gameplay, save compatibility, or original-overlay claims.

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
| `launch_smoke` | 1/10 | `POSITIVE_BLOCKER_RENDER_SMOKE` | A CSB launch/render smoke must prove CSB menu/config/load routing without falling through the DM1-only startup gate. Current proof is a positive front-door render smoke plus a deliberate runtime blocker: matched CSB assets can render options/blocker surfaces, but cannot request runtime launch. | Add an experimental CSB launch-intent fixture only after an explicit media manifest and loader contract covers paired dungeon/game/save/graphics payloads; do not enable production CSB launch. |
| `core_input_movement` | 0/15 | `BLOCKED_RUNTIME` | CSB input must prove mode-specific mouse/keyboard routing from CSB state, including Utility/reincarnate/adventuring modes. | Add CSB state-backed input/movement fixtures. |
| `viewport_ui_render` | 0/20 | `BLOCKED_CAPTURE` | Viewport/HUD/UI parity must use stable CSB original capture/state anchors tied to the Atari ST v2.x renderer lane. | Build capture/overlay fixtures and compare against Firestaff output. |
| `gameplay_systems` | 0/15 | `BLOCKED_RUNTIME` | Prison/champion/new-adventure/combat/creature/item/save behavior cannot inherit DM1 points; it needs CSB source/runtime gates. | Land narrow CSB gameplay source/runtime gates. |
| `audio_timing` | 0/10 | `BLOCKED_RUNTIME` | CSB audio/timing must prove trigger cadence and overlap from CSB references. | Add CSB-specific audio/timing source and runtime evidence. |
| `original_overlay_regression` | 0/10 | `BLOCKED_CAPTURE` | Representative CSB original overlays are required before regression points count. | Produce original-vs-Firestaff overlay regression fixtures. |

## CSB front-door render smoke and launch blocker gate

- `csb_v1_launch_blocker_m12` forces a matched CSB Atari ST version in the M12 launcher, clicks into the CSB options view, verifies both the options and blocker-message views render nonblank startup/menu pixels, then clicks the CSB launch row. It verifies `launchRequested == 0` plus `M12_StartupMenu_GetLaunchIntent(...).valid == 0`. This is a positive front-door render smoke with a deliberate runtime blocker: it prevents matched CSB assets from falling into the DM1-only runtime while preserving diagnostic game/version identity.

## Source-lock anchors audited by the verifier

- ReDMCSB `DEFS.H` lines 468-523: CSB save header format and CSB dungeon identifiers.
- ReDMCSB `CEDTINC8.C` lines 101-118: save-file routing separates `DMSAVE.DAT` and `CSBGAME.DAT`.
- ReDMCSB `CEDTINCH.C` lines 5-64: Make-New-Adventure gate requires a valid CSB game dungeon.
- ReDMCSB `DUNVIEW.C` lines 380-390 and 4547-5205: viewport boxes and object/creature/projectile/explosion draw stack inherited by the CSB parity surface contract.
- CSB lineage `Chaos.cpp`, `Mouse.cpp`, `Graphics.cpp`, and `README`: Utility flow, mode routing, graphics payload boundaries, and required runtime files.
- CSBWin `Game/readme.txt`, `SaveGame.cpp`, `Mouse.cpp`, and `data.cpp`: play workflow, dungeon index usage, viewport/inventory mouse partition, and keyboard modes.

## Non-claims

- No CSB runtime, launch, render, gameplay, save compatibility, or pixel parity is claimed by this matrix.
- No Firestaff runtime code is modified by this matrix.
- DM1 gates cannot be counted as CSB V1 completion without CSB-specific evidence.

## Verification

```sh
python3 -m py_compile tools/verify_csb_v1_completion_matrix.py
python3 tools/verify_csb_v1_completion_matrix.py
python3 tools/verify_firestaff_completion_matrix.py
python3 tools/firestaff_completion_status.py
```

Evidence JSON: `parity-evidence/verification/csb_v1_completion_matrix.json`
