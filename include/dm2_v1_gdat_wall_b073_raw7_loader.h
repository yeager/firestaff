#ifndef FIRESTAFF_DM2_V1_GDAT_WALL_B073_RAW7_LOADER_H
#define FIRESTAFF_DM2_V1_GDAT_WALL_B073_RAW7_LOADER_H
#include "dm2_v1_gdat_wall_b073_raw_intake.h"
typedef struct { int valid,no_draw; const uint8_t *raw7; uint16_t raw7_size,cache_allocation; uint32_t raw7_hash,wall_hash,cache_identity,identity_hash; } DM2_V1_GdatWallB073Raw7LoaderReceipt;
int dm2_v1_gdat_wall_b073_raw7_loader_receipt_build(const DM2_V1_AssetLoader*,const DM2_V1_GdatB073InputReceipt*,const DM2_V1_GdatWallM11CommandPlan*,uint8_t,uint16_t,uint32_t,DM2_V1_GdatWallB073Raw7LoaderReceipt*);
#endif
