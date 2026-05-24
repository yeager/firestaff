# DM1 V1 — Timer-Based Triggers
**Source: ReDMCSB TIMELINE.C**
**Audit: 2026-05-25 | Source-lock: ReDMCSB_WIP20210206**

---

## 1. Timeline System Overview

The timeline is a **binary heap** of events sorted by `(MapIndex, GameTime, Type, Priority, address)`.

**Key structures** (TIMELINE.C lines 1–130):

```c
EVENT*    G0370_ps_Events;         // Event pool
uint16_t* G0371_pui_Timeline;    // Binary heap (event indices)
uint16_t  G0372_ui_EventCount;   // Current event count
uint16_t  G0373_ui_FirstUnusedEventIndex; // Free-list head
uint16_t  G0369_EventMaximumCount; // Max events (platform-specific)
```

### Sorting Key (F0234 — `IsEventABeforeEventB`):
```c
// Primary: earlier GameTime first
if (TIME(A) < TIME(B)) return TRUE;
// Secondary: higher Type_Priority first
// (Type is high byte, Priority is low byte — so type compared first)
// Tertiary: lower memory address (= lower array index) wins
```

### Event Structure (EVENT):
```c
union Map_Time {       // 32-bit: map index (high 16) + game time (low 16)
    uint32_t full;
    struct { uint16_t MapIndex; uint16_t Time; } s;
};
uint8_t   Type;        // Event type (C00..C79)
uint8_t   Priority;     // Subtype priority (0–3)
struct {
    uint16_t MapX;
    uint16_t MapY;
} Location;
union C {              // Payload varies by event type
    uint16_t Slot;          // For C60/C61: group thing
    uint16_t Ticks;         // For group events: ticks remaining
    uint16_t SoundIndex;     // For C20: sound index
    uint16_t Defense;       // For C72/C74/C77/C78: defense delta
    uint16_t Attack;        // For C75: attack value
    struct { uint8_t Cell; uint8_t Effect; } A; // For square events
    ...
};
```

---

## 2. Adding Events to Timeline

**F0238_TIMELINE_AddEvent_GetEventIndex_CPSE** (TIMELINE.C ~487):

```c
// 1. Get event from free list (avoids allocation overhead)
unusedIdx = G0373_ui_FirstUnusedEventIndex;
event = &G0370_ps_Events[unusedIdx];
*event = *P0514_ps_Event;  // Copy event data
G0373_ui_FirstUnusedEventIndex = ((UNUSED_EVENT*)event)->NextUnusedEventIndex;

// 2. Insert into binary heap
G0371_pui_Timeline[G0372_ui_EventCount] = unusedIdx;
F0236_TIMELINE_FixPlacement(G0372_ui_EventCount++);
```

If the event pool is exhausted (BUG0_18), the event is silently dropped:
```c
if (G0372_ui_EventCount >= G0369_EventMaximumCount) {
    return 0; // BUG0_18: projectile stays airborne, explosion never ends, etc.
}
```

---

## 3. Timeline Processing — Main Loop

**F0248_TIMELINE_ProcessEvents** (TIMELINE.C ~1822):
```c
while (F0240_TIMELINE_IsFirstEventExpired_CPSE()) {  // Check if first event's time ≤ G0313_ul_GameTime
    F0239_TIMELINE_ExtractFirstEvent(&event);         // Pop + remove from heap
    ProcessEventByType(event);                         // Dispatch
}
```

**IsFirstEventExpired** (F0240, line ~682):
```c
return (G0372_ui_EventCount &&
        M030_TIME(G0370_ps_Events[G0371_pui_Timeline[0]].Map_Time) <= G0313_ul_GameTime)
```

---

## 4. Timer-Based Event Types

### 4.1 Group Movement — C60/C61 (Silent/Audible)
**Source: F0265_MOVE_CreateEvent60To61_MoveGroup (MOVESENS.C:169) + F0252**

