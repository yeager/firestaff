set pagination off
set confirm off
file /home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DM.EXE
break F0359_COMMAND_ProcessClick_CPSC
break F0380_COMMAND_ProcessQueue_CPSC
break F0377_COMMAND_ProcessType80_ClickInDungeonView
break F0280_CHAMPION_AddCandidateChampionToParty
info files
info breakpoints
