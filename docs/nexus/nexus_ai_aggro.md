# Nexus V1 Aggro System - Aggro Range and Line-of-Sight

## Summary
Nexus V1 has NO aggro system. Single Manhattan distance check (dist<=3 chase, dist<=1 attack).
No LOS check, no scent model, no per-type aggro range, no wall occlusion.

DM1 V1: two-stage sensor (smell + vision), per-type ranges, perception attributes.

## 1. Nexus V1 Aggro
nexus_v1_creatures_tick():
  dist = |cx-party_x| + |cy-party_y|
  if dist<=3: state=2; if dist<=1: state=3; else: state=1

Aggro range: hardcoded 3 for ALL types. Attack range: hardcoded 1 for ALL types.
Sensor: Manhattan distance - ignores walls, doors, type attributes.
LOS: NONE - detects through walls. Scent: NONE.

Whats missing: per-type aggro/attack range, LOS, scent model, smell range, vision attributes, aggro memory, de-aggro beyond range check.

## 2. DM1 V1 Aggro System
ranges bitfield: DM1_SMELL_RANGE(r)=((r)>>8)&0x000F (bits 8-11), DM1_ATTACK_RANGE(r)=(r)>>12 (bits 12-15)
Two-stage detection: Stage 1=scent (NOT blocked by walls), Stage 2=LOS ray-cast (walls block)
F0200_GROUP_GetDistanceToVisibleParty: returns distance or 255 if blocked.
DM1_ATTR: SEE_INVISIBLE(0x0800), NIGHT_VISION(0x1000), LEVITATION(0x0020), NON_MATERIAL(0x0040), ARCHENEMY(0x2000)

## 3. LOS Implementation
DM1: ray-cast from creature to party along primary direction axis.
Blocks: WALL squares, closed doors.
Does NOT block: other creatures, floor items, pit squares (without levitation).

Nexus: no LOS. abs(cx-party_x)+abs(cy-party_y) passes through walls unchanged.
Creature on other side of wall aggroes instantly. No peek mechanics.

## 4. DM1 Aggro State Machine
BEHAVIOR_WANDER -> (smell range) -> APPROACH
APPROACH -> (visible + smell) -> APPROACH
APPROACH -> (attack range) -> ATTACK
ATTACK -> (out of range) -> APPROACH
ATTACK -> (HP low or smell lost) -> FLEE
WANDER -> (HP threshold) -> FLEE

Nexus is flat: patrol<->chase<->attack only. No approach, no flee, no memory.

## 5. Gap Summary
| Feature               | Nexus V1           | DM1 V1                        |
|-----------------------|--------------------|-------------------------------|
| Aggro range           | hardcoded 3        | per-type smell_range (0-15)  |
| Attack range          | hardcoded 1        | per-type attack_range (0-15) |
| LOS check             | None               | ray-cast, walls block        |
| Smell model           | None               | scent range + propagation    |
| Perception attrs      | None               | 5 attributes                 |
| Aggro memory          | None               | last known position          |
| De-aggro              | range-based only   | smell OR LOS lost            |

## 6. Minimum Viable Aggro for Nexus Parity
1. Per-creature-type aggro range (replace hardcoded 3)
2. Wall occlusion for LOS (check wall map before chase state)
3. Scent propagation (flood-fill from party up to smell_radius)
4. Aggro memory (last known pos; approach if LOS lost but smell active)
5. De-aggro (reset to patrol if neither LOS nor smell for N ticks)
