# pass350 DM1 V1 touch live dispatch gate

Date: 2026-05-07
Branch: worker/fix-blockers-pass304-original-capture-20260507
Scope: live M11/game-view touch dispatch integration gate for pass347 source-order routing. No push.

## ReDMCSB source audit anchors

Audited from `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source` before implementation:

- `STARTUP2.C:1179-1182` installs active in-game primary interface mouse input before secondary movement mouse input; keyboard tables are separate.
- `COMMAND.C:375-405` defines the primary interface and secondary movement mouse tables, including champion status boxes, movement arrows, dungeon viewport, screen-wide right-click inventory-leader toggle, and source button masks.
- `COMMAND.C:1379-1449` (`F0358_COMMAND_GetCommandFromMouseInput_CPSC`) walks mouse rows in table order and only hits when the source button mask and coordinate bounds match.
- `COMMAND.C:1641-1661` (`F0359_COMMAND_ProcessClick_CPSC`) checks primary first, falls back to secondary only on no command, then queues command plus original click X/Y.
- `COMMAND.C:1693-1707` (`F0360_COMMAND_ProcessPendingClick`) replays one pending click using stored X/Y/button status.
- `COORD.C:1693-1722` locks PC/I34E screen and viewport dimensions (`320x200`, viewport `x=0 y=33 w=224 h=136`).
- `COORD.C:1915-1921` (`F0798_COMMAND_IsPointInZone`) defines inclusive runtime point-in-zone checks.
- `DEFS.H:197-216` defines `MOUSE_INPUT` and original masks: right `0x0001`, left `0x0002`, left-up `0x0004`.

## Firestaff gate

Added `firestaff_m11_touch_live_dispatch_gate_probe` and CTest `pass350_dm1_v1_touch_live_dispatch_gate`.

The probe runs the live touch-pointer seam used by M11 game-view dispatch:

`TouchPointerEvent -> TOUCHPOINTER_Compat_TranslateEvent -> TOUCHCLICK_Compat_HitTestPrimaryThenSecondary -> DM1_V1_InputCommandQueue`

It proves:

- primary-before-secondary routing on an overlapping champion status-box tap where raw smallest-zone matching would choose a child hand route, but source order must produce primary command `C012` / zone `151`;
- original right/left masks (`0x0001`, `0x0002`) are preserved in dispatch and pending-click state;
- original screen coordinates are preserved for direct 320x200 touches and normalized scaled touches;
- secondary movement and secondary right-click inventory-leader fallback still work after primary miss;
- locked-queue pending replay stores and replays source command/X/Y/button state.

## Manifest

Structured manifest: `parity-evidence/verification/pass350_dm1_v1_touch_live_dispatch_gate/manifest.json`.

## Gates

Required verifier status: `PASS_DM1_V1_TOUCH_LIVE_DISPATCH_GATE`.

Expected probe markers:

- `primaryBeforeSecondaryClickRoutingOk=1`
- `originalButtonMasksAndCoordinatesOk=1`
- `pass350TouchLiveDispatchGateOk=1`
