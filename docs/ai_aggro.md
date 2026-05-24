# DM1 V1 — Creature Aggro / Alert / Detection

**Source-locked to:** ReDMCSB WIP20210206 GROUP.C, MOVESENS.C, DEFS.H

---

## 1. Detection Model

Creatures detect the party through **two independent senses**:

### Sight (F0200 + F0197)

- Uses M054_SIGHT_RANGE(creatureInfo->Ranges) macro
- Plus random variation: SIGHT_RANGE += RANDOM(XXX_RANGE+1) + RANDOM(SMELL_RANGE+1)
  (GROUP.C:1396)
- Requires line-of-sight: walks axis from creature to party, tests each
  intermediate square with F0197_GROUP_IsViewPartyBlocked
- Walls and closed doors block sight
- Special flag MASK0x0004_SIDE_ATTACK: if set, creature sees in all directions
  regardless of facing (GROUP.C:1350)

### Smell (F0201 + F0198)

- Uses M055_SMELL_RANGE(creatureInfo->Ranges) macro
- Smell effective distance: ((smellRange+1) >> 1) — halved, rounded up
- Still requires line-of-sight (same blocking predicate, F0198_IsSmellPartyBlocked)
- Smell is a **backup**: if creature can't see party but smell range covers,
  it uses smell direction for movement (GROUP.C:1441)

Key Formula (GROUP.C:1441):
```
smellEffective >= G0381_distanceToParty
AND F0199_GetDistanceBetweenUnblockedSquares(..., F0198_IsSmellPartyBlocked)
```

---

## 2. Aggro Trigger — F0200 Return Value

F0200_GROUP_GetDistanceToVisibleParty returns 0 if party is not visible,
non-zero Manhattan distance if visible. This is the primary aggro test
(GROUP.C:2101):
```
if (F0200_GROUP_GetDistanceToVisibleParty(...))
  → Party is visible → set BEHAVIOR_ATTACK or BEHAVIOR_APPROACH
```

Called with CM1_WHOLE_CREATURE_GROUP to test the group as a whole, or with
a specific creature index (0–3) to test individual creatures in a group.

---

## 3. Line-of-Sight Blocking

### F0197_GROUP_IsViewPartyBlocked (GROUP.C:1176–1213)

For each step along the axis path:
1. Get square type at (x, y)
2. If SQUARE_IS_WALL → blocked
3. If SQUARE_IS_DOOR and door is closed → blocked
4. If SQUARE_IS_FAKEWALL → blocked (fake walls are opaque!)
5. Otherwise → continue

Note: closed doors block sight even if party is on the other side.
Teleporters and pits do NOT block sight (the ray passes through them).

### F0198_GROUP_IsSmellPartyBlocked (GROUP.C:1214–1238)

Same logic as F0197 but for smell:
- Doors block smell (closed doors block the scent)
- Same wall/fakewall blocking rules

---

## 4. Aggro Range by Creature Type

Ranges are stored in the creature info struct per creature type.
Representative ranges (from DATA.C creature tables, approximate):
- Melee creatures: attack range = 1
- Ranged casters (Vexirk, Wizard Eye, Demon): attack range = 2–7
- Lord Chaos / Red Dragon: attack range = 7, high wariness

Sight/smell ranges vary by creature type but are generally 1–4 squares.

---

## 5. Reaction Events — C30/C31 (GROUP.C:2006–2042)

When the party **bumps into a group** or attacks it physically:
- Event CM3_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT fires
- Creature transitions to BEHAVIOR_ATTACK (C6) if not already attacking/fleeing
- 3/4 chance to look in a random direction to search for party if not visible
- After reaction, creature re-evaluates at next UPDATE_BEHAVIOR_GROUP event

---

## 6. Danger Detection (Flee Triggers)

Creatures also flee from **danger on their own square**:
- Event C29_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE
- Triggers: Poison Cloud on square, closing door, 3+ surrounding flux cages
- Lord Chaos teleport when danger is sensed (GROUP.C:2208 MEDIA297 conditional)
- After fleeing, creature falls back to WANDER (C0) at last known safe position

---

## 7. Key Source Citations

| Function | File:Line | Role |
|---|---|---|
| F0197_GROUP_IsViewPartyBlocked | GROUP.C:1176 | Sight blocking test |
| F0198_GROUP_IsSmellPartyBlocked | GROUP.C:1214 | Smell blocking test |
| F0199_GROUP_GetDistanceBetweenUnblockedSquares | GROUP.C:1239 | Axis walk with predicate |
| F0200_GROUP_GetDistanceToVisibleParty | GROUP.C:1315 | Primary aggro distance |
| F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal | GROUP.C:1417 | Smell-based direction |
| M054_SIGHT_RANGE | DEFS.H macro | Sight range from creature data |
| M055_SMELL_RANGE | DEFS.H macro | Smell range from creature data |
| M056_ATTACK_RANGE | DEFS.H macro | Attack range from creature data |
| C29_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE | DEFS.H:950 | Danger on square event |
| C30_EVENT_CREATE_REACTION_EVENT_30_PARTY_IS_ADJACENT | DEFS.H:951 | Party adjacent event |
| MASK0x0004_SIDE_ATTACK | DEFS.H:1598 | All-direction attack flag |