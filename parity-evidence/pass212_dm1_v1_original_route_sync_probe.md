# Pass212 DM1 V1 original route/capture synchronization probe

Status: `BLOCKED_ROUTE_CAPTURE_SYNC`

Attempt audited: `verification-screens/pass210-n2-original-movement-route-fresh`

This is a manifest-only synchronization probe. It preserves why the original movement/viewport route is or is not promotable without committing raw DOSBox images or viewport crops.

## ReDMCSB seam audited

- `COMMAND.C`: `F0361_COMMAND_ProcessKeyPress`, `F0380_COMMAND_ProcessQueue_CPSC` — input commands must be queued/dequeued before movement evidence is captured.
- `CLIKMENU.C`: `F0365_COMMAND_ProcessTypes1To2_TurnParty`, `F0366_COMMAND_ProcessTypes3To6_MoveParty` — turn and movement handlers mutate party direction/position before redraw.
- `MOVESENS.C`: `F0267_MOVE_GetMoveResult_CPSCE` — movement legality/sensor processing gates post-command position changes.
- `GAMELOOP.C`: `F0002_MAIN_GameLoop_CPSDF` — the main loop owns command processing and redraw cadence.
- `DUNVIEW.C`: `F0128_DUNGEONVIEW_Draw_CPSF` — viewport cells are derived from party map/direction at draw time.
- `DRAWVIEW.C`: `F0097_DUNGEONVIEW_DrawViewport`, `M526_WaitVerticalBlank`, `VIDRV_09_BlitViewPort` — the promotable seam is the presented 224x136 viewport after vblank/blit.

## Shot audit

| shot | route label | classification | raw sha12 | viewport sha12 | reason |
|---:|---|---|---|---|---|
| 1 | `start` | `entrance_menu` | `17bd7e878157` | `701689e73fc0` | from committed pass211 blocker shot binding; raw payload intentionally untracked |
| 2 | `turn_left` | `wall_closeup` | `fbeb1b82cd09` | `1e71ed879980` | from committed pass211 blocker shot binding; raw payload intentionally untracked |
| 3 | `turn_right` | `dungeon_gameplay` | `48ed3743ab6a` | `701689e73fc0` | from committed pass211 blocker shot binding; raw payload intentionally untracked |
| 4 | `forward` | `dungeon_gameplay` | `48ed3743ab6a` | `701689e73fc0` | from committed pass211 blocker shot binding; raw payload intentionally untracked |
| 5 | `turn_left_2` | `wall_closeup` | `fbeb1b82cd09` | `1e71ed879980` | from committed pass211 blocker shot binding; raw payload intentionally untracked |
| 6 | `post_redraw` | `dungeon_gameplay` | `48ed3743ab6a` | `701689e73fc0` | from committed pass211 blocker shot binding; raw payload intentionally untracked |

## Blockers

- readiness shot is 'entrance_menu', not 'dungeon_gameplay'; party-control gameplay was not proven before movement capture
- movement shot 2 (turn_left) classified 'wall_closeup', not 'dungeon_gameplay'
- movement shot 5 (turn_left_2) classified 'wall_closeup', not 'dungeon_gameplay'
- duplicate raw-frame SHA groups show command/post-redraw collapse: fbeb1b82cd09=[2, 5], 48ed3743ab6a=[3, 4, 6]
- duplicate viewport-crop SHA groups show non-distinct presented viewports: 701689e73fc0=[1, 3, 4, 6], 1e71ed879980=[2, 5]

## Warnings

- using committed pass211 blocker manifest fallback because the raw attempt directory is absent/incomplete

## Retry contract

The next N2 route should use readiness/post-vblank waits and promote only if all six frames classify as `dungeon_gameplay` and both raw/crop SHA groups are unique.

```sh
OUT_DIR=$PWD/verification-screens/pass212-n2-original-movement-route-sync-retry DM1_ORIGINAL_STAGE_DIR=<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34 DOSBOX=/usr/bin/dosbox DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=3000 NEW_FILE_TIMEOUT_MS=6000 DM1_ORIGINAL_ROUTE_EVENTS="wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:2200 one wait:2500 shot:readiness_preflight wait:700 kp4 wait:900 shot:turn_left_after_vblank wait:700 kp6 wait:900 shot:turn_right_after_vblank wait:700 kp8 wait:1200 shot:forward_after_vblank wait:700 kp4 wait:900 shot:turn_left_2_after_vblank wait:700 kp6 wait:1200 shot:post_redraw_after_vblank" xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Honesty: No raw screenshots or viewport crops are emitted by this probe; hashes/manifests only.
