#ifndef FIRESTAFF_DM2_V1_GDAT_PIT_ROOF_M11_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_PIT_ROOF_M11_RECEIPT_H

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_runtime.h"

#include <stdint.h>

typedef struct {
    uint8_t roof_enabled;
    uint8_t locate_other_level_succeeded;
    uint8_t remote_tile_type;
    uint8_t remote_tile_bit_08;
    uint32_t source_state_hash;
} DM2_V1_GdatPitRoofSourceState;

typedef struct {
    int valid;
    int no_draw;
    uint8_t view_cell;
    uint8_t graphicsset;
    uint8_t field;
    uint16_t rect_number;
    uint16_t light_parameter;
    uint8_t mirror_flip;
    uint32_t source_state_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitRoofTransformReceipt;

typedef struct {
    int valid;
    int no_draw;
    DM2_V1_GdatPitRoofTransformReceipt transform;
    DM2_V1_QueryGdatSummaryImageReceipt summary;
    DM2_V1_GdatGfxRawMaterialReceipt raw_material;
    uint16_t width;
    uint16_t height;
    uint16_t pixel_stride;
    const uint8_t *indexed_pixels;
    uint32_t indexed_pixel_count;
    DM2_ImageFormat format;
    uint8_t palette16[16];
    uint32_t decoded_hash;
    uint32_t palette_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitRoofM11Receipt;

/* Borrowed decoded U4 storage.  This is an identity handoff only; it never
 * decodes, copies, or draws pixels. */
typedef struct {
    int valid;
    int no_draw;
    const uint8_t *indexed_pixels;
    uint16_t width;
    uint16_t height;
    uint16_t pixel_stride;
    uint32_t indexed_pixel_count;
    uint32_t palette_hash;
    uint32_t material_identity_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitRoofMaterialHandoffReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint8_t light_level;
    uint16_t alphamask;
    uint32_t material_identity_hash;
    uint32_t c_light_receipt_hash;
    uint32_t local_palette_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitRoofB073Receipt;

typedef struct {
    int valid;
    int no_draw;
    uint16_t rect_number;
    uint16_t source_x;
    uint16_t source_y;
    int16_t destination_x;
    int16_t destination_y;
    uint16_t width;
    uint16_t height;
    uint32_t material_identity_hash;
    uint32_t table_hash;
    uint32_t row_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitRoofRaw4Receipt;

typedef struct {
    int valid;
    int no_draw;
    uint16_t alphamask;
    uint8_t alpha_index;
    uint8_t blit_mode;
    uint8_t mirror_flip;
    uint32_t material_identity_hash;
    uint32_t b073_identity_hash;
    uint32_t raw4_identity_hash;
    uint32_t palette_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitRoofAlphaBlendReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint8_t table_count;
    uint16_t table_data_bytes;
    uint16_t color_lookup_bytes;
    uint32_t material_identity_hash;
    uint32_t b073_identity_hash;
    uint32_t raw7_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitRoofB073TablesReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint8_t colors;
    uint8_t transformed_palette16[16];
    uint32_t material_identity_hash;
    uint32_t b073_identity_hash;
    uint32_t tables_identity_hash;
    uint32_t input_palette_hash;
    uint32_t transformed_palette_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitRoofB073TraversalReceipt;

typedef struct {
    int valid;
    int no_draw;
    int16_t destination_x;
    int16_t destination_y;
    uint16_t source_width;
    uint16_t source_height;
    uint16_t scale_x;
    uint16_t scale_y;
    uint8_t blit_mode;
    uint8_t alpha_index;
    uint32_t material_identity_hash;
    uint32_t raw4_identity_hash;
    uint32_t traversal_identity_hash;
    uint32_t alpha_blend_identity_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitRoofDestinationReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint16_t bitmap_width;
    uint16_t bitmap_height;
    uint8_t bitmap_resolution;
    int16_t clip_x;
    int16_t clip_y;
    uint16_t clip_width;
    uint16_t clip_height;
    uint32_t surface_identity_hash;
    uint32_t destination_identity_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitRoofSurfaceClipReceipt;

typedef struct { int valid; int no_draw; uint8_t *framebuffer; uint32_t surface_receipt_hash; uint32_t generation; uint32_t identity_hash; } DM2_V1_GdatPitRoofSurfaceBindingReceipt;

int dm2_v1_gdat_pit_roof_transform_receipt(
    uint8_t graphicsset, uint8_t view_cell, uint16_t light_parameter,
    const DM2_V1_GdatPitRoofSourceState *source,
    DM2_V1_GdatPitRoofTransformReceipt *out_receipt);
int dm2_v1_gdat_pit_roof_m11_receipt_build(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset, uint8_t view_cell,
    uint16_t light_parameter, const DM2_V1_GdatPitRoofSourceState *source,
    DM2_V1_GdatPitRoofM11Receipt *out_receipt);
int dm2_v1_gdat_pit_roof_material_handoff_receipt_build(
    const DM2_V1_GdatPitRoofM11Receipt *material,
    DM2_V1_GdatPitRoofMaterialHandoffReceipt *out_receipt);
int dm2_v1_gdat_pit_roof_b073_receipt_build(
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_CLightM11Receipt *light,
    DM2_V1_GdatPitRoofB073Receipt *out_receipt);
int dm2_v1_gdat_pit_roof_raw4_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitRoofM11Receipt *material,
    DM2_V1_GdatPitRoofRaw4Receipt *out_receipt);
int dm2_v1_gdat_pit_roof_no_draw_admission(
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_CLightM11Receipt *light,
    const DM2_V1_GdatPitRoofB073Receipt *b073,
    const DM2_V1_GdatPitRoofRaw4Receipt *raw4);
int dm2_v1_gdat_pit_roof_alpha_blend_receipt_build(
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofB073Receipt *b073,
    const DM2_V1_GdatPitRoofRaw4Receipt *raw4,
    uint16_t alphamask, uint8_t blit_mode,
    DM2_V1_GdatPitRoofAlphaBlendReceipt *out_receipt);
int dm2_v1_gdat_pit_roof_b073_tables_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofB073Receipt *b073,
    DM2_V1_GdatPitRoofB073TablesReceipt *out_receipt);
int dm2_v1_gdat_pit_roof_b073_traversal_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofB073Receipt *b073,
    const DM2_V1_GdatPitRoofB073TablesReceipt *tables,
    DM2_V1_GdatPitRoofB073TraversalReceipt *out_receipt);
int dm2_v1_gdat_pit_roof_destination_receipt_build(
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofRaw4Receipt *raw4,
    const DM2_V1_GdatPitRoofAlphaBlendReceipt *alpha_blend,
    const DM2_V1_GdatPitRoofB073TraversalReceipt *traversal,
    uint16_t scale_x, uint16_t scale_y, int16_t source_x, int16_t source_y,
    uint8_t blit_mode, DM2_V1_GdatPitRoofDestinationReceipt *out_receipt);
int dm2_v1_gdat_pit_roof_surface_clip_receipt_build(
    const DM2_V1_GdatPitRoofDestinationReceipt *destination,
    uint16_t bitmap_width, uint16_t bitmap_height, uint8_t bitmap_resolution,
    uint32_t surface_identity_hash, int16_t clip_x, int16_t clip_y,
    uint16_t clip_width, uint16_t clip_height,
    DM2_V1_GdatPitRoofSurfaceClipReceipt *out_receipt);
int dm2_v1_gdat_pit_roof_surface_binding_receipt_build(
    const DM2_V1_GdatPitRoofSurfaceClipReceipt *surface,
    const DM2_V1_ViewportSurfaceSnapshot *snapshot,
    DM2_V1_GdatPitRoofSurfaceBindingReceipt *out_receipt);
int dm2_v1_gdat_pit_roof_composition_surface_matches(
    const DM2_V1_GdatPitRoofSurfaceBindingReceipt *binding,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofMaterialHandoffReceipt *handoff);
int dm2_v1_gdat_pit_roof_consume_ordered_m11(
    const DM2_V1_GdatPitRoofSurfaceBindingReceipt *binding,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRoofRaw4Receipt *raw4,
    const DM2_V1_GdatPitRoofAlphaBlendReceipt *alpha_blend,
    const DM2_V1_GdatPitRoofB073TraversalReceipt *traversal,
    const DM2_V1_GdatPitRoofDestinationReceipt *destination,
    const DM2_V1_GdatPitRoofSurfaceClipReceipt *surface);

#endif
