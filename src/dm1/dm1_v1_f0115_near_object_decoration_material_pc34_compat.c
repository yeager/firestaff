#include "dm1_v1_f0115_near_object_decoration_material_pc34_compat.h"

#include "dm1_v1_floor_ornament_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_NEAR_DRAW_F0098_PC34 = 10,
    DM1_V1_NEAR_DRAW_F0108_PC34 = 20,
    DM1_V1_NEAR_DRAW_F0115_PC34 = 30,
    DM1_V1_NEAR_C10_PC34 = 10
};

static int find_material(const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
                         int materialCount, int graphicIndex,
                         const DM1_V1_FloorFeatureSourceMaterialPc34** out,
                         uint32_t* outHash)
{
    int i;
    if (out) *out = 0;
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
        if (!hash || hash != source->pixelsFNV1a) continue;
        if (out) *out = source;
        if (outHash) *outHash = hash;
        return 1;
    }
    return 0;
}

static void identity_palette(unsigned char palette[16])
{
    int i;
    for (i = 0; i < 16; ++i) palette[i] = (unsigned char)i;
}

int dm1_v1_f0115_near_dungeon_provenance_is_valid_pc34(
    const DM1_V1_F0115NearDungeonProvenancePc34* provenance)
{
    uint32_t hash;
    if (!provenance || !provenance->dungeonDatOwned || !provenance->rawBytes ||
        provenance->rawByteCount <= 0 || provenance->squareByteOffset < 0 ||
        provenance->squareByteOffset >= provenance->rawByteCount) return 0;
    hash = DM1_V1_FloorFeatureFNV1aPc34(provenance->rawBytes,
                                         provenance->rawByteCount);
    return hash && hash == provenance->rawBytesFNV1a &&
           provenance->rawBytes[provenance->squareByteOffset] ==
               provenance->squareByte &&
           ((provenance->squareByte >> 5) & 7) == 1;
}

