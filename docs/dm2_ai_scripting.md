# DM2 V1 Trigger/Scripting System — Events vs Hardwired Sensors

## Source Evidence
- `skproject/SKULLWIN/c_events.cpp` — Event queue, UI click events, timer events
- `skproject/SKULLWIN/c_creature.cpp` — `DM2_PROCEED_CCM`, creature action triggers
- `skproject/SKULLWIN/c_ai.cpp` — `DM2_PROCEED_XACT_*` (creature action execution)
- `skproject/SKULLWIN/c_timer.cpp` / `c_tim_proc.cpp` — Timer-based triggers
- `src/dm1/dm1_v1_sensor_trigger_pc34_compat.c` — DM1 hardwired sensor system

## DM1: Hardwired Sensor/Trigger (ReDMCSB MOVESENS.C)
DM1 sensor logic was **compile-time hardwired** in the game binary:
- F0200_GROUP_GetDistanceToVisibleParty — always-on distance sensor
- F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal — smell sensor (always on)
- F0209 T0209085_SingleSquareMove — movement triggered by party proximity
- No event queue, no scripting — game loop continuously re-evaluated same sensors
- Triggers: party enters aggro range -> creature moves toward party
- No timed triggers, no conditional triggers beyond distance/smell

DM1 has no event system. Everything is polled each tick.

## DM2: Event-Driven Triggers
DM2 introduces a layered event system:

### 1. Timer Events (c_tim_proc.cpp, c_timer.cpp)
- Time-based triggers for delayed actions
- DM2_CREATURE_KILL_ON_TIMER_POSITION: creature dies at tile after timer
- DM2_CREATURE_EXPLODE_OR_SUMMON: timed self-destruct / spawn
- Creature w_02 is a timerid (linked to creature.h w_02)
- Timer events processed asynchronously — not polled each frame

### 2. Creature Action Events (via DM2_PROCEED_CCM)
- The `b_1a` byte is the current action state — triggers fire when state machine transitions
- Action completion triggers next action automatically
- No explicit event registration — the `b_1a` value IS the event state

### 3. UI / Interaction Events (c_events.cpp)
- DM2_CLICK_ITEM_SLOT: item slot click
- DM2_HANDLE_UI_EVENT: UI interaction
- Champions interact with dungeon objects (doors, chests, levers)
- Creatures respond to party actions through CCM state machine

### 4. Dungeon Events / Actuators
From c_ai.cpp DM2_PROCEED_XACT_62:
- DM2_FIND_TILE_ACTUATOR(x, y, ...): Find pressure plates, switches
- DM2_GET_WALL_TILE_ANYITEM_RECORD(...): Wall-mounted triggers
- Tiles and walls can have event records attached
- Creatures can activate wall switches (DM2_CREATURE_ACTIVATES_WALL)
- Champions can trigger floor actuators

## Trigger Data in Creature Records
From c_creature.cpp lines 2817/5866:
```c
RG1P = DOWNCAST(c_aidef, DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD(
    unsignedlong(byte_at(DM2_GET_ADDRESS_OF_RECORD(...), 0x4))
));
```
The aidef (AI definition) is per-creature-type data — behavior is **data-driven**, not hardwired.

## Key Differences: DM1 vs DM2 Triggers
| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Trigger type | Always-on polling sensors | Event-driven + polling hybrid |
| Timed triggers | None | Yes (KILL_ON_TIMER, EXPLODE) |
| Script data | Hardwired in binary | AI spec records in dungeon data |
| Conditional logic | Inline distance/smell checks | Named CCM actions with conditions |
| UI triggers | None | Click events in c_events.cpp |
| Dungeon triggers | None | Tile actuators, wall switches |
| Per-creature variation | Different code paths per type | Same CCM dispatch, AI spec data differs |

## The CCM Bytecode as Script
`DM2_PROCEED_CCM` acts as a **bytecode interpreter** for creature behavior:
- Each CCM** function is a named opcode
- Creature database AI spec determines which opcodes are valid for that creature type
- Creature current state (`b_1a`) is the instruction pointer
- This is a minimal scripting system — new creature behaviors can be authored by changing data records

## Gap: Firestaff V1 Scripting
Firestaff V1 does not yet implement a scripting layer. The CCM dispatch and AI spec queries are in skproject (reference), but Firestaff V1 stubs only implement combat resolution, companion state, and dungeon loading — not the full trigger/script system.
