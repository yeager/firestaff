# DM1 V1 — Pathfinding

**Source-locked to:** ReDMCSB WIP20210206 GROUP.C, MOVESENS.C, DEFS.H

---

## 1. Algorithm — No A*

DM1 V1 does **not** use A* or any general-purpose pathfinder.
Instead it uses a **greedy direction-first approach** with collision checks:

1. Compute primary direction toward target (Manhattan, longer axis wins)
2. Compute secondary direction (shorter axis)
3. Try primary → try secondary → try opposite-primary → try random
4. For each direction: F0202_GROUP_IsMovementPossible tests wall/door/group/party
5. If all directions blocked → creature idles (WANDER sticks)

This is **not** pathfinding in the graph-search sense. Creatures navigate
incrementally, one square at a time, with no look-ahead beyond 1–2 squares.

---

## 2. Core Movement Functions

### F0202_GROUP_IsMovementPossible (GROUP.C:1457)

Tests whether a creature/group can move in a given direction from a square.
Checks in order:
1. Bounds (map edges)
2. Square type (wall = blocked, door = blocked unless open)
3. Other groups on target square
4. Flux cage count (creature-specific wariness check for teleporters)
5. Creature-type allowed-on-map check (for teleporters)

Sets blocking flags:
- G0387: CurrentGroupMovementBlockedByWall
- G0388: CurrentGroupMovementBlockedByDoor
- G0389: CurrentGroupMovementBlockedByParty
- G0390: CurrentGroupMovementBlockedByGroup

### F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal (GROUP.C:1556)

Returns the first direction (from priority list) where movement is possible.
Priority: primary → secondary → opposite-primary → opposite-secondary → random.

### F0204_GROUP_IsArchenemyDoubleMovementPossible (GROUP.C:1576)

Special check for Lord Chaos / Red Dragon (archenemies): tests whether the
creature can move **two squares** in the given direction (teleport range).
Used in the approach phase for archenemies only.

---

## 3. Distance Calculation

### F0200_GROUP_GetDistanceToVisibleParty (GROUP.C:1315)

```
Manhattan distance = |groupX - partyX| + |groupY - partyY|
BUT only counts squares where F0197_IsViewPartyBlocked returns FALSE.
Uses F0199_GetDistanceBetweenUnblockedSquares which walks the axis.
```

Line-of-sight is **ray-cast style**: it walks the X or Y axis from group to
party, checking each intermediate square for wall/door obstruction.

### F0199_GROUP_GetDistanceBetweenUnblockedSquares (GROUP.C:1239)

Generic routine shared by sight and smell. Takes a blocking-predicate function
pointer. Used for both F0197 (view blocked) and F0198 (smell blocked).

---

## 4. Group Movement — F0206 SetDirectionGroup

When a valid direction is found, F0206_GROUP_SetDirectionGroup sets the
direction for **all creatures in the group** simultaneously (GROUP.C:1623).
For half-square creatures, each creature gets its own cell assignment
within the 2x2 grid (F0205_GROUP_SetDirection, GROUP.C:1592).

---

## 5. Approach Behavior Pathfinding

When behavior is C7 (APPROACH), GROUP.C:2264–2267:
```
Try primary dir (must succeed)
Try secondary dir with 1/2 random chance
Try opposite-primary dir (must succeed)
Try opposite-secondary with 1/4 random chance
```

Lord Chaos (archenemy) gets double-movement: F0204 check allows it to
teleport two squares in one update cycle (GROUP.C:2276–2279).

---

## 6. Wariness and Teleporter Handling

Creature Wariness field (from DATA.C creature tables) governs teleporter use:
- Wariness < 10: normal movement into teleporters allowed
- Wariness >= 10 (Vexirk, Materializer/Zytaz, Demon, Lord Chaos, Red Dragon):
  only enter teleporters if creature type is allowed on destination map
- BUG0_67 noted at GROUP.C:1924: G0380_T_CurrentGroupThing not set before
  the teleporter-allowability check, so wrong creature type may be tested.

---

## 7. Key Source Citations

| Function | File:Line | Role |
|---|---|---|
| F0199_GROUP_GetDistanceBetweenUnblockedSquares | GROUP.C:1239 | Axis walk for sight/smell |
| F0200_GROUP_GetDistanceToVisibleParty | GROUP.C:1315 | Line-of-sight distance |
| F0202_GROUP_IsMovementPossible | GROUP.C:1457 | Single-step movement test |
| F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal | GROUP.C:1556 | Direction priority |
| F0204_GROUP_IsArchenemyDoubleMovementPossible | GROUP.C:1576 | Archenemy 2-square move |
| F0205_GROUP_SetDirection | GROUP.C:1592 | Per-creature direction |
| F0206_GROUP_SetDirectionGroup | GROUP.C:1623 | Group-wide direction |
| F0197_GROUP_IsViewPartyBlocked | GROUP.C:1176 | Sight blocking predicate |
| F0198_GROUP_IsSmellPartyBlocked | GROUP.C:1214 | Smell blocking predicate |
| G0381_ui_CurrentGroupDistanceToParty | GROUP.C (state) | Manhattan distance state |
| G0382_i_CurrentGroupPrimaryDirectionToParty | GROUP.C (state) | Primary direction state |
| G0383_i_CurrentGroupSecondaryDirectionToParty | GROUP.C (state) | Secondary direction state |