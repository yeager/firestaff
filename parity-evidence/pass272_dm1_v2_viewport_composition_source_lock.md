# Pass272 DM1 V2 viewport composition source-lock gate

Date: 2026-05-06 20:48+02:00
Worktree: `<firestaff-worktree>/firestaff-oauth-n2-dm1v2-pass272-viewport-composition-gate-20260506-2043`
Base: pass271-equivalent asset-binding head `024fcb0`
Status: `PASS_SOURCE_STACK_LOCKED_RUNTIME_PIXEL_PARITY_STILL_OPEN`

## Blocker fixed

Pass271 closed the asset-binding lane but left full DM1 V2 viewport composition unproven. This pass adds the next source-first gate for the renderer stack/order that must drive a D0-D3 viewport comparator.

## ReDMCSB source audit used first

Primary source root: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

The gate verifies source anchors in `DUNVIEW.C` for:

- `F0128_DUNGEONVIEW_Draw_CPSF` entry, floor/ceiling request handling and flipped wall setup.
- Far-to-near square traversal: D4L, D4R, D4C, then D3L/D3R/D3C, D2L/D2R/D2C, D1L/D1R/D1C, D0L/D0R/D0C.
- Element branches for wall, door front, stairs front, fields and object/creature/projectile/explosion overlay calls.
- D0/D3 close-square wall/blocker early-return behavior.

## Firestaff gate added

- Added `tools/verify_dm1_v2_viewport_composition_source_lock.py`.
- Wired CTest `dm1_v2_viewport_composition_source_lock`.
- Updated `tools/verify_dm1_v2_completion_matrix.py` so pass271 and pass272 gates are required in the DM1 V2 matrix.
- Generated `parity-evidence/verification/pass272_dm1_v2_viewport_composition_source_lock.json`.
- Regenerated `parity-evidence/verification/dm1_v2_completion_matrix.json` with the new gate present.

## What this proves

This proves the DM1 V2 viewport composition gate is now source-locked to the original DUNVIEW draw stack/order and bound to the pass271 source-evidenced dungeon-view logical assets.

It does **not** claim final DM1 V2 pixel parity. Remaining exact blockers:

1. Build/emit renderer-side D0-D3 draw-list composition from this source order.
2. Capture a matched original/ReDMCSB/Firestaff viewport state.
3. Run screenshot/region comparator before claiming final visual parity.

## Gates

- `python3 tools/verify_dm1_v2_viewport_composition_source_lock.py` — PASS
- `python3 tools/verify_dm1_v2_completion_matrix.py` — PASS
- `cmake -S . -B build-pass272 -DBUILD_TESTING=ON` — PASS
- `ctest --test-dir build-pass272 --output-on-failure -R "dm1_v2_(movement_viewport_pc34|runtime_shell_pc34|runtime_shell_source_lock|viewport_wall_occlusion_source_lock|dungeon_view_asset_bindings_source_lock|viewport_composition_source_lock|completion_matrix_gate)$"` — PASS
- `git diff --check` — expected gate
- strict changed-file credential scan — expected gate
