# Pass381 — DM1 V1 post-input next F0128 path

Status: `BLOCKED_PASS381_NEXT_F0128_REQUIRES_WAIT_LOOP_EXIT_NOT_PROVED_BY_PASS379`

## Decision

ReDMCSB source puts the next viewport draw on the outer-loop wrap: route keys must first become queued commands, F0380 must dispatch F0365/F0366, those handlers set G0321_B_StopWaitingForPlayerInput, and the wait loop exits only when G0321 and G0301_B_GameTimeTicking are both true. pass379 retained F0128/F0097 breakpoints and delivered host route keys, but it did not prove F0380 dequeue/G0321 wait-loop exit; after 75s it still had no F0128/F0097 hit and a forced pause decoded to IMAGE_TEXT near F0683. So pass379 should be read as a semantic route/wait-loop-exit blocker, not a reason to retarget F0128/F0097 addresses.

## Source path

1. `GAMELOOP.C` draws the viewport with `F0128_DUNGEONVIEW_Draw_CPSF(...)` before the input wait loop.
2. The same outer iteration later resets `G0321_B_StopWaitingForPlayerInput = C0_FALSE` and loops through keyboard drain + `F0380_COMMAND_ProcessQueue_CPSC()`.
3. `COMMAND.C` dispatches queued turn/step commands to `F0365`/`F0366`; `CLIKMENU.C` sets `G0321_B_StopWaitingForPlayerInput = C1_TRUE` in those accepted movement handlers.
4. `GAMELOOP.C` exits the wait loop only when `G0321_B_StopWaitingForPlayerInput` and `G0301_B_GameTimeTicking` are both true; the next F0128 is on the following outer-loop draw, where F0128 calls F0097.

## Why pass379 did not stop at F0128/F0097

pass379 proves debugger control, retained breakpoints, and delivered host route-key events. It does **not** prove that those keys became a semantic queued movement command, that F0380 dispatched it, or that G0321/tick let the input wait loop exit. Its final forced pause decoded to IMAGE_TEXT near F0683, while F0128/F0097 direct-hit flags stayed false.

Manifest: `parity-evidence/verification/pass381_dm1_v1_post_input_next_f0128_path/manifest.json`
