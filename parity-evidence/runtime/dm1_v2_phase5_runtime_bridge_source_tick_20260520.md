# DM1 V2 Phase 5 Runtime Bridge Source-Tick Gate

Scope: narrow Phase 5 bridge slice proving that V2 smooth movement presentation
starts only after a source-accepted V1 movement tick and mutates only the V2
camera controller.

Source audit, ReDMCSB WIP20210206 `Toolchains/Common/Source`:

- `COMMAND.C` `F0380_COMMAND_ProcessQueue_CPSC`, lines 2096-2106: source
  movement commands are rejected while `G0310_i_DisabledMovementTicks` or
  matching `G0311_i_ProjectileDisabledMovementTicks` are active.
- `COMMAND.C` `F0380_COMMAND_ProcessQueue_CPSC`, lines 2150-2156: accepted
  turn/move command IDs dispatch to `F0365`/`F0366` only.
- `CLIKMENU.C` `F0366_COMMAND_ProcessTypes3To6_MoveParty`, lines 278-329:
  wall, door, fakewall, and group collision are resolved before `F0267`.
- `CLIKMENU.C` `F0366_COMMAND_ProcessTypes3To6_MoveParty`, lines 330-346:
  source movement cooldown `G0310` is published from `F0310` after accepted
  source movement, and projectile lockout `G0311` is cleared.
- `CHAMPION.C` `F0310_CHAMPION_GetMovementTicks`, lines 1180-1215: champion
  load, wounds, and Boots of Speed determine source movement ticks.
- `MOVESENS.C` `F0267_MOVE_GetMoveResult_CPSCE`, lines 752-818: party scent
  timing, last movement time, source leave sensor dispatch, destination group
  deletion, and destination enter sensors are source effects.
- `GAMELOOP.C` `F0002_MAIN_GameLoop_CPSDF`, lines 69-155 and 215-219:
  timeline, redraw, game time, cooldown decrement, and command wait cadence are
  source main-loop responsibilities.
- `DUNVIEW.C` `F0128_DUNGEONVIEW_Draw_CPSF`, lines 8318-8612, and `DRAWVIEW.C`
  `F0097_DUNGEONVIEW_DrawViewport`, lines 709-722: viewport draw and present
  cadence are source redraw responsibilities.

Landed gate: `test_dm1_v2_phase5_runtime_bridge_pc34` runs a source V1 movement
pipeline tick, passes only the accepted result/party tuple to the V2 bridge,
starts camera interpolation, ticks the camera, and byte-compares the V1 pipeline
and tick result to prove presentation did not mutate source-owned state.
