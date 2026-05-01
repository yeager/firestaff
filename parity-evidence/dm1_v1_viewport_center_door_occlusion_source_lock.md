# DM1 V1 viewport center-door occlusion source lock

## ReDMCSB source audit

Primary source: `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C`.

- `DUNVIEW.C:4547-4582` (`F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`) documents the object/creature/projectile/explosion pass ordering used when a square body calls into the viewport thing renderer.
- `DUNVIEW.C:6721-6744` (`F0118_DUNGEONVIEW_DrawSquareD3C_CPSF`) handles `C17_ELEMENT_DOOR_FRONT` as one center-square body: floor ornament / door-pass F0115 / frame / button / `F0111_DUNGEONVIEW_DrawDoor`.
- `DUNVIEW.C:7327-7338` (`F0121_DUNGEONVIEW_DrawSquareD2C`) shows the same center-door frame/button/door work for D2C inside the D2C square body.
- `DUNVIEW.C:7873-7888` (`F0124_DUNGEONVIEW_DrawSquareD1C`) shows D1C center-door handling inside the nearest center-square body.

Implication: center-door ornaments, destroyed masks, and buttons belong to the nearest non-open center square that is actually drawn. They must not run as independent later scans that can select a farther D2/D3 door after a nearer center blocker has already occupied the view.

## Firestaff change

- Added `m11_dm1_nearest_blocking_center_door_depth` in `m11_game_view.c` to bind center-door adornments to the nearest non-open center door only.
- Rewired `m11_draw_dm1_center_door_ornaments`, `m11_draw_dm1_center_destroyed_door_masks`, and `m11_draw_dm1_center_door_buttons` to use that single nearest-door depth instead of scanning D1/D2/D3 independently.
- Rewired `m11_draw_dm1_d3r_door_button` to accept `maxVisibleForward` and `cells`, then gate the D3R side button with `m11_dm1_side_lane_wall_clear_for_rel(cells, 3, 1)` before sampling, matching the existing side-wall/side-door occlusion invariant.

## Gate

- Added `tools/verify_v1_viewport_center_door_occlusion_gate.py` and CTest `v1_viewport_center_door_occlusion_gate`.
