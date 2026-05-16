# Pass516 DM1 V1 D1/D0 wall occlusion source lock

Status: passed

## Claim

ReDMCSB composes D1 before D0, then D0C last. D1L/D1R and D0L/D0R side-wall cases draw their side wall zone and return before open-cell content/field tails. D1C front wall only reveals contained things through the explicit front-alcove exception.

## Primary ReDMCSB Evidence

- PASS f0128-d1-then-d0-near-order (DUNVIEW.C:8524-8542)
  - F0128 draws D1L, D1R, D1C, then D0L, D0R, and finally D0C.
  - line 8525: F0122_DUNGEONVIEW_DrawSquareD1L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8529: F0123_DUNGEONVIEW_DrawSquareD1R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8533: F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8537: F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8541: F0126_DUNGEONVIEW_DrawSquareD0R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8542: F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);

- PASS f0122-d1l-side-wall-return (DUNVIEW.C:7436-7460)
  - D1L wall draws the D1L side zone, tests only the facing ornament, then returns before open-cell content.
  - line 7436: case C00_ELEMENT_WALL:
  - line 7446: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C02_WALL_D1R], C713_ZONE_WALL_D1L);
  - line 7454: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C03_WALL_D1L], C713_ZONE_WALL_D1L);
  - line 7459: F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0214_ai_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL], M585_VIEW_WALL_D1L_RIGHT);
  - line 7460: return;

- PASS f0123-d1r-side-wall-return (DUNVIEW.C:7604-7628)
  - D1R mirrors D1L: opposite bitmap when flipped, D1R zone, ornament probe, then return.
  - line 7604: case C00_ELEMENT_WALL:
  - line 7614: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C03_WALL_D1L], C714_ZONE_WALL_D1R);
  - line 7622: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C02_WALL_D1R], C714_ZONE_WALL_D1R);
  - line 7627: F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0216_ai_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL], M586_VIEW_WALL_D1R_LEFT);
  - line 7628: return;

- PASS f0124-d1c-front-wall-alcove-exception (DUNVIEW.C:7828-7844)
  - D1C front wall is opaque/centered and reveals contained content only for the explicit front-alcove exception.
  - line 7834: F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C, G0076_B_UseFlippedWallAndFootprintsBitmaps);
  - line 7840: F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C);
  - line 7842: if (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0218_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M587_VIEW_WALL_D1C_FRONT))
  - line 7843: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0000_CELL_ORDER_ALCOVE);

- PASS f0125-f0126-d0-side-walls-return-before-field (DUNVIEW.C:8007-8159)
  - D0 side-wall cases return before the side-lane teleporter-field tail; open D0 lanes use one back cell only.
  - line 8007: case C00_ELEMENT_WALL:
  - line 8017: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C00_WALL_D0R], C716_ZONE_WALL_D0L);
  - line 8033: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L);
  - line 8038: return;
  - line 8059: F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M610_VIEW_SQUARE_D0L]], C716_ZONE_WALL_D0L);
  - line 8117: case C00_ELEMENT_WALL:
  - line 8127: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C01_WALL_D0L], C717_ZONE_WALL_D0R);
  - line 8139: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R);
  - line 8144: return;
  - line 8159: F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M611_VIEW_SQUARE_D0R]], C717_ZONE_WALL_D0R);

## Firestaff Evidence

- PASS local-d1-d0-wall-specs-present (dm1_v1_viewport_3d_pc34_compat.c:281-312)
  - Firestaff exposes D1/D0 wall metadata with ReDMCSB return/alcove source anchors.

- PASS local-side-occlusion-d1-d0-cell-orders-present (dm1_v1_viewport_3d_pc34_compat.c:139-171)
  - Open side branches keep their source cell-order contracts separate from wall-return blockers.

- PASS local-runtime-test-covers-d1-d0-wall-occlusion (test_dm1_v1_viewport_3d_pc34_compat.c:234-310)
  - The narrow runtime test checks D1/D0 zone/pairing and wall item occlusion outcomes.

## Verification

- command: /home/trv2/work/firestaff/build/test_dm1_v1_viewport_3d_pc34_compat
  - returncode: 0
  - output tail:
~~~
PASS source_evidence.d1c_door_front_occlusion == 1
PASS source_evidence.d1c_door_button_occlusion == 1
PASS source_evidence.d0c_thieves_eye_frame_occlusion == 1
PASS source_evidence.side_occlusion == 1
PASS source_evidence.defs_zones == 1
PASS source_evidence.wall_source_clip_gate == 1
PASS source_evidence.wall_empty_blit_gate == 1
PASS source_evidence.occlusion == 1
PASS source_evidence.command_dispatch == 1
PASS source_evidence.next_redraw == 1
PASS source_evidence.present_wait == 1
PASS dm1_v1_viewport_3d_source_lock
~~~

- command: /usr/bin/python3 /home/trv2/work/firestaff/tools/verify_pass516_dm1_v1_d1_d0_wall_occlusion_source_lock.py --check-only
  - returncode: 0
  - output tail:
~~~
PASS pass516 check-only
~~~

## Non-Claims

- No input or movement dispatch code is changed.
- No renderer runtime behavior is changed.
- DANNESBURK was not used.
