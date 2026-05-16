# Pass586 rebase-readiness manifest

Status: BLOCKED

Branch: worker/pass586-rebase-readiness-manifest-20260516-codex
Worktree: /home/trv2/work/firestaff-worktrees/pass586-rebase-readiness-manifest-20260516-codex
Base: origin/main f280fc75692fa6b8dabb9fd370521c9ec911a3fc
Generated: 2026-05-16

## Scope

This manifest verifies the clean/mergeable branch set recoverable from the pass584 integration-sweeper reflog. The assignment asked for 20 branches, but pass584 persisted reflog exposes 18 unique branch names. I did not invent the missing two.

For each recoverable branch I fetched origin, checked git diff --check origin/main...branch, ran git merge-tree --write-tree origin/main branch, and attempted a rebase only on a scratch branch/worktree.

## ReDMCSB Audit Anchors

Source root: /home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source

- DUNVIEW.C:6781 selects F0124_DUNGEONVIEW_DrawSquareD1C as the D1C draw handler.
- DUNVIEW.C:7727 starts F0124_DUNGEONVIEW_DrawSquareD1C.
- DUNVIEW.C:7784 enters the D1C wall case; DUNVIEW.C:7789-7820 handles Thieves Eye visible-area save/composite.
- DUNVIEW.C:7834 draws the D1C wall set to C712_ZONE_WALL_D1C; DUNVIEW.C:7842-7843 gates alcove/order drawing; DUNVIEW.C:7866-7872 restores/releases Thieves Eye state before door-front handling.
- DUNVIEW.C:8318 starts F0128_DUNGEONVIEW_Draw_CPSF; DUNVIEW.C:8533 calls the D1C draw routine from the full viewport draw.
- MOVESENS.C:316 starts F0267_MOVE_GetMoveResult_CPSCE; MOVESENS.C:556 redraws the dungeon view after pit traversal.
- INPUT.C:642, INPUT.C:664, INPUT.C:682, and INPUT.C:701 route mouse/button transitions through F0359_COMMAND_ProcessClick_CPSC.

## Pass584 Candidate Recovery

Command source: git reflog --date=iso worker/pass584-integration-sweeper-20260516-bosse

Recovered unique branches:
- dm1v1-turn-step-timing-gate-20260515-2345
- worker-dm1v1-move-source-lock-20260515-221813
- worker/dm1-v1-front-wall-viewport-gate
- worker/dm1v1-blocked-movement-collision-gate-20260515
- worker/dm1v1-movement-pipeline-gate-20260515-bosse
- worker/dm1v1-movement-stairs-backstep-cooldown-pass578-20260515-codex
- worker/dm1v1-original-overlay-capture-20260515-f195
- worker/dm1v1-viewport-d3-d2-wallorn-order-pass581-codex
- worker/pass-side-wall-occlusion-source-row-clipping-20260515-2345
- worker/pass-wall-projectile-zone-gate-20260515-2345
- worker/pass563-manifest-refresh-origin-main-20260515-2246
- worker/pass570-dm1v1-d2c-front-order-source-lock-20260515-bosse
- worker/pass575-dm1v1-front-wall-thieves-eye-source-lock-20260515-codex
- worker/pass575-landing-readiness-manifest-20260515-2345
- worker/pass577-dm1v1-creature-projectile-order-20260515-codex
- worker/pass583-action-area-click-mapping
- worker/projectile-explosion-row-clip-pass582-b27520b8
- worker/touch-ui-source-lock-probe

## Branch Results

### dm1v1-turn-step-timing-gate-20260515-2345

- status: REBASE_READY
- commit: 12b69bd899953435fd170281a4844e61d9f6a752
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,dm1_v1_movement_timing_pc34_compat.c dm1_v1_movement_timing_pc34_compat.h,test_dm1_v1_turn_step_timing_gate_pc34_compat.c tools/verify_dm1_v1_turn_step_timing_gate_source_lock.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: 12b69bd899953435fd170281a4844e61d9f6a752
- rebase excerpt: Current branch scratch/pass586-rebase-dm1v1-turn-step-timing-gate-20260515-2345 is up to date.

### worker-dm1v1-move-source-lock-20260515-221813

