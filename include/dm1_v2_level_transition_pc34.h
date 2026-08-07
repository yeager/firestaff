#ifndef FIRESTAFF_DM1_V2_LEVEL_TRANSITION_PC34_H
#define FIRESTAFF_DM1_V2_LEVEL_TRANSITION_PC34_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    TRANS_STAIRS_DOWN,
    TRANS_STAIRS_UP,
    TRANS_PIT_FALL,
    TRANS_TELEPORT,
    TRANS_PORTAL
} M11_V2_TransType;

/* Compatibility-only entry points. ReDMCSB owns the discrete map transition
 * and viewport redraw, so this V2 layer retains no transition state and never
 * alters a source framebuffer. */
void v2_level_trans_init(void);
void v2_level_trans_start(M11_V2_TransType type, int from, int to, int dx,
                          int dy, int ddir, float speed);
bool v2_level_trans_update(float dt);
void v2_level_trans_render_overlay(uint8_t* fb, int w, int h);
bool v2_level_trans_is_active(void);
void v2_level_trans_cancel(void);

int v2_level_transition_tick(float dt);
float v2_level_transition_get_progress(void);
int v2_level_transition_is_active(void);
float v22_transition_duration_for_type(int type);

#endif
