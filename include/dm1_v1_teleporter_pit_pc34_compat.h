#ifndef FIRESTAFF_DM1_V1_TELEPORTER_PIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_TELEPORTER_PIT_PC34_COMPAT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* DM1 V1 Teleporter/Pit — source-locked rotation/move-removal helpers.
 *
 * ARCHITECTURE NOTE (2026-05-26):
 *   The DM1 V1 orchestrator (memory_tick_orchestrator_pc34_compat.c) and
 *   movement pipeline use world->things->teleporters[] /
 *   DungeonTeleporter_Compat and the on-square pit byte from the dungeon
 *   tile directly. There is no parallel "M11_TeleporterPitState" registry
 *   that runtime code consults; an earlier scaffolding registry was
 *   removed to keep one source-locked control path.
 *
 *   What lives here is the small set of source-bound helpers that
 *   orchestrator and tests still need:
 *
 *     m11_apply_teleporter_rotation        — MOVESENS.C F0262/F0263/F0267
 *                                            party/projectile/object/group
 *                                            rotation kernel
 *     m11_apply_group_teleporter_rotation  — MOVESENS.C F0262 group inner
 *     m11_plan_group_move_removal_after_pit_teleporter
 *                                          — MOVESENS.C F0267 + GROUP.C
 *                                            F0187/F0188/F0189 group
 *                                            removal/drop plan
 *     m11_teleporter_rotation_source_evidence
 *     m11_group_move_removal_source_evidence
 */

#define M11_MAPX_PROJECTILE_ASSOCIATED_OBJECT (-2)

#define M11_TELEPORTER_ROTATE_THING_PARTY      0
#define M11_TELEPORTER_ROTATE_THING_PROJECTILE 1
#define M11_TELEPORTER_ROTATE_THING_OBJECT     2
#define M11_TELEPORTER_ROTATE_THING_GROUP      3

#define M11_GROUP_MOVE_REMOVAL_REASON_NONE          0
#define M11_GROUP_CELL_SINGLE_CENTERED              0xFF
#define M11_CREATURE_SIZE_QUARTER_SQUARE            0
#define M11_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED   1
#define M11_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED   2
#define M11_GROUP_MOVE_REMOVAL_SOUND_ONE_TICK_LATER 2

typedef struct {
    int x;
    int y;
    int destX;
    int destY;
    int destLevel;
    int destFacing;
    int isVisible;
    int soundEffect;
    int absoluteRotation;
} M11_TeleporterDef;

typedef struct {
    int movePrevented;
    int reason;
    int dropMovingCreatureFixedPossessions;
    int dropGroupPossessions;
    int dropGroupPossessionsSoundMode;
    int dropMapX;
    int dropMapY;
    int deleteSourceGroup;
    int deleteMapX;
    int deleteMapY;
} M11_GroupMoveRemovalPlan;

int  m11_apply_teleporter_rotation(int thingKind,
                                   int sourceMapX,
                                   const M11_TeleporterDef* teleporter,
                                   int inDirection,
                                   int inCell,
                                   int* outDirection,
                                   int* outCell);
int  m11_apply_group_teleporter_rotation(const M11_TeleporterDef* teleporter,
                                   int creatureCountMinusOne,
                                   int creatureSize,
                                   unsigned int inDirections,
                                   unsigned int inCells,
                                   unsigned int* outDirections,
                                   unsigned int* outCells);
const char* m11_teleporter_rotation_source_evidence(void);
int  m11_plan_group_move_removal_after_pit_teleporter(
        int fallKilledGroup,
        int creatureAllowedOnDestinationMap,
        int sourceMapX,
        int sourceMapY,
        int destinationMapX,
        int destinationMapY,
        M11_GroupMoveRemovalPlan* outPlan);
const char* m11_group_move_removal_source_evidence(void);

#ifdef __cplusplus
}
#endif
#endif
