#ifndef FIRESTAFF_DM2_V1_GDAT_STAIRS_SIDE_M11_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_STAIRS_SIDE_M11_RECEIPT_H

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_viewport_renderer.h"

typedef struct {
    int valid, no_draw;
    uint8_t view_cell, state_variant, graphicsset, field;
    uint16_t rect_number, light_parameter;
    uint32_t identity_hash;
} DM2_V1_GdatStairsSideSourceReceipt;

typedef struct {
    int valid, no_draw;
    DM2_V1_GdatStairsSideSourceReceipt source;
    DM2_V1_QueryGdatSummaryImageReceipt summary;
    DM2_V1_GdatGfxRawMaterialReceipt raw_material;
    const uint8_t *indexed_pixels;
    uint16_t width, height, pixel_stride;
    uint32_t indexed_pixel_count, decoded_hash, palette_hash, identity_hash;
    DM2_ImageFormat format;
    uint8_t palette16[16];
} DM2_V1_GdatStairsSideMaterialReceipt;

typedef struct {
    int valid, no_draw;
    uint16_t rect_number; int16_t destination_x, destination_y;
    uint16_t width, height;
    uint32_t material_identity_hash, raw4_table_hash, raw4_row_hash, identity_hash;
} DM2_V1_GdatStairsSideRaw4Receipt;

typedef struct {
    int valid, no_draw;
    const uint8_t *indexed_pixels;
    uint16_t width, height, pixel_stride;
    uint32_t indexed_pixel_count, palette_hash, material_identity_hash, raw4_identity_hash;
    uint32_t session_identity, data_epoch, composition_identity_hash, identity_hash;
    DM2_V1_ViewportSurfaceSnapshot surface_before, surface_after;
} DM2_V1_GdatStairsSideM11Receipt;

/* SKULLWIN/c_image.cpp:450-475 DRAW_DUNGEON_GRAPHIC hand-off. */
typedef struct {
    int valid, no_draw;
    uint8_t blit_mode, normal_scale, source_offset_is_zero;
    uint8_t palette_transaction_unproven;
    uint16_t alpha_mask;
    uint32_t material_identity_hash, raw4_identity_hash, m11_identity_hash;
    uint32_t identity_hash;
} DM2_V1_GdatStairsSideDungeonGraphicReceipt;

int dm2_v1_gdat_stairs_side_source_receipt(uint8_t cell, uint16_t state_word,
    uint8_t graphicsset, uint16_t light, DM2_V1_GdatStairsSideSourceReceipt *out);
int dm2_v1_gdat_stairs_side_material_receipt_build(const DM2_V1_AssetLoader *,
    const DM2_V1_GdatStairsSideSourceReceipt *, DM2_V1_GdatStairsSideMaterialReceipt *);
int dm2_v1_gdat_stairs_side_raw4_receipt_build(const DM2_V1_AssetLoader *,
    const DM2_V1_GdatStairsSideMaterialReceipt *, DM2_V1_GdatStairsSideRaw4Receipt *);
int dm2_v1_gdat_stairs_side_m11_receipt_build(const DM2_V1_GdatStairsSideMaterialReceipt *,
    const DM2_V1_GdatStairsSideRaw4Receipt *, const DM2_V1_Dm2ViewportM11CompositionReceipt *,
    const DM2_V1_ViewportState *, DM2_V1_GdatStairsSideM11Receipt *);
int dm2_v1_gdat_stairs_side_dungeon_graphic_receipt_build(
    const DM2_V1_GdatStairsSideMaterialReceipt *,
    const DM2_V1_GdatStairsSideRaw4Receipt *,
    const DM2_V1_GdatStairsSideM11Receipt *,
    DM2_V1_GdatStairsSideDungeonGraphicReceipt *);

#endif
