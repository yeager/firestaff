#ifndef FIRESTAFF_DM2_V1_HAND_ACTION_GDAT_H
#define FIRESTAFF_DM2_V1_HAND_ACTION_GDAT_H

#include "dm2_v1_asset_loader.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* skproject/SKWINSPX/src/v4/skguidrw.cpp::DRAW_HAND_ACTION_ICONS
 * (0x29EE:026C) selects INTERFACE_GENERAL/4/dtImage entries 2..5 and then
 * expands the corresponding rect from the interface layout table. */
typedef struct {
    int possession_index;
    int left_or_right;
    int player_position;
    int party_direction;
} DM2_V1_HandActionInput;

typedef struct {
    uint8_t category;
    uint8_t subcategory;
    uint8_t entry;
    uint8_t rectno;
} DM2_V1_HandActionGdatRoute;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t bits_per_pixel;
    int16_t query_offset_x;
    int16_t query_offset_y;
    uint32_t metadata_hash;
} DM2_V1_HandActionImageMetadata;

/* Source-owned DRAW_ICON_PICT_ENTRY material for the non-dead champion
 * backdrop. The rect number remains an identity until QUERY_EXPANDED_RECT's
 * source table is bound by the HUD owner. */
typedef struct {
    int valid;
    DM2_V1_HandActionGdatRoute route;
    DM2_V1_HandActionImageMetadata image_metadata;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    uint16_t decoded_width;
    uint16_t decoded_height;
    DM2_ImageFormat decoded_format;
    uint32_t decoded_pixel_count;
    uint32_t decoded_pixels_hash;
    uint32_t material_hash;
} DM2_V1_HandActionGdatReceipt;

/* Resolve only valid source addresses.  Invalid input produces no route. */
int dm2_v1_hand_action_gdat_route(const DM2_V1_HandActionInput *input,
                                  DM2_V1_HandActionGdatRoute *out_route);

/* Resolve and decode the selected original GDAT image.  This function never
 * manufactures a hand-action bitmap: it returns NULL when the typed source
 * entry cannot be decoded. */
uint8_t *dm2_v1_hand_action_gdat_load_image(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_HandActionInput *input,
    DM2_V1_HandActionGdatRoute *out_route,
    int *out_width,
    int *out_height,
    DM2_ImageFormat *out_format);

/* Validate the complete original GDAT material selected by
 * DRAW_HAND_ACTION_ICONS. This does not create a fallback panel or choose an
 * alternate image when any source query fails. */
int dm2_v1_hand_action_gdat_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_HandActionInput *input,
    DM2_V1_HandActionGdatReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_HAND_ACTION_GDAT_H */
