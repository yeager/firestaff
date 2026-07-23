#ifndef FIRESTAFF_DM1_V1_F0115_F0219_CREATURE_ITEM_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0115_F0219_CREATURE_ITEM_MATERIAL_PC34_COMPAT_H

/* Source-only F0115/F0219-adjacent creature/item admission. This excludes
 * walls, ornaments, fields, doors, inscriptions and C14/C15 effects. */

#include "dm1_v1_floor_feature_material_pc34_compat.h"

#include <stdint.h>

typedef enum DM1_V1_F0115F0219MaterialKindPc34 {
    DM1_V1_F0115_F0219_MATERIAL_ITEM_PC34 = 1,
    DM1_V1_F0115_F0219_MATERIAL_CREATURE_PC34 = 2
} DM1_V1_F0115F0219MaterialKindPc34;

typedef struct DM1_V1_F0115F0219DungeonProvenancePc34 {
    int dungeonDatOwned;
    const unsigned char* rawBytes;
    int rawByteCount;
    uint32_t rawBytesFNV1a;
    int squareByteOffset;
    unsigned char squareByte;
} DM1_V1_F0115F0219DungeonProvenancePc34;

typedef struct DM1_V1_F0115F0219MaterialRequestPc34 {
    int kind;
    int relForward;              /* D1..D3 only */
    int relativeCell;            /* ReDMCSB C00..C03 view cell */
    int thingType;
    int subtype;
    int pileIndex;
    int creatureType;
    int creatureDirection;
    int partyDirection;
    int groupCount;
    int groupIndex;
    int creatureCount;
    int duplicateIndex;
    int viewportX;
    int viewportY;
    int viewportW;
    int viewportH;
    int sourceCellOwner;         /* must equal relativeCell */
} DM1_V1_F0115F0219MaterialRequestPc34;

typedef struct DM1_V1_F0115F0219MaterialReceiptPc34 {
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
} DM1_V1_F0115F0219MaterialReceiptPc34;

int dm1_v1_f0115_f0219_dungeon_provenance_is_valid_pc34(
    const DM1_V1_F0115F0219DungeonProvenancePc34* provenance);

int dm1_v1_f0115_f0219_creature_item_material_receipt_pc34(
    const DM1_V1_F0115F0219MaterialRequestPc34* request,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    const DM1_V1_F0115F0219DungeonProvenancePc34* provenance,
    DM1_V1_F0115F0219MaterialReceiptPc34* outReceipt);

#endif /* FIRESTAFF_DM1_V1_F0115_F0219_CREATURE_ITEM_MATERIAL_PC34_COMPAT_H */
