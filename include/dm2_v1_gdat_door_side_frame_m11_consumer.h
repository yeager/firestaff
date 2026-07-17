#ifndef FIRESTAFF_DM2_V1_GDAT_DOOR_SIDE_FRAME_M11_CONSUMER_H
#define FIRESTAFF_DM2_V1_GDAT_DOOR_SIDE_FRAME_M11_CONSUMER_H

#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_runtime.h"

/* DRAW_DOOR_FRAMES' stationary right-jamb QUERY_TEMP_PICST(1,...,3) route. */
typedef struct {
    int valid;
    uint8_t command_index;
    uint8_t kind;
    uint8_t movement_active;
    uint8_t scene_color_key;
    uint16_t rect_number;
    int16_t destination_x;
    int16_t destination_y;
    uint16_t width;
    uint16_t height;
    uint32_t door_hash;
    uint32_t scene_hash;
    uint32_t raw_hash;
    uint32_t decoded_hash;
    uint32_t palette_hash;
    uint32_t geometry_hash;
    uint32_t rect_table_hash;
    uint32_t rect_row_hash;
    uint32_t composition_identity_hash;
    uint32_t surface_generation;
    uint32_t identity_hash;
} DM2_V1_GdatDoorSideFrameM11Receipt;

int dm2_v1_gdat_door_side_frame_m11_receipt_build(
    const DM2_V1_GdatDoorOverlayM11CommandPlan *door_plan,
    uint8_t command_index,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatDoorSideFrameM11Receipt *out_receipt);
int dm2_v1_gdat_door_side_frame_m11_consume(
    const DM2_V1_GdatDoorSideFrameM11Receipt *receipt,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *door_plan,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    DM2_V1_ViewportState *owner);

#endif
