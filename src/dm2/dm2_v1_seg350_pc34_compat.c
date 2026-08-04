/*
 * dm2_v1_seg350_pc34_compat.c — DM2 segment 350 init functions.
 *
 * Source: skproject/SKWINSPX/src/v4/c_350.cpp
 */

#include "dm2_v1_seg350_pc34_compat.h"
#include <string.h>

void dm2_v1_seg350_entry_init(DM2_V1_Seg350Entry *e)
{
    e->b_00 = 0;
    e->b_01 = 0;
    e->b_02 = 0;
    e->b_03 = 0;
    e->w_04 = 0;
    e->w_06 = 0;
    e->w_08 = 0;
    e->xp_0a = NULL;
}

void dm2_v1_seg350_init(DM2_V1_Seg350 *s)
{
    memset(s, 0, sizeof(*s));
    /* All numeric fields are zeroed by memset.
     * All pointers become NULL (guaranteed on all target platforms).
     * Nested entry is also zeroed. */
}
