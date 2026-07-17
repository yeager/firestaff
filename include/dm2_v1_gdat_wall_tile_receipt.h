#ifndef FIRESTAFF_DM2_V1_GDAT_WALL_TILE_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_WALL_TILE_RECEIPT_H
#include "dm2_v1_gdat_wall_m11_command.h"
#include "dm2_v1_runtime.h"
typedef struct { int valid,no_draw; uint8_t cell,table_branch,delegate_count; int8_t wall_orientation; uint32_t wall_hash,composition_hash,identity_hash; } DM2_V1_GdatWallTileReceipt;
int dm2_v1_gdat_wall_tile_receipt_build(uint8_t,const DM2_V1_GdatWallM11CommandPlan *,const DM2_V1_Dm2ViewportM11CompositionReceipt *,DM2_V1_GdatWallTileReceipt *);
#endif
