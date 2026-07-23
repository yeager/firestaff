#ifndef FIRESTAFF_DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_H
#define FIRESTAFF_DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_H

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_viewport_renderer.h"

#include <stddef.h>
#include <stdint.h>

typedef struct DM2_V1_DoorRenderPlan DM2_V1_DoorRenderPlan;

/* A visible source door can issue two panel halves plus ornate, destroyed,
 * centre-frame, button and two side-jamb commands. Keep the fixed M11
 * receipt large enough for every viewport square, rather than truncating a
 * valid multi-door source frame. */
#define DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_MAX 64

typedef enum {
    DM2_V1_GDAT_DOOR_OVERLAY_ORNATE = 1,
    DM2_V1_GDAT_DOOR_OVERLAY_DESTROYED_MASK = 2,
    DM2_V1_GDAT_DOOR_PANEL = 3,
    DM2_V1_GDAT_DOOR_FRAME = 4,
    DM2_V1_GDAT_DOOR_BUTTON = 5,
    /* SKProject DRAW_DOOR_FRAMES resolves these independently of the
     * centre frame through GRAPHICSSET plus QUERY_CREATURE_BLIT_RECTI. */
    DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT = 6,
    DM2_V1_GDAT_DOOR_SIDE_FRAME_RIGHT = 7,
    /* DRAW_DOOR_FRAMES' yy&1 ceiling slit is a separate GRAPHICSSET/RAW4
     * DRAW_DUNGEON_GRAPHIC transaction. */
    DM2_V1_GDAT_DOOR_ROOF_SLIT = 8
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
    uint8_t mirror_flip;
    /* skproject DRAW_DOOR's selected image and initial stretch/light
     * controls. These are source routing data, not synthesized shading. */
    uint8_t draw_distance;
    uint8_t stretch_dual;
    uint8_t light_palette;
    uint8_t movement_active;
    /* QUERY_TEMP_PICST applies _32cb_0804 after the source local palette is
     * loaded. Nonzero entries retain the c_light receipt that authorized the
     * dt07/2 remap; a base IMG3 palette is never reused as a dark retry. */
    uint8_t palette_darkness;
    uint32_t palette_light_receipt_hash;
    uint32_t palette_transform_hash;
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
    /* GFX16 and GFX256 must resolve this exact immutable GDAT interval. */
    uint16_t material_raw_index;
    const uint8_t *material_source_bytes;
    size_t material_source_byte_count;
    uint32_t raw_hash;
    uint32_t decoded_hash;
    uint32_t palette_hash;
    uint32_t material_receipt_hash;
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

/* Source RAW4 rect numbers for closed door panels and default buttons.
 * These are skproject tlbRectnoDoorPosition / tlbRectnoDoorButton values. */
int dm2_v1_gdat_door_overlay_panel_rect_number(int view_square,
                                               uint16_t *out_rect_number);
int dm2_v1_gdat_door_overlay_button_rect_number(int view_square,
                                                uint16_t *out_rect_number);
/* Query INTERFACE_GENERAL/0/RAW4/0 for a destination rectangle using the source
 * image's dimensions. This is the same RAW4 route DRAW_DOOR/DRAW_DOOR_FRAMES
 * uses; failure leaves the caller's fallback rectangle unchanged. */
int dm2_v1_gdat_door_overlay_query_raw4_destination_rect(
    const DM2_V1_AssetLoader *loader,
    uint16_t rect_number,
    int image_width,
    int image_height,
    DM2_V1_ViewportRect *out_rect);

int dm2_v1_gdat_door_overlay_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_DoorRenderPlan *door_plan,
    DM2_V1_GdatDoorOverlayM11CommandPlan *out_plan);
int dm2_v1_gdat_door_overlay_m11_command_plan_build_for_movement(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_DoorRenderPlan *door_plan,
    int movement_active,
    DM2_V1_GdatDoorOverlayM11CommandPlan *out_plan);
/* Verifies the DRAW_DOOR distance/image/stretch/light control tuple before
 * a source-owned M11 plan reaches the viewport. */
int dm2_v1_gdat_door_overlay_m11_command_plan_draw_controls_valid(
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan);
/* skproject _32cb_0804's stationary `_4976_4226[zz]` branch. `light` is
 * DISPLAY_VIEWPORT's authenticated `glbLightLevel * 10` parameter. */
int dm2_v1_gdat_door_light_palette_darkness(uint8_t light,
                                            uint8_t light_palette,
                                            uint8_t *out_darkness);
/* Recomputes the command receipt after a source-owned local-palette
 * transform. The plan remains invalid when any retained command is corrupt. */
int dm2_v1_gdat_door_overlay_m11_command_plan_refresh_hash(
    DM2_V1_GdatDoorOverlayM11CommandPlan *plan);
void dm2_v1_gdat_door_overlay_m11_command_plan_free(
    DM2_V1_GdatDoorOverlayM11CommandPlan *plan);

#endif
