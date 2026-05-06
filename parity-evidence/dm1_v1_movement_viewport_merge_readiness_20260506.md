# DM1 V1 movement/viewport/evidence source-lock merge-readiness — 2026-05-06

Worker scope: consolidate current N2 head `c6d80ef` and recent worker commits `c475e31`, `46ecd55`, `fb2ac8e`, `89d0972`, plus pass228 runtime-trace branch head `4aeda73`. No push.

## Repository state audited

- Main worktree: `/home/trv2/work/firestaff`, branch `worker/n2-dm1v1-evidence-capture-20260505`, clean before this report.
- Current audited head before this report: `c6d80ef Source-lock DM1 V1 runtime provenance trace`.
- Required source/reference roots present:
  - `/home/trv2/.openclaw/data/firestaff-redmcsb-source/`
  - `/home/trv2/.openclaw/data/firestaff-original-games/DM/`
  - `/home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin/`
- Dirty worktrees check: none found before this report.

## Source audit summary

ReDMCSB source spot-checks were made against `ReDMCSB_WIP20210206/Toolchains/Common/Source` before running gates:

- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC` queue lock/gate/dequeue/dispatch path around lines 2075-2160.
- `CLIKMENU.C:F0365/F0366` turn, step vector, stairs, blocker, movement-result, sensor/timing path around lines 156-350.
- `DUNVIEW.C:F0115/F0128` ordered viewport traversal, per-cell object/creature/projectile passes, and after-all-cells explosion pass around lines 4790-4860, 5195-5205, 5681-5695, 5915-5935, and 8466-8542.
- `DRAWVIEW.C:F0097` viewport-request and final viewport blit path around lines 721-722 and 1056-1068.

Current head already contains the movement and viewport source-audit commits (`c475e31`, `46ecd55`, `fb2ac8e`, `89d0972`) and the compat runtime-provenance guardrail (`c6d80ef`).

## Verification run on current head

Scratch builds used `/tmp/firestaff-merge-readiness-c6d80ef-*` and `/tmp/firestaff-viewport3d-c6d80ef-*` and were removed after successful runs.

Passed:

- `cmake -S . -B <scratch> -DCMAKE_BUILD_TYPE=Release`
- `cmake --build <scratch> --target firestaff_dm1_v1_viewport_draw_order_probe test_dm1_v1_input_command_queue_pc34_compat test_dm1_v1_movement_command_core_pc34_compat test_dm1_v1_movement_pipeline_pc34_compat test_dm1_v1_movement_timing_pc34_compat test_dm1_v1_command_movement_sensor_timing_pc34_compat test_dm1_v1_viewport_3d_pc34_compat firestaff_dm1_v1_movement_core_probe firestaff_dm1_v1_walls_occlusion_blockers_probe -j2`
- `ctest --test-dir <scratch> --output-on-failure -R "^(dm1_v1_viewport_draw_order_probe|dm1_v1_input_command_queue_pc34_compat|dm1_v1_movement_command_core_pc34_compat|dm1_v1_movement_pipeline_pc34_compat|dm1_v1_movement_timing_pc34_compat|dm1_v1_command_movement_sensor_timing_pc34_compat|dm1_v1_movement_source_lock|dm1_v1_movement_command_gate_source_lock|dm1_v1_movement_timing_source_lock|dm1_v1_party_movement_sensor_order_source_lock|dm1_v1_command_movement_sensor_timing_source_lock|dm1_v1_entry_movement_viewport_source_lock|dm1_v1_stairs_pits_viewport_source_lock|dm1_v1_projectile_movement_interlock_source_lock|firestaff_dm1_v1_movement_core_probe|firestaff_dm1_v1_walls_occlusion_blockers_probe|v1_viewport_redmcsb_draw_stack_gate|v1_viewport_occlusion_gate|v1_viewport_side_wall_occlusion_gate|v1_viewport_center_door_occlusion_gate|v1_viewport_wall_depth_source_lock_gate|v1_viewport_wall_blit_transparency_gate|v1_viewport_wall_parity_flip_gate|dm1_v1_viewport_world_redmcsb_source_lock)$"` — 24/24 passed.
- `<scratch>/test_dm1_v1_viewport_3d_pc34_compat` — passed, ending with `PASS dm1_v1_viewport_3d_source_lock`.
- `git diff --check` — passed.

## Pass228 / `4aeda73` status

`4aeda73 test: gate real DM1 runtime trace provenance` is **not** contained by current head `c6d80ef`; it lives on `/home/trv2/work/firestaff-pass228-original-runtime-trace-202605060015`, branch `worker/pass228-original-runtime-trace-202605060015`, ahead of `origin/main` by 3 commits.

Verified separately on that clean pass228 worktree:

- `python3 tools/verify_dm1_original_runtime_trace_real_unresolved_gate.py` — `PASS unresolved real runtime trace blocked until symbol map seams are verified`.
- `git diff --check` — passed.

## Merge-readiness conclusion

Movement + viewport source-lock content currently on `c6d80ef` is merge-ready from the targeted gates above. The important blocker is scope, not test failure: pass228 runtime-trace gate commit `4aeda73` is parallel and absent from `c6d80ef`, so do not claim the full original runtime-trace gate is consolidated into this head until pass228 is intentionally merged/cherry-picked and re-run.

No pixel-parity/original screenshot claim is made by `c6d80ef`; its provenance trace correctly keeps `originalRuntimeObserved=0` and `noPixelParityClaim=1` guardrails.
