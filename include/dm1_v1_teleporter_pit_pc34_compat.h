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
 *     DM1_V1_ApplyTeleporterRotationF0267Pc34Compat        — MOVESENS.C F0262/F0263/F0267
 *                                            party/projectile/object/group
 *                                            rotation kernel
 *     DM1_V1_ApplyGroupTeleporterRotationF0262Pc34Compat  — MOVESENS.C F0262 group inner
 *     DM1_V1_PlanGroupMoveRemovalAfterPitTeleporterF0267Pc34Compat
 *                                          — MOVESENS.C F0267 + GROUP.C
 *                                            F0187/F0188/F0189 group
 *                                            removal/drop plan
 *     DM1_V1_TeleporterRotation_SourceEvidencePc34Compat
 *     DM1_V1_GroupMoveRemoval_SourceEvidencePc34Compat
 */

#define DM1_V1_MAPX_PROJECTILE_ASSOCIATED_OBJECT_PC34 (-2)

#define DM1_V1_TELEPORTER_ROTATE_THING_PARTY_PC34      0
#define DM1_V1_TELEPORTER_ROTATE_THING_PROJECTILE_PC34 1
#define DM1_V1_TELEPORTER_ROTATE_THING_OBJECT_PC34     2
#define DM1_V1_TELEPORTER_ROTATE_THING_GROUP_PC34      3

#define DM1_V1_GROUP_MOVE_REMOVAL_REASON_NONE_PC34          0
#define DM1_V1_GROUP_CELL_SINGLE_CENTERED_PC34              0xFF
#define DM1_V1_CREATURE_SIZE_QUARTER_SQUARE_PC34            0
#define DM1_V1_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED_PC34   1
#define DM1_V1_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED_PC34   2
#define DM1_V1_GROUP_MOVE_REMOVAL_SOUND_ONE_TICK_LATER_PC34 2

#define DM1_V1_GROUP_MOVE_ROUTE_INSERT_PC34                 1
#define DM1_V1_GROUP_MOVE_ROUTE_REMOVE_PC34                 2
#define DM1_V1_GROUP_MOVE_ROUTE_RETRY_PC34                  3
#define DM1_V1_GROUP_MOVE_ROUTE_CHAOS_ADJACENT_INSERT_PC34  4
#define DM1_V1_GROUP_MOVE_ROUTE_KILLED_BY_PROJECTILE_PC34   5

#define DM1_V1_TELEPORTER_SCOPE_CREATURES_PC34              0x01

#define DM1_V1_DIRECTION_NORTH_PC34                         0
#define DM1_V1_DIRECTION_EAST_PC34                          1
#define DM1_V1_DIRECTION_SOUTH_PC34                         2
#define DM1_V1_DIRECTION_WEST_PC34                          3

#define DM1_V1_AI_STATE_WANDER_PC34                         1

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
} DM1_V1_TeleporterDefPc34;

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
} DM1_V1_GroupMoveRemovalPlanPc34;

typedef struct {
    int valid;
    int route;
    int shouldEmitAudibleBuzz;
    int shouldScheduleRetry;
    uint32_t retryFireAtTick;
    int mapX;
    int mapY;
    int removalReason;
} DM1_V1_GroupMoveRoutePlanPc34;

typedef struct {
    int valid;
    int destinationMapX;
    int destinationMapY;
    int route;
    uint32_t retryFireAtTick;
} DM1_V1_OrdinaryGroupMovePlanPc34;

typedef struct {
    int valid;
    int shouldUnlinkSource;
    int shouldLinkDestination;
    int shouldRemoveActiveGroup;
    int shouldRequeue;
    int groupDirection;
    int sourceMapIndex;
    int sourceMapX;
    int sourceMapY;
    int activeMapIndex;
    int activeMapX;
    int activeMapY;
    int activeCells;
    uint32_t nextFireAtTick;
    int nextEventMapX;
    int nextEventMapY;
} DM1_V1_OrdinaryGroupMoveApplyPlanPc34;

typedef struct {
    int valid;
    int shouldFall;
} DM1_V1_GroupPitFallSquarePlanPc34;

typedef struct {
    int valid;
    int shouldTeleport;
    int shouldStopChain;
    int shouldEmitAudibleBuzz;
    int targetMapIndex;
    int targetMapX;
    int targetMapY;
} DM1_V1_GroupTeleporterDestinationPlanPc34;

typedef struct {
    int valid;
    int shouldCreateActiveState;
    int activeStateKind;
    int activeCreatureType;
    int activeMapIndex;
    int activeMapX;
    int activeMapY;
    int activeCells;
    int activeDirection;
    int activeTargetChampionIndex;
    int activeLastSeenPartyMapX;
    int activeLastSeenPartyMapY;
    int activeLastSeenPartyTick;
    int activeReservedGroupIndex;
    int shouldScheduleWanderEvent;
    uint32_t wanderFireAtTick;
    int wanderMapIndex;
    int wanderMapX;
    int wanderMapY;
    int wanderGroupIndex;
    int wanderCreatureType;
    int wanderEventType;
} DM1_V1_GeneratedGroupPlacementPlanPc34;

