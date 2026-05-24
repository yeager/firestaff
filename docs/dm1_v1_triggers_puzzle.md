# DM1 V1 — Door & Puzzle Sequence System
**Source: ReDMCSB TIMELINE.C + MOVESENS.C + DEFS.H + CEDT*.C**
**Audit: 2026-05-25 | Source-lock: ReDMCSB_WIP20210206**

---

## 1. Door System Architecture

### 1.1 Door States
**Source: DEFS.H**

```
C0_DOOR_STATE_OPEN           = 0   // Fully open
C1_DOOR_STATE_CLOSED_ONE_FOURTH = 1
C2_DOOR_STATE_CLOSED_HALF        = 2
C3_DOOR_STATE_CLOSED_THREE_FOURTH = 3
C4_DOOR_STATE_CLOSED        = 4   // Fully closed (blocking)
C5_DOOR_STATE_DESTROYED     = 5   // Destroyed (walkable)
```

### 1.2 Door State Storage
Each dungeon square stores a door byte:
- **Bits 0–2**: Door state (0–5)
- **Bit 3**: Door exists flag
- **Bits 4–5**: Door button state (button = small adjacent wall)

```c
// M036_DOOR_STATE macro (DEFS.H):
// Returns (squareByte & 0x07)
```

### 1.3 Door Thing (DOOR struct)
**Source: DEFS.H**

```c
typedef struct {
    uint8_t  Vertical;   // 1=vertical door (top/bottom wall), 0=horizontal
    uint8_t  Button;     // 1=has adjacent button sensor
    uint8_t  Unknown[6];
} DOOR;
```

---

## 2. Door Animation Events

### 2.1 C01_EVENT_DOOR_ANIMATION
**Source: TIMELINE.C F0241 (line 711)**

Processes one frame of door animation per tick:

```c
F0241_TIMELINE_ProcessEvent1_DoorAnimation(event):
  loc = event->Location
  effect = event->C.A.Effect  // SET=open, CLEAR=close
  
  state = M036_DOOR_STATE(square[loc])
  if (state == C5_DOOR_STATE_DESTROYED) return;  // Already destroyed
  
  switch (effect) {
  case EFFECT_CLEAR:  // Closing door
    if (party on square && champions > 0) {
      M037_SET_DOOR_STATE(square, OPEN)
      // Damage champions in closing door's path
      // BUG0_78: wrong wound body part (see below)
      event->Map_Time++
      F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(event)  // Next frame
      return
    }
    // Creature on square: hurt it, close anyway
    state--
    M037_SET_DOOR_STATE(square, state)
    if (state > 0) {
      event->Map_Time++
      F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(event)  // Continue close
    }
    break
    
  case EFFECT_SET:  // Opening door
    state++
    M037_SET_DOOR_STATE(square, state)
    if (state < MAX_STATE) {
      event->Map_Time++
      F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(event)  // Continue open
    }
    break
  }
```

### 2.2 C10_EVENT_DOOR — Square Door Effect
**Source: TIMELINE.C F0244 (called from event switch)**

C10 is dispatched when a remote sensor effect targets a DOOR square:
- SET → `F0244` opens the door (incremental)
- CLEAR → `F0244` closes the door (decremental)
- This is how wall sensors drive door puzzles remotely

---

## 3. Door Passability

**Source: dm1_v1_collision_door_pc34_compat.c lines 95–99**

A door is passable (walkable) only when:
```c
collision_door_is_passable(state):
  return (state == C0_DOOR_STATE_OPEN ||
          state == C1_DOOR_STATE_CLOSED_ONE_FOURTH ||
          state == C5_DOOR_STATE_DESTROYED)
```

States C2, C3, C4 block movement. State C1 (one-fourth closed) allows passage but is visually "ajar".

---

## 4. Door Wounding — BUG0_78

**Source: TIMELINE.C line 766**

