# Pass274 DM1 V2 D0-D3 draw-list comparator gate

Date: 2026-05-06 21:10+02:00
Worktree: `<firestaff-worktree>/firestaff-oauth-n2-dm1v2-pass274-d0d3-drawlist-comparator-gate-20260506-2055`
Base: `9d5ef16` (`worker/n2-dm1v1-evidence-capture-20260505`)
Status: `PASS_DRAW_LIST_COMPARATOR_SCAFFOLD_PIXEL_PARITY_STILL_OPEN`

## What changed

- Added renderer-side D0-D3 draw-list emission in `dm1_v2_viewport_renderer_pc34.[ch]`.
- Added an exact draw-command comparator for matched viewport states.
- Added a synthetic matched-state test covering D3/D2/D1/D0 command order and mismatch reporting.
- Added `tools/verify_dm1_v2_d0_d3_draw_list_comparator_gate.py` plus CTest/completion-matrix wiring.
- Generated `parity-evidence/verification/pass274_dm1_v2_d0_d3_draw_list_comparator_gate.json`.

## Source audit

Primary source root: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

The gate locks to `DUNVIEW.C` anchors for:

- Floor/ceiling before square traversal (`8337-8338`).
- D0-D3 traversal order from the original source window: D3L/D3R/D3C, D2L/D2R/D2C, D1L/D1R/D1C, D0L/D0R/D0C (`8490-8542`).
- Wall, stairs, door, object/creature/projectile/explosion, and field source branches (`6666`, `6697`, `6721`, `6816`, `6828`).

## What this proves

This makes the next viewport composition step machine-verifiable: for a matched synthetic viewport state, Firestaff can emit a deterministic D0-D3 renderer draw-list and compare it command-for-command.

This does **not** claim final DM1 V2 pixel parity. No matched original/ReDMCSB and Firestaff viewport captures were produced in this pass.

## Exact next step

Feed the draw-list emitter from real DM1 V2 dungeon state for the same `mapX/mapY/direction`, capture matched original/ReDMCSB and Firestaff viewport pixels, then promote this comparator scaffold to a pixel/region comparator gate.
