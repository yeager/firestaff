# Pass230 breakpoint promotion contract

Use `FIRES.EXENEW` only as the decompressed image/layout input. Promote no event until a map/debugger source binding supplies each CS:IP/global address.

- `command_accepted` — `COMMAND.C:2045-2156` `F0380_COMMAND_ProcessQueue_CPSC`: queue dequeue plus turn/step/click dispatch after accepted command id is read Breakpoint anchor: after COMMAND.C:2126 queue unlock or before COMMAND.C:2150 turn/move dispatch.
- `turn_or_step_state_applied` — `CLIKMENU.C:142-328` `F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty`: party direction or destination state after turn/step handler Breakpoint anchor: after CLIKMENU.C:173 for turns; after CLIKMENU.C:328 for steps.
- `party_coordinates_committed` — `MOVESENS.C:316-556` `F0267_MOVE_GetMoveResult_CPSCE`: party X/Y global writes for successful move result Breakpoint anchor: memory write watchpoint on G0306_i_PartyMapX/G0307_i_PartyMapY, source-anchored at MOVESENS.C:442-443.
- `draw_uses_mutated_tuple` — `GAMELOOP.C:35-91` `F0002_MAIN_GameLoop_CPSDF`: post-command draw call consumes direction/X/Y tuple Breakpoint anchor: GAMELOOP.C:90 call to F0128_DUNGEONVIEW_Draw_CPSF with the mutated direction/X/Y tuple.
- `viewport_buffer_composed` — `DUNVIEW.C:8318-8610` `F0128_DUNGEONVIEW_Draw_CPSF`: dungeon view path composes viewport buffer from current party tuple and requests present Breakpoint anchor: before DUNVIEW.C:8608-8611 after buffer composition and before F0097_DUNGEONVIEW_DrawViewport.
- `viewport_present` — `DRAWVIEW.C:709-858` `F0097_DUNGEONVIEW_DrawViewport`: C007 viewport-zone blit presents the composed viewport bitmap Breakpoint anchor: DRAWVIEW.C:857 PC-family VIDRV_09_BlitViewPort call with G0296_puc_Bitmap_Viewport.

Required formula: `runtime_cs = (PSP + 0x10) + map_segment`, `runtime_ip = map_offset`.
