# JOB 3: DM1 V1 Wall Row/Column Clipping — Source Lock Audit

## ReDMCSB Source

### Frame Clipping in F0100_DUNGEONVIEW_DrawWallSetBitmap
- **DUNVIEW.C:3053–3058**: F0100 checks `frame[C4_BYTE_WIDTH]` — if zero,
  the wall slot is empty and the draw is skipped (no-op blit).
- **DUNVIEW.C:3058**: blit call uses `frame[C6_X]` and `frame[C7_Y]` as
  destination coordinates — these are pre-clipped by the frame setup.

### Zone Clipping in COORD.C F0635
- **COORD.C:2390–2409**: F0635 clips MEDIA720 zones and source offsets.
  When a zone rectangle extends beyond the viewport, the function computes
  the overlapping region and returns the clipped zone + source offset.
  This is used by all wall/door/floor draw calls on PC34.

### Empty Blit Guard in IMAGE3.C F0684
- **IMAGE3.C:866–889**: F0684 skips blits where the clipped width or height ≤ 0.
  This is the final guard that prevents out-of-bounds memory access.

### Projectile Zone Clipping
- **DUNVIEW.C:5667–5683**: projectiles use G2028_ac_ViewSquareIndexTo[...]
  to compute row index, then C2900_ZONE_+row*4+viewCell to get PC34 zone.
  D3 front cells (viewCell ≤ C01_VIEW_CELL_FRONT_RIGHT) are clipped/continue'd.
  D0 back cells (viewCell ≥ C02_VIEW_CELL_BACK_RIGHT) are clipped/continue'd.
  This is JOB 4 territory; noted here for the row/column boundary logic.

## Firestaff Implementation

### Wall Frame Descriptors — PASS
- `s_wall_frames[]` (line 50) contains 12 entries matching G0163_aauc_Graphic558_Frame_Walls
  (DUNVIEW.C:581–594). Frame fields: left_x, right_x, top_y, bottom_y,
  byte_width, height, blit_x, blit_y.
- D0C entry (line 71): `{ 0, 223, 0, 135, 0, 0, 0, 0 }` — byte_width=0 means
  empty slot; `dm1_viewport_3d_resolve_wall_blit_clip_gate` will return gate.visible=false
  matching DUNVIEW.C:3053–3058 empty-slot guard.

### Clip Gate Function — PASS
- `dm1_viewport_3d_resolve_wall_blit_clip_gate()` (line 945):
  1. Computes width = frame->right_x - frame->left_x + 1, height = bottom_y - top_y + 1
  2. Clips destination to viewport bounds: dst_x ∈ [0, DM1_VIEWPORT_WIDTH(224)],
     dst_y ∈ [0, DM1_VIEWPORT_HEIGHT(136)]
  3. Adjusts source offset by the dst shift (preserving source crop)
  4. Clips width/height to remaining source bounds
  5. Guards `if (width<=0 || height<=0) return gate` — matches IMAGE3.C F0684 empty-guard
  6. `gate.visible = true` only when a non-empty region remains
- Called by `dm1_viewport_3d_draw_wall()` (line 540) and
  `dm1_viewport_3d_draw_wall_opaque()` (line 574); both guard with `if (!gate.visible) return`.

### Viewport Constants — PASS
- `DM1_VIEWPORT_WIDTH = 224`, `DM1_VIEWPORT_HEIGHT = 136` (header defines)
- `DM1_VIEWPORT_BYTE_WIDTH = 224` for bitmap stride
- All frame destinations verified to be within these bounds by the clip gate.

### Source Ref Documentation — PASS
- Line 1368: `COORD.C:2390-2409 F0635 clips MEDIA720 zones and source offsets; IMAGE3.C:866-889 F0684 skips empty blits` — correctly documented.

## Verdict: PASS

Wall row/column clipping matches ReDMCSB:
1. Frame byte_width=0 → empty wall slot → gate.visible=false (no blit) ✓
2. Destination clipping to viewport 224×136 bounds ✓
3. Source offset adjusted when dst shifts off left/top edge ✓
4. Clipped width/height bounded to remaining source bitmap ✓
5. Empty-result guard prevents zero-size or negative blits ✓
6. Comment references COORD.C:2390-2409 and IMAGE3.C:866-889 ✓

No fix needed.