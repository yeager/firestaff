#include "dm1_v1_random_ornament_pc34_compat.h"

#include <limits.h>

int dm1_v1_dungeon_is_wall_ornament_an_alcove_pc34(
    int ornamentIndex,
    const int *alcoveOrnamentIndices,
    int alcoveOrnamentCount)
{
    int i;

    /* DUNGEON.C F0149:1330-1348 has no default alcove table. */
    if (ornamentIndex < 0 || !alcoveOrnamentIndices || alcoveOrnamentCount <= 0) {
        return 0;
    }
    for (i = 0; i < alcoveOrnamentCount; ++i) {
        if (alcoveOrnamentIndices[i] == ornamentIndex) {
            return 1;
        }
    }
    return 0;
}

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
    int alcoveOrnamentCount)
{
    int sourceY;
    int normalizedDirection;
    int slot;

    if (!aspect || direction < 0 || direction > 3 ||
        randomWallOrnamentCount < 0 || alcoveOrnamentCount < 0 ||
        (alcoveOrnamentCount > 0 && !alcoveOrnamentIndices)) {
        return 0;
    }

    /* DUNGEON.C F0171:2438-2442.  PC3.4 first increments Y for the back
     * wall, then increments direction between right, front, and left. */
    sourceY = mapY + 1;
    normalizedDirection = direction;
    aspect[DM1_V1_SQUARE_ASPECT_BACK_WALL_ORNAMENT_PC34] =
        dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
            backRandomWallOrnamentAllowed, randomWallOrnamentCount,
            mapX, sourceY * (normalizedDirection + 1), mapIndex,
            mapWidth, mapHeight, ornamentRandomSeed, 30);

    normalizedDirection = (normalizedDirection + 1) & 3;
    aspect[DM1_V1_SQUARE_ASPECT_RIGHT_WALL_ORNAMENT_PC34] =
        dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
            leftRandomWallOrnamentAllowed, randomWallOrnamentCount,
            mapX, sourceY * (normalizedDirection + 1), mapIndex,
            mapWidth, mapHeight, ornamentRandomSeed, 30);

    normalizedDirection = (normalizedDirection + 1) & 3;
    aspect[DM1_V1_SQUARE_ASPECT_FRONT_WALL_ORNAMENT_PC34] =
        dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
            frontRandomWallOrnamentAllowed, randomWallOrnamentCount,
            mapX, sourceY * (normalizedDirection + 1), mapIndex,
            mapWidth, mapHeight, ornamentRandomSeed, 30);

    normalizedDirection = (normalizedDirection + 1) & 3;
    aspect[DM1_V1_SQUARE_ASPECT_LEFT_WALL_ORNAMENT_PC34] =
        dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
            rightRandomWallOrnamentAllowed, randomWallOrnamentCount,
            mapX, sourceY * (normalizedDirection + 1), mapIndex,
            mapWidth, mapHeight, ornamentRandomSeed, 30);

    /* F0171:2450-2462 only suppresses random alcoves when the original wall
     * lies outside the current map.  The caller supplies F0174's decoded map
     * alcove list; this routine never substitutes a global ornament list. */
    if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight) {
        for (slot = DM1_V1_SQUARE_ASPECT_BACK_WALL_ORNAMENT_PC34;
             slot <= DM1_V1_SQUARE_ASPECT_LEFT_WALL_ORNAMENT_PC34;
             ++slot) {
            if (dm1_v1_dungeon_is_wall_ornament_an_alcove_pc34(
                    aspect[slot] - 1,
                    alcoveOrnamentIndices, alcoveOrnamentCount)) {
                aspect[slot] = 0;
            }
        }
    }
    return 1;
}

const char *dm1_v1_random_ornament_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C F0169:2371-2379, F0170:2382-2404, "
           "F0171:2407-2462, and F0149:1330-1348";
}
