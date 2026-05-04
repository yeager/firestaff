#ifndef FIRESTAFF_DM1_V2_MOVEMENT_ENGINE_PC34_H
#define FIRESTAFF_DM1_V2_MOVEMENT_ENGINE_PC34_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V2_SUBPIXEL_SCALE 256  /* 8.8 fixed-point */

typedef struct {
    int16_t xPixel;
    int16_t yPixel;
    int16_t xSub;
    int16_t ySub;
    int16_t facingDir;
    int16_t moveState;
    int32_t lastMoveMs;
    int16_t prevX;
    int16_t prevY;
} DM1_V2_PlayerPos;

typedef struct {
    int32_t moveSpeed;
    int32_t turnSpeed;
    int32_t stepThreshold;
    int32_t subPixelAccel;
    int32_t collisionPredictFrames;
} DM1_V2_MoveParams;

void dm1_v2_pos_init(DM1_V2_PlayerPos* p, int x, int y, int dir);
int16_t dm1_v2_get_x(const DM1_V2_PlayerPos* p);
int16_t dm1_v2_get_y(const DM1_V2_PlayerPos* p);
void dm1_v2_set_subpixel(DM1_V2_PlayerPos* p, int16_t xsub, int16_t ysub);
void dm1_v2_move_step(DM1_V2_PlayerPos* p, const DM1_V2_MoveParams* params, int dir, int32_t dtMs);
void dm1_v2_turn(DM1_V2_PlayerPos* p, int turnDir);
int dm1_v2_has_moved(const DM1_V2_PlayerPos* p);
int dm1_v2_snap_to_grid(DM1_V2_PlayerPos* p);
int dm1_v2_collides_at(int px, int py, const int8_t* map, int mapW, int mapH);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_MOVEMENT_ENGINE_PC34_H */