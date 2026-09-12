# Pass510 DM1 V1 viewport wall parity flip source lock

Status: PASS510_DM1_V1_VIEWPORT_WALL_PARITY_FLIP_SOURCE_LOCKED

## ReDMCSB anchors
- DUNVIEW.C:183-200 redmcsb_wallset_native_slots status=PASS
- DUNVIEW.C:2427-2443 redmcsb_flipped_wallset_pair_table status=PASS
- DUNVIEW.C:8354-8414 redmcsb_f0128_flip_select_and_swap status=PASS
- DUNVIEW.C:8543-8579 redmcsb_f0128_restore_native_wallset status=PASS
- DUNVIEW.C:6697-6714 redmcsb_center_walls_use_flip_flag status=PASS

## Firestaff anchors
- m11_game_view.c:42289 firestaff_party_tuple_flip_predicate scope=whole-file-local-evidence status=PASS
- dm1_v1_viewport_3d_pc34_compat.c:284 firestaff_party_tuple_flip_predicate_contract scope=whole-file-local-evidence status=PASS
- m11_game_view.c:42295 firestaff_wallset_variant_binding_before_draw scope=whole-file-local-evidence status=PASS
- m11_game_view.c:43316 firestaff_center_wall_flip_path scope=whole-file-local-evidence status=PASS
- dm1_v1_viewport_3d_pc34_compat.c:1247 firestaff_side_wall_lr_swap_path scope=whole-file-local-evidence status=PASS
- m11_game_view.c:45196 firestaff_side_wall_receipt_dispatch scope=whole-file-local-evidence status=PASS

## Verification
- <local-home>/Documents/Codex/2026-08-24/jobba-med-github-com-yeager-firestaff/work/firestaff-incomplete-20260824/.codex-scratch/release-build/test_dm1_v1_viewport_3d_pc34_compat -> rc=0 passed=True

## Scope
- Locks the source-visible wall parity/native flip path after pass509 startup binding.
- Does not claim original-vs-Firestaff pixel parity or a new DOSBox runtime capture.
- Does not touch movement/pass435 ownership.
