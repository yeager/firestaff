# DM1 V1 C006 Group Generator Runtime Materialization (2026-05-20)

Scope: land the narrow successful empty-square runtime path for C006 floor group-generator events in the Phase 20 tick orchestrator. This follows ReDMCSB source order where the corridor event resolves the generator, `F0185_GROUP_GetGenerated` builds the GROUP data, and `F0267_MOVE_GetMoveResult_CPSCE` performs world placement.

## Source Evidence

- `TIMELINE.C:912-1007` / `F0245_TIMELINE_ProcessEvent5_Square_Corridor`: C05 corridor events scan the square list, find `C006_SENSOR_FLOOR_GROUP_GENERATOR`, decode count/randomization, multiplier, direction RNG, call `F0185_GROUP_GetGenerated`, optionally request audible `M560_SOUND_BUZZ`, disable once-only or delayed generators, and schedule `C65_EVENT_ENABLE_GROUP_GENERATOR`.
- `TIMELINE.C:1009-1031` / `F0246_TIMELINE_ProcessEvent65_EnableGroupGenerator`: C65 scans the target square and changes the first disabled sensor back to C006.
- `GROUP.C:481-548` / `F0185_GROUP_GetGenerated`: obtains a group thing slot, initializes slot/end marker, direction, count, creature type, health, and cells, calls `F0267_MOVE_GetMoveResult_CPSCE(..., CM1_MAPX_NOT_ON_A_SQUARE, 0, mapX, mapY)`, and requests `M560_SOUND_BUZZ` after successful placement.
- `GROUP.C:414-447` / `F0183_GROUP_AddActiveGroup`: when a group enters the party map, active group state stores cells, active index, home/prior square, last-move time, direction slots, and aspect state.
- `GROUP.C:311-337` / `F0180_GROUP_StartWandering`: starts behavior update event 37 one tick later for the square's group.
- `MOVESENS.C:169-191` / `F0265_MOVE_CreateEvent60To61_MoveGroup`: blocked group placement creates event 60/61 five ticks later with group thing in `Event.C.Slot`.
- `MOVESENS.C:316-907` / `F0267_MOVE_GetMoveResult_CPSCE`: destination processing owns group insertion, party/group destination deferral (`830-844`), movement sound (`847-853`), active-group add/remove (`855-876`), and source-map restoration.
- `MOVESENS.C:1553-1793` / `F0276_SENSOR_ProcessThingAdditionOrRemoval`: floor C006 generators are skipped by ordinary thing-addition floor sensor processing (`1704-1705`); the C05 corridor event path is the generator dispatcher.

## Implemented Slice

- The Phase 20 `TIMELINE_EVENT_GROUP_GENERATOR` trigger now applies `F0860_RUNTIME_HandleGroupGenerator_Compat` results to `GameWorld_Compat` for successful empty-square placement.
- A generated group is appended to `DungeonThings_Compat.groups`, linked at the target square head, and initialized with source-derived type/count/direction/cells/health. Firestaff's decoded Phase 9 thing list has no unused-slot free list yet, so append represents the available group slot for this compat runtime slice.
- Party-map generation seeds one `CreatureAIState_Compat` entry as the active-group analogue of `F0183_GROUP_AddActiveGroup`.
- Successful generated placement now schedules the `F0180_GROUP_StartWandering` analogue as `TIMELINE_EVENT_CREATURE_TICK` at `gameTick + 1` on the generated group square. The event carries the generated group slot, creature type, and wander state in the existing Phase 16 event fields.
- Delayed/once-only sensor state is applied and delayed generators schedule the existing C65 re-enable event.
- Sound dispatch now distinguishes the unconditional `F0185` successful-placement buzz from the optional sensor-audible `F0245` buzz.

## Deferred

- Full `F0267` collision/defer path for destination party/group squares, including materialized event 60/61 handling and later insertion.
- Exact original unused-slot freelist semantics; the current decoded runtime appends one group slot.
- Projectile impact, teleporter, pit, cross-map, and full movement-sound side effects outside the empty-square generator materialization path.
