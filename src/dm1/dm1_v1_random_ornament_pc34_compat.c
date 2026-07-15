#include "dm1_v1_random_ornament_pc34_compat.h"

#include <limits.h>

int dm1_v1_dungeon_get_random_ornament_index_pc34(
    uint16_t value1,
    uint16_t value2,
    uint16_t ornamentRandomSeed,
    int modulo)
{
    uint32_t mixed;

    /* DUNGEON.C F0169:2379.  The source call sites always pass modulo 30;
     * reject an invalid adapter call instead of inventing a substitute. */
    if (modulo <= 0) {
        return 0;
    }
    mixed = (((uint32_t)value1 * 31417u) >> 1) +
            ((uint32_t)value2 * 11u) + ornamentRandomSeed;
    return (int)((mixed >> 2) % (uint32_t)modulo);
}

int dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
    int randomOrnamentAllowed,
    int ornamentCount,
    int mapX,
    int mapY,
    int mapIndex,
    int mapWidth,
    int mapHeight,
    uint16_t ornamentRandomSeed,
    int modulo)
{
    uint16_t value1;
    uint16_t value2;
    int index;

    /* F0170 has no fallback: either the original square flag and map metadata
     * select an ordinal, or it returns C0_ORDINAL_NONE. */
    if (!randomOrnamentAllowed || ornamentCount <= 0 || modulo <= 0 ||
        mapIndex < 0 || mapIndex > SHRT_MAX || mapWidth <= 0 || mapHeight <= 0 ||
        mapX < SHRT_MIN || mapX > SHRT_MAX ||
        mapY < SHRT_MIN || mapY > SHRT_MAX) {
        return 0;
    }

    value1 = (uint16_t)(2000 + (mapX * 32) + mapY);
    value2 = (uint16_t)(3000 + (mapIndex * 64) + mapWidth + mapHeight);
    index = dm1_v1_dungeon_get_random_ornament_index_pc34(
        value1, value2, ornamentRandomSeed, modulo);
    return index < ornamentCount ? index + 1 : 0;
}

const char *dm1_v1_random_ornament_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C F0169:2371-2379 and F0170:2382-2404; "
           "F0172:2666 floor-ornament caller";
}
