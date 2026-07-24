#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_F0115_THING_PASS_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_csbgraphics_dat_real_scan.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked contract-only gate, not real-asset bitmap parity.
 * ReDMCSB anchors: DUNVIEW.C F0125_DUNGEONVIEW_DrawSquareD0L lines
 * 7960-8062 and F0126_DUNGEONVIEW_DrawSquareD0R lines 8064-8162;
 * F0128 D0 side dispatch at 8536-8541; F0115 lines 4547-4581,
 * 4806-4811, 4923, 5201-5214, 5295, 5615-5617, 5668-5683,
 * 5916-5923, 5998-5999, 6107, and 6122; F0674_F0128_sub floor/
 * ceiling bitmap copy at 2995-3015; G0163 per-frame wall bitmap rows at
 * 581-594. CSB-lineage cross-reference: Viewport.cpp:1192-1209 open
 * F0L1/F0R1 routes and 1903-1915 center door-facing dispatch.
 */

typedef enum {
    CSB_V1_D0L2_D0R2_F0115_SIDE_D0L2_PC34 = 1,
    CSB_V1_D0L2_D0R2_F0115_SIDE_D0R2_PC34 = 2
} CSB_V1_D0L2D0R2F0115ThingPassSidePc34;

typedef struct {
    const char *contract_scope;
    const char *f0125_d0l_lines;
    const char *f0126_d0r_lines;
    const char *f0128_dispatch_lines;
    const char *f0115_lines;
    const char *f0674_lines;
    const char *frame_lines;
    const char *defs_lines;
    const char *csb_lineage_open_lines;
    const char *csb_lineage_center_door_lines;
} CSB_V1_D0L2D0R2F0115ThingPassEvidencePc34;

typedef struct {
    int side;
    const char *lane_name;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int route_count;
    int f0115_call_count;
    int view_square_index;
    int view_depth;
    int view_lane;
    unsigned int f0115_cell_order;
    int f0115_first_cell;
    int f0115_cell_count;
    int wall_frame_row;
    int wall_frame_x1;
    int wall_frame_x2;
    int wall_frame_y1;
    int wall_frame_y2;
    int wall_frame_byte_width;
    int wall_frame_height;
    int wall_frame_source_x;
    int wall_frame_source_y;
    int viewport_clip_x1;
    int viewport_clip_x2;
    int viewport_clip_y1;
    int viewport_clip_y2;
    int source_clip_y1;
    int source_clip_y2;
    int c10_transparency_flag;
    int transparent_color;
    int no_write_on_transparent;
    int no_write_outside_viewport_clip;
    int no_write_outside_source_y_clip;
    int item_projectile_row;
    int item_projectile_disabled_by_g2028;
    int creature_row;
    int creature_cell_gate;
    int explosion_row;
    int field_aspect_index;
    int wall_zone;
    int ceiling_pit_zone;
    int f0112_before_f0115;
    int teleporter_field_after_f0115;
    int wall_route_excluded;
    int no_f0107_contract;
    int no_f0111_contract;
    int no_custom_backgrounds_contract;
    int f0115_draw_order_objects_first;
    int f0115_draw_order_creatures_second;
    int f0115_draw_order_projectiles_third;
    int f0115_draw_order_explosions_last;
    int f0674_per_frame_bitmap_copy;
    int wall_set_frame_used_for_field;
    int wall_set_draw_order_d0l_before_d0r;
    int csb_lineage_relative_cell;
    int csb_lineage_contents_opcode;
    int csb_lineage_draw_order_opcode;
    int csb_lineage_std_draw_room_objects_opcode;
    int csb_lineage_center_door_draw_order_first;
    int csb_lineage_center_door_draw_order_second;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0115_anchor;
    const char *source_lines;
} CSB_V1_D0L2D0R2F0115ThingPassPc34;

typedef struct {
    int valid;
    int route_backed_by_real_graphics_dat;
    int side;
    int source_graphics_dat_bound;
    int no_synthetic_pixels;
    int no_fallback_visuals;
    int wall_frame_row;
    int source_graphics_item_index;
    size_t source_byte_count;
    uint32_t source_payload_hash;
    int viewport_clip_x1;
    int viewport_clip_x2;
    int viewport_clip_y1;
    int viewport_clip_y2;
    int item_projectile_disabled_by_g2028;
    int creature_cell_gate;
    const char *redmcsb_f0115_anchor;
} CSB_V1_D0L2D0R2F0115ThingPassRealAssetReceiptPc34;

/*
 * The D0 teleporter field is drawn after F0115 (DUNVIEW.C:8050-8059 and
 * 8150-8159).  Unlike the older route fixture, this surface is a caller
 * supplied, package-decoded CSBgraphics raster.  Firestaff never invents
 * pixels for it: the source item must be the field aspect selected by the
 * corresponding G2035 row and must cover the complete clipped D0 lane.
 */
