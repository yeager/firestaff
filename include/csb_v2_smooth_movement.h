
#ifndef FIRESTAFF_CSB_V2_SMOOTH_MOVEMENT_H
#define FIRESTAFF_CSB_V2_SMOOTH_MOVEMENT_H
#include "dm1_v2_anim_timing.h"

/* CSB V2.2 Smooth Movement — same system as DM1 V2.2.
 * CSB shares the DM1 movement engine; V2.2 interpolation
 * works identically. Separate header for CSB-specific
 * animation parameters (CSB has different movement cooldowns).
 *
 * Presentation-only: V1 cooldowns, collision, and sensor timing are
 * unaffected. The renderer interpolates visually between V1 states.
 *
 * ReDMCSB source: COMMAND.C F0380, CLIKMENU.C F0365/F0366,
 * GAMELOOP.C (tick cadence, VBLANK-locked 55ms). */

#define CSB_V2_WALK_EASE   V2_EASE_OUT_CUBIC
#define CSB_V2_TURN_EASE   V2_EASE_OUT_QUAD
#define CSB_V2_STAIRS_EASE V2_EASE_IN_OUT_CUBIC

void csb_v2_smooth_init(void);
void csb_v2_smooth_start_walk(float fx, float fy, float tx, float ty);
void csb_v2_smooth_start_turn(float from, float to);
void csb_v2_smooth_update_from_clock(const V2_AnimClock *clock);
float csb_v2_smooth_get_x(void);
float csb_v2_smooth_get_y(void);
float csb_v2_smooth_get_angle(void);
int   csb_v2_smooth_is_moving(void);
const char *csb_v2_smooth_source_evidence(void);
#endif

