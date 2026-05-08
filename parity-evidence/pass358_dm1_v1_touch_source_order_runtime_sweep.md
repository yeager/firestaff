# pass358 DM1 V1 touch source-order/runtime sweep

Date: 2026-05-07
Branch: worker/fix-blockers-pass304-original-capture-20260507
Scope: verification-only sweep for the active DM1 V1 touch/click route. No runtime behavior changes.

## ReDMCSB source audit anchors

Source root audited: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- `STARTUP2.C:1179-1182` installs `G0441_ps_PrimaryMouseInput` from `G0447_as_Graphic561_PrimaryMouseInput_Interface` before `G0442_ps_SecondaryMouseInput` from `G0448_as_Graphic561_SecondaryMouseInput_Movement`.
- `COMMAND.C:375-405` covers the active primary-interface plus secondary-movement route block.
- `COMMAND.C:375-395` defines the I34E primary interface mouse table: champion status boxes, champion icons, spell parent, action parent, and freeze corner.
- `COMMAND.C:396-405` defines the secondary movement table: six movement arrows, `C080_COMMAND_CLICK_IN_DUNGEON_VIEW`, and right-button `C083_COMMAND_TOGGLE_INVENTORY_LEADER`.
- `COMMAND.C:412-451` defines the inventory table using `CM1_SCREEN_RELATIVE` for close/right-click and `CM2_VIEWPORT_RELATIVE` for save/rest/slot/mouth/eye/backpack routes.
- `COMMAND.C:498-511` defines chest slots and resurrect/reincarnate/cancel panel routes as `CM2_VIEWPORT_RELATIVE`.
- `COMMAND.C:1379-1449` defines `F0358_COMMAND_GetCommandFromMouseInput_CPSC`, which scans mouse input records in table order and matches coordinate bounds plus source button masks.
- `COMMAND.C:1641-1661` covers active click route order plus command/X/Y queueing.
- `COMMAND.C:1641-1644` defines active click route order in `F0359_COMMAND_ProcessClick_CPSC`: primary first, secondary only if primary returns no command.
- `COMMAND.C:1651-1661` queues the resolved command with the original click X/Y.
- `COMMAND.C:1692-1707` defines pending click replay through `F0360_COMMAND_ProcessPendingClick`.
- `COMMAND.C:2296-2324` dispatches `C083`, `C100`, `C111`, `C070`, `C071`, `C080`, and `C081` from the live command pipeline.
- `CLIKMENU.C:519-585` keeps action-area child resolution behind the source `C111` parent route.
- `COORD.C:1693-1698` fixes the I34E viewport origin to `x=0`, `y=33`.
- `DEFS.H:202-215` defines `MOUSE_INPUT` and the original mouse button masks: right `0x0001`, left `0x0002`.
- `DEFS.H:3979-3982` maps I34E panel zones `M664/M665/M666` to layout zones `570/571/573`.

## Firestaff verification

- `touch_click_zone_matrix_pc34_compat.c` already exposes `TOUCHCLICK_Compat_HitTestPrimaryThenSecondary`, source-ordered primary-interface then secondary-movement hit testing, viewport-local dispatch promotion, and scaled-screen dispatch promotion.
- `touch_pointer_input_pc34_compat.c` already routes screen and scaled taps through `TOUCHCLICK_Compat_HitTestPrimaryThenSecondary`; viewport-local events go through `TOUCHCLICK_Compat_MapViewportLocalPointToDispatch` and are queued as original screen coordinates.
- `test_touch_click_zone_matrix_pc34_compat_integration.c` locks overlap cases: primary status left/right beats child/raw matrix ordering, movement fallback, viewport fallback, right-button inventory toggle, viewport-local inventory/panel routes, and scaled-screen dispatch.
- `test_touch_pointer_input_pc34_compat_integration.c` locks the same route through pointer events and the DM1 V1 input command queue.
- `probes/m11/firestaff_m11_touch_live_dispatch_gate_probe.c` exercises the live M11 seam: touch event to source-ordered hit-test to command queue/pending replay.

## Result

This pass found no code defect requiring a route change. The landable change is a verifier/evidence sweep that makes pass358 repeatable and ties pass347/pass350 behavior to exact ReDMCSB anchors.
