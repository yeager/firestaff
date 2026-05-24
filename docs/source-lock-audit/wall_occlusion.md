# JOB 1: DM1 V1 Wall Occlusion (Side Walls) — Source Lock Audit

## ReDMCSB Source

### Draw Order / Occlusion Architecture
- **DUNVIEW.C F0128 lines 8435–8542**: The main draw loop iterates over `s_draw_order[]`
  (F0128 sequence table), computing map coords via `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`,
  then dispatches to F0116–F0127 per square.
- **DUNVIEW.C lines 8466–8477**: Far-object pass for D4L/D4R/D4C uses
  `F0162_DUNGEON_GetSquareFirstObject(...)` with `C0x0001_CELL_ORDER_BACKLEFT`.
  **This happens BEFORE the D3 side-wall helpers** (lines 8478–8499), ensuring
  nearer wall panels occlude any far-object pixels.

### Side Wall Draw (F0116 / F0117)
- **F0116_DUNGEONVIEW_DrawSquareD3L (line 6361)**:
  `C00_ELEMENT_WALL` branch (line 6421): draws `G0698_puc_Bitmap_WallSet_Wall_D3LCR`
  via F0100, then wall ornaments (F0107), then alcove check → return.
  **No F0115 call after wall draw** — so no things rendered behind the wall panel.
- **F0117_DUNGEONVIEW_DrawSquareD3R (line 6500)**: mirror of F0116, same pattern.
- **F0122_DUNGEONVIEW_DrawSquareD1L (line 7391)** and **F0123_DUNGEONVIEW_DrawSquareD1R (line 7559)**:
  same WALL → return pattern (lines 7459–7460, 7627–7628).

### Door-Side Occlusion
- **DUNVIEW.C:6438–6441 (D3L door-side)**: sets `L0200_i_Order = C0x0321_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTRIGHT`,
  then calls F0115 with that order — things beside/through the door drawn at correct layer.
- Same pattern for all other door-side cases (D3R, D2L, D2R, D1L, D1R, D0L, D0R).

## Firestaff Implementation

### Draw Order Table — PASS
- `s_draw_order[]` (line 88) mirrors ReDMCSB F0128 sequence exactly:
  D4L → D4R → D4C → D3L2 → D3R2 → D3L → D3R → D3C → D2L2 → D2R2 → D2L → D2R → D2C → D1L → D1R → D1C → D0L → D0R → D0C.
  Line refs: 88–107.
- Far-object spec `s_far_object_pass_specs[]` (line 116) encodes D4 far-object passes
  with `CELL_ORDER_BACKLEFT (0x0001)`, matching DUNVIEW.C:8466–8477.

### Occlusion Comment — CORRECT
- Lines 113–115 explicitly document: D4 far objects drawn before D3 side-wall helpers,
  "so nearer wall panels occlude any far object pixels."

### Side Wall Draw — PASS
- `s_wall_draw_specs[]` (line 352): each wall entry has `draws_things_after_wall = false`
  for all side wall squares (D3L, D3R, D2L, D2R, D1L, D1R, D0L, D0R).
  Front-center walls (D3C, D2C, D1C) have `true` for alcove cases.
- `dm1_viewport_3d_draw_frame()` (line 660): door frame draws only; the structural
  framework draws side wall panels (no F0115/thing pass after wall return).
  D0L/D0R side walls (lines 875–893): `dm1_viewport_3d_draw_wall_opaque()` with
  parity-aware bitmap selection — matches ReDMCSB side wall no-F0115 pattern.

### Wall Frame Clipping — PASS
- `dm1_viewport_3d_resolve_wall_blit_clip_gate()` (line 945): clips source rectangle
  to wall bitmap bounds, then clips destination to viewport (0..223, 0..135).
  Guards `if (!gate.visible) return` in both `dm1_viewport_3d_draw_wall()` (line 541)
  and `dm1_viewport_3d_draw_wall_opaque()` (line 575).
  Source ref: DUNVIEW.C:3053–3058, COORD.C:2390–2409, IMAGE3.C:866–889.

## Verdict: PASS

Firestaff side wall occlusion matches ReDMCSB:
1. Draw order back-to-front (D4 far-objects before D3 walls) ✓
2. WALL element in F0116/F0117/F0122/F0123 → return without F0115 ✓
3. Door-side → F0115 with correct CELL_ORDER ✓
4. Clipping gate guards empty/out-of-bounds blits ✓
5. `s_wall_draw_specs` encodes `draws_things_after_wall = false` for all side walls ✓

No fix needed.