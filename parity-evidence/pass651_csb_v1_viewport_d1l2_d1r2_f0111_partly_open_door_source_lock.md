# Pass651 CSB V1 D1L2/D1R2 F0111 partly-open door source lock

Status: passed

CSB V1 D1L2/D1R2 partly-open horizontal F0111 door dispatch is source-locked to ReDMCSB F0122/F0123 body calls, F0111 frame/zone/blit order, F0128 ordering, and F0127 follow-up.

## ReDMCSB And Lineage Evidence
- PASS redmcsb-f0111-partly-open-horizontal (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C:4218-4337)
  - line 4218: STATICFUNCTION void F0111_DUNGEONVIEW_DrawDoor(
  - line 4248: if (P0125_ui_DoorState != C0_DOOR_STATE_OPEN)
  - line 4308: P0125_ui_DoorState--;
  - line 4312: P0129_ps_DoorFrames->LeftHorizontal[P0125_ui_DoorState]
  - line 4313: P0129_ps_DoorFrames->RightHorizontal[P0125_ui_DoorState]
  - line 4318: P2084_i_ZoneIndex += P0125_ui_DoorState;
  - line 4322: P2084_i_ZoneIndex + C6_UNKNOWN
  - line 4323: F0654_Call_F0132_VIDEO_Blit
  - line 4325: P2084_i_ZoneIndex += (unsigned int16_t)(3 | MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR);
  - line 4334: F0791_DUNGEONVIEW_DrawBitmapXX
  - line 4334: C10_COLOR_FLESH
- PASS redmcsb-f0122-d1l-body-door-front (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C:7391-7557)
  - line 7391: STATICFUNCTION void F0122_DUNGEONVIEW_DrawSquareD1L(
  - line 7492: case C17_ELEMENT_DOOR_FRONT:
  - line 7494: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0214_ai_SquareAspect[M550_FIRST_THING], P0165_i_Direction, P0166_i_MapX, P0167_i_MapY, M607_VIEW_SQUARE_D1L, C0x0028_CELL_ORDER_DOORPASS1_BACKRIGHT);
  - line 7503: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2111_DoorFrameTopD1L, C732_ZONE_DOOR_FRAME_TOP_D1L);
  - line 7506: F0111_DUNGEONVIEW_DrawDoor(L0214_ai_SquareAspect[M557_DOOR_THING_INDEX], L0214_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M630_ZONE_DOOR_D1L);
  - line 7508: L0213_i_Order = C0x0039_CELL_ORDER_DOORPASS2_FRONTRIGHT;
  - line 7536: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0214_ai_SquareAspect[M550_FIRST_THING], P0165_i_Direction, P0166_i_MapX, P0167_i_MapY, M607_VIEW_SQUARE_D1L, L0213_i_Order);
- PASS redmcsb-f0123-d1r-body-door-front (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C:7559-7725)
  - line 7559: STATICFUNCTION void F0123_DUNGEONVIEW_DrawSquareD1R(
  - line 7660: case C17_ELEMENT_DOOR_FRONT:
  - line 7662: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0216_ai_SquareAspect[M550_FIRST_THING], P0168_i_Direction, P0169_i_MapX, P0170_i_MapY, M608_VIEW_SQUARE_D1R, C0x0018_CELL_ORDER_DOORPASS1_BACKLEFT);
  - line 7671: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2110_DoorFrameTopD1R, C734_ZONE_DOOR_FRAME_TOP_D1R);
  - line 7674: F0111_DUNGEONVIEW_DrawDoor(L0216_ai_SquareAspect[M557_DOOR_THING_INDEX], L0216_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M632_ZONE_DOOR_D1R);
  - line 7676: L0215_i_Order = C0x0049_CELL_ORDER_DOORPASS2_FRONTLEFT;
  - line 7704: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0216_ai_SquareAspect[M550_FIRST_THING], P0168_i_Direction, P0169_i_MapX, P0170_i_MapY, M608_VIEW_SQUARE_D1R, L0215_i_Order);
- PASS redmcsb-f0128-d1-dispatch-f0127-followup (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C:8524-8542)
  - line 8524: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, -1, &L0224_i_MapX, &L0225_i_MapY);
  - line 8525: F0122_DUNGEONVIEW_DrawSquareD1L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8528: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 1, &L0224_i_MapX, &L0225_i_MapY);
  - line 8529: F0123_DUNGEONVIEW_DrawSquareD1R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8533: F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8537: F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8541: F0126_DUNGEONVIEW_DrawSquareD0R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8542: F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);
