# Pass504 - DM1 V1 original route-state diversity blocker

Status: BLOCKED_PASS504_ORIGINAL_ROUTE_STATE_DELTA_DIVERSITY_NOT_PROVEN

## Decision

Original route-state diversity is still blocked at the post-command state-delta proof boundary. The current pass376/pass435 artifacts are useful as quarantined regression inputs, but they do not prove that distinct raw/cropped states came after source-visible F0380 -> F0365/F0366 -> G0321 -> F0128/F0097 transitions.

## ReDMCSB source audit

- GAMELOOP.C:90,164,215-219 F0002_MAIN_GameLoop_CPSDF - ok=True; A useful screenshot must be after F0380 has changed state and the next outer-loop F0128 redraw consumes that new party tuple.
- COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC - ok=True; Route labels are not evidence unless the original run proves this queue pop/load and the matching F0365/F0366 branch.
- CLIKMENU.C:142-174 F0365_COMMAND_ProcessTypes1To2_TurnParty - ok=True; A turn state delta must show the stop-wait write and the party-direction mutation.
- CLIKMENU.C:180-323 F0366_COMMAND_ProcessTypes3To6_MoveParty - ok=True; A movement capture must distinguish accepted movement from source-visible blocked/no-op movement.
- DUNVIEW.C:8318-8610 F0128_DUNGEONVIEW_Draw_CPSF - ok=True; The frame diversity check is only meaningful if the frame came from this updated direction/X/Y composition.
- DRAWVIEW.C:709-858 F0097_DUNGEONVIEW_DrawViewport - ok=True; A crop belongs after the PC34 viewport present boundary, not after setup/menu echo.

## Current evidence

- pass435 status: BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY
- pass498 status: PASS498_ORIGINAL_CAPTURE_BLOCKER_NARROWED_TO_POST_COMMAND_STATE_DELTA
- class sequence matches expected: False
- raw duplicate route indices: {'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': [1, 3, 4, 6], 'fbeb1b82cd096c15c2346f254d9b2b2e8c1a8d0b8d100ba1751c4230c51e3dde': [2, 5]}
- crop duplicate route indices: {'701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c': [1, 3, 4, 6], '1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81': [2, 5]}
- crop rows all 224x136: True

## Route matrix

| # | expected | actual | class ok | raw sha | crop sha |
|---|---|---|---|---|---|
| 1 | dungeon_gameplay | dungeon_gameplay | True | 48ed3743ab6a | 701689e73fc0 |
| 2 | dungeon_gameplay | wall_closeup | False | fbeb1b82cd09 | 1e71ed879980 |
| 3 | dungeon_gameplay | dungeon_gameplay | True | 48ed3743ab6a | 701689e73fc0 |
| 4 | spell_panel | dungeon_gameplay | False | 48ed3743ab6a | 701689e73fc0 |
| 5 | dungeon_gameplay | wall_closeup | False | fbeb1b82cd09 | 1e71ed879980 |
| 6 | inventory | dungeon_gameplay | False | 48ed3743ab6a | 701689e73fc0 |

## Remaining blocker

- current route classes do not match the expected six-state semantic sequence
- current raw route captures contain duplicate frame hashes
- current viewport crops contain duplicate hashes

## Required next evidence

- capture route begins from a verified post-entry gameplay state, not an entrance/setup echo
- each command label is backed by original F0380 queue pop/load/decrement evidence
- turn commands prove F0365 and direction mutation; movement commands prove F0366 or source-visible blocked/no-op handling
- G0321 stop-wait and game-time tick let the wait loop exit before the sampled frame
- the sampled frame is after the later F0128 and F0097/VIDRV present boundary for the same command
- six raw 320x200 frames and six 224x136 viewport crops are class-matching and non-duplicate by hash

Manifest: parity-evidence/verification/pass504_dm1_v1_original_route_state_delta_diversity_blocker/manifest.json
