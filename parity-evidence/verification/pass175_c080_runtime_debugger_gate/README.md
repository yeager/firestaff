# Pass175 C080 runtime debugger/address gate

Classification: `blocked/address-map-required`
Exact remaining blocker: native gdb can open/probe only far enough to show the stock DOS DM.EXE has no ReDMCSB symbol binding; need a DOS real-mode/source-symbol bridge or address map before F0359/F0380/F0377/F0280 can be proven

## Source anchors
- PASS `COMMAND.C:1379-1435` `F0358_COMMAND_GetCommandFromMouseInput_CPSC` — F0358 scans MOUSE_INPUT rectangles/zones and returns the matching command for the click/button state.
- PASS `COMMAND.C:1452-1662` `F0359_COMMAND_ProcessClick_CPSC` — F0359 is the original click queue writer: records pending clicks, resolves primary/secondary mouse inputs via F0358, then queues command/X/Y in G0432_as_CommandQueue.
- PASS `COMMAND.C:2045-2127,2322-2324` `F0380_COMMAND_ProcessQueue_CPSC` — F0380 locks/dequeues command/X/Y from G0432_as_CommandQueue, processes pending clicks, then dispatches C080 to F0377.
- PASS `CLIKVIEW.C:311-350,406-439` `F0377_COMMAND_ProcessType80_ClickInDungeonView` — F0377 is the C080 handler; PC builds subtract viewport origin and empty-hand C05 wall-ornament hits call F0372.
- PASS `DUNVIEW.C:3913-3930,4210-4215,8343-8349` `C05 clickable portrait/door-button storage` — DUNVIEW stores front-wall champion portrait and D1C door-button clickability in C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT; startup clears the clickable storage.
- PASS `MOVESENS.C:1392,1501-1503` `C127_SENSOR_WALL_CHAMPION_PORTRAIT` — MOVESENS identifies wall champion portrait sensors and calls F0280 with the sensor data/portrait index.
- PASS `REVIVE.C:63-150,260-275,600-625` `F0280_CHAMPION_AddCandidateChampionToParty` — F0280 is the candidate transition reached from C127; it checks empty hand/party count and sets up the candidate champion path.

## Breakpoint gates
- `F0359_COMMAND_ProcessClick_CPSC` (COMMAND.C:1452-1662): for screen click (111,82), P0725/P0726=111/82, L1109_i_Command=80, queue write G0432_as_CommandQueue[last]={Command:80,X:111,Y:82}
- `F0380_COMMAND_ProcessQueue_CPSC` (COMMAND.C:2045-2127): L1160_i_Command=80 and L1161/L1162=111/82 after reading G0432_as_CommandQueue[first]
- `F0377_COMMAND_ProcessType80_ClickInDungeonView` (COMMAND.C:2322-2324 + CLIKVIEW.C:311-439): F0377 entered, PC viewport origin subtracted, normalized point hits C05 front-wall ornament/portrait clickable cell
- `F0280_CHAMPION_AddCandidateChampionToParty` (DUNVIEW.C:3913-3930 + MOVESENS.C:1501-1503 + REVIVE.C:63-150,260-275): front-wall C127_SENSOR_WALL_CHAMPION_PORTRAIT reaches F0280 and sets G0299_ui_CandidateChampionOrdinal

## Tool probes
- `dm_exe_file` rc=0: `/usr/bin/file <local-home>/Documents/Codex/2026-08-24/jobba-med-github-com-yeager-firestaff/work/firestaff-incomplete-20260824/.codex-scratch/dm1-pc34-stage/DungeonMasterPC34/DM.EXE`
  - <local-home>/Documents/Codex/2026-08-24/jobba-med-github-com-yeager-firestaff/work/firestaff-incomplete-20260824/.codex-scratch/dm1-pc34-stage/DungeonMasterPC34/DM.EXE: MS-DOS executable, MZ for MS-DOS, LZEXE v0.91 compressed
- `gdb_version` rc=0: `/usr/bin/gdb --version`
  - GNU gdb (Ubuntu 17.1-2ubuntu1) 17.1
  - Copyright (C) 2025 Free Software Foundation, Inc.
  - License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
  - This is free software: you are free to change and redistribute it.
  - There is NO WARRANTY, to the extent permitted by law.
- `gdb_stock_dm_symbol_gate` rc=1: `/usr/bin/gdb --batch -x <local-home>/Documents/Codex/2026-08-24/jobba-med-github-com-yeager-firestaff/work/firestaff-incomplete-20260824/parity-evidence/verification/pass175_c080_runtime_debugger_gate/stock_dm_symbol_gate.gdb`
  - <local-home>/Documents/Codex/2026-08-24/jobba-med-github-com-yeager-firestaff/work/firestaff-incomplete-20260824/parity-evidence/verification/pass175_c080_runtime_debugger_gate/stock_dm_symbol_gate.gdb:3: Error in sourced command file:
  - "<local-home>/Documents/Codex/2026-08-24/jobba-med-github-com-yeager-firestaff/work/firestaff-incomplete-20260824/.codex-scratch/dm1-pc34-stage/DungeonMasterPC34/DM.EXE": not in executable format: file format not recognized
- `dosbox_debug_version`: unavailable
- `dosbox_x_version` rc=1: `/usr/bin/dosbox-x -version`
  - DOSBox-X version 2026.01.02 SDL2, copyright 2011-2026 The DOSBox-X Team.
  - DOSBox-X project maintainer: joncampbell123 (The Great Codeholio)
  - DOSBox-X comes with ABSOLUTELY NO WARRANTY.  This is free software,
  - and you are welcome to redistribute it under certain conditions;
  - please read the COPYING file thoroughly before doing so.

## Non-claims
- does not prove original runtime reached F0359/F0380/F0377/F0280
- does not retire pass175 on visual no-delta
- does not try alternate coordinates
- does not use <private-host> or non-N2 references
