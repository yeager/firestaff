# Pass348 — DM1 V1 NumLock/keypad blocker closure audit

Status: `RECLASSIFIED_PASS348_NUMLOCK_KEYPAD_BLOCKER_NARROWED_NOT_CLOSED`

## Decision

`BLOCKED_PASS333_NUMLOCK_KEYPAD_MODE_BLOCKS_I34E` is **not fully closed** by the pass346/pass347 state at `919e1ce`. It is narrowed:

- Closed for the M11 SDL/live-route layer: NumLock-on keypad symbols `SDLK_KP_1`..`SDLK_KP_6` are accepted by `main_loop_m11.c` and routed to the same M12 movement inputs as the PC34/I34E movement table.
- Not closed as an I34E keyboard-buffer claim: the live-route bridge queues resolved DM1 V1 commands directly into the compat movement pipeline, so it does not prove `IO2.C` `F0540_INPUT_Crawcin` -> `COMMAND.C` `F0361_COMMAND_ProcessKeyPress` receives original PC keypad scancodes from an OS/DOS keyboard buffer.

Recommended blocker state: replace the broad pass333 blocker with a narrower residual note: `RESIDUAL_PASS348_I34E_DOS_KEYBOARD_BUFFER_NOT_PROVEN`. The user-visible M11 NumLock keypad route is covered; original-DOS/FIRES keyboard-buffer parity is still unproven and should not be silently claimed.

## ReDMCSB source audit

All source paths are under `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- `INPUT.C` `F0543_INPUT_DeviceInterruptHandler`, lines 298-430: audited as a non-I34E Amiga-family event handler. It marks mouse/keyboard activity and converts Amiga modifier chords into mouse button events, but it is not the I34E PC34 keyboard path used by the pass333 keypad question.
- `IO2.C` `F0540_INPUT_Crawcin`, lines 5-61: for `MEDIA463_P20JA_P20JB_I34E_I34M_P31J`, I34E reads `G2162_IODriver->IODRV_00_GetKeyboardInput`; `MEDIA707_I34E_I34M` normalizes extended arrow scancodes to `0x004C`, `0x0050`, `0x004B`, and `0x004D` before returning the keycode.
- `COMMAND.C` `G0459_as_Graphic561_SecondaryKeyboardInput_Movement`, lines 636-685: I34E/I34M binds movement commands to `0x004B`, `0x004C`, `0x004D`, `0x004F`, `0x0050`, and `0x0051`.
- `COMMAND.C` `F0361_COMMAND_ProcessKeyPress`, lines 1716-1808: keycodes are compared against primary input and then `G0444_ps_SecondaryKeyboardInput`; matching commands are queued and `G2153_i_QueuedCommandsCount` is incremented on I34E-family builds.
- `COMMAND.C` `F0380_COMMAND_ProcessQueue_CPSC`, lines 2058-2100: I34E uses `G2153_i_QueuedCommandsCount == 0` as the empty queue gate, then dispatches pending movement/turn commands when movement cooldown gates allow it.
- `CLIKMENU.C` `F0365_COMMAND_ProcessTypes1To2_TurnParty`, lines 142-174: turn commands set `G0321_B_StopWaitingForPlayerInput`, highlight the source zone, process stair turns, and update party direction.
- `CLIKMENU.C` `F0366_COMMAND_ProcessTypes3To6_MoveParty`, lines 180-330: movement commands set the waiting flag, select the movement arrow index, handle stairs, update coordinates through `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`, and block movement with `F0357_COMMAND_DiscardAllInput` on walls/doors/groups.

## Product audit at `919e1ce`

- `main_loop_m11.c` accepts script tokens `kp-1`/`kp1` through `kp-6`/`kp6` and maps them to SDL keypad symbols.
- `main_loop_m11.c` handles `SDLK_KP_5`, `SDLK_KP_2`, `SDLK_KP_1`, `SDLK_KP_3`, `SDLK_KP_4`, and `SDLK_KP_6` before WASD aliases, so NumLock-on keypad events no longer collapse to arrow-only or ignored route input.
- `m11_game_view.c` maps M12 movement inputs to DM1 V1 commands and enqueues those command ids with `DM1_V1_MovementPipeline_EnqueueCommandPc34Compat`.
- The pass345 bridge comment explicitly says: “No OS keypad/NumLock synthesis is involved.” This is good for product reliability but means pass348 cannot cite it as proof that the I34E DOS keyboard-buffer path itself is closed.

## Gates

Verifier: `tools/verify_pass348_dm1_v1_numlock_keypad_blocker_closure.py`

Manifest: `parity-evidence/verification/pass348_dm1_v1_numlock_keypad_blocker_closure/manifest.json`
