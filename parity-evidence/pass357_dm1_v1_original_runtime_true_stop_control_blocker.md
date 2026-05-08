# Pass357 — DM1 V1 original-runtime true-stop/control blocker

Status: `BLOCKED_PASS357_ORIGINAL_RUNTIME_TRUE_STOP_CONTROL_REQUIRED`

## Verdict

The ReDMCSB order is source-locked, and Firestaff-side live dispatch is already proven by pass350, but the original-runtime side still lacks the strict true-stop sequence needed to promote `viewport_present` or any original-vs-Firestaff viewport/walls comparator.

## ReDMCSB source audit

- `GAMELOOP.C:80-90` — `F0002_MAIN_GameLoop_CPSDF` — F0128 is the original source seam that consumes direction/mapX/mapY for a viewport redraw. ok=`True`
- `GAMELOOP.C:164-219` — `F0002_MAIN_GameLoop_CPSDF` — Input is drained before F0380 command queue processing in the wait loop. ok=`True`
- `COMMAND.C:2045-2156` — `F0380_COMMAND_ProcessQueue_CPSC` — F0380 is the canonical original dequeue/dispatch seam for turn/move commands. ok=`True`
- `DUNVIEW.C:8336-8611` — `F0128_DUNGEONVIEW_Draw_CPSF` — F0128 composes viewport/walls into G0296 and then calls F0097 for presentation. ok=`True`
- `DRAWVIEW.C:709-858` — `F0097_DUNGEONVIEW_DrawViewport` — For the PC34/I34E path, F0097 resolves C007 and invokes video-driver slot 9 with G0296. ok=`True`
- `VIDEODRV.C:941-957,3566-3582` — `G8133_ac_Code1DispatchTable / F8161_VIDRV_09_BlitViewPort` — Slot 9 is the PC VGA viewport blit implementation; a live VIDRV stop after F0128 would be the presentation proof. ok=`True`

## Prior debugger/control evidence

- `pass318`: `BLOCKED_F0097_AFTER_F0128_WINDOW_NO_HIT` (expected `BLOCKED_F0097_AFTER_F0128_WINDOW_NO_HIT`) ok=`True`
- `pass320`: `BLOCKED_F0128_GATE_NOT_RECAPTURED_STRICT_STOP_FILTER` (expected `BLOCKED_F0128_GATE_NOT_RECAPTURED_STRICT_STOP_FILTER`) ok=`True`
- `pass321`: `BLOCKED_PASS321_MISSING_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE` (expected `BLOCKED_PASS321_MISSING_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE`) ok=`True`
- `pass328`: `BLOCKED_PASS328_BREAKPOINT_ARMING_TIMING` (expected `BLOCKED_PASS328_BREAKPOINT_ARMING_TIMING`) ok=`True`
- `pass350`: `PASS_DM1_V1_TOUCH_LIVE_DISPATCH_GATE` (expected `PASS_DM1_V1_TOUCH_LIVE_DISPATCH_GATE`) ok=`True`

## Promotion rule

Do not promote the original viewport comparator from screenshots, BPLIST/setup echoes, or static offsets. Promote only after one bounded original FIRES route records:
- original FIRES true code stop at F0128_DUNGEONVIEW_Draw_CPSF after controlled movement/turn input
- same bounded run continues and reaches F0097_DUNGEONVIEW_DrawViewport or VIDRV_09_BlitViewPort after that F0128 stop
- transcript/parser marks stops from runtime code lines only, never BP command echoes, BPLIST output, setup collisions, or stale tmux pane text

## Comparator decision

- Firestaff-side live dispatch: `PASS_DM1_V1_TOUCH_LIVE_DISPATCH_GATE`
- Original runtime viewport-present: `missing strict F0128 -> F0097/VIDRV true-stop sequence`
- Promote viewport comparator: `False`

## Next unblocker

Implement a reliable debugger control path that can arm/retain breakpoints after post-load readiness, stop at 23AD:40FE from runtime execution, continue, and then stop at 2809:1E31 or 2809:1EFF in the same controlled original FIRES route.
