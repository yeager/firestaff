# DM2 V1 Actuators: Door, Lever, and Trap Actuators

## Overview

Actuators are the **output/effect side** of the DM2 trigger system. When a sensor fires, the actuator executes its programmed effect. DM2 has ~40 actuator types covering doors, levers/traps, projectiles, teleporters, creature spawners, and logic relays.

**Important architectural note**: Doors in DM2 are **not themselves actuators**. Doors are a **tile record type** (separate from floor/wall). Actuators attached to a door tile **control the door state** by sending open/close/toggle commands.

## Door Actuator Control

When an actuator activates a door, it sends one of three action types:

| Action Type | Value | Meaning |
|------------|-------|---------|
| ACTMSG_OPEN_SET | 0x00 | Force door open |
| ACTMSG_CLOSE_CLEAR | 0x01 | Force door closed |
| ACTMSG_TOGGLE | 0x02 | Toggle current state |

The door has an **internal state machine** via DoorBit09:
- 0 = closing direction
- 1 = opening direction
- 2 = mid-state

DoorBit10 holds the remaining timer for the current animation. When it reaches 0 the door has finished moving.

**Step-behavior modes**: The door tile itself has a step behavior property (C00_ACTEFFECT_STEP_* through C06_ACTEFFECT_STEP_*) which determines how the door reacts to party stepping on it or away from it. These are separate from actuator control.

## Trap and Shooter Actuators

These actuator types fire projectiles or deal damage:

| Type Hex | Name | Description |
|----------|------|-------------|
| 0x07 | SOME_SHOOTER | Generic shooter/trap floor (unclear if implemented in final DM2) |
| 0x08 | MISSILE_SHOOTER | Fires a missile projectile |
| 0x09 | WEAPON_SHOOTER | Fires a weapon (e.g. arrow) |
| 0x0A | MISSILE_SHOOTER_2 | Variant missile shooter |
| 0x0E | ITEM_SHOOTER | Shoots a physical item |
| 0x0F | ITEM_SHOOTER_X2 | Variant (possibly unimplemented) |

Shooter actuators are placed on a floor cell. When triggered, they launch a projectile in a configured direction.

## Lever / Switch Actuators (Wall-Mounted)

Wall-mounted actuators that respond to player keypress:

| Type Hex | Name | Description |
|----------|------|-------------|
| 0x01 | DM1_WALL_SWITCH | DM1 retro wall switch (facing + keypress) |
| 0x17 | 2_STATE_WALL_SWITCH | Toggle: alternate between two states |
| 0x18 | WALL_SWITCH | Standard single-state wall switch |
| 0x1A | KEY_HOLE | Activated by applying a specific key item |
| 0x46 | PUSH_BUTTON_WALL_SWITCH | Push-button style (requires release to activate) |
| 0x26 | SWITCH_SIGN_FOR_CREATURE | A switch whose state is visible to creature AI |

**Activation method**: Player faces the wall and presses a key. The 2-state wall switch (0x17) toggles between on/off each activation. Push-button switch (0x46) activates on press.

## Logic and Wiring Actuators

These connect trigger chains without direct in-world effect:

| Type Hex | Name | Description |
|----------|------|-------------|
| 0x1D | COUNTER | Counts down; fires when it reaches zero (multi-step puzzles) |
| 0x1E | TICK_GENERATOR | Generates periodic ticks that drive multi-step timing |
| 0x20 | RELAY_1 | Passes through activation to wired destination |
| 0x3D | RELAY_2 | Second relay type |
| 0x45 | RELAY_3 | Third relay type |
| 0x16 | CROSS_MAP | Triggers an actuator on a different dungeon map |
| 0x21 | ARRIVAL_DEPARTURE | Fires when party arrives at or leaves a location |
| 0x43 | INVERSE_FLAG | Inverts a bit flag before passing on |
| 0x44 | TEST_FLAG | Tests a flag; fires only if flag is in a specific state |

**Relay chains**: Complex multi-step events are built by wiring actuators in series. Example: pressure plate -> RELAY_1 -> COUNTER (decrement) -> RELAY_2 -> CREATURE_GENERATOR. Only when counter reaches zero does the creature spawn.

## Spawn Actuators

| Type Hex | Name | Description |
|----------|------|-------------|
| 0x2E | CREATURE_GENERATOR | Spawns a creature at the actuator location |
| 0x3C | ITEM_GENERATOR | Generates an item at the actuator location |
| 0x22 | FLYING_ITEM_CATCHER | Catches/stops a flying item |
| 0x23 | FLYING_ITEM_TELEPORTER | Teleports a flying item |
| 0x47 | ITEM_CAPTURE | Captures an item placed on the floor |

CREATURE_GENERATOR uses the current position of the actuator (not data in the record) as the spawn point.

## Visual / Ornate Actuators

| Type Hex | Name | Description |
|----------|------|-------------|
| 0x2C | ORNATE_ANIMATOR | Plays an animation sequence on the tile |
| 0x32 | ORNATE_ANIMATOR_2 | Variant ornate animator |
| 0x41 | ORNATE_STEP_ANIMATOR | Animates when party steps on it |
| 0x3F | SHOP_PANEL | Displays the shop interface |

These actuators drive the visual layer rather than triggering game events.

## Item-Specific Actuators

| Type Hex | Name | Description |
|----------|------|-------------|
| 0x03 | ITEM_WATCHER | Watches for a specific item type on the floor |
| 0x15 | CHARGED_ITEM_WATCHER | Watches for a charged/wanded item |
| 0x04 | DM1_ITEM_EATER | DM1 retro — consumes a specific item |
| 0x40 | ITEM_RECYCLER | Destroys items placed on it; optionally generates different items |

ITEM_RECYCLER is notable for puzzles: item placed on it is destroyed and triggers downstream effects.

## Champion-Specific Actuators

| Type Hex | Name | Description |
|----------|------|-------------|
| 0x7E | RESURECTOR | Champion resurrection cell |
| 0x7F | CHAMPION_MIRROR | DM1 champion mirror (retro) |

## Actuator Data Structure

Actuators occupy record category DB_CATEGORY_ACTUATOR (0x03) in the dungeon data, or DB_CATEGORY_SIMPLE_ACTUATOR (0x02) for 4-byte simple variants:

- **Simple actuator**: 4 bytes (DB_SIZE_SIMPLE_ACTUATOR)
- **Full actuator**: 8 bytes (DB_SIZE_ACTUATOR)

The 8 bytes encode: type (high bits of word at offset +0x4), target coordinates/actuator ID (offset +0x6), and type-specific data. DM2_INVOKE_ACTUATOR() extracts these fields and queues a timed activation.

## DM1 Comparison

DM1 had no separate actuator concept. Doors, levers, and traps were:

- **Doors**: Self-contained tile records with step-behavior hardwired per door
- **Levers**: A wall tile with fixed activate behavior compiled into the binary
- **Traps**: A floor tile with hardwired open/close/damage behavior

DM2 broke these apart into the generic sensor + actuator model, so the same door tile can be controlled by any combination of floor/wall sensors and logic chains.
