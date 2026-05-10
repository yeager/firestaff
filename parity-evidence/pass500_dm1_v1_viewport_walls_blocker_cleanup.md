# Pass500 — DM1 V1 movement/viewport/walls blocker cleanup

Status: `PASS500_SOURCE_LOCK_ADDED_ORIGINAL_CAPTURE_DECISION_REMAINS`

## Mandatory ReDMCSB audit before implementation

Audited local ReDMCSB source at `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/` before changing Firestaff.

Movement/control anchors:
- `COMMAND.C:F0361_COMMAND_ProcessKeyPress` lines 1709-1956 and `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC` lines 2045-2829: command queue/input dispatch boundary.
- `CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty` lines 142-174 and `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty` lines 180-352: turn/step semantics; blocked steps do not change square.
- `DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` lines 1371-1392: relative movement coordinate update.
- `MOVESENS.C` lines 316-843 and 1309-1503: movement sensors and champion portrait wall-sensor handling.

Viewport/wall anchors:
- `DUNVIEW.C:F0100/F0101/F0102/F0765` lines 3048-3180: wall/door blit and opaque occlusion routes.
- `DUNVIEW.C:F0791_DUNGEONVIEW_DrawBitmapXX` lines 3394-3472: source-zone clipping for field contents.
- `DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` lines 4547-5885: items, creatures, projectiles/explosions cell-order layering.
- `DUNVIEW.C:F0116/F0117/F0118` lines 6361-6832 and `F0678/F0679/F0122-F0127` lines 6837-8308: D3/D2/D1/D0 wall branches and returns.
- `DUNVIEW.C:F0124_DUNGEONVIEW_DrawSquareD1C` lines 7873-7938: front-door two-pass content occlusion.
- `DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF` lines 8318-8543: far-to-near viewport replay.
- `DRAWVIEW.C:F0097_DUNGEONVIEW_DrawViewport` lines 709-858: final viewport buffer present.
- `COORD.C` lines 1693-1724 and 1748-1749: PC viewport origin/size and portrait geometry constants.

## Implemented cleanup

Added `tools/verify_pass500_dm1_v1_viewport_walls_blocker_cleanup_source_lock.py` and CTest `pass500_dm1_v1_viewport_walls_blocker_cleanup_source_lock`.

The gate source-locks the wall/viewport blocker-cleanup lane to ReDMCSB draw order, wall/door blit routes, field-content clipping, front-door occlusion ordering, final present, and canonical DM1 V1 data anchors.

## Blocker inventory decision

### Already resolved / superseded at current HEAD
- `pass398_runtime_redraw_blocker.md` — runtime redraw chain proven; verified with `tools/verify_pass398_runtime_redraw_blocker.py` and movement/viewport CTests.
- `pass404_dm1_v1_side_contents_center_blocker_occlusion_gate.md` — side contents center-wall/door occlusion proven; verified with `tools/verify_pass404_dm1_v1_side_contents_center_blocker_occlusion_gate.py` plus Pass500 source lock.
- `pass452_dm1_v1_hall_original_route_state_blocker.md` — stale wrong-facing route superseded by corrected initial-south rerun; verified by manifest status and Pass487/498 follow-ups.
- `pass454_dm1_v1_hall_original_input_mapping_blocker.md` — stale coordinate mapping superseded by corrected rerun; verified by manifest status and Pass487/498 follow-ups.
- `pass207_dm1_v1_original_movement_viewport_blocker_gate.md` — explicitly superseded by later original-capture blocker chain (`pass304`, `pass487`, `pass497`, `pass498`).
- `pass348_dm1_v1_numlock_keypad_blocker_closure.md` — old NumLock/keypad hypothesis narrowed; not the current movement/viewport blocker.
- `pass359_dm1_v1_movement_route_runtime_blocker_followup.md` — classified by later F0365/F0366 gates (`pass475`, current branch `pass495` static boundary) and Pass500 viewport source lock.
- all `dm1_all_graphics_phase*_blocker*` addenda in this sweep are historical classification docs, not current movement/viewport/walls blockers.

### Concrete fixes / gates added now
- Added Pass500 source-lock gate for `pass404`/viewport-wall blocker cleanup and for the wall/viewport half of the `pass357/pass360/pass487` family.
- No ReDMCSB-backed product-code change was made: current product code already has the wall/viewport source gates and pass499 runtime-evidence boundary; the missing part is original DOSBox state-delta/capture proof.

### Require Daniel decision or a dedicated original-capture worker
- `pass487_dm1_v1_original_click_capture_blocker.md`, `pass497_dm1_v1_original_capture_next_blocker.md`, `pass498_dm1_v1_original_post_command_state_delta_boundary.md`: choose whether source/probe/runtime evidence is enough for merge, or require a new N2 DOSBox-debug owned-PTY capture proving post-command state deltas.
- `pass357_dm1_v1_original_runtime_true_stop_control_blocker.md` and `pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing.md`: require live FIRES CS:IP map or equivalent source-bound runtime locator, unless Daniel accepts the static/runtime boundary gates as sufficient.
- `pass304_dm1_v1_original_viewport_capture_blocker_manifest.md`, `pass378_dm1_v1_original_route_semantic_clean_blocker.md`, `pass377_dm1_v1_paired_diff_artifact_blocker.md`: require semantic-clean original route/crops before paired pixel diffs can be promoted.
- `pass242_dm1_v1_dunview_tcc_int6_blocker.md`: do not spend time rebuilding DUNVIEW/TCC unless Daniel wants source-compilation parity; current Pass500 uses source text plus Firestaff gates instead.
- `pass62_v1_title_dosbox_capture_blocker.md` and `pass70_original_dm1_route_crops_blocker_probe.md`: original capture/route issues outside this movement/viewport/walls cleanup lane.
- `blocker-n2-csb-sample-save-search-20260430.md` and CSB/DM2/V2 blocker manifests: outside Daniel's DM1 V1 priority lane.

## Gate

- `python3 tools/verify_pass500_dm1_v1_viewport_walls_blocker_cleanup_source_lock.py`
- `python3 scripts/verify_pass398_runtime_redraw_blocker.py`
- `python3 tools/verify_pass404_dm1_v1_side_contents_center_blocker_occlusion_gate.py`
- `python3 tools/verify_pass499_dm1_v1_wall_occlusion_runtime_evidence_gate.py`
- `ctest --test-dir build-pass500 -R 'pass500_dm1_v1_viewport_walls_blocker_cleanup_source_lock|pass404_dm1_v1_side_contents_center_blocker_occlusion_gate|pass499_dm1_v1_wall_occlusion_runtime_evidence_gate' --output-on-failure`
