# Pass248 — DM1 V1 actual-loader FIRES symbol candidate map

Status: `PASS_ACTUAL_LOADER_RUNTIME_CANDIDATES_READY_NO_PROMOTIONS`

This pass binds the actual `DM.EXE` FIRES load segment from pass246 to pass237 static FIRES.EXENEW candidates. It promotes no runtime hits.

- Actual FIRES load segment: `0733`
- Formula: `runtime_cs = load_segment + static_cs`; `runtime_ip = static_ip`
- Numeric global addresses remain blocked pending debugger/map evidence; source-level watch targets are listed.

## Candidate breakpoints

- `command_accepted` -> `22AF:06E9` from static `1B7C:06E9`; COMMAND.C `F0380_COMMAND_ProcessQueue_CPSC`; citations: COMMAND.C:2075-2081, COMMAND.C:2095-2096, COMMAND.C:2118-2126, COMMAND.C:2151-2155
- `turn_types_1_to_2` -> `1EA4:010D` from static `1771:010D`; CLIKMENU.C `F0365_COMMAND_ProcessTypes1To2_TurnParty`; citations: CLIKMENU.C:167-173
- `move_types_3_to_6` -> `1EA4:01AA` from static `1771:01AA`; CLIKMENU.C `F0366_COMMAND_ProcessTypes3To6_MoveParty`; citations: CLIKMENU.C:264-269, CLIKMENU.C:272-275, CLIKMENU.C:326-328
- `move_get_move_result` -> `1859:0516` from static `1126:0516`; MOVESENS.C `F0267_MOVE_GetMoveResult_CPSCE`; citations: MOVESENS.C:432-443, MOVESENS.C:492-506, MOVESENS.C:556
- `viewport_game_loop_draw_call_site` -> `2AFF:110E` from static `23CC:110E`; GAMELOOP.C `F0002_MAIN_GameLoop_CPSDF -> F0128_DUNGEONVIEW_Draw_CPSF`; citations: GAMELOOP.C:58-63, GAMELOOP.C:78-90

## Source global declarations

- `G0296_puc_Bitmap_Viewport` — `TOWNSGLB.H:1371`
- `G0306_i_PartyMapX` — `TOWNSGLB.H:1382`
- `G0307_i_PartyMapY` — `TOWNSGLB.H:1383`
- `G0308_i_PartyDirection` — `TOWNSGLB.H:1381`
- `G0432_as_CommandQueue` — `TOWNSGLB.H:678`
- `G0433_i_CommandQueueFirstIndex` — `TOWNSGLB.H:679`
- `G0434_i_CommandQueueLastIndex` — `TOWNSGLB.H:680`
- `G0435_B_CommandQueueLocked` — `TOWNSGLB.H:681`

## Artifacts

- `<firestaff-repo>/data/original_runtime/dm1_pc34_i34e_actual_loader_symbol_candidates.v1.json`
- `<firestaff-repo>/parity-evidence/verification/pass248_dm1_v1_actual_loader_symbol_candidate_map/manifest.json`