- status: REBASE_READY
- commit: bd3feb4265489c11b2499efe39a1763fee3efc97
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,parity-evidence/pass580_dm1_v1_forward_collision_timing.md parity-evidence/verification/pass580_dm1_v1_forward_collision_timing/manifest.json,tools/verify_pass580_dm1_v1_forward_collision_timing.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: bd3feb4265489c11b2499efe39a1763fee3efc97
- rebase excerpt: Current branch scratch/pass586-rebase-worker-dm1v1-move-source-lock-20260515-221813 is up to date.

### worker/dm1-v1-front-wall-viewport-gate

- status: REBASE_READY
- commit: a047f522af27c7a3ddccc7a0da5d3169f9323815
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,dm1_v1_viewport_3d_pc34_compat.c dm1_v1_viewport_3d_pc34_compat.h,parity-evidence/verification/pass573_dm1_v1_front_wall_front_cell_viewport_gate.json test_dm1_v1_viewport_3d_pc34_compat.c,tools/verify_dm1_v1_front_wall_front_cell_viewport_gate.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: a047f522af27c7a3ddccc7a0da5d3169f9323815
- rebase excerpt: Current branch scratch/pass586-rebase-worker_dm1-v1-front-wall-viewport-gate is up to date.

### worker/dm1v1-blocked-movement-collision-gate-20260515

- status: REBASE_READY
- commit: 69e52589cb7e44ee4143589fbff533ad592395a2
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,parity-evidence/verification/dm1_v1_blocked_forward_collision_discard_source_lock.json parity-evidence/verification/dm1_v1_blocked_forward_collision_discard_source_lock.md,test_dm1_v1_blocked_forward_collision_discard_pc34_compat.c tools/verify_dm1_v1_blocked_forward_collision_discard_source_lock.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: 69e52589cb7e44ee4143589fbff533ad592395a2
- rebase excerpt: Current branch scratch/pass586-rebase-worker_dm1v1-blocked-movement-collision-gate-20260515 is up to date.

### worker/dm1v1-movement-pipeline-gate-20260515-bosse

- status: REBASE_READY
- commit: 441cdf06a006bb9ca4527a52d4db0f9dbbfef4c9
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,parity-evidence/pass571_dm1_v1_front_command_queue_gate_blockers.md parity-evidence/verification/pass571_dm1_v1_front_command_queue_gate_blockers/manifest.json,tools/verify_pass571_dm1_v1_front_command_queue_gate_blockers.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: 441cdf06a006bb9ca4527a52d4db0f9dbbfef4c9
- rebase excerpt: Current branch scratch/pass586-rebase-worker_dm1v1-movement-pipeline-gate-20260515-bosse is up to date.

### worker/dm1v1-movement-stairs-backstep-cooldown-pass578-20260515-codex

- status: REBASE_READY
- commit: 126e97c58275f1d758de2c5a60b7dad864b0c3ec
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,parity-evidence/pass578_dm1_v1_stairs_backstep_cooldown_gate.md parity-evidence/verification/pass578_dm1_v1_stairs_backstep_cooldown_gate/manifest.json,test_dm1_v1_movement_pipeline_pc34_compat.c tools/verify_pass578_dm1_v1_stairs_backstep_cooldown_gate.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: 126e97c58275f1d758de2c5a60b7dad864b0c3ec
- rebase excerpt: Current branch scratch/pass586-rebase-worker_dm1v1-movement-stairs-backstep-cooldown-pass578-20260515-codex is up to date.

### worker/dm1v1-original-overlay-capture-20260515-f195

- status: REBASE_READY
- commit: a8f50a085df54be4852cd7f1bbd7efff64702180
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: .gitignore,parity-evidence/pass566_dm1_v1_original_overlay_capture_readiness.md parity-evidence/verification/pass566_dm1_v1_original_overlay_capture_readiness/manifest.json,scripts/dosbox_dm1_original_viewport_reference_capture.sh scripts/verify_pass566_dm1_v1_original_overlay_capture_readiness.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: a8f50a085df54be4852cd7f1bbd7efff64702180
- rebase excerpt: Current branch scratch/pass586-rebase-worker_dm1v1-original-overlay-capture-20260515-f195 is up to date.

