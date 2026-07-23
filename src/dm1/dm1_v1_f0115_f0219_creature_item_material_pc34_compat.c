#include "dm1_v1_f0115_f0219_creature_item_material_pc34_compat.h"

#include "dm1_v1_creature_render_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_F0115_DRAW_ORDER_ITEM_PC34 = 10,
    DM1_V1_F0115_DRAW_ORDER_CREATURE_PC34 = 20,
    DM1_V1_F0115_C10_PC34 = 10
};

static int find_material(const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
                         int materialCount, int graphicIndex,
                         const DM1_V1_FloorFeatureSourceMaterialPc34** outSource,
                         uint32_t* outHash)
{
    int i;
    if (outSource) *outSource = 0;
    if (outHash) *outHash = 0u;
    if (!materials || materialCount <= 0 || graphicIndex <= 0) return 0;
    for (i = 0; i < materialCount; ++i) {
        const DM1_V1_FloorFeatureSourceMaterialPc34* source = &materials[i];
        uint32_t hash;
        if (!source->graphicsDatOwned || source->graphicIndex != graphicIndex ||
            !source->indexedPixels || source->width <= 0 || source->height <= 0 ||
            source->indexedPixelCount < source->width * source->height) continue;
        hash = DM1_V1_FloorFeatureFNV1aPc34(source->indexedPixels,
                                             source->indexedPixelCount);
        if (!hash || hash != source->pixelsFNV1a) continue;
        if (outSource) *outSource = source;
        if (outHash) *outHash = hash;
        return 1;
    }
    return 0;
}

int dm1_v1_f0115_f0219_dungeon_provenance_is_valid_pc34(
    const DM1_V1_F0115F0219DungeonProvenancePc34* provenance)
{
    uint32_t hash;
    if (!provenance || !provenance->dungeonDatOwned || !provenance->rawBytes ||
        provenance->rawByteCount <= 0 || provenance->squareByteOffset < 0 ||
        provenance->squareByteOffset >= provenance->rawByteCount) return 0;
    hash = DM1_V1_FloorFeatureFNV1aPc34(provenance->rawBytes,
                                         provenance->rawByteCount);
    return hash && hash == provenance->rawBytesFNV1a &&
           provenance->rawBytes[provenance->squareByteOffset] ==
               provenance->squareByte;
}

static void palette_for_depth(int depth, unsigned char outPalette[16])
{
    const unsigned char* source = 0;
    int i;
    if (depth == 3) source = dm1_creature_palette_d3();
    else if (depth == 2) source = dm1_creature_palette_d2();
    for (i = 0; i < 16; ++i) outPalette[i] = source ? source[i] : (unsigned char)i;
}

