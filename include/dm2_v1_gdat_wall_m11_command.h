#ifndef FIRESTAFF_DM2_V1_GDAT_WALL_M11_COMMAND_H
#define FIRESTAFF_DM2_V1_GDAT_WALL_M11_COMMAND_H

#include "dm2_v1_asset_loader.h"

#include <stddef.h>
#include <stdint.h>

#define DM2_V1_GDAT_WALL_M11_COMMAND_MAX 12

typedef struct {
    uint8_t view_square;
    uint8_t field;
    uint8_t *pixels;
    uint16_t width;
    uint16_t height;
    uint8_t palette16[16];
    uint32_t raw_hash;
    uint32_t decoded_hash;
    uint32_t palette_hash;
    uint16_t material_raw_index;
    const uint8_t *material_source_bytes;
    size_t material_source_byte_count;
    uint32_t material_receipt_hash;
    /* G0163's source crop and destination panel are part of the command,
     * rather than renderer-local defaults. */
    uint16_t source_x;
    uint16_t source_y;
    uint16_t source_width;
    uint16_t source_height;
    uint16_t destination_x;
    uint16_t destination_y;
    uint16_t destination_width;
    uint16_t destination_height;
    /* skproject DM2_DRAW_WALL sends QUERY_TEMP_PICST the viewport-cell
     * RAW4 rectangle (0x2be + cell).  Keep the raw ownership and the exact
     * QUERY_BLIT_RECT result with the image command, not in M11 defaults. */
    uint16_t rect_number;
    uint8_t mirror_flip;
    uint8_t movement_active;
    int8_t movement_query_offset_y;
    uint32_t rect_table_hash;
    uint32_t rect_row_hash;
    uint32_t metadata_hash;
    uint32_t geometry_hash;
} DM2_V1_GdatWallM11Command;

typedef struct DM2_V1_GdatWallM11CommandPlan {
    int valid;
    uint8_t graphicsset;
    uint8_t command_count;
    uint32_t command_hash;
    DM2_V1_GdatWallM11Command commands[DM2_V1_GDAT_WALL_M11_COMMAND_MAX];
} DM2_V1_GdatWallM11CommandPlan;

int dm2_v1_gdat_wall_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    DM2_V1_GdatWallM11CommandPlan *out_plan);
int dm2_v1_gdat_wall_m11_command_plan_build_for_movement(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    int movement_active, DM2_V1_GdatWallM11CommandPlan *out_plan);
void dm2_v1_gdat_wall_m11_command_plan_free(
    DM2_V1_GdatWallM11CommandPlan *plan);

#endif
