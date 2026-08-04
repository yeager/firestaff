# Pass 1103 — DM2 V1 Timer Dispatch Wiring

## What

Wire all 26 of 26 source timer-type-matrix entries into the
DM2_V1_TimerDispatcher framework via adapter functions that bridge the
uniform TimerTypeHandler signature to the narrow per-timer callback APIs.
All 26 entries are now behaviourally implemented (no remaining stub
handlers): the final six — RESURRECTION, PROCESS_CLOUD, STEP_MISSILE,
THINK_CREATURE_A/B, and ORNATE_NOISE — were ported from skproject's
c_tim_proc.cpp / c_ai.cpp / c_cloud.cpp behind the same callback-based
architecture used by the rest of the module.

## Wired timer types

| Type | Name | skproject source | Status |
|------|------|-----------------|--------|
| 0x01 | STEP_DOOR | c_tim_proc.cpp | delegated |
| 0x02 | DESTROY_DOOR | c_tim_proc.cpp:422 | delegated |
| 0x04 | ACTUATE_TILE (classes 2/4/5/6) | c_tim_proc.cpp:4214-4230 | delegated via actuator_tile[] |
| 0x0C | PROCESS_TIMER_0C | c_tim_proc.cpp:30 | delegated |
| 0x0D | RESURRECTION | c_tim_proc.cpp:39 | delegated |
| 0x0E | PROCESS_0E | SkWinCore.cpp:2173 | delegated |
| 0x15 | PROCESS_SOUND | c_tim_proc.cpp:4066 | delegated |
| 0x19 | PROCESS_CLOUD | c_cloud.cpp:587 | delegated |
| 0x1E | STEP_MISSILE | c_tim_proc.cpp:442 | delegated |
| 0x21 | THINK_CREATURE_A | c_ai.cpp:5649 | delegated (indirected) |
| 0x22 | THINK_CREATURE_B | c_ai.cpp:5649 | delegated (indirected) |
| 0x3D | PROCESS_3D | c_tim_proc.cpp:902 | delegated |
| 0x46 | LIGHT | c_tim_proc.cpp:918 | delegated |
| 0x47 | HERO_ENCH_FLAG | c_tim_proc.cpp:4112 | delegated |
| 0x48 | ENCH_POWER | c_tim_proc.cpp:4130 | delegated |
| 0x4B | POISON | c_tim_proc.cpp:4164 | delegated |
| 0x54 | UPDATE_WEATHER | c_tim_proc.cpp:4182 | delegated |
| 0x55 | ORNATE_ANIMATOR | c_tim_proc.cpp:961 | delegated |
| 0x56 | TICK_GENERATOR | c_tim_proc.cpp:994 | delegated |
| 0x58 | RELEASE_DOOR_BUTTON | c_tim_proc.cpp:1068 | delegated |
| 0x59 | PROCESS_TIMER_59 | c_tim_proc.cpp:1077 | delegated |
| 0x5A | ORNATE_NOISE | c_tim_proc.cpp:1092 | delegated |
| 0x5B | RECORD_CLEAR (byte@4 &= ~1) | c_tim_proc.cpp | delegated (inline) |
| 0x5C | RECORD_SET (byte@2 \|= 1) | c_tim_proc.cpp | delegated (inline) |
| 0x5D | MOVE_RECORD_ROTATE | c_tim_proc.cpp:4230 | delegated |
| 0x5E | ALLOC_NEW_CREATURE | c_tim_proc.cpp:4253 | delegated |

## Design

Each adapter extracts timer fields (value_a, value_b, actor, reserved) into
the per-handler callback struct, then delegates to the corresponding timer
ops function in `dm2_v1_timer_ops_pc34_compat.c`. Handlers with missing
callback fields return 0 (fail-closed), propagating the dispatcher's
receipt counting.

`DM2_V1_TimerDispatchWiringContext` aggregates all callback function
pointers the host must provide, including hero-state arrays (timeridx,
curHP, heroflag, ench_power, poisoned/poison) indexed per party slot, and
the actuator-tile subdispatch functions (pitfall/door/teleporter/trickwall)
used by 0x04. Fields left NULL cause their timer types to gracefully
reject (handler returns 0, counted as handler_rejected_count).

