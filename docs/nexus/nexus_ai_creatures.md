# Nexus V1 Creature AI Behavior - 3-State Patrol/Chase/Attack

## Nexus V1 Implementation
File: src/nexus/nexus_v1_creatures.c
Header: include/nexus_v1_creatures.h

State struct: type_index, health, x, y, facing, alive, state (0=idle,1=patrol,2=chase,3=attack,4=flee), ai_timer

State transitions:
  dist = |cx-party_x| + |cy-party_y|
  if dist<=3: state=2 (chase);  if dist<=1: state=3 (attack)
  else: state=1 (patrol)
  if state==2 and ai_timer%(6-speed)==0: move one step toward party

States used: 1=PATROL (no movement), 2=CHASE (move), 3=ATTACK (no move, attack implied)
States NEVER set: 0 (idle), 4 (flee)
Spawn: all creatures start in PATROL state. No scatter, no alert.

AI Timer: incremented each tick. Movement gated by ai_timer%(6-speed). Higher speed -> more frequent moves. speed range 1-5.

## DM1 V1 Equivalent
DM1 uses behavior+action pair: BEHAVIOR_WANDER(0), BEHAVIOR_FLEE(5), BEHAVIOR_ATTACK(6), BEHAVIOR_APPROACH(7)
Plus ACTION: MOVE, ATTACK, FLEE_MOVE, CAST_SPELL, STEAL, ADJUST_CELL

Key DM1 behaviors absent from Nexus:
- WANDER: random patrol when no party detected
- FLEE: fear-triggered retreat (Giggler after steal, low HP)
- APPROACH: visible but not in range

DM1 quarter-square cell system: 4 sub-cells per group, creatures shift cells for optimal melee positioning. Nexus: all creatures 1-cell, no formation shifting.

## Key Gaps
1. No idle/wander: patrol state has no active behavior - creatures sit still
2. No flee: state 4 never triggered
3. No approach state: jumps patrol->chase, no visible-but-out-of-range
4. No scent/vision sensor: Manhattan ignores walls and creature type
5. No quarter-square cells: all 1-cell, no formation
6. No facing update during patrol
