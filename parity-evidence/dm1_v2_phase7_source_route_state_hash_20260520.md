# DM1 V2 Phase 7 source-route state hash gate (2026-05-20)

This narrow gate proves that the V2 movement command adapter can leave source gameplay command IDs untouched when V2 presentation routing is disabled. The same deterministic command script is applied to a ReDMCSB-derived V1 movement model and to the V2 source route, then both normalized gameplay states are hashed after every input.

## Source evidence

- `DEFS.H:235-243` defines command IDs: turn left `1`, turn right `2`, move forward `3`, move right `4`, move backward `5`, move left `6`.
- `COMMAND.C:2045-2156` pops one queued command, blocks movement when movement-disable gates apply, dispatches `C001/C002` to `F0365_COMMAND_ProcessTypes1To2_TurnParty`, and dispatches `C003..C006` to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
- `CLIKMENU.C:142-173` sets `G0321_B_StopWaitingForPlayerInput` and turns the party with `M021_NORMALIZE(G0308_i_PartyDirection + 1)` for right or `+ 3` for left.
- `CLIKMENU.C:224-233` defines movement arrow forward/right step tables: forward `{1,0,-1,0}` and right `{0,1,0,-1}`.
- `CLIKMENU.C:256-270` maps command IDs `3..6` to the movement-arrow index and calls `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`.
- `DUNGEON.C:35-44` defines the source direction-to-step tables.
- `DUNGEON.C:1371-1391` applies forward movement, simulates turning right, then applies right/strafe movement.
- `GAMELOOP.C:164-219` clears `G0321_B_StopWaitingForPlayerInput`, calls `F0380_COMMAND_ProcessQueue_CPSC`, and waits until a command stops input waiting while game time is ticking.

## Verified slice

`test_dm1_v2_source_route_state_hash_pc34` runs this script from `(x=4, y=4, dir=0)`:

```text
3, 3, 2, 4, 1, 6, 5, 2, 3, 4, 4, 1, 3
```

For every command it asserts:

- V2 routing with presentation disabled returns `DM1_V2_MOVEMENT_ROUTE_V1_SOURCE`.
- `sourceCommand == runtimeCommand == ReDMCSB command ID`.
- V1 and V2 normalized gameplay states match after the command.
- Rolling FNV-1a state hashes match after every command.

Final normalized state: `(x=4, y=5, dir=0)`.

Final hash for both lanes: `0x6b6e36b34cca3cd3`.

This does not claim that the enhanced V2 presentation runtime is gameplay-identical. It locks the source-preserving route that Phase 7 can use before layering visual interpolation or enhanced presentation features.
