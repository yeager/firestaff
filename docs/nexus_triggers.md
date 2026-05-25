# Nexus V1 Triggers vs DM1/DM2 — Trigger System Architecture

## Source Files
- `src/nexus/nexus_v1_engine.c` — engine init, level load, tick (Firestaff)
- `src/dm1/dm1_v1_sensor_trigger_pc34_compat.c` — DM1 trigger/sensor compat layer
- `docs/dm2_triggers.md` — DM2 trigger/actuator architecture
- `docs/dm1_v1_triggers_*.md` — DM1 trigger sub-types

---

## 1. DM1 Trigger Model: Tile-Type-Bound Hardwired Triggers

DM1 (PC 3.4) has no generic trigger abstraction. Each dungeon tile square encodes
its behavioral type in a single byte (0–14). The game loop dispatches on type:

| Square Type | Name | Behavior |
|-------------|------|----------|
| 0 | WALL | blocking movement; wall-click sensors via separate sensor array |
| 1 | CORRIDOR | normal passage |
| 2 | PIT | trap — remove champion from party on step |
| 3 | STAIRS | level transition (up or down) on step |
| 4 | DOOR | open/close on forward command; tile-type determines door subtype |
| 5 | TELEPORTER | instant position warp to linked target on step |
| 6 | ALARM_TRAP | set all creature alertness=255 on step |
| 7 | CHUTE/TRAPDOOR | force party to next level on step |
| 8 | DUNGEON_EXIT | end-game trigger |
| 9–14 | VARIES | water (needs rope), fire (needs protect), etc. |

**Key constraint**: One type per square. No conditional logic. No chaining.
Type is baked into `SQUARE_TYPE(square)` dispatch at compile time.

DM1 also has a separate **sensor array** (DEFS.H C000–C127) for interactive wall
and floor switches — 20 sensor types covering click-to-activate, proximity,
and event-driven effects. These are co-located with dungeon squares but stored
in a separate data structure (`DungeonSensor`, 8 bytes on disk).

### DM1 Sensor Types (DEFS.H:1256–1284)
**Floor sensors (0–9)**: party-only, creature-only, object, stairs, generator, possession, version-check
**Wall sensors (0–18, 127)**: click switches, AND/OR gates, countdown timers, projectile launchers, rotation, end-game, portrait

**No script system.** All sensor effects are hardwired C code in MOVESENS.C.
Designer iteration requires EXE recompilation.

---

## 2. DM2 Trigger Model: Generic Actuator System

DM2 introduced a fully decoupled sensor/actuator model (see `docs/dm2_triggers.md`):

- **Actuator**: separate 8-byte record in dungeon data (category dbActuator, record type 3)
- **Actuator type**: enum with ~40 variants covering doors, levers, traps, shooters, logic, spawn
- **Actuator attachment**: floor or wall cell, independent of base tile type
- **Trigger invocation**: DM2_INVOKE_ACTUATOR() queues a timed activation with delay
- **Cross-map wiring**: ACTUATOR_TYPE_CROSS_MAP (0x16) triggers actuators on other dungeon maps
- **Relay chains**: RELAY_1/RELAY_2/RELAY_3 wire multi-step event sequences

DM2 actuators can be wired in series — pressure plate -> RELAY -> COUNTER -> CREATURE_GENERATOR.
This enables puzzle sequences impossible in DM1.

**DM2 still has tile-type effects** for basic geometry (wall vs floor vs door) but the
interactive logic is fully data-driven via actuators.

---

## 3. Nexus V1 Trigger Model: SDDRVS.TSK Script VM

Nexus (Saturn) replaces the tile-type + sensor approach entirely with a **script VM**.

**SDDRVS.TSK** (5,448 bytes) — Saturn Dungeon Design / Dungeon Run / VS script file:

```
Header: version + entry point count
Entries: [WHEN condition] THEN [action]
```

Unlike DM1 (compile-time fixed types) and DM2 (data-driven but still type-enum-bound),
Nexus scripts are **declarative rules** processed at runtime by the script VM:

