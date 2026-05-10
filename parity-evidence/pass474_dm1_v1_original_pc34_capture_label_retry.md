# Pass474 — DM1 V1 original PC34 capture-label retry

Status: BLOCKED_ORIGINAL_PC34_ROUTE_INPUT_NO_STATE_CHANGE

Timestamp: 2026-05-10T04:32:00+00:00

## ReDMCSB source audit before retry
- `COMMAND.C:106-114` — PC34 mouse input zones are command-bearing inputs; arbitrary host coordinates are not evidence unless they hit source command zones.
- `COMMAND.C:397-403` — PC34/I34E mouse dispatch enters the same command queue space as keyboard movement.
- `COMMAND.C:2045-2156` — movement labels are valid only after queued commands survive cooldown/disabled-movement gates and dispatch to handlers.
- `CLIKMENU.C:142-174` — turn labels must follow `F0365_COMMAND_TurnParty`, including direction mutation and sensor remove/add around the turn.
- `CLIKMENU.C:180-347` — step/blocked labels must follow `F0366_COMMAND_ProcessTypes12To27_ClickInDungeonViewTouchFrontWallPressSensorOrMoveParty`; legal moves update party X/Y via `F0267_MOVE_GetMoveResult_CPSCE`, while blocked movement is a no-coordinate-change outcome.
- `DUNGEON.C:1371-1392` — `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` defines relative forward/right vectors for route labels; labels must bind to source tuple changes, not visual guesses.
- `DUNVIEW.C:8318-8611` and `DRAWVIEW.C:709-858` — comparable viewport evidence is after `F0128_DUNGEONVIEW_Draw_CPSF` and PC34 viewport presentation, not setup/menu/pre-blit frames.

## Fresh N2 attempts
Used N2-local PC34 stage only: `$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34`.

A small runner fix made `scripts/dosbox_dm1_original_viewport_reference_capture.sh` prefer the existing Pillow crop path before ImageMagick 6 `convert`, because `convert` failed during extensionless PPM crop normalization.

Artifacts:
- `verification-screens/pass474-n2-original-pc34-forward-west-blocked-fresh/`
- `verification-screens/pass474-n2-original-pc34-forward-south-corridor-fresh/`

Both runs reached six `dungeon_gameplay` frames and wrote normalized 224x136 manifests, but each run produced one repeated raw-frame SHA and one repeated viewport SHA across all six labels:
- raw SHA: `48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397`
- viewport SHA: `701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c`

Therefore the requested fresh labels `forward_west_blocked` and `forward_south_corridor` were captured as files, but are **not promotable**: the route input did not produce source-visible state changes between labels. The exact blocker remains host/DOSBox input delivery or source-stop synchronization after entering gameplay; it is no longer crop normalization.

## Gates
- `bash -n scripts/dosbox_dm1_original_viewport_reference_capture.sh`
- `python3 tools/verify_pass473_dm1_v1_movement_viewport_wall_capture_contract.py`
- two fresh DOSBox PC34 capture runs above
- `python3 tools/pass80_original_frame_classifier.py ...` on both fresh dirs (expected nonzero because duplicate frames/no route state change)
