# Nexus V1 Actuators — Door/Lever/Trap Mechanics vs DM1/DM2

## Source Files
- `docs/dm2_actuators.md` — DM2 actuator taxonomy (~40 types)
- `docs/nexus_sensors.md` — existing Nexus sensor/trigger overview

---

## 1. DM1 Actuator Model (Implicit — No Actuator Concept)

DM1 has no actuator abstraction. Doors, levers, and traps are tile-type behaviors
hardwired in the game loop.

### Door Behavior (Square Type 4)
- Open: party faces door + presses forward -> toggle open/closed
- Door state: open | closed | mid-animation
- DM1 door step behavior constants in dm1_v1_collision_door_pc34_compat.c

### Trap/Alarm Behavior (Square Type 6)
- On step: all creatures alertness = 255 (fully alerted)
- Chute/trapdoor (type 7): forces party to next level
- Pit (type 2): removes champion from party

### Lever/Switch (Wall Sensor C001-C004)
- C001: click -> fire (no object required)
- C002: click while holding ANY object
- C003: click while holding SPECIFIC object (data = object type)
- C004: click -> consume object + fire
- Projectile launchers (C007-C015): wall click -> fire projectile

No relay chains. No conditional logic. No cross-square sequencing.

---

## 2. DM2 Actuator Model (Explicit — Category 3 Records)

DM2 formalizes the actuator concept. Key actuator types:

Door Control: 0x17 2_STATE_WALL_SWITCH, 0x18 WALL_SWITCH, 0x46 PUSH_BUTTON, 0x1A KEY_HOLE
Shooters: 0x08 MISSILE_SHOOTER, 0x09 WEAPON_SHOOTER, 0x0E ITEM_SHOOTER
Spawn: 0x2E CREATURE_GENERATOR, 0x3C ITEM_GENERATOR, 0x22 FLYING_ITEM_CATCHER
Logic: 0x1D COUNTER, 0x1E TICK_GENERATOR, 0x20 RELAY_1, 0x3D RELAY_2, 0x16 CROSS_MAP

DM2_INVOKE_ACTUATOR queues timed activations. Door state via DoorBit09/DoorBit10.

---

## 3. Nexus V1 Actuator Model (SDDRVS.TSK Script Actions)

Nexus has NO pre-defined actuator type enum. Actions are script opcodes in SDDRVS.TSK.

Hypothetical SDDRVS.TSK Action Opcodes:

Movement: ACT_TELEPORT(0x30), ACT_SET_SQUARE_TYPE(0x31), ACT_ROTATE_WALL(0x32),
  ACT_OPEN_DOOR(0x33), ACT_CLOSE_DOOR(0x34), ACT_TOGGLE_DOOR(0x35)
Spawn: ACT_SPAWN_CREATURE(0x40), ACT_CREATE_ITEM(0x41), ACT_GENERATE_OBJECT(0x42)
Audio: ACT_PLAY_TRACK(0x50), ACT_PLAY_VOICE(0x51), ACT_PLAY_EFFECT(0x52)
Party: ACT_DAMAGE_PARTY(0x60), ACT_HEAL_PARTY(0x61), ACT_GIVE_GOLD(0x62),
  ACT_GIVE_ITEM(0x63), ACT_TAKE_ITEM(0x64), ACT_AWARD_XP(0x65)
Logic: ACT_SET_FLAG(0x70), ACT_CLEAR_FLAG(0x71), ACT_WAIT_TICKS(0x72),
  ACT_END_GAME(0x7F)

Nexus doors controlled by script. Levers via COND_PARTY_FACING + COND_PARTY_ON_XY.

---

## 4. Cross-Game Actuator Comparison

| Aspect          | DM1                  | DM2                     | Nexus V1           |
|-----------------|----------------------|-------------------------|---------------------|
| Door control    | Square type 4        | Door tile+0x17/0x18     | Tile+ACT_OPEN/CLOSE|
| Lever/switch    | Wall sensor C001-C004| Actuator 0x17/0x18/0x46 | COND+ACT script     |
| Trap/trapdoor   | Square type 7        | Actuator shooter        | ACT_DAMAGE+WARP     |
| Creature spawn  | Hardwired level data | Actuator 0x2E           | ACT_SPAWN_CREATURE  |
| Logic chains    | None                 | RELAY+COUNTER           | Script AND/OR+flags |
| Timer events    | C006 countdown       | TICK_GENERATOR 0x1E     | ACT_WAIT_TICKS      |

---

## 5. Firestaff Actuator Implementation

Current: nexus_v1_dungeon.c parses grid only. No door state machine.

Required: SDDRVS.TSK parser, door state struct, action dispatcher, timer queue, flag store.

Priority: Door control -> Teleport -> Spawn -> Sound -> Logic.
