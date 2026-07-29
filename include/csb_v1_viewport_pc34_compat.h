#ifndef FIRESTAFF_CSB_V1_VIEWPORT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_csbgraphics_dat_real_scan.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_viewport_f0115_projectile_metadata_pc34_compat.h"
#include "csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

typedef struct CSB_V1_ViewportRuntimeProjectileOverlayPlacement
    CSB_V1_ViewportRuntimeProjectileOverlayPlacement;
typedef struct CSB_V1_ViewportRuntimeObjectOverlayPlacement
    CSB_V1_ViewportRuntimeObjectOverlayPlacement;
typedef struct CSB_V1_ViewportRuntimeGroupOverlayPlacement
    CSB_V1_ViewportRuntimeGroupOverlayPlacement;
typedef struct CSB_V1_ViewportRuntimeExplosionOverlayPlacement
    CSB_V1_ViewportRuntimeExplosionOverlayPlacement;
typedef struct CSB_V1_ViewportRuntimeProjectileSpriteBlit
    CSB_V1_ViewportRuntimeProjectileSpriteBlit;
typedef struct CSB_V1_ViewportRuntimeObjectSpriteBlit
    CSB_V1_ViewportRuntimeObjectSpriteBlit;
typedef struct CSB_V1_ViewportRuntimeObjectIconBlit
    CSB_V1_ViewportRuntimeObjectIconBlit;
typedef struct CSB_V1_ViewportRuntimeGroupSpriteBlit
    CSB_V1_ViewportRuntimeGroupSpriteBlit;
typedef struct CSB_V1_ViewportRuntimeExplosionSpriteBlit
    CSB_V1_ViewportRuntimeExplosionSpriteBlit;

#define CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0111_DOOR 0x0001u
#define CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0115_THING 0x0002u
#define CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0111_DOOR 0x0004u
#define CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0115_THING 0x0008u
#define CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D2_F0111_DOOR 0x0010u
#define CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR 0x0020u
#define CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR 0x0040u
#define CSB_V1_VIEWPORT_FIRST_FRAME_REQUIRED_ROUTES \
    (CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0111_DOOR | \
     CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0115_THING | \
     CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0111_DOOR | \
     CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0115_THING | \
     CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D2_F0111_DOOR)

typedef struct {
    int valid;
    unsigned int route_mask;
    int source_graphics_dat_bound;
    int no_synthetic_pixels;
    int no_fallback_visuals;
    int shared_palette_material_proof;
    uint32_t shared_palette_hash;
    uint32_t d0_door_hash;
    uint32_t d0_thing_hash;
    uint32_t d1_door_hash;
    uint32_t d1_thing_hash;
    uint32_t d2_door_hash;
    /* F0489-selected native packed bytes and F0488-expanded indexed bytes
     * have separate identities. The source payload hash remains above; these
     * values are set only by a checked native-span decode receipt. */
    uint32_t d2_door_decoded_hash;
    /* The mandatory D2C F0111 route carries a value copy of the checked
     * G0694 capture facts.  Do not accept the payload FNV on its own: that
     * would let a caller relabel unrelated GRAPHICS.DAT bytes as D2C door
     * material after the route receipt was produced. */
    int d2_door_capture_valid;
    int d2_door_capture_real_graphics_dat;
    int d2_door_capture_no_synthetic_pixels;
    int d2_door_capture_no_fallback_visuals;
    int d2_door_capture_item_index;
    size_t d2_door_capture_byte_count;
    uint32_t d2_door_capture_payload_hash;
    int d2_door_capture_width;
    int d2_door_capture_height;
    int d2_door_capture_zone;
    int d2_door_capture_transparent_color;
    /* The two far D3 side doors are an all-or-nothing G0693 pair.  They are
     * optional for the original D0/D1/D2 first-frame receipt, but may enter
     * the live draw plan only after the paired real-asset receipt agrees. */
    uint32_t d3l2_door_hash;
    uint32_t d3r2_door_hash;
    uint32_t d3l2_door_decoded_hash;
    uint32_t d3r2_door_decoded_hash;
    CSB_V1_ViewportD3L2D3R2F0111DoorRealAssetReceiptPc34
        d3_pair_real_asset_receipt;
    size_t source_item_count;
    const char *source_evidence;
} CSB_V1_ViewportFirstFrameMaterialProof;

typedef struct {
    int valid;
    int consumed_by_m11_render;
    unsigned int route_mask;
    uint32_t combined_material_hash;
    int required_route_count;
    int real_graphics_session;
    int no_synthetic_pixels;
    int no_fallback_visuals;
    uint32_t capture_identity_hash;
    uint32_t d2_source_payload_hash;
    uint32_t d2_decoded_hash;
    uint32_t d3_source_payload_hash;
    uint32_t d3_decoded_hash;
    const char *source_evidence;
} CSB_V1_ViewportFirstFrameMaterializationReceipt;

typedef enum {
    CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_NONE_PC34 = 0,
    CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D0_F0111_DOOR_PC34,
    CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D0_F0115_THING_PC34,
    CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D1_F0111_DOOR_PC34,
    CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D1_F0115_THING_PC34,
    CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D2_F0111_DOOR_PC34,
    CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3L2_F0111_DOOR_PC34,
    CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3R2_F0111_DOOR_PC34
} CSB_V1_ViewportRuntimeDrawRoutePc34;

#define CSB_V1_VIEWPORT_RUNTIME_FIRST_FRAME_DRAW_COMMAND_BASE_COUNT_PC34 5
#define CSB_V1_VIEWPORT_RUNTIME_FIRST_FRAME_DRAW_COMMAND_CAP_PC34 7

typedef struct {
    unsigned int route_bit;
    const uint8_t *decoded_pixels;
    size_t decoded_size;
    uint32_t decoded_fnv1a;
    int width;
    int height;
} CSB_V1_ViewportFirstFrameMaterialSpanPc34;

typedef struct {
    const uint8_t *decoded_palette;
    size_t decoded_size;
    uint32_t decoded_fnv1a;
} CSB_V1_ViewportFirstFramePaletteSpanPc34;

/* A caller may only name decoded D1/D2/D3 pixels after an original-data capture
 * has repeated both the compressed GRAPHICS.DAT identity and the palette
 * identity. This intentionally has no decoder or palette substitute: the
 * capture owner supplies already verified indexed spans. */
typedef struct {
    int valid;
    int original_graphics_dat_capture;
    int no_synthetic_pixels;
    int no_fallback_visuals;
    const char *source_path;
    const char *source_md5;
    const char *palette_source_path;
    const char *palette_source_md5;
    uint32_t palette_capture_fnv1a;
    uint32_t capture_identity_hash;
    /* ReDMCSB G0695 selects the D1 front-door source as DoorSet * 3 + 2.
     * Graphic558 is the destination frame buffer, never the source asset. */
    int d1_item_index;
    size_t d1_source_byte_count;
    uint32_t d1_source_payload_hash;
    int d2_item_index;
    size_t d2_source_byte_count;
    uint32_t d2_source_payload_hash;
    const uint8_t *d2_decoded_pixels;
    size_t d2_decoded_size;
    uint32_t d2_decoded_fnv1a;
    int d2_width;
    int d2_height;
    int d3_item_index;
    size_t d3_source_byte_count;
    uint32_t d3_source_payload_hash;
    const uint8_t *d3_decoded_pixels;
    size_t d3_decoded_size;
    uint32_t d3_decoded_fnv1a;
    int d3_width;
    int d3_height;
} CSB_V1_ViewportD2D3MaterialCaptureReceiptPc34;

/* GRAPHICS.DAT's compressed/decompressed entry tables prove file layout, but
 * do not by themselves establish that a DUNVIEW native-bitmap index (G0693
 * or G0694) names a particular GRAPHICS.DAT entry. Keep that absence explicit
 * so F0489/F0488 cannot turn an index coincidence into a draw source. */
typedef struct {
    int valid;
    int original_graphics_dat_table;
    int native_bitmap_mapping_proven;
    int raster_blocked_without_mapping;
    const char *source_path;
    const char *source_md5;
    uint32_t native_bitmap_index;
    uint32_t graphics_entry_index;
    uint32_t graphics_entry_count;
    size_t table_span_bytes;
    uint32_t table_span_fnv1a;
} CSB_V1_ViewportGraphicsTableProvenancePc34;

