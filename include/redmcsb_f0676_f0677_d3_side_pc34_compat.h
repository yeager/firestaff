#ifndef FIRESTAFF_REDMCSB_F0676_F0677_D3_SIDE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0676_F0677_D3_SIDE_PC34_COMPAT_H

#include <stdint.h>

enum { REDMCSB_F0676_D3L2 = 0, REDMCSB_F0677_D3R2 = 1,
       REDMCSB_F0676_ELEMENT_WALL = 0, REDMCSB_F0676_ELEMENT_CORRIDOR = 1,
       REDMCSB_F0676_ELEMENT_PIT = 2, REDMCSB_F0676_ELEMENT_TELEPORTER = 5,
       REDMCSB_F0676_ELEMENT_DOOR_SIDE = 16, REDMCSB_F0676_ELEMENT_DOOR_FRONT = 17,
       REDMCSB_F0676_ELEMENT_STAIRS_SIDE = 18, REDMCSB_F0676_ELEMENT_STAIRS_FRONT = 19 };
typedef struct redmcsb_f0676_square_pc34_compat { int16_t element, stairs_up, pit_or_teleporter_visible, wall_ornament, floor_ornament, first_thing, door_thing, door_state; } redmcsb_f0676_square_pc34_compat;
typedef struct redmcsb_f0676_runtime_pc34_compat {
    void (*stairs)(void *, int side, int up); void (*wall)(void *, int side, int flip);
    void (*wall_ornament)(void *, int side, int ordinal); void (*pit)(void *, int side, int flip);
    void (*floor_ornament)(void *, int side, int ordinal);
    void (*things)(void *, int side, int first_thing, int direction, int x, int y, int order);
    void (*door)(void *, int side, int thing, int state); void (*field)(void *, int side); void *context;
} redmcsb_f0676_runtime_pc34_compat;
int redmcsb_f0676_f0677_draw_d3_side_pc34_compat(int side, const redmcsb_f0676_square_pc34_compat *square, int16_t direction, int16_t x, int16_t y, int use_flipped_wall, const redmcsb_f0676_runtime_pc34_compat *runtime);
const char *redmcsb_f0676_f0677_d3_side_source_evidence_pc34(void);
#endif
