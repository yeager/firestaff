# Nexus V1 Group Behavior - Creature Group Coordination

## Summary
Nexus V1 has NO group behavior system. Each creature is independent.
DM1 V1 has cell-based quarter-square formation: up to 4 creatures share a dungeon square.

## 1. Nexus V1 - Flat Per-Creature Model
Nexus_V1_CreatureManager: flat Nexus_Creature active[] array, no grouping.
Per-creature loop in nexus_v1_creatures_tick() - no spatial indexing.
No coordination, no shared aggro, no swarm targeting.
Creatures can overlap positions; no collision handling.

## 2. DM1 V1 - Cell-Based Group System
File: src/dm1/dm1_v1_group_management_pc34_compat.c
M11_Group: mapX/Y, cell[4] (NW/NE/SW/SE sub-cells), creatureIndex[4], creatureCount (0-4), delayFleeingFromTarget.
4 sub-cells per square. Creatures dynamically shift cells during melee.
DM1_ATTR_PREFER_BACK_ROW: ranged creatures prefer rear cells.
DM1_ATTR_ATTACK_ANY_CHAMPION: any cell attacks any champion.
Group movement: synchronized, primary+secondary direction selection, cell occupancy check.
F0209_GROUP_ProcessEvents29to41: group-level behavior decision.

## 3. Comparison
| Feature              | Nexus V1           | DM1 V1                         |
|----------------------|--------------------|--------------------------------|
| Group structure      | None (flat array)  | cell-based (up to 4/square)   |
| Formation system     | None               | quarter-square cell assignment|
| Cell shifting        | None               | ACTION_ADJUST_CELL            |
| Group movement       | per-creature       | synchronized group move       |
| Back-row preference  | None               | PREFER_BACK_ROW attr         |
| Attack-any           | None               | ATTACK_ANY_CHAMPION attr     |
| Shared aggro         | None               | all group members aggro      |
| Swarming             | None               | all attack same champion     |
| Collision handling   | None                | cell occupancy check        |

## 4. DM1 Group Coordination Details

4a. Giggler Steal - Shared Resource
On steal success: entire group flees with shared fleeDelayTicks (F0193).
Nexus: no Giggler, no shared flee.

4b. Melee Cell Adjustment
resolve_quarter_square_melee_cell_adjustment() (GROUP.C:2388-2393):
- Check optimal cell when in attack range
- Shift to adjacent free cell if blocked by group member
- 75% chance to stay in back row if PREFER_BACK_ROW
- Prevents all 4 targeting same champion

4c. Group Primary Direction
m11_group_calc_party_relation(): primary = axis with greater distance to party, secondary = perpendicular.

## 5. What Nexus Needs for Parity
1. Group struct: Nexus_Group { x, y, creature_indices[4], creature_count }
2. Cell occupation check
3. Cell adjustment action
4. Synchronized group movement
5. Back-row logic for ranged creatures
6. Shared aggro across group