```c
// When a group is blocked by party/other group:
F0265_MOVE_CreateEvent60To61_MoveGroup(groupThing, destX, destY, mapIndex, audible):
    event.Type = audible ? C61_EVENT_MOVE_GROUP_AUDIBLE : C60_EVENT_MOVE_GROUP_SILENT
    event.Map_Time = SET_MAP_AND_TIME(mapIndex, G0313_ul_GameTime + 5)  // +5 ticks delay
    event.Location = {destX, destY}
    event.C.Slot = groupThing
    F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&event)
```

**F0252_ProcessEvents60to61_MoveGroup** retries the move after 5 ticks.
- If destination still blocked: re-queue another C60 event (+5 more ticks)
- If destination clear: move the group

### 4.2 Group Generator — C65 Enable + C006 Floor Sensor
**Source: F0246_TIMELINE_ProcessEvent65_EnableGroupGenerator (TIMELINE.C:1010)**

```c
// F0246: Enable first DISABLED sensor on square as C006_GROUP_GENERATOR
if (sensor.Type == C000_SENSOR_DISABLED) {
    sensor.Remote.Type_Data |= C006_SENSOR_FLOOR_GROUP_GENERATOR;
    return;  // Enable only the first disabled sensor
}
```

**C006 activation flow** (F0245_TIMELINE_ProcessEvent5_Square_Corridor, TIMELINE.C ~955):
1. C006 floor sensor is placed on a square (initially as C000_DISABLED or with C006 type)
2. When a creature steps on the square (C007 floor sensor triggers → no wait)
3. Alternatively: a countdown (C006 wall) or gate (C005 wall) dispatches to enable a generator
4. The `F0246` event changes a `C000_DISABLED` sensor to `C006_GROUP_GENERATOR`
5. The next creature step (C007 floor sensor) triggers the C006 → spawns group

Actually: F0245 (C05 Corridors) calls F0246 EnableGroupGenerator for the first disabled sensor, which then becomes a C006 generator.

### 4.3 Countdown Timers — C006 Wall Countdown
**Source: TIMELINE.C F0248 lines 1198–1266**

A countdown sensor starts at `sensorData` (0–511). Each SET increments; each TOGGLE/CLEAR decrements.

```c
// Per wall event on a countdown sensor:
data = sensor->sensorData;
if (eventEffect == EFFECT_SET) {
    if (data < 511) data++;
} else {
    data--;
}
if (data <= 0) {
    // Fire effect to remote target AND disable sensor
    M044_SET_TYPE_DISABLED(sensor);
    F0268_SENSOR_AddEvent(effect, targetX, targetY, targetCell, effect, gameTime + delay);
}
```

### 4.4 Door Animations — C01/C10 Events
**Source: F0241_TIMELINE_ProcessEvent1_DoorAnimation (TIMELINE.C:711)**

```c
// C01_EVENT_DOOR_ANIMATION — incremental door state machine
// Door states: C0=OPEN, C1=CLOSED_ONE_FOURTH, C2=CLOSED_HALF,
//              C3=CLOSED_THREE_FOURTH, C4=CLOSED, C5=DESTROYED

switch (effect) {
case EFFECT_CLEAR:  // Close door (decrement state toward CLOSED)
    if (party on square && champions > 0) {
        M037_SET_DOOR_STATE(square, OPEN);
        // damage champions on closing
        event.Map_Time++;  // Add another tick for next animation frame
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&event);
        return;
    }
    // creature handling...
    L0596_i_DoorState--;
    M037_SET_DOOR_STATE(square, L0596_i_DoorState);
    if (L0596_i_DoorState > 0) {
        event.Map_Time++;
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&event);  // Continue animation
    }
    break;

case EFFECT_SET:   // Open door (increment state toward OPEN)
    L0596_i_DoorState++;
    M037_SET_DOOR_STATE(square, L0596_i_DoorState);
    if (L0596_i_DoorState < MAX_DOOR_STATE) {
        event.Map_Time++;
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&event);  // Continue animation
    }
    break;
}
```

### 4.5 Fake Wall Timings — C07 Event
**Source: F0242_TIMELINE_ProcessEvent7_Square_FakeWall (TIMELINE.C ~820)**

```c
// C07_EVENT_FAKEWALL — toggle square aspect between WALL and CORRIDOR
// Effect: SET → make fake wall solid, CLEAR → make walkable
// Used for secret doors that open after a delay
```

