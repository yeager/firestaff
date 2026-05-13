# Pass504 DM1 V2 viewport D1C door occlusion source lock

Date: 2026-05-13
Scope: DM1 V2 viewport/walls/occlusion evidence only. This pass does not touch
movement core and does not claim pixel parity.

## ReDMCSB audit

Primary source root:
~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/.

Exact anchors:

- DUNVIEW.C:7727 F0124_DUNGEONVIEW_DrawSquareD1C
- DUNVIEW.C:7873 case C17_ELEMENT_DOOR_FRONT
- DUNVIEW.C:7874 D1C floor ornament draw
- DUNVIEW.C:7875 pass 1 object/creature/projectile/explosion draw with C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT
- DUNVIEW.C:7877-7899 D1C door-frame top/left/right draw
- DUNVIEW.C:7901-7902 optional D1C door button draw
- DUNVIEW.C:7908 D1C door draw
- DUNVIEW.C:7910 pass 2 order C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT
- DUNVIEW.C:7937 pass 2 object/creature/projectile/explosion draw

## Firestaff evidence

Added tools/verify_dm1_v2_viewport_d1c_door_occlusion_source_lock.py and CTest
dm1_v2_viewport_d1c_door_occlusion_source_lock.

The gate verifies the ReDMCSB line anchors, the source-order window, and the V2
draw-list contract in dm1_v2_viewport_renderer_pc34.c:

- DM1_V2_VIEW_SQUARE_D1C + DM1_V2_ELEMENT_DOOR_FRONT
- floor ornament: DUNVIEW.C:7873-7874
- pass 1 objects behind the frame: DUNVIEW.C:7875
- door frame/button/door: DUNVIEW.C:7877-7910
- pass 2 objects in front of the frame: DUNVIEW.C:7910-7937

## Remaining blocker

This is source-lock and draw-list evidence. A same-state original/Firestaff
viewport capture and pixel/region comparator is still required before claiming
DM1 V2 D1C door visual parity.
