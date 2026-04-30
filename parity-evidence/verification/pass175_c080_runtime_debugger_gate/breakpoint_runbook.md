# Pass175 C080 runtime debugger runbook

## enqueue
- symbol: `F0359_COMMAND_ProcessClick_CPSC`
- source: `COMMAND.C:1452-1662`
- expect: for screen click (111,82), P0725/P0726=111/82, L1109_i_Command=80, queue write G0432_as_CommandQueue[last]={Command:80,X:111,Y:82}
- if missing: host/emulator mouse translation or active mouse-input table blocks before original queue

## dequeue
- symbol: `F0380_COMMAND_ProcessQueue_CPSC`
- source: `COMMAND.C:2045-2127`
- expect: L1160_i_Command=80 and L1161/L1162=111/82 after reading G0432_as_CommandQueue[first]
- if missing: queued command is dropped/overwritten/timed differently before C080 dispatch

## C080 handler
- symbol: `F0377_COMMAND_ProcessType80_ClickInDungeonView`
- source: `COMMAND.C:2322-2324 + CLIKVIEW.C:311-439`
- expect: F0377 entered, PC viewport origin subtracted, normalized point hits C05 front-wall ornament/portrait clickable cell
- if missing: C080 dispatch or screen-to-viewport coordinate binding differs from source-locked assumption

## candidate transition
- symbol: `F0280_CHAMPION_AddCandidateChampionToParty`
- source: `DUNVIEW.C:3913-3930 + MOVESENS.C:1501-1503 + REVIVE.C:63-150,260-275`
- expect: front-wall C127_SENSOR_WALL_CHAMPION_PORTRAIT reaches F0280 and sets G0299_ui_CandidateChampionOrdinal
- if missing: front-wall C05 hit-state/sensor face/data blocks after F0377 but before candidate transition
