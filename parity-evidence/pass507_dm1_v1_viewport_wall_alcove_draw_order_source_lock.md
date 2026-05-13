# Pass 507 DM1 V1 viewport wall/alcove draw-order source lock

Lane B evidence pass for DM1 V1 viewport walls. This pass is intentionally
source-only: it does not touch movement, command routing, or pass435.

## ReDMCSB anchors

- DUNVIEW.C:3502 F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF
  identifies wall ornaments that switch wall handling into the alcove-content
  exception.
- DUNVIEW.C:4561-4581 in
  F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF defines
  the packed cell-order contract. A first nibble of 0 means wall-alcove object
  drawing; cells otherwise draw objects, creatures and projectiles in source
  order, with explosions after all cells.
- DUNVIEW.C:7874-7937 in F0124_DUNGEONVIEW_DrawSquareD1C proves the front-door
  occlusion split: rear cells, frame/door, then front cells.
- DUNVIEW.C:8318-8542 in F0128_DUNGEONVIEW_Draw_CPSF proves the visible square
  replay order from D4 through D0.

## Firestaff anchors

- dm1_v1_viewport_3d_pc34_compat.c:78-102 mirrors the F0128 draw sequence in
  s_draw_order.
- dm1_v1_viewport_3d_pc34_compat.c:109-117 mirrors the F0115 per-cell layer
  order.
- dm1_v1_viewport_3d_pc34_compat.c:135-140 mirrors the door-front two-pass
  occlusion sequence.
- dm1_v1_viewport_3d_pc34_compat.c:194-210 keeps wall draw specs, wall-case
  returns and the front-alcove exception together.
- test_dm1_v1_viewport_3d_pc34_compat.c covers the tables through
  test_redmcsb_f0128_draw_order, test_f0115_cell_order_and_layer_z_order,
  test_door_front_occlusion_split_passes and the wall bitmap selection test.

## Gate
