# Pass241 — DM1 V1 runtime CS:IP candidate bridge

Status: `PASS_RUNTIME_CSIP_CANDIDATES_BRIDGED_NO_PROMOTIONS`

## Result

Pass235 already captured FIRES.EXENEW runtime entry/load segment in DOSBox. Pass237 already produced static disassembly candidates tied to ReDMCSB source seams. This pass binds those two text artifacts into numeric DOSBox breakpoint candidates without promoting any runtime hit.

- Captured load segment: `01ed` from entry `01ED:0000`; PSP `01DD` confirms PSP+0x10.
- Formula: `runtime_cs = 0x01ed + static_cs`; `runtime_ip = static_ip`.
- FIRES.EXENEW sha256 guard: `fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94`.

## ReDMCSB source seam audit

- command_accepted: COMMAND.C [2045, 2156] / F0380_COMMAND_ProcessQueue_CPSC — ok=True.
- turn_handler: CLIKMENU.C [142, 179] / F0365_COMMAND_ProcessTypes1To2_TurnParty — ok=True.
- move_handler: CLIKMENU.C [180, 347] / F0366_COMMAND_ProcessTypes3To6_MoveParty — ok=True.
- move_result: MOVESENS.C [316, 850] / F0267_MOVE_GetMoveResult_CPSCE — ok=True.
- draw_uses_mutated_tuple: GAMELOOP.C [55, 95] / F0002_MAIN_GameLoop_CPSDF — ok=True.
- viewport_buffer_composed: DUNVIEW.C [8318, 8611] / F0128_DUNGEONVIEW_Draw_CPSF — ok=True.
- viewport_present: DRAWVIEW.C [709, 858] / F0097_DUNGEONVIEW_DrawViewport — ok=True.

## Candidate bridge

- command_accepted -> command_accepted: static 1b7c:06e9 => runtime candidate 1d69:06e9 (F0380_COMMAND_ProcessQueue_CPSC).
- turn_types_1_to_2 -> turn_or_step_state_applied: static 1771:010d => runtime candidate 195e:010d (F0365_COMMAND_ProcessTypes1To2_TurnParty).
- move_types_3_to_6 -> turn_or_step_state_applied: static 1771:01aa => runtime candidate 195e:01aa (F0366_COMMAND_ProcessTypes3To6_MoveParty).
- move_get_move_result -> party_coordinates_committed: static 1126:0516 => runtime candidate 1313:0516 (F0267_MOVE_GetMoveResult_CPSCE).
- viewport_game_loop_draw_call_site -> draw_uses_mutated_tuple: static 23cc:110e => runtime candidate 25b9:110e (F0002_MAIN_GameLoop_CPSDF -> F0128_DUNGEONVIEW_Draw_CPSF).

## Guardrail

No bridged candidate entry in `data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json` is promoted by this pass. These candidates are enough for a reproducible non-manual breakpoint bridge, but not enough to claim `verified_runtime_hit` until the debugger actually stops on the bridged seam with state evidence.

Evidence manifest: `parity-evidence/verification/pass241_dm1_v1_runtime_csip_candidate_bridge/manifest.json`.
