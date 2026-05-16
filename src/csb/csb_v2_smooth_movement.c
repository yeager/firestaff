
#include "csb_v2_smooth_movement.h"

static V2_Anim g_csb_walk_x, g_csb_walk_y, g_csb_turn;

void csb_v2_smooth_start_walk(float fx, float fy, float tx, float ty) {
    v2_anim_start_v1_tick(&g_csb_walk_x, fx, tx, CSB_V2_WALK_EASE);
    v2_anim_start_v1_tick(&g_csb_walk_y, fy, ty, CSB_V2_WALK_EASE);
}

void csb_v2_smooth_start_turn(float from, float to) {
    v2_anim_start_v1_tick(&g_csb_turn, from, to, CSB_V2_TURN_EASE);
}

const char *csb_v2_smooth_source_evidence(void) {
    return "CSB V2.2: shared DM1 V2.2 smooth movement, CSB-specific easing\n"
           "V1 tick rate preserved (55ms), visual interpolation only\n";
}

