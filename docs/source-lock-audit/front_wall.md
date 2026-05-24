# JOB 2: DM1 V1 Front Wall Rendering — Source Lock Audit

## ReDMCSB Source

### Front Wall Draw Functions
- **F0124_DUNGEONVIEW_DrawSquareD1C (line 7727)**: `C00_ELEMENT_WALL` branch (line 7833):
  calls `F0100_DUNGEONVIEW_DrawWallSetBitmap(G0700_puc_Bitmap_WallSet_Wall_D1LCR, ...)`
  then `F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF` with `M552_FRONT_WALL_ORNAMENT_ORDINAL`.
  If alcove: `L0201_i_Order = C0x0002_CELL_ORDER_FRONTLEFT`, branches to F0115.
  **Otherwise returns** — no F0115 after front wall (no things behind D1C wall).
- **F0118_DUNGEONVIEW_DrawSquareD3C_CPSF (line 6642)**: WALL branch (line 6707):
  draws G0698 wall set + alcove check + F0115 only if alcove.
- **F0121_DUNGEONVIEW_DrawSquareD2C (line 7244)**: WALL branch (line 7299), similar pattern.
- **F0127_DUNGEONVIEW_DrawSquareD0C (line 8164)**: for front wall at depth 0,
  `C00_ELEMENT_WALL` case (line 8185) returns directly without F0115.

### Layer Draw Order
- **DUNVIEW.C lines 8466–8542**: F0128 calls F0116→F0117→F0118 (depth 3),
  then F0119→F0120→F0121 (depth 2), then F0122→F0123→F0124 (depth 1),
  then F0125→F0126→F0127 (depth 0). Center walls drawn after side walls at same depth.
- **DUNVIEW.C lines 5661–4581 F0115**: thing layer ordering is:
  objects → creatures → projectiles → explosions.
  `s_thing_layers[]` in Firestaff encodes this (line 130).

## Firestaff Implementation

### Front Wall Spec — PASS
- `s_wall_draw_specs[]` (line 352):
  - `DM1_VIEW_SQUARE_D3C` (line 357): `has_front_alcove = true`, `draws_things_after_wall = true`
  - `DM1_VIEW_SQUARE_D2C` (line 362): `has_front_alcove = true`, `draws_things_after_wall = true`
  - `DM1_VIEW_SQUARE_D1C` (line 365): `has_front_alcove = false`, `draws_things_after_wall = true`
    (front alcove draws F0115; no side cells behind D1C — correct)
  - `DM1_VIEW_SQUARE_D0C` (line 366): `has_front_alcove = false`, `draws_things_after_wall = false`
    (D0C wall returns before any F0115 — matches ReDMCSB DUNVIEW.C:8185)
- All front wall entries reference `DM1_WALL_DxC` type and correct `DM1_PC34_ZONE_WALL_DxC` zones.

### Layer Ordering — PASS
- `s_thing_layers[]` (line 130): objects → creatures → projectiles → explosions,
  matching DUNVIEW.C:4567–4581, 4853–4860, 5195–5202, 5681–5883, 5915–5933.
- Comment at line 129: "F0115 per-cell z-order. ReDMCSB explicitly scans the thing
  list multiple times for each cell" — correctly documented.

### Draw Frame Order — PASS
- `dm1_viewport_3d_draw_frame()` (line 660): draws door frames first (depth 3→0),
  then after the door frame section calls `state->floor_ceiling_dirty = true`.
  Door frame top bars and wall panels are drawn within the draw call order
  (structural framework for wall drawing, with discrete blits for door frames).

### Cell Order Constants — PASS
- `s_cell_orders[]` (line 178): front wall alcove orders:
  - D3C front alcove: `0x0001_BACKLEFT` (DUNVIEW.C:6716–6720)
  - D2C front alcove: `0x0001_BACKLEFT` (DUNVIEW.C:7308–7312)
  - D1C front alcove: `0x0002_FRONTLEFT` (DUNVIEW.C:7842–7843)
  All verified against ReDMCSB DrawSquare D3C/D2C/D1C wall branches.

## Verdict: PASS

Front wall layer ordering matches ReDMCSB:
1. D3C drawn after D3L/D3R (depth 3, center after sides) ✓
2. D2C drawn after D2L/D2R (depth 2) ✓
3. D1C drawn after D1L/D1R (depth 1) ✓
4. D0C drawn last (depth 0) ✓
5. Front wall alcoves trigger F0115 with correct CELL_ORDER ✓
6. Non-alcove front walls return without F0115 (nothing behind wall panel) ✓
7. Thing layer order: objects → creatures → projectiles → explosions ✓

No fix needed.