# Pass510 DM1 V1 viewport wall parity flip source lock

Status: PASS510_DM1_V1_VIEWPORT_WALL_PARITY_FLIP_SOURCE_LOCKED

## ReDMCSB anchors
- DUNVIEW.C:183-200 redmcsb_wallset_native_slots status=PASS
- DUNVIEW.C:2427-2443 redmcsb_flipped_wallset_pair_table status=PASS
- DUNVIEW.C:8354-8414 redmcsb_f0128_flip_select_and_swap status=PASS
- DUNVIEW.C:8543-8579 redmcsb_f0128_restore_native_wallset status=PASS
- DUNVIEW.C:6697-6714 redmcsb_center_walls_use_flip_flag status=PASS

## Firestaff anchors
- m11_game_view.c:9072 firestaff_party_tuple_flip_predicate status=PASS
- m11_game_view.c:9127 firestaff_wallset_variant_binding_before_draw status=PASS
- m11_game_view.c:9688 firestaff_center_wall_flip_path status=PASS
- m11_game_view.c:10254 firestaff_side_wall_lr_swap_path status=PASS

## Local references
- dm1_pc34_graphics /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT exists=True sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- dm1_pc34_dungeon /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT exists=True sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- greatstone_index /home/trv2/.openclaw/data/firestaff-greatstone-atlas/index/SUMMARY.md exists=True sha256=b8ee685a2b60a49f305d0f1423e329d5e1019382b53598510833a46840bc3e2d
- csbwin_source /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin/CSBwin.cpp exists=True sha256=89418e01b0a8eef330451320d19078a3510cbc699f635c8af22820365e4ceb23
- csb_lineage_source /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/CSBwin.cpp exists=True sha256=8842aa9caa32cfab0873576bc19ddf9aaced2decc2493d8a58f0d67a4a871cdb

## Gates
- /usr/bin/python3 tools/verify_pass509_dm1_v1_wallset_startup_binding.py -> rc=0 passed=True
- /usr/bin/python3 tools/verify_pass508_dm1_v1_viewport_wall_runtime_readiness.py -> rc=0 passed=True

## Scope
- Locks the source-visible wall parity/native flip path after pass509 startup binding.
- Does not claim original-vs-Firestaff pixel parity or a new DOSBox runtime capture.
- Does not touch movement/pass435 ownership.
