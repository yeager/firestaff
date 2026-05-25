# DM2 V1 Trigger System vs DM1

## Overview

DM1 used **hardwired, tile-type-bound** triggers. DM2 introduced a **generic actuator system** where any floor or wall cell can hold an actuator independently of its base tile type.

## DM1 Trigger Model

In DM1, triggers were baked into specific map elements:

- **Wall switches** — fixed to wall tiles with switch graphics; activated by facing the wall and pressing a key
- **Pressure plates / floor switches** — floor tiles that depress when stepped on
- **Pit traps** — floor tiles that open on step or timer
- **Door opening** — handled per-door with hardwired event chains

DM1 had **no generic trigger abstraction** — each dungeon designer had a small fixed set of trigger types. The trigger logic was compiled into the game binary.

## DM2 Trigger Model (Actuator System)

DM2 replaced the hardwired approach with a **data-driven actuator architecture**:

- An actuator is a **separate data record** stored in the dungeon data file (category dbActuator, record type 3)
- An actuator is attached to a **floor or wall cell**, not a tile type
- Actuators have **8 bytes of data** (DB_SIZE_ACTUATOR) or 4 bytes (DB_SIZE_SIMPLE_ACTUATOR)
- Each actuator has a **type** (from ACTUATOR_TYPE_* enum) and **per-type data fields**
- An actuator can trigger **another actuator** on the same or different map (cross-map)
- Actuators can be **wired in series/relay chains** via relay actuator types

### Actuator Type Taxonomy (key families)

| Type Hex | Name | Category |
|----------|------|----------|
| 0x01 | DM1_WALL_SWITCH | DM1 retro |
| 0x03 | ITEM_WATCHER | Floor |
| 0x04 | DM1_ITEM_EATER | DM1 retro |
| 0x05 | DM1_BITFIELDS_TRIGGER | DM1 retro |
| 0x06 | DM1_COUNTER | DM1 retro |
| 0x08 | MISSILE_SHOOTER | Projectile |
| 0x09 | WEAPON_SHOOTER | Projectile |
| 0x0E | ITEM_SHOOTER | Projectile |
| 0x15 | CHARGED_ITEM_WATCHER | Floor |
| 0x16 | CROSS_MAP | Teleport/Wiring |
| 0x17 | 2_STATE_WALL_SWITCH | Wall |
| 0x18 | WALL_SWITCH | Wall |
| 0x1A | KEY_HOLE | Wall |
| 0x1D | COUNTER | Logic |
| 0x1E | TICK_GENERATOR | Timing |
| 0x20 | RELAY_1 | Wiring |
| 0x21 | ARRIVAL_DEPARTURE | Wiring |
| 0x22 | FLYING_ITEM_CATCHER | Floor |
| 0x23 | FLYING_ITEM_TELEPORTER | Floor |
| 0x26 | SWITCH_SIGN_FOR_CREATURE | Wall |
| 0x2A | ALCOVE_ITEM_TRAP | Trap |
| 0x2C | ORNATE_ANIMATOR | Visual |
| 0x2E | CREATURE_GENERATOR | Spawn |
| 0x31 | WORK_TIMER | Timing |
| 0x3B | PLACED_ITEM_TELEPORTER | Teleport |
| 0x3C | ITEM_GENERATOR | Spawn |
| 0x3D | RELAY_2 | Wiring |
| 0x3F | SHOP_PANEL | Shop |
| 0x40 | ITEM_RECYCLER | Utility |
| 0x46 | PUSH_BUTTON_WALL_SWITCH | Wall |
| 0x47 | ITEM_CAPTURE | Floor |
| 0x7E | RESURECTOR | Champion |
| 0x7F | CHAMPION_MIRROR | DM1 retro |

### Trigger Invocation Path

1. Player/creature activates a sensor (steps on floor, uses wall switch, etc.)
2. DM2_INVOKE_ACTUATOR is called with the actuator pointer, a flag value, and a delay
3. The function computes a **future game-tick** when the actuator fires (based on delay parameter)
4. The actuator is placed in a **pending queue**
5. DM2_PROCESS_ACTUATOR_TICK_GENERATOR() runs each game tick, dequeues expired actuators
6. For each expired actuator: reads its type-specific data, executes the effect (fire projectile, toggle door, spawn creature, etc.)

### Cross-Map Wiring

ACTUATOR_TYPE_CROSS_MAP (0x16) allows one actuator to trigger another map's actuator. This enables global event chains across level transitions.

## Key Differences Summary

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Trigger abstraction | Hardwired tile types | Generic actuator records |
| Trigger placement | Bound to specific tile types | Any floor or wall cell |
| Number of trigger types | ~5 (switch, plate, trap, door, item) | 40+ actuator types |
| Cross-map triggers | Not possible | Via CROSS_MAP actuator |
| Delay/timer support | Very limited | TICK_GENERATOR, WORK_TIMER |
| Event chaining | Single-step | Multi-step relay chains |
| Data location | Compiled in binary | Dungeon data file |
