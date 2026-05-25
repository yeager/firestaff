# DM2 V1 — Time System: Turn/Time Flow in DM2 vs DM1

**Audit date:** 2026-05-25
**Sources:** SKULL.ASM, skproject SKWIN/SkWinCore.cpp, include/dm2_v1_game.h, include/dm2_v1_outdoor_renderer.h, docs/dm2_game_loop.md, docs/dm2_movement.md, docs/dm2_overview.md

---

## 1. DM1 Time System (Brief)

DM1 had no persistent time-of-day system. The game operated on a simple turn counter - each discrete player action (move one tile, attack, use item) consumed one turn. Creature AI and event timers were driven by turn ticks, but there was no clock visible to the player and no day/night cycle.

- Turn pacing: approximately 1 real-second per turn at normal game speed
- No time-of-day tracking in save format
- No weather system

---

## 2. DM2 Time System — Overview

DM2 introduces a dual-layer time system:

1. **Turn counter** - continues the DM1 tradition: each discrete action consumes one game turn
2. **Time of day** - a 1440-minute (24-hour) clock that advances as turns pass, driving sky color, weather, and potentially creature behavior

From include/dm2_v1_game.h:
```c
typedef struct {
    int party_x, party_y, party_dir;
    int current_level;
    int outdoor;            // 0=indoor dungeon, 1=outdoor
    int gold;
    int reputation;
    int time_of_day;        // minutes from midnight (0-1439)
    const char *data_dir;
} DM2_V1_GameState;
```

Starting time: time_of_day = 720 (noon).

---

## 3. Time-of-Day Cycle

The time_of_day field ranges from 0 (midnight) to 1439 (23:59), advancing each time a turn passes. The outdoor renderer uses this to compute sky color:

From include/dm2_v1_outdoor_renderer.h:
```c
typedef struct {
    int time_of_day;        // minutes from midnight (0-1439)
    // weather, sky color state...
} DM2_V1_OutdoorState;
```

Sky color is derived from time_of_day:
- Dawn gradient (morning)
- Noon (midday)
- Afternoon
- Dusk (evening gradient)
- Night

The outdoor renderer uses the clock value to interpolate sky gradients.

---

## 4. Game Loop Timing (per-frame)

DM2's GAME_LOOP() (SkWinCore.cpp) processes timing at multiple granularities:

### 4a. Int 08h Timer Tick (Hardware IRQ 0)

The INT08 handler (_INT08_HANDLER) fires at a hardware-defined rate and drives:
- Movement tick (party movement, creature AI)
- Time-of-day advance (at a configured rate per tick)
- Weather zone updates

### 4b. Timer System (c_timer, c_tim_proc)

DM2 has a multi-timer system with distinct timer types:

- **PROCESS_TIMER_0C** - per-champion light/torch timers. Each champion has a torch duration that counts down. When the torch expires, light radius shrinks. This is a separate timer per champion slot.
- **PROCESS_TIMER_RESURRECTION** - death/resurrection countdown. When a champion dies, a resurrection timer counts down. If it expires before resurrection is performed, the champion is permanently lost.
- **CONTINUE_ORNATE_ANIMATOR** - special animation sequences for wall ornaments (animated tiles, lever transitions, door sequences).
- **CONTINUE_TICK_GENERATOR** - game event ticker. The tick generator fires periodic events that drive creature AI, projectile motion, and actuator processing.

### 4c. Event Queue (c_eventqueue, c_events)

DM2 uses an event-driven architecture where actions can be scheduled for future execution:
- Queued events processed each frame via PROCEED_TIMERS()
- Used for: delayed actuator firing, projectile flight paths, creature spawn scheduling, animation sequences

---

## 5. Turn Advancement Rate

The exact rate at which time_of_day advances (how many turns = how many minutes) is derived from SKULL.ASM runtime analysis and not yet fully confirmed in the stub. Based on context:
- Indoor: same approximate pace as DM1 (~1 turn/second real time)
- Outdoor: possibly faster (larger open areas, different tick rate)

The tick generator (CONTINUE_TICK_GENERATOR) is the primary mechanism by which game ticks are normalized across different game states.

---

## 6. Weather and Time Interaction

DM2 introduces weather zones that exist in outdoor areas. Weather is tracked in the outdoor renderer state and updates each tick:

Weather conditions (from outdoor renderer):
- Clear sky
- Rain (wet conditions, affects visibility)
- Fog (obscured visibility)
- Storm (severe weather, gray sky)

Weather affects:
- Movement (possibly slowed in rain/storm)
- Visibility (fog/storm reduces sight range)
- Outdoor renderer sky color (gray overlay for storm/fog)

Weather timers: weather zones may have duration timers set by dungeon designers, processed each tick.

---

## 7. Turn System in Dungeons (Indoor)

Indoor turn flow is similar to DM1:
1. Player takes an action (move, attack, use item, cast spell, interact)
2. Action consumes one turn
3. All creatures take one turn (AI tick)
4. Time advances (time_of_day ticks forward)
5. Pending actuators/processes fire
6. Render frame

Collision detection: wall-sliding approach same as DM1 (attempted but not fully enforced in stub).

---

## 8. Turn System Outdoors (Outdoor)

Outdoor areas use a different movement/combat model:
- No first-person view - outdoor renderer draws sky, ground, trees, buildings
- Party moves on an outdoor grid (different tile size from dungeon grid)
- Creatures in outdoor zones have different pathfinding (c_ai handles both indoor and outdoor)
- Turn advancement continues at the same rate as indoor

---

## 9. Save Format Time Tracking

DM2 save format includes time_of_day, gold, and reputation in the GameState struct - fields absent from the DM1 save format.

```c
// DM1 save: dungeon level, party position, inventory, champions
// DM2 save: adds outdoor flag, gold, reputation, time_of_day
```

---

## 10. Comparison: Time System

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Turn counter | Yes | Yes (same tile-based model) |
| Time-of-day clock | None | 1440-minute cycle |
| Day/night graphics | None | Sky color changes with time |
| Weather system | None | Rain/fog/storm in outdoor zones |
| Torch timer | Per-party global | Per-champion (PROCESS_TIMER_0C) |
| Resurrection timer | None | PROCESS_TIMER_RESURRECTION |
| Event queue | Limited | Full c_eventqueue system |
| Tick generator | Fixed rate | CONTINUE_TICK_GENERATOR |

---

## STATUS: AUDIT COMPLETE