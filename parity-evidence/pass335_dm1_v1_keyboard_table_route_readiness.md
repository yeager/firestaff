# Pass335 — DM1 V1 keyboard-table route readiness

Status: `BLOCKED_PASS335_TABLE_READY_BUT_KEYPAD_CODES_WRONG`

## ReDMCSB anchors
- `COMMAND.C` movement table definition and I34E key-code mapping: `{'KEYBOARD_INPUT* G0444_ps_SecondaryKeyboardInput;': 25, 'KEYBOARD_INPUT G0459_as_Graphic561_SecondaryKeyboardInput_Movement[7]': 636, 'MEDIA707_I34E_I34M': 677, 'C001_COMMAND_TURN_LEFT,     0x004B': 678, 'C003_COMMAND_MOVE_FORWARD,  0x004C': 679, 'C002_COMMAND_TURN_RIGHT,    0x004D': 680, 'C006_COMMAND_MOVE_LEFT,     0x004F': 681, 'C005_COMMAND_MOVE_BACKWARD, 0x0050': 682, 'C004_COMMAND_MOVE_RIGHT,    0x0051': 683}`
- `STARTUP2.C` new-game dungeon input install: `{'G0443_ps_PrimaryKeyboardInput = G0458_as_Graphic561_PrimaryKeyboardInput_Interface;': 1181, 'G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;': 1182, 'F0003_MAIN_ProcessNewPartyMap_CPSE(G0309_i_PartyMapIndex);': 1183}`
- `PANEL.C` panel close returns to dungeon movement input: `{'G0442_ps_SecondaryMouseInput = G0448_as_Graphic561_SecondaryMouseInput_Movement;': 2348, 'G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;': 2349, 'F0357_COMMAND_DiscardAllInput();': 2141}`
- `COMMAND.C` keyboard processing path into F0361/F0380: `{'void F0361_COMMAND_ProcessKeyPress': 1709, 'G0443_ps_PrimaryKeyboardInput': 24, 'G0444_ps_SecondaryKeyboardInput': 25, 'G2153_i_QueuedCommandsCount++': 1357, 'void F0380_COMMAND_ProcessQueue_CPSC': 2045, 'G2153_i_QueuedCommandsCount == 0': 2084, 'F0365_COMMAND_ProcessTypes1To2_TurnParty': 2151, 'F0366_COMMAND_ProcessTypes3To6_MoveParty': 2155}`
- `GAMELOOP.C` runtime key poll and queue process loop: `{'F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());': 167, 'F0380_COMMAND_ProcessQueue_CPSC();': 215}`

## Runtime decision

- Pointer check before route keys: `{'ok': True, 'rawBytes': ['F4', '26', '23', '2C'], 'nearOffsetHex': '26F4', 'farSegmentHex': '2C23', 'displayQuirk': 'first byte high nibble elided in dosbox-debug data overview'}`
- Expected movement table offset: `26F4`
- Stops: `[]`
- Blocker: `BLOCKED_PASS335_TABLE_READY_BUT_KEYPAD_CODES_WRONG`

Completion matrix: unchanged; evidence narrows the existing DM1 V1 primary blocker but does not change the 52% score.

Manifest: `parity-evidence/verification/pass335_dm1_v1_keyboard_table_route_readiness/manifest.json`
