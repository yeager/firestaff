# DM1 V1 Creature Terrain Interaction — Source Audit

## ReDMCSB Source
- firestaff_pc34_flattened_amalgam.c:2868-2965: F0202_GROUP_IsMovementPossible
- firestaff_pc34_flattened_amalgam.c:12186-12197: Pit fall damage logic
- firestaff_pc34_flattened_amalgam.c:2524: BUG0_66 smoke on pit fall
- firestaff_pc34_flattened_amalgam.c:2254: F0267_MOVE_GetMoveResult_CPSCE creature death handling
- firestaff_pc34_flattened_amalgam.c:2917-2932: Teleporter creature movement logic + wariness check
- DEFS.H:1617: MASK0x0020_LEVITATION creature attribute

## F0202_GROUP_IsMovementPossible — Full Logic

Line 2898 pit check:
  squareType != C02_ELEMENT_PIT
  OR (imaginary AND allowImaginaryPitsFakeWalls)
  OR NOT open
  OR creature has LEVITATION

Line 2917-2932 teleporter check:
  squareType == TELEPORTER AND open AND wariness >= 10
  THEN get teleporter from square thing list
  THEN check scope includes CREATURES
  THEN check creature type allowed on destination map
  THEN check door state passable (door height vs creature height)
  THEN check NON_MATERIAL attribute bypasses door

## Creature Pit Interaction

### Falling Through Pit (line 12186)
Condition: open pit AND NOT imaginary AND NOT levitating
- Target: destination map at same (mapX, mapY)
- Damage: health = health / 2 (integer divide)
- If creature type not allowed on destination map: removed (no death event)
- Smoke explosion at source (BUG0_66 — cosmetic, falls into pit before visible)

### Creature Types That Can Walk Into Pits
- Non-levitating creatures: blocked unless pit is imaginary/closed
- Levitating creatures (MASK0x0020): always pass open pits
- Imaginary pits: passable with allowImaginaryPitsFakeWalls flag

### Creature Death on Pit Fall (BUG0_66 comment, line 2524)
When a creature dies by falling through a pit, possessions are dropped
on the destination map (special case in F0267_MOVE_GetMoveResult_CPSCE).
Smoke explosion placed on source map (bug — smoke should be on destination).

## Creature Teleporter Interaction

### Teleporter Scope Rules (line 2919)
- Scope must include MASK0x0001_SCOPE_CREATURES
- Creature type must be allowed on target map (F0139_DUNGEON_IsCreatureAllowedOnMap)
- Creature must pass door check if teleporter is on a door square

### Door Check at Teleporter (line 2929)
  NOT (doorState <= creatureHeight OR doorState == DESTROYED OR creature NON_MATERIAL)
If creature is too tall for door gap, movement blocked.

### Wariness Threshold
Wariness >= 10 (Vexirk, Materializer/Zytaz, Demon, Lord Chaos, Red Dragon/Dragon)
Only these creature types will voluntarily enter teleporters.

### BUG0_67 (line 3220)
G0380_T_CurrentGroupThing not set before the F0139 creature-type-allowed check
when non-party-map creature considers entering teleporter. Can cause wrong
teleport behavior for groups not currently on party map.

## Creature-Wall Interaction

- Wall squares (C00_ELEMENT_WALL): always impassable, no exceptions
- F0202 checks wall first, then door, then pit, then teleporter

## Firestaff Implementation

File: src/shared/firestaff_pc34_flattened_amalgam.c
- F0202_GROUP_IsMovementPossible fully implemented with pit/teleporter/door/wall checks
- creature_ai_tick_event_group_move() handles non-party-map creature movement (line 3220)
- orch_handle_creature_tick_group_move_compat() schedules creature move events

File: src/memory/memory_tick_orchestrator_pc34_compat.c:2462
  orch_handle_creature_tick_group_move_compat() — deferred group move scheduling

## STATUS: ALIGNED
Creature-terrain interaction fully source-locked. Pit damage, teleporter scope,
levitation, wariness all traced to ReDMCSB. BUG0_66 and BUG0_67 documented.
