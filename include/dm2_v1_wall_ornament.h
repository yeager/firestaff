#ifndef DM2_V1_WALL_ORNAMENT_H
#define DM2_V1_WALL_ORNAMENT_H

#include <stdint.h>
#include "dm2_v1_asset_loader.h"

typedef struct {
    int valid;
    uint8_t wall_gfx_index;
    uint16_t colorkey;
    uint16_t position;
    uint16_t do_not_flip;
    uint16_t alcove_type;
    uint16_t item_inside_displacement;
    uint8_t image_field;
    DM2_V1_GdatImageMetadata image_metadata;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    uint16_t decoded_width;
    uint16_t decoded_height;
    DM2_ImageFormat decoded_format;
    uint32_t decoded_pixel_count;
    uint32_t decoded_pixels_hash;
    uint32_t material_hash;
} DM2_V1_WallOrnamentReceipt;

/* Source: skproject DRAW_WALL_ORNATE. This binds only the original GDAT
 * scalar inputs; it deliberately performs no image lookup or rendering. */
int dm2_v1_wall_ornament_receipt(const DM2_V1_AssetLoader *loader,
                                 uint8_t wall_gfx_index,
                                 DM2_V1_WallOrnamentReceipt *out_receipt);

/* The caller provides the exact image field selected by DRAW_WALL_ORNATE
 * (front/side/flag-adjusted). No view-side fallback or replacement image is
 * selected here. */
int dm2_v1_wall_ornament_material_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t wall_gfx_index,
    uint8_t image_field,
    DM2_V1_WallOrnamentReceipt *out_receipt);
#endif
