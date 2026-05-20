#include "dm1_v1_viewport_fakewall_pc34_compat.h"

#include "memory_dungeon_dat_pc34_compat.h"

int M11_DM1_ViewportEffectiveElementForSquarePc34(unsigned char square)
{
    int elementType = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;

    /* ReDMCSB DUNGEON.C:F0172, lines 2651-2664: an open fakewall is
     * converted to corridor aspect for viewport drawing, while a closed
     * fakewall is converted to wall aspect.  Movement still has its own
     * CLIKMENU.C:F0366 imaginary-fakewall passability rule. */
    if (elementType == DUNGEON_ELEMENT_FAKEWALL) {
        return (square & 0x04) ? DUNGEON_ELEMENT_CORRIDOR : DUNGEON_ELEMENT_WALL;
    }
    return elementType;
}

int M11_DM1_ViewportSquareIsWallLikePc34(unsigned char square)
{
    return M11_DM1_ViewportEffectiveElementForSquarePc34(square) == DUNGEON_ELEMENT_WALL;
}

int M11_DM1_ViewportSquareIsOpenPc34(unsigned char square)
{
    int elementType = M11_DM1_ViewportEffectiveElementForSquarePc34(square);
    if (elementType == DUNGEON_ELEMENT_WALL) {
        return 0;
    }
    if (elementType == DUNGEON_ELEMENT_DOOR) {
        int doorState = square & 0x07;
        return doorState == 0 || doorState == 5;
    }
    return 1;
}

int M11_DM1_ViewportSquareHasFloorOrnamentPathPc34(unsigned char square)
{
    int elementType = M11_DM1_ViewportEffectiveElementForSquarePc34(square);
    return elementType == DUNGEON_ELEMENT_CORRIDOR ||
           elementType == DUNGEON_ELEMENT_PIT ||
           elementType == DUNGEON_ELEMENT_TELEPORTER;
}
