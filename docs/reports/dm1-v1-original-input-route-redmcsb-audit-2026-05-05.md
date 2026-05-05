# DM1 V1 original input route ReDMCSB audit (2026-05-05)

Scope: ReDMCSB source audit only, from `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`, to narrow the validated original PC34 route for movement, viewport clicks, and wall handling without emulator guessing.

## Route found

1. **Game view installs movement input tables.** `CHAMPION.C:1408-1412` sets primary mouse to `G0447_as_Graphic561_PrimaryMouseInput_Interface`, secondary mouse to `G0448_as_Graphic561_SecondaryMouseInput_Movement`, primary keyboard to `G0458_as_Graphic561_PrimaryKeyboardInput_Interface`, secondary keyboard to `G0459_as_Graphic561_SecondaryKeyboardInput_Movement`, then calls `F0357_COMMAND_DiscardAllInput()`.
2. **Movement mouse zones are explicit.** `COMMAND.C:396-405` maps turn/move commands and viewport clicks through `G0448_as_Graphic561_SecondaryMouseInput_Movement`: `C001_COMMAND_TURN_LEFT`, `C003_COMMAND_MOVE_FORWARD`, `C002_COMMAND_TURN_RIGHT`, `C006_COMMAND_MOVE_LEFT`, `C005_COMMAND_MOVE_BACKWARD`, `C004_COMMAND_MOVE_RIGHT`, `C080_COMMAND_CLICK_IN_DUNGEON_VIEW`, and right-click inventory toggle.
3. **PC34 keyboard movement mapping is explicit.** In `G0459_as_Graphic561_SecondaryKeyboardInput_Movement`, `COMMAND.C:677-684` under `MEDIA707_I34E_I34M` maps `0x004B/0x004C/0x004D/0x004F/0x0050/0x0051` to turn-left, forward, turn-right, move-left, backward, move-right respectively. Constants are defined in `DEFS.H:238-243`; viewport click is `DEFS.H:305`.
4. **Input events enqueue commands.** `F0359_COMMAND_ProcessClick_CPSC` calls `F0358_COMMAND_GetCommandFromMouseInput_CPSC` against primary then secondary mouse tables and queues matching command/X/Y at `COMMAND.C:1641-1660`; left-button-up can enqueue `C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL` at `COMMAND.C:1637-1639`.
5. **Main loop drains input and command queue.** `GAMELOOP.C:164-168` loops over keyboard buffer and calls `F0361_COMMAND_ProcessKeyPress(...)`; `GAMELOOP.C:215-219` calls `F0380_COMMAND_ProcessQueue_CPSC()` until `G0321_B_StopWaitingForPlayerInput`/time-tick conditions end the wait.
6. **Queue dispatch is the canonical route.** `F0380_COMMAND_ProcessQueue_CPSC` takes queued command/X/Y at `COMMAND.C:2095-2127`, blocks movement if `G0310_i_DisabledMovementTicks` or projectile-disabled movement applies at `COMMAND.C:2096`, dispatches turns to `F0365_COMMAND_ProcessTypes1To2_TurnParty` at `COMMAND.C:2150-2152`, moves to `F0366_COMMAND_ProcessTypes3To6_MoveParty` at `COMMAND.C:2154-2156`, and viewport clicks to `F0377_COMMAND_ProcessType80_ClickInDungeonView` at `COMMAND.C:2322-2324`.

## Movement and wall evidence

- `F0365_COMMAND_ProcessTypes1To2_TurnParty` sets `G0321_B_StopWaitingForPlayerInput = C1_TRUE`, highlights the turn box, handles stairs, processes party removal/addition sensors, and updates direction via `F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(...))` at `CLIKMENU.C:156-173`.
- `F0366_COMMAND_ProcessTypes3To6_MoveParty` derives movement arrow index from command at `CLIKMENU.C:256`, computes destination using `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(...)` at `CLIKMENU.C:269`, reads square type at `CLIKMENU.C:270`, and treats walls/closed doors/closed non-imaginary fakewalls as movement blockers at `CLIKMENU.C:278-288`.
- Blocked movement discards input, waits a vertical blank for later media, clears `G0321_B_StopWaitingForPlayerInput`, and returns at `CLIKMENU.C:317-323`.
- Allowed movement calls `F0267_MOVE_GetMoveResult_CPSCE(...)` for source/destination occupancy/sensor side effects at `CLIKMENU.C:325-329`, sets `G0310_i_DisabledMovementTicks` from champion movement ticks, and clears projectile movement delay at `CLIKMENU.C:330-347`.
- The coordinate primitive is `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` at `DUNGEON.C:1371-1391`; relative-square helper `F0152_DUNGEON_GetRelativeSquare` calls it and returns `F0151_DUNGEON_GetSquare(...)` at `DUNGEON.C:1481-1492`.
- Projectile movement throttling is real route state: shooting sets `G0311_i_ProjectileDisabledMovementTicks = 4` and `G0312_i_LastProjectileDisabledMovementDirection = L0990_ui_Direction` at `CHAMPION.C:2068-2069`; throwing sets the same tick count and party direction at `CHAMPION.C:2190-2191`. The queue checks these at `COMMAND.C:2096`.

## Viewport click and wall evidence

