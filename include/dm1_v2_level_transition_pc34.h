#ifndef FIRESTAFF_DM1_V2_LEVEL_TRANSITION_PC34_H
#define FIRESTAFF_DM1_V2_LEVEL_TRANSITION_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

typedef enum {
    TRANS_STAIRS_DOWN,
    TRANS_STAIRS_UP,
    TRANS_PIT_FALL,
    TRANS_TELEPORT,
    TRANS_PORTAL
} M11_V2_TransType;

typedef struct {
    M11_V2_TransType type;
    int from_level;
    int to_level;
    float progress;
    float speed;
    bool active;
    int dest_x;
    int dest_y;
    int dest_dir;
} M11_V2_LevelTransition;

void v2_level_trans_init(void);
void v2_level_trans_start(M11_V2_TransType type, int from, int to, int dx, int dy, int ddir, float speed);
bool v2_level_trans_update(float dt);
void v2_level_trans_render_overlay(uint8_t* fb, int w, int h);
bool v2_level_trans_is_active(void);
void v2_level_trans_cancel(void);

#endif
