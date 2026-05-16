# Pass542 Lane B - DM1 V1 door composition occlusion source lock

Status: PASS542_LANEB_DM1_V1_DOOR_COMPOSITION_OCCLUSION_SOURCE_LOCKED

## ReDMCSB Anchors
- DUNVIEW.C:4013-4217: F0109 draws door ornaments into G0074_puc_Bitmap_Temporary.
- DUNVIEW.C:4218-4340: F0111 copies the door panel to temp, applies ornaments/masks, then blits temp to viewport.
- Copy/base ornament/Thieves Eye/blit lines: 4260, 4262, 4294, 4334.

## Firestaff Check
- src/dm1/dm1_v1_viewport_3d_pc34_compat.c source evidence carries the F0109/F0111 temporary-door composition lock.

No movement, capture, original-runtime, or pixel parity claim is made by this gate.
