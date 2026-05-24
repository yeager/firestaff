# DM1 V1 — Creature Group Spawning (C006)
**Source: ReDMCSB TIMELINE.C + GROUP.C + MOVESENS.C + DEFS.H**
**Audit: 2026-05-25 | Source-lock: ReDMCSB_WIP20210206**

---

## 1. Overview

Creature groups spawn via **C006 floor sensors** placed on corridor squares. C006 is **event-triggered only** — the F0276 floor sensor dispatch **skips C006** (MOVESENS.C:~1377: `goto T0276079`).

The spawning pipeline:
1. A C05_EVENT_CORRIDOR fires on a corridor square
2. **F0245** iterates sensors on the square; finds C006 sensors
3. **F0185_GROUP_GetGenerated** allocates and places the creature group
4. If `OnceOnly=0` and `Ticks!=0`: queues a **C65** event to re-enable the generator later

---

## 2. C006 Sensor — Data Layout

```
C006_SENSOR_FLOOR_GROUP_GENERATOR sensor fields (from F0245 + DEFS.H macros):
  Remote.Type_Data  = C006_SENSOR_FLOOR_GROUP_GENERATOR
  Remote.TargetMapX/Y = spawn location (usually same square)
  Remote.Value     = creature count | MASK0x0008_RANDOMIZE (bit 3)
                     (Value 3 = 2 creatures, Value 1 = 1 creature, etc.)
  Remote.OnceOnly  = one-shot (1) vs repeatable (0)
  Remote.Audible   = play BUZZ sound on spawn (1)
  Local.LocalEffect = creature type  (via M040_DATA)
  M045_HEALTH_MULTIPLIER(sensor) = health multiplier (0 = use map difficulty)
  M046_TICKS(sensor)             = respawn delay in ticks (0 = no respawn)
```

**Value encoding**: `Remote.Value` holds the creature count in bits 0–2, and bit 3 (`MASK0x0008_RANDOMIZE`) indicates whether the count should be randomized on each spawn.

---

## 3. Spawn Trigger Flow

### 3.1 F0245_ProcessEvent5_Square_Corridor (TIMELINE.C lines 955–1010)

**Source: TIMELINE.C:955–1010**

```c
F0245_ProcessEvent5_Square_Corridor(EVENT* e):
  mapX = e->Location.MapX
  mapY = e->Location.MapY

  for each THING on square (F0161_GetFirst / F0159_GetNext):
    if THING_TYPE == SENSOR:
      sensor = F0156_GetThingData(sensorThing)
      if sensor.Type != C006_SENSOR_FLOOR_GROUP_GENERATOR: continue

      // Extract spawn parameters
      creatureCount = sensor->Remote.Value
      if (creatureCount & MASK0x0008_RANDOMIZE):
        creatureCount = RANDOM(creatureCount & 0x0007)  // Randomize count
      else:
        creatureCount--  // Pre-decrement (Value field is N+1, not N)

      healthMult = M045_HEALTH_MULTIPLIER(sensor)
      if (healthMult == 0):
        healthMult = currentMap->difficulty  // Fall back to map difficulty

      // Create the group
      F0185_GROUP_GetGenerated(
        M040_DATA(sensor),   // creature type
        healthMult,
        creatureCount,
        RANDOM(4),           // random initial direction
        mapX, mapY
      )

      // Play BUZZ if audible flag set
      if sensor->Remote.Audible:
        F0064_SOUND_RequestPlay_CPSD(M560_SOUND_BUZZ, mapX, mapY, ...)

      // Once-only: disable immediately after spawn
      if sensor->Remote.OnceOnly:
        SET_TYPE_DISABLED(sensor)
      else:
        // Repeatable: queue C65 event to re-enable this generator
        ticks = M046_TICKS(sensor)
        if ticks != 0:
          if (ticks > 127) ticks = (ticks - 126) << 6  // Expand large delays
          ADD_EVENT(C65_EVENT_ENABLE_GROUP_GENERATOR, gameTime + ticks)
```

### 3.2 F0185_GROUP_GetGenerated (GROUP.C lines 481–530)

**Source: GROUP.C:481–530**

```c
THING F0185_GROUP_GetGenerated(type, healthMult, count, direction, mapX, mapY):
  // Check spawn limits
  if (activeGroupCount >= maxActiveGroups - 5) && (mapIndex == partyMapIndex):
    return NONE
  if (no unused GROUP thing available):
    return NONE

  // Allocate GROUP thing
  groupThing = F0166_DUNGEON_GetUnusedThing(C04_THING_TYPE_GROUP)
  group = GetThingData(groupThing)

  // Initialize
  group->Type = type
  group->Count = count
  group->Direction = direction

  // Health per creature:
  // health[i] = (baseHealth * healthMult) + RANDOM(baseHealth/4 + 1)
  baseHealth = creatureInfo[type].baseHealth
  for each creature i in count:
    group->Health[i] = (baseHealth * healthMult) + RANDOM(baseHealth/4 + 1)

  // Place group on square
  if F0267_MOVE_GetMoveResult_CPSCE(groupThing, NOT_ON_SQUARE, 0, mapX, mapY):
    // Blocked by party → group dies immediately, return NONE
    return NONE

  F0064_SOUND_RequestPlay_CPSD(M560_SOUND_BUZZ, mapX, mapY, ...)
  return groupThing
```

