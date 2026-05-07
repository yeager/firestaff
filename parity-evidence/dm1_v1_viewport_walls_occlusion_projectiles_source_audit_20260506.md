# DM1 V1 viewport walls/occlusion/projectiles ReDMCSB audit — 2026-05-06

Scope: worker-only source audit for the N2 DM1 V1 viewport lane.  This is a
source-lock note, not a new pixel-parity claim.

Primary source:
`<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C`

## Source findings

- Master viewport draw order is `F0128_DUNGEONVIEW_Draw_CPSF`: D4 object lanes
  first (`DUNVIEW.C:8466-8477`), then D3 (`DUNVIEW.C:8490-8499`), D2
  (`DUNVIEW.C:8512-8521`), D1 (`DUNVIEW.C:8524-8533`), and D0
  (`DUNVIEW.C:8536-8542`).  Firestaff already exposes this through
  `dm1_v1_viewport_3d_pc34_compat.c:s_draw_order` and locks it in
  `test_dm1_v1_viewport_3d_pc34_compat.c:test_redmcsb_f0128_draw_order`.

- Thing draw stack is `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`.
  ReDMCSB initializes the deferred thing state at `DUNVIEW.C:4790-4791`, scans
  objects at `DUNVIEW.C:4820`, defers groups/projectiles/explosions at
  `DUNVIEW.C:4840-4848`, draws creatures at `DUNVIEW.C:5201-5246`, draws
  projectiles at `DUNVIEW.C:5646-5693`, and runs the explosion pass at
  `DUNVIEW.C:5917-5933` with explosion bitmap work continuing through
  `DUNVIEW.C:6138-6199`.  Firestaff already locks the visible layer contract in
  `dm1_v1_viewport_3d_pc34_compat.c:s_thing_layers`,
  `test_dm1_v1_viewport_3d_pc34_compat.c:test_f0115_cell_order_and_layer_z_order`,
  and `tools/verify_v1_viewport_redmcsb_draw_stack_gate.py`.

- Wall blockers/occlusion are explicit early returns from the square draw
  functions.  Side D1 blockers draw the wall/ornament and return:
  `F0122_DUNGEONVIEW_DrawSquareD1L` uses wall blits at `DUNVIEW.C:7445-7455`,
  side ornament at `DUNVIEW.C:7459`, then `return` at `DUNVIEW.C:7460`;
  `F0123_DUNGEONVIEW_DrawSquareD1R` mirrors this at `DUNVIEW.C:7613-7623`,
  `DUNVIEW.C:7627`, and `DUNVIEW.C:7628`.  Center D1 walls draw the front wall
  at `DUNVIEW.C:7833-7840`; only front-wall alcoves branch into `F0115` at
  `DUNVIEW.C:7842-7844`, otherwise the wall case returns at `DUNVIEW.C:7872`.
  Firestaff already captures this as `s_wall_draw_specs` and the occlusion fields
  in `test_dm1_v1_viewport_3d_pc34_compat.c:test_pc34_wall_bitmap_selection`.

- Door/open-square ordering around blockers matches the same source family: D1L
  door fronts draw floor ornament + first F0115 pass at `DUNVIEW.C:7493-7494`,
  draw the door frame/door at `DUNVIEW.C:7496-7506`, then run the second thing
  pass at `DUNVIEW.C:7535-7536`; D1R mirrors at `DUNVIEW.C:7661-7674` and
  `DUNVIEW.C:7703-7704`; D1C door front starts at `DUNVIEW.C:7873-7879` after
  the front-wall blocker/alcove case.

## Worker conclusion

No renderer code change was warranted in this pass: the existing Firestaff
source-lock surfaces already cover the ReDMCSB draw order, wall blocker returns,
front-alcove exceptions, and projectile/explosion deferral.  The useful landable
change is this audit note tying the current N2 lane directly to exact ReDMCSB
file/function/line evidence.
