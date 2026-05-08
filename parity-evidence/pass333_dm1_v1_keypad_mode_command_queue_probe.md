# Pass333 — DM1 V1 keypad mode to command queue

Status: `BLOCKED_PASS333_NUMLOCK_KEYPAD_MODE_BLOCKS_I34E`

## ReDMCSB anchors
- `COMMAND.C` I34E movement key table and F0361/F0380 queue: `{'KEYBOARD_INPUT G0459_as_Graphic561_SecondaryKeyboardInput_Movement[7]': 46, 'MEDIA707_I34E_I34M': 602, 'C001_COMMAND_TURN_LEFT,     0x004B': 678, 'C003_COMMAND_MOVE_FORWARD,  0x004C': 679, 'C002_COMMAND_TURN_RIGHT,    0x004D': 680, 'void F0361_COMMAND_ProcessKeyPress': 1709, 'G2153_i_QueuedCommandsCount++': 1357, 'void F0380_COMMAND_ProcessQueue_CPSC': 2045, 'G2153_i_QueuedCommandsCount == 0': 2084, 'F0365_COMMAND_ProcessTypes1To2_TurnParty': 2151, 'F0366_COMMAND_ProcessTypes3To6_MoveParty': 2155}`
- `MOVESENS.C` movement legality/result: `{'BOOLEAN F0267_MOVE_GetMoveResult_CPSCE': 316, 'P0557_T_Thing == C0xFFFF_THING_PARTY': 433}`
- `DUNVIEW.C` F0128 redraw entry: `{'void F0128_DUNGEONVIEW_Draw_CPSF': 8318, 'F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)': 8610}`

## Runtime decision

- Hypothesis: `NumLock/keypad mode prevents I34E movement codes from entering the command queue`
- Stops: `[]`
- Blocker: `BLOCKED_PASS333_NUMLOCK_KEYPAD_MODE_BLOCKS_I34E`

Manifest: `parity-evidence/verification/pass333_dm1_v1_keypad_mode_command_queue_probe/manifest.json`
