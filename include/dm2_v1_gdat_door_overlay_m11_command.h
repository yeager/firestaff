#ifndef FIRESTAFF_DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_H
#define FIRESTAFF_DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_H

#include "dm2_v1_asset_loader.h"

#include <stdint.h>

typedef struct DM2_V1_DoorRenderPlan DM2_V1_DoorRenderPlan;

#define DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_MAX 20

typedef enum {
    DM2_V1_GDAT_DOOR_OVERLAY_ORNATE = 1,
    DM2_V1_GDAT_DOOR_OVERLAY_DESTROYED_MASK = 2,
    DM2_V1_GDAT_DOOR_PANEL = 3,
    DM2_V1_GDAT_DOOR_FRAME = 4,
    DM2_V1_GDAT_DOOR_BUTTON = 5
} DM2_V1_GdatDoorOverlayKind;

typedef struct {
    int gdat_index;
    uint8_t view_square;
    uint8_t kind;
    uint8_t category;
    uint8_t entry_index;
    uint8_t field;
    uint8_t *pixels;
    uint16_t width;
    uint16_t height;
    uint8_t door_opening_dir;
    uint8_t door_state;
    uint8_t door_open_pct;
    /* skproject DRAW_DOOR's selected image and initial stretch/light
     * controls. These are source routing data, not synthesized shading. */
    uint8_t draw_distance;
    uint8_t stretch_dual;
    uint8_t light_palette;
    uint16_t color_key;
    uint16_t no_frames;
    /* skproject DRAW_DOOR passes tlbRectnoDoorPosition[cell] through
     * QUERY_BLIT_RECT.  These are the resolved RAW4 destination coordinates
     * for the closed-panel route, never viewport approximations. */
    uint16_t rect_number;
    int16_t rect_x;
    int16_t rect_y;
    uint16_t rect_width;
    uint16_t rect_height;
    /* DRAW_DOOR halves the source bitmap for horizontal opening states before
     * submitting its left/right RAW4 destinations. */
    uint16_t source_x;
    uint16_t source_y;
    uint16_t source_width;
    uint16_t source_height;
    uint8_t palette16[16];
    uint32_t raw_hash;
    uint32_t decoded_hash;
    uint32_t palette_hash;
    uint32_t rect_table_hash;
    uint32_t rect_row_hash;
    uint32_t geometry_hash;
    uint32_t selection_hash;
} DM2_V1_GdatDoorOverlayM11Command;

typedef struct DM2_V1_GdatDoorOverlayM11CommandPlan {
    int valid;
    uint8_t command_count;
    uint32_t command_hash;
    DM2_V1_GdatDoorOverlayM11Command commands[
        DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_MAX];
} DM2_V1_GdatDoorOverlayM11CommandPlan;

int dm2_v1_gdat_door_overlay_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_DoorRenderPlan *door_plan,
    DM2_V1_GdatDoorOverlayM11CommandPlan *out_plan);
void dm2_v1_gdat_door_overlay_m11_command_plan_free(
    DM2_V1_GdatDoorOverlayM11CommandPlan *plan);

#endif
