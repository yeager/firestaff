# Pass519 DM1 V1 D1C door-front field source lock

Status: passed

Claim: D1C door-front renders rear cells before frame/button/door, composes door ornaments/masks before final door blit, draws front cells after the door, then leaves teleporter field as a final overlay after F0115.

## Primary ReDMCSB Evidence

- PASS d1c-door-front-split (DUNVIEW.C:7873-7911)
  - line 7873: case C17_ELEMENT_DOOR_FRONT:
  - line 7874: F0108_DUNGEONVIEW_DrawFloorOrnament(L0218_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M595_VIEW_FLOOR_D1C);
  - line 7875: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT);
  - line 7886: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2112_DoorFrameTopD1LCR, C733_ZONE_DOOR_FRAME_TOP_D1C);
  - line 7887: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2117_DoorFrameLeftD1C, C726_ZONE_DOOR_FRAME_LEFT_D1C);
  - line 7893: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2117_DoorFrameLeftD1C, C727_ZONE_DOOR_FRAME_RIGHT_D1C);
  - line 7902: F0110_DUNGEONVIEW_DrawDoorButton(M000_INDEX_TO_ORDINAL(C0_DOOR_BUTTON), C3_VIEW_DOOR_BUTTON_D1C);
  - line 7908: F0111_DUNGEONVIEW_DrawDoor(L0218_ai_SquareAspect[M557_DOOR_THING_INDEX], L0218_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M631_ZONE_DOOR_D1C);
  - line 7910: L0217_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;
  - line 7911: goto T0124018;

- PASS d1c-front-cells-then-field (DUNVIEW.C:7936-7955)
  - line 7936: T0124018:
  - line 7937: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, L0217_i_Order);
  - line 7943: if (L0218_ai_SquareAspect[C0_ELEMENT] == C05_ELEMENT_TELEPORTER) {
  - line 7955: F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M606_VIEW_SQUARE_D1C]], C712_ZONE_WALL_D1C);

- PASS f0111-door-ornament-before-final-blit (DUNVIEW.C:4255-4334)
  - line 4255: L0116_ps_Door = (DOOR*)(G0284_apuc_ThingData[C00_THING_TYPE_DOOR]) + P0124_ui_DoorThingIndex;
  - line 4262: F0109_DUNGEONVIEW_DrawDoorOrnament(L0116_ps_Door->OrnamentOrdinal, P0128_i_ViewDoorOrnamentIndex);
  - line 4294: F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C16_DOOR_ORNAMENT_THIEVES_EYE_MASK), C2_VIEW_DOOR_ORNAMENT_D1LCR);
  - line 4301: if (P0125_ui_DoorState == C5_DOOR_STATE_DESTROYED) {
  - line 4302: F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C15_DOOR_ORNAMENT_DESTROYED_MASK), P0128_i_ViewDoorOrnamentIndex);
  - line 4334: F0791_DUNGEONVIEW_DrawBitmapXX(G0074_puc_Bitmap_Temporary, G0296_puc_Bitmap_Viewport, P2084_i_ZoneIndex, AL0114_ui_Flip, C10_COLOR_FLESH);

## Firestaff Evidence

- PASS firestaff-d1c-door-front-metadata (dm1_v1_viewport_3d_pc34_compat.c:240-244)

- PASS firestaff-d1c-runtime-test (test_dm1_v1_viewport_3d_pc34_compat.c:707-770)

## Verification

- /Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
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

- /opt/homebrew/opt/python@3.14/bin/python3.14 /Users/bosse/.openclaw/workspace-main/tools/verify_pass519_dm1_v1_d1c_door_front_field_source_lock.py --check-only: rc=0
~~~
PASS pass519 check-only
~~~

## Non-Claims

- No input or movement queue code was changed.
- No renderer runtime behavior was changed.
- No original DOS pixel parity is claimed.
- DANNESBURK was not used.
