# Nexus V1 Event Timeline — Tick Scheduling vs DM1/DM2

## Source Files
- `docs/dm1_v1_triggers_timer.md` — DM1 C006 countdown timer, TIMELINE.C event processing
- `docs/dm2_actuators.md` — DM2 tick generator, actuator queue
- `docs/nexus_sensors.md` — Nexus SDDRVS.TSK event model
- `src/dm1/dm1_v1_event_timer_pc34_compat.c` — DM1 event timer system
- `src/nexus/nexus_v1_engine.c` — Nexus tick stub

---

## 1. DM1 Event Timeline

DM1 has minimal event scheduling. Most triggers fire immediately (on tick when condition met).
The exception is the **C006 countdown timer wall sensor** (TIMELINE.C lines 1136-1350).

### C006 Countdown Timer (TIMELINE.C F0245)
- Attached to a wall square as a wall-event sensor
- Counts down from a stored value each tick
- Fires (triggers its linked effect) when counter reaches zero
- Timer can be reset by other events (relay from another sensor)
- One countdown active per wall sensor instance

### DM1 Tick Rate
- 55ms per tick (~18.2 Hz)
- Same as DM2 and Nexus V1
- All immediate effects: sensor fires -> effect executes in same tick
- No future-tick queuing for basic triggers

### TIMELINE.C Event Processing (F0245)
- `F0245_SENSOR_ProcessWallEventSensor` called each tick for active wall-event sensors
- Walks wall sensor list, evaluates countdown state
- When counter hits zero: fires linked effect (local or remote)
- No cross-level timelines (each level independent)

---

## 2. DM2 Event Timeline

DM2 has a proper tick-scheduled event system via **TICK_GENERATOR actuator** (type 0x1E)
and **DM2_INVOKE_ACTUATOR** with delay parameter.

### DM2 Actuator Invocation Path
```
1. Sensor fires (floor/wall event)
2. DM2_INVOKE_ACTUATOR(actuator, flag, delay)
   - Computes fire_tick = current_tick + delay
   - Inserts into pending_actuator_priority_queue (sorted by fire_tick)
3. DM2_PROCESS_ACTUATOR_TICK_GENERATOR() called each tick
   - Walks queue, dequeues all where fire_tick <= current_tick
   - Calls actuator type handler: effect executed (spawn, projectile, door, etc.)
```

### TICK_GENERATOR (0x1E)
- Produces a tick pulse at a configured interval
- Drives periodic events: lights that blink, sounds that repeat, waves that spawn
- Can be wired to: RELAY -> COUNTER -> CREATURE_GENERATOR (wave spawner)
- Interval stored in actuator type-specific data (bytes 0-3 of 8-byte record)

### WORK_TIMER (0x31)
- Similar to TICK_GENERATOR but one-shot or repeating
- Used for timed puzzle sequences

### Cross-Map Timeline via CROSS_MAP (0x16)
- One map's actuator can trigger a target actuator on a different dungeon map
- Target map must be loaded or referenced by ID
- Enables world-state persistence across level transitions

### DM2 Tick Rate
- Same ~18.2 Hz as DM1
- Priority queue sorted by fire_tick for O(log n) enqueue/dequeue

---

## 3. Nexus V1 Event Timeline (SDDRVS.TSK Script)

Nexus V1 tick handling is in `nexus_v1_tick()` (nexus_v1_engine.c):

```c
void nexus_v1_tick(Nexus_V1_Engine *engine) {
    if (!engine || !engine->initialized) return;
    /* Nexus uses same V1 tick rate as DM1 (55ms / 18.2 Hz).
     * Game logic: movement, combat, creature AI, timer events.
     * FUTURE: full game logic integration. */
}
```

Currently a stub. Full implementation would route SDDRVS.TSK events through the tick scheduler.

### Hypothetical Nexus Tick Integration

```c
// Per-tick script evaluation
void nexus_v1_script_tick(Nexus_V1_Engine *engine) {
    // 1. Evaluate condition rules (WHEN clauses)
    for each rule in SDDRVS.TSK.rules:
        if (condition_true(rule.condition, engine.game_state)):
            // 2. Queue action for next tick or immediate
            if (rule.delay_ticks > 0):
                schedule_action(rule.action, current_tick + rule.delay_ticks)
            else:
                dispatch_action(rule.action)

    // 3. Process scheduled actions
    while (scheduled_queue.front.fire_tick <= current_tick):
        dispatch_action(scheduled_queue.dequeue().action)
}

// Script rule structure (reversed from 5,448-byte file)
struct ScriptRule {
    uint8_t  condition_op;    // COND_* opcode
    uint8_t  condition_data;  // operand (coord, item, level, etc.)
    uint8_t  action_op;       // ACT_* opcode
    uint8_t  action_data[3];  // action parameters
    uint8_t  delay_ticks;     // 0 = immediate, N = delay
    uint8_t  flags;           // once_only, active, etc.
};
```

### Nexus Event Scheduling vs DM1/DM2

| Aspect           | DM1              | DM2                      | Nexus V1             |
|------------------|------------------|--------------------------|----------------------|
| Tick rate        | 55ms / 18.2 Hz   | ~18.2 Hz                 | 55ms / 18.2 Hz       |
| Immediate events | Yes (sensor fire) | Yes                      | Yes (script dispatch)|
| Delayed events   | No (C006 only)   | Yes (DM2_INVOKE delay)   | Yes (delay_ticks)   |
| Periodic events  | No               | Yes (TICK_GENERATOR)     | Yes (script loop)   |
| Cross-map events | No               | Yes (CROSS_MAP 0x16)    | Yes (ACT_TELEPORT)  |
| Event queue      | None (immediate) | Priority queue by fire_tick | Scheduled action list |
| Script evaluation| N/A              | N/A                      | Per-tick rule scan  |

---

## 4. Nexus Tick Implementation Path

For Firestaff to implement Nexus tick/event system:

1. **Tick loop** (nexus_v1_tick): call script_evaluator + game_logic each ~55ms
2. **Script evaluator**: scan SDDRVS.TSK rules, fire matching conditions
3. **Scheduled action queue**: priority queue of (fire_tick, action) pairs
4. **Action dispatcher**: switch on ACT_* opcode -> call implementation
5. **State persistence**: flag[256] store, counter state, timer state across ticks

### Suggested Implementation Order
```
Phase 1: Basic tick + immediate script dispatch (no delay)
Phase 2: Scheduled action queue (delay_ticks support)
Phase 3: TICK_GENERATOR-equivalent (periodic script rules)
Phase 4: Cross-level timeline (ACT_TELEPORT triggers load of new LEV##.DGN)
```
