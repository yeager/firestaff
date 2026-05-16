# Pass576 DM1 V1 side-wall occlusion source-row clipping

Status: passed

## Claim

Side-wall lanes are source-locked separately from front-wall/front-cell gates. ReDMCSB draws D3/D2/D1/D0 side lanes in F0128 row order, side-wall cases draw their side zones and return before open-lane/field paths, and the shared bitmap path retains source-row clipping before blit.

## Primary ReDMCSB Evidence

- PASS redmcsb_pc34_side_wall_zone_ids_are_not_front_cells (DEFS.H:4042-4057)
  - PC34 side-wall zones are distinct D3/D2/D1/D0 side lanes; this gate excludes D3C/D2C/D1C/D0C front/center cells.
  - line 4042: #define C702_ZONE_WALL_D3L2
  - line 4043: #define C703_ZONE_WALL_D3R2
  - line 4045: #define C705_ZONE_WALL_D3L
  - line 4046: #define C706_ZONE_WALL_D3R
  - line 4047: #define C707_ZONE_WALL_D2L2
  - line 4048: #define C708_ZONE_WALL_D2R2
  - line 4050: #define C710_ZONE_WALL_D2L
  - line 4051: #define C711_ZONE_WALL_D2R
  - line 4053: #define C713_ZONE_WALL_D1L
  - line 4054: #define C714_ZONE_WALL_D1R
  - line 4056: #define C716_ZONE_WALL_D0L
  - line 4057: #define C717_ZONE_WALL_D0R

- PASS redmcsb_f0128_draws_side_rows_before_centers (DUNVIEW.C:8478-8542)
  - F0128 traverses side lanes in row order before each same-depth center/front cell.
  - line 8482: F0676_DrawD3L2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8486: F0677_DrawD3R2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8491: F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8495: F0117_DUNGEONVIEW_DrawSquareD3R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8499: F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8504: F0678_DrawD2L2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8508: F0679_DrawD2R2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8513: F0119_DUNGEONVIEW_DrawSquareD2L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8517: F0120_DUNGEONVIEW_DrawSquareD2R_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8521: F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8525: F0122_DUNGEONVIEW_DrawSquareD1L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8529: F0123_DUNGEONVIEW_DrawSquareD1R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8533: F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8537: F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8541: F0126_DUNGEONVIEW_DrawSquareD0R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8542: F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);

- PASS redmcsb_far_side_wall_cases_return_before_field_paths (DUNVIEW.C:6846-6896)
  - D2L2/D2R2 wall cases draw side zones and return before teleporter field paths.
  - line 6848: case C00_ELEMENT_WALL:
  - line 6851: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C05_WALL_D2R2], C707_ZONE_WALL_D2L2);
  - line 6854: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C06_WALL_D2L2]
  - line 6858: , C707_ZONE_WALL_D2L2);
  - line 6862: return;
  - line 6863: case C05_ELEMENT_TELEPORTER:
  - line 6864: F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[C09_VIEW_SQUARE_D2L2]], C707_ZONE_WALL_D2L2);
  - line 6868: STATICFUNCTION void F0679_DrawD2R2(
  - line 6879: case C00_ELEMENT_WALL:
  - line 6882: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C06_WALL_D2L2], C708_ZONE_WALL_D2R2);
  - line 6885: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C05_WALL_D2R2]
  - line 6889: , C708_ZONE_WALL_D2R2);
  - line 6893: return;
  - line 6894: case C05_ELEMENT_TELEPORTER:
  - line 6895: F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[C10_VIEW_SQUARE_D2R2]], C708_ZONE_WALL_D2R2);

- PASS redmcsb_d1_side_wall_cases_return_before_open_lane_paths (DUNVIEW.C:7436-7628)
  - D1L/D1R wall cases draw side zones, apply only side ornament checks, then return.
  - line 7436: case C00_ELEMENT_WALL:
  - line 7446: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C02_WALL_D1R], C713_ZONE_WALL_D1L);
  - line 7454: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C03_WALL_D1L], C713_ZONE_WALL_D1L);
  - line 7459: F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0214_ai_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL], M585_VIEW_WALL_D1L_RIGHT);
  - line 7460: return;
  - line 7604: case C00_ELEMENT_WALL:
  - line 7614: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C03_WALL_D1L], C714_ZONE_WALL_D1R);
  - line 7622: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C02_WALL_D1R], C714_ZONE_WALL_D1R);
  - line 7627: F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0216_ai_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL], M586_VIEW_WALL_D1R_LEFT);
  - line 7628: return;

- PASS redmcsb_d0_side_wall_cases_return_before_source_row_field_paths (DUNVIEW.C:7999-8159)
  - D0L/D0R wall cases draw side zones and return before the source-row field/open-lane paths.
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

