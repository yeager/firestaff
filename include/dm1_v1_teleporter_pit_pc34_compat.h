#ifndef FIRESTAFF_DM1_V1_TELEPORTER_PIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_TELEPORTER_PIT_PC34_COMPAT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Based on ReDMCSB MOVESENS.C F0276 sensor processing, pit/teleporter chains.
 * F0267_MOVE_GetMoveResult handles pit fall, F0276 triggers teleporter. */

#define M11_MAPX_PROJECTILE_ASSOCIATED_OBJECT (-2)

#define M11_TELEPORTER_ROTATE_THING_PARTY      0
#define M11_TELEPORTER_ROTATE_THING_PROJECTILE 1
#define M11_TELEPORTER_ROTATE_THING_OBJECT     2
#define M11_TELEPORTER_ROTATE_THING_GROUP      3

#define M11_GROUP_MOVE_REMOVAL_REASON_NONE        0
#define M11_GROUP_CELL_SINGLE_CENTERED             0xFF
#define M11_CREATURE_SIZE_QUARTER_SQUARE           0
#define M11_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED 1
#define M11_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED 2
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
typedef struct { int x; int y; int isOpen; int damageOnFall; int destLevel; int destX; int destY; int level; } M11_PitDef;
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
#define M11_MAX_TELEPORTERS 64
#define M11_MAX_PITS 64
typedef struct {
    M11_TeleporterDef teleporters[M11_MAX_TELEPORTERS]; int teleporterCount;
    M11_PitDef pits[M11_MAX_PITS]; int pitCount;
    int chainDepthLimit;
} M11_TeleporterPitState;
void m11_teleporter_pit_init(M11_TeleporterPitState* s);
int  m11_add_teleporter(M11_TeleporterPitState* s, int x, int y, int destX, int destY, int destLevel, int destFacing, int visible);
int  m11_add_pit(M11_TeleporterPitState* s, int x, int y, int open, int damage, int sourceLevel, int destLevel, int destX, int destY);
int  m11_check_teleporter(const M11_TeleporterPitState* s, int x, int y, M11_TeleporterDef* out);
int  m11_check_pit(const M11_TeleporterPitState* s, int x, int y, int currentLevel, M11_PitDef* out);
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
int  m11_resolve_pit_chain(const M11_TeleporterPitState* s, int startX, int startY, int startLevel, int levitating, int* finalX, int* finalY, int* finalLevel, int* totalDamage);
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
