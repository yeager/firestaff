# Pass208 next debugger breakpoint seams

## command_accepted
- source: `COMMAND.C:2095-2127,2150-2156,2322-2324`
- symbols: `F0380_COMMAND_ProcessQueue_CPSC`, `F0365_COMMAND_ProcessTypes1To2_TurnParty`, `F0366_COMMAND_ProcessTypes3To6_MoveParty`, `F0377_COMMAND_ProcessType80_ClickInDungeonView`
- breakpoint intent: queued command is read from G0432_as_CommandQueue and before C001/C002/C003-C006/C080 dispatch

## movement_applied
- source: `CLIKMENU.C:256-270,317-329,345-347`
- symbols: `F0366_COMMAND_ProcessTypes3To6_MoveParty`, `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`, `F0267_MOVE_GetMoveResult_CPSCE`
- breakpoint intent: after destination X/Y are computed and before/after the allowed F0267_MOVE_GetMoveResult_CPSCE call

## party_coordinates_committed
- source: `MOVESENS.C:438-444,493-496,573-578`
- symbols: `F0267_MOVE_GetMoveResult_CPSCE`, `G0306_i_PartyMapX`, `G0307_i_PartyMapY`
- breakpoint intent: party thing commits destination coordinates, including teleporter/pit chained commits

## viewport_present
- source: `GAMELOOP.C:83-91,164-168,215-219`
- symbols: `F0128_DUNGEONVIEW_Draw_CPSF`, `F0380_COMMAND_ProcessQueue_CPSC`
- breakpoint intent: main loop redraws using the current party direction/X/Y after command processing cadence

## viewport_present_blit
- source: `DRAWVIEW.C:849-858`
- symbols: `F0097_DUNGEONVIEW_DrawViewport`, `VIDRV_09_BlitViewPort`
- breakpoint intent: PC34 route resolves C007_ZONE_VIEWPORT and blits G0296_puc_Bitmap_Viewport to the video driver

## viewport_buffer_composed
- source: `DUNVIEW.C:8608-8616`
- symbols: `F0097_DUNGEONVIEW_DrawViewport`, `F0098_DUNGEONVIEW_DrawFloorAndCeiling`
- breakpoint intent: DUNVIEW has requested viewport draw and precomputed the next floor/ceiling base

## Loader entry
- stock FIRES compressed loader entry: `1665:000e` relative to DOS load-image base (`PSP+0x10`).
- DOSBox expression once PSP is known: `bp (psp_segment + 0x10 + 0x1665):0x000e`.
