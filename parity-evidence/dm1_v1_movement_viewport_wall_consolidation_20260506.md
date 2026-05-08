# DM1 V1 movement + viewport/walls consolidation — 2026-05-06

Scope: N2 worker-only consolidation on branch `worker/n2-dm1v1-evidence-capture-20260505`. This note ties the recent pass206/pass207/pass212/pass223/pass224/pass227 evidence to source citations and to the runtime binding commits now present on this branch. It does **not** claim pixel parity or a successful original-game movement capture.

## ReDMCSB audit anchors used before implementation

Primary source root: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

- Movement result state is global and source-owned by `MOVESENS.C:5-14` (`G0397_i_MoveResultMapX`, `G0398_i_MoveResultMapY`, `G0399_ui_MoveResultMapIndex`, `G0400_i_MoveResultDirection`, `G0401_ui_MoveResultCell`, sensor rotation fields). Runtime/state probes must bind to those seams before claiming movement parity.
- Party/map changes during pit/teleporter movement redraw through `MOVESENS.C:505-556`: teleporter direction updates at `505-518`, group/projectile/object rotation at `520-531`, and fall redraw through `F0128_DUNGEONVIEW_Draw_CPSF(...)` at `556`.
- Viewport presentation is requested through `DRAWVIEW.C:F0097_DUNGEONVIEW_DrawViewport` at `709-723`, where the viewport request flag and vertical blank wait make post-redraw timing a source-level concern, not just a screenshot timing concern.
- Wall-set graphics are loaded and mirrored in `DUNVIEW.C:F0095_DUNGEONVIEW_LoadWallSet` at `2124-2223`; current map floor/wall-set reload decisions are at `2298-2335`.
- Master viewport draw begins in `DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF` at `8318-8357`, chooses flipped/native wall bitmaps at `8357-8418`, draws D3 side walls at `8445-8464`, then traverses D4→D0 cells at `8466-8542`, restoring native wall pointers at `8543-8582` and freeing the temporary bitmap at `8593-8603`.

## Consolidated pass evidence

- pass206: `tools/pass206_dm1_v1_original_runner_minimal_gate.py` and `parity-evidence/verification/pass206_dm1_v1_original_runner_minimal_gate/manifest.json` preserve the minimal original-runner blocker. Expected status remains `BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE`.
- pass207: `tools/pass207_dm1_v1_original_movement_viewport_blocker_gate.py` and its manifest preserve the movement/viewport route blocker. Expected status remains `BLOCKED_MOVEMENT_VIEWPORT_ROUTE_NOT_PROMOTABLE`.
- pass212: `tools/pass212_dm1_v1_original_route_sync_probe.py`, README, report, and manifest are restored on this branch. It records the route/capture synchronization blocker and keeps raw image payloads untracked.
- pass223: `tools/pass223_dm1_v1_post_redraw_instrumentation_lock.py` and report lock the post-redraw instrumentation seam against ReDMCSB viewport presentation timing.
- pass224: `tools/pass224_dm1_v1_runtime_state_probe_scaffold.py` and report scaffold the runtime-state binding without pretending the original runtime trace is complete.
- pass227: `tools/pass227_dm1_v1_original_runtime_hook_design.py` and report document the original runtime hook API contract needed before promoting original movement/viewport capture.

## Commit consolidation status

- `d14f6ca Inventory FIRES runtime binding artifacts` was already present at task start.
- Source commit `6ae10aa Salvage DM1 V1 viewport route blockers` was present on sibling branch `worker/n2-dm1v1-viewport-walls-source-lock-20260506-0237`; equivalent content was landed here as `f057fae Salvage DM1 V1 viewport route blockers`.
- Source commit `1fa6a8c Bootstrap DM1 original runtime trace map` was present on the same sibling branch; equivalent content was landed here as `116f0a7 Bootstrap DM1 original runtime trace map`.
- Missing probe/tool dependencies for pass212/pass223/pass224/pass227 were then landed as worker-only commits on this branch so CTest can exercise them locally.

## Merge-readiness conclusion

The lane is mergeable as metadata/tests/docs for blocker preservation and runtime-hook readiness: manifests and CTest entries now travel with their tools, the runtime trace map is present, and the source audit points to exact ReDMCSB functions/lines. The blocker is still runtime promotion, not source-lock coverage: pass206/pass207/pass212 intentionally remain blocker gates until a post-vblank original route produces unique gameplay frames and a verified runtime state trace.
