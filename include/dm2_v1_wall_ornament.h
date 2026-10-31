#ifndef FIRESTAFF_DM2_V1_WALL_ORNAMENT_H
#define FIRESTAFF_DM2_V1_WALL_ORNAMENT_H

#include <stdint.h>
#include <stddef.h>

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

/* Decoded skproject RectTable node from GDAT fmtRect (0x04). The raw table
 * starts with 0xfc0d, a range count, range bounds, then four little-endian
 * words per rectangle. Source: SKWINSPX/src/v4/skcore.cpp COMPRESS_RECTS /
 * QUERY_RECT and skbltgfx.cpp QUERY_BLIT_RECT. */
typedef struct {
    uint16_t rect_number;
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
} DM2_V1_RectTableNode;

typedef struct {
    int16_t destination_x;
    int16_t destination_y;
    int16_t destination_width;
    int16_t destination_height;
    int16_t source_x;
    int16_t source_y;
    int16_t source_width;
    int16_t source_height;
} DM2_V1_WallOrnamentBlitRect;

int dm2_v1_wall_ornament_rect_number(
    int source_cell, int lateral, unsigned position, uint16_t *out_rect_number);

int dm2_v1_wall_ornament_rect_table_query(
    const uint8_t *raw_rects, size_t raw_length, uint16_t rect_number,
    DM2_V1_RectTableNode *out);

/* Resolve skproject QUERY_BLIT_RECT for an already-decoded image size. */
int dm2_v1_wall_ornament_query_blit_rect(
    const uint8_t *raw_rects, size_t raw_length, uint16_t rect_number,
    int big_endian, int alignment_override, int image_width, int image_height,
    DM2_V1_WallOrnamentBlitRect *out);

int dm2_v1_wall_ornament_query_loaded_blit_rect(
    const DM2_V1_AssetLoader *loader, uint16_t rect_number,
    int alignment_override, int image_width, int image_height,
    DM2_V1_WallOrnamentBlitRect *out);

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
