# pass347 touch input primary/secondary source-order abstraction

Date: 2026-05-07
Branch base: worker/fix-blockers-pass304-original-capture-20260507 at 1840517
Worktree: /home/trv2/work/firestaff-worktrees/pass347-touch-input-source-lock-20260507
Scope: separate touchscreen feature lane; no push.

## ReDMCSB source audit anchors

Primary evidence was audited from:
`/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

- `STARTUP2.C:1179-1182` installs active in-game input tables: `G0441_ps_PrimaryMouseInput` = interface, `G0442_ps_SecondaryMouseInput` = movement, with keyboard tables separate.
- `COMMAND.C:375-395` defines `G0447_as_Graphic561_PrimaryMouseInput_Interface`, including champion status boxes, bar graph toggles, champion icon regions, spell-area parent, and action-area parent.
- `COMMAND.C:396-405` defines `G0448_as_Graphic561_SecondaryMouseInput_Movement`, including six movement arrows, dungeon viewport click, and right-button inventory-leader toggle over `C002_ZONE_SCREEN`.
- `COMMAND.C:1379-1449` defines `F0358_COMMAND_GetCommandFromMouseInput_CPSC`; it walks a `MOUSE_INPUT` table in order, checks coordinate bounds and button masks, and returns the first matching command.
- `COMMAND.C:1641-1644` defines the active click route order in `F0359_COMMAND_ProcessClick_CPSC`: primary mouse input first, then secondary mouse input only if no primary command matched.
- `COMMAND.C:1651-1661` stores resolved command id plus original click X/Y into the command queue.
- `COMMAND.C:1692-1707` replays one pending click via `F0360_COMMAND_ProcessPendingClick` after queue unlock.
- `COORD.C:1693-1722` source-locks PC/I34E 320x200 screen and viewport origin/extent (`x=0`, `y=33`, `w=224`, `h=136`).
- `COORD.C:1915-1921` defines inclusive point-in-zone semantics used by runtime layout zones.
- `DEFS.H:197-211` defines `KEYBOARD_INPUT` and `MOUSE_INPUT`; `DEFS.H:213-216` defines mouse button masks (`0x0001` right, `0x0002` left, `0x0004` left-up).

## Firestaff change

- Added `TOUCHCLICK_Compat_HitTestPrimaryThenSecondary`, a source-ordered active in-game hit-test helper over the existing source-backed zone matrix.
- Updated `TOUCHPOINTER_Compat_TranslateEvent` and scaled-screen dispatch to use primary-interface before secondary-movement routing for screen-space taps.
- Kept viewport-local inventory/panel routes explicit; they still promote through the viewport-origin seam and do not pretend to be active interface/movement routes.
- Extended `test_touch_click_zone_matrix_pc34_compat_integration.c` and `test_touch_pointer_input_pc34_compat_integration.c` to lock left/right overlap on champion status boxes, movement fallback, dungeon viewport fallback, and source right-click inventory-leader behavior.

## Parity notes

- Touch does not synthesize keyboard movement.
- Original button masks remain `0x0001` right and `0x0002` left.
- Original click coordinates are preserved into dispatch/queue results.
- Overlapping zones now use the ReDMCSB `F0359` order instead of raw matrix insertion order for screen-space touch dispatch.

## Gates

Recorded in final pass report. No raw source dumps are embedded here.