typedef struct {
    int source_graphics_dat_bound;
    int no_synthetic_pixels;
    int no_fallback_visuals;
    int source_graphics_item_index;
    const unsigned char *pixels;
    size_t pixel_count;
    int width;
    int height;
    int stride;
    uint32_t source_payload_hash;
} CSB_V1_D0L2D0R2F0115TeleporterSourceRasterPc34;

typedef struct {
    int valid;
    int side;
    int field_aspect_index;
    int source_graphics_item_index;
    size_t copied_pixels;
    size_t transparent_pixels;
    size_t clipped_pixels;
    uint32_t source_payload_hash;
} CSB_V1_D0L2D0R2F0115TeleporterRenderReceiptPc34;

/*
 * Direct CSBgraphics.dat material path for the D0 teleporter field.  The
 * entry index is deliberately independent of the ReDMCSB field-aspect zone:
 * CSBWin overrides are archive entries while G2035 still owns the in-world
 * field selection.  The palette receipt must have been admitted from the
 * exact same hash-owned cache; neither pixels nor palette are inferred.
 */
typedef struct {
    uint32_t csbgraphics_entry_index;
    const CSB_V1_CSBGraphicsDatPaletteSourceReceipt *palette_receipt;
} CSB_V1_D0L2D0R2F0115CacheTeleporterRequestPc34;

typedef struct {
    int valid;
    int consumed_hash_admitted_csbgraphics_dat;
    int no_synthetic_pixels;
    int no_fallback_visuals;
    int side;
    int field_aspect_index;
    uint32_t csbgraphics_entry_index;
    uint32_t palette_entry_index;
    uint32_t decoded_raster_hash;
    uint32_t palette_hash;
    size_t copied_pixels;
    size_t transparent_pixels;
} CSB_V1_D0L2D0R2F0115CacheTeleporterReceiptPc34;

int csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_init_pc34(void);

size_t csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_count_pc34(void);

const CSB_V1_D0L2D0R2F0115ThingPassPc34 *
csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(size_t index);

const CSB_V1_D0L2D0R2F0115ThingPassPc34 *
csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(int side);

const CSB_V1_D0L2D0R2F0115ThingPassEvidencePc34 *
csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_evidence_pc34(void);

int csb_v1_viewport_d0l2_d0r2_f0115_viewport_clip_contains_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int x,
    int y);

int csb_v1_viewport_d0l2_d0r2_f0115_source_y_visible_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int source_y);

unsigned char csb_v1_viewport_d0l2_d0r2_f0115_blend_pixel_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    unsigned char destination,
    unsigned char source);

int csb_v1_viewport_d0l2_d0r2_f0115_apply_pixel_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int x,
    int y,
    int source_y,
    unsigned char source,
    unsigned char *destination);

int csb_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

int csb_v1_viewport_d0l2_d0r2_f0115_projectile_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

int csb_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

int csb_v1_viewport_d0l2_d0r2_f0115_centered_explosion_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture);

int csb_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

int csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_real_asset_receipt_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int source_graphics_dat_bound,
    int no_synthetic_pixels,
    int no_fallback_visuals,
    int source_graphics_item_index,
    size_t source_byte_count,
    uint32_t source_payload_hash,
    CSB_V1_D0L2D0R2F0115ThingPassRealAssetReceiptPc34 *out_receipt);

/* Render a real CSBgraphics teleporter field into a 224x136 indexed D0
 * viewport. Returns one only when the complete source-owned lane was
 * admitted and composited; malformed or synthetic input is a strict no-draw.
 */
int csb_v1_viewport_d0l2_d0r2_f0115_render_teleporter_field_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    const CSB_V1_D0L2D0R2F0115TeleporterSourceRasterPc34 *source,
    unsigned char *viewport,
    size_t viewport_size,
    int viewport_stride,
    CSB_V1_D0L2D0R2F0115TeleporterRenderReceiptPc34 *out_receipt);

/* Decode and composite a field directly from the admitted CSBgraphics.dat
 * cache.  A malformed cache, unrelated palette receipt, size mismatch, or
 * decode failure is a strict no-draw and leaves the viewport unchanged. */
int csb_v1_viewport_d0l2_d0r2_f0115_render_teleporter_field_from_cache_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_D0L2D0R2F0115CacheTeleporterRequestPc34 *request,
    unsigned char *viewport,
    size_t viewport_size,
    int viewport_stride,
    CSB_V1_D0L2D0R2F0115CacheTeleporterReceiptPc34 *out_receipt);

const char *csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
