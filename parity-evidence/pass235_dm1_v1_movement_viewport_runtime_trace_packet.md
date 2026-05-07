# Pass235 — DM1 PC34 movement→viewport runtime trace packet

Status: `BLOCKED_RUNTIME_HITS_REQUIRED`.

This is the next trace packet after pass229/pass230/pass233/pass234. It does not promote any static source line or EXENEW offset; it only narrows the original-runtime hit list that must be captured before CS:IP/global addresses can enter the symbol map.

## Ordered runtime-hit chain

- PASS `command_accepted` — `COMMAND.C:2075-2156` / `F0380_COMMAND_ProcessQueue_CPSC`
  - runtime observable: code breakpoint after queue unlock or on turn/move dispatch, proving accepted command id and X/Y came from G0432_as_CommandQueue
  - line 2095: `L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;`
  - line 2126: `G0435_B_CommandQueueLocked = C0_FALSE;`
  - line 2151: `F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);`
  - line 2155: `F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);`
- PASS `turn_or_step_state_applied` — `CLIKMENU.C:156-328` / `F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty`
  - runtime observable: code breakpoint after direction write for turns or after legal step calls F0267, with party dir/source/destination context captured
  - line 172: `F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(G0308_i_PartyDirection + ((P0734_i_Command == C002_COMMAND_TURN_RIGHT) ? 1 : 3)));`
  - line 269: `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection, G0465_ai_Graphic561_MovementArrowToStepForwardCount[AL1118_ui_MovementArrowIndex], G0466_ai_Graphic561_MovementArrowToStepRightCount[AL1118_ui_MovementArrowIndex], &L1121_i_MapX, &L1122_i_MapY);`
  - line 278: `L1117_B_MovementBlocked = C0_FALSE;`
  - line 317: `if (L1117_B_MovementBlocked) {`
  - line 328: `F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);`
- PASS `party_coordinates_committed` — `MOVESENS.C:438-506` / `F0267_MOVE_GetMoveResult_CPSCE`
  - runtime observable: data watchpoints on resolved G0306/G0307 addresses, tied back to the MOVESENS write site CS:IP
  - line 442: `G0306_i_PartyMapX = P0560_i_DestinationMapX;`
  - line 443: `G0307_i_PartyMapY = P0561_i_DestinationMapY;`
  - line 493: `if (P0557_T_Thing == C0xFFFF_THING_PARTY) {`
  - line 494: `G0306_i_PartyMapX = P0560_i_DestinationMapX;`
  - line 495: `G0307_i_PartyMapY = P0561_i_DestinationMapY;`
- PASS `draw_uses_mutated_tuple` — `GAMELOOP.C:80-90` / `F0002_MAIN_GameLoop_CPSDF`
  - runtime observable: code breakpoint at the F0128 call proving the draw consumes the post-command G0308/G0306/G0307 tuple
  - line 84: `if (!G0300_B_PartyIsResting) {`
  - line 88: `if (!G0423_i_InventoryChampionOrdinal) {`
  - line 90: `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);`
- PASS `viewport_buffer_composed` — `DUNVIEW.C:8336-8611` / `F0128_DUNGEONVIEW_Draw_CPSF`
  - runtime observable: code breakpoint after viewport composition before F0097, with P0183/P0184/P0185 and G0296 pointer captured
  - line 8357: `if (G0076_B_UseFlippedWallAndFootprintsBitmaps = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001) {`
  - line 8367: `F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);`
  - line 8542: `F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);`
  - line 8610: `F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);`
- PASS `viewport_present` — `DRAWVIEW.C:849-857` / `F0097_DUNGEONVIEW_DrawViewport`
  - runtime observable: code breakpoint on the PC34 viewport blit call proving composed G0296 reaches C007_ZONE_VIEWPORT presentation
  - line 850: `F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);`
  - line 857: `(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);`

## Promotion guardrail

- symbol map: `<firestaff-repo>/data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json`
- runtime image fixture: `<firestaff-repo>/data/original_runtime/dm1_pc34_i34e_runtime_image.v1.json`
- promotable entries now: `[]`
- entries still requiring verified runtime hits: `['command_accepted', 'turn_or_step_state_applied', 'party_coordinates_committed', 'draw_uses_mutated_tuple', 'viewport_buffer_composed', 'viewport_present']`
- required formula: `runtime_cs = (PSP + 0x10) + map_segment`; `runtime_ip = map_offset`

## Blocker

Exact blocker: `blocked/runtime-base-and-symbol-map-unavailable` — Need FIRES.MAP/public symbols or another reproducible debugger binding from ReDMCSB source seams/globals to loaded original FIRES CS:IP/data addresses.

Required debugger evidence: a reproducible FIRES.MAP/public-symbol table or debugger-observed bindings for the six events above, including PSP/load segment, runtime CS:IP, and G0306/G0307/G0308/G0296 data addresses where applicable.
