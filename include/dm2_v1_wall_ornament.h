#ifndef FIRESTAFF_DM2_V1_WALL_ORNAMENT_H
#define FIRESTAFF_DM2_V1_WALL_ORNAMENT_H

#include <stdint.h>

#include "dm2_v1_weather_gdat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_WALL_ORNATE_COLORKEY_FIELD          0x04u
#define DM2_V1_WALL_ORNATE_POSITION_FIELD          0x05u
#define DM2_V1_WALL_ORNATE_DO_NOT_FLIP_FIELD       0x07u
#define DM2_V1_WALL_ORNATE_ALCOVE_TYPE_FIELD       0x0au
#define DM2_V1_WALL_ORNATE_ITEM_DISPLACEMENT_FIELD 0xfdu
#define DM2_V1_WALL_ORNATE_FRONT_IMAGE_FIELD       0x01u
#define DM2_V1_WALL_ORNATE_ALCOVE_TYPE_LIMIT       0x03u

typedef struct {
    int valid;
    uint8_t wall_gfx_index;
    uint16_t alcove_type;
    uint32_t source_hash;
} DM2_V1_WallOrnateAlcoveTypeReceipt;

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

int dm2_v1_GET_WALL_ORNATE_ALCOVE_TYPE(
    const DM2_V1_AssetLoader *loader,
    uint8_t index,
    DM2_V1_WallOrnateAlcoveTypeReceipt *out);

int dm2_v1_wall_ornament_material_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t index,
    uint8_t image_field,
    DM2_V1_WallOrnamentReceipt *out);

int dm2_v1_wall_ornament_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t index,
    DM2_V1_WallOrnamentReceipt *out);

const char *dm2_v1_wall_ornament_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_WALL_ORNAMENT_H */
