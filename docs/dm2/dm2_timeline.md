# DM2 V1 Event Timeline vs DM1

## Overview

DM1 and DM2 both use a **game-tick-based event timeline**. A game tick is a fixed time unit (approximately 50ms — 20 ticks per second). All time-sensitive events are scheduled relative to the current gametick counter. The complexity and flexibility of the event system increased dramatically from DM1 to DM2.

## The Game Tick

- timdat.gametick — global tick counter, increments every ~50ms
- All time delays are specified in tick units
- DM2_INVOKE_ACTUATOR() accepts a delay parameter (in ticks) and computes the fire tick as: fire_tick = current_gametick + actuator_delay

## DM1 Event Model

DM1 events were **linear and single-purpose**:

- **Timed events** were queued as simple tick + function pointer entries
- **Door timers** were a single value per door — the door auto-closed after N ticks
- **Creature AI** ran on a fixed schedule per creature type
- **Projectiles** had fixed velocities and traveled until collision
- **Pressure plates** had one delay parameter — how long before they reset

There was **no general-purpose multi-timer infrastructure**. Each system (door, trap, creature) had its own hardcoded timer.

## DM2 Tick Generator / Queue System

DM2 introduced a **structured tick queue** for actuator scheduling.

### DM2_PROCESS_ACTUATOR_TICK_GENERATOR

This function runs each game tick and processes pending actuator events. Its core algorithm:

1. Reads the **actuator queue count** from ddat.savegamep4->warr_00[2]
2. Iterates over all floor/wall cells in the current map
3. For each cell that has an actuator decoration flag (bit 0x10), looks up the corresponding actuator record
4. Checks if the actuator's **fire tick** has been reached (current_gametick >= fire_tick)
5. If reached: dispatches the event based on actuator type

### DM2_INVOKE_ACTUATOR

void DM2_INVOKE_ACTUATOR(actuator_ptr, flag_value, delay_ticks)

- Reads actuator type from word at offset +0x4 (shifted)
- Reads actuator target from word at offset +0x6
- Computes fire_tick = current_gametick + delay_ticks
- Queues the event for future execution

This separation of when to fire from what to do is the core DM2 timing model.

## Event Categories in DM2

### 1. Immediate Events (delay = 0)
DM2_INVOKE_ACTUATOR(ptr, flag, 0) — fires on the next tick. Used for simple relays and instant door open/close.

### 2. Delayed Events (delay > 0)
DM2_INVOKE_ACTUATOR(ptr, flag, N) — fires N ticks in the future. Used for door close timers, delayed traps.

### 3. Periodic Events — TICK_GENERATOR
ACTUATOR_TYPE_TICK_GENERATOR (0x1E) and ACTUATOR_FLOOR_TYPE__INFINITE_TICK_GENERATOR (0x1E):
- Generates ticks indefinitely on a fixed period
- Each generated tick can trigger a downstream actuator
- Used for blinking lights, recurring projectiles, self-resetting mechanisms

### 4. Step-Timed Events
Floor/wall type activation may have a **step behavior** that schedules a future event:
- STEP_ON__OPEN_SET — door opens when stepped on
- STEP_OFF__CLOSE_CLEAR — door closes when stepped off
- STEP_CLOSE__OPEN_SET — door closes on step-away, opens immediately
- STEP_CONSTANT__OPEN — door stays open while occupied

### 5. Creature AI Events
Creature movement, attack, and spell casting are time-scheduled. Each creature has an individual tick timer and fires events on its own schedule.

### 6. Multi-Tick Counters
ACTUATOR_TYPE_COUNTER (0x1D) counts down over multiple steps. Example: 3 pressure plates must be stepped on in sequence, each step decrements the counter, output fires only when counter reaches 0.

### 7. Cross-Map Events
ACTUATOR_TYPE_CROSS_MAP (0x16) changes the current map, then fires the target actuator on the new map. The map change is part of the event timeline.

## DM1 vs DM2 Timeline Comparison

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Tick resolution | ~50ms (20 Hz) | Same |
| Periodic timers | Per-door hardwired | TICK_GENERATOR (generic) |
| Delayed events | Single-value per door | Any actuator can be delayed |
| Event queue | Linear per-system | Structured tick queue |
| Multi-step events | Not supported | Relay + counter chains |
| Cross-map events | Not supported | CROSS_MAP actuator |
| Creature scheduling | Fixed per-type | Per-creature individual timer |
| Projectile timing | Hardcoded velocity | Shooter actuator with type-specific data |

## Key DM2 Timing Functions

| Function | File | Role |
|----------|------|------|
| DM2_PROCESS_ACTUATOR_TICK_GENERATOR | c_tim_proc.cpp | Main tick queue processor |
| DM2_INVOKE_ACTUATOR | c_tim_proc.cpp | Queue a timed actuator event |
| DM2_QUEUE_TIMER | c_tim_proc.cpp | Queue a generic timer event |
| DM2_CHANGE_CURRENT_MAP_TO | c_tim_proc.cpp | Map switch (cross-map events) |
| DM2_ACTIVATE_TICK_GENERATOR | c_tim_proc.cpp | Activate a specific tick generator |

## Tick Generator Internal Logic

The tick generator loop in c_tim_proc.cpp (lines 4395-4470) iterates over the map grid:
- Reads x1,y1 and x2,y2 bounds from ddat.v1e03d8 (coordinate ranges)
- For each cell in range, if the cell floor/wall byte has bit 0x10 set, the cell has a floor actuator
- Fetches the corresponding actuator record from dm2_v1e038c (record table base)
- Records of type 0x1E (TICK_GENERATOR) or 0x33-0x37 (bitflag timers) are processed specially

This grid-scan approach means the tick processor scans the entire map each tick — suitable for small maps, O(n) per tick for large dungeons.
