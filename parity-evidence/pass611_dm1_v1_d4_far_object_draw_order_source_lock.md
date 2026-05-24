# Pass611 DM1 V1 D4 far-object draw-order source lock

Status: passed

Claim: ReDMCSB F0128 draws D4L, D4R, and D4C as direct far-object F0115 passes with cell order 0x0001 before any D3/D2/D1/D0 wall helper runs; this is a deterministic source-lock gate only, not a pixel-parity promotion.

## Primary ReDMCSB Evidence
- PASS dunview-f0128-d4-before-d3 (DUNVIEW.C:8466-8482)
  - line 8468: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1
  - line 8469: M598_VIEW_SQUARE_D4L, C0x0001_CELL_ORDER_BACKLEFT);
  - line 8472: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, 1
  - line 8473: M599_VIEW_SQUARE_D4R, C0x0001_CELL_ORDER_BACKLEFT);
  - line 8476: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, 0
  - line 8477: M597_VIEW_SQUARE_D4C, C0x0001_CELL_ORDER_BACKLEFT);
  - line 8482: F0676_DrawD3L2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
- PASS dunview-f0115-cell-layer-contract (DUNVIEW.C:4547-4582)
  - line 4547: STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(
  - line 4562: If the first nibble is 0, then the function call is to draw objects in an alcove on a wall square.
  - line 4564: The remaining nibbles contain ordinals of square view cells to draw
  - line 4580: Draw only explosions at specified cell, except for Fluxcages
- PASS drawview-present-boundary (DRAWVIEW.C:709-858)
  - line 709: void F0097_DUNGEONVIEW_DrawViewport(
  - line 721: G0324_B_DrawViewportRequested = C1_TRUE
  - line 722: M526_WaitVerticalBlank();
  - line 847: F0638_GetZone(C007_ZONE_VIEWPORT, L2413_ai_Box);
  - line 857: (*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);
- PASS command-viewport-zone-and-queue (COMMAND.C:396-405)
  - line 396: G0448_as_Graphic561_SecondaryMouseInput_Movement
  - line 403: C080_COMMAND_CLICK_IN_DUNGEON_VIEW
  - line 403: C007_ZONE_VIEWPORT
- PASS command-queue-mutation-before-redraw (COMMAND.C:2045-2156)
  - line 2045: void F0380_COMMAND_ProcessQueue_CPSC(
  - line 2075: G0435_B_CommandQueueLocked = C1_TRUE;
  - line 2087: G0435_B_CommandQueueLocked = C0_FALSE;
  - line 2151: F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);
  - line 2155: F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);
- PASS movesens-fall-redraw-caveat (MOVESENS.C:316-356)
  - line 316: BOOLEAN F0267_MOVE_GetMoveResult_CPSCE(
  - line 318: P0558_i_SourceMapX
  - line 318: P0560_i_DestinationMapX
- PASS movesens-f0128-while-falling-blocker (MOVESENS.C:548-558)
  - line 550: F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF();
  - line 556: F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY);
  - line 556: BUG0_28
- PASS defs-pc34-d4-square-ids (DEFS.H:2595-2615)
  - line 2612: #define M597_VIEW_SQUARE_D4C 16
  - line 2613: #define M598_VIEW_SQUARE_D4L 17
  - line 2614: #define M599_VIEW_SQUARE_D4R 18
- PASS defs-viewport-zone-id (DEFS.H:3748-3756)
  - line 3752: #define C007_ZONE_VIEWPORT
- PASS coord-zone-clip-contract (COORD.C:2389-2410)
  - line 2389: #ifdef MEDIA720_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J
  - line 2394: M708_ZONE_WIDTH(P2130_pi_XYZ)
  - line 2402: M709_ZONE_HEIGHT(P2130_pi_XYZ)
  - line 2409: return NULL;

## Firestaff Evidence
- PASS far-object-struct (include/dm1_v1_viewport_3d_pc34_compat.h)
- PASS far-object-table (src/dm1/dm1_v1_viewport_3d_pc34_compat.c)
- PASS far-object-d4l (src/dm1/dm1_v1_viewport_3d_pc34_compat.c)
- PASS far-object-d4r (src/dm1/dm1_v1_viewport_3d_pc34_compat.c)
- PASS far-object-d4c (src/dm1/dm1_v1_viewport_3d_pc34_compat.c)
- PASS far-object-test (tests/test_dm1_v1_viewport_3d_pc34_compat.c)
- PASS far-object-evidence (src/dm1/dm1_v1_viewport_3d_pc34_compat.c)

## Verification
- /home/trv2/work/firestaff/builds/n2-build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
PASS source_evidence.d0c_thieves_eye_frame_occlusion == 1
PASS source_evidence.side_occlusion == 1
PASS source_evidence.defs_zones == 1
PASS source_evidence.wall_source_clip_gate == 1
PASS source_evidence.wall_empty_blit_gate == 1
PASS source_evidence.occlusion == 1
PASS source_evidence.command_dispatch == 1
PASS source_evidence.next_redraw == 1
PASS source_evidence.present_wait == 1
PASS source_evidence.same_viewport_mouse == 1
PASS source_evidence.same_viewport_turn == 1
PASS source_evidence.same_viewport_move == 1
PASS source_evidence.same_viewport_draw == 1
PASS source_evidence.same_viewport_present == 1
PASS source_evidence.same_viewport_assets == 1
PASS dm1_v1_viewport_3d_source_lock
~~~
- /usr/bin/python3 /home/trv2/work/firestaff/tools/verify_pass611_dm1_v1_d4_far_object_draw_order_source_lock.py --check-only: rc=0
~~~
PASS pass611 check-only
~~~

## Non-Claims
- No renderer pixel parity claim.
- No original PC34 same-run transcript/frame binding claim.
- No movement queue, viewport capture, audible C006, or item panel behavior changed.
- No CSB, DM2, or Nexus behavior claim.
