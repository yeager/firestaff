# Pass 175 — original C080 queue breakpoint probe

Purpose: stop coordinate guessing and isolate whether the original DM1 PC runtime sees the source portrait click as `C080`, reaches `F0380`/`F0377`, and then reaches `F0280`.

## Verdict

- probe classification: **retired/firestaff-source-locked-c080-gate-passed-original-debugger-symbol-binding-blocked**
- exact next blocker: Original binary in-process route classification still needs a DOS real-mode/source-symbol bridge or address map; the stale Firestaff pass175 no-delta implementation blocker is retired by the source-locked M11 C080 gate.
- N2 debugger availability: `debugger-binary-present`

## Source-audited command path

- `COMMAND.C` 1-16 — validated: G0432_as_CommandQueue, first/last indices, queue lock, and pending-click globals are the original queue storage to watch.
- `COMMAND.C` 108-114, 397-403 — validated: The movement secondary mouse table maps the viewport/zone C007 left-click to C080_COMMAND_CLICK_IN_DUNGEON_VIEW.
- `COMMAND.C` 1458-1662 — validated: F0365 records pending click fields, resolves primary/secondary mouse input into L1109_i_Command, and writes nonzero commands plus X/Y into G0432_as_CommandQueue.
- `COMMAND.C` 2045-2127, 2322-2324 — validated: F0380 dequeues command/X/Y from G0432_as_CommandQueue, unlocks/processes pending clicks, then dispatches C080 to F0377_COMMAND_ProcessType80_ClickInDungeonView.
- `ENTRANCE.C` 850-883 — validated: F0441_STARTEND_ProcessEntrance discards input, waits on C099_MODE_WAITING_ON_ENTRANCE, processes keys, and calls F0380 each loop before loading the dungeon.
- `CLIKVIEW.C` 311, 347-349, 405-431 — validated: F0377 is the C080 handler; PC builds subtract the viewport origin, then empty-hand C05 wall-ornament hits call F0372 to touch the front-wall sensor.
- `MOVESENS.C` 1392, 1501-1502 — validated: A no-leader party may still trigger C127_SENSOR_WALL_CHAMPION_PORTRAIT; that case calls F0280_CHAMPION_AddCandidateChampionToParty.
- `REVIVE.C` 63-150 — validated: F0280_CHAMPION_AddCandidateChampionToParty is the semantic transition: it requires an empty hand and party count <4, then consumes G0305_ui_PartyChampionCount to build the candidate champion.

## Runtime evidence carried from pass162/pass174

- pass162 buckets: `{'blocked/portrait-c080-no-visible-delta': 2}`
- `source_gated_portrait_then_resurrect`: `blocked/portrait-c080-no-visible-delta` — gated gameplay reached, but source portrait click x=111/y=82 produced no visible candidate transition; blocker is now C007/C080 mouse delivery or front-wall hit-state mismatch before F0280
  - unique hashes: `['48ed3743ab6a', 'ceb0c2eec633']`
  - portrait deltas: `[{'from': 'after_gameplay_gate', 'to': 'click_111_82_4', 'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}, {'from': 'click_111_82_4', 'to': 'after_source_portrait_111_82', 'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}]`
  - choice deltas: `[{'from': 'after_source_portrait_111_82', 'to': 'click_130_115_6', 'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}, {'from': 'click_130_115_6', 'to': 'after_source_c160_resurrect', 'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}]`
- `source_gated_portrait_then_reincarnate`: `blocked/portrait-c080-no-visible-delta` — gated gameplay reached, but source portrait click x=111/y=82 produced no visible candidate transition; blocker is now C007/C080 mouse delivery or front-wall hit-state mismatch before F0280
  - unique hashes: `['48ed3743ab6a', 'ceb0c2eec633']`
  - portrait deltas: `[{'from': 'after_gameplay_gate', 'to': 'click_111_82_4', 'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}, {'from': 'click_111_82_4', 'to': 'after_source_portrait_111_82', 'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}]`
  - choice deltas: `[{'from': 'after_source_portrait_111_82', 'to': 'click_186_115_6', 'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}, {'from': 'click_186_115_6', 'to': 'after_source_c161_reincarnate', 'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}]`

## Breakpoint plan

- **mouse interrupt / enqueue candidate** — `F0359_COMMAND_ProcessClick_CPSC` (COMMAND.C:1452-1662)
  - condition: after click x=111,y=82, expect L1109_i_Command == C080 and queue write into G0432_as_CommandQueue with X=111,Y=82 (screen-relative PC coordinates).
  - log: `P0725_i_X, P0726_i_Y, P0727_i_ButtonsStatus, L1109_i_Command, G0433_i_CommandQueueFirstIndex, G0434_i_CommandQueueLastIndex`
- **queue dequeue** — `F0380_COMMAND_ProcessQueue_CPSC` (COMMAND.C:2045-2127)
  - condition: break before/after L1160/L1161/L1162 are read; expect L1160_i_Command == C080_COMMAND_CLICK_IN_DUNGEON_VIEW and L1161/L1162 == 111/82.
  - log: `G0432_as_CommandQueue, G0433_i_CommandQueueFirstIndex, G0434_i_CommandQueueLastIndex, L1160_i_Command, L1161_i_CommandX, L1162_i_CommandY`
- **C080 dispatch** — `F0377_COMMAND_ProcessType80_ClickInDungeonView` (COMMAND.C:2322-2324 + CLIKVIEW.C:311-431)
  - condition: break on function entry; PC-normalized viewport point should become x=111-G2067, y=82-G2068 and hit C05 front-wall ornament/portrait box.
  - log: `P0752_i_X, P0753_i_Y, G2067_i_ViewportScreenX, G2068_i_ViewportScreenY, AL1150_ui_ViewCell`
- **front-wall portrait sensor** — `F0280_CHAMPION_AddCandidateChampionToParty` (MOVESENS.C:1501-1502 + REVIVE.C:63-150)
  - condition: break on F0280; expect P0596_ui_ChampionPortraitIndex/sensorData == 10 and G0305_ui_PartyChampionCount to advance after candidate setup.
  - log: `P0596_ui_ChampionPortraitIndex, G0415_ui_LeaderEmptyHanded, G0305_ui_PartyChampionCount, G0299_ui_CandidateChampionOrdinal`

## Interpretation

The source path is internally consistent: viewport `C007` left-click should become `C080`, be queued in `G0432_as_CommandQueue`, dequeued by `F0380`, dispatched to `F0377`, touch the front-wall sensor, and call `F0280` for `C127_SENSOR_WALL_CHAMPION_PORTRAIT`. N2 now has `/usr/bin/dosbox-debug` and `/usr/bin/dosbox-x`, but the stock-original debugger route is still blocked at symbol/address binding (`blocked/gdb-cannot-bind-stock-dos-exe-or-redmcsb-symbols`). The stale Firestaff implementation blocker is retired by `run_firestaff_m11_game_view_probe.sh` passing 599/599 invariants, including `INV_GV_07I0` and `INV_GV_07I1` C080 front-door source-route coverage. See `retirement_addendum.md`.
