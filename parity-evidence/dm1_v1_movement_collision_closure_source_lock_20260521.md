# DM1 V1 movement/collision closure source lock - 2026-05-21

## Scope

Movement/collision/input closure audit for the PC 3.4 lane. This artifact records the ReDMCSB source anchors used before touching Firestaff and narrows the landable change in this pass to repairing the already-tracked Pass578 gate.

## ReDMCSB anchors

- `COMMAND.C:1304-1377` - `F0357_COMMAND_DiscardAllInput` discards queued input after blocked movement while retaining release/stop commands and replaying a pending click after unlock.
- `COMMAND.C:2045-2156` - `F0380_COMMAND_ProcessQueue_CPSC` locks the command queue, exits early while movement cooldown/projectile movement gates are active, otherwise dequeues and dispatches turn/move commands.
- `CLIKMENU.C:180-347` - `F0366_COMMAND_ProcessTypes3To6_MoveParty` applies stamina cost, handles backward-on-stairs before normal target-square movement, blocks walls/closed doors/real closed fakewalls/groups, discards input on blocked movement, and assigns movement cooldown only after accepted movement.
- `MOVESENS.C:316-910` - `F0267_MOVE_GetMoveResult_CPSCE` resolves party/group movement side effects, projectile-impact precheck, teleporter/pit chains, scent update, group deletion on destination, group deferral, and source/destination sensor calls.
- `MOVESENS.C:1553-1794` - `F0276_SENSOR_ProcessThingAdditionOrRemoval` processes movement-triggered floor/wall sensors and defers rotation-effect processing until the sensor scan completes.

## Firestaff gap closed

`pass578_dm1_v1_stairs_backstep_cooldown_gate` was still registered as an expected-failure CTest and its verifier read three Firestaff files from the repository root instead of their real `src/dm1/` and `tests/` paths. The product movement code and test labels were already present; this pass repairs the verifier paths and promotes the CTest out of `WILL_FAIL` so the cooldown-gated backward-on-stairs guard can protect the lane normally.

## Non-claims

- No original DOS runtime capture is promoted.
- No pixel/overlay parity claim is made.
- No movement product behavior is changed in this pass.
