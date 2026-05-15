# pass560 DM1 V1 touch UI zone source lock

Date: 2026-05-15
Branch: worker/pass560-touch-ui-zone-source-lock-20260515
Host: N2 / firestaff-worker
Scope: evidence/probe only. No movement implementation or runtime route code is changed by this pass.

## Primary evidence

Primary source root audited on N2:

/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source

The touch-screen feature lane must stay source-locked to ReDMCSB mouse/click semantics:

- COMMAND.C:375-395 defines the active primary interface mouse table: champion status boxes, champion icon cells, spell parent, action parent, and fixed freeze-game screen corner.
- COMMAND.C:396-405 defines the active secondary movement/dungeon mouse table: six movement arrows, dungeon viewport click C080, and right-button leader inventory toggle C083.
- COMMAND.C:412-451 defines inventory-mode mouse zones as screen-relative close/right-click plus viewport-relative save/rest/music/object-slot/mouth/eye controls.
- COMMAND.C:498-506 defines chest slot mouse zones; COMMAND.C:508-511 defines resurrect/reincarnate/cancel panel zones.
- COMMAND.C:1379-1449 defines F0358_COMMAND_GetCommandFromMouseInput_CPSC, the table-order hit test against X/Y/button status.
- COMMAND.C:1641-1644 defines the active route order: primary mouse first, secondary mouse only if primary returns no command.
- COMMAND.C:1651-1661 stores only command id plus original click X/Y into the command queue.
- COMMAND.C:2296-2324 dispatches UI/dungeon click commands after queue resolution: C083, C100, C111, C070, C071, and C080.
- DEFS.H:202-216 defines MOUSE_INPUT and original button masks: right 0x0001, left 0x0002, left-up 0x0004.
- DEFS.H:3748-3982 names the relevant I34E zone ids from screen, viewport, action, spell, champion, inventory, and resurrect/reincarnate/cancel groups.
- COORD.C:1693-1722 locks the PC/I34E native screen and viewport dimensions: 320x200 screen, 224x136 viewport, viewport origin x=0 y=33.
- COORD.C:1915-1921 keeps point-in-zone source bounds inclusive.

Secondary references available but not used as primary authority in this pass:

- /home/trv2/.openclaw/data/firestaff-greatstone-atlas/
- /home/trv2/.openclaw/data/firestaff-original-games/DM/
- /home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/
- /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin/
- /home/trv2/.openclaw/data/firestaff-csb-source/CSB/

## Project audit

Existing Firestaff source already has the correct feature-lane boundary:

- touch_click_zone_matrix_pc34_compat.c/.h holds the source-backed DM1 V1 click-zone matrix and source-ordered active hit test.
- touch_pointer_input_pc34_compat.c/.h normalizes touch/pointer events to original mouse X/Y/button state and enqueues through the DM1 V1 input command queue.
- test_touch_click_zone_matrix_pc34_compat_integration.c gates the 104-entry matrix, viewport rect 0,33,224,136, source-order primary/secondary behavior, and representative zone coordinates.
- test_touch_pointer_input_pc34_compat_integration.c gates the pointer seam into the command queue and pending-click replay behavior.
- probes/m11/firestaff_m11_touch_live_dispatch_gate_probe.c gates the live M11 dispatch seam used by the game view.

No project movement code was edited. The movement source remains an input-table consumer behind the original queue/dispatch path.

## Zone coverage locked

The current matrix is intentionally broad enough for touchscreen UI work while remaining table-driven:

- active screen-relative primary interface zones: champion status/toggle/bar graphs, champion icon cells, spell parent, action parent, freeze corner;
- active screen-relative secondary zones: six movement arrows, dungeon viewport click, right-button leader inventory toggle;
- action/spell child zones used after parent command dispatch;
- champion name/ready/action-hand child zones;
- inventory viewport-relative controls: close/save/rest/music, body slots, backpack slots, chest slots, mouth, eye;
- resurrect/reincarnate/cancel viewport-relative panel zones.

Representative audited coordinates already locked in tests:

| Route | Command | Zone | Coord mode | Box |
| --- | ---: | ---: | --- | --- |
| movement.turn_left | 1 | 68 | screen | x=234 y=125 w=28 h=21 |
| movement.forward | 3 | 70 | screen | x=263 y=125 w=27 h=21 |
| viewport.dungeon | 80 | 7 | screen | x=0 y=33 w=224 h=136 |
| action.parent | 111 | 11 | screen | x=233 y=77 w=87 h=45 |
| action.row0 | 113 | 82 | screen | x=234 y=86 w=85 h=11 |
| action.icon1 | 117 | 90 | screen | x=255 y=86 w=20 h=35 |
| spell.symbol1 | 101 | 245 | screen | x=235 y=51 w=13 h=11 |
| spell.cast | 108 | 252 | screen | x=234 y=63 w=70 h=11 |
| champion0.action_hand | 21 | 212 | screen | x=24 y=10 w=16 h=16 |
| inventory.ready_hand | 28 | 507 | viewport | x=6 y=53 w=16 h=16 |
| inventory.eye | 71 | 546 | viewport | x=12 y=13 w=16 h=16 |
| system.freeze_game | 147 | 0 | screen | x=0 y=198 w=2 h=2 |

## Invariants

- Touch does not synthesize keyboard movement.
- Touch does not bypass the original command queue.
- Screen-space taps search active primary interface before secondary movement, matching COMMAND.C:1641-1644.
- Viewport-local taps are promoted through source viewport origin x=0 y=33 before command queueing.
- Original button masks are preserved: right 0x0001, left 0x0002.
- Dungeon viewport tap remains command C080 with original click X/Y for F0377_COMMAND_ProcessType80_ClickInDungeonView.
- Action/spell child routes remain child routes; parent commands C111 and C100 continue to own downstream resolution.

## Verification

Self-contained source-lock verifier:

python3 tools/verify_pass560_dm1_v1_touch_ui_zone_source_lock.py

Expected marker:

PASS_DM1_V1_TOUCH_UI_ZONE_SOURCE_LOCK
