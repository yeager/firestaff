# Pass566 - DM1 V1 original overlay capture readiness

Status: `PASS566_DM1_V1_ORIGINAL_OVERLAY_CAPTURE_READINESS`

This is a small N2-local readiness manifest for the next original overlay/capture run. It does not generate screenshots or claim pixel parity. It fixes the immediate N2 blocker where the original capture harness defaulted to an in-repo staged tree that is absent in fresh worktrees, while N2 already has a stable local DM1 PC 3.4 stage.

## N2 original stage

- Default stage preferred by `scripts/dosbox_dm1_original_viewport_reference_capture.sh`: `$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34`
- Variant: `DM1 PC 3.4 DOS original stage`
- The readiness verifier hash-locks this stage's `DM.EXE`, `DATA/GRAPHICS.DAT`, and `DATA/DUNGEON.DAT`, plus canonical `GRAPHICS.DAT`, `DUNGEON.DAT`, and `TITLE`.

## ReDMCSB audit anchors

- `COMMAND.C` `G0459_as_Graphic561_PrimaryKeyboardInput_DungeonView` lines `677-683`: PC I34E/I34M movement keys map turn/move commands.
- `COMMAND.C` `F0380_COMMAND_ProcessQueue_CPSC` lines `2120-2156`: queued commands dispatch to `F0365_COMMAND_ProcessTypes1To2_TurnParty` and `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
- `CLIKMENU.C` `F0365_COMMAND_ProcessTypes1To2_TurnParty` lines `142-174`: turn commands update party direction through source sensor boundaries.
- `CLIKMENU.C` `F0366_COMMAND_ProcessTypes3To6_MoveParty` lines `180-346`: movement commands project destination coordinates, reject walls/closed doors/fake walls, and update disabled movement ticks.
- `DUNGEON.C` `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` lines `1371-1391`: relative forward/right movement projects map coordinates.
- `DUNVIEW.C` `F0128_DUNGEONVIEW_Draw_CPSF` lines `8318-8543`: viewport replay draws floor/ceiling and far-to-near D4/D3/D2/D1/D0 squares.
- `DRAWVIEW.C` `F0097_DUNGEONVIEW_DrawViewport` lines `709-858`: the prepared viewport bitmap is presented to the PC video driver.

## Next concrete capture command

```bash
OUT_DIR=$PWD/verification-screens/pass566-original-overlay-diagnostic \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 shot:title enter wait:1200 shot:pre_enter_menu click:260,50 wait:1200 shot:after_enter_click click:276,140 wait:600 shot:forward_1 click:276,140 wait:600 shot:forward_2 click:246,140 wait:600 shot:left_turn_probe' \
DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Run `python3 scripts/verify_pass566_dm1_v1_original_overlay_capture_readiness.py` before the capture. It writes `parity-evidence/verification/pass566_dm1_v1_original_overlay_capture_readiness/manifest.json`.

## Claim boundary

This pass proves only that the N2 source files, emulator tools, route shape, and ReDMCSB source anchors are ready. The next step still has to inspect/classify raw captures before any original-runtime pixel parity claim.
