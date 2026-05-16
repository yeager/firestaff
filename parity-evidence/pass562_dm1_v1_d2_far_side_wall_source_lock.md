# Pass562 DM1 V1 D2 far-side wall source lock

Status: passed

Claim: D2L2 and mirrored D2R2 use the ReDMCSB PC34 far-side wall lanes: wall cases draw the paired native/parity wallset into C707/C708 and return, while teleporter cases draw only the matching field zone.

## Primary ReDMCSB Evidence

- PASS defs-pc34-d2-far-side-zones (DEFS.H:4042-4049)
  - line 4047: #define C707_ZONE_WALL_D2L2
  - line 4048: #define C708_ZONE_WALL_D2R2

- PASS d2l2-wall-and-field-branch (DUNVIEW.C:6836-6865)
  - line 6837: STATICFUNCTION void F0678_DrawD2L2(
  - line 6848: case C00_ELEMENT_WALL:
  - line 6851: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C05_WALL_D2R2], C707_ZONE_WALL_D2L2);
  - line 6854: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C06_WALL_D2L2]
  - line 6858: C707_ZONE_WALL_D2L2);
  - line 6862: return;
  - line 6863: case C05_ELEMENT_TELEPORTER:
  - line 6864: C09_VIEW_SQUARE_D2L2]], C707_ZONE_WALL_D2L2);

- PASS d2r2-mirrored-wall-and-field-branch (DUNVIEW.C:6868-6895)
  - line 6868: STATICFUNCTION void F0679_DrawD2R2(
  - line 6879: case C00_ELEMENT_WALL:
  - line 6882: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C06_WALL_D2L2], C708_ZONE_WALL_D2R2);
  - line 6885: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C05_WALL_D2R2]
  - line 6889: C708_ZONE_WALL_D2R2);
  - line 6893: return;
  - line 6894: case C05_ELEMENT_TELEPORTER:
  - line 6895: C10_VIEW_SQUARE_D2R2]], C708_ZONE_WALL_D2R2);

## Firestaff Evidence

- PASS firestaff-d2-far-side-wall-metadata (dm1_v1_viewport_3d_pc34_compat.c:281-302)
  - line 291: DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2
  - line 291: DM1_PC34_ZONE_WALL_D2L2
  - line 291: DUNVIEW.C:6849-6858
  - line 291: DUNVIEW.C:6848-6862 wall case returns
  - line 292: DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2
  - line 292: DM1_PC34_ZONE_WALL_D2R2
  - line 292: DUNVIEW.C:6880-6889
  - line 292: DUNVIEW.C:6882-6893 wall case returns

- PASS firestaff-d2-far-side-runtime-test (test_dm1_v1_viewport_3d_pc34_compat.c:229-249)
  - line 239: DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2
  - line 239: DM1_PC34_ZONE_WALL_D2L2
  - line 239: "6862"
  - line 240: DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2
  - line 240: DM1_PC34_ZONE_WALL_D2R2
  - line 240: "6893"

- PASS firestaff-source-evidence-string (dm1_v1_viewport_3d_pc34_compat.c:1118-1138)
  - line 1128: DUNVIEW.C:6849-6893 F0678/F0679 PC34 D2L2/D2R2 side-wall zones and wall-case returns

## Verification

- /home/trv2/work/firestaff-worktrees/pass601-dm1v1-gameplay-systems-source-lock-20260516/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
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

- /usr/bin/python3 /home/trv2/work/firestaff-worktrees/pass601-dm1v1-gameplay-systems-source-lock-20260516/tools/verify_pass562_dm1_v1_d2_far_side_wall_source_lock.py --check-only: rc=0
~~~
PASS pass562 check-only
~~~
