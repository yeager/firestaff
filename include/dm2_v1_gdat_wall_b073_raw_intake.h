#ifndef FIRESTAFF_DM2_V1_GDAT_WALL_B073_RAW_INTAKE_H
#define FIRESTAFF_DM2_V1_GDAT_WALL_B073_RAW_INTAKE_H
#include "dm2_v1_gdat_wall_b073_output_receipt.h"
typedef struct { int valid,no_draw; uint8_t command_index; const uint8_t *raw7_bytes,*groups_bytes,*lookup_bytes; uint16_t raw7_size,groups_size,lookup_size,cache_allocation; uint32_t raw7_hash,groups_hash,lookup_hash,cache_identity,wall_hash,identity_hash; } DM2_V1_GdatWallB073RawIntake;
int dm2_v1_gdat_wall_b073_raw_intake_build(const DM2_V1_GdatB073InputReceipt*,const DM2_V1_GdatWallM11CommandPlan*,uint8_t,const uint8_t*,uint16_t,uint32_t,const uint8_t*,uint16_t,uint32_t,const uint8_t*,uint16_t,uint32_t,uint16_t,uint32_t,DM2_V1_GdatWallB073RawIntake*);
#endif
