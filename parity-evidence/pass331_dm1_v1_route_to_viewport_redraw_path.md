# Pass331 — DM1 V1 route-to-viewport-redraw path

Status: `BLOCKED_PASS331_ROUTE_KEYS_NOT_COMMAND_QUEUE`

## ReDMCSB anchors
- `COMMAND.C` F0380 command dequeue/dispatch: `{'void F0380_COMMAND_ProcessQueue_CPSC': 2045, 'G2153_i_QueuedCommandsCount == 0': 2084, 'F0365_COMMAND_ProcessTypes1To2_TurnParty': 2151, 'F0366_COMMAND_ProcessTypes3To6_MoveParty': 2155, 'void F0361_COMMAND_ProcessKeyPress': 1709, 'G0459_as_Graphic561_SecondaryKeyboardInput_Movement': 46}`
- `MOVESENS.C` movement legality/result: `{'BOOLEAN F0267_MOVE_GetMoveResult_CPSCE': 316, 'P0557_T_Thing == C0xFFFF_THING_PARTY': 433}`
- `DUNVIEW.C` F0128 redraw entry: `{'void F0128_DUNGEONVIEW_Draw_CPSF': 8318, 'F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)': 8610}`

## Runtime decision

- Hypothesis: `route keys are delivered but not mapped to DM command queue`
- Stops: `[]`
- Blocker: `BLOCKED_PASS331_ROUTE_KEYS_NOT_COMMAND_QUEUE`

Manifest: `parity-evidence/verification/pass331_dm1_v1_route_to_viewport_redraw_path/manifest.json`
