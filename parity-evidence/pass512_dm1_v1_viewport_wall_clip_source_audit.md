# Pass512 DM1 V1 viewport wall clip source audit

Status: passed

## Primary ReDMCSB evidence
- DUNVIEW.C:436-440 wall_frame_source_offsets
- DUNVIEW.C:3048-3076 transparent_and_opaque_wall_blit_routes
- DUNVIEW.C:3394-3470 f0791_source_row_clip_and_flip_adjustment
- DUNVIEW.C:8446-8542 far_to_near_wall_square_replay

## Firestaff evidence
- dm1_v1_viewport_3d_pc34_compat.c:638-677 local_clip_gate_contract
- dm1_v1_viewport_3d_pc34_compat.c:376-399 local_transparent_wall_rows_use_clip_gate
- dm1_v1_viewport_3d_pc34_compat.c:410-427 local_opaque_wall_rows_use_clip_gate
- test_dm1_v1_viewport_3d_pc34_compat.c:560-615 local_clip_tests_cover_source_and_viewport_occlusion

## Secondary references
- /home/trv2/.openclaw/data/firestaff-greatstone-atlas/index/pages.json greatstone_pc34_context_index
- /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin/Viewport.cpp csbwin_viewport_script_lineage
- /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/Viewport.cpp csb_viewport_script_lineage

## DM1 canonical anchors
- GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e bytes 363417
- DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 bytes 33357

## Non-claims
- No new original runtime screenshot was captured.
- No pixel-parity promotion is claimed.
- CSBWin/CSB are lineage references only; ReDMCSB remains primary.
