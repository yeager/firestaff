#ifndef FIRESTAFF_DM2_V1_GDAT_WALL_B073_M11_CONSUMER_H
#define FIRESTAFF_DM2_V1_GDAT_WALL_B073_M11_CONSUMER_H

#include "dm2_v1_gdat_wall_b073_interpreter.h"
#include "dm2_v1_gdat_wall_trim_receipt.h"

typedef struct {
    int valid;
    uint8_t command_index;
    uint8_t source_flip;
    uint8_t alpha_enabled;
    uint8_t alpha_index;
    uint16_t source_x;
    uint16_t source_y;
    uint16_t destination_x;
    uint16_t destination_y;
    uint16_t width;
    uint16_t height;
    uint32_t wall_hash;
    uint32_t interpreter_identity_hash;
    uint32_t output_identity_hash;
    uint32_t trim_identity_hash;
    uint32_t composition_identity_hash;
    uint32_t surface_generation;
    uint32_t cache_hash;
    uint32_t identity_hash;
} DM2_V1_GdatWallB073M11Receipt;

int dm2_v1_gdat_wall_b073_m11_receipt_build(
    const DM2_V1_GdatB073InputReceipt *input,
    const DM2_V1_GdatWallM11CommandPlan *wall_plan,
    uint8_t command_index,
    const DM2_V1_GdatWallB073InterpreterReceipt *interpreter,
    const DM2_V1_GdatWallB073OutputReceipt *output,
    const DM2_V1_GdatWallTrimReceipt *trim,
    const DM2_V1_GdatWallTrimM11Receipt *trim_m11,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatWallB073M11Receipt *out_receipt);

int dm2_v1_gdat_wall_b073_m11_consume(
    const DM2_V1_GdatWallB073M11Receipt *receipt,
    const DM2_V1_GdatB073InputReceipt *input,
    const DM2_V1_GdatWallM11CommandPlan *wall_plan,
    const DM2_V1_GdatWallB073InterpreterReceipt *interpreter,
    const DM2_V1_GdatWallB073OutputReceipt *output,
    const DM2_V1_GdatWallTrimReceipt *trim,
    const DM2_V1_GdatWallTrimM11Receipt *trim_m11,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    DM2_V1_ViewportState *owner);

#endif