typedef struct {
    int valid;
    int original_graphics_dat_capture;
    int f0489_native_bitmap_selected;
    int f0488_expand_4bpp;
    int no_synthetic_pixels;
    int no_fallback_visuals;
    const char *source_path;
    const char *source_md5;
    const char *palette_source_path;
    const char *palette_source_md5;
    uint32_t palette_capture_fnv1a;
    uint32_t capture_identity_hash;
    const CSB_V1_ViewportGraphicsTableProvenancePc34 *d2_table_provenance;
    const CSB_V1_ViewportGraphicsTableProvenancePc34 *d3_table_provenance;
    int d2_item_index;
    uint32_t d2_source_payload_hash;
    const uint8_t *d2_packed_pixels;
    size_t d2_packed_size;
    uint32_t d2_packed_fnv1a;
    int d3_item_index;
    uint32_t d3_source_payload_hash;
    const uint8_t *d3_packed_pixels;
    size_t d3_packed_size;
    uint32_t d3_packed_fnv1a;
} CSB_V1_ViewportD2D3NativePackedCapturePc34;

typedef struct {
    int valid;
    const char *source_path;
    const char *source_md5;
    CSB_V1_ViewportFirstFramePaletteSpanPc34 palette;
    CSB_V1_ViewportD2D3MaterialCaptureReceiptPc34 d2_d3_capture;
    CSB_V1_ViewportFirstFrameMaterialSpanPc34 materials[
        CSB_V1_VIEWPORT_RUNTIME_FIRST_FRAME_DRAW_COMMAND_CAP_PC34];
} CSB_V1_ViewportFirstFrameMaterialBytesPc34;

/* ReDMCSB MEMORY.C F0489 selects the native packed bitmap and F0488 expands
 * its 4bpp rows. This adapter accepts no GRAPHICS.DAT entry-number mapping;
 * the caller must provide the already source-selected native span and exact
 * palette/capture identity. */
int csb_v1_viewport_decode_d2_d3_native_packed_capture_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof,
    const CSB_V1_ViewportFirstFramePaletteSpanPc34 *palette,
    const CSB_V1_ViewportD2D3NativePackedCapturePc34 *packed_capture,
    uint8_t *d2_decoded_pixels, size_t d2_decoded_capacity,
    uint8_t *d3_decoded_pixels, size_t d3_decoded_capacity,
    CSB_V1_ViewportD2D3MaterialCaptureReceiptPc34 *out_receipt);

/* Reads only the original big-endian GRAPHICS.DAT signature and its bounded
 * compressed/decompressed table span. No native-index to entry-index mapping
 * is guessed here, so the returned receipt deliberately keeps raster blocked
 * until source evidence supplies such a mapping. */
int csb_v1_viewport_admit_graphics_table_provenance_pc34(
    const uint8_t *table_bytes, size_t table_size,
    const char *source_path, const char *source_md5,
    uint32_t native_bitmap_index,
    CSB_V1_ViewportGraphicsTableProvenancePc34 *out_receipt);

typedef struct {
    int valid;
    int consumed_by_raster;
    int rejected;
    int command_count;
    uint32_t combined_material_hash;
    uint32_t raster_hash;
} CSB_V1_ViewportFirstFrameRasterReceiptPc34;

typedef enum {
    CSB_V1_VIEWPORT_LIVE_SURFACE_WALL_PC34 = 0,
    CSB_V1_VIEWPORT_LIVE_SURFACE_FLOOR_PC34,
    CSB_V1_VIEWPORT_LIVE_SURFACE_DOOR_PC34
} CSB_V1_ViewportLiveSurfaceKindPc34;

#define CSB_V1_VIEWPORT_LIVE_FRAME_SURFACE_COUNT_PC34 3
#define CSB_V1_VIEWPORT_OPERATOR_DECLARATION_MAX_FRAMES_PC34 32

typedef struct {
    CSB_V1_ViewportLiveSurfaceKindPc34 kind;
    const uint8_t *decoded_pixels;
    size_t decoded_size;
    uint32_t decoded_fnv1a;
    int width;
    int height;
    int clip_x;
    int clip_y;
    int clip_w;
    int clip_h;
    int transparent_color;
} CSB_V1_ViewportLiveSurfaceSpanPc34;

typedef struct {
    int valid;
    unsigned int frame_number;
    int door_state;
    const char *source_path;
    const char *source_md5;
    CSB_V1_ViewportFirstFramePaletteSpanPc34 palette;
    CSB_V1_ViewportLiveSurfaceSpanPc34 surfaces[
        CSB_V1_VIEWPORT_LIVE_FRAME_SURFACE_COUNT_PC34];
} CSB_V1_ViewportLiveFrameSourcePc34;

typedef struct {
    int valid;
    unsigned int last_frame_number;
    int last_door_state;
    uint32_t source_identity_hash;
    uint32_t palette_hash;
} CSB_V1_ViewportLiveFrameProgressionPc34;

typedef struct {
    int valid;
    int consumed_by_m11_render;
    unsigned int frame_number;
    int door_state;
    uint32_t source_identity_hash;
    uint32_t palette_hash;
    uint32_t wall_hash;
    uint32_t floor_hash;
    uint32_t door_hash;
} CSB_V1_ViewportLiveFrameReceiptPc34;

typedef struct {
    uint32_t entry_index;
    int width;
    int height;
    uint32_t decoded_fnv1a;
    int clip_x;
    int clip_y;
    int clip_w;
    int clip_h;
    int transparent_color;
} CSB_V1_ViewportLiveSurfaceDeclarationPc34;

typedef struct {
    unsigned int frame_number;
    int door_state;
    const char *source_path;
    const char *source_md5;
    const CSB_V1_CSBGraphicsDatPaletteSourceReceipt *palette_receipt;
    CSB_V1_ViewportLiveSurfaceDeclarationPc34 surfaces[
        CSB_V1_VIEWPORT_LIVE_FRAME_SURFACE_COUNT_PC34];
} CSB_V1_ViewportLiveFrameDeclarationPc34;

typedef struct {
    int valid;
    char source_path[CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP];
    char source_md5[CSB_V1_CSBGRAPHICS_DAT_REAL_MD5_CAP];
    const CSB_V1_CSBGraphicsDatPaletteSourceReceipt *palette_receipt;
    uint8_t palette_bytes[CSB_V1_CSBGRAPHICS_DAT_PALETTE_BYTES];
    uint8_t *decoded_surface_bytes[CSB_V1_VIEWPORT_LIVE_FRAME_SURFACE_COUNT_PC34];
    CSB_V1_ViewportLiveFrameSourcePc34 source;
} CSB_V1_ViewportLiveFrameMaterialPc34;

typedef struct {
    unsigned int frame_number;
    int door_state;
    uint32_t wall_entry_index;
    uint32_t floor_entry_index;
    uint32_t door_entry_index;
    const char *source_path;
    const char *source_md5;
} CSB_V1_ViewportLiveDungeonStatePc34;

typedef struct {
    int valid;
    int invalidated_previous;
    unsigned int frame_number;
    int door_state;
    uint32_t state_identity_hash;
    const CSB_V1_ViewportLiveFrameDeclarationPc34 *declaration;
} CSB_V1_ViewportLiveDungeonSelectionPc34;

typedef struct {
    int valid;
    int real_asset_matched;
    int terminal_session_owned;
    int viewport_frame_consumed;
    int no_synthetic_surface;
    uint32_t session_generation;
    uint32_t source_tick;
} CSB_V1_ViewportVerifiedDungeonIngressPc34;

typedef struct {
    int valid;
    const char *source_path;
    const char *source_md5;
    const CSB_V1_CSBGraphicsDatPaletteSourceReceipt *palette_receipt;
    const CSB_V1_ViewportLiveFrameDeclarationPc34 *declarations;
    size_t declaration_count;
} CSB_V1_ViewportOperatorDeclarationCorpusPc34;

