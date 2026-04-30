# Pass175 breakpoint checklist

## mouse interrupt / enqueue candidate
- symbol: `F0359_COMMAND_ProcessClick_CPSC`
- source: `COMMAND.C:1452-1662`
- condition: after click x=111,y=82, expect L1109_i_Command == C080 and queue write into G0432_as_CommandQueue with X=111,Y=82 (screen-relative PC coordinates).
- log: `P0725_i_X, P0726_i_Y, P0727_i_ButtonsStatus, L1109_i_Command, G0433_i_CommandQueueFirstIndex, G0434_i_CommandQueueLastIndex`

## queue dequeue
- symbol: `F0380_COMMAND_ProcessQueue_CPSC`
- source: `COMMAND.C:2045-2127`
- condition: break before/after L1160/L1161/L1162 are read; expect L1160_i_Command == C080_COMMAND_CLICK_IN_DUNGEON_VIEW and L1161/L1162 == 111/82.
- log: `G0432_as_CommandQueue, G0433_i_CommandQueueFirstIndex, G0434_i_CommandQueueLastIndex, L1160_i_Command, L1161_i_CommandX, L1162_i_CommandY`

## C080 dispatch
- symbol: `F0377_COMMAND_ProcessType80_ClickInDungeonView`
- source: `COMMAND.C:2322-2324 + CLIKVIEW.C:311-431`
- condition: break on function entry; PC-normalized viewport point should become x=111-G2067, y=82-G2068 and hit C05 front-wall ornament/portrait box.
- log: `P0752_i_X, P0753_i_Y, G2067_i_ViewportScreenX, G2068_i_ViewportScreenY, AL1150_ui_ViewCell`

## front-wall portrait sensor
- symbol: `F0280_CHAMPION_AddCandidateChampionToParty`
- source: `MOVESENS.C:1501-1502 + REVIVE.C:63-150`
- condition: break on F0280; expect P0596_ui_ChampionPortraitIndex/sensorData == 10 and G0305_ui_PartyChampionCount to advance after candidate setup.
- log: `P0596_ui_ChampionPortraitIndex, G0415_ui_LeaderEmptyHanded, G0305_ui_PartyChampionCount, G0299_ui_CandidateChampionOrdinal`
