# pass792 DM1 V1 Viewport D2L2/D2R2 Side-Wall

- Status: PASS792_DM1_V1_VIEWPORT_D2L2_D2R2_SIDE_WALL_LOCKED
- Gate: F0678_DrawD2L2 (DUNVIEW.C:6837-6865) and F0679_DrawD2R2 (DUNVIEW.C:6868-6896) are the side-row (depth=2, lateral=+-2) wall dispatchers. Each handles exactly two element cases: C00_ELEMENT_WALL dispatches F0104_DrawFloorPitOrStairsBitmap with G2107_WallSet[C06_WALL_D2L2+2] / [C05_WALL_D2R2+2] (PC_FIX_CODE_SIZE) into C707_ZONE_WALL_D2L2 / C708_ZONE_WALL_D2R2 and returns; C05_ELEMENT_TELEPORTER dispatches F0113_DrawField with C09_VIEW_SQUARE_D2L2 / C10_VIEW_SQUARE_D2R2 field aspect to C707 / C708. No default case, no break after either case, no F0107/F0108/F0111/F0115 in body, no stairs/pit branch, no alcove/corridor/door route, no F0124 dispatch. F0128_DUNGEONVIEW_Draw_CPSF (DUNVIEW.C:8503-8508) dispatches F0678 at relative (2,-2) and F0679 at (2,+2), AFTER F0118 (D3C, DUNVIEW.C:8499) and BEFORE F0119 (D2L, DUNVIEW.C:8513).
- Runtime assertion floor: 225 assertions in `tests/test_dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat.c`.
- Expected test output: `assertions=225 failures=0 hash=0xb1fce2ad`.

## ReDMCSB Anchors

- DUNVIEW.C:6837-6865
- DUNVIEW.C:6868-6896
- DUNVIEW.C:8503-8508
- DEFS.H:2088
- DEFS.H:2605
- DEFS.H:2606
- DEFS.H:3428
- DEFS.H:3429
- DEFS.H:4047
- DEFS.H:4048

## Non-Overlap

- Not the d2l2_d2r2 wall-set blit gate (F0104/F0105).
- Not the d2l2_d2r2 f0107 wall-ornament alcove gate.
- Not the d2l2_d2r2 f0108 wall-composition / floor-ornament / floor-ceiling-ornament gates.
- Not the d2l2_d2r2 f0111 door-front pair blit.
- Not the d2l2_d2r2 f0115 thing pass.
- Not the d2l2_d2r2 stairs/pit dispatch.
- Not the integrated D0L2/D0R2, D1C, D1L/D1R, D2C, D2L/D2R, D3L/D3R, D3C, or CSB-lineage viewport gates.
- Contract-only; no real-asset or original-DOS pixel parity.

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass792_dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat/manifest.json`