int dm1_v1_f0115_near_object_decoration_material_receipt_pc34(
    const DM1_V1_F0115NearMaterialRequestPc34* request,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    const DM1_V1_F0115NearDungeonProvenancePc34* provenance,
    DM1_V1_F0115NearMaterialReceiptPc34* outReceipt)
{
    DM1_V1_F0115NearMaterialReceiptPc34 receipt;
    const DM1_V1_FloorFeatureSourceMaterialPc34* source;
    int graphicIndex;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!request || request->relForward < 0 || request->relForward > 1 ||
        request->relSide < -1 || request->relSide > 1 ||
        !dm1_v1_f0115_near_dungeon_provenance_is_valid_pc34(provenance)) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.kind = request->kind;
    receipt.cellOwner = -1;
    receipt.transparentColor = DM1_V1_NEAR_C10_PC34;
    receipt.dungeonBytesFNV1a = provenance->rawBytesFNV1a;
    receipt.dungeonSquareByteOffset = provenance->squareByteOffset;
    receipt.dungeonSquareByte = provenance->squareByte;
    identity_palette(receipt.paletteMap);

    switch (request->kind) {
    case DM1_V1_F0115_NEAR_FLOOR_PC34:
    case DM1_V1_F0115_NEAR_CEILING_PC34:
        if (request->floorSet < 0) return 0;
        graphicIndex = request->kind == DM1_V1_F0115_NEAR_FLOOR_PC34
            ? (int)dm1_floor_set_floor_graphic(request->floorSet)
            : (int)dm1_floor_set_ceiling_graphic(request->floorSet);
        if (!find_material(materials, materialCount, graphicIndex, &source,
                           &receipt.sourcePixelsFNV1a)) return 0;
        receipt.drawOrder = DM1_V1_NEAR_DRAW_F0098_PC34;
        receipt.graphicIndex = graphicIndex;
        receipt.cropW = source->width;
        receipt.cropH = source->height;
        receipt.dstX = request->viewportX;
        receipt.dstY = request->viewportY +
            (request->kind == DM1_V1_F0115_NEAR_FLOOR_PC34
                ? DM1_PC34_VIEWPORT_FLOOR_Y : 0);
        receipt.dstW = DM1_VIEWPORT_WIDTH;
        receipt.dstH = request->kind == DM1_V1_F0115_NEAR_FLOOR_PC34
            ? DM1_PC34_VIEWPORT_FLOOR_H : DM1_PC34_VIEWPORT_CEILING_H;
        receipt.transparentColor = -1;
        break;

    case DM1_V1_F0115_NEAR_FLOOR_ORNAMENT_PC34: {
        DM1_FloorOrnamentRenderPlanPc34 plan;
        if (request->relForward != 1 || request->floorOrnamentIndex < 0 ||
            !dm1_v1_floor_ornament_render_plan_pc34(
                request->relForward, request->relSide,
                request->floorOrnamentIndex, &plan) ||
            !find_material(materials, materialCount, plan.blit.graphicIndex,
                           &source, &receipt.sourcePixelsFNV1a)) return 0;
        receipt.drawOrder = DM1_V1_NEAR_DRAW_F0108_PC34;
        receipt.graphicIndex = plan.blit.graphicIndex;
        receipt.cropW = source->width;
        receipt.cropH = source->height;
        receipt.dstX = request->viewportX + plan.blit.dstX;
        receipt.dstY = request->viewportY + plan.blit.dstY;
        receipt.dstW = plan.blit.width;
        receipt.dstH = plan.blit.height;
        receipt.flipHorizontal = plan.flipHorizontal;
        break;
    }

    case DM1_V1_F0115_NEAR_NORMAL_OBJECT_PC34: {
        int sourceRow;
        int zoneX;
        int zoneY;
        int scaleIndex;
        int shiftSet;
        int shiftXIndex;
        int shiftYIndex;
        int shiftX;
        int shiftY;
        if (request->thingType < 5 || request->thingType > 10 ||
            request->sourceCellOwner < 0 || request->sourceCellOwner > 3 ||
            request->relForward == 0 || request->relSide < -1 ||
            request->relSide > 1) return 0;
        graphicIndex = (int)dm1_item_sprite_index(request->thingType,
                                                   request->subtype);
        sourceRow = dm1_viewport_3d_f0115_c2500_c2900_row(
            request->relForward, request->relSide);
        scaleIndex = dm1_viewport_3d_object_source_scale_index(
            request->relForward, request->sourceCellOwner);
        if (!find_material(materials, materialCount, graphicIndex, &source,
                           &receipt.sourcePixelsFNV1a) || sourceRow < 0 ||
            !dm1_viewport_3d_c2500_object_raw_zone_point(
                sourceRow, request->sourceCellOwner, &zoneX, &zoneY)) return 0;
        receipt.dstW = source->width *
            dm1_viewport_3d_object_source_scale_units(scaleIndex) / 32;
        receipt.dstH = source->height *
            dm1_viewport_3d_object_source_scale_units(scaleIndex) / 32;
        if (receipt.dstW < 1) receipt.dstW = 1;
        if (receipt.dstH < 1) receipt.dstH = 1;
        shiftSet = (scaleIndex + 1) >> 1;
        if (shiftSet > 2) shiftSet = 2;
        dm1_viewport_3d_object_pile_shift_indices(request->pileIndex,
                                                   &shiftXIndex, &shiftYIndex);
        shiftX = dm1_viewport_3d_object_source_shift_value(shiftSet, shiftXIndex);
        shiftY = dm1_viewport_3d_object_source_shift_value(shiftSet, shiftYIndex);
        receipt.drawOrder = DM1_V1_NEAR_DRAW_F0115_PC34;
        receipt.cellOwner = request->sourceCellOwner;
        receipt.graphicIndex = graphicIndex;
        receipt.cropW = source->width;
        receipt.cropH = source->height;
        receipt.dstX = request->viewportX + zoneX - receipt.dstW / 2 + shiftX;
        receipt.dstY = request->viewportY + zoneY - receipt.dstH + shiftY;
        receipt.flipHorizontal =
            (dm1_object_aspect_graphic_info(dm1_item_aspect_index(
                request->thingType, request->subtype)) & 1u) != 0 &&
            (request->sourceCellOwner == 1 || request->sourceCellOwner == 3);
        break;
    }

    default:
        return 0;
    }
    receipt.paletteFNV1a = DM1_V1_FloorFeatureFNV1aPc34(
        receipt.paletteMap, (int)sizeof(receipt.paletteMap));
    if (!receipt.sourcePixelsFNV1a || !receipt.paletteFNV1a ||
        receipt.cropW <= 0 || receipt.cropH <= 0 ||
        receipt.dstW <= 0 || receipt.dstH <= 0) return 0;
    receipt.valid = 1;
    *outReceipt = receipt;
    return 1;
}
