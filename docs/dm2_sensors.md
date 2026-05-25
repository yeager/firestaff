# DM2 V1 Sensor Types: Floor and Wall Sensors vs DM1

## Overview

Sensors are the **input side** of the trigger system — they detect a condition and activate an actuator. In DM2, sensors are classified by the surface they occupy (floor vs wall) and by the **trigger condition** (who or what activates them).

## Floor Sensors

Floor sensors occupy the ACTUATOR_FLOOR_TYPE__* namespace and are placed on a floor cell. The key floor sensor types:

| Floor Type Hex | Name | Trigger Condition |
|---------------|------|-------------------|
| 0x01 | EVERYTHING | Any entity (party, creature, item) steps on it |
| 0x03 | PARTY | Only party members step on it |
| 0x04 | ITEM | An item is dropped or placed on it |
| 0x07 | CREATURE | A creature steps on it |
| 0x08 | ITEM_POSSESSION | Party member possesses a specific item |
| 0x0B | CREATURE_KILLER | Kills creatures that step on it |
| 0x16 | CROSS_MAP | Cross-map teleport trigger |
| 0x1D | COUNTER | Counter reaches zero |
| 0x1E | INFINITE_TICK_GENERATOR | Infinite periodic firing |
| 0x20 | RELAY_1 | Relay (wired from another actuator) |
| 0x21 | ARRIVAL_DEPARTURE | Party arrives at or departs from location |
| 0x26 | MISSILE_EXPLOSION | Missile lands on it |
| 0x27 | CROSS_SCENE | Scene transition trigger |
| 0x28 | CREATURE_AI_STATE | Creature enters specific AI state |
| 0x2A | ALCOVE_ITEM | Item placed in alcove |
| 0x2E | PARTY_TELEPORTER | Party teleporter |
| 0x30 | SHOP | Party enters shop |
| 0x3A | CREATURE_ANIMATOR | Animates creatures on the tile |
| 0x3B | ITEM_TELEPORTER | Teleports items |
| 0x49 | ITEM_CAPTURE_FROM_CREATURE | Captures item from creature |

**Key insight**: Floor type 0x01 (EVERYTHING) is the universal floor sensor — it fires for any entity that steps on it. This replaces multiple DM1-specific floor tile types.

## Wall Sensors

Wall sensors are attached to wall cells. The primary wall sensor types (from ACTUATOR_TYPE_*):

| Wall Type Hex | Name | Trigger Condition |
|---------------|------|-------------------|
| 0x01 | DM1_WALL_SWITCH | DM1 retro wall switch (facing keypress) |
| 0x17 | 2_STATE_WALL_SWITCH | Toggle switch: on/off state |
| 0x18 | WALL_SWITCH | Standard wall switch (keypress when facing) |
| 0x1A | KEY_HOLE | Key is used on the wall cell |
| 0x46 | PUSH_BUTTON_WALL_SWITCH | Push-button style wall switch |
| 0x26 | SWITCH_SIGN_FOR_CREATURE | Switch visible to creatures |

**Activation**: Wall switches are activated by the player facing the wall and pressing a key. DM2 tracks the direction the party is facing.

## DM1 Sensor Model (Comparison)

DM1 had **no generic sensor system**. Sensors were fused to specific tile types:

- **Pressure plates**: A specific floor tile graphic with a hardwired depress-on-step behavior
- **Wall switches**: A specific wall tile graphic with a hardwired activate-on-keypress behavior
- **Pit traps**: A floor tile that opened on step or timer; not a separate sensor from the trap itself
- **Item squares**: A floor tile that detected specific item types (very limited)

Each DM1 sensor was **compiled logic** — changing its behavior required modifying the game binary.

## DM2 Sensor Design Improvements

1. **Separation of sensor from effect**: The sensor (floor/wall type) is separate from the actuator type. A floor pressure plate can trigger any actuator effect, not just a fixed behavior.

2. **Multi-entity floor types**: DM2 floor sensors distinguish between PARTY (0x03), CREATURE (0x07), ITEM (0x04), and ITEM_POSSESSION (0x08). DM1 had no such distinction.

3. **Wall switch diversity**: DM1 had essentially one wall switch type. DM2 has at least 4 variants: 2-state toggle, push-button, standard, and key-hole.

4. **Possession-based triggers**: ACTUATOR_FLOOR_TYPE__ITEM_POSSESSION (0x08) is unique to DM2 — activates when a party member carrying a specific item steps on it. Enables puzzles like bring the torch to this square.

5. **Cross-map and arrival sensors**: ARRIVAL_DEPARTURE (0x21) and CROSS_MAP (0x16) allow global event chains impossible in DM1.

## Sensor-to-Actuator Binding

DM2 separates **sensor** (what triggers) from **actuator** (what happens). In GDED, you place an actuator on a cell and configure both the floor/wall type (sensor) and actuator type (effect). Separate UI pages in EdPIActu1.cpp and EdPIActu2Page.cpp handle these two concerns.

## Floor Decoration vs Wall Decoration

The visual representation of a sensor/actuator cell is stored as a floor or wall decoration offset:
- DM2_GET_FLOOR_DECORATION_OF_ACTUATOR(ptr) — returns the floor graphic index
- DM2_GET_WALL_DECORATION_OF_ACTUATOR(ptr) — returns the wall graphic index

These are separate from the base tile, allowing the sensor overlay (switch plate, floor switch) to be drawn on top of any underlying floor or wall tile.
