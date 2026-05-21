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
const char* m11_teleporter_rotation_source_evidence(void);
int  m11_resolve_pit_chain(const M11_TeleporterPitState* s, int startX, int startY, int startLevel, int levitating, int* finalX, int* finalY, int* finalLevel, int* totalDamage);

#ifdef __cplusplus
}
#endif
#endif
