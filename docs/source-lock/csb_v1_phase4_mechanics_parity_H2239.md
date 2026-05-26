# CSB V1 Phase 4 — Mechanics Parity: Sensors, Actuators, Teleporters, Pits, Doors, End Conditions

**Phase:** 4 of CSB V1 implementation
**Date:** 2026-05-26
**Sources:** ReDMCSB TIMELINE.C, MOVESENS.C, DUNGEON.C, ENDGAME.C, DEFS.H · BugsAndChanges.htm

---

## Overview

Phase 4 source-locks the CSB dungeon mechanics that diverge from DM1:
sensors, actuators, teleporters/pits, doors, end conditions, and the
`F0249_TIMELINE_MoveTeleporterOrPitSquareThings` group-processing fix.

Two new sensor types are CSB-only (C009_VERSION_CHECKER, C018_END_GAME).
All other sensor/actuator types are shared with DM1; the delta is in
trigger behavior and processing order.

---

## Part I — New Sensor Types

### 1. End Game Sensor — `C018_SENSOR_WALL_END_GAME` (CSB CHANGE7_21)

| Property | Value |
|----------|-------|
| Type | Wall sensor type 18 |
| Trigger | `F0248_TIMELINE_ProcessEvent6_Square_Wall` → TIMELINE.C:1319 |
| Effect | Sets `G0302_B_GameWon = TRUE`, calls `F0666_endgame()` |
| Delay | `Value` field = delay in seconds (not ticks) · CHANGE8_02 |
| Comment | DEFS.H:1202: *"Value is a delay in seconds for end game sensor"* |

**Trigger sequence (TIMELINE.C:1319–1340):**

```
if (L0640_ui_SensorType == C018_SENSOR_WALL_END_GAME) {
    F1012_PALETTE_SetCurtain(C0_BLACK_PALETTE);          // MEDIA671
    F0694_SetMultipleColorsInPalette(G2100_i_BlackPaletteIndex);
    F0022_MAIN_Delay(60 * L0638_ps_Sensor->Remote.Value); // CHANGE8_02: delay
    G0524_B_RestartGameAllowed = C0_FALSE;               // MEDIA277
    G0302_B_GameWon = C1_TRUE;
    F0666_endgame();                                      // MEDIA629
    F0444_STARTEND_Endgame(C1_TRUE);                      // MEDIA277
}
```

**DM1:** No end game sensor. DM1 ends via death of all champions or
in-dungeon scripted events only.

**CSB 2.1 (CHANGE8_02):** Optional integer-second delay before ending,
stored in the sensor's `Value` field. Not used in the original CSB dungeon.

**Source:** DEFS.H:1283 · TIMELINE.C:1319–1340 · ENDGAME.C:984–1002
(BUG0_00: G0302_B_GameWon guard in F0666_endgame)

---

### 2. Version Checker Sensor — `C009_SENSOR_FLOOR_VERSION_CHECKER` (CSB CHANGE7_23)

| Property | Value |
|----------|-------|
| Type | Floor sensor type 9 |
| Trigger | `F0276_SENSOR_ProcessThingAdditionOrRemoval` → MOVESENS.C:1716 |
| Condition | Triggered only if `data_value <= game_engine_version` |
| CSB 2.0 version | 20 (hard-coded) |
| CSB 2.1 version | 21 (CHANGE8_06) |
| Use | Dungeon designer version gate for content compatibility |

**Implementation (MOVESENS.C:1716–1750):**

```c
case C009_SENSOR_FLOOR_VERSION_CHECKER:
    if ((L0767_i_ThingType != CM1_THING_TYPE_PARTY) || !P0592_B_AddThing || P0591_B_PartySquare)
        goto T0276079;
#ifdef MEDIA342_S21E_G21E_A21E_A22E   // CHANGE8_06: version 21
    L0768_B_TriggerSensor = (L0779_i_SensorData <= 21);
#endif
#ifdef MEDIA337_S20E_G20E_A20ED_A20E  // CSB 2.0: version 20
    L0768_B_TriggerSensor = (L0779_i_SensorData <= 20);
#endif
    break;
```

**DM1:** No version checker sensor. CSB adds a dungeon data version gate
so designers can require a minimum engine version before triggering content.

**Source:** DEFS.H:1265 · MOVESENS.C:1716–1750 · BugsAndChanges.htm:CHANGE7_23,CHANGE8_06

---

## Part II — Actuator / Wall Sensor Types (Shared with DM1)

All wall sensor types C001–C017 exist in both DM1 and CSB with identical
semantics. The CSB delta is the trigger *context* and the
`F0248_TIMELINE_ProcessEvent6_Square_Wall` function that processes them.

