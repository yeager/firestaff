#include "dm1_v2_level_transition_pc34.h"

/* PC34 changes map/party state and draws the resulting source viewport
 * directly. It has no independent V2 layer for stairs, pits, teleports, or
 * portals, so these retained compatibility entries never schedule or draw an
 * invented transition. */

void v2_level_trans_init(void) {
}

void v2_level_trans_start(M11_V2_TransType type, int from, int to,
                          int dx, int dy, int ddir, float speed) {
    (void)type;
    (void)from;
    (void)to;
    (void)dx;
    (void)dy;
    (void)ddir;
    (void)speed;
}

bool v2_level_trans_update(float dt) {
    (void)dt;
    return false;
}

bool v2_level_trans_is_active(void) {
    return false;
}

void v2_level_trans_cancel(void) {
}

void v2_level_trans_render_overlay(uint8_t* fb, int w, int h) {
    (void)fb;
    (void)w;
    (void)h;
}

typedef enum {
    V2_TRANS_NONE,
    V2_TRANS_DESCEND,
    V2_TRANS_ASCEND,
    V2_TRANS_PIT_FALL,
    V2_TRANS_TELEPORT,
} V2_TransitionType;

void v2_level_transition_start(V2_TransitionType type, int from, int to,
                               float speed) {
    (void)type;
    (void)from;
    (void)to;
    (void)speed;
}

int v2_level_transition_tick(float dt) {
    (void)dt;
    return 0;
}

float v2_level_transition_get_progress(void) {
    return 0.0f;
}

V2_TransitionType v2_level_transition_get_type(void) {
    return V2_TRANS_NONE;
}

int v2_level_transition_is_active(void) {
    return 0;
}

float v22_transition_duration_for_type(int type) {
    (void)type;
    return 0.0f;
}
