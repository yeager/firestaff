# DM1 V1 — Conditional Logic in Dungeon Events
**Source: ReDMCSB MOVESENS.C + TIMELINE.C + CEDT files**
**Audit: 2026-05-25 | Source-lock: ReDMCSB_WIP20210206**

---

## 1. Overview

DM1 V1 has **no scripting language**. Conditional logic is baked into:
- **Sensor type dispatch** (per-type if/else chains in F0275, F0276)
- **Event type dispatch** (switch in `F0248_TIMELINE_ProcessEvent6_Square_Wall`, lines 1136–1350)
- **HOLD effect bistable flip-flop** (SET when active, CLEAR when inactive)
- **AND/OR gate sensors** (wall type C005) — the only multi-input conditional construct

---

## 2. Sensor-Level Conditionals

### 2.1 Floor Sensor Conditionals (F0276 — MOVESENS.C ~1553)

Each floor sensor type has an **independent if/else skip chain**. The switch at F0276:~1600:

```c
switch (sensorType) {
case C001_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT:
    // Skip if: partySquare OR hasObject OR hasGroup
    if (partySquare || hasObject || hasGroup) goto skip;
    trigger = TRUE;
    break;

case C002_SENSOR_FLOOR_THERON_PARTY_CREATURE:
    // Skip if: thingType > GROUP OR partySquare OR hasGroup
    if (thingType > C04_THING_TYPE_GROUP || partySquare || hasGroup) goto skip;
    trigger = TRUE;
    break;

case C003_SENSOR_FLOOR_PARTY:
    // Skip if: thingType != PARTY OR championCount == 0
    if (thingType != CM1_THING_TYPE_PARTY || championCount == 0) goto skip;
    if (sensorData == 0) {
        if (partySquare) goto skip;
    } else {
        if (!isAdd) trigger = FALSE;
        else trigger = (sensorData == ORDINAL(partyDirection));
    }
    break;

case C004_SENSOR_FLOOR_OBJECT:
    // Skip if: objType mismatch OR squareHasSameTypeObj
    if (sensorData != objType || squareHasSameTypeObj) goto skip;
    trigger = TRUE;
    break;

case C005_SENSOR_FLOOR_PARTY_ON_STAIRS:
    // Skip if: thingType != PARTY OR squareType != STAIRS
    if (thingType != CM1_THING_TYPE_PARTY || squareType != C03_ELEMENT_STAIRS) goto skip;
    trigger = TRUE;
    break;

case C006_SENSOR_FLOOR_GROUP_GENERATOR:
    // Always skip — event-triggered only
    goto skip;

case C007_SENSOR_FLOOR_CREATURE:
    // Skip if: thingType > GROUP OR thingType == PARTY OR hasGroup
    if (thingType > C04_THING_TYPE_GROUP || thingType == CM1_THING_TYPE_PARTY || hasGroup) goto skip;
    trigger = TRUE;
    break;

case C008_SENSOR_FLOOR_PARTY_POSSESSION:
    // Skip if: thingType != PARTY
    if (thingType != CM1_THING_TYPE_PARTY) goto skip;
    trigger = F0274_SENSOR_IsObjectInPartyPossession(sensorData);
    break;

case C009_SENSOR_FLOOR_VERSION_CHECKER:
    // Skip if: thingType != PARTY OR !isAdd OR partySquare
    if (thingType != CM1_THING_TYPE_PARTY || !isAdd || partySquare) goto skip;
    trigger = (sensorData <= 20); // DM1 version 2.0 check
    break;
}
```

**RevertEffect XOR**: After the per-type decision, result is XORed with `Remote.RevertEffect`:
```c
triggerSensor ^= Remote.RevertEffect;
```

**HOLD resolution** (F0276:~1670):
```c
if (effect == C03_EFFECT_HOLD) {
    effect = triggerSensor ? C00_EFFECT_SET : C01_EFFECT_CLEAR;
} else {
    if (!triggerSensor) goto skip;
}
```

### 2.2 Wall Sensor Conditionals (F0275 — MOVESENS.C ~1309)

```c
switch (sensorType) {
case C001_SENSOR_WALL_ORNAMENT_CLICK:
    doNotTrigger = FALSE;
    if (effect == C03_EFFECT_HOLD) goto skip;
    break;

case C002_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT:
    doNotTrigger = (leaderEmptyHanded != revertEffect);
    break;

case C003_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT:
case C004_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED:
    doNotTrigger = ((sensorData == F0032_OBJECT_GetType(leaderHandObj)) == revertEffect);
    break;

case C011_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE:
case C017_SENSOR_WALL_CLICK_OBJ_REMOVED_REMOVE_SENSOR:
    if (sensorCountInCell > 0) goto skip;  // not last sensor on cell

case C012_SENSOR_WALL_OBJECT_GENERATOR_ROTATE:
    if (sensorCountInCell > 0) goto skip;
    doNotTrigger = !leaderEmptyHanded;
    break;

case C013_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE:
    // Complex: store/retrieve, leader hand state changes
    ...

case C016_SENSOR_WALL_OBJECT_EXCHANGER:
    // Swap leader object with square object
    ...

case C127_SENSOR_WALL_CHAMPION_PORTRAIT:
    // Always triggers (even with no leader)
    break;
}
// Default for C005..C010, C014..C015, C018: goto skip (event-only)
```

---

