# Pass327 — DM1 V1 non-tmux runtime evidence fallback

Status: `BLOCKED_PASS327_NON_TMUX_RAW_STREAM_NO_STRICT_CODE_STOP_PROMOTION`

## ReDMCSB source audit

- PASS `DRAWVIEW.C:709` — f0097_entry / `void F0097_DUNGEONVIEW_DrawViewport`
- PASS `DRAWVIEW.C:721` — f0097_sets_draw_request / `G0324_B_DrawViewportRequested = C1_TRUE`
- PASS `DRAWVIEW.C:842` — f0097_blits_to_screen / `F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport`
- PASS `DRAWVIEW.C:857` — f0097_vidrv_slot9_call / `(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);`
- PASS `DUNVIEW.C:8318` — f0128_entry / `void F0128_DUNGEONVIEW_Draw_CPSF`
- PASS `DUNVIEW.C:8367` — f0128_ceiling_viewport_bitmap / `F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);`
- PASS `DUNVIEW.C:8606` — f0128_calls_f0097_not_entrance / `F0097_DUNGEONVIEW_DrawViewport(G0309_i_PartyMapIndex != C255_MAP_INDEX_ENTRANCE);`
- PASS `DUNVIEW.C:8610` — f0128_calls_f0097_dungeon_view / `F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);`
- PASS `COMMAND.C:2045` — f0380_entry / `void F0380_COMMAND_ProcessQueue_CPSC`
- PASS `COMMAND.C:2095` — f0380_dequeues_command / `G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command`
- PASS `COMMAND.C:2155` — f0380_dispatches_move / `F0366_COMMAND_ProcessTypes3To6_MoveParty`
- PASS `CLIKMENU.C:180` — move_party_entry / `F0366_COMMAND_ProcessTypes3To6_MoveParty`
- PASS `MOVESENS.C:442` — move_result_commit_x / `G0306_i_PartyMapX = P0560_i_DestinationMapX`
- PASS `MOVESENS.C:556` — move_draw_while_falling / `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY)`

## Fallback path

- Uses `pexpect` raw PTY control of `dosbox-debug`; `tmuxRequired=false`.
- Uses strict pass321 parser rules: setup echoes/BPLIST do not count as runtime stops.
- Pre-arms source/public-symbol-locked F0128 plus movement BPM fallback addresses from pass322; arms F0097/VIDRV only after a real F0128 stop.

## Runtime result

- ran: `True`; method: `pexpect_raw_pty_no_tmux`; usedTmux: `False`
- direct hits: `{'f0128': False, 'f0097AfterF0128': False, 'movementOrF0380': False}`

## Decision

No runtime promotion unless strict F0128 → F0097/VIDRV → movement/F0380 evidence appears in order. This pass creates/verifies the non-tmux fallback path; current run remains blocked if the raw stream still lacks strict code-stop lines.
