# Pass 1140 — DM2 Timer Processing Dispatcher (c_tim_proc.cpp)

## Source
skproject/SKULLWIN/c_tim_proc.cpp (4680 lines)

## Target
- `include/dm2_v1_tim_proc_pc34_compat.h`
- `src/dm2/dm2_v1_tim_proc_pc34_compat.c`
- `tests/test_dm2_v1_tim_proc_pc34_compat.c`

## What was ported

The main timer event loop (DM2_PROCEED_TIMERS) and individual timer-type
handlers.  The skproject register-machine code (c_nreg, RG1L, etc.) was
translated to clean C with callback-based architecture.

### Timer types dispatched (28 types)

| Type | Name | Handler |
|------|------|---------|
| 0x00 | no-op | skip |
| 0x01 | STEP_DOOR | dm2_v1_step_door |
| 0x02 | DESTROY_DOOR | dm2_v1_process_timer_destroy_door |
| 0x04 | ACTUATE | tile-type dispatch (wall/floor/pit/door/tele/trick) |
| 0x0C | TIMER_0C | dm2_v1_process_timer_0c |
| 0x0D | RESURRECTION | dm2_v1_process_timer_resurrection |
| 0x0E | TIMER_0E | process item bonus with record backup |
| 0x15 | SOUND | process_sound callback |
| 0x19 | CLOUD | process_cloud callback |
| 0x1D/0x1E | MISSILE | dm2_v1_step_missile |
| 0x21/0x22 | THINK | think_creature callback |
| 0x3C/0x3D | TIMER_3D | dm2_v1_process_timer_3d |
| 0x46 | LIGHT | dm2_v1_process_timer_light |
| 0x47 | ENCH_47 | hero enchantment timer |
| 0x48 | ENCH_POWER | party enchantment decay |
| 0x4B | POISON | process_poison callback |
| 0x54 | WEATHER | update_weather callback |
| 0x55 | ORNATE_ANIM | continue ornate animator |
| 0x56 | TICK_GEN | continue tick generator |
| 0x58 | DOOR_BUTTON | release door button |
| 0x59 | TIMER_59 | continuous ornate done |
| 0x5A | ORNATE_NOISE | ornate noise loop |
| 0x5B/0x5C | ACTIVATE | set record bit |
| 0x5D | PARTY_WARP | party rotation/warp |
| 0x5E | CREATURE_SPAWN | spawn creature timer |

### Exported functions

- `dm2_v1_proceed_timers` — main timer loop
- `dm2_v1_process_timer_0c` — hero ready flag
- `dm2_v1_process_timer_resurrection` — 3-phase resurrection
- `dm2_v1_process_timer_destroy_door` — destroy door tile
- `dm2_v1_step_door` — animate door open/close
- `dm2_v1_step_missile` — advance missile
- `dm2_v1_process_timer_3d` — move record + noise
- `dm2_v1_process_timer_light` — light level table
- `dm2_v1_timproc_compute_flag` — flag toggle helper (SKW_3a15_1da8)
- `dm2_v1_invoke_message` — queue actuate timer
- `dm2_v1_invoke_actuator` — resolve actuator target

### Sub-dispatchers (type 0x04 ACTUATE by tile)

The full actuator sub-dispatchers (DM2_ACTUATE_WALL_MECHA with 67-case
switch, DM2_ACTUATE_FLOOR_MECHA, DM2_ACTUATE_PITFALL, DM2_ACTUATE_DOOR,
DM2_ACTUATE_TELEPORTER, DM2_ACTUATE_TRICKWALL) are structurally mapped
in the dispatcher.  Their internal logic delegates via callbacks to
ACTIVATE_SHOOTER, ACTIVATE_RELAY1/2, ACTIVATE_ORNATE_ANIMATOR,
ACTIVATE_TEST_FLAG, ACTIVATE_INVERSE_FLAG, ACTIVATE_TICK_GENERATOR,
ACTIVATE_CONTINUOUS_ORNATE_ANIMATOR, ACTIVATE_CREATURE_KILLER,
ANIMATE_CREATURE, and ADVANCE_TILES_TIME.

### Callback struct

76 function pointers covering: timer queue, map, record, door, creature,
hero/party, item, cloud, light, sound, actuator, missile, and misc
operations.

## What was skipped

- Platform code (c_allegro, c_addon) — not applicable
- Direct memory manipulation of mapdat/ddat/timdat/party globals —
  replaced by callbacks

## Tests (22)

| # | Test | Verifies |
|---|------|----------|
| 1 | null_safety | all functions handle NULL cb/tim/receipt |
| 2 | compute_flag | yB=0->1, yB=1->0, yB=2->toggle, yB=3->0 |
| 3 | timer_0c_alive | hero flag set when HP > 0 |
| 4 | timer_0c_dead | hero flag not set when HP = 0 |
| 5 | resurrection_phase0 | bring_champion_to_life called |
| 6 | resurrection_phase2 | cloud created, data += 5, requeued |
| 7 | destroy_door | door_destroyed flag set |
| 8 | timer_3d | record moved, noise queued |
| 9 | timer_light | light changed, continuation queued |
| 10 | timer_light_zero | zero value -> immediate return |
| 11 | invoke_message | timer queued with correct type 0x04 |
| 12 | invoke_actuator | actuator record parsed, invoked |
| 13 | proceed_empty | no timers -> no processing |
| 14 | proceed_sound | SOUND type dispatched |
| 15 | proceed_weather | WEATHER type dispatched |
| 16 | proceed_cloud | CLOUD type dispatched |
| 17 | proceed_think | THINK type dispatched |
| 18 | proceed_poison | POISON type dispatched |
| 19 | proceed_light | LIGHT type dispatched + recalc |
| 20 | proceed_multiple | 3 timers processed in sequence |
| 21 | direction_tables | dx/dy match skproject table1d27fc/table1d2804 |
| 22 | timer_type_enum | enum values match skproject settype constants |

## Build verification

```
ninja -C build test_dm2_v1_tim_proc_pc34_compat  # 0 warnings
ctest --test-dir build -R dm2_v1_tim_proc -j1    # 22/22 passed
```
