# DM1 V1 — Event Trigger System (Sensors as Events)
**Source: ReDMCSB MOVESENS.C + TIMELINE.C**
**Audit: 2026-05-25 | Source-lock: ReDMCSB_WIP20210206**

---

## 1. Sensor Thing Architecture

Sensors are `THING_TYPE_SENSOR` objects placed on dungeon squares (per-cell).
Each square has a linked list of sensors; traversal via `F0161_DUNGEON_GetSquareFirstThing` / `F0159_DUNGEON_GetNextThing`.

Sensor data layout (from DEFS.H + MOVESENS.C):

```
SENSOR:
  Remote.Type_Data    — sensor type (C000..C127)
  Remote.TargetMapX  — remote target X
  Remote.TargetMapY  — remote target Y
  Remote.TargetCell   — remote target cell
  Remote.Effect       — EFFECT_SET / CLEAR / TOGGLE / HOLD
  Remote.OnceOnly     — disable after trigger
  Remote.RevertEffect — xor mask on trigger decision
  Remote.Value        — delay (ticks)
  Remote.LocalEffect  — 0=remote, 1=local
  Remote.Next         — linked-list next pointer
  Local.LocalEffect   — local effect value
  Local.Multiple      — low byte: kinetic energy; high byte: step energy (launchers)
```

---

## 2. Two Sensor Contexts

### Floor Sensors (F0276 — `SENSOR_ProcessThingAdditionOrRemoval`)
Triggered when a thing **enters or leaves** a square.
- **Caller**: `F0267_MOVE_GetMoveResult_CPSCE` lines 801–896 — every party/group/projectile move.
- **Key**: `F0276_SENSOR_ProcessThingAdditionOrRemoval(mapX, mapY, thing, partySquare, isAdd)`
- Things that **levitate** (see F0264 `MOVE_IsLevitating`) skip sensor processing on departure.

### Wall Sensors (F0275 — `SENSOR_IsTriggeredByClickOnWall`)
Triggered when the leader **clicks on a wall ornament** (viewport click routing).
- **Caller**: `CLIKVIEW.C` viewport click handler.
- **Key**: `F0275_SENSOR_IsTriggeredByClickOnWall(mapX, mapY, cell)`
- Types C005–C010, C014–C015, C018 are **event-only**: F0275 skips them via `default:` goto.

---

## 3. Event Dispatch — Timeline

All sensor-triggered actions go through the **timeline event queue**:

1. `F0268_SENSOR_AddEvent(type, mapX, mapY, cell, effect, time)`  
   Creates an `EVENT` and inserts it via `F0238_TIMELINE_AddEvent_GetEventIndex_CPSE`.
2. Priority assignment (lines 1017–1031):
   - CLEAR → priority 3
   - TOGGLE → priority 2
   - SET → priority 1
3. `F0238_TIMELINE_AddEvent_GetEventIndex_CPSE` inserts into a binary-heap timeline (`G0371_pui_Timeline`).
4. `F0248_TIMELINE_ProcessEvent6_Square_Wall` (lines 1136–1350) processes wall square events: **SET/CLEAR/TOGGLE** on remote target squares.

---

## 4. Floor Sensor Types (C000..C009, DEFS.H lines 1256–1265)

| Type | Name | Trigger Condition | Floor-Triggered? |
|------|------|-------------------|-----------------|
| C000 | DISABLED | Never | — |
| C001 | FLOOR_THERON_PARTY_CREATURE_OBJECT | Party/creature/object entry (not on same square) | ✅ |
| C002 | FLOOR_THERON_PARTY_CREATURE | Party/creature only (no object) | ✅ |
| C003 | FLOOR_PARTY | Party entry; optional direction check | ✅ |
| C004 | FLOOR_OBJECT | Object of specific type placed | ✅ |
| C005 | FLOOR_PARTY_ON_STAIRS | Party on stairs square | ✅ |
| C006 | FLOOR_GROUP_GENERATOR | **EVENT-TRIGGERED ONLY** (F0276 skips) | ❌ |
| C007 | FLOOR_CREATURE | Any creature (not party/object) | ✅ |
| C008 | FLOOR_PARTY_POSSESSION | Party carries specific object | ✅ |
| C009 | FLOOR_VERSION_CHECKER | `data <= 20` (version check) | ✅ |

**Key skip rule (F0276 lines 1377–1389)**:
```c
// Bug0_09: discarding a thing to make room may trigger an unwanted sensor
if (M012_TYPE(L0278_T_Thing) == P0295_ui_ThingType) {
    // discarding to make space — may trigger sensor on source square
}
```

---

## 5. Wall Sensor Types (C001..C018 + C127, DEFS.H lines 1266–1284)

