# Pass563 DM1 V1 D1 side wall source lock

Status: failed

Claim: D1L and mirrored D1R use the ReDMCSB PC34 side-wall lanes: F0128 draws D1L then D1R before D1C, and each D1 side-wall case draws its side-specific wall zone and returns before open-lane content/field paths.

## Primary ReDMCSB Evidence

- PASS defs-pc34-d1-side-wall-zones (DEFS.H:4052-4054)
  - line 4053: #define C713_ZONE_WALL_D1L
  - line 4054: #define C714_ZONE_WALL_D1R

- PASS f0128-d1-row-left-right-before-center (DUNVIEW.C:8518-8533)
  - line 8521: F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8524: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, -1, &L0224_i_MapX, &L0225_i_MapY);
  - line 8525: F0122_DUNGEONVIEW_DrawSquareD1L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8528: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 1, &L0224_i_MapX, &L0225_i_MapY);
  - line 8529: F0123_DUNGEONVIEW_DrawSquareD1R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8532: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 0, &L0224_i_MapX, &L0225_i_MapY);
  - line 8533: F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);

- PASS d1l-wall-branch-draws-zone-and-returns (DUNVIEW.C:7436-7460)
  - line 7436: case C00_ELEMENT_WALL:
  - line 7446: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C02_WALL_D1R], C713_ZONE_WALL_D1L);
  - line 7454: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C03_WALL_D1L], C713_ZONE_WALL_D1L);
  - line 7459: F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0214_ai_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL], M585_VIEW_WALL_D1L_RIGHT);
  - line 7460: return;

- PASS d1r-wall-branch-mirrors-zone-and-returns (DUNVIEW.C:7604-7628)
  - line 7604: case C00_ELEMENT_WALL:
  - line 7614: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C03_WALL_D1L], C714_ZONE_WALL_D1R);
  - line 7622: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C02_WALL_D1R], C714_ZONE_WALL_D1R);
  - line 7627: F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0216_ai_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL], M586_VIEW_WALL_D1R_LEFT);
  - line 7628: return;

## Firestaff Evidence

- FAIL firestaff-d1-side-wall-metadata (dm1_v1_viewport_3d_pc34_compat.c:280-290)
  - missing: DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R
  - missing: DM1_PC34_ZONE_WALL_D1L
  - missing: DUNVIEW.C:7445-7455
  - missing: DUNVIEW.C:7459-7460 side ornament then return
  - missing: DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L
  - missing: DM1_PC34_ZONE_WALL_D1R
  - missing: DUNVIEW.C:7613-7623
  - missing: DUNVIEW.C:7627-7628 side ornament then return

- PASS firestaff-d1-side-wall-runtime-test (test_dm1_v1_viewport_3d_pc34_compat.c:240-245)
  - line 244: DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R
  - line 244: DM1_PC34_ZONE_WALL_D1L
  - line 244: "7460"
  - line 245: DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L
  - line 245: DM1_PC34_ZONE_WALL_D1R
  - line 245: "7628"

- FAIL firestaff-source-evidence-string (dm1_v1_viewport_3d_pc34_compat.c:1068-1093)
  - missing: DUNVIEW.C:7391-7557 D1L stairs/pit/floor-ornament/ceiling-pit/F0115/teleporter-field order; wall returns before F0115
  - missing: DUNVIEW.C:7559-7725 D1R stairs/pit/floor-ornament/ceiling-pit/F0115/teleporter-field order; wall returns before F0115
  - missing: DUNVIEW.C:7391 F0122_DUNGEONVIEW_DrawSquareD1L
  - missing: DUNVIEW.C:7559 F0123_DUNGEONVIEW_DrawSquareD1R

## Verification

- /home/trv2/work/firestaff-worktrees/pass593-dm1v1-landable-batch/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
PASS source_evidence.far_door_front_occlusion == 1
PASS source_evidence.d1_side_door_front_occlusion == 1
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

- /usr/bin/python3 /home/trv2/work/firestaff-worktrees/pass593-dm1v1-landable-batch/tools/verify_pass563_dm1_v1_d1_side_wall_source_lock.py --check-only: rc=1
~~~
FAIL pass563 check-only: firestaff-d1-side-wall-metadata,firestaff-source-evidence-string
~~~
