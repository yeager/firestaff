#ifndef FIRESTAFF_CSB_CSB_V1_VIEWPORT_D1L_D1R_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_CSB_V1_VIEWPORT_D1L_D1R_WALL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_startup_img3_decode_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CSB V1 D1L/D1R wall source-lock and PC3.4 GRAPHICS.DAT binding.
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

/* One material selected by PC/I34 F0095 and consumed by F0122/F0123.
 * `pixels` returned by the loader belongs to the caller; its receipt ties it
 * to one compressed original GRAPHICS.DAT record rather than a look-alike
 * host buffer. */
typedef struct {
    int valid;
    int wall_set;
    int side;
    uint32_t graphics_entry_index;
    int width;
    int height;
    int transparent_color;
    int flip_horizontally;
    CSB_V1_StartupGraphicDecodeReceipt_PC34 decode_receipt;
} CSB_V1_D1LD1RWallMaterialPc34;

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

/* Loads only the mapped PC3.4 source record for this side wall. The call
 * fails closed for an unknown side, invalid wall set/catalog, a decoder
 * mismatch, or a non-60x111 source raster. */
int csb_v1_viewport_d1l_d1r_wall_load_graphics_dat_material_pc34(
    const char *graphics_dat_path,
    int wall_set,
    int side,
    uint32_t graphics_entry_count,
    unsigned char **out_pixels,
    CSB_V1_D1LD1RWallMaterialPc34 *out_material);

uint16_t csb_v1_viewport_d1l_d1r_wall_preserve_first_thing_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec,
    uint16_t first_thing);

int csb_v1_viewport_d1l_d1r_wall_preserve_map_identity_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec,
    int in_map_x,
    int in_map_y,
    int *out_map_x,
    int *out_map_y);

const char *csb_v1_viewport_d1l_d1r_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
