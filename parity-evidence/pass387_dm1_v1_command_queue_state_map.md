# pass387 DM1 V1 command queue state map

Date: 2026-05-08
Branch: `pass387-source-queue-state-map`
Scope: source-locked evidence map only; no runtime or parity code changes.
Primary source: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

## ReDMCSB source audit anchors

- `DEFS.H:229-233` defines the queued `COMMAND` payload as `{ X, Y, Command }`.
- `DEFS.H:235-244` defines movement command values: `C001_COMMAND_TURN_LEFT`, `C002_COMMAND_TURN_RIGHT`, `C003_COMMAND_MOVE_FORWARD`, `C004_COMMAND_MOVE_RIGHT`, `C005_COMMAND_MOVE_BACKWARD`, `C006_COMMAND_MOVE_LEFT`; `CM1_COMMAND_NONE` is `-1` and `C000_COMMAND_NONE` is zero.
- `DEFS.H:213-218` defines mouse button/status masks used by input tables, including right, left, left-up, right-up, and bonus-dungeon bits.
- `DEFS.H:3260-3265` sets `M529_COMMAND_QUEUE_SIZE`: 4 for early media and 8 for `MEDIA728_I34E_I34M_A36M_A35E_A35M`. DM1 PC/I34E therefore uses the 8-sized ring plus one storage slot in `G0432_as_CommandQueue[M529_COMMAND_QUEUE_SIZE + 1]`.
- `COMMAND.C:6-16` declares the ring state: `G0432_as_CommandQueue`, `G2153_i_QueuedCommandsCount`, `G0433_i_CommandQueueFirstIndex`, `G0434_i_CommandQueueLastIndex`, `G0435_B_CommandQueueLocked`, pending-click coordinates/buttons (`G0437_i_PendingClickX`, `G0438_i_PendingClickY`, `G0439_i_PendingClickButtonsStatus`), and `G0436_B_PendingClickPresent`.
- `STARTUP2.C:1179-1182` installs the normal in-game producers: primary mouse interface table, secondary mouse movement table, primary keyboard interface table, and secondary keyboard movement table.
- `COMMAND.C:375-405` maps primary interface mouse commands first, then secondary movement/viewport/inventory-leader mouse commands; movement mouse table rows use command values `C001` through `C006`, `C080_COMMAND_CLICK_IN_DUNGEON_VIEW`, and `C083_COMMAND_TOGGLE_INVENTORY_LEADER`.
- `COMMAND.C:636-644` maps secondary keyboard movement commands to the same `C001` through `C006` movement command values.
- `COMMAND.C:1379-1449` source-locks mouse table resolution: `F0358_COMMAND_GetCommandFromMouseInput_CPSC` walks the active table in order, checks zone/box containment plus button mask, and returns first matching command or `C000_COMMAND_NONE`.
- `COMMAND.C:1631-1662` source-locks click enqueue: left-up becomes `C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL`; leave-champion-region becomes `C129_COMMAND_RELEASE_CHAMPION_ICON`; otherwise primary mouse is tried first and secondary only when primary returns none. A nonzero command increments `G2153_i_QueuedCommandsCount`, advances `G0434_i_CommandQueueLastIndex`, and stores command plus original click `X/Y` in `G0432_as_CommandQueue`.
- `COMMAND.C:1692-1707` source-locks pending-click replay: after queue unlock, `F0360_COMMAND_ProcessPendingClick` replays one pending click by calling `F0359_COMMAND_ProcessClick_CPSC(G0437_i_PendingClickX, G0438_i_PendingClickY, G0439_i_PendingClickButtonsStatus)`.

## F0380 dequeue and eligibility state