- `F0377_COMMAND_ProcessType80_ClickInDungeonView` normalizes click coordinates by viewport origin for later media at `CLIKVIEW.C:347-350`, computes the front square from `G0306_i_PartyMapX/G0307_i_PartyMapY` plus direction vectors at `CLIKVIEW.C:351-355`, and branches by `G0285_i_SquareAheadElement`.
- Door-button/wall-ornament zone hits trigger door toggle events or wall sensors: door-button path uses `F0268_SENSOR_AddEvent(...)` at `CLIKVIEW.C:378-389`; wall ornament/front sensor path calls `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor()` at `CLIKVIEW.C:424-432` and again after fountain/alcove handling at `CLIKVIEW.C:477-497`.
- Empty-hand object pickup and leader-hand drop route through viewport cells: grab at `CLIKVIEW.C:406-438`, drop to piles at `CLIKVIEW.C:449-466` and `CLIKVIEW.C:499-510`.
- Front wall knocking is routed only when square ahead is wall and click is in the front wall zones, then `F0664_COMMAND_ProcessType80_ClickInDungeonView_KnockOnFrontWall(...)` is called at `CLIKVIEW.C:443-446`.
- Closed imaginary fakewall state is tracked from `F0151_DUNGEON_GetSquare(...)` and fakewall flags at `CLIKVIEW.C:403-405`; release/stop pressing is routed through `C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL` at `COMMAND.C:2129-2147` and the main loop guards stale closed-imaginary-wall pressing state at `GAMELOOP.C:188-213`.


## Coordinate, draw, and presentation seam evidence

- **COORD viewport origin/extent for PC34-route clicks.** `COORD.C:1693-1699` defines the viewport origin (`G2067_i_ViewportScreenX = 0`; PC/PC34 route `G2068_i_ViewportScreenY = 33`), while `COORD.C:1713-1722` fixes the 320x200 screen and 224x136 viewport dimensions used by input normalization and presentation.
- **Game loop redraws from mutated party state.** `GAMELOOP.C:80-90` redraws the dungeon view via `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)`; `GAMELOOP.C:164-168` drains keyboard input before command processing; `GAMELOOP.C:215-219` drains `F0380_COMMAND_ProcessQueue_CPSC()` until the source wait-release condition is reached.
- **DUNVIEW viewport base is rebuilt before wall comparison.** `DUNVIEW.C:2959` includes `DRAWVIEW.C` into the dungeon-view compilation unit; `DUNVIEW.C:2962-3003` clears/copies the 224x136 viewport floor/ceiling base and sets bitmap width/height before wall and object composition.
- **DUNVIEW wall/door evidence is composited into the same viewport target.** `DUNVIEW.C:3048-3092` blits wall-set and door bitmaps into `G0296_puc_Bitmap_Viewport`; therefore original wall evidence must be a post-composition viewport crop, not an isolated asset or emulator-level guess.
- **DUNVIEW clickable wall ornament zones feed the viewport-click route.** In the wall ornament/inscription path, `DUNVIEW.C:3897-3902` applies the PC34 `MEDIA707_I34E_I34M` zone shift for unreadable inscriptions, and `DUNVIEW.C:3921-3924` draws via `F0791_DUNGEONVIEW_DrawBitmapXX(...)` then copies the front-wall clickable zone into `G2210_aai_XYZ_DungeonViewClickable[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT]`.
- **DRAWVIEW presentation seam.** `DRAWVIEW.C:709-724` requests viewport presentation and waits for vblank on classic routes; the PC34 route resolves `C007_ZONE_VIEWPORT` with `F0638_GetZone(...)` and calls the video driver blit at `DRAWVIEW.C:849-858`. Captures are only comparable after this seam.

## Focused N2 gate results from this pass

Passed source gates:

- `tools/verify_dm1_v1_input_command_queue_source_lock.py`
- `tools/verify_dm1_v1_movement_command_gate_source_lock.py`
- `tools/verify_dm1_v1_movement_source_lock.py`
- `scripts/verify_dm1_v1_viewport_world_redmcsb_gate.py`
- `tools/verify_v1_viewport_redmcsb_draw_stack_gate.py`

Focused blocker gate:

- `tools/pass207_dm1_v1_original_movement_viewport_blocker_gate.py` returned `BLOCKED_MOVEMENT_VIEWPORT_ROUTE_NOT_PROMOTABLE` with `mismatch_count=4`, report `parity-evidence/pass207_dm1_v1_original_movement_viewport_blocker_gate.md`, manifest `parity-evidence/verification/pass207_dm1_v1_original_movement_viewport_blocker_gate/manifest.json`.
- The blocker is semantic capture alignment, not source-route ambiguity: the current pass206/pass207 original-runner attempt repeats `wall_closeup` frames where expected post-command gameplay/spell/inventory states should appear, so it cannot be promoted as original-faithful movement/viewport evidence.

## What this narrows/unblocks

- For Firestaff PC34 movement validation, the route should be modeled as **input table -> command queue -> `F0380_COMMAND_ProcessQueue_CPSC` -> `F0365`/`F0366`/`F0377`**, not as direct SDL/emulator key movement.
- The PC34 keyboard codes for movement are the `MEDIA707_I34E_I34M` entries in `COMMAND.C:677-684`.
- The canonical movement-blocking wall logic is `CLIKMENU.C:278-288`, with blocked-return behavior at `CLIKMENU.C:317-323` and movement delay state at `CLIKMENU.C:345-347`.
- The canonical viewport/wall click route is `C080_COMMAND_CLICK_IN_DUNGEON_VIEW` dispatch at `COMMAND.C:2322-2324` into `CLIKVIEW.C:311+`, especially front wall sensor/knock evidence at `CLIKVIEW.C:431`, `CLIKVIEW.C:443-446`, and `CLIKVIEW.C:496`.

## Blockers / not proven here

- This pass did not prove raw BIOS/interrupt scancode capture for PC34 (`I34E/I34M`) from hardware to `F0361_COMMAND_ProcessKeyPress`; the ReDMCSB shared source clearly exposes the table and command route, but the platform-specific raw input source still needs a targeted IBM/NEC/PC34 audit if byte-perfect hardware capture is required.
- No emulator behavior was used; all claims above are source-line evidence only.
