# pass162 C080 queue trace

Purpose: stop portrait coordinate guessing and isolate whether pass162's x=111,y=82 source-locked portrait click reaches the original queue/dispatch path or blocks in mouse translation/hit-state before F0280.

Classification: `ready/probe-plan-emitted`

## Source-audited path
- PASS `DEFS.H:305` `C080_COMMAND_CLICK_IN_DUNGEON_VIEW` — C080 is command ordinal 80.
- PASS `DEFS.H:3752` `C007_ZONE_VIEWPORT` — C007 is the viewport zone used by the PC movement secondary mouse table.
- PASS `COMMAND.C:1-16` `G0432_as_CommandQueue` — Original command queue storage, first/last indices, queue lock, and pending-click fields.
- PASS `COMMAND.C:106-114` `C007 -> C080 mouse route` — PC secondary movement mouse input maps viewport left-click box 0..223,33..168 to C080.
- PASS `COMMAND.C:397-403` `C007 -> C080 zone route` — Zone-based movement table maps C007_ZONE_VIEWPORT left-click to C080.
- PASS `COMMAND.C:1452-1662` `F0359_COMMAND_ProcessClick_CPSC` — Actual mouse-click queue writer: derives command from primary/secondary mouse tables and writes nonzero command plus X/Y into G0432_as_CommandQueue.
- PASS `CLIKMENU.C:142-174` `F0365_COMMAND_ProcessTypes1To2_TurnParty` — Required audit symbol: in this ReDMCSB tree F0365 is turn-party handling, not the C080 mouse queue writer; it is dispatched only for C001/C002 in F0380.
- PASS `COMMAND.C:2045-2127` `F0380_COMMAND_ProcessQueue_CPSC dequeue` — F0380 locks/dequeues command, X, Y from G0432_as_CommandQueue and unlocks before dispatch.
- PASS `COMMAND.C:2150-2152` `F0380 -> F0365 dispatch` — F0380 dispatches only C001/C002 turn commands to F0365.
- PASS `COMMAND.C:2322-2324` `F0380 -> F0377 dispatch` — F0380 dispatches C080 to F0377 with dequeued X/Y.
- PASS `CLIKVIEW.C:311-350` `F0377_COMMAND_ProcessType80_ClickInDungeonView` — C080 handler; PC builds normalize screen coordinates by subtracting viewport origin before hit testing.
- PASS `CLIKVIEW.C:406-439` `F0377 empty-hand front-wall hit` — Empty-hand C05 door-button/wall-ornament hit calls F0372, otherwise object cells call grab/drop paths.
- PASS `CLIKVIEW.C:5-27` `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor` — F0372 computes the square in front of the party and invokes F0275 on the wall face opposite party direction.
- PASS `MOVESENS.C:1501-1503` `C127_SENSOR_WALL_CHAMPION_PORTRAIT -> F0280` — A clicked champion portrait wall sensor calls F0280 with sensorData/portrait index.
- PASS `REVIVE.C:63-88` `F0280_CHAMPION_AddCandidateChampionToParty` — Candidate champion entry point reached after C127 portrait sensor processing.

## Narrow probe gates
1. **mouse translation / queue write** — COMMAND.C:1452-1662 F0359_COMMAND_ProcessClick_CPSC (not F0365 in this tree); expect: after x=111,y=82 left click, P0725/P0726 are 111/82, L1109_i_Command == 80, G0432_as_CommandQueue[last].Command == 80 with X=111,Y=82; if missing: host/DOSBox mouse translation or active mouse-input table is blocking before the original queue
2. **queue dequeue** — COMMAND.C:2045-2127 F0380_COMMAND_ProcessQueue_CPSC; expect: L1160_i_Command == 80 and L1161/L1162 == 111/82; if missing: queue overwrite/drop/BUG0_73 collision or wrong timing before dispatch
3. **C080 dispatch / viewport normalization** — COMMAND.C:2322-2324 + CLIKVIEW.C:311-350; expect: F0377 is entered; normalized point remains inside C05 wall ornament/portrait hit zone for the source-locked front wall; if missing: C080 is not dispatched or screen-to-viewport translation is different than the visual click assumption
4. **front-wall sensor hit-state** — CLIKVIEW.C:406-439, CLIKVIEW.C:5-27, MOVESENS.C:1501-1503, REVIVE.C:63-88; expect: pose map0 x=1 y=3 dir=South touches front square x=1 y=4 opposite face and reaches F0280(sensorData=10); if missing: front-wall hit zone/state/sensor face is blocking after F0377 but before F0280

## Tool status
- dosbox: `/usr/bin/dosbox`
- dosbox-x: `/usr/bin/dosbox-x`
- gdb: `None`

## Non-claims
- does not prove the stock original binary reached C080/F0377/F0280
- does not use DANNESBURK
- does not claim x=111,y=82 is wrong; it narrows where to instrument before changing coordinates

Next step: Run the emitted gate list in DOSBox-X/debugger against the source-locked pass162 pose; classify first missing gate instead of trying more coordinates.
