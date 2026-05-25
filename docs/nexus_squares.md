# Nexus V1 Special Squares Audit

## 1. Square Type System

Nexus DGN files store square types as 16-bit values with the lower 5 bits
extracted (mask 0x1F) in nexus_v1_level_get_square(). This matches the
DM1 convention where square types 0-31 are meaningful.

Square type 0 = solid wall (no passage). Types 1-31 = various floor/passable
squares with different properties.

## 2. Teleporter Squares

In DM1, teleporter squares (type 9 and 10) are hardwired in the game loop:
- Type 9: level teleport (D0-D7 transitions)
- Type 10: intra-level teleport (same level, different position)

In Nexus, teleporters are scripted via SDDRVS.TSK rather than hardwired.
This allows designers to place teleporters anywhere without being constrained
to specific type codes.

Teleporter rendering in Nexus 3D: nexus_viewport.c draws floor/ceiling for
open squares (sq != 0). Wall faces drawn where sq == 0. Teleporter overlays
likely handled by 3D geometry in DGN files.

## 3. Door Squares

Nexus uses 3D polygon geometry for doors rather than DM1 sprite overlays.

DM1: Doors rendered as 2D sprites overlaid on wall squares during viewport
rendering. Door state (open/closed) toggles the sprite.

Nexus: Door geometry embedded in wall/floor meshes in DGN files.
Open/close state likely controlled via the SDDRVS.TSK script VM rather
than a hardwired door type.

Door squares in Nexus view: nexus_viewport_render() draws wall faces where
sq == 0. When a wall square is a door, the 3D geometry switches between
open/closed door mesh variants based on game state.

## 4. Trap Squares

Nexus trap behavior is scripted in SDDRVS.TSK (the task/script file).

DM1 traps are hardwired to specific square types:
- Type 6: alarm trap (all creatures chase party)
- Type 7: chute/trapdoor (party falls to next level)
- Type 11: teleport trap

Nexus traps: declarative rules in script file, allowing any combination
of trigger condition plus effect without being bound to square type numbers.

## 5. Stairs / Level Transition Squares

Stairs in Nexus are handled via the 3D geometry in DGN files.

DM1 stairs: square types 2 (up) and 3 (down) rendered as distinct sprites.

Nexus stairs: likely a geometry variant in the DGN floor/ceiling mesh for
stairs squares, plus an SDDRVS.TSK script that handles level transitions.

## 6. Special Squares Summary vs DM1

| Square Type     | DM1 Behavior        | Nexus V1 Behavior        |
|-----------------|---------------------|--------------------------|
| Wall (0)        | Blocks movement     | 3D wall geometry         |
| Floor (1)       | Normal passage      | 3D floor mesh            |
| Stairs Up (2)   | Go up one level     | 3D stairs mesh + script  |
| Stairs Down (3) | Go down one level   | 3D stairs mesh + script  |
| Teleporter (9/10)| Hardwired jump     | Scripted in TSK           |
| Door (type 8)   | 2D sprite overlay   | 3D door geometry         |
| Trap types      | Hardwired effects   | Scripted in TSK          |
| Other special   | Varies              | Scripted in TSK          |

## 7. Implementation Notes from Firestaff

nexus_v1_dungeon.c: only square type loading is implemented (grid parsing).
Square type semantics (teleporter vs door vs trap) are NOT yet implemented;
those need SDDRVS.TSK script VM integration.

nexus_viewport.c: draws walls where sq == 0, floors where sq != 0.
No special handling yet for door animations, teleporter effects, or trap triggers.

The DGN geometry blob (post-grid portion) likely contains per-square
3D mesh identifiers that map to wall type, door state, stairs variant, etc.
