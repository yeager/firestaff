# Pass279 — DM1 V1 evidence/capture source-lock follow-up

Status: `PASS_EVIDENCE_CAPTURE_SOURCE_LOCKED_F0380_RUNTIME_HIT_REMAINS`

## Source-lock result

- PASS `input_table_to_queue` — `COMMAND.C:1641-1812` `F0361_COMMAND_ProcessKeyPress / F0359_COMMAND_ProcessClick_CPSC`
  - line 1705: `F0359_COMMAND_ProcessClick_CPSC`
  - line 1709: `F0361_COMMAND_ProcessKeyPress`
  - line 1658: `G0432_as_CommandQueue`
  - line 1693: `F0360_COMMAND_ProcessPendingClick`
- PASS `queue_dequeue_dispatch` — `COMMAND.C:2045-2160` `F0380_COMMAND_ProcessQueue_CPSC`
  - line 2045: `void F0380_COMMAND_ProcessQueue_CPSC`
  - line 2095: `G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command`
  - line 2151: `F0365_COMMAND_ProcessTypes1To2_TurnParty`
  - line 2155: `F0366_COMMAND_ProcessTypes3To6_MoveParty`
- PASS `turn_mutates_direction` — `CLIKMENU.C:142-179` `F0365_COMMAND_ProcessTypes1To2_TurnParty`
  - line 142: `void F0365_COMMAND_ProcessTypes1To2_TurnParty`
  - line 156: `G0321_B_StopWaitingForPlayerInput = C1_TRUE`
  - line 172: `F0284_CHAMPION_SetPartyDirection`
- PASS `move_mutates_coordinates` — `CLIKMENU.C:180-347` `F0366_COMMAND_ProcessTypes3To6_MoveParty`
  - line 180: `void F0366_COMMAND_ProcessTypes3To6_MoveParty`
  - line 269: `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`
  - line 272: `F0267_MOVE_GetMoveResult_CPSCE`
  - line 345: `G0310_i_DisabledMovementTicks`
- PASS `game_loop_consumes_tuple` — `GAMELOOP.C:55-95` `F0002_MAIN_GameLoop_CPSDF`
  - line 69: `F0261_TIMELINE_Process_CPSEF();`
  - line 90: `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);`
- PASS `viewport_composes_buffer` — `DUNVIEW.C:8318-8611` `F0128_DUNGEONVIEW_Draw_CPSF`
  - line 8318: `void F0128_DUNGEONVIEW_Draw_CPSF`
  - line 8367: `G0296_puc_Bitmap_Viewport`
  - line 8542: `F0127_DUNGEONVIEW_DrawSquareD0C`
  - line 8610: `F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);`
- PASS `viewport_present_blit` — `DRAWVIEW.C:709-858` `F0097_DUNGEONVIEW_DrawViewport`
  - line 709: `void F0097_DUNGEONVIEW_DrawViewport`
  - line 721: `G0324_B_DrawViewportRequested = C1_TRUE`
  - line 847: `F0638_GetZone(C007_ZONE_VIEWPORT`
  - line 857: `VIDRV_09_BlitViewPort`

## Evidence state

- Original DOS capture route is unblocked by pass249: launch reaches live dungeon gameplay frames, controlled `kp5` changes viewport hashes, and 224x136 crops exist outside the repo.
- Public-symbol/source-map blocker is unblocked by pass273: FIRES.MAP/DM.MAP were found and map source functions/globals to runtime addresses without committing original binaries.
- Runtime hook proof remains blocked by pass278: controlled queue writes, party tuple mutation, and F0128 hit are present, but no proven F0380 dequeue BP hit at `22F4:0699`.

## Runtime addresses locked for the next attempt

- `F0380_COMMAND_ProcessQueue_CPSC` — `22F4:0699`
- `F0365_COMMAND_ProcessTypes1To2_TurnParty` — `1EA4:010D`
- `F0366_COMMAND_ProcessTypes3To6_MoveParty` — `1EA4:01AA`
- `F0267_MOVE_GetMoveResult_CPSCE` — `1859:0516`
- `F0128_DUNGEONVIEW_Draw_CPSF` — `23AD:40FE`
- `F0097_DUNGEONVIEW_DrawViewport` — `2809:1E31`

## Runtime globals locked for watches

- `G0432_as_CommandQueue` — `2C20:3E7A`
- `G0308_i_PartyDirection` — `2C20:3C92`
- `G0306_i_PartyMapX` — `2C20:3C94`
- `G0307_i_PartyMapY` — `2C20:3CE0`
- `G0296_puc_Bitmap_Viewport` — `2C20:3D2A`

## Exact remaining blocker

Need a stock DM1 PC34 dosbox-debug transcript proving a controlled key/click command is dequeued by `F0380_COMMAND_ProcessQueue_CPSC` at `22F4:0699`, then mutates `G0308` or `G0306/G0307`, and is consumed by `F0128_DUNGEONVIEW_Draw_CPSF` at `23AD:40FE`. Source route, public symbols, capture route, and F0128 hit are no longer the blocker.

Manifest: `<repo>/parity-evidence/verification/pass279_dm1_v1_evidence_capture_source_lock_followup/manifest.json`