- PASS redmcsb-f0127-object-pass-boundary (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C:8288-8296)
  - line 8292: F0112_DUNGEONVIEW_DrawCeilingPit(C069_GRAPHIC_CEILING_PIT_D0C, C871_ZONE_CEILING_PIT_D0C
  - line 8294: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0222_ai_SquareAspect[M550_FIRST_THING], P0180_i_Direction, P0181_i_MapX, P0182_i_MapY, M609_VIEW_SQUARE_D0C, C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT);
- PASS redmcsb-defs-door-zone-baselines (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DEFS.H:2088-4260)
  - line 2088: #define C10_COLOR_FLESH           10
  - line 2600: #define M607_VIEW_SQUARE_D1L  4
  - line 2601: #define M608_VIEW_SQUARE_D1R  5
  - line 2605: #define C09_VIEW_SQUARE_D2L2  9
  - line 2606: #define C10_VIEW_SQUARE_D2R2 10
  - line 3508: #define C6_UNKNOWN
  - line 3516: #define MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR 0x4000
  - line 4047: #define C707_ZONE_WALL_D2L2
  - line 4048: #define C708_ZONE_WALL_D2R2
  - line 4053: #define C713_ZONE_WALL_D1L
  - line 4054: #define C714_ZONE_WALL_D1R
  - line 4245: #define M630_ZONE_DOOR_D1L
  - line 4247: #define M632_ZONE_DOOR_D1R
- PASS csb-lineage-f1-door-dispatch (/Users/bosse/.openclaw/data/firestaff-csbwin-source/CSBWin/Viewport.cpp:1903-1915)
  - line 1903: ui16 StdDrawF1DoorFacing[]
  - line 1908: StdDoorFacingFrameLeftBitmapF1
  - line 1909: StdDoorFacingFrameRightBitmapF1
  - line 1913: F1DoorRecordIndex, F1DoorState, StdDoorGraphicsF1, StdDoorRectsF1
  - line 1914: StdDrawDoor
  - line 1915: F1Contents,  F1xy, F1, DrawOrder349,  StdDrawRoomObjects

## Firestaff Evidence
- PASS header-public-include (include/csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.h:1-210)
  - line 56: f0128_dispatch_order
  - line 62: f0127_followup_order
  - line 76: left_horizontal_frame_bitmap
  - line 77: right_horizontal_frame_bitmap
  - line 94: CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34
  - line 126: csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34
- PASS module-spec-and-zone-math (src/csb/csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.c:1-380)
  - line 9: ReDMCSB: DUNVIEW.C F0122 lines 7391-7557.
  - line 11: CSB_F0128_D1L_ORDER = 13
  - line 12: CSB_F0128_D1R_ORDER = 14
  - line 16: CSB_F0127_ORDER = 18
  - line 21: CSB_ZONE_DOOR_D1L = 3780
  - line 22: CSB_ZONE_DOOR_D1R = 3800
  - line 41: DUNVIEW.C:4218-4337
  - line 89: D1L.LeftHorizontal
  - line 102: D1R.RightHorizontal
  - line 195: spec->door_zone_base + door_state + spec->first_half_dest_zone_offset
  - line 211: spec->door_zone_base + door_state +
  - line 212: spec->second_half_zone_offset | spec->second_half_zone_mask
- PASS test-asserts-pass651-surface (tests/test_csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.c:1-520)
  - line 100: test_f0128_dispatch_and_followup
  - line 170: test_f0111_state_frame_and_blit_math
  - line 209: frame.left.d1l2
  - line 228: d1l2.first.state2
  - line 247: d1r2.second.state2
  - line 317: probe.second_half
  - line 356: assertion_count_at_least_70
- PASS cmake-registration (CMakeLists.txt:80-2140)
  - line 97: src/csb/csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.c
  - line 2114: add_executable(test_csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat
  - line 2126: NAME csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat

## Non-Claims
- No renderer output or real-asset pixel parity claim.
- No game-data load or archive/materialization behavior change.
- No changes to the main CSB viewport module.
- No DM1, DM2, Nexus, or Theron behavior claim.

Manifest: parity-evidence/verification/pass651_csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_source_lock/manifest.json
