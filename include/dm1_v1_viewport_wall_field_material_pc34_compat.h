#ifndef FIRESTAFF_DM1_V1_VIEWPORT_WALL_FIELD_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_WALL_FIELD_MATERIAL_PC34_COMPAT_H

/* Source-only F0110/F0112/F0113-adjacent viewport material admission.
 * The adapter owns no framebuffer and deliberately excludes door, C10/C11,
 * inscription, F0111 and F0114 paths. */

#include "dm1_v1_floor_feature_material_pc34_compat.h"
#include "dm1_v1_field_teleporter_effect_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_wall_ornament_pc34_compat.h"

#include <stdint.h>

typedef struct DM1_V1_ViewportDungeonProvenancePc34 {
    int dungeonDatOwned;
    const unsigned char* rawBytes;
    int rawByteCount;
    uint32_t rawBytesFNV1a;
    int squareByteOffset;
    unsigned char squareByte;
} DM1_V1_ViewportDungeonProvenancePc34;

typedef enum DM1_V1_ViewportWallFieldDrawPhasePc34 {
    DM1_V1_VIEWPORT_DRAW_PHASE_WALL_PC34 = 10,
    DM1_V1_VIEWPORT_DRAW_PHASE_WALL_ORNAMENT_PC34 = 20,
    DM1_V1_VIEWPORT_DRAW_PHASE_FIELD_AFTER_THINGS_PC34 = 40
} DM1_V1_ViewportWallFieldDrawPhasePc34;

typedef struct DM1_V1_ViewportWallFieldMaterialReceiptPc34 {
    int valid;
    int drawPhase;
    int graphicIndex;
    int auxiliaryGraphicIndex;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
    int transparentColor;
    int flipHorizontal;
    int paletteIndex;
    unsigned char paletteMap[16];
    unsigned short palette[16];
    uint32_t sourcePixelsFNV1a;
    uint32_t auxiliaryPixelsFNV1a;
    uint32_t paletteMapFNV1a;
    uint32_t paletteFNV1a;
    uint32_t dungeonBytesFNV1a;
    int dungeonSquareByteOffset;
    unsigned char dungeonSquareByte;
} DM1_V1_ViewportWallFieldMaterialReceiptPc34;

int dm1_v1_viewport_dungeon_provenance_is_valid_pc34(
    const DM1_V1_ViewportDungeonProvenancePc34* provenance);

int dm1_v1_viewport_wall_original_material_receipt_pc34(
    const DM1_ViewportSideWallHostReceiptPc34* wall,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    const DM1_V1_ViewportDungeonProvenancePc34* provenance,
    int paletteIndex,
    DM1_V1_ViewportWallFieldMaterialReceiptPc34* outReceipt);

int dm1_v1_viewport_wall_ornament_original_material_receipt_pc34(
    int globalOrnamentIndex,
    int viewWallIndex,
    int maxHeight,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    const DM1_V1_ViewportDungeonProvenancePc34* provenance,
    int paletteIndex,
    DM1_V1_ViewportWallFieldMaterialReceiptPc34* outReceipt);

int dm1_v1_viewport_field_original_material_receipt_pc34(
    const DM1_FieldRenderPlanPc34* fieldPlan,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    const DM1_V1_ViewportDungeonProvenancePc34* provenance,
    int paletteIndex,
    DM1_V1_ViewportWallFieldMaterialReceiptPc34* outReceipt);

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_WALL_FIELD_MATERIAL_PC34_COMPAT_H */
