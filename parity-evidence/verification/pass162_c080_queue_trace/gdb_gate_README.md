# Pass162 C080 gdb/debugger gate

Classification: `blocked/gdb-cannot-bind-stock-dos-exe-or-redmcsb-symbols`
First missing gate: debugger/source-symbol binding prerequisite; C080 mouse/queue/front-wall gates were not reached

## Exact commands run
- `gdb_version` rc=0: `/usr/bin/gdb --version`
  - GNU gdb (Ubuntu 15.1-1ubuntu1~24.04.1) 15.1
  - Copyright (C) 2024 Free Software Foundation, Inc.
  - License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
  - This is free software: you are free to change and redistribute it.
  - There is NO WARRANTY, to the extent permitted by law.
- `dosbox_x_version` rc=1: `/usr/bin/dosbox-x -version`
  - DOSBox-X version 2024.03.01 SDL2, copyright 2011-2024 The DOSBox-X Team.
  - DOSBox-X project maintainer: joncampbell123 (The Great Codeholio)
  - DOSBox-X comes with ABSOLUTELY NO WARRANTY.  This is free software,
  - and you are welcome to redistribute it under certain conditions;
  - please read the COPYING file thoroughly before doing so.
- `dm_exe_file` rc=0: `/usr/bin/file /home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DM.EXE`
  - /home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DM.EXE: MS-DOS executable, MZ for MS-DOS, LZEXE v0.91 compressed
- `gdb_stock_dm_symbol_gate` rc=1: `/usr/bin/gdb --batch -x /home/trv2/work/firestaff/parity-evidence/verification/pass162_c080_queue_trace/pass162_dm_exe_symbol_gate.gdb`
  - /home/trv2/work/firestaff/parity-evidence/verification/pass162_c080_queue_trace/pass162_dm_exe_symbol_gate.gdb:3: Error in sourced command file:
  - "/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DM.EXE": not in executable format: file format not recognized

## Runnable artifacts
- gdb: `gdb --batch -x /home/trv2/work/firestaff/parity-evidence/verification/pass162_c080_queue_trace/pass162_dm_exe_symbol_gate.gdb`
- DOSBox-X: `dosbox-x -conf /home/trv2/work/firestaff/parity-evidence/verification/pass162_c080_queue_trace/dosbox-x-pass162-runtime-gate.conf`

## Source citations audited
- PASS `COMMAND.C:1452-1662` `F0359_COMMAND_ProcessClick_CPSC` — Actual mouse-click queue writer: derives command from primary/secondary mouse tables and writes nonzero command plus X/Y into G0432_as_CommandQueue.
- PASS `COMMAND.C:2045-2127` `F0380_COMMAND_ProcessQueue_CPSC dequeue` — F0380 locks/dequeues command, X, Y from G0432_as_CommandQueue and unlocks before dispatch.
- PASS `COMMAND.C:2322-2324` `F0380 -> F0377 dispatch` — F0380 dispatches C080 to F0377 with dequeued X/Y.
- PASS `CLIKVIEW.C:311-350` `F0377_COMMAND_ProcessType80_ClickInDungeonView` — C080 handler; PC builds normalize screen coordinates by subtracting viewport origin before hit testing.
- PASS `CLIKVIEW.C:406-439` `F0377 empty-hand front-wall hit` — Empty-hand C05 door-button/wall-ornament hit calls F0372, otherwise object cells call grab/drop paths.
- PASS `CLIKVIEW.C:5-27` `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor` — F0372 computes the square in front of the party and invokes F0275 on the wall face opposite party direction.
- PASS `MOVESENS.C:1501-1503` `C127_SENSOR_WALL_CHAMPION_PORTRAIT -> F0280` — A clicked champion portrait wall sensor calls F0280 with sensorData/portrait index.
- PASS `REVIVE.C:63-88` `F0280_CHAMPION_AddCandidateChampionToParty` — Candidate champion entry point reached after C127 portrait sensor processing.

## Non-claims
- does not prove stock original binary reached C080/F0377/F0280
- does not classify mouse translation vs queue dequeue vs C080 dispatch vs F0280 because gdb could not bind symbols to the stock DOS executable
- does not do coordinate guessing

Next step: Use DOSBox-X built with/started in its debugger, a DOS real-mode gdbstub, or an address map from ReDMCSB symbols to the loaded DM.EXE image, then apply the emitted breakpoint order.
