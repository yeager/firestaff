# Pass251 — DM1 V1 original FIRES runtime-hit binding audit

Status: `BLOCKED_RUNTIME_HIT_BINDING_PREREQUISITES_MISSING`

This pass starts from the ReDMCSB source seams, then checks whether N2-local candidate CS:IP rows and debugger evidence are sufficient to promote original FIRES runtime hits. They are not yet sufficient.

## ReDMCSB source audit

- `command_accepted` — `COMMAND.C:2045-2160` `F0380_COMMAND_ProcessQueue_CPSC`; refs: 2075-2081 queue lock/empty test, 2095-2096 command read from G0432/G0433, 2118-2126 X/Y read, first-index advance, unlock, 2151-2155 turn/move dispatch; watch targets: G0432_as_CommandQueue, G0433_i_CommandQueueFirstIndex, G0434_i_CommandQueueLastIndex, G0435_B_CommandQueueLocked, G0308_i_PartyDirection
- `turn_types_1_to_2` — `CLIKMENU.C:142-179` `F0365_COMMAND_ProcessTypes1To2_TurnParty`; refs: 167 current square read at G0306/G0307, 171-173 remove/add party sensor state around F0284_CHAMPION_SetPartyDirection; watch targets: G0306_i_PartyMapX, G0307_i_PartyMapY, G0308_i_PartyDirection
- `move_types_3_to_6` — `CLIKMENU.C:180-328` `F0366_COMMAND_ProcessTypes3To6_MoveParty`; refs: 264 source X/Y seeded from G0306/G0307, 269 relative movement computes destination, 272-274 stairs path writes G0306/G0307, 326-328 non-stairs path calls F0267 with source/destination; watch targets: G0306_i_PartyMapX, G0307_i_PartyMapY, G0308_i_PartyDirection
- `move_get_move_result` — `MOVESENS.C:316-560` `F0267_MOVE_GetMoveResult_CPSCE`; refs: 442-443 successful destination writes G0306/G0307, 494-506 teleporter destination rewrites X/Y/direction, 556 pit-fall redraw calls F0128 with direction/destination; watch targets: G0306_i_PartyMapX, G0307_i_PartyMapY, G0308_i_PartyDirection
- `viewport_game_loop_draw_call_site` — `GAMELOOP.C:35-95` `F0002_MAIN_GameLoop_CPSDF -> F0128_DUNGEONVIEW_Draw_CPSF`; refs: 61 level-change movement result uses current G0306/G0307, 78 music update uses current party tuple, 90 F0128 draw call consumes G0308/G0306/G0307; watch targets: G0306_i_PartyMapX, G0307_i_PartyMapY, G0308_i_PartyDirection, G0296_puc_Bitmap_Viewport
- `viewport_buffer_composed` — `DUNVIEW.C:8318-8610` `F0128_DUNGEONVIEW_Draw_CPSF`; refs: 8318 function entry, 8367/8422/8431/8434 viewport-buffer floor/ceiling writes/flips, 8468-8540 relative-square walk, 8606/8610 F0097 viewport request; watch targets: G0296_puc_Bitmap_Viewport, G0306_i_PartyMapX, G0307_i_PartyMapY, G0308_i_PartyDirection, G0324_B_DrawViewportRequested
- `viewport_present` — `DRAWVIEW.C:709-858` `F0097_DUNGEONVIEW_DrawViewport`; refs: 721 sets G0324_B_DrawViewportRequested, 842 C007 blit on one PC-family path, 857 VIDRV_09_BlitViewPort with G0296_puc_Bitmap_Viewport; watch targets: G0296_puc_Bitmap_Viewport, G0324_B_DrawViewportRequested

Global declarations:

- `G0296_puc_Bitmap_Viewport` — `TOWNSGLB.H:1371`
- `G0306_i_PartyMapX` — `TOWNSGLB.H:1382`
- `G0307_i_PartyMapY` — `TOWNSGLB.H:1383`
- `G0308_i_PartyDirection` — `TOWNSGLB.H:1381`
- `G0432_as_CommandQueue` — `TOWNSGLB.H:678`
- `G0433_i_CommandQueueFirstIndex` — `TOWNSGLB.H:679`
- `G0434_i_CommandQueueLastIndex` — `TOWNSGLB.H:680`
- `G0435_B_CommandQueueLocked` — `TOWNSGLB.H:681`

## Runtime candidate/binding status

- `command_accepted` candidate `22AF:06E9` — accepted by pass247 parser; classification `actual_loader_runtime_breakpoint_candidate_only_not_verified_hit`; blocker: Capture loaded FIRES PSP/load segment and debugger hit at this queue-dispatch prologue or branch after command dequeuing.
- `turn_types_1_to_2` candidate `1EA4:010D` — accepted by pass247 parser; classification `actual_loader_runtime_breakpoint_candidate_only_not_verified_hit`; blocker: Runtime hit on 1771:010d with command 1/2 and party-direction before/after values.
- `move_types_3_to_6` candidate `1EA4:01AA` — accepted by pass247 parser; classification `actual_loader_runtime_breakpoint_candidate_only_not_verified_hit`; blocker: Runtime hit on 1771:01aa with command 3..6 and source/destination party coordinates.
- `move_get_move_result` candidate `1859:0516` — accepted by pass247 parser; classification `actual_loader_runtime_breakpoint_candidate_only_not_verified_hit`; blocker: Runtime hit on 1126:0516 plus watchpoints proving G0306/G0307 writes for the party path.
- `viewport_game_loop_draw_call_site` candidate `2AFF:110E` — accepted by pass247 parser; classification `actual_loader_runtime_breakpoint_candidate_only_not_verified_hit`; blocker: Need FIRES.MAP or debugger stepping from F0128 through DUNVIEW helpers; current static evidence only identifies a viewport-adjacent loop cluster.

## Exact remaining missing binding

- Pass247 proves `/usr/bin/dosbox-debug` accepts the candidate `BP` commands in a PTY (`TERM=vt100`), but it used `DEBUG COMMAND.COM` as a debugger-loop target and is not an actual stock FIRES gameplay hit transcript.
- `data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json` still has no `verified_runtime_hit` entries and no numeric `global_addresses`.
- Required next evidence: one original `DM.EXE`/`FIRES` gameplay route transcript that hits the candidate CS:IP breakpoints and reads/watches the corresponding globals in route order.

## Artifacts

- `<firestaff-repo>/parity-evidence/verification/pass251_dm1_v1_original_fires_runtime_hit_binding_audit/manifest.json`
- `<firestaff-repo>/parity-evidence/pass251_dm1_v1_original_fires_runtime_hit_binding_audit.md`