typedef DM1_V1_OrdinaryGroupMoveApplyPlanPc34 M11_OrdinaryGroupMoveApplyPlan;
typedef DM1_V1_GroupTeleporterDestinationPlanPc34 M11_GroupTeleporterDestinationPlan;
typedef DM1_V1_GeneratedGroupPlacementPlanPc34 M11_GeneratedGroupPlacementPlan;
typedef DM1_V1_TeleporterDefPc34 M11_TeleporterDef;
typedef DM1_V1_GroupMoveRemovalPlanPc34 M11_GroupMoveRemovalPlan;
typedef DM1_V1_GroupMoveRoutePlanPc34 M11_GroupMoveRoutePlan;
typedef DM1_V1_OrdinaryGroupMovePlanPc34 M11_OrdinaryGroupMovePlan;
typedef DM1_V1_GroupPitFallSquarePlanPc34 M11_GroupPitFallSquarePlan;

typedef struct {
    int valid;
    int shouldAttempt;
    int shouldInsertAdjacent;
    int candidateMapX;
    int candidateMapY;
} DM1_V1_LordChaosAdjacentRetryPlanPc34;

typedef DM1_V1_LordChaosAdjacentRetryPlanPc34 M11_LordChaosAdjacentRetryPlan;

int  DM1_V1_ApplyTeleporterRotationF0267Pc34Compat(int thingKind,
                                   int sourceMapX,
                                   const DM1_V1_TeleporterDefPc34* teleporter,
                                   int inDirection,
                                   int inCell,
                                   int* outDirection,
                                   int* outCell);
int  DM1_V1_ApplyGroupTeleporterRotationF0262Pc34Compat(const DM1_V1_TeleporterDefPc34* teleporter,
                                   int creatureCountMinusOne,
                                   int creatureSize,
                                   unsigned int inDirections,
                                   unsigned int inCells,
                                   unsigned int* outDirections,
                                   unsigned int* outCells);
const char* DM1_V1_TeleporterRotation_SourceEvidencePc34Compat(void);
int  DM1_V1_PlanGroupMoveRemovalAfterPitTeleporterF0267Pc34Compat(
        int fallKilledGroup,
        int creatureAllowedOnDestinationMap,
        int sourceMapX,
        int sourceMapY,
        int destinationMapX,
        int destinationMapY,
        DM1_V1_GroupMoveRemovalPlanPc34* outPlan);
int  DM1_V1_PlanDeferredGroupMoveRouteF0267Pc34Compat(
        int fallKilledGroup,
        int creatureAllowedOnDestinationMap,
        int destinationBlocked,
        int audibleEvent,
        int chaosAdjacentAvailable,
        uint32_t currentFireAtTick,
        int destinationMapX,
        int destinationMapY,
        int chaosAdjacentMapX,
        int chaosAdjacentMapY,
        DM1_V1_GroupMoveRoutePlanPc34* outPlan);
int  DM1_V1_PlanOrdinaryGroupMoveF0267Pc34Compat(
        int sourceMapX,
        int sourceMapY,
        int direction,
        int destinationPassable,
        int destinationBlocked,
        int killedByProjectile,
        uint32_t currentTick,
        DM1_V1_OrdinaryGroupMovePlanPc34* outPlan);
int  DM1_V1_PlanOrdinaryGroupMoveApplyF0267Pc34Compat(
        const DM1_V1_OrdinaryGroupMovePlanPc34* movePlan,
        int sourceMapIndex,
        int sourceMapX,
        int sourceMapY,
        int direction,
        int groupCells,
        uint32_t currentTick,
        DM1_V1_OrdinaryGroupMoveApplyPlanPc34* outPlan);
int  DM1_V1_PlanGroupPitFallSquareF0267Pc34Compat(
        int squareType,
        int pitSquareType,
        int pitOpen,
        int pitImaginary,
        DM1_V1_GroupPitFallSquarePlanPc34* outPlan);
int  DM1_V1_PlanGroupTeleporterDestinationF0267Pc34Compat(
        int squareType,
        int teleporterSquareType,
        int teleporterOpen,
        int teleporterFound,
        int teleporterScope,
        int teleporterAudible,
        int targetMapIndex,
        int targetMapX,
        int targetMapY,
        int sourceMapIndex,
        int sourceMapX,
        int sourceMapY,
        int mapCount,
        DM1_V1_GroupTeleporterDestinationPlanPc34* outPlan);
int  DM1_V1_PlanGeneratedGroupPlacementF0183F0180Pc34Compat(
        int partyMapIndex,
        int mapIndex,
        int mapX,
        int mapY,
        int groupIndex,
        int creatureType,
        int groupCells,
        int groupDirection,
        int activeGroupCount,
        int activeGroupCapacity,
        uint32_t currentTick,
        DM1_V1_GeneratedGroupPlacementPlanPc34* outPlan);
int  DM1_V1_PlanLordChaosAdjacentRetryF0252Pc34Compat(
        int creatureType,
        int randomGate,
        int randomDirection,
        int sourceMapX,
        int sourceMapY,
        int candidateAllowedSquare,
        int candidateBlocked,
        DM1_V1_LordChaosAdjacentRetryPlanPc34* outPlan);
