# Nexus V1 Special Squares Audit

## 1. Square Type System

Nexus DGN files store square types as 16-bit values with the lower 5 bits
extracted (mask 0x1F) in nexus_v1_level_get_square(). This matches the
DM1 convention where square types 0-31 are meaningful.

Square type 0 = solid wall (no passage). Types 1-31 = various floor/passable
squares with different properties.

## 2. Teleporter Squares

The descriptions below are investigation hypotheses, not runtime contracts.
The verified corpus proves DGN bytes and Structure1F records, but not that
their low bits select DM1-like event types or that `SDDRVS.TSK` owns dispatch.
SDDRVS is currently classified as the Saturn sound-driver task; SLEV/SAL
event ownership still requires original-Saturn capture.

In DM1, teleporter squares (type 9 and 10) are hardwired in the game loop:
- Type 9: level teleport (D0-D7 transitions)
- Type 10: intra-level teleport (same level, different position)

An earlier draft described Nexus teleporters as scripted via SDDRVS.TSK.
That claim is not source-locked and must not enable runtime routes.

Teleporter rendering in Nexus 3D: nexus_viewport.c draws floor/ceiling for
open squares (sq != 0). Wall faces drawn where sq == 0. Teleporter overlays
likely handled by 3D geometry in DGN files.

## 3. Door Squares

Nexus uses 3D polygon geometry for doors rather than DM1 sprite overlays.

DM1: Doors rendered as 2D sprites overlaid on wall squares during viewport
rendering. Door state (open/closed) toggles the sprite.

Nexus: Door geometry is evidence for a door-like surface, but the
open/closed selector and event owner remain unproven.
Open/close state and its owner are not source-locked.

Door squares in Nexus view: nexus_viewport_render() draws wall faces where
sq == 0. When a wall square is a door, the 3D geometry switches between
open/closed door mesh variants based on game state.

## 4. Trap Squares

Nexus trap ownership and behavior remain unresolved; SDDRVS is not a proven
trap script file.

DM1 traps are hardwired to specific square types:
- Type 6: alarm trap (all creatures chase party)
- Type 7: chute/trapdoor (party falls to next level)
- Type 11: teleport trap

No trap route is enabled from a DGN square value alone.

## 5. Stairs / Level Transition Squares

Stairs in Nexus are handled via the 3D geometry in DGN files.

DM1 stairs: square types 2 (up) and 3 (down) rendered as distinct sprites.

Nexus stairs: a possible geometry variant in the DGN mesh; the destination
and transition dispatch are not source-locked.

## 6. Special Squares Summary vs DM1

| Square Type     | DM1 Behavior        | Nexus V1 Behavior        |
|-----------------|---------------------|--------------------------|
| Wall (0)        | Blocks movement     | 3D wall geometry         |
| Floor (1)       | Normal passage      | 3D floor mesh            |
| Stairs Up (2)   | Go up one level     | Unresolved; capture-gated |
| Stairs Down (3) | Go down one level   | Unresolved; capture-gated |
| Teleporter (9/10)| Hardwired jump     | Unresolved; capture-gated |
| Door (type 8)   | 2D sprite overlay   | 3D door geometry         |
| Trap types      | Hardwired effects   | Unresolved; capture-gated |
| Other special   | Varies              | Unresolved; capture-gated |

## 7. Implementation Notes from Firestaff

nexus_v1_dungeon.c: only square type loading is implemented (grid parsing).
Square type semantics (teleporter vs door vs trap) are NOT yet implemented;
their owner and dispatch need original-Saturn capture. SDDRVS.TSK must not be
treated as a script VM without that evidence.

nexus_viewport.c: draws walls where sq == 0, floors where sq != 0.
No special handling yet for door animations, teleporter effects, or trap triggers.

The DGN geometry blob (post-grid portion) likely contains per-square
3D mesh identifiers that map to wall type, door state, stairs variant, etc.
