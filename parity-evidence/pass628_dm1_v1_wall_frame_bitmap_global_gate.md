# Pass628 DM1 V1 wall-frame bitmap global gate

Status: passed

The DM1 V1 wall/door-frame bitmap base pointer is source-locked to the PC34 G2107/G2110-G2120 offset model and remains null-guarded until assets are wired.

## ReDMCSB evidence
- PASS pc34-wallset-doorframe-offsets (DUNVIEW.C:125-158)
  - line 126: int16_t G2108_Floor = -1;
  - line 144: int16_t G3015_i_WallSet_Wall_D0R = -17;
  - line 145: int16_t G2115_DoorFrameTopD2LCR = -36;
  - line 147: int16_t G2120_DoorFrameLeftD3L = -29;
  - line 148: int16_t G2119_DoorFrameLeftD3C = -30;
  - line 150: int16_t G2117_DoorFrameLeftD1C = -32;
  - line 151: int16_t G2116_DoorFrameFrontD0C = -34;
- PASS pc34-g2107-wallset-array-order (DUNVIEW.C:183-205)
  - line 183: int16_t G2107_WallSet[15]
  - line 186: -17,  /* Wall D0R */
  - line 187: -16,  /* Wall D0L */
  - line 188: -15,  /* Wall D1R */
  - line 189: -14,  /* Wall D1L */
  - line 190: -13,  /* Wall D1C */
- PASS pc34-draw-indexed-bitmaps (DUNVIEW.C:3048-3105)
  - line 3048: void F0100_DUNGEONVIEW_DrawWallSetBitmap(
  - line 3055: F0132_VIDEO_Blit(P0105_puc_Bitmap, G0296_puc_Bitmap_Viewport
  - line 3096: void F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally(
- PASS pc34-door-frame-index-calls (DUNVIEW.C:6446-6454)
  - line 6453: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2120_DoorFrameLeftD3L, C718_ZONE_DOOR_FRAME_LEFT_D3L);
  - line 6454: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2121_, C719_ZONE_DOOR_FRAME_RIGHT_D3L);
- PASS pc34-front-door-frame-index-calls (DUNVIEW.C:6725-6739)
  - line 6734: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2119_DoorFrameLeftD3C, C722_ZONE_DOOR_FRAME_LEFT_D3C);
  - line 6735: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2119_DoorFrameLeftD3C, C723_ZONE_DOOR_FRAME_RIGHT_D3C);
- PASS pc34-parity-wallset-switch (DUNVIEW.C:8396-8413)
  - line 8396: G3011_i_WallSet_Wall_D3C = G3049_i_WallSetFlipped_Wall_D3C;
  - line 8406: G3015_i_WallSet_Wall_D0R = G3059_i_WallSetFlipped_Wall_D0R;
  - line 8413: F0007_MAIN_CopyBytes(M772_CAST_PC(G3048_WallSetFlipped), M772_CAST_PC(G2107_WallSet), sizeof(G2107_WallSet));

## Firestaff evidence
- PASS global-definition-null (src/dm1/dm1_v1_viewport_3d_pc34_compat.c)
- PASS draw-frame-base-read (src/dm1/dm1_v1_viewport_3d_pc34_compat.c)
- PASS door-frame-null-guard (src/dm1/dm1_v1_viewport_3d_pc34_compat.c)
- PASS wall-loop-null-guard (src/dm1/dm1_v1_viewport_3d_pc34_compat.c)
- PASS csb-back-null-guard (src/dm1/dm1_v1_viewport_3d_pc34_compat.c)
- PASS draw-wall-null-safe (src/dm1/dm1_v1_viewport_3d_pc34_compat.c)
- PASS c-regression-default-null (tests/test_dm1_v1_viewport_3d_pc34_compat.c)
- PASS c-regression-no-write (tests/test_dm1_v1_viewport_3d_pc34_compat.c)
- PASS cmake-registration (CMakeLists.txt)

## Verification
- /Users/bosse/.openclaw/workspace-main/build-csb-v2-phase4/test_dm1_v1_viewport_3d_pc34_compat: rc=0
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
PASS source_evidence.same_viewport_mouse == 1
PASS source_evidence.same_viewport_turn == 1
PASS source_evidence.same_viewport_move == 1
PASS source_evidence.same_viewport_draw == 1
PASS source_evidence.same_viewport_present == 1
PASS source_evidence.same_viewport_assets == 1
PASS dm1_v1_viewport_3d_source_lock
~~~
- /opt/homebrew/opt/python@3.14/bin/python3.14 /Users/bosse/.openclaw/workspace-main/tools/verify_pass628_dm1_v1_wall_frame_bitmap_global_gate.py --check-only: rc=0
~~~
PASS pass628 check-only
~~~

## Non-claims
- No original DOS framebuffer parity claim.
- No GRAPHICS.DAT asset extraction or publication claim.
- No renderer output change; this pass adds a guard regression and evidence gate.
- No CSB, DM2, Nexus, Theron, or DM1 V2 behavior claim.