0x04 ACTUATE_TILE is not registered in `dispatcher->handlers[]` at all —
the dispatcher core (`dm2_v1_proceed_timers.c`) already owns the
square-class subdispatch via `dispatcher->tile_class_at` and
`dispatcher->actuator_tile[0..6]`; this wiring module only supplies
classes 2 (pitfall), 4 (door), 5 (teleporter), and 6 (trickwall). Classes
0/1 (wall/floor mecha no-op) and 3 (fall-through no-op) are handled inside
the dispatcher core itself and need no adapter here.

0x0D RESURRECTION, 0x19 PROCESS_CLOUD, 0x1E STEP_MISSILE, and 0x5A
ORNATE_NOISE each got a new callback struct + implementation in
`dm2_v1_timer_ops_pc34_compat.{h,c}` (`dm2_v1_process_timer_resurrection`,
`dm2_v1_process_cloud`, `dm2_v1_step_missile`,
`dm2_v1_continue_ornate_noise`), wired into
`DM2_V1_TimerDispatchWiringContext` the same way as the rest of the module
(record-address, tile-value, creature-query, and record-mutation
callbacks, all fail-closed on NULL).

STEP_MISSILE and PROCESS_CLOUD implement the source's core control flow
(wall bounce/deletion, party/creature hit detection with AI-spec-flag
reflection for missiles; door/party/creature damage plus type-specific
decay — 7, 0x28, 0x64 — for clouds) but are intentionally simplified
relative to the ~450-line STEP_MISSILE and ~180-line PROCESS_CLOUD
source bodies: multi-cell step chains, trickwall/door sub-actuation, and
some of the source's record-link bookkeeping are left to the individual
callbacks (`move_record`, `cut_record_from`, `append_record_to`,
`attack_door`, etc.) rather than reproduced inline. Behavioural parity for
the outer decision tree is preserved; byte-for-byte parity of the full
source routines is not yet claimed for these two.

0x21/0x22 THINK_CREATURE_A/B are wired through a new indirection field,
`think_creature_handler` / `think_creature_context`, rather than a direct
call into `dm2_v1_think_creature_pc34_compat.h`: that module's
`dm2_v1_think_creature_timer_handler` already implements the full
DM2_GET_CREATURE_AT + body-callback boundary (see
`pass_dm2_v1_think_creature_pc34_compat.md`-equivalent evidence in that
header's own comment block), but pulls in the DM2-002 record-pool and
dungeon-loader dependency chain. Indirecting through a plain
`DM2_V1_TimerTypeHandler` function pointer lets the host bind the real
handler at runtime while keeping this wiring module's unit tests free of
that dependency chain — `dispatcher->handlers[0x21]` and `[0x22]` both
resolve to the same `handle_think_creature` adapter, matching the source
(both timer types share one `DM2_THINK_CREATURE` body).

`dm2_v1_process_timer_0c` was renamed `dm2_v1_process_timer_0c_cb` in
`dm2_v1_timer_ops_pc34_compat.{h,c}` to avoid a duplicate-symbol link
error against the pre-existing `dm2_v1_process_timer_0c(DM2_V1_HeroState*)`
in `dm2_v1_hero_ops_pc34_compat.c`, which implements the same source
routine against a different (already-wired) hero-state abstraction.

## Test

`test_dm2_v1_timer_dispatch_wiring` — verifies wiring count (26), full
handler-table population for every known type except 0x04 (which is
verified via `actuator_tile[]` instead), NULL safety, fail-closed on
missing callbacks, end-to-end RELEASE_DOOR_BUTTON and 5B_RECORD_CLEAR
handlers with a mock record, and end-to-end 0x04 subdispatch routing to
the door actuator via a fake tile-class lookup. Also covers the six newly
implemented handlers: RESURRECTION's final (yB==0) phase invoking
`bring_champion_to_life`, PROCESS_CLOUD decaying a type-7 cloud to zero
and consuming the timer, STEP_MISSILE bouncing off (deleting at) a wall
tile, THINK_CREATURE_A/B delegating to a bound `think_creature_handler`
(and rejecting when unbound), and ORNATE_NOISE clearing the frame counter
when the record is inactive.