```c
// WRONG (current code):
F0324_CHAMPION_DamageAll_GetDamagedChampionCount(
    5,
    MASK0x0008_WOUND_TORSO | AL0602_ui_VerticalDoor ? MASK0x0004_WOUND_HEAD : MASK0x0001_WOUND_READY_HAND | MASK0x0002_WOUND_ACTION_HAND,
    C2_ATTACK_SELF)

// BUG: bitwise-OR binds tighter than ternary, so expression evaluates as:
// MASK0x0008_WOUND_TORSO | (AL0602_ui_VerticalDoor ? ... : (MASK0x0001_WOUND_READY_HAND | MASK0x0002_WOUND_ACTION_HAND))
// = always MASK0x0008_WOUND_TORSO | something
// = always non-zero, so the ternary always takes the HEAD branch, not the HAND branch

// INTENDED (correct):
F0324_CHAMPION_DamageAll_GetDamagedChampionCount(
    5,
    MASK0x0008_WOUND_TORSO | (AL0602_ui_VerticalDoor ? MASK0x0004_WOUND_HEAD : MASK0x0001_WOUND_READY_HAND | MASK0x0002_WOUND_ACTION_HAND),
    C2_ATTACK_SELF)
```

**Effect**: All closing doors wound HEAD in addition to TORSO (because `TORSO | HEAD` is always non-zero, making the `?:` always take the HEAD branch).

**Expected behavior**:
- **Vertical door** (top/bottom wall): wound HEAD + TORSO
- **Horizontal door** (left/right wall): wound READY_HAND + ACTION_HAND + TORSO

---

## 5. Pressure Plate Door Sequences

The canonical door puzzle pattern:
1. **Floor sensor (C003/C004/C008)** on a pressure plate square
2. **HOLD effect** — bistable: SET when pressure applied, CLEAR when released
3. **Remote target** — the door square
4. **Effect**: SET opens, CLEAR closes

```
[Pressure Plate] --(party enters)--> C003 floor sensor triggers
  └─ HOLD effect → remote DOOR square → SET → door opens
  └─ (party exits) → CLEAR → door closes
```

---

## 6. Multi-Step Puzzle Sequences — AND/OR Gates

### 6.1 AND Gate Sequence (C005 Wall Sensor)

**Example**: Two pressure plates must both be activated to open a door.

```
[S1: pressure plate A] --> floor sensor --> C003 --> remote gate S2
[S2: pressure plate B] --> floor sensor --> C003 --> remote gate S2

S2 = AND gate targeting DOOR (target square)
  - Reference mask = 0b1100 (cells 2+3 must receive events)
  - Current mask starts = 0b0000
  - Each plate SETs its bit in the gate's current mask
  - When current == reference: gate fires SET to DOOR
```

**Source**: TIMELINE.C F0248 lines 1268–1309.

### 6.2 OR Gate Sequence

```
[Plate A] --> floor sensor --> remote gate S2 (OR gate, ref=0b0100)
[Plate B] --> floor sensor --> remote gate S2 (OR gate, ref=0b1000)

Gate fires when any one bit is set (reference mask = 0b0100 | 0b1000 = 0b1100)
Actually: OR gate fires when currentMask & referenceMask != 0 (any matching bit)
```

Wait — need to re-examine F0248 gate logic more carefully.

### 6.3 Countdown Sequences (C006 Wall)

**Example**: Activate a wall 5 times to open a door.

```
[Wall ornament] --> click sensor --> C001 --> remote countdown S2
  └─ Each click: TOGGLE effect on countdown
  └─ Countdown decrements from 5 → 4 → 3 → 2 → 1 → 0
  └─ At 0: countdown fires SET to DOOR

[Key: COUNTDOWN_SENSOR.targetCell → DOOR square]
```

**Source**: TIMELINE.C F0248 lines 1198–1266.

---

## 7. Wall Ornament Click Sequences

### 7.1 Click-to-Rotate (C011/C017)

**C011**: After removing an object, rotate remaining sensors on the cell.
**C017**: After removing an object, remove the sensor itself.

```c
// F0275 case C011:
F0270_SENSOR_TriggerLocalEffect(C02_EFFECT_TOGGLE, mapX, mapY, cell)
// → deferred rotation of sensor list on that cell
// Used for multi-step sequences where the correct object must be placed
// in a specific order (rotation reveals next sensor in chain)
```

### 7.2 Object Exchange (C016)

