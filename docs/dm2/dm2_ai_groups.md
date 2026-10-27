# DM2 V1 Group Behavior — Creature Coordination

## Source Evidence
- `skproject/SKULLWIN/c_creature.cpp` — `c_creature` struct (group cell packing, `b_1a`/`b_17` coordination)
- `skproject/SKULLWIN/c_ai.cpp` — `DM2_THINK_CREATURE` (multi-creature planning)
- `skproject/SKULLWIN/c_creature.h` — `SPX_Creature` (multi-segment creature), `c_creature` class
- `src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c` — DM1 group behavior (ReDMCSB F0209_GROUP_*)

## DM1 Group Behavior (ReDMCSB Reference)
DM1 group behavior from ReDMCSB GROUP.C:
- **Leader-follower cells**: Group members occupy specific packed cells (2 bits per creature: 0=front, 1=right, 2=back, 3=left)
- **F0202_GROUP_IsMovementPossible**: Tests if group formation can shift
- **F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal**: Wall avoidance for groups
- **T0209085_SingleSquareMove**: Group movement in one square
- **Direction coherence**: F0205_GROUP_SetDirection / F0206_GROUP_SetDirectionGroup — all members same direction
- **Distance tracking**: F0200_GROUP_GetDistanceToVisibleParty — group-level distance to party

DM1 groups act as a **rigid formation** with a leader. Movement is synchronized; all members move together.

## DM2 Group Behavior: More Independent Coordination
DM2 `c_creature` struct supports multi-segment creatures via `SPX_Creature` (up to 4 HP pools per creature visual):
```
SPX_Creature:
  w_00: Next object ID
  w_02: Next possession ID
  b_04: SPX_Creature type (per-segment creature type)
  b_05: Position in group (each segment = a creature in the group)
  w_06..w_0e: Hit points for segments 1-4
```

The `b_05` field (position index) replaces DM1 packed 2-bit cell system.

## Group Coordination Mechanism in DM2
- **Individual `b_1a` state bytes**: DM1 had one behavior per group; DM2 has individual `b_1a` per `c_creature` entry
- **Shared target (`w_18`)**: All segments of multi-segment creature share same target x/y
- **DM2_THINK_CREATURE**: Called per creature, not per group — individual segment decisions
- **No formation-locked movement**: DM2_WALK_NOW is per-creature, not per-formation
- **No group cell packing**: The 2-bit packed-cell system (DM1 F0209) is absent in DM2

## New DM2 Group Behaviors
- **Creatures in same group can have different `b_1a` actions**: one segment attacks, another fetches item
- **Merchant groups**: multiple creatures with merchant (0x17-0x2B) behaviors coordinated
- **Swarm/flanking**: individual creature decision-making allows organic swarm behavior
- **Multi-segment creatures**: 4 independently-tracked hit-point pools per visual creature

## Shared AI Spec
From c_creature.cpp lines 2817/5866:
```c
RG1P = DOWNCAST(c_aidef, DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD(...));
```
Creatures sharing same AI spec share behavioral flags. Each `c_creature` has its own state byte (`b_1a`), so **same type /= synchronized behavior**.

## Contrast Summary
| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Movement | Formation-locked, group as unit | Per-creature independent |
| Cell packing | 2 bits per member, fixed positions | No packing; per-creature x/y |
| Direction | group-wide direction set | Individual `b_1a` per creature |
| Target sharing | Group-level target | Per-creature (SPX multi-segment shared) |
| State machine | One behavior per group | One `b_1a` byte per creature entry |
| Behavior diversity | Same for all group members | Can differ per creature in same group |