### worker/dm1v1-viewport-d3-d2-wallorn-order-pass581-codex

- status: REBASE_READY
- commit: 1cb8549d7e8b53df879b39b07fb929d0e9098433
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,tools/verify_pass581_dm1_v1_d3_d2_wall_ornament_order_source_lock.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: 1cb8549d7e8b53df879b39b07fb929d0e9098433
- rebase excerpt: Current branch scratch/pass586-rebase-worker_dm1v1-viewport-d3-d2-wallorn-order-pass581-codex is up to date.

### worker/pass-side-wall-occlusion-source-row-clipping-20260515-2345

- status: REBASE_READY
- commit: 0ec378d627f1535e8fd8e4784d4d82d79b5d54fb
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,parity-evidence/pass576_dm1_v1_side_wall_occlusion_source_row_clipping.md parity-evidence/verification/pass576_dm1_v1_side_wall_occlusion_source_row_clipping/manifest.json,tools/verify_pass576_dm1_v1_side_wall_occlusion_source_row_clipping.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: 0ec378d627f1535e8fd8e4784d4d82d79b5d54fb
- rebase excerpt: Current branch scratch/pass586-rebase-worker_pass-side-wall-occlusion-source-row-clipping-20260515-2345 is up to date.

### worker/pass-wall-projectile-zone-gate-20260515-2345

- status: REBASE_READY
- commit: c933d2210f269368860cc369e5851e61a97e585e
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,dm1_v1_viewport_3d_pc34_compat.c dm1_v1_viewport_3d_pc34_compat.h,parity-evidence/pass566_dm1_v1_projectile_wall_zone_gate.md parity-evidence/verification/pass566_dm1_v1_projectile_wall_zone_gate/manifest.json,test_dm1_v1_viewport_3d_pc34_compat.c tools/verify_dm1_v1_projectile_wall_zone_gate.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: c933d2210f269368860cc369e5851e61a97e585e
- rebase excerpt: Current branch scratch/pass586-rebase-worker_pass-wall-projectile-zone-gate-20260515-2345 is up to date.

### worker/pass563-manifest-refresh-origin-main-20260515-2246

- status: REBASE_READY
- commit: 289673381f89ff624d0f1871b5496c0ee9541431
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: parity-evidence/verification/pass563_dm1_v1_pc34_original_c254_boundary/manifest.json
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: 289673381f89ff624d0f1871b5496c0ee9541431
- rebase excerpt: Current branch scratch/pass586-rebase-worker_pass563-manifest-refresh-origin-main-20260515-2246 is up to date.

### worker/pass570-dm1v1-d2c-front-order-source-lock-20260515-bosse

- status: REBASE_READY
- commit: c962bce3b546f2a87c266afbfcecccdd144ea7d9
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,dm1_v1_viewport_3d_pc34_compat.c dm1_v1_viewport_3d_pc34_compat.h,parity-evidence/pass570_dm1_v1_d2c_front_order_source_lock.md parity-evidence/verification/pass570_dm1_v1_d2c_front_order_source_lock/manifest.json,test_dm1_v1_viewport_3d_pc34_compat.c tools/verify_pass570_dm1_v1_d2c_front_order_source_lock.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: c962bce3b546f2a87c266afbfcecccdd144ea7d9
- rebase excerpt: Current branch scratch/pass586-rebase-worker_pass570-dm1v1-d2c-front-order-source-lock-20260515-bosse is up to date.

### worker/pass575-dm1v1-front-wall-thieves-eye-source-lock-20260515-codex

- status: REBASE_READY
- commit: a3ea6a4997235f8f5ac7ec8ffbdba5161ff31019
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,dm1_v1_viewport_3d_pc34_compat.c parity-evidence/pass575_dm1_v1_d1c_thieves_eye_front_wall_source_lock.md,parity-evidence/verification/pass575_dm1_v1_d1c_thieves_eye_front_wall_source_lock/manifest.json test_dm1_v1_viewport_3d_pc34_compat.c,tools/verify_pass575_dm1_v1_d1c_thieves_eye_front_wall_source_lock.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: a3ea6a4997235f8f5ac7ec8ffbdba5161ff31019
- rebase excerpt: Current branch scratch/pass586-rebase-worker_pass575-dm1v1-front-wall-thieves-eye-source-lock-20260515-codex is up to date.

