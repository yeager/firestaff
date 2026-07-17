#ifndef FIRESTAFF_DM2_V1_GDAT_WALL_B073_OUTPUT_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_WALL_B073_OUTPUT_RECEIPT_H
#include "dm2_v1_gdat_b073_input_receipt.h"
#include "dm2_v1_gdat_wall_m11_command.h"
typedef struct { int valid,no_draw; uint8_t command_index; const uint8_t *cache_palette_bytes; uint16_t cache_palette_bytes_count,cache_allocation; uint32_t raw7_identity,lookup_identity,traversal_identity,cache_identity,wall_hash,identity_hash; } DM2_V1_GdatWallB073OutputReceipt;
int dm2_v1_gdat_wall_b073_output_receipt_build(const DM2_V1_GdatB073InputReceipt*,const DM2_V1_GdatWallM11CommandPlan*,uint8_t,const uint8_t*,uint16_t,uint16_t,uint32_t,DM2_V1_GdatWallB073OutputReceipt*);
#endif
