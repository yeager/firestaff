# Pass394 — DM1 V1 dispatch to movement state

Status: `PASS394_DISPATCH_TO_MOVEMENT_STATE_PROVEN`

## ReDMCSB-first audit
- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC` — `[2045, 2829]`
- `CLIKMENU.C:F0364_COMMAND_TakeStairs` — `[124, 140]`
- `CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty` — `[142, 174]`
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty` — `[180, 352]`
- `DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` — `[1371, 1421]`
- `CHAMPION.C:F0284_CHAMPION_SetPartyDirection` — `[93, 178]`
- `CHAMPION.C:F0310_CHAMPION_GetMovementTicks` — `[1180, 1215]`
- `MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE` — `[316, 1000]`
- `GAMELOOP.C` — `F0128 viewport draw + G0321/G0301 wait loop audited`
- `DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF` — `redraws from current party direction/map coordinates`

## Proven state effects
- `F0365` turn: stop-wait true, current-square sensor leave/enter, party/champion direction mutation; stairs path mutates map/direction via `F0364`.
- `F0366` accepted step: stop-wait true, living champion stamina decrement, relative coordinate calculation, blocker predicates before mutation, `F0267` move-result/sensor/timing path, `G0310` cooldown set and `G0311` projectile cooldown cleared.
- `F0366` blocked step: input discard + PC-34 vblank, stop-wait false, return before `F0267`, cooldown assignment, and viewport-redraw side effects.
- Viewport redraw: main loop redraws `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)` after stop-wait/tick gating.

## Verifier/probe
- `scripts/verify_pass394_dm1_v1_dispatch_to_movement_state.py` writes `parity-evidence/verification/pass394_dm1_v1_dispatch_to_movement_state/manifest.json`.
- Existing focused C probe: `test_dm1_v1_movement_command_core_pc34_compat`.
- Existing integration probe: `test_dm1_v1_command_movement_sensor_timing_pc34_compat`.