- PASS redmcsb_shared_bitmap_source_row_clipping_seam (DUNVIEW.C:3394-3472)
  - F0791 resolves source/destination clipping through F0635 before blitting side-wall-adjacent bitmap zones.
  - line 3394: STATICFUNCTION void F0791_DUNGEONVIEW_DrawBitmapXX(
  - line 3412: if (P2081_i_ZoneIndex == CM1_UNKNOWN) {
  - line 3423: F0635_(P0101_puc_Bitmap_Source, G2032_ai_XYZ, P2081_i_ZoneIndex, &L2447_i_Width, &L2448_i_Height)
  - line 3427: if ((L2449_i_ > L2450_i_) && M007_GET(P2082_i_Flip, MASK0x0001_FLIP_HORIZONTAL))
  - line 3435: L2447_i_Width += L2449_i_;
  - line 3438: if ((L2449_i_ > L2450_i_) && M007_GET(P2082_i_Flip, MASK0x0002_FLIP_VERTICAL))
  - line 3446: L2448_i_Height += L2449_i_;
  - line 3464: F0132_VIDEO_Blit(P0101_puc_Bitmap_Source, P0102_puc_Bitmap_Destination, G2032_ai_XYZ, L2447_i_Width, L2448_i_Height

## Firestaff Evidence

- PASS firestaff_side_wall_metadata_has_returning_side_lanes_only (dm1_v1_viewport_3d_pc34_compat.c:291-300)
  - Firestaff metadata encodes side wall returns for far-side, D1, and D0 side lanes without center/front cells.
  - line 291: DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2
  - line 291: DM1_PC34_ZONE_WALL_D2L2
  - line 291: DUNVIEW.C:6848-6862 wall case returns
  - line 292: DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2
  - line 292: DM1_PC34_ZONE_WALL_D2R2
  - line 292: DUNVIEW.C:6882-6893 wall case returns
  - line 296: DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R
  - line 296: DM1_PC34_ZONE_WALL_D1L
  - line 296: DUNVIEW.C:7459-7460 side ornament then return
  - line 297: DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L
  - line 297: DM1_PC34_ZONE_WALL_D1R
  - line 297: DUNVIEW.C:7627-7628 side ornament then return
  - line 299: DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R
  - line 299: DM1_PC34_ZONE_WALL_D0L
  - line 299: DUNVIEW.C:8036-8038 wall case returns
  - line 300: DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L
  - line 300: DM1_PC34_ZONE_WALL_D0R
  - line 300: DUNVIEW.C:8142-8144 wall case returns

- PASS firestaff_wall_clip_gate_retains_source_offsets_and_occlusion (dm1_v1_viewport_3d_pc34_compat.c:729-768)
  - The local wall clip gate preserves source X/Y offsets, clips to source and viewport bounds, and can mark fully occluded rows invisible.
  - line 729: DM1_ViewportBlitClipGate dm1_viewport_3d_resolve_wall_blit_clip_gate
  - line 741: int src_x = frame->blit_x;
  - line 742: int src_y = frame->blit_y;
  - line 749: if (dst_x < 0) { src_x -= dst_x; width += dst_x; dst_x = 0; }
  - line 756: if (src_x + width > source_width) width = source_width - src_x;
  - line 759: if (width <= 0 || height <= 0) return gate;
  - line 761: gate.visible = true;
  - line 762: gate.src_x = (int16_t)src_x;
  - line 763: gate.src_y = (int16_t)src_y;

- PASS firestaff_narrow_runtime_assertions_cover_side_walls_and_clip_rows (test_dm1_v1_viewport_3d_pc34_compat.c:222-248)
  - Existing narrow runtime assertions cover side wall zones/returns; the same file also asserts source-row clipping edge cases.
  - line 239: DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2
  - line 239: DM1_PC34_ZONE_WALL_D2L2
  - line 239: "6862"
  - line 240: DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2
  - line 240: DM1_PC34_ZONE_WALL_D2R2
  - line 240: "6893"
  - line 244: DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R
  - line 244: DM1_PC34_ZONE_WALL_D1L
  - line 244: "7460"
  - line 245: DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L
  - line 245: DM1_PC34_ZONE_WALL_D1R
  - line 245: "7628"
  - line 247: DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R
  - line 247: DM1_PC34_ZONE_WALL_D0L
  - line 247: "8038"
  - line 248: DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L
  - line 248: DM1_PC34_ZONE_WALL_D0R
  - line 248: "8144"

- PASS firestaff_clip_row_runtime_assertions_are_registered (test_dm1_v1_viewport_3d_pc34_compat.c:799-855)
  - Source-row clipping has explicit visible, source-occluded, viewport-occluded, and draw-copy assertions.
  - line 799: static void test_wall_source_row_clip_occlusion_gate(void)
  - line 804: wall_clip_gate.151713.src_x
  - line 805: wall_clip_gate.151713.src_y
  - line 828: wall_clip_gate.occluded_source_row
  - line 832: wall_clip_gate.occluded_viewport
  - line 835: static void test_wall_draw_uses_clip_gate_source_offsets(void)
  - line 847: wall_clip_draw.source_offset_next
  - line 854: wall_clip_draw.opaque_copies_transparent_color

## DM1 Hash Locks
- PASS DM1 canonical PC34/V1 GRAPHICS.DAT: /home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e bytes=363417
- PASS DM1 canonical PC34/V1 DUNGEON.DAT: /home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 bytes=33357

## Non-Claims
- No front-wall/front-cell behavior is promoted by this pass.
- No renderer behavior, release artifact, tag, or external state is changed.
- No original runtime capture or pixel-parity claim is made.
- Only N2-local ReDMCSB and canonical DM1 references are used.
