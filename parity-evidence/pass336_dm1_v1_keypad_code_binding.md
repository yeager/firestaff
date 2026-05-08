# Pass336 — DM1 V1 keypad-code binding after table readiness

Status: `BLOCKED_PASS336_ROUTE_INJECTION_LAYER_AFTER_READY`

## Exact I34E runtime binding
- `C001_COMMAND_TURN_LEFT` -> `0x004B` (PC extended Left arrow scancode 0x4B converted to 0x004B (K) by IO2.C:54-55; COMMAND.C:678)
- `C003_COMMAND_MOVE_FORWARD` -> `0x004C` (PC extended Up arrow scancode 0x48 converted to 0x004C (L) by IO2.C:47-50; COMMAND.C:679)
- `C002_COMMAND_TURN_RIGHT` -> `0x004D` (PC extended Right arrow scancode 0x4D converted to 0x004D (M) by IO2.C:57-58; COMMAND.C:680)
- `C006_COMMAND_MOVE_LEFT` -> `0x004F` (I34E movement table direct code 0x004F (O); COMMAND.C:681)
- `C005_COMMAND_MOVE_BACKWARD` -> `0x0050` (PC extended Down arrow scancode 0x50 converted to 0x0050 (P) by IO2.C:51-52; COMMAND.C:682)
- `C004_COMMAND_MOVE_RIGHT` -> `0x0051` (I34E movement table direct code 0x0051 (Q); COMMAND.C:683)

## Source anchors
- `INPUT.C` `F0540_INPUT_Crawcin/F0541_INPUT_WaitForMouseOrKeyboardActivity (non-I34E audit)` lines 260-283,423-425,621-642: Audited because INPUT.C contains platform input helpers, but the I34E PC34 path is selected through DEFS.H M527/M528 to IO2.C F0539/F0540, not these Amiga-family event handlers.
- `IO2.C` `F0540_INPUT_Crawcin` lines 27-61: I34E reads G2162_IODriver->IODRV_00_GetKeyboardInput; extended arrows 0x1248/0x1250/0x124B/0x124D are normalized to 0x004C/0x0050/0x004B/0x004D.
- `COMMAND.C` `G0459_as_Graphic561_SecondaryKeyboardInput_Movement` lines 636-685: I34E/I34M secondary movement table binds commands to 0x004B,0x004C,0x004D,0x004F,0x0050,0x0051.
- `COMMAND.C` `F0361_COMMAND_ProcessKeyPress` lines 1709-1813: P0728_KeyCode is compared against primary, then G0444 secondary table; matched entries queue commands and increment G2153_i_QueuedCommandsCount.
- `COMMAND.C` `F0380_COMMAND_ProcessQueue_CPSC` lines 2045-2156: I34E queue count gates processing; turn/move commands call F0365/F0366.
- `GAMELOOP.C` `game loop keyboard poll` lines 164-168,215: M527/M528 poll keyboard buffer and feed F0361 before F0380 processes the queue.
- `STARTUP2.C` `new-game dungeon input install` lines 1179-1183: G0444 is installed to G0459 movement table before ProcessNewPartyMap.
- `DEFS.H` `I34E input macros` lines 3157-3160: I34E-family builds use F0539_INPUT_Cconis/F0540_INPUT_Crawcin for M527/M528.

## Runtime distinction
- `pass335_numlock_kp_symbols`: status `BLOCKED_PASS335_TABLE_READY_BUT_KEYPAD_CODES_WRONG`, route `['one', 'click:276,140', 'one', 'numlock', 'kp5', 'kp4', 'kp6', 'kp5']`, stops `[]`
- `pass336_arrow_symbols`: status `BLOCKED_PASS335_TABLE_READY_BUT_KEYPAD_CODES_WRONG`, route `['left', 'up', 'right', 'down']`, stops `[]`
- `pass336_post_ready_toprow_control`: status `BLOCKED_PASS335_TABLE_READY_BUT_KEYPAD_CODES_WRONG`, route `['one', 'one']`, stops `[]`

Decision: route injection layer remains blocked after the dungeon table is ready. The required F0361 codes are known (`0x004B/0x004C/0x004D/0x004F/0x0050/0x0051`), but the current post-ready xdotool route is not delivering even control keys into the DOS keyboard buffer.

Manifest: `parity-evidence/verification/pass336_dm1_v1_keypad_code_binding/manifest.json`
