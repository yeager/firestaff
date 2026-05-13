# Pass435 follow-up - DM1 V1 movement/viewport original-route blocker

Status: `BLOCKED_DUPLICATE_ORIGINAL_MOVEMENT_ROUTE`

Scope: N2-only evidence for the pass435/pass434/pass376 original overlay/capture readiness lane. This does not change movement implementation, viewport wall tests, renderer behavior, or parity claims.

## Result

The current original PC34 route is executable on N2 and reaches original DM1 gameplay frames, but it still cannot unblock movement/viewport/walls overlay promotion. A fresh six-shot movement-only attempt reproduced the pass376/pass435 blocker exactly: only two unique raw frame hashes were produced, and the route alternates between one repeated `dungeon_gameplay` frame and one repeated `wall_closeup` frame.

This narrows the blocker to route-state diversity / post-command state-delta evidence, not missing original assets, not missing DOSBox capture tooling, and not ReDMCSB source ambiguity.

## Fresh N2 Attempt

- worktree: `/home/trv2/work/firestaff-worktrees/pass435-route-capture-unblock-20260513-codex`
- attempt dir: `verification-screens/pass435-20260513-movement-route-attempt`
- runner: `xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run`
- original stage: `~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34`
- program: `DM -vv -sn -pk`
- labels: `movement_initial`, `movement_turn_left`, `movement_turn_right`, `movement_forward`, `movement_turn_right_2`, `movement_turn_left_2`

Classifier command:

```sh
python3 tools/pass80_original_frame_classifier.py \
  verification-screens/pass435-20260513-movement-route-attempt \
  --expected dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay \
  --fail-on-duplicates
```

Classifier result:

- pass: `False`
- classes: `dungeon_gameplay, wall_closeup, dungeon_gameplay, dungeon_gameplay, wall_closeup, dungeon_gameplay`
- repeated raw hashes:
  - `48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397` x4
  - `fbeb1b82cd096c15c2346f254d9b2b2e8c1a8d0b8d100ba1751c4230c51e3dde` x2
- blocker problems:
  - `image0002-raw.png` classified `wall_closeup`, expected `dungeon_gameplay`
  - `image0005-raw.png` classified `wall_closeup`, expected `dungeon_gameplay`
  - duplicate raw frames detected

## Original Asset Lock

The fresh attempt used N2-local original DM1 PC34 assets:

- `DM.EXE`: `4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4`
- `DATA/GRAPHICS.DAT`: `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
- `DATA/DUNGEON.DAT`: `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`

Required local references were present on N2: `firestaff-redmcsb-source`, `firestaff-greatstone-atlas`, `firestaff-csbwin-source/CSBWin`, `firestaff-csb-source/CSB`, and `firestaff-original-games/DM`.

## ReDMCSB Source Audit

These are the source boundaries that make duplicate screenshot hashes non-promotable for movement/viewport/walls:

- `COMMAND.C:2045` `F0380_COMMAND_ProcessQueue_CPSC` owns queue dequeue; `COMMAND.C:2151` dispatches turns to `F0365`; `COMMAND.C:2155` dispatches moves to `F0366`.
- `CLIKMENU.C:142` `F0365_COMMAND_ProcessTypes1To2_TurnParty` owns accepted turns; `CLIKMENU.C:172` mutates party direction with `F0284_CHAMPION_SetPartyDirection(...G0308_i_PartyDirection...)`.
- `CLIKMENU.C:180` `F0366_COMMAND_ProcessTypes3To6_MoveParty` owns accepted movement; `CLIKMENU.C:269` computes target coordinates; `CLIKMENU.C:272-274` and `CLIKMENU.C:326-328` call `F0267_MOVE_GetMoveResult_CPSCE` / commit movement state.
- `DUNGEON.C:1371` `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` is the source coordinate transform used by movement and viewport projection.
- `LOADSAVE.C:1941-1943` loads the initial party X/Y/direction from `InitialPartyLocation`.
- `DUNVIEW.C:8318` `F0128_DUNGEONVIEW_Draw_CPSF` composes the viewport for a known direction/X/Y tuple; `DUNVIEW.C:8468-8542` walks far-to-near viewport squares and calls D3/D2/D1/D0 draw functions; `DUNVIEW.C:8610` presents through `F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)`.
- `DRAWVIEW.C:709` `F0097_DUNGEONVIEW_DrawViewport` owns the PC34 viewport-present boundary; `DRAWVIEW.C:847-857` resolves `C007_ZONE_VIEWPORT` and calls `VIDRV_09_BlitViewPort(G0296_puc_Bitmap_Viewport, ...)`.

## Decision

For movement/viewport/walls, the blocker is now narrower than the older mixed HUD/spell/inventory pass435 predicate:

- capture tooling works and materializes six 320x200 original frames plus six 224x136 viewport crops;
- source and original assets are present on N2;
- the current route is not promotable because it has only two unique visual states and repeated hashes;
- a wall-closeup is a valid wall-state observation, but the duplicated two-state loop does not prove a source-visible post-command movement/viewport sequence.

Next evidence must either produce a route with non-duplicate movement/turn/blocked-wall states tied to route labels, or pair the repeated visual states with runtime proof that the relevant commands were source-accepted as intentional no-op/blocked-wall outcomes through `F0380 -> F0365/F0366 -> F0128 -> F0097`.

Until then, keep pass376/pass435 captures quarantined as blocker evidence only.

## Non-claims

No original-vs-Firestaff pixel parity, no viewport wall parity promotion, no movement implementation change, and no claim that the original movement handlers are defective.
