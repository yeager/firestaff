# Pass474 — DM1 V1 live mouse down/up C080 probe

Status: `BLOCKED_PASS474_NO_MOUSE_EVENT_SAMPLE`

no strict mouse event sample

## ReDMCSB source audit
- `IO.C` `callback_gate` ok=`True`
  - line 687: `int16_t F0781_MouseHandler`
  - line 705: `if (P2383_i_MouseEvent < C32_MOUSE_EVENT_CHANGE_SCREEN_REGION)`
  - line 706: `F0359_COMMAND_ProcessClick_CPSC(P2381_i_X, P2382_i_Y, P2383_i_MouseEvent);`
- `IBMIO.C` `driver_down_up` ok=`True`
  - line 773: `void F8096_ProcessMouseState(register int16_t P3280_i_MouseX`
  - line 804: `(*G8067_MouseHandler)(P3280_i_MouseX, P3281_i_MouseY, (P3282_i_RawButtonStatus & 1) ? C02_MOUSE_EVENT_LEFT_BUTTON_DOWN : C04_MOUSE_EVENT_LEFT_BUTTON_UP);`
  - line 809: `G8091_PreviousRawButtonStatus = P3282_i_RawButtonStatus;`
- `COMMAND.C` `c080_queue_dispatch` ok=`True`
  - line 403: `{ C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   CM1_SCREEN_RELATIVE, C007_ZONE_VIEWPORT`
  - line 1452: `void F0359_COMMAND_ProcessClick_CPSC`
  - line 1643: `L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput`
  - line 1658: `G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command`
  - line 2045: `void F0380_COMMAND_ProcessQueue_CPSC`
  - line 2323: `F0377_COMMAND_ProcessType80_ClickInDungeonView(L1161_i_CommandX, L1162_i_CommandY);`
- `CLIKVIEW.C` `f0377_front_sensor` ok=`True`
  - line 311: `void F0377_COMMAND_ProcessType80_ClickInDungeonView`
  - line 348: `P0752_i_X -= G2067_i_ViewportScreenX`
  - line 367: `C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT`
  - line 5: `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor`
- `MOVESENS.C` `c127_to_f0280` ok=`True`
  - line 1501: `case C127_SENSOR_WALL_CHAMPION_PORTRAIT`
  - line 1502: `F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)`
- `REVIVE.C` `f0280_candidate` ok=`True`
  - line 63: `void F0280_CHAMPION_AddCandidateChampionToParty`
  - line 272: `G0299_ui_CandidateChampionOrdinal`
  - line 127: `G0305_ui_PartyChampionCount`

## Runtime predicates
- sampled events: `[]` / `[]`
- stops: `['other']`
- F0359 hit: `False`; F0380 hit: `False`; F0377 hit: `False`; F0280 hit: `False`

## Artifacts
- Manifest: `parity-evidence/verification/pass474_dm1_v1_live_mouse_down_up_c080_probe/manifest.json`
- Transcript: `parity-evidence/verification/pass474_dm1_v1_live_mouse_down_up_c080_probe/pass474_dm1_v1_live_mouse_down_up_c080_probe_runtime.clean.txt`
- Command log: `parity-evidence/verification/pass474_dm1_v1_live_mouse_down_up_c080_probe/pass474_dm1_v1_live_mouse_down_up_c080_probe_command_log.json`
- Click log: `parity-evidence/verification/pass474_dm1_v1_live_mouse_down_up_c080_probe/pass474_dm1_v1_live_mouse_down_up_c080_probe_click_log.json`
