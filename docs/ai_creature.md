# DM1 V1 — Creature AI Behavior

**Source-locked to:** ReDMCSB WIP20210206 GROUP.C, DEFS.H
**Companion:** src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c

---

## 1. Behavior State Machine

Creatures use a per-group Behavior enum (GROUP.C:1372–1378, DEFS.H):

| Value | Name | Description |
|---|---|---|
| C0 | BEHAVIOR_WANDER | Move toward party if smelt; otherwise random dirs |
| C5 | BEHAVIOR_FLEE | Afraid — flees from square where party/danger was last seen |
| C6 | BEHAVIOR_ATTACK | Attack party; ranged casters line up shots; melee closes in |
| C7 | BEHAVIOR_APPROACH | Not in attack range — moves toward last known party position |

Values C2–C4 (USELESS) are declared but **never assigned** (GROUP.C:329, 476 note BUG0_00).

Behavior is stored per-group (Group->Behavior), not per-creature, but individual
creatures fire separate UPDATE_BEHAVIOR_CREATURE_N events (38–41) for per-creature
attack timing (GROUP.C:2126–2127).

---

## 2. Behavior Transition Logic (GROUP.C F0209 GROUP_ProcessEvents29to41)

### Wandering (C0) — lines 2100–2148

WANDER
  ├─ See party? (F0200_GetDistanceToVisibleParty > 0)
  │   ├─ In attack range? → BEHAVIOR_ATTACK (C6)
  │   └─ Not in range? → BEHAVIOR_APPROACH (C7)
  └─ No sight
      ├─ Smell party? (F0201 + smell range check)
      │   └─ Try primary dir → possible move → WANDER or same dir
      └─ No smell
          └─ Random direction walk (M004_RANDOM(4) + movement possible test)

### Approach (C7) — lines 2236–2256

- Same Manhattan distance check as WANDER, but path-found condition succeeds.
- If creature can see party while approaching → transitions to ATTACK (C6).
- If path lost → falls back to WANDER (C0).

### Attack (C6) — lines 2073–2134, 2376–2410

- Melee (attack range == 1): F0207_GROUP_IsCreatureAttacking, quarter-square cell adjustment.
- Ranged (attack range > 1): checks alignment (same row or column),
  F0207 line 1695 branch → projectile creation via F0212_PROJECTILE_Create.
- Giggler special: F0193_GROUP_StealFromChampion — steals random item slot, then flees
  (C5) with 3/4 probability.
- Quarter-square creatures shift cells before attacking; back-row attackers have 3/4
  chance to stay in back row.

### Flee (C5) — lines 2289–2308, 2336–2340

- Triggered by: Giggler steal, danger on square, taking damage when HP low.
- Uses F0201 primary direction as **opposite** (flee from target, not toward).
- T0209094 label: tries primary dir → secondary → opposite-primary →
  opposite-secondary → give up and fall through to WANDER (C0).

---

## 3. Event System

Timeline drives all behavior. Key events (DEFS.H:956–960):
- C37_EVENT_UPDATE_BEHAVIOR_GROUP — recalculate group-wide behavior
- C38..C41_EVENT_UPDATE_BEHAVIOR_CREATURE_0..3 — per-creature attack timing

Reaction events (GROUPED BY C29..C36):
- C30: party adjacent — force ATTACK if not already attacking/fleeing
- C31: party adjacent — bump-triggered
- C32..C36: aspect update events (visual only)

Events are accumulated even when Life is Frozen (BUG0_14 noted at GROUP.C:1982).

---

## 4. Creature Type Attributes

Creature attributes in Ranges struct (DEFS.H):
- M054_SIGHT_RANGE / M055_SMELL_RANGE / M056_ATTACK_RANGE macros
- SIDE_ATTACK flag (0x0004): creature can see/attack in all 4 directions
- ATTACK_ANY_CHAMPION flag (0x0010): creature can target any party member
- PREFER_BACK_ROW flag: attackers that stay in back row when possible

Creature properties from DATA.C creature tables:
- Wariness (0–15+): higher = more cautious, avoids teleporters
- Movement speed: ticks per move/animation cycle
- Attack damage, attack range, attack type (melee/ranged)

---

## 5. Key Source Citations

| Function | File:Line | Role |
|---|---|---|
| F0200_GROUP_GetDistanceToVisibleParty | GROUP.C:1315 | Line-of-sight distance to party |
| F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal | GROUP.C:1417 | Smell-based direction |
| F0197_GROUP_IsViewPartyBlocked | GROUP.C:1176 | Wall/square blocks sight check |
| F0198_GROUP_IsSmellPartyBlocked | GROUP.C:1214 | Wall/square blocks smell check |
| F0207_GROUP_IsCreatureAttacking | GROUP.C:1645 | Melee/ranged attack resolution |
| F0193_GROUP_StealFromChampion | GROUP.C:1013 | Giggler steal behavior |
| F0209_GROUP_ProcessEvents29to41 | GROUP.C:1837 | Main behavior FSM |
| T0209094_FleeFromTarget | GROUP.C:2290 | Flee direction selection |
| MASK0x0080_IS_ATTACKING | DEFS.H:606 | Per-creature attacking bit |
| C0..C7_BEHAVIOR_* | DEFS.H:1372–1378 | Behavior enum |