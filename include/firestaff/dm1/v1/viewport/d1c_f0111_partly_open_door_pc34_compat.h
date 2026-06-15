#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1C_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1C_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D1C center F0111 partly-open door source-lock gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor; open state guard at
 *   4248, partly-open state decrement at 4308, horizontal frame selection
 *   at 4311-4313, first-half C10 blit through zone + C6_UNKNOWN at
 *   4317-4324, and second-half zone shift by 3 | MASK0x4000 before the
 *   F0791 C10 blit at 4325-4334.
 * - DUNVIEW.C:7873-7911 F0124_DUNGEONVIEW_DrawSquareD1C
 *   C17_ELEMENT_DOOR_FRONT branch; it preserves DoorPass1 order 0x0218,
 *   dispatches F0111 at 7905/7908 with
 *   G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, 96x88,
 *   C2_VIEW_DOOR_ORNAMENT_D1LCR, G0186_s_Graphic558_Frames_Door_D1C,
 *   and M631_ZONE_DOOR_D1C, then preserves DoorPass2 order 0x0349.
 * - DUNVIEW.C:8518-8533 F0128_DUNGEONVIEW_Draw_CPSF dispatch order;
 *   D1C is the depth=1 center lane call to F0124 at line 8533.
 * - DUNVIEW.C:694-705 defines G0186_s_Graphic558_Frames_Door_D1C.
 * - DEFS.H:1039-1043 C0..C4 door states; 2088 C10_COLOR_FLESH;
 *   2599 M606_VIEW_SQUARE_D1C; 2791 C2_VIEW_DOOR_ORNAMENT_D1LCR;
 *   3508 C6_UNKNOWN; 3516 MASK0x4000; 4259 M631_ZONE_DOOR_D1C;
 *   5458 G0695 extern; 5543 G0186 extern.
 *
 * This gate is contract-only. It does not load game data and does not claim
 * real-asset or original-DOS pixel parity. The closed-door sibling is
 * tests/test_dm1_v1_viewport_d1c_f0111_door_pc34_compat.c.
 */

#define DM1_V1_D1C_F0111_PARTLY_OPEN_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D1C_F0111_PARTLY_OPEN_C6_UNKNOWN_PC34 6
#define DM1_V1_D1C_F0111_PARTLY_OPEN_MASK0X4000_PC34 0x4000

typedef struct {
    int input_state;
    int accepted_partly_open;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int view_square_m606_d1c;
    int view_depth;
    int center_lane;
    int door_zone_m631_d1c;
    int door_width;
    int door_height;
    int door_byte_count;
    int door_ornament_view;
    int doorpass1_order;
    int doorpass2_order;
    int decremented_state;
    int first_half_base_zone;
    int first_half_clip_zone;
    int first_half_c10_blit;
    int second_half_shift;
    int second_half_zone;
    int second_half_c10_blit;
    int left_horizontal_frame_selected;
    int right_horizontal_frame_selected;
    const char *left_horizontal_frame_name;
    const char *right_horizontal_frame_name;
} DM1_V1_D1CF0111PartlyOpenDoorTracePc34;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int d1c_partly_one;
    int d1c_partly_two;
    int d1c_partly_three;
    int closed_rejections;
    int open_rejections;
    int unknown_rejections;
    int door_dim_96x88_anchors;
    int c10_zone_blits;
    int second_half_shifts;
    int doorpass_order_anchors;
} DM1_V1_D1CF0111PartlyOpenDoorSelfTestResultPc34;

int dm1_v1_viewport_d1c_f0111_partly_open_door_trace_pc34(
    int door_state,
    DM1_V1_D1CF0111PartlyOpenDoorTracePc34 *out_trace);

const char *
dm1_v1_viewport_d1c_f0111_partly_open_door_source_evidence_pc34(void);

int run_dm1_v1_viewport_d1c_f0111_partly_open_door_self_test(void);

const DM1_V1_D1CF0111PartlyOpenDoorSelfTestResultPc34 *
dm1_v1_viewport_d1c_f0111_partly_open_door_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
