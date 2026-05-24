# DM1 V1 — Creature Grouping and Coordination

**Source-locked to:** ReDMCSB WIP20210206 GROUP.C, DEFS.H
**Companion:** src/dm1/dm1_v1_group_management_pc34_compat.c

---

## 1. Group Structure

A **group** is a collection of 1–4 creatures occupying a single 2x2 cell grid
on one map square:

```
Square 2x2 grid:
  Cell 0 (NW) | Cell 1 (NE)
  -----------+----------
  Cell 2 (SW) | Cell 3 (SE)
```

- Each cell holds one creature (0xFF = empty)
- Creatures can be different types within the same group
- Groups are the unit of movement, behavior, and most AI decisions
- Group state tracked in G0375_auc_ActiveGroups array (max 16)
- Active group count in G0377_i_ActiveGroupCount

Group data per entry:
- mapX, mapY: square position
- Behavior: C0/C5/C6/C7 (per-group behavior)
- Cells[4]: creature index per cell (0xFF = empty)
- Aspect[4]: per-creature aspect flags (attacking, etc.)
- Hit points, creature count

---

## 2. Group Movement Coordination

When a direction is selected by F0203_GetFirstPossibleMovementDirectionOrdinal,
the **entire group** moves together via F0206_GROUP_SetDirectionGroup:

```
F0206_GROUP_SetDirectionGroup(ActiveGroup*, direction, creatureCount, creatureSize)
```

- direction: 0=N, 1=E, 2=S, 3=W
- creatureSize: C1_SIZE_HALF_SQUARE or full-size

The F0203 priority for direction selection (primary → secondary →
opposite-primary → opposite-secondary → random) ensures **all creatures
in the group move in the same direction** as a coordinated unit.

---

## 3. Per-Creature Attack Timing

Even though group behavior is shared, individual creatures fire separate
events (C38..C41 = UPDATE_BEHAVIOR_CREATURE_0..3) for per-creature
attack timing (GROUP.C:2126–2127):

```
Group->Behavior = C6_BEHAVIOR_ATTACK;
NextEvent.Type = C38_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + creatureIndex;
F0208_GROUP_AddEvent(&NextEvent,
    F0179_GROUP_GetCreatureAspectUpdateTime(ActiveGroup, i, FALSE));
```

This allows different creatures in the same group to attack at different
times (staggered), creating the appearance of coordinated-but-independent timing.

---

## 4. Quarter-Square Cell Adjustment (GROUP.C:2388–2410)

For creatures with size = QUARTER_SQUARE (tiny creatures like Giggler):
- 4 creatures can occupy the same square, one per cell
- During attack, quarter-square creatures may shift cells:
  - Back-row attackers: 3/4 chance to stay back, 1/4 chance to move forward
  - Cell adjustment tries to keep attacking creatures in front rows
- packed_group_cell / packed_group_cell_update macros manage cell assignments
  (GROUP.C cell packing in F0209 T0209085)

---

## 5. Group vs. Single Creature

- **Half-square**: up to 2 per square (cells 0+2 or 1+3), 2-wide formation
- **Full-square**: 1 per square, occupies entire 2x2 grid
- **Quarter-square**: 1–4 per square, individual cells

F0176_GROUP_GetCreatureOrdinalInCell: scan cells array to find which
creature is at a given cell index.

---

## 6. Group Spawning / Group Generators

Groups are placed by:
1. **Initial dungeon load**: group data in dungeon DAT files
2. **Group generator sensors** (C006_SENSOR_FLOOR_GROUP_GENERATOR):
   event-triggered, not floor-triggered; activated by TIMELINE events
3. **Event-driven spawning**: F0193_GROUP_AddAllActiveGroups adds all groups
   from dungeon data to the active group array

Group generators are skipped by floor sensor evaluation (F0276 case C006 → skip).

---

## 7. Key Source Citations

| Function | File:Line | Role |
|---|---|---|
| F0175_GROUP_GetThing | GROUP.C (F0175) | Walk thing list at (x,y) for TYPE_GROUP |
| F0176_GROUP_GetCreatureOrdinalInCell | GROUP.C (F0176) | Scan cells array for creature |
| F0193_GROUP_AddAllActiveGroups | GROUP.C:1098 | Add all groups from dungeon data |
| F0194_GROUP_RemoveAllActiveGroups | GROUP.C:1082 | Remove all groups |
| F0196_GROUP_InitializeActiveGroups | GROUP.C:1135 | Init active group array |
| F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal | GROUP.C:1556 | Coordinated direction |
| F0205_GROUP_SetDirection | GROUP.C:1592 | Per-creature direction |
| F0206_GROUP_SetDirectionGroup | GROUP.C:1623 | Group-wide direction |
| F0209 T0209085_SingleSquareMove | GROUP.C:1850–1908 | Cell shift during movement |
| G0375_auc_ActiveGroups | GROUP.C (state) | Active group array |
| G0377_i_ActiveGroupCount | GROUP.C (state) | Active group count |
| G0384_auc_GroupMovementTestedDirections | GROUP.C (state) | Per-direction tested flag |