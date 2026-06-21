/*
 * csb_v1_grey_lord_combat_pc34_compat.c
 *
 * Source-locked per ReDMCSB DEFS.H:1679, Attack.cpp:2423,
 * and the BUG0_69 fix that widens IsLordChaosHere() to
 * include Grey Lord proximity.
 */
#include "csb_v1_grey_lord_combat_pc34_compat.h"

static int g_csb_v1_grey_lord_aware = 1;

int csb_v1_grey_lord_aware_get(void) {
    return g_csb_v1_grey_lord_aware;
}

void csb_v1_grey_lord_aware_set(int enabled) {
    g_csb_v1_grey_lord_aware = enabled ? 1 : 0;
}

int csb_v1_is_lord_chaos_or_grey_lord_here(int cellMask,
                                           int hasLordChaosCell,
                                           int hasGreyLordCell) {
    (void)cellMask;
    /* Original ReDMCSB IsLordChaosHere(): */
    /*   1. cellMask has Lord-Chaos cell bit, OR
     *   2. (in CSB) cellMask has Grey-Lord cell bit AND
     *      csb_v1_grey_lord_aware is on. */
    if (hasLordChaosCell) return 1;
    if (g_csb_v1_grey_lord_aware && hasGreyLordCell) return 1;
    return 0;
}
