#ifndef FIRESTAFF_DM1_V1_RANDOM_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_RANDOM_ORNAMENT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB DUNGEON.C F0169/F0170.  These functions select an ornament from
 * existing map metadata only; an ordinal of zero means that the original
 * random gate selected no ornament.
 */
int dm1_v1_dungeon_get_random_ornament_index_pc34(
    uint16_t value1,
    uint16_t value2,
    uint16_t ornamentRandomSeed,
    int modulo);

int dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
    int randomOrnamentAllowed,
    int ornamentCount,
    int mapX,
    int mapY,
    int mapIndex,
    int mapWidth,
    int mapHeight,
    uint16_t ornamentRandomSeed,
    int modulo);

/* ReDMCSB DUNGEON.C F0149. ornamentIndex and every list entry are zero-based
 * map-global wall-ornament indices. */
int dm1_v1_dungeon_is_wall_ornament_an_alcove_pc34(
    int ornamentIndex,
    const int *alcoveOrnamentIndices,
    int alcoveOrnamentCount);

/* PC3.4 square-aspect slots from DEFS.H MEDIA720. */
enum {
    DM1_V1_SQUARE_ASPECT_BACK_WALL_ORNAMENT_PC34 = 3,
    DM1_V1_SQUARE_ASPECT_RIGHT_WALL_ORNAMENT_PC34 = 4,
    DM1_V1_SQUARE_ASPECT_FRONT_WALL_ORNAMENT_PC34 = 5,
    DM1_V1_SQUARE_ASPECT_LEFT_WALL_ORNAMENT_PC34 = 6,
    DM1_V1_SQUARE_ASPECT_COUNT_PC34 = 7
};

/*
 * ReDMCSB DUNGEON.C F0171 (PC3.4 MEDIA720 body).  aspect must be a seven
 * element source square-aspect array.  alcoveOrnamentIndices are map-owned
 * zero-based indices for F0149; no list is synthesized when source metadata
 * has not been decoded.
 */
int dm1_v1_dungeon_set_random_wall_ornament_ordinals_pc34(
    int *aspect,
    int backRandomWallOrnamentAllowed,
    int leftRandomWallOrnamentAllowed,
    int frontRandomWallOrnamentAllowed,
    int rightRandomWallOrnamentAllowed,
    int randomWallOrnamentCount,
    int direction,
    int mapX,
    int mapY,
    int mapIndex,
    int mapWidth,
    int mapHeight,
    uint16_t ornamentRandomSeed,
    const int *alcoveOrnamentIndices,
    int alcoveOrnamentCount);

const char *dm1_v1_random_ornament_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_RANDOM_ORNAMENT_PC34_COMPAT_H */