const char* DM1_V1_GroupMoveRemoval_SourceEvidencePc34Compat(void);

#define M11_MAPX_PROJECTILE_ASSOCIATED_OBJECT DM1_V1_MAPX_PROJECTILE_ASSOCIATED_OBJECT_PC34
#define M11_TELEPORTER_ROTATE_THING_PARTY DM1_V1_TELEPORTER_ROTATE_THING_PARTY_PC34
#define M11_TELEPORTER_ROTATE_THING_PROJECTILE DM1_V1_TELEPORTER_ROTATE_THING_PROJECTILE_PC34
#define M11_TELEPORTER_ROTATE_THING_OBJECT DM1_V1_TELEPORTER_ROTATE_THING_OBJECT_PC34
#define M11_TELEPORTER_ROTATE_THING_GROUP DM1_V1_TELEPORTER_ROTATE_THING_GROUP_PC34
#define M11_GROUP_MOVE_REMOVAL_REASON_NONE DM1_V1_GROUP_MOVE_REMOVAL_REASON_NONE_PC34
#define M11_GROUP_CELL_SINGLE_CENTERED DM1_V1_GROUP_CELL_SINGLE_CENTERED_PC34
#define M11_CREATURE_SIZE_QUARTER_SQUARE DM1_V1_CREATURE_SIZE_QUARTER_SQUARE_PC34
#define M11_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED DM1_V1_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED_PC34
#define M11_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED DM1_V1_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED_PC34
#define M11_GROUP_MOVE_REMOVAL_SOUND_ONE_TICK_LATER DM1_V1_GROUP_MOVE_REMOVAL_SOUND_ONE_TICK_LATER_PC34
#define M11_GROUP_MOVE_ROUTE_INSERT DM1_V1_GROUP_MOVE_ROUTE_INSERT_PC34
#define M11_GROUP_MOVE_ROUTE_REMOVE DM1_V1_GROUP_MOVE_ROUTE_REMOVE_PC34
#define M11_GROUP_MOVE_ROUTE_RETRY DM1_V1_GROUP_MOVE_ROUTE_RETRY_PC34
#define M11_GROUP_MOVE_ROUTE_CHAOS_ADJACENT_INSERT DM1_V1_GROUP_MOVE_ROUTE_CHAOS_ADJACENT_INSERT_PC34
#define M11_GROUP_MOVE_ROUTE_KILLED_BY_PROJECTILE DM1_V1_GROUP_MOVE_ROUTE_KILLED_BY_PROJECTILE_PC34
#define M11_TELEPORTER_SCOPE_CREATURES DM1_V1_TELEPORTER_SCOPE_CREATURES_PC34
#define M11_DIRECTION_NORTH DM1_V1_DIRECTION_NORTH_PC34
#define M11_DIRECTION_EAST DM1_V1_DIRECTION_EAST_PC34
#define M11_DIRECTION_SOUTH DM1_V1_DIRECTION_SOUTH_PC34
#define M11_DIRECTION_WEST DM1_V1_DIRECTION_WEST_PC34
#define M11_AI_STATE_WANDER DM1_V1_AI_STATE_WANDER_PC34
#define m11_teleporter_rotation_source_evidence \
    DM1_V1_TeleporterRotation_SourceEvidencePc34Compat
#define m11_group_move_removal_source_evidence \
    DM1_V1_GroupMoveRemoval_SourceEvidencePc34Compat
#define m11_apply_teleporter_rotation \
    DM1_V1_ApplyTeleporterRotationF0267Pc34Compat
#define m11_apply_group_teleporter_rotation \
    DM1_V1_ApplyGroupTeleporterRotationF0262Pc34Compat
#define m11_plan_group_move_removal_after_pit_teleporter \
    DM1_V1_PlanGroupMoveRemovalAfterPitTeleporterF0267Pc34Compat
#define m11_plan_deferred_group_move_route_f0267 \
    DM1_V1_PlanDeferredGroupMoveRouteF0267Pc34Compat
#define m11_plan_ordinary_group_move_f0267 \
    DM1_V1_PlanOrdinaryGroupMoveF0267Pc34Compat
#define m11_plan_ordinary_group_move_apply_f0267 \
    DM1_V1_PlanOrdinaryGroupMoveApplyF0267Pc34Compat
#define m11_plan_group_pit_fall_square_f0267 \
    DM1_V1_PlanGroupPitFallSquareF0267Pc34Compat
#define m11_plan_group_teleporter_destination_f0267 \
    DM1_V1_PlanGroupTeleporterDestinationF0267Pc34Compat
#define m11_plan_generated_group_placement_f0183_f0180 \
    DM1_V1_PlanGeneratedGroupPlacementF0183F0180Pc34Compat
#define m11_plan_lord_chaos_adjacent_retry_f0252 \
    DM1_V1_PlanLordChaosAdjacentRetryF0252Pc34Compat

#ifdef __cplusplus
}
#endif
#endif
