#include "dm1_v1_stairs_exit_direction_f0155_pc34_compat.h"

static int dm1_v1_f0155_square_type(uint8_t rawSquare)
{
    return (rawSquare >> 5) & 0x07;
}

static int dm1_v1_f0155_square_type_at(
    const uint8_t *columnMajorSquares,
    int width,
    int height,
    int mapX,
    int mapY)
{
    if (!columnMajorSquares || width <= 0 || height <= 0 ||
        mapX < 0 || mapX >= width || mapY < 0 || mapY >= height) {
        return DM1_V1_F0155_ELEMENT_WALL_PC34;
    }
    return dm1_v1_f0155_square_type(
        columnMajorSquares[(mapX * height) + mapY]);
}

const char *DM1_V1_F0155_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:1562 F0155_DUNGEON_GetStairsExitDirection takes "
        "P0274_i_MapX and P0275_i_MapY; DUNGEON.C:1568 reads the current "
        "square type; DUNGEON.C:1570 tests MASK0x0008_STAIRS_NS_ORIENTATION; "
        "DUNGEON.C:1572-1582 checks the north/east neighbor and returns "
        "NORTH/SOUTH or EAST/WEST depending on whether that neighbor is "
        "blocked by wall or stairs.";
}

int DM1_V1_Dungeon_GetStairsExitDirectionF0155Pc34Compat(
    const uint8_t *columnMajorSquares,
    int width,
    int height,
    int mapX,
    int mapY,
    DM1_V1_StairsExitDirectionF0155Pc34 *out)
{
    static const int stepEast[2] = { 0, 1 };
    static const int stepNorth[2] = { -1, 0 };
    int rawSquare;
    int squareType;
    int northSouthOriented;
    int checkedX;
    int checkedY;
    int checkedType;
    int blocked;

    if (!out) {
        return 0;
    }
    out->valid = 0;
    out->exitDirection = -1;
    out->checkedMapX = mapX;
    out->checkedMapY = mapY;
    out->checkedSquareType = DM1_V1_F0155_ELEMENT_WALL_PC34;
    out->checkedSquareBlocked = 1;
    out->northSouthOrientedStairs = 0;

    if (!columnMajorSquares || width <= 0 || height <= 0 ||
        mapX < 0 || mapX >= width || mapY < 0 || mapY >= height) {
        return 0;
    }

    rawSquare = columnMajorSquares[(mapX * height) + mapY];
    squareType = dm1_v1_f0155_square_type((uint8_t)rawSquare);
    if (squareType != DM1_V1_F0155_ELEMENT_STAIRS_PC34) {
        return 0;
    }

    northSouthOriented =
        (rawSquare & DM1_V1_F0155_STAIRS_NS_ORIENTATION_MASK_PC34) ? 1 : 0;
    checkedX = mapX + stepEast[northSouthOriented ? 0 : 1];
    checkedY = mapY + stepNorth[northSouthOriented ? 0 : 1];
    checkedType = dm1_v1_f0155_square_type_at(
        columnMajorSquares, width, height, checkedX, checkedY);
    blocked = (checkedType == DM1_V1_F0155_ELEMENT_WALL_PC34 ||
               checkedType == DM1_V1_F0155_ELEMENT_STAIRS_PC34);

    out->valid = 1;
    out->exitDirection = (blocked << 1) + (northSouthOriented ? 0 : 1);
    out->checkedMapX = checkedX;
    out->checkedMapY = checkedY;
    out->checkedSquareType = checkedType;
    out->checkedSquareBlocked = blocked;
    out->northSouthOrientedStairs = northSouthOriented;
    return 1;
}

int F0155_DUNGEON_GetStairsExitDirection(
    const uint8_t *columnMajorSquares,
    int width,
    int height,
    int mapX,
    int mapY)
{
    DM1_V1_StairsExitDirectionF0155Pc34 result;

    if (!DM1_V1_Dungeon_GetStairsExitDirectionF0155Pc34Compat(
            columnMajorSquares, width, height, mapX, mapY, &result)) {
        return -1;
    }
    return result.exitDirection;
}
