#ifndef FIRESTAFF_DM2_V1_GDAT_DOOR_BUTTON_M11_CONSUMER_H
#define FIRESTAFF_DM2_V1_GDAT_DOOR_BUTTON_M11_CONSUMER_H

#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_runtime.h"

typedef struct {
    int valid;
    uint8_t command_index;
    uint8_t field;
    uint16_t rect_number;
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
    uint32_t door_hash, scene_hash, raw_hash, decoded_hash, palette_hash;
    uint32_t geometry_hash, table_hash, row_hash, composition_hash;
    uint32_t surface_generation, identity_hash;
} DM2_V1_GdatDoorButtonM11Receipt;

int dm2_v1_gdat_door_button_m11_receipt_build(
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan, uint8_t command_index,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner, DM2_V1_GdatDoorButtonM11Receipt *out);
int dm2_v1_gdat_door_button_m11_consume(
    const DM2_V1_GdatDoorButtonM11Receipt *receipt,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    DM2_V1_ViewportState *owner);

#endif
