# Nexus V1 Scripting — SDDRVS.TSK Script VM vs DM1/DM2

## Source Files
- `docs/nexus_sensors.md` — existing Nexus sensor overview with SDDRVS.TSK mention
- `src/nexus/nexus_v1_engine.c` — nexus_v1_tick stub, TODO comment
- `docs/dm2_ai_scripting.md` — DM2 scripting reference
- `docs/dm1_v1_triggers_conditional.md` — DM1 conditional trigger (C005 AND/OR gate)
- `docs/dm1_v1_triggers_events.md` — DM1 event-driven triggers (C006 countdown)

---

## 1. DM1: No Script System

DM1 has zero script capability. All game logic is hardwired in C:

- Tile types encode behavior directly (square type byte -> switch in game loop)
- Wall/floor sensors: static type-enum dispatch (C000-C127 in DEFS.H)
- No conditional expressions, no action sequences, no variable state
- C005 AND/OR gate is the closest thing to logic — purely boolean (all-or-nothing per sensor count)
- C006 countdown timer: one hardwired timer per wall-event sensor
- Designer iteration requires recompiling the EXE

### DM1 Trigger Hardwiring Examples
```c
// Door open (game.c or collision handler)
if (squareType == 4 && party_facing == direction) {
    toggle_door_state(x, y);
}

// Teleporter (movement handler)
if (squareType == 5) {
    warp_party(targetX, targetY, targetLevel);
}

// Alarm trap (movement handler)
if (squareType == 6) {
    for (all creatures) creature.alertness = 255;
}
```

Every dungeon interaction is compile-time fixed behavior. No data file controls logic.

---

## 2. DM2: Data-Driven Actuators (Still Not Scripting)

DM2 advances to data-driven actuators but still has no scripting:

- Actuator type enum (~40 types) dispatched in a switch statement
- Each type has fixed opcode handler: case 0x08 -> MISSILE_SHOOTER_exec()
- No condition expressions, no action sequences per actuator
- Logic chains via relay/wiring: RELAY_1 -> COUNTER -> RELAY_2 -> CREATURE_GENERATOR
- Designer edits dungeon data file (category/record database) not code

DM2 ai_scripting.md covers creature AI scripting (DM2 Champions of Darkness scripting
language), not the dungeon trigger/actuator system.

### DM2 Actuator Execution
```c
switch (actuator.type) {
    case 0x08: MISSILE_SHOOTER_exec(actuator.data); break;
    case 0x2E: CREATURE_GENERATOR_exec(actuator.pos); break;
    case 0x16: CROSS_MAP_exec(actuator.target_map_id, actuator.target_idx); break;
    // ...
}
```

No dynamic condition evaluation, no expressions, no string-based rules.

---

## 3. Nexus V1: SDDRVS.TSK Script VM (True Scripting)

Nexus implements a task/script VM. SDDRVS.TSK (5,448 bytes) is a declarative
rule-based script file processed by the Nexus engine at runtime.

### Script Format (Reversed from File Size and Saturn Dungeon Design Context)

The 5,448-byte file suggests a binary format with repeating rule entries.
Hypothesized structure:

```
Header (16 bytes):
  uint32_t version        // e.g. 1
  uint32_t rule_count     // number of rules in file
  uint32_t entry_offset   // offset to first rule
  uint32_t flags          // global flags

Rule entries (variable, ~40-80 bytes each):
  uint8_t  condition_op    // COND_* condition opcode
  uint8_t  c_x             // condition: mapX
  uint8_t  c_y             // condition: mapY
  uint8_t  c_level         // condition: dungeon level
  uint16_t c_data          // condition operand (item type, creature ID, etc.)
  uint8_t  action_op       // ACT_* action opcode
  uint8_t  a_x             // action: target mapX
  uint8_t  a_y             // action: target mapY
  uint8_t  a_level         // action: target level
  uint8_t  a_data[4]       // action parameters
  uint8_t  delay_ticks     // 0 = immediate, N = delay before fire
  uint8_t  flags           // once_only, active, etc.
  uint8_t  reserved[3]     // padding
```

Rule count estimate: 5448 / ~68 bytes per rule = ~80 rules maximum.

### Condition Opcodes (Hypothetical)
```
COND_PARTY_ON_XY       = 0x20  // party coordinates match c_x, c_y, c_level
COND_PARTY_FACING      = 0x21  // party facing direction == c_data
COND_CHAMPION_HAS      = 0x22  // any champion carries item type c_data
COND_CHAMPION_LEVEL    = 0x23  // champion level >= c_data
COND_LEVEL_LOADED      = 0x24  // current dungeon level == c_data
COND_SQUARE_TYPE       = 0x25  // tile at (c_x,c_y) == c_data
COND_TIME_ELAPSED      = 0x26  // tick counter >= c_data
COND_EVENT_FLAG        = 0x27  // flag[c_data] is set
COND_CREATURE_DEAD     = 0x28  // creature type c_data was killed
COND_PARTY_HAS_GOLD    = 0x29  // party gold >= c_data
```

