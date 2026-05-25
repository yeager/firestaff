# Nexus V1 Sensors — Floor/Wall Sensor Types vs DM1/DM2

## Source Files
- `src/dm1/dm1_v1_sensor_trigger_pc34_compat.c` — DM1 sensor system (F0720–F0726)
- `docs/dm1-v1-dungeon-audit/dungeon_sensors.md` — DM1 sensor struct (MEDIA016 layout)
- `docs/dm2_sensors.md` — DM2 sensor/actuator separation
- `docs/nexus_sensors.md` — existing Nexus sensor overview

---

## 1. DM1 Floor Sensors (F0722 — MOVESENS.C F0276)

DM1 floor sensors are evaluated when a thing is added to or removed from a dungeon square.
The evaluator in `F0722_SENSOR_EvaluateFloor_Compat` dispatches on `sensorType`:

| Type | Name | Trigger Condition |
|------|------|-----------------|
| C000 | DISABLED | Never fires |
| C001 | THERON_PARTY_CREATURE_OBJECT | Any non-party/creature/object thing enters square |
| C002 | THERON_PARTY_CREATURE | Party or creature enters square |
| C003 | PARTY | Party enters; optionally directional (data=ordinal(dir)) |
| C004 | OBJECT | Specific object type enters (sensorData = type) |
| C005 | PARTY_ON_STAIRS | Party steps on stairs square |
| C006 | GROUP_GENERATOR | **Event-triggered only** — not evaluated on floor events |
| C007 | CREATURE | Non-party creature enters square |
| C008 | PARTY_POSSESSION | Party enters while carrying specific item (data=itemType) |
| C009 | VERSION_CHECKER | Party enters on DM1 V1 (≤version 20) — demo protection |

### Floor Sensor Evaluation Logic (F0276 per-type)

```
C001: skip if P0591_B_PartySquare || L0772_B_SquareContainsObject || L0773_B_SquareContainsGroup
C002: skip if thingType > GROUP || partySquare || hasGroup
C003: skip if thingType != PARTY || championCount==0; directional: data==ordinal(dir)
C004: skip if data != F0032_OBJECT_GetType(thing) || hasSameTypeObj
C005: skip if thingType != PARTY || squareType != STAIRS
C006: skip entirely (group generator is event-triggered, not floor-reactive)
C007: skip if thingType > GROUP || thingType == PARTY || hasGroup
C008: skip if thingType != PARTY; trigger=partyHasObjectType(data)
C009: skip if thingType != PARTY || !isAdd || partySquare; trigger=(data <= 20)
```

### HOLD Effect Resolution (F0728)
`DM1_EFFECT_HOLD` (value 3) resolves to SET or CLEAR based on `triggerActive`:
```c
HOLD: triggerActive ? DM1_EFFECT_SET : DM1_EFFECT_CLEAR
```
Used for toggle switches that must remember state.

### Once-Only Sensors (F0272 line ~1180)
If `sensor->onceOnly` is set, the sensor is disabled (M044_SET_TYPE_DISABLED) after firing.
This prevents repeat triggers on permanent events.

---

## 2. DM1 Wall Sensors (F0723 — MOVESENS.C F0275)

Wall sensors are evaluated on player wall-click (face wall + keypress):
`F0723_SENSOR_EvaluateWall_Compat` dispatches on `sensorType`:

| Type | Name | Trigger Condition |
|------|------|-----------------|
| C001 | CLICK | Any wall click |
| C002 | CLICK_WITH_ANY_OBJECT | Click while party holds any object |
| C003 | CLICK_WITH_SPECIFIC_OBJECT | Click while party holds specific object (data=type) |
| C004 | CLICK_WITH_SPECIFIC_OBJECT_REMOVED | Same, plus object consumed after trigger |
| C005 | AND_OR_GATE | Event-triggered (not click-referenced) |
| C006 | COUNTDOWN | Event-triggered, countdown timer (C006 in TIMELINE.C) |
| C007 | SINGLE_PROJECTILE_LAUNCHER_NEW_OBJECT | Click → fire projectile, create object at target |
| C008 | SINGLE_PROJECTILE_LAUNCHER_EXPLOSION | Click → fire explosion projectile |
| C009 | DOUBLE_PROJECTILE_LAUNCHER_NEW_OBJECT | Click → fire 2 projectiles, create object |
| C010 | DOUBLE_PROJECTILE_LAUNCHER_EXPLOSION | Click → fire 2 explosion projectiles |
| C011 | CLICK_REMOVED_ROTATE | Click → remove sensor, rotate wall |
| C012 | OBJECT_GENERATOR_ROTATE | Click → generate object, rotate wall |
| C013 | SINGLE_OBJECT_STORAGE_ROTATE | Click → store one object, rotate wall |
| C014 | SINGLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT | Projectile launcher, target is a square object |
| C015 | DOUBLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT | Same for double |
| C016 | OBJECT_EXCHANGER | Exchange an object between two storage squares |
| C017 | CLICK_REMOVED_REMOVE_SENSOR | Click → remove both object and sensor |
| C018 | END_GAME | Click → trigger end game sequence |
| C127 | PORTRAIT | Champion portrait interaction |

