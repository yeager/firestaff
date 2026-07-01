#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1C_F0115_DOOR_FRAME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1C_F0115_DOOR_FRAME_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_D1C_DOOR_FRAME_PART_TOP = 0,
    DM1_V1_D1C_DOOR_FRAME_PART_LEFT = 1,
    DM1_V1_D1C_DOOR_FRAME_PART_RIGHT = 2,
    DM1_V1_D1C_DOOR_FRAME_PART_INVALID = -1
};

/*
 * DM1 V1 D1C F0115 door-frame edge source-lock gate hash.
 *
 * FNV-1a 32-bit hash of the source-locked door-frame edge fixture
 * (TOP/LEFT/RIGHT edges around the D1C door opening drawn by
 * F0124_DUNGEONVIEW_DrawSquareD1C at DUNVIEW.C:7873-7911 with the
 * frame blits at 7886-7893, the door at 7908, and the front F0115
 * order C0x0349 at 7910/7937). The hash pins the entire surface to
 * a single 32-bit value and is used to detect any drift in the
 * self-test invariants across rebases.
 */
#define DM1_V1_D1C_F0115_DOOR_FRAME_EDGE_HASH_PC34 0x1f9a7c34u

#define DM1_V1_D1C_F0115_DOOR_FRAME_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D1C_F0115_DOOR_FRAME_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D1C_F0115_DOOR_FRAME_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D1C_F0115_DOOR_FRAME_VIEWPORT_HEIGHT_PC34 136

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
} DM1_V1_ViewportD1CF0115DoorFramePc34Contract;

/*
 * Per-edge door-frame trace record.
 *
 * The trace is the deterministic, hash-stable mirror of the contract
 * for one frame part (TOP/LEFT/RIGHT). The probe pins every field
 * against the source-locked ReDMCSB anchors in
 * `dm1_v1_viewport_d1c_f0115_door_frame_pc34_source_evidence()` so a
 * regression in any field is detected on the very next CTest run.
 */
typedef struct {
    int part;
    int part_kind;          /* DM1_V1_D1C_DOOR_FRAME_PART_* (or PART_INVALID) */
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int zone;
    int uses_f0104;
    int uses_f0105;
    int flip_horizontal;
    int reuses_left_bitmap;
    const char *bitmap_symbol;
    int f0124_anchor_line;
    int dispatch_line;
    int cell_order_rear;
    int cell_order_front;
    int transparent_color;
    int framebuffer_strip_byte_width;
    int framebuffer_strip_destination_x;
    int framebuffer_strip_destination_y;
    int non_overlap_f0122_d1l;
    int non_overlap_f0123_d1r;
    uint32_t deterministic_hash;
} DM1_V1_D1CF0115DoorFrameEdgeTracePc34;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int top_edge_count;
    int left_edge_count;
    int right_edge_count;
    int invalid_part_count;
    int zone_anchor_checks;
    int f0104_route_checks;
    int f0105_route_checks;
    int flip_mask_checks;
    int cell_order_pairing_checks;
    int framebuffer_strip_checks;
    int transparency_color_checks;
    int f0124_anchor_checks;
    int f0128_anchor_checks;
    int non_overlap_checks;
    int bitmap_symbol_checks;
} DM1_V1_D1CF0115DoorFrameEdgeSelfTestResultPc34;

const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *
dm1_v1_viewport_d1c_f0115_door_frame_pc34_contract(void);

const char *
dm1_v1_viewport_d1c_f0115_door_frame_pc34_source_evidence(void);

int dm1_v1_viewport_d1c_f0115_door_frame_order_role_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int cell_order);

int dm1_v1_viewport_d1c_f0115_door_frame_zone_for_part_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part);

int dm1_v1_viewport_d1c_f0115_door_frame_flip_for_part_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part);

const char *
dm1_v1_viewport_d1c_f0115_door_frame_bitmap_for_part_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part);

int dm1_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

/*
 * Per-edge door-frame source-lock trace.
 *
 * `part` selects the door-frame edge (TOP/LEFT/RIGHT). Out-of-range
 * values yield a PART_INVALID trace (zone=-1, dispatch_line=-1,
 * bitmap_symbol=NULL) so callers can distinguish a real edge from a
 * bad call without an extra boolean. Returns 1 on a valid edge, 1 on
 * an invalid edge (the trace is still populated so the source-evidence
 * probe can audit PART_INVALID paths).
 */
int dm1_v1_viewport_d1c_f0115_door_frame_edge_trace_pc34(
    int part,
    DM1_V1_D1CF0115DoorFrameEdgeTracePc34 *out_trace);

/*
 * Run the door-frame edge self-test. Returns 0 when all assertions
 * pass and the deterministic_hash matches the expected constant;
 * returns 1 on any failure. The library surfaces its last result via
 * `dm1_v1_viewport_d1c_f0115_door_frame_edge_last_self_test_result_pc34`.
 */
int run_dm1_v1_viewport_d1c_f0115_door_frame_edge_self_test_pc34(void);

const DM1_V1_D1CF0115DoorFrameEdgeSelfTestResultPc34 *
dm1_v1_viewport_d1c_f0115_door_frame_edge_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
