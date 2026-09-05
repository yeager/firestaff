# Pass512 DM1 V1 viewport wall clip source audit

Status: passed

## Primary ReDMCSB evidence
- DUNVIEW.C:436-440 wall_frame_source_offsets
- DUNVIEW.C:3048-3076 transparent_and_opaque_wall_blit_routes
- DUNVIEW.C:3394-3470 f0791_source_row_clip_and_flip_adjustment
- DUNVIEW.C:8446-8542 far_to_near_wall_square_replay

## Firestaff evidence
- dm1_v1_viewport_3d_pc34_compat.c:2594-2640 local_clip_gate_contract
- dm1_v1_viewport_3d_pc34_compat.c:1622-1654 local_transparent_wall_rows_use_clip_gate
- dm1_v1_viewport_3d_pc34_compat.c:1656-1685 local_opaque_wall_rows_use_clip_gate
- test_dm1_v1_viewport_3d_pc34_compat.c:2183-2285 local_clip_tests_cover_source_and_viewport_occlusion

## DM1 canonical anchors
- DATA/GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e bytes 363417
- DATA/DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 bytes 33357

## Non-claims
- No new original runtime screenshot was captured.
- No pixel-parity promotion is claimed.