int dm1_v1_f0115_f0219_creature_item_material_receipt_pc34(
    const DM1_V1_F0115F0219MaterialRequestPc34* request,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    const DM1_V1_F0115F0219DungeonProvenancePc34* provenance,
    DM1_V1_F0115F0219MaterialReceiptPc34* outReceipt)
{
    DM1_V1_F0115F0219MaterialReceiptPc34 receipt;
    const DM1_V1_FloorFeatureSourceMaterialPc34* source;
    int expectedGraphic;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!request || (request->kind != DM1_V1_F0115_F0219_MATERIAL_ITEM_PC34 &&
                     request->kind != DM1_V1_F0115_F0219_MATERIAL_CREATURE_PC34) ||
        request->relForward < 1 || request->relForward > 3 ||
        request->relativeCell < 0 || request->relativeCell > 3 ||
        request->sourceCellOwner != request->relativeCell ||
        request->viewportW <= 0 || request->viewportH <= 0 ||
        !dm1_v1_f0115_f0219_dungeon_provenance_is_valid_pc34(provenance) ||
        ((provenance->squareByte >> 5) & 7) != 1) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.kind = request->kind;
    receipt.cellOwner = request->sourceCellOwner;
    receipt.transparentColor = DM1_V1_F0115_C10_PC34;
    receipt.dungeonBytesFNV1a = provenance->rawBytesFNV1a;
    receipt.dungeonSquareByteOffset = provenance->squareByteOffset;
    receipt.dungeonSquareByte = provenance->squareByte;

    if (request->kind == DM1_V1_F0115_F0219_MATERIAL_ITEM_PC34) {
        int sourceRow;
        int zoneX;
        int zoneY;
        int scaleIndex;
        int shiftSet;
        int shiftX;
        int shiftY;
        int shiftXIndex;
        int shiftYIndex;
        expectedGraphic = (int)dm1_item_sprite_index(request->thingType,
                                                      request->subtype);
        if (!find_material(materials, materialCount, expectedGraphic, &source,
                           &receipt.sourcePixelsFNV1a)) return 0;
        sourceRow = dm1_viewport_3d_f0115_c2500_c2900_row(
            request->relForward, 0);
        scaleIndex = dm1_viewport_3d_object_source_scale_index(
            request->relForward, request->relativeCell);
        if (sourceRow < 0 || !dm1_viewport_3d_c2500_object_raw_zone_point(
                sourceRow, request->relativeCell, &zoneX, &zoneY)) return 0;
        receipt.dstW = source->width *
            dm1_viewport_3d_object_source_scale_units(scaleIndex) / 32;
        receipt.dstH = source->height *
            dm1_viewport_3d_object_source_scale_units(scaleIndex) / 32;
        /* PC34's derived 6x3 item sprites legitimately become 1px high
         * at distance. Reject only an actual zero-sized source result. */
        if (receipt.dstW < 1) receipt.dstW = 1;
        if (receipt.dstH < 1) receipt.dstH = 1;
        shiftSet = (scaleIndex + 1) >> 1;
        if (shiftSet > 2) shiftSet = 2;
        dm1_viewport_3d_object_pile_shift_indices(request->pileIndex,
                                                   &shiftXIndex, &shiftYIndex);
        shiftX = dm1_viewport_3d_object_source_shift_value(shiftSet, shiftXIndex);
        shiftY = dm1_viewport_3d_object_source_shift_value(shiftSet, shiftYIndex);
        receipt.drawOrder = DM1_V1_F0115_DRAW_ORDER_ITEM_PC34;
        receipt.graphicIndex = expectedGraphic;
        receipt.dstX = request->viewportX + zoneX - receipt.dstW / 2 + shiftX;
        receipt.dstY = request->viewportY + zoneY - receipt.dstH + shiftY;
        receipt.flipHorizontal =
            (dm1_object_aspect_graphic_info(dm1_item_aspect_index(
                request->thingType, request->subtype)) & 1u) != 0 &&
            (request->relativeCell == 1 || request->relativeCell == 3);
        palette_for_depth(1, receipt.paletteMap);
    } else {
        DM1_CreatureDrawPlacement placement;
        int mirror = 0;
        expectedGraphic = (int)dm1_creature_sprite_for_view(
            request->creatureType, request->relForward,
            request->creatureDirection, request->partyDirection, 0, &mirror);
        if (!find_material(materials, materialCount, expectedGraphic, &source,
                           &receipt.sourcePixelsFNV1a) ||
            !dm1_creature_center_draw_placement(
                request->creatureType, request->relForward,
                request->viewportX, request->viewportY, request->viewportW,
                request->viewportH, request->groupCount, request->groupIndex,
                request->creatureCount, request->duplicateIndex, &placement)) return 0;
        receipt.drawOrder = DM1_V1_F0115_DRAW_ORDER_CREATURE_PC34;
        receipt.graphicIndex = expectedGraphic;
        receipt.dstX = placement.x;
        receipt.dstY = placement.y;
        receipt.dstW = placement.w;
        receipt.dstH = placement.h;
        receipt.flipHorizontal = mirror;
        receipt.transparentColor = dm1_creature_transparent_color(request->creatureType);
        palette_for_depth(request->relForward, receipt.paletteMap);
    }
    receipt.cropX = 0;
    receipt.cropY = 0;
    receipt.cropW = source->width;
    receipt.cropH = source->height;
    receipt.paletteFNV1a = DM1_V1_FloorFeatureFNV1aPc34(
        receipt.paletteMap, (int)sizeof(receipt.paletteMap));
    if (receipt.dstW <= 0 || receipt.dstH <= 0 || !receipt.paletteFNV1a) return 0;
    receipt.valid = 1;
    *outReceipt = receipt;
    return 1;
}