typedef struct {
    int valid;
    char source_path[CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP];
    char source_md5[CSB_V1_CSBGRAPHICS_DAT_REAL_MD5_CAP];
    const CSB_V1_CSBGraphicsDatPaletteSourceReceipt *palette_receipt;
    CSB_V1_ViewportLiveFrameDeclarationPc34
        declarations[CSB_V1_VIEWPORT_OPERATOR_DECLARATION_MAX_FRAMES_PC34];
    size_t declaration_count;
} CSB_V1_ViewportOperatorDeclarationManifestPc34;

typedef struct {
    CSB_V1_ViewportRuntimeDrawRoutePc34 route;
    unsigned int route_bit;
    uint32_t material_hash;
    uint32_t palette_hash;
    /* Filled only by the checked GRAPHICS.DAT material-byte handoff. */
    const uint8_t *decoded_pixels;
    size_t decoded_size;
    int decoded_width;
    int decoded_height;
    /* Active map G0693/G0694 record admitted by the checked byte handoff.
     * Zero means that this command has no source-index receipt. */
    int source_graphics_item_index;
    const uint8_t *decoded_palette;
    size_t decoded_palette_size;
    int view_depth;
    int view_square;
    int draw_order;
    int clip_x;
    int clip_y;
    int clip_w;
    int clip_h;
    int transparent_color;
    int party_dir;
    int party_x;
    int party_y;
    int input_forward;
    int input_side;
} CSB_V1_ViewportRuntimeDrawCommandPc34;

typedef struct {
    int valid;
    int consumed_by_m11_render;
    int real_graphics_session;
    int no_synthetic_pixels;
    int no_fallback_visuals;
    int palette_bound;
    int clip_bound;
    int state_bound;
    int input_bound;
    unsigned int route_mask;
    int command_count;
    uint32_t shared_palette_hash;
    uint32_t plan_hash;
    CSB_V1_ViewportRuntimeDrawCommandPc34
        commands[CSB_V1_VIEWPORT_RUNTIME_FIRST_FRAME_DRAW_COMMAND_CAP_PC34];
    const char *source_evidence;
} CSB_V1_ViewportRuntimeDrawPlanPc34;

typedef int (*CSB_V1_ViewportProjectileSpriteDrawer)(
    void *user,
    const CSB_V1_ViewportRuntimeProjectileSpriteBlit *blit,
    uint8_t *screen_pixels,
    int screen_stride);

typedef int (*CSB_V1_ViewportExplosionSpriteDrawer)(
    void *user,
    const CSB_V1_ViewportRuntimeExplosionSpriteBlit *blit,
    uint8_t *screen_pixels,
    int screen_stride);

typedef int (*CSB_V1_ViewportObjectSpriteDrawer)(
    void *user,
    const CSB_V1_ViewportRuntimeObjectSpriteBlit *blit,
    uint8_t *screen_pixels,
    int screen_stride);

typedef int (*CSB_V1_ViewportObjectIconDrawer)(
    void *user,
    const CSB_V1_ViewportRuntimeObjectIconBlit *blit,
    uint8_t *screen_pixels,
    int screen_stride);

typedef int (*CSB_V1_ViewportGroupSpriteDrawer)(
    void *user,
    const CSB_V1_ViewportRuntimeGroupSpriteBlit *blit,
    uint8_t *screen_pixels,
    int screen_stride);

#define CSB_V1_VIEWPORT_RUNTIME_DRAWER_BINDING_TYPE_DEFINED 1
typedef struct CSB_V1_ViewportRuntimeDrawerBinding {
    int src_x;
    int src_y;
    int dst_x;
    int dst_y;
    int width;
    int height;
    const uint16_t *mask_words;
    size_t mask_word_count;
} CSB_V1_ViewportCustomBackgroundMask;

/* CSB V1 Viewport — CSB-specific rendering differences
 *
 * CSB shares the DM1 viewport engine but has:
 * - Different wall sets (CSB dungeon themes)
 * - Custom room backgrounds (per DSA script)
 * - Extended creature graphics
 * - Prison door / intro sequence renderer
 *
 * Source: CSBWin/Viewport.cpp (7290 lines)
 * Source: CSBWin/Graphics.cpp (3186 lines)
 * Base: ReDMCSB DUNVIEW.C (shared viewport core)
 */

#define CSB_V1_VIEWPORT_RUNTIME_DRAW_COUNTS_TYPE_DEFINED 1
typedef struct CSB_V1_ViewportRuntimeDrawCounts {
    int wall_set_index;
    int custom_background;
    int prison_door_open;  /* 0-100 open percentage for intro */
    int has_custom_ceiling;
    uint32_t ambient_color;

    /* Viewport pixel buffer (224×136, the dungeon view area).
     * Points into the global g_framebuffer or a screen buffer.
     * Viewport occupies pixel rows [33..168] of a 320×200 screen. */
    uint8_t *viewport_pixels;
    int      viewport_stride;  /* bytes per row (320 for screen) */

    /* Dungeon data for rendering decisions.
     * When viewport_pixels is NULL, render calls are no-ops. */
    const uint8_t *dungeon_grid;
    int dungeon_width;
    int dungeon_height;

    /* Optional live runtime overlays.  CSB V1 runtime owns these lists;
     * the viewport treats NULL as "no live projectile/explosion overlay". */
    const CSB_V1_RuntimeProfile *runtime_profile;
    const struct ProjectileList_Compat *runtime_projectiles;
    const struct ExplosionList_Compat *runtime_explosions;
    CSB_V1_ViewportObjectSpriteDrawer object_sprite_drawer;
    void *object_sprite_user;
    /* A verified CSBGRAPHICS.DAT object drawer must fail closed. Diagnostic
     * icons/markers remain available only to data-free geometry probes. */
    int object_sprite_drawer_source_bound;
    int runtime_object_sprite_drawn_count;
    CSB_V1_ViewportObjectIconDrawer object_icon_drawer;
    void *object_icon_user;
    int runtime_object_icon_drawn_count;
    int runtime_object_marker_drawn_count;
    CSB_V1_ViewportGroupSpriteDrawer group_sprite_drawer;
    void *group_sprite_user;
    /* A real M11 CSBgraphics session must never replace a missing creature
     * bitmap with the diagnostic group marker.  Data-free callers retain the
     * marker fallback so geometry-only regressions can still inspect their
     * placement plans. */
    int group_sprite_drawer_source_bound;
    int runtime_group_sprite_drawn_count;
    int runtime_group_marker_drawn_count;
    int runtime_projectile_material_resolved_count;
    int runtime_projectile_material_icon_drawn_count;
    CSB_V1_ViewportProjectileSpriteDrawer projectile_sprite_drawer;
    void *projectile_sprite_user;
    int runtime_projectile_sprite_drawn_count;
    int runtime_projectile_marker_drawn_count;
    const CSB_V1_F0219ProjectileImpactMaterialHandoffPc34
        *post_teleport_projectile_handoffs;
    size_t post_teleport_projectile_handoff_count;
    int runtime_post_teleport_projectile_handoff_drawn_count;
    int runtime_post_teleport_projectile_handoff_blocked_count;
    CSB_V1_ViewportExplosionSpriteDrawer explosion_sprite_drawer;
    void *explosion_sprite_user;
    int runtime_explosion_sprite_drawn_count;
    int runtime_explosion_marker_drawn_count;
    int runtime_overlay_source_required;
    int runtime_overlay_source_admitted;
    uint32_t runtime_overlay_source_hash;
    int real_graphics_session;
    const CSB_V1_ViewportFirstFrameMaterialProof *first_frame_material_proof;
    const CSB_V1_ViewportFirstFrameMaterialBytesPc34 *first_frame_material_bytes;
    const char *first_frame_material_source_path;
    const char *first_frame_material_source_md5;
    int first_frame_material_consumed_count;
    int first_frame_material_blocked_count;
    uint32_t first_frame_material_hash;
    int first_frame_draw_plan_consumed_count;
    int first_frame_draw_plan_blocked_count;
    int first_frame_draw_plan_command_count;
    uint32_t first_frame_draw_plan_hash;
    uint32_t first_frame_draw_plan_palette_hash;
    int first_frame_material_raster_consumed_count;
    int first_frame_material_raster_blocked_count;
    uint32_t first_frame_material_raster_hash;

    /* The shared F0128/F0098 core asks its caller for the PC3.4-expanded
     * C079/C078 aperture.  CSB owns this bridge so a verified session never
     * falls through to a host-generated floor or ceiling surface. */
    DM1_ViewportGraphicProviderCallback graphic_provider_callback;
    void *graphic_provider_user_data;

    DM1_ViewportWallOrnamentOrdinalCallback wall_ornament_ordinal_callback;
    void *wall_ornament_ordinal_user_data;

    /* Optional CSBgraphics.dat CustomBackgrounds bridge. The CSB boot layer
     * owns the plan/cache/skin-def bytes; the viewport can select a
     * cell/default skin, decode CSBWin room mask geometry, and still prefer
     * caller-supplied synthetic masks for focused tests. */
    const void *csbgraphics_plan;
    const void *csbgraphics_cache;
    const uint16_t *custom_background_skin_def_words;
    size_t custom_background_skin_def_word_count;
    const uint8_t *custom_background_cell_skins;
    int custom_background_cell_skin_width;
    int custom_background_cell_skin_height;
    int custom_background_loaded_level;
    int custom_background_default_skin;
    int custom_background_selected_skin_num;
    int custom_background_used_default_skin;
    int custom_background_room_num;
    int custom_background_last_room_num;
    uint32_t custom_background_applied_room_mask;
    CSB_V1_ViewportCustomBackgroundMask custom_background_layer_masks[3];
    uint8_t custom_background_layer_mask_valid[3];
    int custom_background_applied_count;
} CSB_V1_ViewportConfig;