### Summary of Wall Sensor Types (DEFS.H:1234–1282)

| Index | Name | Description |
|-------|------|-------------|
| C001 | `SENSOR_WALL_ORNAMENT_CLICK` | Click wall ornament |
| C002 | `SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT` | Click + any object |
| C003 | `SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT` | Click + specific object |
| C004 | `SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED` | Object removed + click |
| C005 | `SENSOR_WALL_AND_OR_GATE` | Multi-cell AND/OR logic gate |
| C006 | `SENSOR_WALL_COUNTDOWN` | Countdown timer (SET=inc, CLEAR=dec) |
| C007 | `SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_NEW_OBJECT` | Launch projectile |
| C008 | `SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_EXPLOSION` | Launch + explosion |
| C009 | `SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_NEW_OBJECT` | Double projectile |
| C010 | `SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_EXPLOSION` | Double + explosion |
| C011 | `SENSOR_WALL_ORNAMENT..._REMOVED_ROTATE_SENSORS` | Rotate sensors on removal |
| C014 | `SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT` | Square object launcher |
| C015 | `SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT` | Double square launcher |
| C016 | `SENSOR_WALL_OBJECT_EXCHANGER` | Swap two objects |
| C017 | `SENSOR_WALL_..._REMOVE_SENSOR` | Remove sensor on object removal |
| **C018** | **`SENSOR_WALL_END_GAME`** | **CSB NEW — end game** |
| C127 | `SENSOR_WALL_CHAMPION_PORTRAIT` | Click champion portrait |

**Source:** DEFS.H:1234–1283 · TIMELINE.C:1198–1345 (F0248 handler)

### Floor Sensor Types (DEFS.H:1202–1233)

| Index | Name | Description |
|-------|------|-------------|
| C003 | `SENSOR_FLOOR_PARTY` | Party enters square |
| C004 | `SENSOR_FLOOR_OBJECT` | Object placed on square |
| C005 | `SENSOR_FLOOR_PARTY_ON_STAIRS` | Party on stairs |
| C006 | `SENSOR_FLOOR_GROUP_GENERATOR` | Creature group spawned |
| C007 | `SENSOR_FLOOR_CREATURE` | Creature enters square |
| C008 | `SENSOR_FLOOR_PARTY_POSSESSION` | Party has specific item |
| **C009** | **`SENSOR_FLOOR_VERSION_CHECKER`** | **CSB NEW — version gate** |

**Source:** DEFS.H:1202–1265 · MOVESENS.C (F0276_SENSOR_ProcessThingAdditionOrRemoval)

---

## Part III — Teleporters and Pits

### F0249_TIMELINE_MoveTeleporterOrPitSquareThings (CHANGE7_22_FIX)

Both DM1 and CSB use the same teleporter/pit square mechanics:
when a teleporter or pit opens, all things on that square are moved to
the same target location. The **CSB fix** changes *processing order*.

**Bug (DM1):** If a group is on the square followed by a projectile that
impacts the group, processing the projectile first removes the group from
the thing list, breaking the linked-list traversal and causing an
infinite loop or missed processing.

**DM1 behavior:** Things processed in arbitrary linked-list order.

**CSB CHANGE7_22_FIX behavior (TIMELINE.C:1398–1420):**

1. **Group processed first** — `F0175_GROUP_GetThing()` retrieves the
   group thing; `F0267_MOVE_GetMoveResult_CPSCE()` moves the group.
2. **Then remaining things counted** — Count non-group things on square.
3. **Then each non-group processed exactly once** — Loop with
   `L0649_i_ThingsToMoveCount` guard prevents infinite loops.

```c
// CHANGE7_22_FIX: Process group FIRST
if ((L0645_T_Thing = F0175_GROUP_GetThing(P0525_ui_MapX, P0526_ui_MapY)) != C0xFFFE_THING_ENDOFLIST) {
    F0267_MOVE_GetMoveResult_CPSCE(L0645_T_Thing, P0525_ui_MapX, P0525_ui_MapY, P0525_ui_MapX, P0525_ui_MapY);
}
// Count remaining things
L0645_T_Thing = F0162_DUNGEON_GetSquareFirstObject(P0525_ui_MapX, P0525_ui_MapY);
L0649_i_ThingsToMoveCount = 0;
while (L0645_T_Thing != C0xFFFE_THING_ENDOFLIST) {
    if (M012_TYPE(L0645_T_Thing) > C04_THING_TYPE_GROUP) {
        L0649_i_ThingsToMoveCount++;
    }
    L0645_T_Thing = F0159_DUNGEON_GetNextThing(L0645_T_Thing);
}
// Process each thing exactly once
while ((L0645_T_Thing != C0xFFFE_THING_ENDOFLIST) && L0649_i_ThingsToMoveCount) {
    L0649_i_ThingsToMoveCount--;
    L0648_T_NextThing = F0159_DUNGEON_GetNextThing(L0645_T_Thing);
    F0267_MOVE_GetMoveResult_CPSCE(L0645_T_Thing, ...);
    L0645_T_Thing = L0648_T_NextThing;
}
```

