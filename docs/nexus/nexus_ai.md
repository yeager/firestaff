# Nexus V1 AI System Audit

## Summary
The historical Nexus creature study contains a minimal DM1-shaped
patrol/chase/attack model, but it is not a source-authenticated Saturn AI.
Retail Nexus MNS/creature records are retained as provenance; production
action/AI dispatch remains fail-closed until the SLEV/DM.BIN owner and runtime
writeback are captured. DM1 comparisons below are lineage references only.

## 1. Nexus V1 AI Architecture
File: src/nexus/nexus_v1_creatures.c
States: 1=patrol, 2=chase (dist<=3), 3=attack (dist<=1)
No smell, no vision, no LOS, no groups, no scripts.
State struct: type_index, health, x, y, facing, alive, state, ai_timer

## 2. DM1 V1 AI Architecture
Files: src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c, include/dm1_v1_creature_ai_behavior_pc34_compat.h, src/dm1/dm1_v1_group_management_pc34_compat.c

DM1_BEHAVIOR: WANDER(0), FLEE(5), ATTACK(6), APPROACH(7)
DM1_ACTION: MOVE, ATTACK, FLEE_MOVE, CAST_SPELL, STEAL, ADJUST_CELL
Per-creature ranges bitfield: DM1_SMELL_RANGE(r)=((r)>>8)&0x000F, DM1_ATTACK_RANGE(r)=(r)>>12
DM1_ATTR: SEE_INVISIBLE(0x0800), NIGHT_VISION(0x1000), LEVITATION(0x0020), NON_MATERIAL(0x0040), ARCHENEMY(0x2000), ATTACK_ANY_CHAMPION(0x0010), PREFER_BACK_ROW(0x0008)
Event queue per creature/group: nextEventType + nextEventDelayTicks
F0209_GROUP_ProcessEvents29to41 timeline processing
F0823 projectile system: Fireball, Lightning Bolt, Poison Cloud, Slime
Giggler steal: F0193 steal attempt, flee on success

## 3. Comparison

| Feature              | Nexus V1           | DM1 V1                    |
|----------------------|--------------------|---------------------------|
| States               | 3                  | 6+ behavior states        |
| Sensor model         | Manhattan distance | Smell + Vision ranges    |
| LOS check            | None               | F0200 ray-cast, walls block |
| Aggro range          | hardcoded <=3      | per-type smell_range     |
| Attack range         | hardcoded <=1      | per-type attack_range    |
| Group coordination   | None               | cell-based formation     |
| Projectile attacks   | None               | full projectile VM       |
| Event timeline       | None               | event queue per group    |
| Scripting            | None               | none in DM1 (hardwired)  |
| Fleeing AI           | None               | BEHAVIOR_FLEE via fear   |

## 4. Nexus V2 Status
No changes to creature AI. nexus_v2_*.c: atmosphere, lighting, upscaler, render pipeline.

## 5. Conclusion
Do not describe the study model as playable Nexus parity. The next evidence
must bind creature state reads, action dispatch, timing, damage/writeback,
SLEV ownership and SFX/HUD consumers from an authentic Saturn trace.
