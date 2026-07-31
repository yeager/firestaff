#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34 = 0,
    CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34 = 1
} CSB_V1_ViewportD3L2D3R2F0111DoorSidePc34;

typedef struct {
    CSB_V1_ViewportD3L2D3R2F0111DoorSidePc34 side;
    int view_square;
    int element_door_front;
    int floor_view;
    const char *floor_ornament_ordinal_slot;
    int f0115_view_square;
    int door_pass1_order;
    int door_pass2_order;
    int door_native_bitmap_index_family;
    int door_ornament_view;
    int door_zone;
    int transparent_color;
    int native_bitmap_byte_width;
    int native_bitmap_height;
    int frame_byte_width;
    int l0201_pwall_left_index;
    int l0201_pwall_right_index;
    int lineage_frame_bitmap_index;
    int lineage_frame_rect_index;
    int lineage_door_graphic_index;
    int lineage_door_graphic_size;
    int lineage_door_nearness;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34;

typedef struct {
    int ok;
    int f0108_called;
    int f0108_floor_view;
    const char *f0108_ordinal_slot;
    int f0115_pass1_called;
    int f0115_pass1_view_square;
    int f0115_pass1_order;
    int f0111_called;
    int f0111_native_bitmap_index_family;
    int f0111_door_ornament_view;
    int f0111_zone;
    int f0115_pass2_order_set_before_dispatch;
    int f0115_pass2_order;
    int f0128_dispatch_reached_after_pass2;
    int f0111_transparent_color;
    int native_bitmap_fetches_via_f0489;
    int resolved_native_bitmap_index;
    int preserved_frame_byte_width;
    int f0107_called;
    int l0201_pwall_parity_preserved;
    int lineage_pwall_left_index;
    int lineage_pwall_right_index;
    int pixel_anchor_ready;
    const char *source_lock_evidence;
} CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34;

typedef struct {
    int valid;
    int route_backed_by_real_graphics_dat;
    int source_graphics_dat_bound;
    int no_synthetic_pixels;
    int no_fallback_visuals;
    int source_graphics_item_index;
    size_t source_byte_count;
    uint32_t source_payload_hash;
    int d3l2_view_square;
    int d3r2_view_square;
    int d3l2_door_zone;
    int d3r2_door_zone;
    int d3l2_pass1_order;
    int d3r2_pass1_order;
    int d3l2_pass2_order;
    int d3r2_pass2_order;
    int door_ornament_view;
    int transparent_color;
    int native_bitmap_byte_width;
    int native_bitmap_height;
    const char *redmcsb_d3l2_function;
    const char *redmcsb_d3r2_function;
} CSB_V1_ViewportD3L2D3R2F0111DoorRealAssetReceiptPc34;

/* ReDMCSB DUNVIEW.C:2651-2658 selects three consecutive door graphics for
 * each map DoorSet: D3, D2 and D1.  These helpers keep the GRAPHICS.DAT
 * identity tied to that source formula instead of a captured cache index. */
int csb_v1_viewport_door_graphic_index_pc34(int door_set, int depth);
/* Select G0693/G0694/G0695 from the two active MAP.D DoorSet selectors and
 * the DB0 door-type bit.  A malformed map selector or a non-PC34 depth is
 * unavailable, never coerced to DoorSet 0. */
int csb_v1_viewport_door_graphic_index_from_map_pc34(int door_set0,
                                                      int door_set1,
                                                      uint16_t door_word,
                                                      int depth);
int csb_v1_viewport_door_graphic_index_valid_pc34(int graphic_index,
                                                   int depth);
int csb_v1_viewport_d3_door_graphic_index_valid_pc34(int graphic_index);

size_t csb_v1_viewport_d3l2_d3r2_f0111_door_count_pc34(void);

const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *
csb_v1_viewport_d3l2_d3r2_f0111_door_at_pc34(size_t index);

const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *
csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
    CSB_V1_ViewportD3L2D3R2F0111DoorSidePc34 side);

int csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *spec,
    int door_type,
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 *out_result);

int csb_v1_viewport_d3l2_d3r2_f0111_door_apply_c10_blit_pc34(
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

int csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2,
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3r2,
    int source_graphics_dat_bound,
    int no_synthetic_pixels,
    int no_fallback_visuals,
    int source_graphics_item_index,
    size_t source_byte_count,
    uint32_t source_payload_hash,
    CSB_V1_ViewportD3L2D3R2F0111DoorRealAssetReceiptPc34 *out_receipt);

const char *csb_v1_viewport_d3l2_d3r2_f0111_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
