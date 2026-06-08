#ifndef FIRESTAFF_CSB_CSB_V1_VIEWPORT_D1L_D1R_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_CSB_V1_VIEWPORT_D1L_D1R_WALL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 D1L/D1R wall source-lock, synthetic only.
 * Required source anchors: ReDMCSB DUNVIEW.C:7391-7560 F0122 D1L body,
 * DUNVIEW.C:7559-7725 F0123 D1R body, DUNVIEW.C:3113-3156 F0104 native
 * blit, DUNVIEW.C:3185-3247 F0105 scratch flip, DUNVIEW.C:4547-4581 and
 * 5668-5671 F0115 thing-pass follow-up keep-out, DUNVIEW.C:8318-8542
 * F0128 post-D1L/D1R dispatch and follow-up writes, DUNVIEW.C:8294 F0127
 * D1C follow-up boundary, DUNGEON.C:1769-1838 F0163, 1840-1905 F0164,
 * and 2466-2523 F0172 dungeon map sources, DEFS.H:2088 C10_COLOR_FLESH,
 * DEFS.H:4040-4057 C716/C717 viewport-zone spec, and CSB-lineage
 * Viewport.cpp:1192-1209,1903-1915 row composition.
 */

typedef enum {
    CSB_V1_D1L_D1R_WALL_SIDE_D1L_PC34 = 1,
    CSB_V1_D1L_D1R_WALL_SIDE_D1R_PC34 = 2
} CSB_V1_D1LD1RWallSidePc34;

typedef struct {
    int copied_pixels;
    int transparent_pixels;
    int clipped_pixels;
    int rejected;
} CSB_V1_D1LD1RWallBlitStatsPc34;

typedef struct {
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int no_c17_door_ornament;
    int no_door_state_byte;
    int no_f0111_dispatch;
    int no_f0108_floor_ornament_coupling;
    int no_c15_destroyed_mask;
    int not_csb_d1l2_d1r2_wall_gate;
    int not_dm1_wall_or_stairs_pit_dispatch;
    int side;
    int redmcsb_function_number;
    int f0128_draw_order_index;
    int view_square;
    int view_depth;
    int view_lateral;
    int wall_element;
    int teleporter_element;
    int wall_zone;
    int neighbor_d1c_wall_zone;
    int native_wall_index;
    int flipped_wall_index;
    int frame_row;
    int frame_x1;
    int frame_x2;
    int frame_y1;
    int frame_y2;
    int frame_byte_width;
    int frame_height;
    int frame_source_x;
    int frame_source_y;
    int clip_width;
    int clip_height;
    int transparent_color;
    int screen_width;
    int screen_height;
    int viewport_width;
    int viewport_height;
    int f0104_native_body_row_blit;
    int f0105_row_local_scratch_flip;
    int f0107_wall_ornament_after_body;
    int f0115_thing_pass_keepout;
    int f0128_resets_map_coordinates_after_side;
    int f0128_draws_d1c_after_pair;
    int f0127_d1c_followup_boundary;
    int f0163_link_thing_keepout;
    int f0164_unlink_thing_keepout;
    int f0172_square_aspect_read;
    int lineage_open_row_composition;
    int lineage_door_row_cross_check;
    const char *route_name;
    const char *frame_symbol;
    const char *bitmap_symbol;
    const char *source_lines;
} CSB_V1_D1LD1RWallSpecPc34;

typedef struct {
    int ok;
    int route_count;
    int identities_ok;
    int coordinates_ok;
    int c10_transparency_ok;
    int row_local_flip_ok;
    int edge_clip_ok;
    int f0128_followup_ok;
    int dungeon_identity_ok;
    int scope_keepout_ok;
    int d1l_copied_pixels;
    int d1r_copied_pixels;
    uint16_t first_thing_before;
    uint16_t first_thing_after;
    int map_x_before;
    int map_y_before;
    int map_x_after;
    int map_y_after;
} CSB_V1_D1LD1RWallRunResultPc34;

size_t csb_v1_viewport_d1l_d1r_wall_spec_count_pc34(void);

const CSB_V1_D1LD1RWallSpecPc34 *
csb_v1_viewport_d1l_d1r_wall_spec_at_pc34(size_t index);

const CSB_V1_D1LD1RWallSpecPc34 *
csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(int side);

int csb_v1_viewport_d1l_d1r_wall_zone_for_square_pc34(int view_square);

int csb_v1_viewport_d1l_d1r_wall_map_viewport_x_to_source_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec,
    int viewport_x,
    int flipped_variant,
    int *out_source_x);

int csb_v1_viewport_d1l_d1r_wall_apply_c10_frame_clip_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec,
    const uint8_t *source,
    int source_width,
    int source_height,
    uint8_t *viewport,
    int viewport_width,
    int viewport_height,
    int flipped_variant,
    CSB_V1_D1LD1RWallBlitStatsPc34 *stats);

uint16_t csb_v1_viewport_d1l_d1r_wall_preserve_first_thing_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec,
    uint16_t first_thing);

int csb_v1_viewport_d1l_d1r_wall_preserve_map_identity_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec,
    int in_map_x,
    int in_map_y,
    int *out_map_x,
    int *out_map_y);

int csb_v1_viewport_d1l_d1r_wall_pc34_compat_run(
    CSB_V1_D1LD1RWallRunResultPc34 *out_result);

const char *csb_v1_viewport_d1l_d1r_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
