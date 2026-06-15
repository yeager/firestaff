/*
 * csb_v1_dungeon_header_pc34_compat.c
 *
 * Source-locked per ReDMCSB CEDTINC8.C:101-118 (CSBGAME.DAT vs
 * DMSAVE.DAT dispatch) and M13_PLAN.md:303 (header layouts).
 * DM1 PC 3.4: NumLevel() = 14.
 * CSB:        NumLevel() = 24.
 */
#include "csb_v1_dungeon_header_pc34_compat.h"

int csb_v1_dungeon_header_num_level(int variant) {
    switch (variant) {
    case CSB_V1_DUNGEON_VARIANT_DM1_PC34: return 14;
    case CSB_V1_DUNGEON_VARIANT_CSB:       return 24;
    default:                                return 14; /* default DM1 */
    }
}

int csb_v1_dungeon_header_is_csb(int variant) {
    return variant == CSB_V1_DUNGEON_VARIANT_CSB;
}
