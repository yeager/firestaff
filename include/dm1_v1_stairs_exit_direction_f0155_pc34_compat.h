#ifndef DM1_V1_STAIRS_EXIT_DIRECTION_F0155_PC34_COMPAT_H
#define DM1_V1_STAIRS_EXIT_DIRECTION_F0155_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0155_DIR_NORTH_PC34 = 0,
    DM1_V1_F0155_DIR_EAST_PC34 = 1,
    DM1_V1_F0155_DIR_SOUTH_PC34 = 2,
    DM1_V1_F0155_DIR_WEST_PC34 = 3
};

enum {
    DM1_V1_F0155_ELEMENT_WALL_PC34 = 0,
    DM1_V1_F0155_ELEMENT_STAIRS_PC34 = 3,
    DM1_V1_F0155_STAIRS_NS_ORIENTATION_MASK_PC34 = 0x08
};

typedef struct {
    int valid;
    int exitDirection;
    int checkedMapX;
    int checkedMapY;
    int checkedSquareType;
    int checkedSquareBlocked;
    int northSouthOrientedStairs;
} DM1_V1_StairsExitDirectionF0155Pc34;

const char *DM1_V1_F0155_SourceEvidencePc34(void);

int F0155_DUNGEON_GetStairsExitDirection(
    const uint8_t *columnMajorSquares,
    int width,
    int height,
    int mapX,
    int mapY);

int DM1_V1_Dungeon_GetStairsExitDirectionF0155Pc34Compat(
    const uint8_t *columnMajorSquares,
    int width,
    int height,
    int mapX,
    int mapY,
    DM1_V1_StairsExitDirectionF0155Pc34 *out);

#ifdef __cplusplus
}
#endif

#endif
