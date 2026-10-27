# DM1 V1 — Touchscreen Sensor System

## Source Lock
ReDMCSB WIP20210206: MOVESENS.C F0268/F0270/F0271/F0272/F0275/F0276; TIMELINE.C F0248; DEFS.H lines 1256-1305; DATA.C:470; CLIKVIEW.C F0372; G0059_auc_Graphic562_SquareTypeToEventType.

## Sensor Architecture Overview
DM1 V1 has two distinct sensor trigger contexts:
1. **Wall sensors** — triggered by tapping on the dungeon viewport wall areas (front wall, side walls, doors)
2. **Floor sensors** — triggered by movement (party, creatures, objects entering/occupying squares)

Both share the same underlying sensor data structures but use different trigger evaluation functions.

## Square Type → Event Type Mapping
G0059_auc_Graphic562_SquareTypeToEventType[7] (DATA.C:470):

| Square Type | Value | Event Type |
|-------------|-------|------------|
| C00_ELEMENT_WALL | 0 | C06_EVENT_WALL |
| C01_ELEMENT_CORRIDOR | 1 | C05_EVENT_CORRIDOR |
| C02_ELEMENT_PIT | 2 | C09_EVENT_PIT |
| C03_ELEMENT_STAIRS | 3 | C00_EVENT_NONE |
| C04_ELEMENT_DOOR | 4 | C10_EVENT_DOOR |
| C05_ELEMENT_TELEPORTER | 5 | C08_EVENT_TELEPORTER |
| C06_ELEMENT_FAKEWALL | 6 | C07_EVENT_FAKEWALL |

## Touch → Wall Sensor Flow (F0372 → F0275)
CLIKVIEW.C F0372 click in dungeon view calls F0275_IsTriggeredByClickOnWall:

```
1. Classify viewport click → view cell (0=BACK_LEFT, 1=BACK_RIGHT, 2=FRONT_LEFT, 3=FRONT_RIGHT)
2. Compute targetMapX/Y = party position + direction step for far cells (2/3)
3. For far cells: trigger front wall sensor at (targetMapX, targetMapY)
4. For near cells (0/1): no sensor trigger from tap (movement triggers floor sensors)
```

### F0275_IsTriggeredByClickOnWall
MOVESENS.C F0275 (~line 1309):
```
Input: click map position (mx, my), current party position
Output: triggered sensor effect or none

Switch on sensor type at target position:
  C001..C004, C011..C013, C127: click-triggered
  C005, C006, C007, C008, C014, C015, C018: event-triggered only — skip via default:goto
  C000 (DISABLED): always skip
  C127: champion portrait — responds to tap for champion interaction
```

## Wall Sensor Types (DEFS.H lines 1266-1284)
| Type | Name | Trigger | Effect |
|------|------|---------|--------|
| C000 | DISABLED | Never | — |
| C001 | WALL_LEVER | Click | Toggle lever state |
| C002 | WALL_BUTTON | Click | One-shot wall button |
| C003 | WALL_DOOR_BUTTON | Click | Toggle door state |
| C004 | WALL_KEY_REQUIRED | Click | Unlock if champion has key |
| C011 | WALL_TELEPORT_TRIGGER | Click | Teleport to destination |
| C012 | WALL_MESSAGE_TRIGGER | Click | Display text message |
| C013 | WALL_CHAMPION_SUMMON | Click | Add champion to party |
| C127 | PORTRAIT | Click | Champion interaction |

Non-clickable wall sensors (skipped in F0275):
- C005 (AND gate), C006 (OR gate), C007 (launcher), C008 (group launcher), C014, C015, C018

## Floor Sensor Evaluation (F0276)
MOVESENS.C F0276_ProcessThingAdditionOrRemoval (~line 1553):

Called when any thing (party, creature, object) is added to or removed from a dungeon square.

```
Input: thing type, thing position, isAddition boolean
Output: trigger result with effect

switch(sensorType):
  C001: skip if partySquare OR hasObject OR hasGroup
  C002: skip if thingType > GROUP OR partySquare OR hasGroup
  C003: skip if thingType != PARTY OR championCount==0
         data==0: skip if partySquare
         data!=0: trigger if (isAddition AND data==ordinal(partyDir))
  C004: skip if data != objType OR hasSameTypeObj
  C005: skip if thingType != PARTY OR squareType != STAIRS
  C006: skip (group generator — event-triggered only)
  C007: skip if thingType > GROUP OR thingType == PARTY OR hasGroup
  C008: trigger if party has object type (data)
  C009: skip if thingType != PARTY OR !isAddition OR partySquare
        trigger if (data <= version)
```

## HOLD Effect Resolution
When a sensor has EFFECT_HOLD:
```
F0275: if HOLD → SET if not G0293_B_DoNotTriggerSensor, else CLEAR
F0276: if HOLD → SET if G0293_B_TriggerSensor, else CLEAR
```
This converts the HOLD sentinel into a binary trigger based on whether the sensor is currently blocking movement.

## Sensor Event Scheduling
F0268_SENSOR_AddEvent maps effect to timed event:
- TIMELINE.C F0248 processes wall event sensors (including C006 countdown)
- Effects that require deferred execution (e.g., rotation XP gain) are queued as timeline events
- F0270_TriggerLocalEffect: rotation/XP effects on party
- F0271_ProcessRotationEffect: deferred rotation execution
- F0272_TriggerEffect: dispatches to local (in-party) or remote (timeline) handler

## Touch Trigger Priority
When player taps viewport cell 2/3 (front wall):
```
1. F0372 click handler → F0275 wall sensor check
2. If sensor triggered → queue effect → sensor consumed
3. If no sensor → check grabbable object (F0373)
4. If no object → check creature (F0375)
5. If no creature → no action
```
The order of precedence for viewport tap is: wall sensor > floor object > creature attack.

## Fake Wall Tap
Tapping a fake wall square triggers C07_EVENT_FAKEWALL (no physical interaction, but may reveal hidden passage or trigger event). F0275 processes fake wall same as regular wall sensor with appropriate event type from G0059.

## Sensor Source Evidence
- MOVESENS.C F0275 line ~1309: IsTriggeredByClickOnWall switch
- MOVESENS.C F0276 line ~1553: ProcessThingAdditionOrRemoval floor evaluation
- MOVESENS.C F0270/F0271/F0272: local vs remote effect dispatch
- TIMELINE.C F0248: wall event sensor timeline processing
- DEFS.H lines 1256-1305: sensor type/effect constants and classification
- DATA.C:470: G0059 square type to event type mapping
- CLIKVIEW.C F0372: touch front wall sensor entry point