### worker/pass575-landing-readiness-manifest-20260515-2345

- status: REBASE_READY
- commit: 698d398f96a878451688fe5eb68d6d6c9f11b326
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: parity-evidence/landing-readiness/pass575_dm1_v1_d1c_thieves_eye_front_wall_20260515.md
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: 698d398f96a878451688fe5eb68d6d6c9f11b326
- rebase excerpt: Current branch scratch/pass586-rebase-worker_pass575-landing-readiness-manifest-20260515-2345 is up to date.

### worker/pass577-dm1v1-creature-projectile-order-20260515-codex

- status: REBASE_READY
- commit: f481675ee58a57d8d3889d6fd336704a6f777f45
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,parity-evidence/pass577_dm1_v1_creature_projectile_order_source_lock.md parity-evidence/verification/pass577_dm1_v1_creature_projectile_order_source_lock/manifest.json,tools/verify_pass577_dm1_v1_creature_projectile_order_source_lock.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: f481675ee58a57d8d3889d6fd336704a6f777f45
- rebase excerpt: Current branch scratch/pass586-rebase-worker_pass577-dm1v1-creature-projectile-order-20260515-codex is up to date.

### worker/pass583-action-area-click-mapping

- status: REBASE_READY
- commit: 4f983325aae3dd7da432f1a291b607f097cdc011
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,action_area_routes_pc34_compat.c action_area_routes_pc34_compat.h,docs/dm1_v1_touchscreen_source_lock_manifest.md parity-evidence/pass_touch_click_zone_matrix.md,test_action_area_routes_pc34_compat_integration.c
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: 4f983325aae3dd7da432f1a291b607f097cdc011
- rebase excerpt: Current branch scratch/pass586-rebase-worker_pass583-action-area-click-mapping is up to date.

### worker/projectile-explosion-row-clip-pass582-b27520b8

- status: REBASE_READY
- commit: 6e2bea390c678124a6fb75127c848e989ed59591
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: CMakeLists.txt,parity-evidence/pass582_dm1_v1_explosion_viewport_clip_source_lock.md parity-evidence/verification/pass582_dm1_v1_explosion_viewport_clip_source_lock/manifest.json,tools/verify_pass582_dm1_v1_explosion_viewport_clip_source_lock.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: 6e2bea390c678124a6fb75127c848e989ed59591
- rebase excerpt: Current branch scratch/pass586-rebase-worker_projectile-explosion-row-clip-pass582-b27520b8 is up to date.

### worker/touch-ui-source-lock-probe

- status: REBASE_READY
- commit: 5992579d91e931c7d4494eaad588d9b228d0ed5f
- ahead/behind vs origin/main: 0	1 (left=origin/main, right=branch)
- merge-base: f280fc75692fa6b8dabb9fd370521c9ec911a3fc; base state: already_based_on_current_origin_main
- changed files sample: parity-evidence/verification/touch_movement_viewport_source_lock.json,tools/verify_touch_movement_viewport_source_lock.py
- git diff --check origin/main...branch: exit 0
- git merge-tree --write-tree origin/main branch: exit 0
- scratch rebase onto origin/main: exit 0; scratch head: 5992579d91e931c7d4494eaad588d9b228d0ed5f
- rebase excerpt: Current branch scratch/pass586-rebase-worker_touch-ui-source-lock-probe is up to date.

## Verification

- git fetch origin --prune: run before branch checks.
- git diff --check origin/main...branch: run for each recovered pass584 branch.
- git merge-tree --write-tree origin/main branch: run for each recovered pass584 branch.
- Scratch rebase worktrees root: /home/trv2/work/firestaff-worktrees/pass586-rebase-readiness-scratch.
- No shared worker branches were rewritten; only scratch/pass586-rebase-* branches were created/updated.

## Blockers

- Assignment requested 20 pass584-identified branches; pass584 persisted reflog exposes 18 unique branch names. Missing two names are not recoverable from the branch state I found.