**AND/OR gate (C005)**: Multiple sensors on one wall cell combine via AND/OR logic.
If `sensorCountInCell > 1` for the clicked cell, gates evaluate all remaining sensors.

### Wall Rotation (F0272, F0726)
Wall rotation is a deferred action driven by specific sensor types:
- C011, C012, C013: click triggers rotation of the wall square
- `rotationPending = 1` is set in `F0726_SENSOR_ProcessWallClick_Compat`
- Rotation executes after all sensors on the cell have been processed

### Countdown Timers (C006 — TIMELINE.C lines 1136–1350)
`F0245_SENSOR_ProcessWallEventSensor` handles countdown triggers:
- C006 countdown: decrements a counter each tick; fires when counter reaches zero
- TIMELINE.C maps wall event sensors to periodic tick evaluation

---

## 3. DM2 Sensor Model (vs DM1)

DM2 separates **sensor** (trigger source) from **actuator** (effect executor).
See `docs/dm2_sensors.md` for full coverage. Key distinction:

- DM1 sensor = same record as actuator (SENSOR struct holds effect + target)
- DM2 sensor → actuator invocation via DM2_INVOKE_ACTUATOR()
- DM2 has NO sensor type enum for floor/wall — all triggerable things are actuators

---

## 4. Nexus V1 Sensor Model (vs DM1/DM2)

Nexus does NOT use DM1-style sensor type constants or DM2-style actuator enums.
Instead, sensors are defined in **SDDRVS.TSK** as condition clauses:

```c
// Hypothetical SDDRVS.TSK condition opcodes (reversed):
COND_PARTY_ON_XY      = 0x20  // party at (mapX, mapY, cell)
COND_PARTY_FACING     = 0x21  // party facing direction N
COND_CHAMPION_HAS     = 0x22  // champion holds item type N
COND_CHAMPION_LEVEL   = 0x23  // champion level >= N
COND_LEVEL_LOADED     = 0x24  // current dungeon level == N
COND_SQUARE_TYPE      = 0x25  // tile type at (x,y) == N
COND_TIME_ELAPSED     = 0x26  // tick counter >= N
COND_EVENT_FLAG       = 0x27  // flag N is set
```

**Floor sensor equivalent**: COND_PARTY_ON_XY at each relevant coordinate pair.
**Wall sensor equivalent**: COND_PARTY_FACING + COND_SQUARE_TYPE combo.
**Pressure plate**: COND_PARTY_ON_XY with no action side-effect (pure trigger).

Unlike DM1 (one type per square) and DM2 (one actuator per cell),
Nexus SDDRVS.TSK can attach **multiple condition rules** to the same square —
enabling complex multi-trigger choreography impossible in earlier engines.

---

## 5. Firestaff Nexus Sensor Implementation

Current status in `src/nexus/nexus_v1_dungeon.c`:
- Grid parsing only — `NexusDungeonGrid` struct with tile type array
- No sensor data structure — sensor/script attachment is TODO
- No SDDRVS.TSK parser — condition opcodes unreversed

Missing for full sensor support:
1. SDDRVS.TSK bytecode parser (reverse-engineer opcodes/operands)
2. Sensor attachment struct per dungeon tile
3. Condition evaluator on tick
4. Action dispatcher (teleport, spawn, sound, door control)

The 5,448-byte SDDRVS.TSK file is the primary blocker. Suggested approach:
- Hexdump the file and look for ASCII strings (action names, tile references)
- Look for repeating 8- or 16-byte patterns (rule entries)
- Cross-reference with LEV##.DGN coordinates mentioned in the script