**DM1 BUG0_22:** Infinite loop or missed things when ≥2 things on closed
teleporter/pit, or group + projectile on pit square.

**Source:** TIMELINE.C:1353–1476 (F0249) · BugsAndChanges.htm:CHANGE7_22

---

## Part IV — Door Types

Both DM1 and CSB use the same door type IDs (wooden=0, iron=1, Ra=2).
The CSB delta is the **defense point values** (HP) used for melee attacks.

### Door Defense Points (DUNGEON.C:561–565)

| Type | Name | Defense (HP) | Notes |
|------|------|-------------|-------|
| 0 | Wooden door | **42** | Destroyed by melee (attack < 100) |
| 1 | Iron door | **230** | Indestructible by melee (attack limit = 100) |
| 2 | Ra door | **255** | Indestructible; `MASK0x0004_ANIMATED`, `MASK0x0001_CREATURES_CAN_SEE_THROUGH` |

**CSB DUNGEON.C:563–565:**
```c
{ 0,               42  },   /* Door type 1 Wooden door */
{ 0,              230  },   /* Door type 2 Iron door */
{ MASK0x0004_ANIMATED | MASK0x0001_CREATURES_CAN_SEE_THROUGH, 255 } }; /* Door type 3 Ra door */
```

**DM1:** Identical door types and defense values — no change.

**Source:** DUNGEON.C:561–565 (pass563/compat) · DEFS.H door type comments

---

## Part V — End Conditions

### G0302_B_GameWon — Game Won Flag

| Property | Value |
|----------|-------|
| Global | `G0302_B_GameWon` |
| Type | `BOOLEAN` |
| Set by | `C018_SENSOR_WALL_END_GAME` trigger (TIMELINE.C:1338) |
| Checked by | `F0666_endgame()` (ENDGAME.C:994), `F0444_STARTEND_Endgame()` (ENDGAME.C:215,254) |
| Effect | Prevents restart; triggers credits display |

### F0666_endgame() — Endgame Entry Point (ENDGAME.C:984)

```c
void F0666_endgame(void) {
    if (!G0302_B_GameWon) {       // BUG0_00 guard
        return;
    }
    if (G0302_B_GameWon) {        // confirmed won
        F0444_STARTEND_Endgame(C1_TRUE);  // draw credits
    }
}
```

### F0444_STARTEND_Endgame() — Endgame UI Flow (ENDGAME.C:101–327)

1. Draw champion mirror/portrait zones (DEFS.H:412–419: `C412_ZONE_ENDGAME_CHAMPION_MIRROR_0`…)
2. If `G0302_B_GameWon == TRUE`: show victory text + credits
3. Display restart zone (`C437_ZONE_ENDGAME_RESTART`) and quit zone (`C438_ZONE_ENDGAME_QUIT`)

**Endgame UI zones (DEFS.H:3827–3848):**

| Zone ID | Name | Description |
|---------|------|-------------|
| 412–415 | `ENDGAME_CHAMPION_MIRROR_0..3` | Champion mirror display |
| 416–419 | `ENDGAME_CHAMPION_PORTRAIT_0..3` | Champion portrait |
| 437 | `ENDGAME_RESTART` | Restart button |
| 438 | `ENDGAME_QUIT` | Quit button |

**DM1:** No `G0302_B_GameWon` flag or endgame sensor. Endgame was
implicit via party death or dungeon-specific triggers.

**Source:** ENDGAME.C:984–1002 (F0666), 101–327 (F0444) · DEFS.H:3827–3848 · TIMELINE.C:1338

---

## Part VI — Bug Fixes Affecting Dungeon Logic

### CHANGE7_17_FIX (BUG0_09) — Sensor Squares Ignored During Thing Discard

**DM1 bug:** When discarding a thing to make room (e.g., Screamer Slice after
killing a Screamer), the discard search could land on a square with an
enabled sensor, inadvertently triggering it (e.g., closing a door under a
pressure plate).

**Fix:** `DUNGEON.C:1998–2001`: Squares with enabled sensors are now
explicitly skipped when searching for a thing to discard.

```c
/* CHANGE7_17_FIX: Squares with enabled sensors are ignored
   when searching for a thing to discard */
if (M039_TYPE((SENSOR*)L0282_ps_Generic))  /* If sensor is not disabled */
    continue;  /* skip this square */
```

