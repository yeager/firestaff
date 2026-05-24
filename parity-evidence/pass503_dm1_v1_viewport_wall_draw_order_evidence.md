# Pass503 - DM1 V1 viewport wall draw-order evidence

Status: PASS_PASS503_DM1_V1_VIEWPORT_WALL_DRAW_ORDER_EVIDENCE

## ReDMCSB source locks
- DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF lines 8318-8543 ok=True: Viewport composition replays source square functions from far to near before presentation.
  - line 8318: void F0128_DUNGEONVIEW_Draw_CPSF(
  - line 8338: F0098_DUNGEONVIEW_DrawFloorAndCeiling();
  - line 8469: M598_VIEW_SQUARE_D4L
  - line 8482: F0676_DrawD3L2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8491: F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8499: F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8521: F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8533: F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);
  - line 8542: F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);
- DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF lines 4547-4582 ok=True: Per-cell content order is objects, creatures, projectiles, then explosions/fluxcage.
  - line 4547: STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(
  - line 4570: draw each object found
  - line 4573: Draw one creature at the cell being processed
  - line 4576: Draw only projectiles at specified cell
  - line 4580: Draw only explosions at specified cell
  - line 4582: If a Fluxcage is present, draw the fluxcage
- DUNVIEW.C F0100/F0101/F0102/F0765 wall and door blits lines 3048-3180 ok=True: Wall and door panels write into G0296 with explicit transparent or opaque routes.
  - line 3048: void F0100_DUNGEONVIEW_DrawWallSetBitmap(
  - line 3055: F0132_VIDEO_Blit(P0105_puc_Bitmap, G0296_puc_Bitmap_Viewport
  - line 3065: void F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency(
  - line 3072: CM1_COLOR_NO_TRANSPARENCY
  - line 3082: void F0102_DUNGEONVIEW_DrawDoorBitmap(
  - line 3159: STATICFUNCTION void F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(
- DUNVIEW.C F0676/F0677/F0116/F0117/F0118 D3 wall branches lines 6226-6836 ok=True: D3 walls occlude by drawing wall zones and returning, except front alcoves that intentionally hand contents to F0115.
  - line 6226: void F0676_DrawD3L2(
  - line 6259: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C11_WALL_D3L2], C702_ZONE_WALL_D3L2);
  - line 6293: void F0677_DrawD3R2(
  - line 6326: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C10_WALL_D3R2], C703_ZONE_WALL_D3R2);
  - line 6361: STATICFUNCTION void F0116_DUNGEONVIEW_DrawSquareD3L(
  - line 6434: L0200_i_Order = C0x0000_CELL_ORDER_ALCOVE;
  - line 6642: STATICFUNCTION void F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(
  - line 6714: F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C);
- DUNVIEW.C F0678/F0679/F0119-F0127 near wall branches lines 6837-8308 ok=True: D2, D1, and D0 wall paths preserve the same wall-return/alcove exception pattern.
  - line 6837: STATICFUNCTION void F0678_DrawD2L2(
  - line 6854: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C06_WALL_D2L2]
  - line 7244: STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C(
  - line 7306: F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C09_WALL_D2C], C709_ZONE_WALL_D2C);
  - line 7727: STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(
  - line 7843: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0000_CELL_ORDER_ALCOVE);
  - line 7960: STATICFUNCTION void F0125_DUNGEONVIEW_DrawSquareD0L(
  - line 8064: STATICFUNCTION void F0126_DUNGEONVIEW_DrawSquareD0R(
  - line 8164: STATICFUNCTION void F0127_DUNGEONVIEW_DrawSquareD0C(
- DUNVIEW.C F0124_DUNGEONVIEW_DrawSquareD1C lines 7873-7938 ok=True: Front doors draw rear contents, door/frame, then front contents.
  - line 7873: case C17_ELEMENT_DOOR_FRONT:
  - line 7875: C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT
  - line 7905: F0111_DUNGEONVIEW_DrawDoor(L0218_ai_SquareAspect[M557_DOOR_THING_INDEX]
  - line 7910: L0217_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;
  - line 7875: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING]
- DRAWVIEW.C F0097_DUNGEONVIEW_DrawViewport lines 709-858 ok=True: DUNVIEW composition buffer G0296 is presented by DRAWVIEW after vblank/present gating through the PC34 viewport zone and video-driver blit route.
  - line 709: void F0097_DUNGEONVIEW_DrawViewport(
  - line 721: G0324_B_DrawViewportRequested = C1_TRUE
  - line 722: M526_WaitVerticalBlank();
  - line 721: G0296_puc_Bitmap_Viewport
  - line 850: F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);
  - line 857: (*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);

## Firestaff hooks

- src/dm1/dm1_v1_viewport_3d_pc34_compat.c line 88 ok=True: firestaff-draw-order-table
- src/dm1/dm1_v1_viewport_3d_pc34_compat.c line 352 ok=True: firestaff-wall-spec-table
- src/dm1/dm1_v1_viewport_3d_pc34_compat.c line 129 ok=True: firestaff-thing-layer-table
- src/dm1/dm1_v1_viewport_3d_pc34_compat.c line 172 ok=True: firestaff-door-front-occlusion-table
- tools/verify_pass496_dm1_v1_wall_occlusion_spec_matrix.py line 15 ok=True: pass496-matrix-gate-present
- parity-evidence/pass502_dm1_v1_viewport_wall_occlusion_audit.md line 38 ok=True: pass502-blocker-doc-present

## DM1 anchors

- GRAPHICS.DAT exists=True sha256=2c3aa836925c
- DUNGEON.DAT exists=True sha256=d90b6b1c38fd
- TITLE exists=True sha256=adc7f1916eee
- README.md exists=True sha256=e8e82274f72f

## N2-local secondary references

- /home/trv2/.openclaw/data/firestaff-greatstone-atlas/index/pages.json lines 1-120 ok=True: Greatstone local atlas is present as data-extraction context for DM/CSB graphics and dungeon assets.
- /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin/Viewport.cpp lines 935-1938 ok=True: CSBWin carries a table-driven viewport cell/draw-order model with door-facing two-phase object/door/object rows.
- /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin/Viewport.cpp lines 6694-6819 ok=True: CSBWin DrawViewport summarizes each relative cell and interprets the selected draw script in cell order.
- /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/Viewport.cpp lines 935-1938 ok=True: CSB lineage source mirrors the same viewport cell/draw-order and door-facing script structure.
- /home/trv2/.openclaw/data/firestaff-original-games/DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.md lines 1-220 ok=True: DM originals include a local PC34-vs-Greatstone manifest for asset provenance cross-checking.

## Non-claims

- This is source/probe evidence only.
- Pixel parity still needs the same-viewport original/Firestaff runtime capture described by pass502.
