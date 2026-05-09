# Pass464 — DM1 V1 mouse callback event branch

Status: `BLOCKED_PASS464_F0781_EVENT_SAMPLE_UNSTABLE`

F0781 branch-region was reached, but the debugger event sample is not stable enough to prove either the change-screen-region skip or the F0359 call path

## ReDMCSB source audit

- `IO.C` `game_callback_to_f0359` ok=`True` missing=`[]`
  - line 687: `int16_t F0781_MouseHandler`
  - line 705: `if (P2383_i_MouseEvent < C32_MOUSE_EVENT_CHANGE_SCREEN_REGION)`
  - line 706: `F0359_COMMAND_ProcessClick_CPSC(P2381_i_X, P2382_i_Y, P2383_i_MouseEvent);`
  - line 1407: `(*(G2161_IODriver->IODRV_02_SetMouseHandler))(F0781_MouseHandler);`
- `IBMIO.C` `pc_driver_status_to_callback` ok=`True` missing=`[]`
  - line 773: `void F8096_ProcessMouseState(register int16_t P3280_i_MouseX`
  - line 793: `G8040_CurrentMouseX = P3280_i_MouseX;`
  - line 804: `(*G8067_MouseHandler)(P3280_i_MouseX, P3281_i_MouseY, (P3282_i_RawButtonStatus & 1) ? C02_MOUSE_EVENT_LEFT_BUTTON_DOWN : C04_MOUSE_EVENT_LEFT_BUTTON_UP);`
  - line 807: `(*G8067_MouseHandler)(P3280_i_MouseX, P3281_i_MouseY, (P3282_i_RawButtonStatus & 2) ? C01_MOUSE_EVENT_RIGHT_BUTTON_DOWN : C08_MOUSE_EVENT_RIGHT_BUTTON_UP);`
  - line 881: `void F8101_SetMouseHandler(int16_t (*P3283_pfi_)())`
  - line 888: `G8067_MouseHandler = P3283_pfi_;`
  - line 2381: `(char*)F8101_SetMouseHandler, /*  2 */`
- `COMMAND.C` `f0359_queue_entry` ok=`True` missing=`[]`
  - line 1452: `void F0359_COMMAND_ProcessClick_CPSC`
  - line 1641: `L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput`
  - line 1643: `L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput`
  - line 1658: `G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command`

## Runtime

- Method: fresh-entry original runtime; arm F0781 mouse callback and F0359 click queue entry; drive client-relative movement-panel clicks; strict stops require (Running)->prompt
- Route input after arming: `True`
- Stops: `['F0781_ConditionalAfterCmp', 'other']`
- Retained at arm: `{'F0781_MouseHandler': True, 'F0781_EventCmp': True, 'F0781_ConditionalAfterCmp': False, 'F0359_COMMAND_ProcessClick_CPSC': True}`
- Retained final: `{'F0781_MouseHandler': False, 'F0781_EventCmp': False, 'F0781_ConditionalAfterCmp': False, 'F0359_COMMAND_ProcessClick_CPSC': False}`

## Conclusion

- F0781 branch-region was reached, but the debugger event sample is not stable enough to prove either the change-screen-region skip or the F0359 call path

## Artifacts

- Manifest: `parity-evidence/verification/pass464_dm1_v1_mouse_callback_event_branch/manifest.json`
- Transcript: `parity-evidence/verification/pass464_dm1_v1_mouse_callback_event_branch/pass464_dm1_v1_mouse_callback_event_branch_runtime.clean.txt`
- Route keylog: `parity-evidence/verification/pass464_dm1_v1_mouse_callback_event_branch/pass464_dm1_v1_mouse_callback_event_branch_route_keylog.json`
- Command log: `parity-evidence/verification/pass464_dm1_v1_mouse_callback_event_branch/pass464_dm1_v1_mouse_callback_event_branch_command_log.json`