- `COMMAND.C:2045-2078` enters `F0380_COMMAND_ProcessQueue_CPSC`, locks `G0435_B_CommandQueueLocked`, and computes the next first index from `G0434_i_CommandQueueLastIndex + 1` modulo `M529_COMMAND_QUEUE_SIZE + 1`.
- `COMMAND.C:2083-2094` treats `G2153_i_QueuedCommandsCount == 0` as empty for the PC/I34E media path, unlocks, optionally processes a pending click, then exits the queue pass.
- `COMMAND.C:2095-2101` reads the candidate command at `G0433_i_CommandQueueFirstIndex` but does not pop it if movement is temporarily disabled by `G0310_i_DisabledMovementTicks` or matching projectile movement lock state; the pass unlocks and exits instead.
- `COMMAND.C:2118-2127` is the actual pop: captures queued `X/Y`, decrements `G2153_i_QueuedCommandsCount`, increments/wraps `G0433_i_CommandQueueFirstIndex`, unlocks, and then processes a pending click.
- `COMMAND.C:2128-2148` handles the special stop-pressing command `C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL` after the pop, updating eye/mouth/wall press state and returning without normal command dispatch.

## F0365/F0366 selection predicates

- `COMMAND.C:2150-2152` dispatches exactly `C001_COMMAND_TURN_LEFT` or `C002_COMMAND_TURN_RIGHT` to `F0365_COMMAND_ProcessTypes1To2_TurnParty`.
- `COMMAND.C:2154-2156` dispatches the inclusive range `C003_COMMAND_MOVE_FORWARD` through `C006_COMMAND_MOVE_LEFT` to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
- `CLIKMENU.C:142-174` shows `F0365` sets `G0321_B_StopWaitingForPlayerInput = C1_TRUE`, highlights the relevant turn arrow, processes stairs as a special case, and otherwise updates direction through sensor departure/arrival notifications.
- `CLIKMENU.C:180-260` and `CLIKMENU.C:317-323` show `F0366` sets `G0321_B_StopWaitingForPlayerInput = C1_TRUE`, highlights the movement arrow (`C070_ZONE_MOVE_FORWARD + movement index` for the PC/I34E media path), decrements stamina, and sets it back to `C0_FALSE` only when movement is blocked.

## StopWaiting and viewport redraw relationship

- `GAMELOOP.C:164-219` clears `G0321_B_StopWaitingForPlayerInput`, drains keyboard input, runs press-release cleanup, calls `F0380_COMMAND_ProcessQueue_CPSC`, disables highlight if no stop-waiting command fired, and repeats until both `G0321_B_StopWaitingForPlayerInput` and game-time ticking are true.
- `DUNVIEW.C:8604-8611` is the source-locked dungeon-view redraw/presentation call after the dungeon-view draw path: `F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)` when the party is not at the entrance for the I34E media path.
- `COMMAND.C:2340-2355` and `COMMAND.C:2383-2415` show non-movement command paths that explicitly draw rest/freeze viewport state via `F0097_DUNGEONVIEW_DrawViewport(C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME)`, then swap active input tables and discard queued input.
- Inference from the above anchors: `G0321_B_StopWaitingForPlayerInput` controls exit from the input-wait loop, not the draw itself. Turn/move commands set it when accepted; blocked movement clears it and remains in the wait loop. Viewport redraw is performed by downstream dungeon/rest/freeze draw paths, not by the generic queue pop alone.

## Runtime probe recommendation

Next runtime probe should be a narrow DM1 V1 debugger trace around one left turn, one forward move, and one blocked forward move:

1. Break/log at `F0359_COMMAND_ProcessClick_CPSC` after nonzero enqueue and record input `X/Y/buttons`, resolved command, `G0434_i_CommandQueueLastIndex`, `G0433_i_CommandQueueFirstIndex`, and `G2153_i_QueuedCommandsCount`.
2. Break/log at `F0380_COMMAND_ProcessQueue_CPSC` immediately before eligibility checks, immediately after the actual pop, and at the `F0365`/`F0366` dispatch predicates.
3. Watch `G0321_B_StopWaitingForPlayerInput` and the first downstream `F0097_DUNGEONVIEW_DrawViewport` call to prove accepted turn/move exits the wait loop while blocked movement leaves it false and defers redraw until a valid state-changing path.

This keeps runtime validation focused on queue state transitions and avoids re-probing unrelated renderer ordering.
