# Pass360 — DM1 V1 original runtime true-stop blocker narrowing

Status: `BLOCKED_PASS360_ORIGINAL_RUNTIME_TRUE_STOP_BLOCKER_NARROWED`

## Verdict

The remaining blocker is now narrower: movement routing and debugger control are not the active problem. The unresolved item is a source-bound original FIRES true-stop sequence at the candidate F0128 target, followed by F0097 or VIDRV slot 9 in the same bounded run.

## ReDMCSB source audit

- `GAMELOOP.C:80-90` — `F0002_MAIN_GameLoop_CPSDF` — F0128 is the source redraw seam that consumes the current party tuple. ok=`True`
- `GAMELOOP.C:164-219` — `F0002_MAIN_GameLoop_CPSDF` — Keyboard input is drained before F0380 queue dispatch during the wait loop. ok=`True`
- `COMMAND.C:2045-2156` — `F0380_COMMAND_ProcessQueue_CPSC` — F0380 remains the source movement command dequeue/dispatch seam. ok=`True`
- `CLIKMENU.C:142-174,180-347` — `F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty` — Accepted turns/steps mutate direction/coordinates and apply movement cooldown. ok=`True`
- `DUNVIEW.C:8336-8611` — `F0128_DUNGEONVIEW_Draw_CPSF` — F0128 composes viewport content and calls F0097 for presentation. ok=`True`
- `DRAWVIEW.C:709-858` — `F0097_DUNGEONVIEW_DrawViewport` — F0097 is the PC34 viewport-present seam through video-driver slot 9. ok=`True`
- `VIDEODRV.C:941-957,3566-3582` — `G8133_ac_Code1DispatchTable / F8161_VIDRV_09_BlitViewPort` — VIDRV slot 9 is the source viewport blit implementation. ok=`True`

## Prior runtime reconciliation

- `pass315`: `F0128_RUNTIME_HIT_VERIFIED_F0380_REMAINS_BLOCKED` (expected `F0128_RUNTIME_HIT_VERIFIED_F0380_REMAINS_BLOCKED`) — legacy F0128 claim to reclassify under strict parser ok=`True`
- `pass324`: `PASS_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE_FOUND` (expected `PASS_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE_FOUND`) — owned-PTY strict control primitive ok=`True`
- `pass326`: `BLOCKED_PASS326_DIRECT_PTY_F0128_CODE_STOP_NOT_PROVEN` (expected `BLOCKED_PASS326_DIRECT_PTY_F0128_CODE_STOP_NOT_PROVEN`) — strict F0128 target stop attempt ok=`True`
- `pass328`: `BLOCKED_PASS328_BREAKPOINT_ARMING_TIMING` (expected `BLOCKED_PASS328_BREAKPOINT_ARMING_TIMING`) — post-load arming timing attempt ok=`True`
- `pass329`: `BLOCKED_PASS329_CODE_STOP_TRANSITION_NOT_EMITTED` (expected `BLOCKED_PASS329_CODE_STOP_TRANSITION_NOT_EMITTED`) — breakpoint retention / arming timing attempt ok=`True`
- `pass330`: `BLOCKED_PASS330_CPU_NEVER_REACHES_F0128_UNDER_ROUTE` (expected `BLOCKED_PASS330_CPU_NEVER_REACHES_F0128_UNDER_ROUTE`) — route-to-target stop transition investigation ok=`True`
- `pass357`: `BLOCKED_PASS357_ORIGINAL_RUNTIME_TRUE_STOP_CONTROL_REQUIRED` (expected `BLOCKED_PASS357_ORIGINAL_RUNTIME_TRUE_STOP_CONTROL_REQUIRED`) — prior consolidated true-stop blocker ok=`True`
- `pass359`: `PASS_DM1_V1_MOVEMENT_ROUTE_RUNTIME_BLOCKER_CLASSIFIED` (expected `PASS_DM1_V1_MOVEMENT_ROUTE_RUNTIME_BLOCKER_CLASSIFIED`) — movement route retired / original true-stop active classification ok=`True`

## Latest controlled runtime attempt

- Pass330 status: `BLOCKED_PASS330_CPU_NEVER_REACHES_F0128_UNDER_ROUTE`; ran=`True`; bounded seconds=`75`
- Direct hits: `{'f0097_2809_1EFF_after_f0128': False, 'f0128_23AD_40FE': False}`; retained post-route=`True`; post-route pause code=`280C:1528`
- Blocker: `F0128 breakpoint armed/retained but no strict running-to-23AD:40FE prompt transition emitted`
- Exact next command: `python3 tools/pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py --seconds 75 && python3 tools/verify_pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py && python3 tools/verify_pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing.py`

## Blocker narrowing

Not blocked by:
- Firestaff M11 movement route (pass349/pass351/pass352/pass358/pass359)
- owned-PTY debugger control primitive (pass324)
- BPLIST/setup parser ambiguity (strictly rejected here)
- breakpoint retention alone (pass329/pass330 retain F0128 but still do not stop at it)

Still blocked by: `No bounded controlled original FIRES run produces a strict post-running code stop at F0128, followed by F0097 or VIDRV slot 9, at the current candidate CS:IP targets.`

Next unblocker: `Re-establish the live FIRES CS:IP map or an equivalent source-bound runtime locator for F0128/F0097/VIDRV, then rerun a single owned-PTY sequence that records F0128 -> F0097/VIDRV after controlled movement input.`

Next command: `python3 tools/pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py --seconds 75 && python3 tools/verify_pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py && python3 tools/verify_pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing.py`

## Non-claims

- new original FIRES F0128/F0097/VIDRV true stop
- original-vs-Firestaff pixel parity
- runtime proof from static offsets alone
- runtime proof from BPLIST/setup text
