# Pass508 DM1 V1 viewport/wall runtime-readiness evidence

Status: FAIL_PASS508_DM1_V1_VIEWPORT_WALL_RUNTIME_READINESS

## ReDMCSB anchors
- DUNVIEW.C:8466-8542 F0128_DUNGEONVIEW_Draw_CPSF status=PASS
- DUNVIEW.C:7784-7844 F0124_DUNGEONVIEW_DrawSquareD1C status=PASS
- DUNVIEW.C:4800-4926 F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF status=PASS
- DUNVIEW.C:7873-7938 F0124_DUNGEONVIEW_DrawSquareD1C status=PASS
- DRAWVIEW.C:847-858 F0097_DUNGEONVIEW_DrawViewport status=PASS

## Firestaff readiness
- src/engine/m11_game_view.c normal_renderer_batches_with_near_replay_guard status=PASS
- src/engine/m11_game_view.c wall_alcove_item_source_cell_gate status=PASS

## Secondary local references
- dm1_pc34_graphics /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT exists=True sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- dm1_pc34_dungeon /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT exists=True sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- greatstone_index /home/trv2/.openclaw/data/firestaff-greatstone-atlas/index/SUMMARY.md exists=True sha256=b8ee685a2b60a49f305d0f1423e329d5e1019382b53598510833a46840bc3e2d
- csbwin_cpp /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin/CSBwin.cpp exists=True sha256=89418e01b0a8eef330451320d19078a3510cbc699f635c8af22820365e4ceb23

## Gates
- /usr/bin/python3 tools/verify_v1_viewport_alcove_wall_item_gate.py -> rc=1 passed=False
- /usr/bin/python3 tools/verify_v1_viewport_d1c_doorpass_source_lock_gate.py -> rc=0 passed=True
- /usr/bin/python3 tools/verify_pass500_dm1_v1_viewport_walls_blocker_cleanup_source_lock.py -> rc=0 passed=True

## Blockers
- No original DOSBox same-viewport capture was produced by this pass.
- No Firestaff-vs-original pixel comparator promotion is claimed.
- Firestaff normal V1 renderer remains guarded batched replay, not exact ReDMCSB F0128 per-square replay.
