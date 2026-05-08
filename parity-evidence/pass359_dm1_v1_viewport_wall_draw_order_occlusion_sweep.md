# pass359 DM1 V1 viewport wall draw-order/occlusion sweep

Date: 2026-05-07
Branch: worker/fix-blockers-pass304-original-capture-20260507
Scope: verification-only follow-up for DM1 V1 viewport wall draw-order and occlusion. No runtime behavior changes.

## ReDMCSB source audit anchors

Source root audited: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- `DRAWVIEW.C:709-900` locks `F0097_DUNGEONVIEW_DrawViewport`: the final screen blit presents `G0296_puc_Bitmap_Viewport` after the dungeon-view buffer has been drawn.
- `DUNVIEW.C:2962-3003` locks `F0098_DUNGEONVIEW_DrawFloorAndCeiling`: floor/ceiling is the clean base before walls and contents replay over it.
- `DUNVIEW.C:3048-3082` locks wall/door bitmap blit helpers: wall-set blits target the viewport buffer via original zones, with separate no-transparency and door bitmap paths.
- `DUNVIEW.C:4547-4910` locks `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`: each visible square consumes encoded cell-order nibbles and rescans things per cell. This is ordered replay, not primitive depth sorting.
- `DUNVIEW.C:6400-6835` locks D3 side/center wall branches: wall squares draw wall zones and return; door-front branches split content into rear pass, door/frame pass, and front pass.
- `DUNVIEW.C:7244-7937` locks D2C/D1C center-square branches, including center wall early returns, door-front two-pass replay, floor ornament ordering, ceiling pits, and shared object/creature/projectile draw.
- `DUNVIEW.C:8318-8618` locks `F0128_DUNGEONVIEW_Draw_CPSF`: source draw order walks visible cells from far D4/D3 through near D0, then calls `F0097_DUNGEONVIEW_DrawViewport`.
- `COORD.C:1693-1724` locks the DM1 viewport origin/size: x=0, y=33, 224x136 viewport bitmap.
- `COORD.C:174-186` locks native negative-bitmap wall panel dimensions for the DM1 wall set.
- `DEFS.H:4042-4057` locks wall zone constants `C702..C717` used by DUNVIEW wall blits.

## Firestaff verification

- `m11_game_view.c:m11_dm1_max_visible_forward_from_center`, `m11_dm1_nearest_blocking_center_depth_index`, and `m11_dm1_nearest_blocking_center_door_depth` keep source-style center-line blocking helpers: nearest non-open center square stops deeper center decoration.
- `m11_game_view.c:m11_draw_dm1_front_walls` draws front wall panels with layout-696 resolved D1C/D2C/D3C zones and stops after the nearest wall-like center square.
- `m11_game_view.c:m11_draw_dm1_side_walls` draws side wall panels far-to-near with layout-696 side zones and same-lane occlusion guards.
- `m11_game_view.c:m11_draw_dm1_side_contents` keeps side-lane contents bounded by nearer closed center blockers.
- `m11_game_view.c:m11_draw_viewport` wires the normal viewport pass: sample cells, draw source-bound floor/wall/door layers, replay nearer side occluders after a farther blocking center square, then draw side and center contents.
- Existing gates are chained by the new verifier: source wall draw-order lock, viewport draw-order, wall depth, generic occlusion, side-wall occlusion, center-door occlusion, and ReDMCSB draw-stack source shape.

## Result

This pass found no new runtime defect. It makes the current viewport/wall draw-order state repeatable after pass356/pass357/pass358 by tying Firestaff’s source-bound wall/occlusion seams to exact ReDMCSB functions and re-running the relevant viewport gates as one follow-up sweep.