## 3. AND/OR Gate Logic (C005 — Wall Sensor)

**Source: TIMELINE.C F0248 lines 1268–1309**

The AND/OR gate uses a **bitmask stored in `sensorData`**:

```
sensorData (16-bit):
  High nibble (bits 12-15): reference mask (the "expected" pattern)
  Low nibble (bits 0-3):    current input mask (accumulated state)

Bit 0 = Cell 0 event received
Bit 1 = Cell 1 event received
Bit 2 = Cell 2 event received
Bit 3 = Cell 3 event received
```

**Processing per wall event (F0248 lines 1268–1309)**:
1. Extract bit for the triggering cell
2. Apply effect:
   - **TOGGLE**: flip the bit in current mask
   - **SET**: set bit to 1
   - **CLEAR**: set bit to 0
3. After update, compare `currentMask == referenceMask`
4. If equal AND `Remote.RevertEffect == 0` → fire effect on target square
5. If equal AND `Remote.RevertEffect == 1` → fire **opposite** effect on target square
6. If `effect == HOLD`: always dispatch SET/CLEAR (the comparison result)

**Firing condition**:
```c
// F0248:~1306
if (currentMask == referenceMask) {
    int fireEffect = RevertEffect ? OPPOSITE(resolvedEffect) : resolvedEffect;
    F0268_SENSOR_AddEvent(fireEffect, targetX, targetY, targetCell, fireEffect, gameTime + delay);
}
```

---

## 4. Countdown Sensor Logic (C006 — Wall Countdown)

**Source: TIMELINE.C F0248 lines 1198–1266**

```
sensorData (16-bit counter):
  0..510: remaining activations
  511:   exhausted (sensor disabled)
```

**Per-event processing**:
- **SET effect**: increment counter (max 511)
- **TOGGLE/CLEAR effect**: decrement counter

When counter reaches 0:
1. Sensor disabled (type set to `C000_DISABLED`)
2. Effect dispatched to remote target
3. If `OnceOnly`: already disabled after first trigger

**Special case**: `sensorData = 511` with SET → counter stays at 511 (max cap).

---

## 5. Event Type Dispatch (TIMELINE.C F0248 ~1136–1350)

The main wall event processor:

```c
F0248_TIMELINE_ProcessEvent6_Square_Wall(EVENT* e):
  loc = e->Location
  cell = e->C.A.Cell
  effect = e->C.A.Effect

  // Iterate sensors on the event square
  for each SENSOR on loc:
    if (M039_TYPE(sensor) == DISABLED) continue
    switch (sensor->Remote.Type_Data):
      case C005_WALL_AND_OR_GATE:
        // Gate bitmask update + fire check
        ...
      case C006_WALL_COUNTDOWN:
        // Counter decrement/increment + disable check
        ...
      case C007..C010, C014..C015: // Launcher sensors
        F0247_TIMELINE_TriggerProjectileLauncher(sensor, e)
        ...
      case C018_WALL_END_GAME:
        // End game sequence trigger
        ...
```

---

## 6. HOLD as Bistable Conditional

The HOLD effect acts as a **flip-flop** driven by the trigger state:

| Trigger Active | HOLD resolves to |
|---------------|-----------------|
| YES (thing present) | EFFECT_SET |
| NO (thing removed) | EFFECT_CLEAR |

**Application**: A door pressure plate using HOLD will:
1. SET the door state to OPEN when party steps on it
2. CLEAR (close) the door when party steps off

This is the primary mechanism for **party presence–driven doors**.

---

## 7. Bug: Bistable Door Wiring (BUG0_78)

TIMELINE.C:766 has a missing-parenthesis bug in door animation damage:

```c
// WRONG (BUG0_78):
F0324_CHAMPION_DamageAll_GetDamagedChampionCount(
    5,
    MASK0x0008_WOUND_TORSO | AL0602_ui_VerticalDoor ? MASK0x0004_WOUND_HEAD : MASK0x0001_WOUND_READY_HAND | MASK0x0002_WOUND_ACTION_HAND,
    C2_ATTACK_SELF)

// CORRECT (intended):
F0324_CHAMPION_DamageAll_GetDamagedChampionCount(
    5,
    MASK0x0008_WOUND_TORSO | (AL0602_ui_VerticalDoor ? MASK0x0004_WOUND_HEAD : MASK0x0001_WOUND_READY_HAND | MASK0x0002_WOUND_ACTION_HAND),
    C2_ATTACK_SELF)
```

The bitwise-OR bind tighter than `?:` causes all doors to wound HEAD in addition to TORSO, rather than vertical=head+torso and horizontal=hands+torso.

---

## 8. Source Citations

| Function | File | Line | Role |
|----------|------|------|------|
| `F0276_SENSOR_ProcessThingAdditionOrRemoval` | MOVESENS.C | ~1553 | Floor sensor conditional dispatch |
| `F0275_SENSOR_IsTriggeredByClickOnWall` | MOVESENS.C | ~1309 | Wall sensor conditional dispatch |
| `F0248_TIMELINE_ProcessEvent6_Square_Wall` | TIMELINE.C | 1136–1350 | Wall event + AND/OR gate + countdown |
| `F0247_TIMELINE_TriggerProjectileLauncher` | TIMELINE.C | ~1033 | Launcher conditional + object selection |
| `F0274_SENSOR_IsObjectInPartyPossession` | MOVESENS.C | ~1234 | Possession check (helper) |