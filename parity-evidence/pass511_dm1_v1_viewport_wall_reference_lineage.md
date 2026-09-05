# Pass511 DM1 V1 viewport/wall reference lineage

Status: passed

## ReDMCSB primary anchors
- DUNVIEW.C:8318-8610 f0128_far_to_near_then_present
- DUNVIEW.C:7727-7938 f0124_d1c_alcove_and_door_occlusion
- DUNVIEW.C:4547-5933 f0115_layer_order
- DRAWVIEW.C:709-858 drawview_viewport_present_boundary

## DM1 canonical anchors
- DATA/GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e bytes 363417
- DATA/DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 bytes 33357

## Blockers
- No new original same-viewport runtime capture was produced.
- No Firestaff-vs-original pixel parity promotion is claimed.
- Runtime and pixel-parity closure are tracked by their own gates rather than this source-lineage audit.
