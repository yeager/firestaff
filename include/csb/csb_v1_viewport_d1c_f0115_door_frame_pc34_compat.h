#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1C_F0115_DOOR_FRAME_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1C_F0115_DOOR_FRAME_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_D1C_DOOR_FRAME_PART_TOP = 0,
    CSB_V1_D1C_DOOR_FRAME_PART_LEFT = 1,
    CSB_V1_D1C_DOOR_FRAME_PART_RIGHT = 2
};

typedef struct {
    int contract_only;
    int view_square_d1c;
    int view_depth;
    int view_lane;
    int element_door_front;
    int f0115_rear_order;
    int f0115_front_order;
    int frame_top_zone;
    int frame_left_zone;
    int frame_right_zone;
    int door_zone_d1c;
    int transparent_color;
    int flip_horizontal_mask;
    int top_uses_f0104;
    int left_uses_f0104;
    int right_uses_f0105;
    int right_reuses_left_bitmap;
    int f0115_rear_precedes_frame;
    int frame_precedes_door_bitmap;
    int door_bitmap_precedes_front_f0115;
    int terminal_f0115_uses_l0217_order;
    int uses_f0122_d1l;
    int uses_f0123_d1r;
    const char *frame_top_bitmap_symbol;
    const char *frame_left_bitmap_symbol;
    const char *frame_right_bitmap_symbol;
    const char *redmcsb_f0124_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_f0104_anchor;
    const char *redmcsb_f0105_anchor;
    const char *redmcsb_defs_anchor;
    const char *redmcsb_f0128_anchor;
    const char *source_evidence;
} CSB_V1_ViewportD1CF0115DoorFramePc34Contract;

const CSB_V1_ViewportD1CF0115DoorFramePc34Contract *
csb_v1_viewport_d1c_f0115_door_frame_pc34_contract(void);

const char *
csb_v1_viewport_d1c_f0115_door_frame_pc34_source_evidence(void);

int csb_v1_viewport_d1c_f0115_door_frame_order_role_pc34(
    const CSB_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int cell_order);

int csb_v1_viewport_d1c_f0115_door_frame_zone_for_part_pc34(
    const CSB_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part);

int csb_v1_viewport_d1c_f0115_door_frame_flip_for_part_pc34(
    const CSB_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part);

const char *
csb_v1_viewport_d1c_f0115_door_frame_bitmap_for_part_pc34(
    const CSB_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part);

int csb_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
    const CSB_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

#ifdef __cplusplus
}
#endif

#endif