typedef struct {
    CSB_V1_ViewportObjectSpriteDrawer object_sprite_drawer;
    void *object_sprite_user;
    int object_sprite_drawer_source_bound;
    CSB_V1_ViewportObjectIconDrawer object_icon_drawer;
    void *object_icon_user;
    CSB_V1_ViewportGroupSpriteDrawer group_sprite_drawer;
    void *group_sprite_user;
    int group_sprite_drawer_source_bound;
    CSB_V1_ViewportProjectileSpriteDrawer projectile_sprite_drawer;
    void *projectile_sprite_user;
    CSB_V1_ViewportExplosionSpriteDrawer explosion_sprite_drawer;
    void *explosion_sprite_user;
    int runtime_overlay_source_required;
    int runtime_overlay_source_admitted;
    uint32_t runtime_overlay_source_hash;
    int real_graphics_session;
    const CSB_V1_ViewportFirstFrameMaterialProof *first_frame_material_proof;
    const CSB_V1_ViewportFirstFrameMaterialBytesPc34 *first_frame_material_bytes;
    const char *first_frame_material_source_path;
    const char *first_frame_material_source_md5;
    DM1_ViewportGraphicProviderCallback graphic_provider_callback;
    void *graphic_provider_user_data;
} CSB_V1_ViewportRuntimeDrawerBinding;

typedef struct {
    int object_sprite_drawn_count;
    int object_icon_drawn_count;
    int object_marker_drawn_count;
    int group_sprite_drawn_count;
    int group_marker_drawn_count;
    int projectile_sprite_drawn_count;
    int projectile_material_resolved_count;
    int projectile_material_icon_drawn_count;
    int projectile_marker_drawn_count;
    int post_teleport_projectile_handoff_drawn_count;
    int post_teleport_projectile_handoff_blocked_count;
    int explosion_sprite_drawn_count;
    int explosion_marker_drawn_count;
    int real_asset_blocked_count;
    int first_frame_material_consumed_count;
    int first_frame_material_blocked_count;
    uint32_t first_frame_material_hash;
    int first_frame_draw_plan_consumed_count;
    int first_frame_draw_plan_blocked_count;
    int first_frame_draw_plan_command_count;
    uint32_t first_frame_draw_plan_hash;
    uint32_t first_frame_draw_plan_palette_hash;
    int first_frame_material_raster_consumed_count;
    int first_frame_material_raster_blocked_count;
    uint32_t first_frame_material_raster_hash;
} CSB_V1_ViewportRuntimeDrawCounts;

typedef struct {
    int room_num;
    int relative_forward;
    int relative_side;
    int call_order;
    int applies_before_cell_draw;
    int source_mask_slots;
    int source_bitmap_slots;
    int large_bitmap_min_bytes;
    int middle_bitmap_min_bytes;
    int near_bitmap_min_bytes;
    const char *csbwin_function;
    const char *source_lines;
} CSB_V1_ViewportCustomBackgroundSlotSpec;

typedef struct {
    const CSB_V1_ViewportCustomBackgroundSlotSpec *room_slot;
    int uses_csbwin_relative_translation;
    int checks_map_bounds_before_skin_lookup;
    int uses_cell_skin_before_default_skin;
    int skin_def_background_graphic_id;
    int skin_def_min_bytes;
    int large_bitmap_skin_def_index;
    int large_mask_skin_def_index;
    int large_mask_min_bytes;
    int large_bitmap_min_bytes;
    int middle_bitmap_skin_def_index;
    int middle_mask_skin_def_index;
    int middle_mask_min_bytes;
    int middle_bitmap_min_bytes;
    int near_bitmap_skin_def_index;
    int near_mask_skin_def_index;
    int near_mask_min_bytes;
    int near_bitmap_min_bytes;
    int applies_near_layer;
    int near_layer_room_num_limit;
    int selects_csd_i34_background_bitmap;
    int selects_redmcsb_base_bitmap;
    int extends_redmcsb_floor_ceiling_baseline;
    int applybackground_masked_composite;
    const char *csbwin_function;
    const char *source_lines;
} CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec;

typedef enum {
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L2 = 0,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D3C,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D3R,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D3R2,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D2L2,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D2L,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D2C,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D2R,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D2R2,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D1L,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D1C,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D1R,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D0L,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D0C,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D0R
} CSB_V1_ViewportCustomBackgroundViewIndex;

typedef struct {
    const uint8_t *bitmap;
    const uint8_t *mask;
    int byte_width;
    int height;
    int is_valid;
} CSB_V1_ViewportCustomBackgroundSelection;

typedef enum {
    CSB_V1_CUSTOM_BACKGROUND_LAYER_LARGE = 0,
    CSB_V1_CUSTOM_BACKGROUND_LAYER_MIDDLE = 1,
    CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR = 2
} CSB_V1_ViewportCustomBackgroundLayer;

typedef struct {
    CSB_V1_ViewportCustomBackgroundLayer layer;
    int bitmap_skin_def_index;
    int mask_skin_def_index;
    int bitmap_min_bytes;
    int mask_min_bytes;
    int applies_to_room;
} CSB_V1_ViewportCustomBackgroundLayerPlan;