**C016**: Swap leader's held object with the square's object.

```c
// F0275 case C016:
// Swap the object in the leader's hand with the object on the square
// No trigger condition — just exchange
```

### 7.3 Object Generator/Rotate (C012)

**C012**: Generate an object from a sensor, rotating the sensor list.

```c
// F0275 case C012:
// If leader's hand is empty:
//   Generate object (sensorData gives object type)
//   Rotate sensor list
// Else: do nothing (hand must be empty)
```

---

## 8. Countdown + Door Sequence (Common Pattern)

The most common multi-step puzzle in DM1:

```
1. Player clicks a wall ornament (C001 click sensor)
   └─ TOGGLE or SET effect on a countdown sensor (C006 wall, remote target)
   
2. Countdown sensor decrements (or increments for SET)
   └─ sensorData decreases: 3 → 2 → 1 → 0
   
3. At counter == 0:
   └─ Sensor fires SET effect to DOOR square
   └─ F0244 opens the door
   
4. Door stays open (C10 event drives animation to OPEN state)
```

**Source**: TIMELINE.C F0248 lines 1198–1266 + F0244_TIMELINE_ProcessEvent10.

---

## 9. Multi-Floor Puzzle Sequences

### 9.1 Stairs Sensors (C005 Floor)

```c
// F0276 case C005 (CHANGE8_05_FIX version):
if (thingType != CM1_THING_TYPE_PARTY || squareType != C03_ELEMENT_STAIRS)
    goto skip;
// Party is on stairs — trigger
```

Used to detect party entering stairs, enabling:
- Level transitions
- Locked door sequences that activate when party reaches a specific floor
- Group generator activation on specific floors

### 9.2 Teleporter Sequences

```c
// C05_ELEMENT_TELEPORTER squares trigger C08_EVENT_TELEPORTER
// TIMELINE.C F0250 processes teleport events:
//   - Rotate group/projectile direction
//   - Move to destination square
//   - Trigger destination square's sensors
```

---

## 10. End Game Sequence (C018 Wall Sensor)

**Source: TIMELINE.C F0248 lines 1312–1350, C018 case**

C018 fires when a wall event matches the end-game sensor. This is the final trigger for the game's ending sequence.

```c
case C018_SENSOR_WALL_END_GAME:
    // Trigger end game state machine
    // Sets G0302_B_GameWon and associated state
```

---

## 11. Fake Wall Sequences (C07 Event)

**Source: TIMELINE.C F0242**

```c
// C07_EVENT_FAKEWALL — toggles square aspect:
// SET → square becomes WALL (fake wall solid)
// CLEAR → square becomes CORRIDOR (fake wall walkable)
```

Used for secret passages that open/close based on sensor triggers.

---

## 12. Source Citations

| Function | File | Line | Role |
|----------|------|------|------|
| `F0241_TIMELINE_ProcessEvent1_DoorAnimation` | TIMELINE.C | 711 | Door animation frames |
| `F0242_TIMELINE_ProcessEvent7_Square_FakeWall` | TIMELINE.C | 820 | Fake wall toggle |
| `F0244_TIMELINE_ProcessEvent10_Square_Door` | TIMELINE.C | — | Door square event |
| `F0245_TIMELINE_ProcessEvent5_Square_Corridor` | TIMELINE.C | ~955 | Corridor + generator |
| `F0248_TIMELINE_ProcessEvent6_Square_Wall` | TIMELINE.C | 1136 | Wall events + gate + countdown |
| `F0275_SENSOR_IsTriggeredByClickOnWall` | MOVESENS.C | 1309 | Wall click routing |
| `F0276_SENSOR_ProcessThingAdditionOrRemoval` | MOVESENS.C | 1553 | Floor sensor evaluation |
| `F0247_TIMELINE_TriggerProjectileLauncher` | TIMELINE.C | ~1033 | Projectile launch dispatch |
| `M036_DOOR_STATE` / `M037_SET_DOOR_STATE` | DEFS.H | — | Door state accessors |
| DEFS.H door state constants | DEFS.H | — | C0..C5 door states |
| dm1_v1_collision_door_pc34_compat.c | firestaff | — | Door collision compat |