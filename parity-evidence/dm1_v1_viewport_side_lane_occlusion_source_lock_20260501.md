# DM1 V1 viewport side-lane occlusion source lock — 2026-05-01

## Change

`m11_game_view.c` now treats any nearer **non-open** side-lane square as the occluder for farther side-lane wall, door, ornament, destroyed-door-mask, content, and projectile paths. The old guard only stopped on wall/fake-wall squares, which let farther side-lane contents/projectiles bleed through a nearer closed side door.

## ReDMCSB source citations

Primary source: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C`.

- `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` signature and view-cell order semantics: `DUNVIEW.C:4547-4564`.
- `F0115` content order is objects, then creatures, then projectiles, then explosions: `DUNVIEW.C:4565-4582`.
- Projectile pass restarts from the first square thing and assigns C2900 projectile zones: `DUNVIEW.C:5679-5684`.
- `F0128_DUNGEONVIEW_Draw_CPSF` square traversal is far-to-near: D4 contents first, then D3 side/center, D2 side/center, D1 side/center, D0 side/center: `DUNVIEW.C:8466-8542`.
- The side/center call order that matters for this guard is D3L/D3R then D3C (`DUNVIEW.C:8488-8499`), D2L/D2R then D2C (`DUNVIEW.C:8510-8521`), and D1L/D1R then D1C (`DUNVIEW.C:8522-8533`).

## Firestaff integration points

- `m11_dm1_side_lane_clear_before_depth()` / `m11_dm1_side_lane_clear_for_rel()` now use `!m11_viewport_cell_is_open()` instead of `m11_viewport_cell_is_wall_like()` for nearer side-lane occluders.
- This guard feeds side wall panels, wall ornaments, side doors, side door ornaments, destroyed side-door masks, D3R door-button routing, and `m11_draw_dm1_side_contents()`.
- `m11_draw_dm1_side_contents()` remains after source wall/door panels and before center-lane content, matching the existing source-backed C2500/C2900/C3200 side-content path while preventing dirty-lane bleed through closed side doors.

## Verification

- `cmake --build build --target firestaff_m11 -j2` — passed. Existing warnings only: `snprintf` truncation in `m11_dialog_source_split_two_lines`, possible uninitialized `skillY` in `M11_GameView_Draw`.
- `ctest --test-dir build -R "m11_viewport_state|m11_game_view|m11_capture_route_state" --output-on-failure` — 3/3 passed.
