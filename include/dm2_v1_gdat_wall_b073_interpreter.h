#ifndef FIRESTAFF_DM2_V1_GDAT_WALL_B073_INTERPRETER_H
#define FIRESTAFF_DM2_V1_GDAT_WALL_B073_INTERPRETER_H

#include "dm2_v1_gdat_wall_b073_raw7_loader.h"

typedef struct {
    int valid;
    int no_draw;
    uint8_t command_index;
    const uint8_t *cache_palette_bytes;
    uint16_t cache_palette_bytes_count;
    uint16_t cache_allocation;
    uint32_t input_cache_hash;
    uint32_t output_cache_hash;
    uint32_t raw7_hash;
    uint32_t cache_identity;
    uint32_t wall_hash;
    uint32_t identity_hash;
} DM2_V1_GdatWallB073InterpreterReceipt;

int dm2_v1_gdat_wall_b073_interpreter_build(
    const DM2_V1_GdatB073InputReceipt *input,
    const DM2_V1_GdatWallM11CommandPlan *wall_plan,
    uint8_t command_index,
    const DM2_V1_GdatWallB073Raw7LoaderReceipt *raw7,
    uint8_t *cache_palette_bytes,
    uint16_t cache_palette_bytes_count,
    uint16_t cache_allocation,
    uint32_t cache_identity,
    DM2_V1_GdatWallB073InterpreterReceipt *out_receipt,
    DM2_V1_GdatWallB073OutputReceipt *out_output);

#endif
