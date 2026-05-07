# Pass359 — DM1 V1 movement-route/runtime blocker follow-up

Status: `PASS_DM1_V1_MOVEMENT_ROUTE_RUNTIME_BLOCKER_CLASSIFIED`

## Verdict

The Firestaff M11 movement route is no longer the active blocker: pass349 proves full launcher-to-live movement, pass351/pass352 keep redraw and route regressions locked, and pass358 keeps touch source-order locked. The remaining blocker is narrower: original FIRES/DOS runtime control still needs a strict true-stop sequence from F0128 to F0097/VIDRV before any original-vs-Firestaff viewport comparator can be promoted.

## ReDMCSB source audit

- `STARTUP2.C:1179-1183` — `F0004_MAIN_ProcessEntranceCommand160_NewGame` — Dungeon runtime installs movement-capable mouse/keyboard tables before drawing the new party map. ok=`True`
- `COMMAND.C:636-685` — `G0459_as_Graphic561_SecondaryKeyboardInput_Movement` — The PC34/I34E movement key table maps normalized movement keys to C001..C006. ok=`True`
- `COMMAND.C:1641-1661` — `F0359_COMMAND_ProcessClick_CPSC` — Mouse clicks resolve primary interface first, then secondary movement, and enqueue command/X/Y in source order. ok=`True`
- `COMMAND.C:1709-1813` — `F0361_COMMAND_ProcessKeyPress` — Keyboard input is table-resolved into the same command queue used by mouse movement commands. ok=`True`
- `COMMAND.C:2045-2156` — `F0380_COMMAND_ProcessQueue_CPSC` — F0380 is the canonical command queue consumer and movement dispatcher. ok=`True`
- `CLIKMENU.C:142-174,180-347` — `F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty` — Accepted turns/steps mutate party facing/position and set the source movement cooldown. ok=`True`
- `GAMELOOP.C:80-90,164-168,215` — `F0002_MAIN_GameLoop_CPSDF` — The main loop drains input into F0361/F0380 and uses F0128 as the redraw seam for the current party tuple. ok=`True`
- `DUNVIEW.C:8336-8611` — `F0128_DUNGEONVIEW_Draw_CPSF` — F0128 composes the dungeon viewport into G0296 and then calls F0097 for presentation. ok=`True`
- `DRAWVIEW.C:709-858` — `F0097_DUNGEONVIEW_DrawViewport` — F0097 is the PC34 viewport-present seam through video-driver slot 9. ok=`True`

## Prior evidence classification

- `pass331`: `BLOCKED_PASS331_ROUTE_KEYS_NOT_COMMAND_QUEUE` (expected `BLOCKED_PASS331_ROUTE_KEYS_NOT_COMMAND_QUEUE`) — old original/DOS route-key blocker before the Firestaff launcher route was proven ok=`True`
- `pass349`: `FULL_LAUNCHER_KEYPAD_RUNTIME_ROUTE_PROVED` (expected `FULL_LAUNCHER_KEYPAD_RUNTIME_ROUTE_PROVED`) — full product launcher-to-live-movement proof for Firestaff M11 ok=`True`
- `pass351`: `PASS_DM1_V1_LIVE_VIEWPORT_REDRAW_PARITY_SWEEP` (expected `PASS_DM1_V1_LIVE_VIEWPORT_REDRAW_PARITY_SWEEP`) — source-locked live route to movement to viewport redraw sweep ok=`True`
- `pass352`: `PASS_DM1_V1_MOVEMENT_ROUTE_REGRESSION_MATRIX_CONSOLIDATED` (expected `PASS_DM1_V1_MOVEMENT_ROUTE_REGRESSION_MATRIX_CONSOLIDATED`) — route-surface regression matrix after pass349/pass356 integration ok=`True`
- `pass357`: `BLOCKED_PASS357_ORIGINAL_RUNTIME_TRUE_STOP_CONTROL_REQUIRED` (expected `BLOCKED_PASS357_ORIGINAL_RUNTIME_TRUE_STOP_CONTROL_REQUIRED`) — current original FIRES true-stop/presentation blocker ok=`True`
- `pass358`: `PASS_DM1_V1_TOUCH_SOURCE_ORDER_RUNTIME_SWEEP` (expected `PASS_DM1_V1_TOUCH_SOURCE_ORDER_RUNTIME_SWEEP`) — touch source-order/runtime route guard ok=`True`

## Classification

- Firestaff movement route: `unblocked/proved by pass349 plus pass351/pass352/pass358 guards`
- Active blocker: `original FIRES strict true-stop sequence: F0128_DUNGEONVIEW_Draw_CPSF then F0097_DUNGEONVIEW_DrawViewport or VIDRV_09_BlitViewPort in one bounded controlled run`
- Do not promote from: `BPLIST/setup echo`, `static offsets alone`, `screenshots without runtime code-stop sequence`, `Firestaff-side route proof as original DOS pixel parity`

## Non-claims

- new original DOSBox/FIRES debugger hit
- F0128->F0097 true-stop transcript
- original-vs-Firestaff pixel parity
