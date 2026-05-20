# Pass608 - DM1 V1 same-viewport capture blocker

Status: BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE

## Decision

No promotable same-viewport original/Firestaff manifest exists yet. Firestaff has exact fixture state and viewport hashes, and original PC34 assets/tools are present, but the latest N2 original diagnostic still lacks the command-state-redraw-present transcript required to bind an original frame to any Firestaff map/X/Y/direction row.

## Source audit

- COMMAND.C:106-114 G0448_as_Graphic561_SecondaryMouseInput_Movement ok=True - PC34 mouse route tokens must hit source command zones before labels mean anything.
- GAMELOOP.C:164-219 F0002_MAIN_GameLoop_CPSDF ok=True - A capture must be after command processing lets the wait loop exit.
- COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC ok=True - Each original shot needs F0380 pop/count delta and F0365/F0366 dispatch.
- CLIKMENU.C:142-174 F0365_COMMAND_ProcessTypes1To2_TurnParty ok=True - Turn shots must prove source direction mutation.
- CLIKMENU.C:180-347 F0366_COMMAND_ProcessTypes3To6_MoveParty ok=True - Move shots must prove accepted movement or source-visible blocked/no-op handling.
- DUNVIEW.C:8318-8611 F0128_DUNGEONVIEW_Draw_CPSF ok=True - The viewport bitmap must be composed from the same direction/X/Y tuple.
- DRAWVIEW.C:709-858 F0097_DUNGEONVIEW_DrawViewport ok=True - A promotable crop belongs after the PC34 viewport-present blit.

## Firestaff fixture

- 01_ingame_start_latest: map=0 x=1 y=3 dir=2 tick=0 spell=0 inventory=0
- 02_ingame_turn_right_latest: map=0 x=1 y=3 dir=3 tick=1 spell=0 inventory=0
- 03_ingame_move_forward_latest: map=0 x=0 y=3 dir=3 tick=2 spell=0 inventory=0
- 04_ingame_spell_panel_latest: map=0 x=0 y=3 dir=3 tick=2 spell=1 inventory=0
- 05_ingame_after_cast_latest: map=0 x=0 y=3 dir=3 tick=3 spell=0 inventory=0
- 06_ingame_inventory_panel_latest: map=0 x=0 y=3 dir=3 tick=3 spell=0 inventory=1

## Fresh original diagnostic

- Command: `OUT_DIR=$PWD/verification-screens/pass601-same-viewport-original-diagnostic DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=5000 NEW_FILE_TIMEOUT_MS=6000 DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 shot:title enter wait:1200 shot:pre_enter_menu click:260,50 wait:1200 shot:after_enter_click click:276,140 wait:600 shot:forward_1 click:276,140 wait:600 shot:forward_2 click:246,140 wait:600 shot:left_turn_probe' DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run`
- Transcript scaffold: SCAFFOLD_ONLY_MISSING_ORIGINAL_RUNTIME_DEBUG_FIELDS rows=0

| # | label | expected | actual | raw sha | crop sha |
|---|---|---|---|---|---|
| 1 | title | title_or_menu | graphics_320x200_unclassified | `6176af21cb32` | `358136006c6d` |
| 2 | pre_enter_menu | entrance_menu | entrance_menu | `9f95e1d8fae6` | `ea845264f922` |
| 3 | after_enter_click | dungeon_gameplay | entrance_menu | `17bd7e878157` | `701689e73fc0` |
| 4 | forward_1 | dungeon_gameplay | entrance_menu | `17bd7e878157` | `701689e73fc0` |
| 5 | forward_2 | dungeon_gameplay | dungeon_gameplay | `355a191cd07b` | `701689e73fc0` |
| 6 | left_turn_probe | dungeon_gameplay | dungeon_gameplay | `48ed3743ab6a` | `701689e73fc0` |

## Blockers

- fresh original diagnostic class sequence is not the expected title/menu -> entrance -> gameplay route
- fresh original diagnostic has duplicate raw 320x200 frames
- fresh original diagnostic has duplicate 224x136 viewport crops
- fresh original diagnostic produced no command/state/redraw transcript rows
- original rows do not bind map/X/Y/direction to F0380 -> F0365/F0366 -> F0128 -> F0097 for the same sampled frame

## Promotion requires

- exact original map/X/Y/direction/wall-door state for every sampled frame
- command id and queue delta from F0380 for the sampled command
- matching F0365/F0366 handler/state delta or source-visible blocked/no-op proof
- later F0128 tuple and F0097/VIDRV present boundary before screenshot acceptance
- a Firestaff fixture row with the same map/X/Y/direction/wall-door state and reproducible viewport hash

## Non-claims

- no original-vs-Firestaff pixel parity is claimed
- no ReDMCSB source-table gap is claimed
- no Firestaff renderer behavior is changed
- fresh original images are diagnostic-only and remain unpromoted

Manifest: parity-evidence/verification/pass608_dm1_v1_same_viewport_capture_blocker/manifest.json