typedef struct {
    int view_square;
    int wall_zone;
    int draws_wall_ornament;
    int ornament_ordinal_slot;
    int view_wall_index;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportWallOrnamentRouteSpec;

typedef struct {
    int view_square;
    int view_wall_index;
    int ordinal_zero_skips_blit;
    int ordinal_to_index_delta;
    int native_bitmap_index_increment;
    int zone_base;
    int coordinate_set_stride;
    int scale_x;
    int scale_y;
    int horizontal_flip;
    int transparent_color;
    int uses_scaled_bitmap;
    int uses_f0791_blit;
    const char *ornament_ordinal_slot;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportWallOrnamentBlitSpec;

typedef struct {
    int view_square;
    int view_wall_index;
    int evaluates_alcove_predicate;
    int updates_facing_alcove_state;
    int updates_facing_vi_altar_state;
    int updates_facing_fountain_state;
    int updates_wall_clickbox;
    int draws_champion_portrait_overlay;
    int uses_d3_i34_scaled_bitmap_path;
    int derived_bitmap_cache_slot;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportWallOrnamentSideEffectSpec;

typedef struct {
    int view_square;
    int view_wall_index;
    int ornament_ordinal_slot;
    int returns_alcove_to_square_draw;
    int uses_d2_scaled_bitmap_path;
    int uses_d1_native_bitmap_path;
    int native_bitmap_index_increment;
    int derived_bitmap_index_increment;
    int scale;
    int horizontal_flip;
    int updates_d1_front_state;
    int updates_wall_clickbox;
    int draws_champion_portrait_overlay;
    int zone_base;
    int coordinate_set_stride;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportWallOrnamentD1D2PathSpec;

typedef struct {
    int view_square;
    int floor_view_index;
    int draws_corridor_floor_ornament;
    int draws_pit_floor_ornament;
    int draws_door_front_floor_ornament;
    int door_front_pass1_order;
    int door_front_pass2_order;
    int door_zone;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportFloorOrnamentRouteSpec;

typedef struct {
    int view_square;
    int floor_view_index;
    int ordinal_zero_skips_blit;
    int ordinal_to_index_delta;
    int native_bitmap_index_increment;
    int coordinate_set_index;
    int zone_base;
    int coordinate_set_stride;
    int horizontal_flip;
    int transparent_color;
    const char *door_front_ordinal_slot;
    const char *corridor_pit_ordinal_slot;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportFloorOrnamentBlitSpec;

typedef struct {
    int view_square;
    int floor_view_index;
    int door_front_floor_ornament_order;
    int door_front_rear_f0115_order;
    int door_front_f0111_order;
    int door_front_front_f0115_order;
    uint16_t door_front_rear_cell_order;
    uint16_t door_front_front_cell_order;
    uint16_t corridor_cell_order;
    uint16_t side_cell_order;
    int f0115_objects_layer_order;
    int f0115_creatures_layer_order;
    int f0115_projectiles_layer_order;
    int f0115_projectile_g2028_row;
    int f0115_projectile_zone_base;
    int f0115_projectile_zone_stride;
    int f0115_projectile_restarts_thing_list;
    int f0115_projectile_requires_cell_match;
    int f0115_projectile_suppresses_depth3_front_cells;
    int f0115_explosions_layer_order;
    int f0115_explosions_after_all_cells;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportThingPassOrderSpec;

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int view_depth;
    int object_visibility_row;
    int requires_item_type_range;
    int requires_thing_cell_match;
    int suppresses_depth3_front_cells;
    int suppresses_depth0_back_cells;
    unsigned char first_visible_cell_ordinal;
    unsigned char last_visible_cell_ordinal;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportObjectVisibilitySpec;

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int view_depth;
    int object_visibility_row;
    int object_zone_base;
    int object_zone_cell_stride;
    int shifts_objects_and_creatures;
    int shift_set_index;
    int pile_shift_advances_per_object;
    int transparent_color;
    int uses_f0791_blit;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportObjectBlitSpec;

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int view_depth;
    int projectile_visibility_row;
    int projectile_zone_base;
    int projectile_zone_cell_stride;
    int requires_projectile_type;
    int requires_thing_cell_match;
    int restarts_thing_list;
    int suppresses_depth3_front_cells;
    int suppresses_depth0_back_cells;
    int derived_bitmap_cache_slot_for_scaled_path;
    int transparent_color;
    int uses_f0791_blit;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportProjectileBlitSpec;

struct CSB_V1_ViewportRuntimeProjectileOverlayPlacement {
    int visible;
    int forward;
    int side;
    int view_square;
    int view_cell;
    int source_zone;
    int viewport_x;
    int viewport_y;
    int source_zone_row;
    int sprite_x;
    int sprite_y;
    int sprite_w;
    int sprite_h;
    int sprite_aspect_index;
    int sprite_relative_dir;
    int sprite_relative_cell;
    int sprite_graphic_index;
    int sprite_flip_flags;
    int sprite_derived_bitmap_cache_slot;
    int sprite_transparent_color;
    int sprite_uses_f0791_blit;
    int material_thing;
    int material_icon_index;
};

struct CSB_V1_ViewportRuntimeProjectileSpriteBlit {
    int view_cell;
    int source_zone;
    int viewport_x;
    int viewport_y;
    int x;
    int y;
    int w;
    int h;
    int aspect_index;
    int graphic_index;
    int forward;
    int relative_dir;
    int relative_cell;
    int flip_flags;
    int source_zone_row;
    int derived_bitmap_cache_slot;
    int transparent_color;
    int uses_f0791_blit;
};

struct CSB_V1_ViewportRuntimeObjectOverlayPlacement {
    int visible;
    int forward;
    int side;
    int view_square;
    int view_cell;
    int object_row;
    int source_zone;
    int sprite_thing_type;
    int sprite_subtype_index;
    int sprite_relative_cell;
    int sprite_pile_index;
    int sprite_viewport_x;
    int sprite_viewport_y;
    int sprite_viewport_w;
    int sprite_viewport_h;
    int sprite_depth_index;
    int viewport_x;
    int viewport_y;
    int screen_x;
    int screen_y;
    int used_source_zone;
    int pile_index;
    int object_scale_index;
    int shift_set;
    int shift_x_index;
    int shift_y_index;
    int pile_shift_x;
    int pile_shift_y;
    int marker_screen_x;
    int marker_screen_y;
    int icon_index;
    int icon_screen_x;
    int icon_screen_y;
    int icon_draw_x;
    int icon_draw_y;
    int sprite_transparent_color;
    int sprite_uses_f0791_blit;
};

struct CSB_V1_ViewportRuntimeObjectSpriteBlit {
    int thing_type;
    int subtype_index;
    int relative_cell;
    int pile_index;
    int viewport_x;
    int viewport_y;
    int viewport_w;
    int viewport_h;
    int depth_index;
    int source_zone;
    int source_zone_row;
    int transparent_color;
    int uses_f0791_blit;
};

struct CSB_V1_ViewportRuntimeObjectIconBlit {
    int icon_index;
    int draw_x;
    int draw_y;
    int transparent_color;
};

struct CSB_V1_ViewportRuntimeGroupOverlayPlacement {
    int visible;
    int forward;
    int side;
    int view_square;
    int view_cell;
    int coordinate_set;
    int source_zone;
    int viewport_x;
    int viewport_y;
    int screen_x;
    int screen_y;
    int used_source_zone;
    int depth_index;
    int sprite_creature_type;
    int sprite_direction;
    int sprite_relative_side;
    int sprite_x;
    int sprite_y;
    int sprite_w;
    int sprite_h;
    int marker_screen_x;
    int marker_screen_y;
    int sprite_coordinate_set;
    int sprite_source_zone;
    int sprite_shift_mask;
    int sprite_transparent_color;
    int sprite_uses_f0791_blit;
};

struct CSB_V1_ViewportRuntimeGroupSpriteBlit {
    int creature_type;
    int direction;
    int relative_side;
    int x;
    int y;
    int w;
    int h;
    int depth_index;
    int coordinate_set;
    int source_zone;
    int shift_mask;
    int transparent_color;
    int uses_f0791_blit;
};

struct CSB_V1_ViewportRuntimeExplosionOverlayPlacement {
    int visible;
    int forward;
    int side;
    int view_square;
    int view_cell;
    int source_zone;
    int viewport_x;
    int viewport_y;
    int used_source_zone;
    int source_zone_row;
    int depth_index;
    int sprite_x;
    int sprite_y;
    int sprite_w;
    int sprite_h;
    int sprite_aspect_index;
    int sprite_graphic_index;
    int sprite_is_smoke;
    int sprite_frame;
    int sprite_max_frames;
    int sprite_attack;
    int sprite_transparent_color;
    int sprite_uses_f0791_blit;
};

struct CSB_V1_ViewportRuntimeExplosionSpriteBlit {
    int view_cell;
    int source_zone;
    int source_zone_row;
    int viewport_x;
    int viewport_y;
    int x;
    int y;
    int w;
    int h;
    int forward;
    int aspect_index;
    int graphic_index;
    int is_smoke;
    int frame;
    int max_frames;
    int attack;
    int depth_index;
    int transparent_color;
    int uses_f0791_blit;
};

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int view_depth;
    int creature_visibility_row;
    int requires_group_marker;
    int rejects_missing_creature_row;
    int creature_zone_base;
    int creature_coordinate_set_stride;
    int creature_zone_cell_stride;
    int shifts_objects_and_creatures;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportCreatureVisibilitySpec;

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int view_depth;
    int explosion_row;
    int field_aspect_index;
    int restarts_thing_list_after_cells;
    int rebirth_requires_visible_row_and_cell_match;
    int fluxcage_defers_to_field;
    int fluxcage_field_zone;
    int rebirth_step1_zone_base;
    int rebirth_step2_zone_base;
    int centered_zone_base;
    int side_zone_base;
    int side_zone_cell_stride;
    int transparent_color;
    int uses_f0791_blit;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportExplosionBlitSpec;

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int draws_only_for_teleporter;
    int after_thing_pass;
    int field_aspect_index;
    int field_zone;
    int uses_f0113_draw_field;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportTeleporterFieldSpec;

typedef struct {
    int view_square;
    int door_zone_base;
    int closed_record_type;
    int closed_parent_record;
    int closed_parent_x;
    int closed_parent_y;
    int clip_record;
    int native_bitmap_width;
    int native_bitmap_height;
    int clipped_width;
    int clipped_height;
    int closed_dst_x;
    int closed_dst_y;
    int door_ornament_index;
    int skips_open_state;
    int shifts_zone_by_state;
    int horizontal_first_half_zone_offset;
    int horizontal_second_half_zone_offset;
    int destroyed_state;
    int destroyed_mask_ornament_ordinal;
    int destroyed_mask_uses_view_ornament_index;
    int transparent_color;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportDoorPanelBlitSpec;

typedef struct {
    int forward;
    int side;
    int map_x;
    int map_y;
    int first_thing;
} CSB_V1_ViewportRuntimeOverlayCell;

typedef enum {
    CSB_V1_VIEWPORT_RUNTIME_OVERLAY_OBJECT = 1,
    CSB_V1_VIEWPORT_RUNTIME_OVERLAY_GROUP = 2
} CSB_V1_ViewportRuntimeThingOverlayKind;

typedef struct {
    CSB_V1_ViewportRuntimeThingOverlayKind kind;
    uint16_t thing;
    int forward;
    int side;
    int map_x;
    int map_y;
    int object_pile_index;
    int group_slot_index;
    int group_cell;
    CSB_V1_RuntimeObjectOverlayInfo object_info;
    CSB_V1_RuntimeGroupOverlayInfo group_info;
    CSB_V1_ViewportRuntimeObjectOverlayPlacement object_placement;
    CSB_V1_ViewportRuntimeGroupOverlayPlacement group_placement;
} CSB_V1_ViewportRuntimeThingOverlay;

void csb_v1_viewport_init(CSB_V1_ViewportConfig *cfg);
void csb_v1_viewport_set_wall_set(CSB_V1_ViewportConfig *cfg, int set);
void csb_v1_viewport_set_custom_background(CSB_V1_ViewportConfig *cfg, int bg_id);
void csb_v1_viewport_apply_runtime_drawer_binding(
    CSB_V1_ViewportConfig *cfg,
    const CSB_V1_ViewportRuntimeDrawerBinding *binding);
void csb_v1_viewport_runtime_draw_counts_reset(
    CSB_V1_ViewportRuntimeDrawCounts *counts);
void csb_v1_viewport_runtime_draw_counts_from_config(
    const CSB_V1_ViewportConfig *cfg,
    CSB_V1_ViewportRuntimeDrawCounts *counts);
int csb_v1_viewport_first_frame_material_proof_valid_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof);
int csb_v1_viewport_admit_first_frame_materialization_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof,
    int real_graphics_session,
    CSB_V1_ViewportFirstFrameMaterializationReceipt *out_receipt);
int csb_v1_viewport_build_first_frame_runtime_draw_plan_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof,
    int real_graphics_session,
    int party_dir,
    int party_x,
    int party_y,
    CSB_V1_ViewportRuntimeDrawPlanPc34 *out_plan);
int csb_v1_viewport_bind_first_frame_material_bytes_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof,
    CSB_V1_ViewportRuntimeDrawPlanPc34 *plan,
    const CSB_V1_ViewportFirstFrameMaterialBytesPc34 *bytes,
    CSB_V1_ViewportFirstFrameMaterializationReceipt *out_receipt);
int csb_v1_viewport_consume_first_frame_material_raster_pc34(
    const CSB_V1_ViewportFirstFrameMaterializationReceipt *receipt,
    const CSB_V1_ViewportRuntimeDrawPlanPc34 *plan,
    const CSB_V1_ViewportFirstFrameMaterialBytesPc34 *bytes,
    const char *expected_source_path,
    const char *expected_source_md5,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    CSB_V1_ViewportFirstFrameRasterReceiptPc34 *out_receipt);
int csb_v1_viewport_admit_live_frame_progression_pc34(
    CSB_V1_ViewportLiveFrameProgressionPc34 *progression,
    const CSB_V1_ViewportFirstFrameMaterializationReceipt *base_receipt,
    const CSB_V1_ViewportLiveFrameSourcePc34 *source,
    const char *expected_source_path,
    const char *expected_source_md5,
    CSB_V1_ViewportLiveFrameReceiptPc34 *out_receipt);
int csb_v1_viewport_consume_live_frame_raster_pc34(
    const CSB_V1_ViewportLiveFrameReceiptPc34 *receipt,
    const CSB_V1_ViewportLiveFrameSourcePc34 *source,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    uint32_t *out_raster_hash);
int csb_v1_viewport_materialize_live_frame_from_csbgraphics_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_ViewportLiveFrameDeclarationPc34 *declaration,
    CSB_V1_ViewportLiveFrameMaterialPc34 *out_material);
void csb_v1_viewport_live_frame_material_free_pc34(
    CSB_V1_ViewportLiveFrameMaterialPc34 *material);
int csb_v1_viewport_select_live_dungeon_state_pc34(
    const CSB_V1_ViewportLiveFrameDeclarationPc34 *declarations,
    size_t declaration_count,
    const CSB_V1_ViewportLiveDungeonStatePc34 *state,
    const CSB_V1_ViewportLiveDungeonSelectionPc34 *previous,
    CSB_V1_ViewportLiveDungeonSelectionPc34 *out_selection);
int csb_v1_viewport_live_dungeon_state_from_verified_ingress_pc34(
    const CSB_V1_ViewportVerifiedDungeonIngressPc34 *frame_receipt,
    const uint8_t *dungeon_grid,
    int dungeon_width,
    int dungeon_height,
    int square_x,
    int square_y,
    const CSB_V1_ViewportLiveDungeonStatePc34 *explicit_material_identity,
    CSB_V1_ViewportLiveDungeonStatePc34 *out_state);
int csb_v1_viewport_admit_operator_declaration_corpus_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_ViewportOperatorDeclarationCorpusPc34 *corpus);
int csb_v1_viewport_parse_operator_declaration_manifest_pc34(
    const char *text,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsDatPaletteSourceReceipt *palette_receipt,
    CSB_V1_ViewportOperatorDeclarationManifestPc34 *out_manifest);
void csb_v1_viewport_set_first_frame_material_proof_pc34(
    CSB_V1_ViewportConfig *cfg,
    const CSB_V1_ViewportFirstFrameMaterialProof *proof);
size_t csb_v1_viewport_custom_background_slot_spec_count(void);
const CSB_V1_ViewportCustomBackgroundSlotSpec *
csb_v1_viewport_get_custom_background_slot_spec(size_t index);
const CSB_V1_ViewportCustomBackgroundSlotSpec *
csb_v1_viewport_get_custom_background_slot_spec_for_room(int room_num);
size_t csb_v1_viewport_custom_background_bitmap_application_spec_count(void);
const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *
csb_v1_viewport_get_custom_background_bitmap_application_spec(size_t index);
const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *
csb_v1_viewport_get_custom_background_bitmap_application_spec_for_room(int room_num);
int csb_v1_viewport_custom_background_translate_cell(
    const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *spec,
    int party_x,
    int party_y,
    int facing,
    int *out_x,
    int *out_y);
CSB_V1_ViewportCustomBackgroundSelection
csb_v1_viewport_custom_background_load_and_select_pc34(
    const uint8_t *skin_def,
    size_t skin_def_size,
    CSB_V1_ViewportCustomBackgroundViewIndex view_index);
size_t csb_v1_viewport_custom_background_layer_plan_pc34(
    int room_num,
    CSB_V1_ViewportCustomBackgroundLayerPlan *out_layers,
    size_t out_capacity);
int csb_v1_viewport_custom_background_apply_aligned_mask_pc34(
    const CSB_V1_ViewportCustomBackgroundMask *mask,
    const uint32_t *bitmap_words,
    size_t bitmap_word_count,
    uint32_t *viewport_words,
    size_t viewport_word_count,
    int viewport_width_pixels);

/* Wire dungeon grid for element routing in back-wall rendering.
 * Must be called before render if using CSB back-wall squares (D3L2/D3R2).
 * Source: ReDMCSB DUNVIEW.C:6233 F0172_SetSquareAspect · dm1_viewport_3d_get_dungeon_element */
void csb_v1_viewport_set_dungeon_grid(CSB_V1_ViewportConfig *cfg,
                                       const uint8_t *grid,
                                       int width, int height);
int csb_v1_viewport_build_dungeon_grid(
    const CSB_V1_DungeonData *dungeon,
    int level,
    uint8_t out_grid[CSB_V1_MAX_SQUARE_SIZE * CSB_V1_MAX_SQUARE_SIZE]);

/* Bind one M11 viewport config to a live, loader-owned CSB dungeon grid.
 * A missing or invalid dungeon clears the grid binding and rejects the
 * frame; callers must not substitute procedural map data. */
int csb_v1_viewport_bind_live_dungeon_grid(
    CSB_V1_ViewportConfig *cfg,
    const CSB_V1_DungeonData *dungeon,
    int level,
    uint8_t out_grid[CSB_V1_MAX_SQUARE_SIZE * CSB_V1_MAX_SQUARE_SIZE]);

/* Render one dungeon view frame using the DM1 viewport engine.
 * party_dir: facing direction (0=N, 1=E, 2=S, 3=W)
 * party_x, party_y: position on dungeon grid
 *
 * Uses dm1_viewport_3d_draw_frame internally; CSB config
 * selects CSB-specific wall set and custom backgrounds.
 *
 * No-op when cfg->viewport_pixels is NULL.
 */
void csb_v1_viewport_render_frame(CSB_V1_ViewportConfig *cfg,
                                   int party_dir,
                                   int party_x,
                                   int party_y);

size_t csb_v1_viewport_wall_ornament_route_spec_count(void);
const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec(size_t index);
const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec_for_square(int view_square);

size_t csb_v1_viewport_wall_ornament_blit_spec_count(void);
const CSB_V1_ViewportWallOrnamentBlitSpec *csb_v1_viewport_get_wall_ornament_blit_spec(size_t index);
const CSB_V1_ViewportWallOrnamentBlitSpec *csb_v1_viewport_get_wall_ornament_blit_spec_for_square(int view_square);
int csb_v1_viewport_wall_ornament_blit_zone(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                            int coordinate_set);
int csb_v1_viewport_wall_ornament_native_bitmap_index(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                                      int base_native_bitmap_index);
int csb_v1_viewport_wall_ornament_blit_pixels(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                              const uint8_t *source,
                                              int source_stride,
                                              uint8_t *destination,
                                              int destination_stride,
                                              int width,
                                              int height);

size_t csb_v1_viewport_wall_ornament_side_effect_spec_count(void);
const CSB_V1_ViewportWallOrnamentSideEffectSpec *csb_v1_viewport_get_wall_ornament_side_effect_spec(size_t index);
const CSB_V1_ViewportWallOrnamentSideEffectSpec *csb_v1_viewport_get_wall_ornament_side_effect_spec_for_square(int view_square);

size_t csb_v1_viewport_wall_ornament_d1d2_path_spec_count(void);
const CSB_V1_ViewportWallOrnamentD1D2PathSpec *csb_v1_viewport_get_wall_ornament_d1d2_path_spec(size_t index);
int csb_v1_viewport_wall_ornament_d1d2_path_zone(
    const CSB_V1_ViewportWallOrnamentD1D2PathSpec *spec,
    int coordinate_set);

size_t csb_v1_viewport_floor_ornament_route_spec_count(void);
const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec(size_t index);
const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec_for_square(int view_square);

size_t csb_v1_viewport_floor_ornament_blit_spec_count(void);
const CSB_V1_ViewportFloorOrnamentBlitSpec *csb_v1_viewport_get_floor_ornament_blit_spec(size_t index);
const CSB_V1_ViewportFloorOrnamentBlitSpec *csb_v1_viewport_get_floor_ornament_blit_spec_for_square(int view_square);
int csb_v1_viewport_floor_ornament_blit_zone(const CSB_V1_ViewportFloorOrnamentBlitSpec *spec,
                                             int coordinate_set);
int csb_v1_viewport_floor_ornament_native_bitmap_index(const CSB_V1_ViewportFloorOrnamentBlitSpec *spec,
                                                       int base_native_bitmap_index);

size_t csb_v1_viewport_thing_pass_order_spec_count(void);
const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec(size_t index);
const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec_for_square(int view_square);

size_t csb_v1_viewport_object_visibility_spec_count(void);
const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec(size_t index);
const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec_for_square(int view_square);
int csb_v1_viewport_object_visibility_allows_cell(const CSB_V1_ViewportObjectVisibilitySpec *spec,
                                                  unsigned char cell_ordinal);

size_t csb_v1_viewport_object_blit_spec_count(void);
const CSB_V1_ViewportObjectBlitSpec *csb_v1_viewport_get_object_blit_spec(size_t index);
const CSB_V1_ViewportObjectBlitSpec *csb_v1_viewport_get_object_blit_spec_for_square(int view_square);
int csb_v1_viewport_object_blit_zone(const CSB_V1_ViewportObjectBlitSpec *spec,
                                     unsigned char view_cell);
int csb_v1_viewport_object_blit_layout_zone(const CSB_V1_ViewportObjectBlitSpec *spec,
                                            unsigned char view_cell);

size_t csb_v1_viewport_projectile_blit_spec_count(void);
const CSB_V1_ViewportProjectileBlitSpec *csb_v1_viewport_get_projectile_blit_spec(size_t index);
const CSB_V1_ViewportProjectileBlitSpec *csb_v1_viewport_get_projectile_blit_spec_for_square(int view_square);
int csb_v1_viewport_projectile_blit_zone(const CSB_V1_ViewportProjectileBlitSpec *spec,
                                         unsigned char view_cell);
int csb_v1_viewport_projectile_material_overlay_color(int material_icon_index);
int csb_v1_viewport_creature_marker_overlay_color(int creature_type);
int csb_v1_viewport_draw_runtime_object_marker(
    uint8_t *screen_pixels,
    int screen_stride,
    int screen_height,
    const CSB_V1_ViewportRuntimeObjectOverlayPlacement *placement,
    int material_icon_index);
int csb_v1_viewport_draw_runtime_group_marker(
    uint8_t *screen_pixels,
    int screen_stride,
    int screen_height,
    const CSB_V1_ViewportRuntimeGroupOverlayPlacement *placement,
    int creature_type);
int csb_v1_viewport_runtime_projectile_overlay_placement(
    int party_dir,
    int party_x,
    int party_y,
    int projectile_map_x,
    int projectile_map_y,
    int projectile_cell,
    CSB_V1_ViewportRuntimeProjectileOverlayPlacement *out_placement);
int csb_v1_viewport_runtime_bind_projectile_sprite(
    int party_dir,
    const struct ProjectileInstance_Compat *projectile,
    CSB_V1_ViewportRuntimeProjectileOverlayPlacement *placement);
int csb_v1_viewport_runtime_bind_projectile_material(
    const CSB_V1_RuntimeProfile *runtime,
    const struct ProjectileInstance_Compat *projectile,
    CSB_V1_ViewportRuntimeProjectileOverlayPlacement *placement);
int csb_v1_viewport_runtime_projectile_sprite_blit(
    const CSB_V1_ViewportRuntimeProjectileOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeProjectileSpriteBlit *out_blit);
int csb_v1_viewport_runtime_projectile_material_icon_blit(
    const CSB_V1_ViewportRuntimeProjectileOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeObjectIconBlit *out_blit);
int csb_v1_viewport_runtime_projectile_sprite_rect(
    int source_zone,
    int viewport_x,
    int viewport_y,
    int *out_source_zone_row,
    int *out_x,
    int *out_y,
    int *out_w,
    int *out_h);
int csb_v1_viewport_runtime_overlay_side_range(
    int forward,
    int *out_min_side,
    int *out_max_side);
void csb_v1_viewport_runtime_map_from_relative(
    int party_dir,
    int party_x,
    int party_y,
    int forward,
    int side,
    int *out_x,
    int *out_y);
int csb_v1_viewport_runtime_square_allows_thing_overlay(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int x,
    int y);
size_t csb_v1_viewport_runtime_collect_overlay_cells(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int party_dir,
    int party_x,
    int party_y,
    CSB_V1_ViewportRuntimeOverlayCell *out_cells,
    size_t out_capacity);
size_t csb_v1_viewport_runtime_collect_thing_overlays(
    const CSB_V1_RuntimeProfile *runtime,
    CSB_V1_ViewportRuntimeThingOverlay *out_overlays,
    size_t out_capacity);
int csb_v1_viewport_runtime_object_overlay_placement(
    int forward,
    int side,
    int relative_cell,
    CSB_V1_ViewportRuntimeObjectOverlayPlacement *out_placement);
int csb_v1_viewport_runtime_object_overlay_pile_placement(
    int forward,
    int side,
    int relative_cell,
    int pile_index,
    CSB_V1_ViewportRuntimeObjectOverlayPlacement *out_placement);
int csb_v1_viewport_runtime_object_sprite_blit(
    const CSB_V1_ViewportRuntimeObjectOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeObjectSpriteBlit *out_blit);
int csb_v1_viewport_runtime_object_source_zone_row(
    int source_zone,
    int fallback_source_zone_row);
int csb_v1_viewport_runtime_object_icon_blit(
    const CSB_V1_ViewportRuntimeObjectOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeObjectIconBlit *out_blit);
int csb_v1_viewport_runtime_group_overlay_placement(
    int forward,
    int side,
    int coordinate_set,
    CSB_V1_ViewportRuntimeGroupOverlayPlacement *out_placement);
int csb_v1_viewport_runtime_creature_coordinate_set(int creature_type);
int csb_v1_viewport_runtime_group_overlay_creature_placement(
    int forward,
    int side,
    int creature_type,
    int visible_count,
    int creature_cell,
    CSB_V1_ViewportRuntimeGroupOverlayPlacement *out_placement);
int csb_v1_viewport_runtime_group_overlay_slot_placement(
    int forward,
    int side,
    int coordinate_set,
    int visible_count,
    int slot_index,
    CSB_V1_ViewportRuntimeGroupOverlayPlacement *out_placement);
int csb_v1_viewport_runtime_group_sprite_blit(
    const CSB_V1_ViewportRuntimeGroupOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeGroupSpriteBlit *out_blit);
int csb_v1_viewport_runtime_explosion_overlay_placement(
    int party_dir,
    int party_x,
    int party_y,
    int explosion_map_x,
    int explosion_map_y,
    int explosion_cell,
    CSB_V1_ViewportRuntimeExplosionOverlayPlacement *out_placement);
int csb_v1_viewport_runtime_bind_explosion_sprite(
    const struct ExplosionInstance_Compat *explosion,
    CSB_V1_ViewportRuntimeExplosionOverlayPlacement *placement);
int csb_v1_viewport_runtime_explosion_sprite_blit(
    const CSB_V1_ViewportRuntimeExplosionOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeExplosionSpriteBlit *out_blit);
int csb_v1_viewport_runtime_explosion_sprite_rect(
    int forward,
    int source_zone,
    int viewport_x,
    int viewport_y,
    int *out_depth_index,
    int *out_x,
    int *out_y,
    int *out_w,
    int *out_h);
int csb_v1_viewport_projectile_blit_pixels(const CSB_V1_ViewportProjectileBlitSpec *spec,
                                           int flip_flags,
                                           const uint8_t *source,
                                           int source_stride,
                                           uint8_t *destination,
                                           int destination_stride,
                                           int width,
                                           int height);

size_t csb_v1_viewport_creature_visibility_spec_count(void);
const CSB_V1_ViewportCreatureVisibilitySpec *csb_v1_viewport_get_creature_visibility_spec(size_t index);
const CSB_V1_ViewportCreatureVisibilitySpec *csb_v1_viewport_get_creature_visibility_spec_for_square(int view_square);
int csb_v1_viewport_creature_visibility_zone(const CSB_V1_ViewportCreatureVisibilitySpec *spec,
                                             int coordinate_set,
                                             unsigned char view_cell);

size_t csb_v1_viewport_explosion_blit_spec_count(void);
const CSB_V1_ViewportExplosionBlitSpec *csb_v1_viewport_get_explosion_blit_spec(size_t index);
const CSB_V1_ViewportExplosionBlitSpec *csb_v1_viewport_get_explosion_blit_spec_for_square(int view_square);
int csb_v1_viewport_explosion_rebirth_step1_zone(const CSB_V1_ViewportExplosionBlitSpec *spec);
int csb_v1_viewport_explosion_rebirth_step2_zone(const CSB_V1_ViewportExplosionBlitSpec *spec);
int csb_v1_viewport_explosion_centered_zone(const CSB_V1_ViewportExplosionBlitSpec *spec);
int csb_v1_viewport_explosion_side_zone(const CSB_V1_ViewportExplosionBlitSpec *spec,
                                        unsigned char view_cell);

size_t csb_v1_viewport_teleporter_field_spec_count(void);
const CSB_V1_ViewportTeleporterFieldSpec *csb_v1_viewport_get_teleporter_field_spec(size_t index);
const CSB_V1_ViewportTeleporterFieldSpec *csb_v1_viewport_get_teleporter_field_spec_for_square(int view_square);

size_t csb_v1_viewport_door_panel_blit_spec_count(void);
const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec(size_t index);
const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec_for_square(int view_square);
int csb_v1_viewport_door_panel_first_half_zone(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                               int door_state,
                                               int horizontal_door);
int csb_v1_viewport_door_panel_final_zone(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                          int door_state,
                                          int horizontal_door);
int csb_v1_viewport_door_panel_blit_pixels(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                           int door_state,
                                           const uint8_t *source,
                                           int source_stride,
                                           uint8_t *destination,
                                           int destination_stride);

const char *csb_v1_viewport_source_evidence(void);

int csb_v1_viewport_near_wall_d2_wall_bitmap_index(int view_square, int use_flipped_wall_bitmaps);

int csb_v1_viewport_near_wall_d2_wall_zone(int view_square);

int csb_v1_viewport_near_wall_d2_wall_uses_flipped_blit( int view_square, int use_flipped_wall_bitmaps);

int csb_v1_viewport_f0115_object_native_graphic_pc34(
    int thingType, int subtype, int alcoveSlot);

int csb_v1_viewport_f0115_first_object_native_graphic_pc34(
    int thingType, int subtype);

int csb_v1_viewport_f0115_blit_first_object_native_family_pc34(
    int nativeGraphic,
    const uint8_t *sourcePixels,
    int sourceWidth,
    int sourceHeight,
    uint8_t *framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int framebufferStride,
    int drawX,
    int drawY,
    int drawW,
    int drawH,
    int depthOrdinal,
    int mirror);

int csb_v1_viewport_f0115_native_group_front_graphic_pc34(int creatureType);

int csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
    int creatureType, int nativeGraphic,
    const uint8_t *sourcePixels, int sourceWidth, int sourceHeight,
    uint8_t *framebuffer, int framebufferWidth, int framebufferHeight,
    int framebufferStride, int drawX, int drawY, int drawW, int drawH,
    int depthOrdinal, int zone, int mirror);

int csb_v1_viewport_f0115_blit_f0093_group_front_family_pc34(
    int creatureType, const struct DungeonMapDesc_Compat *map,
    const uint8_t *sourcePixels, int sourceWidth, int sourceHeight,
    uint8_t *framebuffer, int framebufferWidth, int framebufferHeight,
    int framebufferStride, int drawX, int drawY, int drawW, int drawH,
    int depthOrdinal, int zone, int mirror);

int csb_v1_viewport_f0115_blit_m715_m716_m717_m718_projectile_family_pc34(
    int graphicIndex,
    const uint8_t *sourcePixels, int sourceWidth, int sourceHeight,
    uint8_t *framebuffer, int framebufferWidth, int framebufferHeight,
    int framebufferStride, int drawX, int drawY, int drawW, int drawH,
    int depthOrdinal, int mirror);

#endif