**Source:** DUNGEON.C:1996–2001 · BugsAndChanges.htm:CHANGE7_17

---

### CHANGE7_18_FIX (BUG0_10) — Clear Bit 15 Before Thing Type Check

**DM1 bug:** Thing type extraction used a raw value that could have bit 15
set, giving an incorrect type.

**Fix:** `DUNGEON.C`: Clear bit 15 to get the actual thing type before
using the value in type comparisons and switch statements.

**Source:** DUNGEON.C (BUG0_10) · BugsAndChanges.htm:CHANGE7_18

---

### CHANGE7_19_FIX (BUG0_69) — Lord Chaos Teleporter Direction

**DM1 bug:** When Lord Chaos senses danger (Poison Cloud, closing door, or
3+ adjacent Fluxcages), his teleport direction was initialized with a
random value, potentially teleporting into walls or other invalid locations.

**Fix:** `GROUP.C (CHANGE7_19)`: Lord Chaos teleporter direction is now
properly initialized, and allowed-map checks are fixed.

**Source:** GROUP.C (CHANGE7_19) · BugsAndChanges.htm:CHANGE7_19,BUG0_69

---

## Summary — DM1 vs CSB Delta for Phase 4

| Element | DM1 | CSB Delta |
|---------|-----|-----------|
| Sensor type 9 (VERSION_CHECKER) | None | **NEW** — engine version gate |
| Sensor type 18 (END_GAME) | None | **NEW** — game won trigger |
| End game sensor delay | N/A | **NEW** (CHANGE8_02) — `Value` field = seconds |
| Group on teleporter/pit | Arbitrary order; infinite loop bug | **FIXED** (CHANGE7_22) — group first, count guard |
| Lord Chaos teleport direction | Random init (bug) | **FIXED** (CHANGE7_19) |
| Thing discard on sensor square | Triggers sensor (bug) | **FIXED** (CHANGE7_17) |
| Thing type bit 15 | Raw value (bug) | **FIXED** (CHANGE7_18) |
| Door types | Wooden/Iron/Ra | Unchanged (42/230/255 HP) |
| Endgame flow | Implicit via death | **NEW** — `G0302_B_GameWon`, `F0666_endgame()`, zone UI |
| All other sensor/actuator types | C001–C017 | Identical to DM1 |

---

## Implementation Status

| Element | Status | File(s) |
|---------|--------|---------|
| `C018_SENSOR_WALL_END_GAME` trigger | Skeleton | `csb_v1_dungeon_world_pc34_compat.c` stub only |
| `C009_SENSOR_FLOOR_VERSION_CHECKER` | Skeleton | `csb_v1_dungeon_world_pc34_compat.c` stub only |
| `F0248_TIMELINE_ProcessEvent6_Square_Wall` actuator pipeline | Not implemented | Source-locked only |
| `F0249_TIMELINE_MoveTeleporterOrPitSquareThings` (CHANGE7_22) | Not implemented | Source-locked only |
| `F0666_endgame()` / `G0302_B_GameWon` | Not implemented | Source-locked only |
| Door types (wooden/iron/Ra) | Skeleton | `csb_v1_dungeon_world_pc34_compat.c` tile types defined |
| Bug fixes (CHANGE7_17/18/19/22) | Not implemented | Source-locked only |

---

## Source Citations

| File | Lines | Content |
|------|-------|---------|
| ReDMCSB DEFS.H | 1202,1265,1283 | Sensor type constants; Value = seconds for end game |
| ReDMCSB TIMELINE.C | 1136–1350 | F0248 (actuator processing), F0249 (teleporter/pit group fix) |
| ReDMCSB TIMELINE.C | 1319–1340 | C018_END_GAME trigger sequence + CHANGE8_02 delay |
| ReDMCSB MOVESENS.C | 1716–1750 | C009_VERSION_CHECKER implementation + CHANGE8_06 |
| ReDMCSB DUNGEON.C | 561–565 | Door defense point values |
| ReDMCSB DUNGEON.C | 1996–2001 | CHANGE7_17_FIX: sensor squares in discard |
| ReDMCSB ENDGAME.C | 101–327 | F0444_STARTEND_Endgame() |
| ReDMCSB ENDGAME.C | 984–1002 | F0666_endgame() + G0302_B_GameWon guard |
| ReDMCSB DEFS.H | 3827–3848 | Endgame UI zone constants (412–438) |
| ReDMCSB GROUP.C | CHANGE7_19 | Lord Chaos teleporter direction fix |
| BugsAndChanges.htm | CHANGE7_17,18,19,21,22,23,CHANGE8_02,06 | Change descriptions |
