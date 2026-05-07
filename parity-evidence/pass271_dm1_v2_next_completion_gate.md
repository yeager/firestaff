# Pass271 DM1 V2 next completion gate: source-evidenced dungeon-view asset bindings

Date: 2026-05-06 20:07+02:00
Worktree: `<firestaff-worktree>/firestaff-oauth-n2-dm1v2-pass271-asset-binding-20260506-2007`
Base: pass267 completion matrix commit `9a2c036`
Scope: pass267 DM1 V2 asset-binding lane. This pass does **not** claim DM1 V2 is complete.

## Gate selected

Pass267 listed three highest-payoff follow-ups: input/runtime transcript parity, viewport composition, and asset binding. I picked the smallest verified gate with concrete payoff: promote the shared dungeon-view logical IDs for walls/floor/ceiling/door/stairs from catalog placeholders to manifest entries with ReDMCSB source evidence.

## Source audit used before binding

Primary source root: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

Bound source anchors:

- `DEFS.H:2348-2350`: PC34/I34E floor/ceiling set constants `M650_GRAPHIC_FLOOR_SET_0_FLOOR` and `M651_GRAPHIC_FLOOR_SET_0_CEILING`.
- `DEFS.H:2351-2373`: PC34/I34E wall-set graphics, including center and side D0-D3 wall constants.
- `DEFS.H:2374-2377`: stairs/door-set range boundary and `M633_GRAPHIC_FIRST_DOOR_SET`.
- `DEFS.H:2430-2458`: stairs graphic count and down-stairs front/side bitmap ordinals.
- `DUNVIEW.C:2037-2054`: `F0094_DUNGEONVIEW_LoadFloorSet` loads paired floor/ceiling bitmaps.
- `DUNVIEW.C:2962-3002`: `F0098_DUNGEONVIEW_DrawFloorAndCeiling` draws the 224-wide ceiling/floor bands.
- `DUNVIEW.C:3048-3130`: wall, door, and stairs bitmap draw helper routes.
- `DUNVIEW.C:4218-4304`: `F0111_DUNGEONVIEW_DrawDoor` closed/destroyed door draw route.
- `DUNVIEW.C:8418-8515`: flipped wall/floor/ceiling setup and D3 square draw-order composition route.

## Changes

Added `assets-v2/manifests/firestaff-v2-dungeon-view-source-bound.manifest.json` with source-evidenced entries for:

1. `fs.v2.dungeon-view.wall.front-source`
2. `fs.v2.dungeon-view.wall.side-source`
3. `fs.v2.dungeon-view.floor.base-source`
4. `fs.v2.dungeon-view.ceiling.base-source`
5. `fs.v2.dungeon-view.door.wood.closed-source`
6. `fs.v2.dungeon-view.stairs.down-source`

Updated `assets-v2/catalog/logical-ids/shared-v2-logical-ids.json` so these shared logical IDs now bind to those manifest entries:

- `fs.v2.shared.dungeon-view.wall.front`
- `fs.v2.shared.dungeon-view.wall.side`
- `fs.v2.shared.dungeon-view.floor.base`
- `fs.v2.shared.dungeon-view.ceiling.base`
- `fs.v2.shared.dungeon-view.door.wood.closed`
- `fs.v2.shared.dungeon-view.stairs.down`

Added `tools/verify_dm1_v2_dungeon_view_asset_bindings.py` and wired CTest `dm1_v2_dungeon_view_asset_bindings_source_lock`.

Generated verification: `parity-evidence/verification/pass271_dm1_v2_dungeon_view_asset_bindings.json`.

## Completion impact

Closed one pass267 unknown lane: dungeon-view asset bindings for core walls/floor/ceiling/door/stairs are now source-evidenced manifest bindings instead of bare planned/layout placeholders.

Still blocked / next exact step:

- Full DM1 V2 viewport composition remains unproven: no D0-D3 wall/door/stairs/floor/ceiling screenshot/capture comparator yet.
- Final rebuilt art acceptance remains unproven; these bindings are source contracts, not production-art parity proof.
- Next best pass: use these bound IDs as the asset side of a D0-D3 viewport composition matrix and compare against original/runtime or ReDMCSB-derived captures.

## Gates

Run/expected on N2:

- `python3 tools/verify_dm1_v2_dungeon_view_asset_bindings.py`
- `python3 scripts/validate_v2_logical_catalog.py` requiring source-evidenced manifest bindings for the six logical IDs above.
- `cmake -S . -B /tmp/firestaff-pass271-dm1v2-asset-gate -DCMAKE_BUILD_TYPE=Release`
- `ctest --test-dir /tmp/firestaff-pass271-dm1v2-asset-gate -R "dm1_v2_dungeon_view_asset_bindings_source_lock|dm1_v2_completion_matrix_gate" --output-on-failure`
- `git diff --check`
- strict changed-file credential scan
