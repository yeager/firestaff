# Pass209 — DM1 V1 original delayed-click movement/viewport gate

Status: `BLOCKED_STATIC_IDENTICAL_FRAMES_AFTER_DELAYED_ROUTE`

Scope: N2-only delayed-click follow-up for original movement/viewport/wall evidence. This gate records the source anchors and route diagnostics; it does not claim original pixel parity.

## ReDMCSB source audit before capture promotion

- PASS `relative-movement-vector-source` — `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` at DUNGEON.C:35-44, DUNGEON.C:1371-1391: Movement captures must be interpreted through the source relative direction-to-map-vector transform.
- PASS `command-dispatch-turn-step-boundary` — `F0380_COMMAND_ProcessQueue_CPSC` at COMMAND.C:2045-2156: A key/click route is movement evidence only after the original queue dispatch reaches the turn/step handlers.
- PASS `turn-handler-mutates-direction` — `F0365_COMMAND_ProcessTypes1To2_TurnParty` at CLIKMENU.C:142-179: Turn shots must happen after source direction mutation and input-wait release.
- PASS `step-handler-resolves-walls-and-cooldown` — `F0366_COMMAND_ProcessTypes3To6_MoveParty` at CLIKMENU.C:180-347: Forward/side shots must pass original wall/door legality and movement cooldown logic before a viewport delta is expected.
- PASS `game-loop-redraw-from-current-party-state` — `F0002_MAIN_GameLoop_CPSDF` at GAMELOOP.C:90-90, GAMELOOP.C:150-155, GAMELOOP.C:215-219: Promotable route frames must be sampled after the original game loop redraws from current party direction/X/Y.
- PASS `viewport-draw-square-order-and-present-request` — `F0128_DUNGEONVIEW_Draw_CPSF` at DUNVIEW.C:8318-8616: Viewport/wall comparisons are source-ordered dungeon-view draws ending in a viewport presentation request.
- PASS `floor-ceiling-and-wall-target-buffer` — `F0098_DUNGEONVIEW_DrawFloorAndCeiling / F0100_DUNGEONVIEW_DrawWallSetBitmap` at DUNVIEW.C:2962-3003, DUNVIEW.C:3048-3110: Wall evidence must be the composited 224x136 viewport buffer, not standalone wallset assets.
- PASS `presented-viewport-zone` — `F0097_DUNGEONVIEW_DrawViewport` at DRAWVIEW.C:709-724, DRAWVIEW.C:840-858: The comparable original anchor is the presented viewport zone after draw-request/vblank, not an arbitrary full-screen tick.

## Reproducible route attempted on N2

```sh
OUT_DIR=$PWD/verification-screens/pass209-delayed-click-zone-route \
DM1_ORIGINAL_STAGE_DIR=/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 \
DOSBOX=/usr/bin/dosbox \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:1500 shot:party_hud kp5 wait:700 shot kp5 wait:700 shot f1 wait:700 shot:spell_panel one wait:700 shot i wait:700 shot:inventory_panel' \
xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

- route shape: `{'tokens': 25, 'shot_count': 6, 'clicks': ['click:276,140'], 'labels': ['party_hud', 'spell_panel', 'inventory_panel'], 'invalid_tokens': []}`
- labels manifest rows: `6` from `verification-screens/pass209-delayed-click-zone-route/original_viewport_shot_labels.tsv`

## Delayed-click route diagnostics

- raw frames: `6`; dimensions: `['320x200']`; materialized ignored PNGs on N2: `0`
- raw SHA counts: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 6}`
- raw mtimes: `['2026-05-05T10:01:26.563584Z', '2026-05-05T10:01:26.563584Z', '2026-05-05T10:01:26.563584Z', '2026-05-05T10:01:26.563584Z', '2026-05-05T10:01:26.564584Z', '2026-05-05T10:01:26.564584Z']`
- viewport crops: `6`; dimensions: `['224x136']`; materialized ignored PPMs on N2: `0`
- viewport SHA counts: `{'701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c': 6}`
- classifier: pass=`False`, class_counts=`{'dungeon_gameplay': 6}`, unique_raw_sha256=`['48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397']`

## Blocker decision

The delayed-click route is **not promotable** for movement/viewport/wall parity. All six 320x200 raw frames have the same SHA-256, and all six normalized 224x136 viewport crops have the same SHA-256. That proves the route reached a stable dungeon-looking frame, but it does not prove original turn/step/menu dispatch or post-command viewport redraw.

Classifier mismatches:
- `verification-screens/pass209-delayed-click-zone-route/image0004-raw.png`: `dungeon_gameplay` expected `spell_panel` sha256=`48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397`
- `verification-screens/pass209-delayed-click-zone-route/image0006-raw.png`: `dungeon_gameplay` expected `inventory` sha256=`48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397`

Original anchors preserved: raw frame SHA `48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397`; viewport crop SHA `701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c`.

Non-claims: no DANNESBURK/192.168.2.126 use, no push, no original-vs-Firestaff pixel parity claim.
