/*
 * csb_v1_neophyte_mode_pc34_compat.c
 *
 * Source-locked per ReDMCSB Character.cpp:665 (skill validation),
 * data.cpp:88 (default neophyteSkills = false), Recording.cpp:246
 * (replay neophyte mode set true), and PANEL.C:26 / CEDT006.C:141
 * (NEOPHYTE rank at index 0 of the rank-name table).
 */
#include "csb_v1_neophyte_mode_pc34_compat.h"

static int g_csb_v1_neophyte_enabled = 0;

int csb_v1_neophyte_skills_mode_get(void) {
    return g_csb_v1_neophyte_enabled;
}

void csb_v1_neophyte_skills_mode_set(int enabled) {
    g_csb_v1_neophyte_enabled = enabled ? 1 : 0;
}

int csb_v1_neophyte_display_for_level(unsigned int level) {
    /* When neophyte mode is enabled, level 0 is displayed as
     * NEOPHYTE.  When disabled, level 0 falls back to NOVICE
     * (DM1 behaviour).  Levels 1+ are always NOVICE+. */
    if (level == 0U && g_csb_v1_neophyte_enabled) {
        return 1;
    }
    return 0;
}
