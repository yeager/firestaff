#include "dm1_v1_viewport_wall_field_material_pc34_compat.h"

#include "firestaff/dm1/v1/palette_dungeon_view_pc34_compat.h"

#include <string.h>

enum { DM1_V1_ELEMENT_TELEPORTER_PC34 = 5 };

static int find_material(const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
                         int materialCount, int graphicIndex,
                         uint32_t* outHash)
{
    int i;
    if (outHash) *outHash = 0u;
    if (!materials || materialCount <= 0 || graphicIndex < 0) return 0;
    for (i = 0; i < materialCount; ++i) {
        const DM1_V1_FloorFeatureSourceMaterialPc34* source = &materials[i];
        uint32_t hash;
        if (!source->graphicsDatOwned || source->graphicIndex != graphicIndex ||
            !source->indexedPixels || source->width <= 0 || source->height <= 0 ||
            source->indexedPixelCount < source->width * source->height) continue;
        hash = DM1_V1_FloorFeatureFNV1aPc34(source->indexedPixels,
                                             source->indexedPixelCount);
        if (hash == 0u || hash != source->pixelsFNV1a) continue;
        if (outHash) *outHash = hash;
        return 1;
    }
    return 0;
}

int dm1_v1_viewport_dungeon_provenance_is_valid_pc34(
    const DM1_V1_ViewportDungeonProvenancePc34* provenance)
{
    uint32_t hash;
    if (!provenance || !provenance->dungeonDatOwned || !provenance->rawBytes ||
        provenance->rawByteCount <= 0 || provenance->squareByteOffset < 0 ||
        provenance->squareByteOffset >= provenance->rawByteCount) return 0;
    hash = DM1_V1_FloorFeatureFNV1aPc34(provenance->rawBytes,
                                         provenance->rawByteCount);
    return hash != 0u && hash == provenance->rawBytesFNV1a &&
           provenance->rawBytes[provenance->squareByteOffset] ==
               provenance->squareByte;
}

static int begin_receipt(const DM1_V1_ViewportDungeonProvenancePc34* provenance,
                         int paletteIndex,
                         DM1_V1_ViewportWallFieldMaterialReceiptPc34* receipt)
{
    int i;
    if (!receipt || !dm1_v1_viewport_dungeon_provenance_is_valid_pc34(provenance) ||
        paletteIndex < 0 || paletteIndex >= dm1_v1_palette_dungeon_view_rows_pc34()) {
        return 0;
    }
    memset(receipt, 0, sizeof(*receipt));
    for (i = 0; i < 16; ++i) {
        receipt->paletteMap[i] = (unsigned char)i;
        receipt->palette[i] = (unsigned short)dm1_v1_palette_dungeon_view_pc34(
            paletteIndex, i);
    }
    receipt->paletteIndex = paletteIndex;
    receipt->paletteMapFNV1a = DM1_V1_FloorFeatureFNV1aPc34(
        receipt->paletteMap, (int)sizeof(receipt->paletteMap));
    receipt->paletteFNV1a = DM1_V1_FloorFeatureFNV1aPc34(
        (const unsigned char*)receipt->palette, (int)sizeof(receipt->palette));
    receipt->dungeonBytesFNV1a = provenance->rawBytesFNV1a;
    receipt->dungeonSquareByteOffset = provenance->squareByteOffset;
    receipt->dungeonSquareByte = provenance->squareByte;
    return receipt->paletteMapFNV1a != 0u && receipt->paletteFNV1a != 0u;
}

int dm1_v1_viewport_wall_original_material_receipt_pc34(
    const DM1_ViewportSideWallHostReceiptPc34* wall,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    const DM1_V1_ViewportDungeonProvenancePc34* provenance,
    int paletteIndex,
    DM1_V1_ViewportWallFieldMaterialReceiptPc34* outReceipt)
{
    DM1_V1_ViewportWallFieldMaterialReceiptPc34 receipt;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!wall || !wall->handled || !wall->draw_wall || !wall->material.valid ||
        wall->material.transparent_color != 10 || wall->width <= 0 ||
        wall->height <= 0 || !begin_receipt(provenance, paletteIndex, &receipt) ||
        !find_material(materials, materialCount, wall->material.graphic_index,
                       &receipt.sourcePixelsFNV1a)) return 0;
    receipt.valid = 1;
    receipt.drawPhase = DM1_V1_VIEWPORT_DRAW_PHASE_WALL_PC34;
    receipt.graphicIndex = wall->material.graphic_index;
    receipt.dstX = wall->dst_x;
    receipt.dstY = wall->dst_y;
    receipt.width = wall->width;
    receipt.height = wall->height;
    receipt.transparentColor = wall->material.transparent_color;
    receipt.flipHorizontal = wall->material.flip_horizontally ? 1 : 0;
    *outReceipt = receipt;
    return 1;
}