### 4.6 Sound Events — C20
**Source: TIMELINE.C:1894**
```c
case C20_EVENT_PLAY_SOUND:
    F0064_SOUND_RequestPlay_CPSD(event.C.SoundIndex, loc.MapX, loc.MapY, MODE_PLAY_IF_PRIORITIZED);
```

### 4.7 Spell Effect Timers — C70..C79
| Event | Effect | Processing |
|-------|--------|-----------|
| C70 | Light | Call F0257 + update palette |
| C71 | Invisibility | Decrement party count; redraw if 0 |
| C72 | Champion Shield | Remove champion defense bonus, redraw |
| C73 | Thieves Eye | Decrement party count |
| C74 | Party Shield | Remove party defense bonus |
| C75 | Poison Champion | Apply poison tick, decrement count |
| C77 | Spell Shield | Remove spell shield defense |
| C78 | Fire Shield | Remove fire shield defense |
| C79 | Footprints | Decrement party count |

---

## 5. Game Clock — G0313_ul_GameTime

**Game time** (`G0313_ul_GameTime`) advances by 1 each VBLANK (video refresh cycle, ~60 Hz on NTSC, ~50 Hz on PAL).

**Party movement** updates `G0362_l_LastPartyMovementTime`, used for scent tracking:
```c
G0407_s_Party.Scents[...].Location = {destX, destY, destMapIndex}
G0407_s_Party.ScentStrengths[...] = 0
F0317_CHAMPION_AddScentStrength(destX, destY, MASK0x8000_MERGE_CYCLES | 24)
```

**Timed door animations**: each frame adds `event.Map_Time++` (one tick) before re-queueing.

---

## 6. Priority Queue — Binary Heap Properties

The timeline binary heap invariant: **parent fires before children**.

```c
// F0236_FIX_PLACEMENT — restore heap invariant after insert
while (P0512_ui_TimelineIndex > 0) {
    parent = (P0512_ui_TimelineIndex - 1) >> 1;
    if (EventIsBefore(event, timeline[parent])) {
        timeline[P0512_ui_TimelineIndex] = timeline[parent];
        P0512_ui_TimelineIndex = parent;
    }
}
// Then check children for "sift down"
```

---

## 7. Source Citations

| Function | File | Line | Role |
|----------|------|------|------|
| `F0233_TIMELINE_Initialize_CPSE` | TIMELINE.C | 50 | Allocate event pool + timeline |
| `F0238_TIMELINE_AddEvent_GetEventIndex_CPSE` | TIMELINE.C | 487 | Insert event |
| `F0239_TIMELINE_ExtractFirstEvent` | TIMELINE.C | 664 | Pop first event |
| `F0240_TIMELINE_IsFirstEventExpired_CPSE` | TIMELINE.C | 682 | Check if event is due |
| `F0234_TIMELINE_IsEventABeforeEventB` | TIMELINE.C | 126 | Sort comparator |
| `F0236_TIMELINE_FixPlacement` | TIMELINE.C | 334 | Heap insert fix |
| `F0241_TIMELINE_ProcessEvent1_DoorAnimation` | TIMELINE.C | 711 | Door animation |
| `F0242_TIMELINE_ProcessEvent7_Square_FakeWall` | TIMELINE.C | 820 | Fake wall toggle |
| `F0245_TIMELINE_ProcessEvent5_Square_Corridor` | TIMELINE.C | ~955 | Corridor + generator enable |
| `F0246_TIMELINE_ProcessEvent65_EnableGroupGenerator` | TIMELINE.C | 1010 | Enable generator sensor |
| `F0248_TIMELINE_ProcessEvent6_Square_Wall` | TIMELINE.C | 1136 | Wall events + gate/countdown |
| `F0252_TIMELINE_ProcessEvents60to61_MoveGroup` | TIMELINE.C | ~1531 | Group retry move |
| `G0313_ul_GameTime` | GAMELOOP.C | — | Master game clock |
| `G0059_auc_Graphic562_SquareTypeToEventType` | DATA.C | ~470 | Square→event mapping |