| Type | Name | Trigger | Click-Triggered? |
|------|------|---------|-----------------|
| C001 | WALL_ORNAMENT_CLICK | Always (HOLD→skip) | ✅ |
| C002 | WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT | Leader has/hasn't object | ✅ |
| C003 | WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT | Specific object in hand | ✅ |
| C004 | WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED | Specific object, then removed | ✅ |
| C005 | WALL_AND_OR_GATE | **EVENT-TRIGGERED** (wall event) | ❌ |
| C006 | WALL_COUNTDOWN | **EVENT-TRIGGERED** (wall event) | ❌ |
| C007 | WALL_LAUNCH_SINGLE_EXPLOSION | **EVENT-TRIGGERED** | ❌ |
| C008 | WALL_LAUNCH_SINGLE_NEW_OBJECT | **EVENT-TRIGGERED** | ❌ |
| C009 | WALL_LAUNCH_SINGLE_SQUARE_OBJECT | **EVENT-TRIGGERED** | ❌ |
| C010 | WALL_LAUNCH_DOUBLE_EXPLOSION | **EVENT-TRIGGERED** | ❌ |
| C011 | WALL_CLICK_OBJ_REMOVED_ROTATE | Object removed, rotate | ✅ |
| C012 | WALL_OBJECT_GENERATOR_ROTATE | Empty hand, generate | ✅ |
| C013 | WALL_SINGLE_OBJECT_STORAGE_ROTATE | Store/retrieve | ✅ |
| C014 | WALL_LAUNCH_SINGLE_SQUARE_OBJECT | **EVENT-TRIGGERED** | ❌ |
| C015 | WALL_LAUNCH_DOUBLE_SQUARE_OBJECT | **EVENT-TRIGGERED** | ❌ |
| C016 | WALL_OBJECT_EXCHANGER | Swap object with square | ✅ |
| C017 | WALL_CLICK_OBJ_REMOVED_REMOVE_SENSOR | Remove sensor after | ✅ |
| C018 | WALL_END_GAME | **EVENT-TRIGGERED** (end game sequence) | ❌ |
| C127 | WALL_CHAMPION_PORTRAIT | Special | ✅ |

---

## 6. HOLD Effect Resolution

When `Remote.Effect == C03_EFFECT_HOLD`, the trigger decision is **bistable**:

**Floor sensor (F0276 line ~1670)**:
```c
L0778_i_Effect = L0768_B_TriggerSensor ? C00_EFFECT_SET : C01_EFFECT_CLEAR;
```

**Wall sensor (F0275 line ~1518–1519)**:
```c
if (L0756_i_SensorEffect == C03_EFFECT_HOLD) {
    L0756_i_SensorEffect = L0753_B_DoNotTriggerSensor ? C01_EFFECT_CLEAR : C00_EFFECT_SET;
}
```

**Effect on remote target**: The resolved effect is dispatched via `G0059_auc_Graphic562_SquareTypeToEventType` mapping:
- SET on a WALL square → `C06_EVENT_WALL`
- CLEAR on a DOOR → `C01_EVENT_DOOR_ANIMATION`  
- etc.

---

## 7. Local Effects

Two local effects fire **immediately** (no timeline event):
- `C10_EFFECT_ADD_300XP_STEAL_SKILL` — calls `F0269_SENSOR_AddSkillExperience` to add 300 Steal XP
- **Rotation effect** — stored in globals `G0403–G0406`, deferred to `F0271_SENSOR_ProcessRotationEffect` after all sensors on the square are processed (ensures only one rotation per square per step)

Rotation effect (`F0271`) relinks sensor chains: unlinks the first sensor of the target type and appends it after the last sensor of that type, effectively rotating the sensor list.

---

## 8. Once-Only Sensors

```c
// F0272 line ~1180
if (P0575_ps_Sensor->Remote.OnceOnly) {
    M044_SET_TYPE_DISABLED(sensor);  // set type to C000_DISABLED
}
```
After triggering, the sensor type is set to `C000_DISABLED`. It remains on the square (in the thing list) but will never fire again.

---

## 9. Event Type Map (G0059 — DATA.C line ~470)

```
Square Type → Event Type (G0059_auc_Graphic562_SquareTypeToEventType[7]):
  C00_ELEMENT_WALL       → C06_EVENT_WALL
  C01_ELEMENT_CORRIDOR  → C05_EVENT_CORRIDOR
  C02_ELEMENT_PIT       → C09_EVENT_PIT
  C03_ELEMENT_STAIRS    → C00_EVENT_NONE
  C04_ELEMENT_DOOR      → C10_EVENT_DOOR
  C05_ELEMENT_TELEPORTER → C08_EVENT_TELEPORTER
  C06_ELEMENT_FAKEWALL  → C07_EVENT_FAKEWALL
```

This maps a square's base type to the event type that should fire when a sensor targets it.

---

## 10. Bugs / Anomalies

- **BUG0_29**: Explosions can trigger floor sensors after being teleported (should levitate). `F0264_MOVE_IsLevitating` returns `C0_FALSE` for explosions. MOVESENS.C:893.
- **BUG0_09**: Discarding a thing to make storage room may trigger an unwanted sensor on the source square. MOVESENS.C:1996.
- **BUG0_26**: Explosions fall into pits — not considered levitating. `F0264_MOVE_IsLevitating` should return `C1_TRUE` for explosions. MOVESENS.C:152.
- **BUG0_78**: Door closing wounds wrong body part — missing parentheses in bitmask combine. TIMELINE.C:766.

---

## 11. Source Citations

| Function | File | Line | Role |
|----------|------|------|------|
| `F0267_MOVE_GetMoveResult_CPSCE` | MOVESENS.C | ~700 | Entry — calls sensor processing |
| `F0276_SENSOR_ProcessThingAdditionOrRemoval` | MOVESENS.C | ~1553 | Floor sensor dispatch |
| `F0275_SENSOR_IsTriggeredByClickOnWall` | MOVESENS.C | ~1309 | Wall sensor click routing |
| `F0268_SENSOR_AddEvent` | MOVESENS.C | ~1000 | Add event to timeline |
| `F0272_SENSOR_TriggerEffect` | MOVESENS.C | ~1154 | Effect dispatch: local vs remote |
| `F0271_SENSOR_ProcessRotationEffect` | MOVESENS.C | ~1100 | Deferred rotation |
| `F0248_TIMELINE_ProcessEvent6_Square_Wall` | TIMELINE.C | 1136–1350 | Wall square event processing |
| `G0059_auc_Graphic562_SquareTypeToEventType` | DATA.C | ~470 | Square type → event type |
| DEFS.H sensor/effect constants | DEFS.H | 1256–1305 | Type and effect enums |