/*
 * csb_v1_projectile_speed_pc34_compat.c
 *
 * Source-locked per PROJEXPL.C CHANGE7_20_IMPROVEMENT.  Single
 * process-wide flag toggled by csb_v1_save_load on save load.
 */
#include "csb_v1_projectile_speed_pc34_compat.h"

static int g_csb_v1_proj_speed_norm = 0;

int csb_v1_projectile_speed_normalization_get(void) {
    return g_csb_v1_proj_speed_norm;
}

void csb_v1_projectile_speed_normalization_set(int enabled) {
    g_csb_v1_proj_speed_norm = enabled ? 1 : 0;
}