int dm1_v1_viewport_wall_ornament_original_material_receipt_pc34(
    int globalOrnamentIndex, int viewWallIndex, int maxHeight,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials, int materialCount,
    const DM1_V1_ViewportDungeonProvenancePc34* provenance, int paletteIndex,
    DM1_V1_ViewportWallFieldMaterialReceiptPc34* outReceipt)
{
    DM1_WallOrnamentRenderPlanPc34 plan;
    DM1_V1_ViewportWallFieldMaterialReceiptPc34 receipt;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    /* Global ornament 0 is the inscription lane and is intentionally excluded. */
    if (globalOrnamentIndex <= 0 ||
        !dm1_v1_wall_ornament_render_plan_pc34(globalOrnamentIndex,
                                                viewWallIndex, maxHeight, &plan) ||
        !plan.paletteMapValid || plan.transparentColor != 10 || plan.width <= 0 ||
        plan.height <= 0 || !begin_receipt(provenance, paletteIndex, &receipt) ||
        !find_material(materials, materialCount, plan.graphicIndex,
                       &receipt.sourcePixelsFNV1a)) return 0;
    receipt.valid = 1;
    receipt.drawPhase = DM1_V1_VIEWPORT_DRAW_PHASE_WALL_ORNAMENT_PC34;
    receipt.graphicIndex = plan.graphicIndex;
    receipt.srcX = plan.srcX;
    receipt.srcY = plan.srcY;
    receipt.dstX = plan.dstX;
    receipt.dstY = plan.dstY;
    receipt.width = plan.width;
    receipt.height = plan.height;
    receipt.transparentColor = plan.transparentColor;
    receipt.flipHorizontal = plan.flipHorizontal;
    memcpy(receipt.paletteMap, plan.paletteMap, sizeof(receipt.paletteMap));
    receipt.paletteMapFNV1a = DM1_V1_FloorFeatureFNV1aPc34(
        receipt.paletteMap, (int)sizeof(receipt.paletteMap));
    if (receipt.paletteMapFNV1a == 0u) return 0;
    *outReceipt = receipt;
    return 1;
}

int dm1_v1_viewport_field_original_material_receipt_pc34(
    const DM1_FieldRenderPlanPc34* fieldPlan,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    const DM1_V1_ViewportDungeonProvenancePc34* provenance,
    int paletteIndex,
    DM1_V1_ViewportWallFieldMaterialReceiptPc34* outReceipt)
{
    DM1_FieldAssetBindingPc34 binding;
    DM1_V1_ViewportWallFieldMaterialReceiptPc34 receipt;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!fieldPlan || !dm1_v1_field_square_is_visible_open_pc34(
            provenance ? provenance->squareByte : 0) ||
        !dm1_v1_viewport_dungeon_provenance_is_valid_pc34(provenance) ||
        ((provenance->squareByte >> 5) & 7) != DM1_V1_ELEMENT_TELEPORTER_PC34 ||
        !dm1_v1_field_asset_binding_pc34(fieldPlan, &binding) ||
        !begin_receipt(provenance, paletteIndex, &receipt) ||
        !find_material(materials, materialCount, binding.fieldGraphicIndex,
                       &receipt.sourcePixelsFNV1a) ||
        (binding.maskRequired && !find_material(materials, materialCount,
                                                binding.maskGraphicIndex,
                                                &receipt.auxiliaryPixelsFNV1a))) {
        return 0;
    }
    receipt.valid = 1;
    receipt.drawPhase = DM1_V1_VIEWPORT_DRAW_PHASE_FIELD_AFTER_THINGS_PC34;
    receipt.graphicIndex = binding.fieldGraphicIndex;
    receipt.auxiliaryGraphicIndex = binding.maskRequired ? binding.maskGraphicIndex : -1;
    receipt.dstX = fieldPlan->dstX;
    receipt.dstY = fieldPlan->dstY;
    receipt.width = fieldPlan->dstW;
    receipt.height = fieldPlan->dstH;
    receipt.transparentColor = binding.transparentColor;
    receipt.flipHorizontal = binding.maskFlip;
    *outReceipt = receipt;
    return 1;
}
