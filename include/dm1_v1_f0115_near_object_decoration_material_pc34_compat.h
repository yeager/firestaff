#ifndef FIRESTAFF_DM1_V1_F0115_NEAR_OBJECT_DECORATION_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0115_NEAR_OBJECT_DECORATION_MATERIAL_PC34_COMPAT_H

/* Source-only D0/D1 floor/ceiling, F0108 decoration, and normal-object
 * material admission. ReDMCSB DUNVIEW.C F0094/F0098/F0108/F0115 owns this
 * order. C14/C15 are intentionally not representable here. */

#include "dm1_v1_floor_feature_material_pc34_compat.h"

#include <stdint.h>

typedef enum DM1_V1_F0115NearMaterialKindPc34 {
    DM1_V1_F0115_NEAR_FLOOR_PC34 = 1,
    DM1_V1_F0115_NEAR_CEILING_PC34,
    DM1_V1_F0115_NEAR_FLOOR_ORNAMENT_PC34,
    DM1_V1_F0115_NEAR_NORMAL_OBJECT_PC34
} DM1_V1_F0115NearMaterialKindPc34;

typedef struct DM1_V1_F0115NearDungeonProvenancePc34 {
    int dungeonDatOwned;
    const unsigned char* rawBytes;
    int rawByteCount;
    uint32_t rawBytesFNV1a;
    int squareByteOffset;
    unsigned char squareByte;
} DM1_V1_F0115NearDungeonProvenancePc34;

typedef struct DM1_V1_F0115NearMaterialRequestPc34 {
    int kind;
    int relForward;       /* D0 or D1 only */
    int relSide;          /* -1, 0, +1 */
    int sourceCellOwner;  /* C00..C03 for normal objects */
    int floorSet;
    int floorOrnamentIndex;
    int thingType;        /* only source C05..C10 normal objects */
    int subtype;
    int pileIndex;
    int viewportX;
    int viewportY;
} DM1_V1_F0115NearMaterialRequestPc34;

typedef struct DM1_V1_F0115NearMaterialReceiptPc34 {
    int valid;
    int kind;
    int drawOrder;
    int cellOwner;
    int graphicIndex;
    int cropX;
    int cropY;
    int cropW;
    int cropH;
    int dstX;
    int dstY;
    int dstW;
    int dstH;
    int transparentColor;
    int flipHorizontal;
    unsigned char paletteMap[16];
    uint32_t sourcePixelsFNV1a;
    uint32_t paletteFNV1a;
    uint32_t dungeonBytesFNV1a;
    int dungeonSquareByteOffset;
    unsigned char dungeonSquareByte;
} DM1_V1_F0115NearMaterialReceiptPc34;

int dm1_v1_f0115_near_dungeon_provenance_is_valid_pc34(
    const DM1_V1_F0115NearDungeonProvenancePc34* provenance);

/* Requires an owned, hash-stable decoded PC34 GRAPHICS.DAT surface and a
 * hash-stable raw corridor byte from DUNGEON.DAT. No caller data becomes a
 * substitute bitmap when either source is absent. */
int dm1_v1_f0115_near_object_decoration_material_receipt_pc34(
    const DM1_V1_F0115NearMaterialRequestPc34* request,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    const DM1_V1_F0115NearDungeonProvenancePc34* provenance,
    DM1_V1_F0115NearMaterialReceiptPc34* outReceipt);

#endif /* FIRESTAFF_DM1_V1_F0115_NEAR_OBJECT_DECORATION_MATERIAL_PC34_COMPAT_H */
