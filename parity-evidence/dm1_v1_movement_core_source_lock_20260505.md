# DM1 V1 movement core source lock (2026-05-05)

Scope: input -> command queue -> turn/step dispatch -> collision/blockers -> sensor/timing side effects for DM1 V1 PC-34 compatibility.

## ReDMCSB anchors

Reference root: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

- `COMMAND.C:106-121` defines movement mouse hit rows; `COMMAND.C:252-305` defines movement key rows, with PC/A2x `MEDIA433` shifted-arrow fixes at `294-304`.
- `COMMAND.C:1304-1377` flushes buffered input after blocked movement; `COMMAND.C:2045-2156` locks the command queue, gates movement commands on `G0310/G0311`, dequeues one command, and dispatches turn (`F0365`) or move (`F0366`).
- `CLIKMENU.C:142-174` turns the party, including stairs-while-turning behavior and same-square leave/enter sensor processing.
- `CLIKMENU.C:180-347` maps C003..C006 to relative movement vectors, handles backward-on-stairs and target-stairs consequences, rejects walls/closed doors/closed real fakewalls/groups, discards input on blocks, applies `G0310` step cooldown, and clears projectile cooldown.
- `DUNGEON.C:1371-1421` is the relative coordinate math; `DUNGEON.C:1508-1582` is stairs level-change location and exit facing.
- `CHAMPION.C:1180-1215` is the movement tick formula, including the original `load == maxLoad` slow-cadence bug.
- `MOVESENS.C:752-783` records true square-change scent/timestamp; `MOVESENS.C:799-818` performs party leave/enter movement effects; `MOVESENS.C:1553-1794` walks/filters sensor triggers in list order.
- `GAMELOOP.C:150-155` decrements movement cooldowns; `GAMELOOP.C:215-219` runs one queued command until input wait is released; `DRAWVIEW.C:709-724` schedules viewport redraw after state mutation.

## Firestaff integration points checked

- `dm1_v1_input_command_queue_pc34_compat.c` locks key/mouse command mapping, pending-click replay, cooldown gating, and blocked-input discard.
- `dm1_v1_movement_command_core_pc34_compat.c` composes queue dispatch with turn/step execution, stairs consequence paths, wall/door/fakewall/group collision gates, source/destination sensor effects, and accepted-step timing.
- `dm1_v1_movement_timing_pc34_compat.c` computes slowest living champion movement ticks, successful-step scent/timestamp results, projectile cooldown clearing, and loop cooldown decrements.
- `test_dm1_v1_command_movement_sensor_timing_pc34_compat.c` exercises end-to-end command->movement->sensor/timing behavior, blocked side-effect suppression, projectile/disabled movement gates, turning, mouse pending replay, and empty-party group collision bug.

## Verification run

Commands run on N2 in `<firestaff-repo>`:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target test_dm1_v1_command_movement_sensor_timing_pc34_compat -j2
./build/test_dm1_v1_command_movement_sensor_timing_pc34_compat
python3 tools/verify_dm1_v1_command_movement_sensor_timing_source_lock.py
python3 tools/verify_dm1_v1_movement_command_gate_source_lock.py
python3 tools/verify_dm1_v1_party_movement_sensor_order_source_lock.py
```

Results:

- `dm1V1CommandMovementSensorTimingIntegrationOk=1`
- `dm1_v1_command_movement_sensor_timing_source_lock=pass citations=25`
- `dm1_v1_movement_command_gate_source_lock=pass citations=12`
- `dm1_v1_party_movement_sensor_order_source_lock=pass citations=9`
