# Pass 1103 — DM2 V1 Timer Dispatch Wiring

## What

Wire 7 implemented timer ops handlers into the DM2_V1_TimerDispatcher
framework via adapter functions that bridge the uniform TimerTypeHandler
signature to the narrow per-timer callback APIs.

## Wired timer types

| Type | Name | skproject source |
|------|------|-----------------|
| 0x02 | DESTROY_DOOR | c_tim_proc.cpp:422 |
| 0x0E | PROCESS_0E | SkWinCore.cpp:2173 |
| 0x3D | PROCESS_3D | c_tim_proc.cpp:902 |
| 0x46 | LIGHT | c_tim_proc.cpp:918 |
| 0x55 | ORNATE_ANIMATOR | c_tim_proc.cpp:961 |
| 0x56 | TICK_GENERATOR | c_tim_proc.cpp:994 |
| 0x58 | RELEASE_DOOR_BUTTON | c_tim_proc.cpp:1068 |

## Design

Each adapter extracts timer fields (value_a, value_b, actor, reserved) into
the per-handler callback struct, then delegates to the existing timer ops
function. Handlers with missing callback fields return 0 (fail-closed),
propagating the dispatcher's receipt counting.

`DM2_V1_TimerDispatchWiringContext` aggregates all callback function pointers
the host must provide. Fields left NULL cause their timer types to gracefully
reject (handler returns 0, counted as handler_rejected_count).

## Remaining types (19 of 26)

0x01 STEP_DOOR, 0x04 ACTUATE_TILE subdispatch, 0x0C, 0x0D RESURRECTION,
0x15 SOUND, 0x19 CLOUD, 0x1E STEP_MISSILE, 0x21/0x22 THINK_CREATURE,
0x47 HERO_ENCH_FLAG, 0x48 ENCH_POWER, 0x4B POISON, 0x54 UPDATE_WEATHER,
0x59, 0x5A ORNATE_NOISE, 0x5B/0x5C record flag ops, 0x5D MOVE_RECORD_ROTATE,
0x5E ALLOC_NEW_CREATURE.

## Test

`test_dm2_v1_timer_dispatch_wiring` — verifies wiring count, handler
population, NULL safety, fail-closed on missing callbacks, and end-to-end
RELEASE_DOOR_BUTTON handler with mock record.
