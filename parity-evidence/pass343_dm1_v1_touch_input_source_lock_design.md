# pass343 DM1 V1 touch/click input source-lock design

Date: 2026-05-07
Branch: worker/pass343-dm1-v1-touch-input-source-lock-design-20260507
Scope: design/evidence only. No runtime behavior is changed by this pass.

## ReDMCSB source anchors audited first

Reference root audited on N2:
`/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

Exact source-lock anchors:

- `STARTUP2.C:1179-1182` installs the active in-game input tables: primary mouse = interface, secondary mouse = movement, primary keyboard = interface, secondary keyboard = movement.
- `COMMAND.C:375-395` defines `G0447_as_Graphic561_PrimaryMouseInput_Interface`, including champion status boxes, champion icons, spell-area parent, action-area parent, and freeze-game click.
- `COMMAND.C:396-405` defines `G0448_as_Graphic561_SecondaryMouseInput_Movement`: turn-left, move-forward, turn-right, move-left, move-backward, move-right, dungeon viewport click, and right-button inventory-leader toggle.
- `COMMAND.C:412-419` begins the champion-inventory primary mouse table with right-button close inventory and viewport-relative save/rest/close/music/inventory-slot entries.
- `COMMAND.C:461-511` defines action-name/action-icon subroutes, spell-area subroutes, champion name/hand subroutes, chest slots, and resurrect/reincarnate/cancel panel entries.
- `COMMAND.C:1379-1449` is the source hit-test seam: `F0358_COMMAND_GetCommandFromMouseInput_CPSC` walks the current mouse table, checks coordinate mode/zone and button mask, and returns the command id.
- `COMMAND.C:1452-1661` is the source click-to-command-queue seam: `F0359_COMMAND_ProcessClick_CPSC` records a pending click if the queue is locked; otherwise it resolves primary mouse input first, then secondary mouse input, and stores command/x/y in the queue.
- `COMMAND.C:1692-1707` replays one pending click after queue unlock through `F0360_COMMAND_ProcessPendingClick`.
- `COMMAND.C:2045-2156` is the queue processing seam: `F0380_COMMAND_ProcessQueue_CPSC` locks the queue, respects empty/movement-disabled gates, dequeues one command, replays a pending click after unlock, then routes turns to `F0365` and movement to `F0366`.
- `COMMAND.C:2296-2324` dispatches command ids relevant to this touch lane: C083 inventory toggle, C100 spell-area parent, C111 action-area parent, C070 mouth, C071 eye, and C080 dungeon-view click.
- `CLIKMENU.C:142-174` handles turn commands; `CLIKMENU.C:180-330` handles movement commands and discards queued input after blocked movement.
- `CLIKMENU.C:519-585` resolves action-area child clicks by reusing `F0358_COMMAND_GetCommandFromMouseInput_CPSC` on the action names/icons sub-tables.
- `COORD.C:1693-1722` source-locks the PC/I34E screen and viewport constants: 320x200 screen, viewport origin x=0 y=33, viewport size 224x136.
- `COORD.C:1915-1920` source-locks inclusive point-in-zone bounds.

## Existing Firestaff seams mapped

Current Firestaff files already expose a clean provider-neutral seam without needing behavior changes in this pass:

- `touch_click_zone_matrix_pc34_compat.{h,c}` is the source-backed zone matrix. It stores command id, zone index, coordinate mode, button mask, native 320x200 or viewport-local bounds, group name, and evidence text.
- `touch_pointer_input_pc34_compat.{h,c}` is the pointer/touch adapter. It accepts native-screen, scaled-screen, or viewport-local touch events and translates them to original screen coordinates plus original mouse button masks.
- `dm1_v1_input_command_queue_pc34_compat.{h,c}` is the existing queue abstraction mirroring ReDMCSB click enqueue, pending-click replay, movement disabled gate, and turn/move dispatch identity.

The important parity rule is: touch must not synthesize keyboard movement and must not bypass the ReDMCSB mouse tables. A tap becomes the same original `x/y/buttonStatus` and command id that `F0358`/`F0359` would have produced.

## Proposed touch/click seam

Recommended seam:

1. Platform event providers normalize touch/mouse input only to one of three spaces:
   - native `SCREEN_320X200`,
   - letterboxed/scaled `SCALED_SCREEN`, or
   - explicit `VIEWPORT_LOCAL` for inventory/panel overlays.
2. `TOUCHPOINTER_Compat_TranslateEvent` maps the point to original 320x200 coordinates and hit-tests the source-backed matrix.
3. `TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue` feeds `DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat` with the resolved command id, original screen coordinates, and original button mask.
4. The downstream queue and command processor remain the sole owner of movement timing, blocked-movement handling, pending-click replay, action-area child resolution, dungeon-view click dispatch, and inventory/menu side effects.

This keeps touchscreen support additive: provider-specific gesture code stops at coordinate normalization; DM1 V1 command parity remains locked to ReDMCSB mouse/click semantics.

## Non-negotiable invariants

- Native logical input space remains 320x200; viewport-local y is promoted by the PC/I34E viewport origin y=33 before queue dispatch.
- Button masks remain source-equivalent: right button `0x0001`, left button `0x0002`.
- Primary-interface mouse input is searched before secondary movement mouse input, matching `COMMAND.C:1641-1644`.
- Movement taps enqueue commands C001/C003/C002/C006/C005/C004 only through the mouse-command queue; they do not synthesize arrow keys.
- The dungeon viewport tap remains C080 and carries original click coordinates for `F0377_COMMAND_ProcessType80_ClickInDungeonView`.
- Action-area parent C111 remains a parent command; action-name/icon subroutes are resolved later by `CLIKMENU.C:519-585` from the original click coordinates.
- Inventory/panel viewport-relative zones remain viewport-local in the matrix and are promoted at the touch seam, not hard-coded in game logic.
- Queue lock/pending-click replay remains single-pending-click behavior; touch does not create a parallel queue.

## Design status

No blocker found for a touchscreen-support lane if implementation is limited to the seam above. The high-risk mistake would be adding touch-specific movement or UI shortcuts downstream of the command queue. That would break the ReDMCSB route order and should be rejected.