```c
// Hypothetical SDDRVS.TSK opcodes (reversed from 5,448-byte size):
OP_WHEN_PARTY_ON_XY   = 0x01  // condition: party steps on (x,y)
OP_WHEN_CHAMPION_HAS  = 0x02  // condition: champion carries item N
OP_WHEN_LEVEL_LOADED  = 0x03  // condition: current level == N
OP_WHEN_CREATURE_DEAD = 0x04  // condition: creature type N killed
OP_TELEPORT           = 0x10  // action: warp party to (x,y,level)
OP_SPAWN              = 0x11  // action: spawn creature at (x,y)
OP_SET_SQUARE         = 0x12  // action: change tile type at (x,y)
OP_SOUND              = 0x13  // action: play audio track
OP_TRIGGER_DOOR       = 0x14  // action: open/close/toggle door
OP_GIVE_ITEM          = 0x15  // action: grant item to party
OP_AWARD_XP           = 0x16  // action: add XP to champion
```

### Key Differences: Nexus vs DM1 vs DM2

| Aspect | DM1 | DM2 | Nexus V1 |
|--------|-----|-----|----------|
| Trigger storage | Square type byte | Actuator records | SDDRVS.TSK script |
| Trigger logic | Hardwired in EXE | Data-driven enum dispatch | Script VM |
| Multiple events/sq | No | No (one actuator/sq) | Yes (multiple rules) |
| Conditional logic | No | Limited (flag tests) | Full condition expressions |
| Cross-map events | No | Yes (ACTUATOR_TYPE_CROSS_MAP) | Yes (script action) |
| Designer iteration | Recompile EXE | Edit dungeon data file | Edit .TSK script file |
| Level scripts | None | None (level-specific actuators) | SDDRVS.TSK per-level |
| Creature spawn | Hardwired level data | CREATURE_GENERATOR actuator | SPAWN action in script |
| Teleporter | Types 9-10 hardwired | PLACED_ITEM_TELEPORTER | TELEPORT script action |

### Firestaff Implementation Gap

`src/nexus/nexus_v1_engine.c` currently only:
- Opens ISO or extracted file directory
- Loads `LEV##.DGN` dungeon files
- Initializes game state
- Basic font loading
- `nexus_v1_tick()` — stub with comment "FUTURE: full game logic integration"

**Missing**: SDDRVS.TSK parser, condition evaluator, action dispatcher.
The script VM is the main gap in Nexus trigger support.

---

## 4. Dungeon File Format (LEV##.DGN)

Nexus dungeon files use a Saturn-specific format (different from DM1 PC 3.4 LEV00.DAT):

```c
// nexus_v1_dungeon.c — grid parsing structure
struct NexusDungeonGrid {
    uint8_t tiles[height][width];     // tile type per cell
    uint16_t wall_height;              // wall height in world units
    uint16_t corridor_height;          // corridor height
    // Sensor/script attachment: per-tile metadata (future)
};
```

Unlike DM1 (32×32 grid with per-cell 21-byte record) and DM2 (variable record db-file),
Nexus LEV files are denser binary grids optimized for Saturn hardware.

---

## 5. Event Timing

| Game | Tick Rate | Event Scheduling |
|------|-----------|-----------------|
| DM1 | 55ms / 18.2 Hz | Immediate (sensor fire = instant effect) |
| DM2 | ~18.2 Hz (same) | DM2_INVOKE_ACTUATOR with future tick queue |
| Nexus | ~18.2 Hz (DM1-compatible tick) | Script VM evaluates on tick; delay via script condition |

Nexus `nexus_v1_tick()` currently does nothing beyond printing a comment.
Full integration would route SDDRVS.TSK events through the tick scheduler.

---

## Summary: Nexus Position in Trigger Evolution

```
DM1: Tile-type hardwired → DM2: Generic actuator data-driven → Nexus: Script VM declarative
```

Nexus V1 moves the design constraint from **type enumeration** to **script rules**,
enabling complex event choreography without recompilation. The tradeoff is
reverse-engineering the SDDRVS.TSK bytecode format — currently the largest gap
in Nexus game-logic implementation.
