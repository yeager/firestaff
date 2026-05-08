# Pass315 — DM1 V1 F0128 runtime-hit verifier

Status: `F0128_RUNTIME_HIT_VERIFIED_F0380_REMAINS_BLOCKED`

## Runtime result

- Verified narrow F0128 breakpoint hit: `True` at `23AD:40FE`.
- Debugger command control seen: `True`.
- Symbol-map entry `viewport_buffer_composed` promoted: `True`.
- Scope guard: F0380 dequeue and F0097 viewport present remain blocked/unpromoted.

## Source anchors

- GAMELOOP.C F0002_MAIN_GameLoop_CPSDF lines 55-95 ok=True
  - line 90: `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);`
- DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF lines 8318-8611 ok=True
  - line 8318: `void F0128_DUNGEONVIEW_Draw_CPSF`
  - line 8367: `G0296_puc_Bitmap_Viewport`
  - line 8610: `F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);`
- DRAWVIEW.C F0097_DUNGEONVIEW_DrawViewport lines 709-858 ok=True
  - line 709: `void F0097_DUNGEONVIEW_DrawViewport`
  - line 842: `C007_ZONE_VIEWPORT`
  - line 857: `VIDRV_09_BlitViewPort`
- COMMAND.C F0380_COMMAND_ProcessQueue_CPSC lines 2045-2156 ok=True
  - line 2045: `void F0380_COMMAND_ProcessQueue_CPSC`
  - line 2151: `F0365_COMMAND_ProcessTypes1To2_TurnParty`
  - line 2155: `F0366_COMMAND_ProcessTypes3To6_MoveParty`
- CLIKMENU.C F0365/F0366 movement dispatch lines 142-328 ok=True
  - line 142: `F0365_COMMAND_ProcessTypes1To2_TurnParty`
  - line 180: `F0366_COMMAND_ProcessTypes3To6_MoveParty`
  - line 272: `F0267_MOVE_GetMoveResult_CPSCE`
- CHAMPION.C F0284_CHAMPION_SetPartyDirection lines 117-130 ok=True
  - line 129: `G0308_i_PartyDirection = P0600_i_Direction;`
- MOVESENS.C F0267_MOVE_GetMoveResult_CPSCE lines 438-444 ok=True
  - line 442: `G0306_i_PartyMapX = P0560_i_DestinationMapX;`
  - line 443: `G0307_i_PartyMapY = P0561_i_DestinationMapY;`

Evidence manifest: `parity-evidence/verification/pass315_dm1_v1_f0128_runtime_hit_verifier/manifest.json`.
