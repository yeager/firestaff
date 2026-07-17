#ifndef FIRESTAFF_DM2_V1_GDAT_WALL_TILE_LOADABLE_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_WALL_TILE_LOADABLE_RECEIPT_H
#include <stdint.h>
typedef struct { uint8_t category,selector,image_field,scale_x,scale_y,flip; uint16_t query1,query2,query3,alpha; int loadable,active; uint32_t source_identity; } DM2_V1_GdatWallTileLoadableInput;
typedef struct { int valid,no_draw; DM2_V1_GdatWallTileLoadableInput input; uint8_t alpha_forced; uint32_t identity_hash; } DM2_V1_GdatWallTileLoadableReceipt;
int dm2_v1_gdat_wall_tile_loadable_receipt_build(const DM2_V1_GdatWallTileLoadableInput *,DM2_V1_GdatWallTileLoadableReceipt *);
#endif
