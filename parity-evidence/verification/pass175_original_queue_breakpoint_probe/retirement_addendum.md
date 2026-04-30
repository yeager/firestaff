# Pass175 C080 blocker retirement addendum

## Classification

- `pass175_redmcsb_c080_no_delta_blocker_audit`: **retired / historical-only**.
- `pass175_original_queue_breakpoint_probe`: **narrowed**, not solved at stock-original-symbol level. N2 now has `/usr/bin/dosbox-debug` and `/usr/bin/dosbox-x`, but native `gdb` still cannot bind the stock DOS `DM.EXE` to ReDMCSB symbols without an address map or DOS real-mode bridge.

## Source-locked gate run

Command run on N2 from `/home/trv2/work/firestaff`:

```sh
./run_firestaff_m11_game_view_probe.sh /tmp/pass175-c080-gate-1777564645 ~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA
```

Result: `599/599 invariants passed`. Relevant C080 assertions from `/tmp/pass175-c080-gate.out`:

- `PASS INV_GV_07I0 V1 C080 front-door path ignores non-button viewport clicks instead of using procedural steering/toggle shortcuts`
- `PASS INV_GV_07I1 V1 C080 source D1C door-button zone x167..174/y43..51 toggles the front door through the door animation path`
- `PASS INV_GV_07I space toggles a closed front door into an animating step and updates the real dungeon square one state closer to OPEN`

## Debugger feasibility check

Command run:

```sh
python3 tools/pass162_c080_gdb_gate.py
```

Artifact: `parity-evidence/verification/pass162_c080_queue_trace/gdb_gate_manifest.json`.

Classification: `blocked/gdb-cannot-bind-stock-dos-exe-or-redmcsb-symbols`; first missing gate: `debugger/source-symbol binding prerequisite; C080 mouse/queue/front-wall gates were not reached`.

## ReDMCSB citations used

- `COMMAND.C:106-114` / `COMMAND.C:397-403`: PC and zone mouse tables map viewport left-click/C007 to `C080_COMMAND_CLICK_IN_DUNGEON_VIEW`.
- `COMMAND.C:1452-1662`: `F0359_COMMAND_ProcessClick_CPSC` is the actual mouse queue writer in this tree; requested `F0365` is turn-party handling, not C080 enqueue.
- `COMMAND.C:2045-2127` and `COMMAND.C:2322-2324`: `F0380_COMMAND_ProcessQueue_CPSC` dequeues command/X/Y and dispatches C080 to `F0377_COMMAND_ProcessType80_ClickInDungeonView`.
- `CLIKVIEW.C:311-350` and `CLIKVIEW.C:406-439`: `F0377` normalizes PC viewport coordinates and routes empty-hand C05 front-wall hits into `F0372`.
- `CLIKVIEW.C:5-27`, `MOVESENS.C:1501-1503`, `REVIVE.C:63-88`: front-wall portrait sensor (`C127_SENSOR_WALL_CHAMPION_PORTRAIT`) reaches `F0280_CHAMPION_AddCandidateChampionToParty`.

## State

Do not keep pass175 as a current Firestaff implementation blocker. The remaining original-runtime question is only a debugger/address-map problem: classify where the stock binary route lands among mouse translation, queue dequeue, C080 dispatch, F0377 hit-state, and F0280.