---

## 4. F0246 — C65 Enable Group Generator Event

**Source: TIMELINE.C:1010–1030**

C65 events re-enable a **disabled** sensor as C006 (for repeatable generators):

```c
F0246_ProcessEvent65_EnableGroupGenerator(EVENT* e):
  for each THING on square:
    if THING_TYPE == SENSOR:
      sensor = GetThingData(sensorThing)
      if sensor.Type == C000_SENSOR_DISABLED:
        sensor.Remote.Type_Data |= C006_SENSOR_FLOOR_GROUP_GENERATOR
        return  // Only enable the FIRST disabled sensor
```

This is the only way a C000_DISABLED sensor becomes an active C006 generator.

---

## 5. Once-Only vs Repeatable Generators

| Mode | OnceOnly | Ticks | Behavior after spawn |
|------|---------|-------|-------------------|
| One-shot | 1 | 0 | Sensor type → C000_DISABLED; never fires again |
| Repeatable | 0 | >0 | C65 event queued; after delay, F0246 re-enables as C006 |
| Repeatable | 0 | 0 | Fires every time the corridor event fires (infinite waves) |

**BUG0_18**: If the event pool is exhausted when adding the C65 respawn event, the generator becomes effectively one-shot.

---

## 6. Group Placement — F0267 Interaction

**Source: GROUP.C:530 / MOVESENS.C:830–840**

```c
if F0267_MOVE_GetMoveResult_CPSCE(groupThing, NOT_ON_SQUARE, 0, mapX, mapY):
  // Returns C1_TRUE if:
  //   - Party is on the destination square (blocked)
  //   - Another group is on the destination square
  return NONE  // Group dies — no retry event is created!
```

Unlike C60/C61 (group movement retry), C006 **does not** create a retry event when blocked by the party. The spawn silently fails.

---

## 7. Active Groups — Combat Tracking

**Source: GROUP.C global variables**

```c
ACTIVE_GROUP* G0375_ps_ActiveGroups;   // Combat participant array
uint16_t G0376_ui_MaximumActiveGroupCount;
uint16_t G0377_ui_CurrentActiveGroupCount = 0;
```

**F0183_GROUP_AddActiveGroup** (GROUP.C ~350):
- Called for C6_BEHAVIOR_ATTACK groups after spawn
- Tracks distance-to-party, primary/secondary directions
- Used for melee target selection in combat

---

## 8. Creature Health Scaling

The generator computes individual creature health:

```
health[i] = (creatureInfo[type].baseHealth * healthMult) + RANDOM(baseHealth/4 + 1)
```

**Examples** (with map difficulty = 1):
- Lord of the Flame (baseHealth=35): 35*1 + RANDOM(9) = 35–43
- Fire Elemental (baseHealth=20): 20*1 + RANDOM(6) = 20–25

With healthMult > 1: harder enemies on harder difficulty levels.

---

## 9. Source Citations

| Function | File | Line | Role |
|----------|------|------|------|
| `F0245_TIMELINE_ProcessEvent5_Square_Corridor` | TIMELINE.C | 955 | Corridor processing + C006 dispatch |
| `F0185_GROUP_GetGenerated` | GROUP.C | 481 | Group allocation + health + placement |
| `F0246_TIMELINE_ProcessEvent65_EnableGroupGenerator` | TIMELINE.C | 1010 | C65: enable disabled→C006 |
| `F0267_MOVE_GetMoveResult_CPSCE` | MOVESENS.C | ~830 | Group placement + party-blocking check |
| `F0183_GROUP_AddActiveGroup` | GROUP.C | ~350 | Add to active combat list |
| `F0184_GROUP_RemoveActiveGroup` | GROUP.C | — | Remove from active list |
| `F0209_GROUP_ProcessEvents29to41` | TIMELINE.C | ~1852 | Group AI event dispatch |
| `F0276_SENSOR_ProcessThingAdditionOrRemoval` | MOVESENS.C | ~1553 | Floor sensor (C006→skip) |
| `M040_DATA`, `M045_HEALTH_MULTIPLIER`, `M046_TICKS` | DEFS.H | — | Sensor data accessors |
| DEFS.H C006_SENSOR_FLOOR_GROUP_GENERATOR | DEFS.H | ~1265 | Sensor type constant |