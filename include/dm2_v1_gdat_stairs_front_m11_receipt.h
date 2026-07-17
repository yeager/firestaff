#ifndef FIRESTAFF_DM2_V1_GDAT_STAIRS_FRONT_M11_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_STAIRS_FRONT_M11_RECEIPT_H

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdint.h>

/* Bounded SKULLWIN/c_gui_vp.cpp:468-528 source selector. */
typedef struct {
    int valid;
    int no_draw;
    uint8_t view_cell;
    uint8_t state_variant;
    uint8_t graphicsset;
    uint8_t field;
    uint16_t rect_number;
    uint16_t light_parameter;
    uint32_t identity_hash;
} DM2_V1_GdatStairsFrontSourceReceipt;

/* The non-loadable primary-image fallback at c_gui_vp.cpp:514-527. */
typedef struct {
    int valid;
    int no_draw;
    uint8_t view_cell;
    uint8_t state_variant;
    uint8_t graphicsset;
    uint8_t field;
    uint16_t rect_number;
    uint16_t light_parameter;
    uint8_t blit_mode;
    uint8_t normal_scale;
    uint8_t palette_transaction_unproven;
    uint32_t identity_hash;
} DM2_V1_GdatStairsFrontFallbackReceipt;

typedef struct {
    int valid;
    int no_draw;
    DM2_V1_GdatStairsFrontSourceReceipt source;
    DM2_V1_QueryGdatSummaryImageReceipt summary;
    DM2_V1_GdatGfxRawMaterialReceipt raw_material;
    const uint8_t *indexed_pixels;
    uint16_t width;
    uint16_t height;
    uint16_t pixel_stride;
    uint32_t indexed_pixel_count;
    DM2_ImageFormat format;
    uint8_t palette16[16];
    uint32_t decoded_hash;
    uint32_t palette_hash;
    uint32_t identity_hash;
} DM2_V1_GdatStairsFrontMaterialReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint16_t rect_number;
    int16_t destination_x;
    int16_t destination_y;
    uint16_t width;
    uint16_t height;
    uint32_t material_identity_hash;
    uint32_t raw4_table_hash;
    uint32_t raw4_row_hash;
    uint32_t identity_hash;
} DM2_V1_GdatStairsFrontRaw4Receipt;

/* M11 ownership record.  It carries the exact borrowed U4 bytes and the
 * live owner surface, but grants no pixel write: DRAW_DUNGEON_GRAPHIC's
 * B073/DRAW_PICST transform is not admitted by this package. */
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
    uint32_t raw4_identity_hash;
    uint32_t session_identity;
    uint32_t data_epoch;
    uint32_t composition_identity_hash;
    DM2_V1_ViewportSurfaceSnapshot surface_before;
    DM2_V1_ViewportSurfaceSnapshot surface_after;
    uint32_t identity_hash;
} DM2_V1_GdatStairsFrontM11Receipt;

typedef struct {
    int valid;
    int no_draw;
    uint8_t blit_mode;
    uint8_t scale_x;
    uint8_t scale_y;
    int16_t offset_x;
    int16_t offset_y;
    int16_t palette_mode;
    int16_t palette_arg;
    uint16_t alpha_mask;
    uint8_t palette_transaction_unproven;
    uint32_t fallback_identity_hash;
    uint32_t material_identity_hash;
    uint32_t raw4_identity_hash;
    uint32_t m11_identity_hash;
    uint32_t identity_hash;
} DM2_V1_GdatStairsFrontFallbackTempPicstReceipt;

int dm2_v1_gdat_stairs_front_source_receipt(uint8_t view_cell,
    uint16_t state_word, uint8_t graphicsset, uint16_t light_parameter,
    int gdat_loadable, DM2_V1_GdatStairsFrontSourceReceipt *out_receipt);
int dm2_v1_gdat_stairs_front_fallback_receipt(uint8_t view_cell,
    uint16_t state_word, uint8_t graphicsset, uint16_t light_parameter,
    int primary_gdat_loadable, DM2_V1_GdatStairsFrontFallbackReceipt *out_receipt);
int dm2_v1_gdat_stairs_front_material_receipt_build(const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatStairsFrontSourceReceipt *source,
    DM2_V1_GdatStairsFrontMaterialReceipt *out_receipt);
int dm2_v1_gdat_stairs_front_fallback_material_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatStairsFrontFallbackReceipt *fallback,
    DM2_V1_GdatStairsFrontMaterialReceipt *out_receipt);
int dm2_v1_gdat_stairs_front_raw4_receipt_build(const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatStairsFrontMaterialReceipt *material,
    DM2_V1_GdatStairsFrontRaw4Receipt *out_receipt);
int dm2_v1_gdat_stairs_front_m11_receipt_build(
    const DM2_V1_GdatStairsFrontMaterialReceipt *material,
    const DM2_V1_GdatStairsFrontRaw4Receipt *raw4,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatStairsFrontM11Receipt *out_receipt);
int dm2_v1_gdat_stairs_front_m11_receipt_matches(
    const DM2_V1_GdatStairsFrontM11Receipt *receipt,
    const DM2_V1_GdatStairsFrontMaterialReceipt *material,
    const DM2_V1_GdatStairsFrontRaw4Receipt *raw4,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner);
int dm2_v1_gdat_stairs_front_fallback_temp_picst_receipt_build(
    const DM2_V1_GdatStairsFrontFallbackReceipt *fallback,
    const DM2_V1_GdatStairsFrontMaterialReceipt *material,
    const DM2_V1_GdatStairsFrontRaw4Receipt *raw4,
    const DM2_V1_GdatStairsFrontM11Receipt *m11,
    DM2_V1_GdatStairsFrontFallbackTempPicstReceipt *out_receipt);

#endif
