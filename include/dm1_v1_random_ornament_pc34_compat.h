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

const char *dm1_v1_random_ornament_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_RANDOM_ORNAMENT_PC34_COMPAT_H */