### Action Opcodes (Hypothetical)
```
ACT_TELEPORT           = 0x30  // warp party to (a_x, a_y, a_level)
ACT_SET_SQUARE_TYPE    = 0x31  // change tile at (a_x,a_y) to a_data
ACT_ROTATE_WALL        = 0x32  // rotate wall at (a_x,a_y) 90 degrees
ACT_OPEN_DOOR          = 0x33  // open door at (a_x,a_y)
ACT_CLOSE_DOOR         = 0x34  // close door at (a_x,a_y)
ACT_TOGGLE_DOOR        = 0x35  // toggle door at (a_x,a_y)
ACT_SPAWN_CREATURE     = 0x40  // spawn creature a_data at (a_x,a_y)
ACT_CREATE_ITEM        = 0x41  // create item a_data at (a_x,a_y)
ACT_GENERATE_OBJECT    = 0x42  // projectile launch at (a_x,a_y)
ACT_PLAY_TRACK         = 0x50  // play CD audio track a_data
ACT_PLAY_VOICE         = 0x51  // play voice line a_data
ACT_PLAY_EFFECT        = 0x52  // play sound effect a_data
ACT_DAMAGE_PARTY       = 0x60  // deal a_data HP to party
ACT_HEAL_PARTY         = 0x61  // heal a_data HP to champion
ACT_GIVE_GOLD          = 0x62  // award a_data gold
ACT_GIVE_ITEM          = 0x63  // grant item a_data to party
ACT_AWARD_XP           = 0x65  // add a_data XP to champion
ACT_SET_FLAG           = 0x70  // set flag[a_data]
ACT_CLEAR_FLAG         = 0x71  // clear flag[a_data]
ACT_WAIT_TICKS         = 0x72  // delay a_data ticks before next action
ACT_END_GAME           = 0x7F  // trigger end game sequence
```

---

## 4. Cross-Game Scripting Comparison

| Aspect              | DM1            | DM2                | Nexus V1              |
|---------------------|----------------|--------------------|-----------------------|
| Scripting paradigm  | None           | None               | Task/Rule VM (SDDRVS.TSK) |
| Script storage      | N/A            | N/A                | Separate .TSK file    |
| Condition logic     | Boolean types  | Boolean types      | Full expressions      |
| Action sequences    | Single effect  | Relay chains       | Multi-action rules    |
| Variables/state     | None           | Counter/timer only | 256 flags + counters  |
| Designer iteration  | Recompile EXE  | Edit dungeon data  | Edit .TSK file        |
| Script file format  | N/A            | N/A                | Binary rule entries   |
| Max rules           | N/A            | N/A                | ~80 (from 5,448 bytes)|

---

## 5. Firestaff Scripting Implementation

### Current State
- `nexus_v1_engine.c`: nexus_v1_tick is stub
- SDDRVS.TSK: identified but not parsed
- No condition evaluator, no action dispatcher

### Required Components for Script VM

```c
// Script rule (parsed from SDDRVS.TSK)
typedef struct {
    uint8_t  condition_op;
    uint8_t  c_x, c_y, c_level;
    uint16_t c_data;
    uint8_t  action_op;
    uint8_t  a_x, a_y, a_level;
    uint8_t  a_data[4];
    uint8_t  delay_ticks;
    uint8_t  flags;  // ONCE_ONLY | ACTIVE
} ScriptRule;

// Script VM state
typedef struct {
    ScriptRule *rules;
    int         rule_count;
    uint8_t     flags[256];       // event flags
    uint32_t    tick_counter;     // elapsed ticks since load
    ScheduledAction *scheduled;   // priority queue
    int         scheduled_count;
} ScriptVM;

// Core VM functions
int  script_vm_init(ScriptVM *vm, const uint8_t *tsk_data, size_t tsk_size);
void script_vm_tick(ScriptVM *vm, GameState *gs);
void script_vm_dispatch_action(ScriptVM *vm, const ScriptRule *rule, GameState *gs);
int  script_vm_evaluate_condition(ScriptVM *vm, const ScriptRule *rule, GameState *gs);
void script_vm_schedule(ScriptVM *vm, const ScriptRule *rule, uint32_t fire_tick);
```

### Reverse-Engineering Next Steps

1. **Hexdump SDDRVS.TSK**: Look for opcode patterns, repeating entries, ASCII strings
2. **Identify entry boundaries**: Find uniform record size by scanning for patterns
3. **Cross-reference with LEV##.DGN**: Script should reference specific dungeon coordinates
4. **Look for magic bytes**: Header version field at offset 0
5. **Compare multiple dungeons**: Different LEV files may reference different TSK subsets

### Implementation Priority
```
P0: Parse header, read rule_count, verify entry boundaries
P1: Implement condition evaluators (COND_PARTY_ON_XY, COND_LEVEL_LOADED)
P2: Implement action dispatchers (ACT_TELEPORT, ACT_OPEN_DOOR)
P3: Add scheduled action queue (delay_ticks support)
P4: Add flag state (ACT_SET_FLAG, ACT_CLEAR_FLAG)
P5: Periodic script rules (TICK_GENERATOR equivalent)
```

---

## 6. Why Scripting Matters for Firestaff

Without SDDRVS.TSK reverse-engineering, Nexus V1 is a dungeon renderer.
With the script VM:
- Dungeon puzzles work (pressure plates, levers, timed doors)
- Creature spawn events fire correctly
- Level transitions work (ACT_TELEPORT with level change)
- Sound/music triggers tied to gameplay events
- Full gameplay loop: exploration -> trigger -> consequence -> reward

The script VM is the bridge from "renders dungeon tiles" to "plays the game."
