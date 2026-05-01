# Pass206 — DM1 V1 original-runner minimal gate

Status: `BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE`

Scope: N2 Linux-only original DM1 PC34 runner/capture readiness for movement/viewport evidence. No Firestaff renderer code changed; no pixel parity is claimed.

## ReDMCSB source audit

- PASS `pc34-movement-input-bindings` — COMMAND.C:396-402, COMMAND.C:670-675: Original PC34 exposes movement by right-panel zones and keyboard scan codes before command dispatch.
- PASS `command-dispatch-to-turn-step` — COMMAND.C:2096-2156: The route must reach command processing, not just produce visually changing frames.
- PASS `turn-step-state-mutation-and-wait-stop` — CLIKMENU.C:156-173, CLIKMENU.C:237-347: Successful turn/step captures must be after the source movement handler has stopped the input wait loop.
- PASS `game-loop-redraw-cadence` — GAMELOOP.C:90-90, GAMELOOP.C:215-219: A movement/viewport gate must sample after redraw from current party state, not during menu/input churn.
- PASS `viewport-draw-and-present-seam` — DUNVIEW.C:8318-8616: The viewport reference is the dungeon-view draw from updated direction/map coordinates.
- PASS `viewport-present-request` — DRAWVIEW.C:709-724, DRAWVIEW.C:840-858: The comparison seam is the presented 224x136 viewport after the draw request/vblank path.

## N2 runner prerequisites

- `dosbox`: `/usr/bin/dosbox`
- `xvfb-run`: `/usr/bin/xvfb-run`
- `xdotool`: `/usr/bin/xdotool`
- `python3`: `/usr/bin/python3`
- `convert`: `/usr/bin/convert`
- `ffmpeg`: `/usr/bin/ffmpeg`

- PASS `DM.EXE` `4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4` at `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DM.EXE`
- PASS `DATA/DUNGEON.DAT` `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` at `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT`
- PASS `DATA/GRAPHICS.DAT` `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` at `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT`

## Reproducible dry-run command

```sh
DM1_ORIGINAL_STAGE_DIR=/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 \
DOSBOX=/usr/bin/dosbox \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:1500 shot:party_hud kp5 wait:700 shot kp5 wait:700 shot f1 wait:700 shot:spell_panel one wait:700 shot i wait:700 shot:inventory_panel' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run
```

## Existing N2 route attempt audit

- Attempt: `/home/trv2/work/firestaff/verification-screens/pass112-n2-stable-hud-route`
- Classifier status: `BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE`
- Capture count/dimensions: `6` / `{'320x200': 6}`
- Viewport crop PPM count: `6`
- Class counts: `{'dungeon_gameplay': 2, 'wall_closeup': 4}`
- Duplicate SHA counts >1: `{'ee7741746ea9b30739238e9f0780f57982bd0abe07bf60cea24e9cf92018e89c': 4}`

### Semantic mismatches blocking promotion

- shot 3: `wall_closeup` expected `dungeon_gameplay` (`verification-screens/pass112-n2-stable-hud-route/image0003-raw.png`)
- shot 4: `wall_closeup` expected `spell_panel` (`verification-screens/pass112-n2-stable-hud-route/image0004-raw.png`)
- shot 5: `wall_closeup` expected `dungeon_gameplay` (`verification-screens/pass112-n2-stable-hud-route/image0005-raw.png`)
- shot 6: `wall_closeup` expected `inventory` (`verification-screens/pass112-n2-stable-hud-route/image0006-raw.png`)

## Decision

This is a landable gate because it separates three facts cleanly:

1. ReDMCSB source seams for movement/viewport capture are cited and checked.
2. N2 has the Linux runner prerequisites and exact PC34 input hashes for a reproducible DOSBox capture attempt.
3. The current six-shot route is **not promotable** as original movement/viewport evidence because semantic classifier mismatches and duplicate frames remain.

Non-claims: no DANNESBURK use, no push, no original-vs-Firestaff pixel parity claim.
