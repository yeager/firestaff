#include "dm1_v1_f0115_source_material_handoff_pc34_compat.h"

#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <string.h>

static const char *const kSourceAnchor =
    "ReDMCSB DUNVIEW.C F0115:4820-5078 (C2500/G0209/F0791 object piles), "
    "5668-5900 (C2900/M613 and F0142/G0209 projectiles)";

static const char *const kF0248SourceAnchor =
    "ReDMCSB TIMELINE.C F0248/F0247, PROJEXPL.C F0212/F0213/F0810, "
    "DUNVIEW.C F0115:5668-6220 (raw C14/C15 plus GRAPHICS.DAT palette)";

uint32_t dm1_v1_f0115_source_material_fnv1a_pc34(const uint8_t *bytes,
                                                  size_t byteCount)
{
    uint32_t hash = UINT32_C(2166136261);
    size_t i;
    if (!bytes || byteCount == 0) return 0u;
    for (i = 0; i < byteCount; ++i) {
        hash ^= bytes[i];
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static int source_surface_is_admitted(const DM1_V1_F0115SourcePixelsPc34 *surface)
{
    size_t required;
    if (!surface || !surface->sourceOwned || !surface->pixels ||
        surface->graphicIndex <= 0 || surface->width <= 0 || surface->height <= 0) return 0;
    required = (size_t)surface->width * (size_t)surface->height;
    return surface->pixelCount >= required &&
           dm1_v1_f0115_source_material_fnv1a_pc34(surface->pixels, required) != 0u;
}

int dm1_v1_f0115_source_material_handoff_pc34(
    const DM1_V1_F0115SourceMaterialInputPc34 *input,
    DM1_V1_F0115SourceMaterialHandoffPc34 *outHandoff)
{
    DM1_V1_F0115SourceMaterialHandoffPc34 handoff;
    const DM1_V1_F0115SourcePixelsPc34 *surface;
    int row;
    int viewLane;

    if (!outHandoff) return 0;
    memset(&handoff, 0, sizeof(handoff));
    handoff.noDraw = 1;
    handoff.transparentColor = 10;
    handoff.sourceAnchor = kSourceAnchor;
    *outHandoff = handoff;
    if (!input || !(surface = input->surface) || !source_surface_is_admitted(surface) ||
        input->viewportW <= 0 || input->viewportH <= 0 || input->relativeCell < 0 ||
        input->relativeCell > 3) return 1;

    row = input->sourceZoneRow;
    if (row < 0) row = dm1_viewport_3d_f0115_c2500_c2900_row(
        input->relativeForward, input->relativeSide);
    viewLane = input->relativeSide < 0 ? -1 : (input->relativeSide > 0 ? 1 : 0);

    if (input->kind == DM1_V1_F0115_SOURCE_MATERIAL_FLOOR_OBJECT_PC34) {
        DM1_ItemSpriteBlitPlan plan;
        DM1_F0115FloorObjectMaterialReceiptPc34 receipt;
        if (row < 0 || !dm1_v1_f0115_floor_object_material_receipt_pc34(
                input->thingType, input->subtype, 2500 + row * 4 + input->relativeCell,
                row, 10, 1, surface->graphicIndex, surface->width, surface->height,
                &receipt) || !dm1_item_sprite_blit_plan(
                &plan, input->thingType, input->subtype, input->relativeCell,
                input->pileIndex, input->relativeForward, row, input->viewportX,
                input->viewportY, input->viewportX, input->viewportY, input->viewportW,
                input->viewportH, surface->width, surface->height)) return 1;
        handoff.graphicIndex = (int)receipt.graphic_index;
        handoff.sourceZone = receipt.source_zone;
        handoff.sourceZoneRow = row;
        handoff.pileIndex = input->pileIndex;
        handoff.drawX = plan.draw_x; handoff.drawY = plan.draw_y;
        handoff.drawW = plan.draw_w; handoff.drawH = plan.draw_h;
        handoff.mirror = plan.use_mirror;
    } else {
        DM1_ProjectileMaterialResolutionPc34 resolution;
        DM1_ThrownObjectProjectileBlitPlanPc34 plan;
        int usesObject;
        if (row < 0 || !dm1_v1_projectile_material_resolve_pc34(
                input->projectileSubtype, input->thingType, input->subtype,
                input->weaponProjectileAspectOrdinal, &resolution) ||
            resolution.graphic_index != (int)surface->graphicIndex) return 1;
        usesObject = resolution.uses_object_aspect;
        if ((input->kind == DM1_V1_F0115_SOURCE_MATERIAL_THROWN_OBJECT_PC34 && !usesObject) ||
            (input->kind == DM1_V1_F0115_SOURCE_MATERIAL_NATIVE_PROJECTILE_PC34 && usesObject)) return 1;
        if (usesObject) {
            if (!dm1_v1_thrown_object_projectile_blit_plan_pc34(
                    &plan, resolution.graphic_index, resolution.aspect_index,
                    input->relativeForward, input->relativeCell, viewLane, row,
                    input->viewportX, input->viewportY, input->viewportW, input->viewportH,
                    surface->width, surface->height)) return 1;
        } else {
            int zoneX;
            int zoneY;
            int scale;
            if (!dm1_viewport_3d_c2900_projectile_raw_zone_point(
                    row, input->relativeCell, &zoneX, &zoneY)) return 1;
            memset(&plan, 0, sizeof(plan));
            scale = dm1_v1_projectile_scale_units(input->relativeForward,
                                                   input->relativeCell);
            plan.draw_w = surface->width * scale / 32;
            plan.draw_h = surface->height * scale / 32;
            if (plan.draw_w < 1 || plan.draw_h < 1) return 1;
            plan.draw_x = input->viewportX + zoneX - plan.draw_w / 2;
            plan.draw_y = input->viewportY + zoneY - plan.draw_h / 2;
        }
        handoff.graphicIndex = resolution.graphic_index;
        handoff.sourceZone = 2900 + row * 4 + input->relativeCell;
        handoff.sourceZoneRow = row;
        handoff.drawX = plan.draw_x; handoff.drawY = plan.draw_y;
        handoff.drawW = plan.draw_w; handoff.drawH = plan.draw_h;
        handoff.mirror = plan.use_mirror;
    }
    handoff.valid = 1; handoff.noDraw = 0; handoff.usesF0791Blit = 1;
    handoff.pixels = surface->pixels;
    handoff.pixelCount = (size_t)surface->width * (size_t)surface->height;
    handoff.materialFNV1a = dm1_v1_f0115_source_material_fnv1a_pc34(
        handoff.pixels, handoff.pixelCount);
    *outHandoff = handoff;
    return 1;
}

int dm1_v1_f0115_source_material_to_square_pc34(
    int viewSquare,
    int materialKind,
    const DM1_V1_F0115SourceMaterialHandoffPc34 *handoff,
    DM1_V1_F0115SquareMaterialPc34 *outMaterial)
{
    DM1_V1_F0115SquareMaterialPc34 material;
    if (!outMaterial) return 0;
    memset(&material, 0, sizeof(material));
    if (!handoff || !handoff->valid || handoff->noDraw ||
        !handoff->usesF0791Blit || handoff->transparentColor != 10 ||
        handoff->graphicIndex <= 0 || !handoff->pixels || handoff->pixelCount == 0 ||
        handoff->drawW <= 0 || handoff->drawH <= 0 || handoff->materialFNV1a == 0u ||
        viewSquare < 0 || viewSquare >= DM1_V1_F0128_VIEW_SQUARE_COUNT ||
        materialKind < DM1_V1_F0115_MATERIAL_NORMAL_OBJECT_PC34 ||
        materialKind > DM1_V1_F0115_MATERIAL_EXPLOSION_PC34) return 0;
    material.square = viewSquare;
    material.kind = materialKind;
    material.graphicIndex = handoff->graphicIndex;
    material.pixels = handoff->pixels;
    material.width = handoff->drawW;
    material.height = handoff->drawH;
    material.transparentColor = handoff->transparentColor;
    *outMaterial = material;
    return 1;
}

static unsigned short read_u16le(const unsigned char *bytes)
{
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

static int f0248_surface_and_palette_are_original(
    const DM1_V1_F0248LiveEffectMaterialInputPc34 *input,
    uint32_t *outPixelsFNV1a,
    uint32_t *outPaletteFNV1a)
{
    const DM1_V1_F0115SourcePixelsPc34 *surface;
    size_t required;

    if (outPixelsFNV1a) *outPixelsFNV1a = 0u;
    if (outPaletteFNV1a) *outPaletteFNV1a = 0u;
    if (!input || !(surface = input->surface) || !source_surface_is_admitted(surface) ||
        !surface->verifiedPc34GraphicsDat ||
        surface->graphicIndex != (unsigned int)input->expectedGraphicIndex ||
        !input->paletteOwnedByPc34GraphicsDat || !input->palette ||
        input->paletteByteCount != 16u) return 0;
    required = (size_t)surface->width * (size_t)surface->height;
    if (outPixelsFNV1a) {
        *outPixelsFNV1a = dm1_v1_f0115_source_material_fnv1a_pc34(
            surface->pixels, required);
        if (*outPixelsFNV1a == 0u) return 0;
    }
    if (outPaletteFNV1a) {
        *outPaletteFNV1a = dm1_v1_f0115_source_material_fnv1a_pc34(
            input->palette, input->paletteByteCount);
        if (*outPaletteFNV1a == 0u) return 0;
    }
    return 1;
}

int dm1_v1_f0248_live_effect_material_receipt_pc34(
    const DM1_V1_F0248LiveEffectMaterialInputPc34 *input,
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 *outReceipt)
{
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 receipt;
    const unsigned char *raw;
    int index;
    size_t rawSize;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.noDraw = 1;
    receipt.sourceAnchor = kF0248SourceAnchor;
    *outReceipt = receipt;
    if (!input || !input->things || !input->things->loaded ||
        input->expectedGraphicIndex <= 0 ||
        !f0248_surface_and_palette_are_original(
            input, &receipt.graphicsPixelsFNV1a, &receipt.paletteFNV1a)) {
        return 1;
    }

    if (input->kind == DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34) {
        index = THING_GET_INDEX(input->rawThing);
        rawSize = 8u;
        if (THING_GET_TYPE(input->rawThing) != THING_TYPE_PROJECTILE || index < 0 ||
            index >= input->things->projectileCount ||
            input->things->thingCounts[THING_TYPE_PROJECTILE] !=
                input->things->projectileCount ||
            !input->things->rawThingData[THING_TYPE_PROJECTILE] ||
            !input->things->projectiles) return 1;
        raw = input->things->rawThingData[THING_TYPE_PROJECTILE] +
              (size_t)index * rawSize;
        if (read_u16le(raw) != input->things->projectiles[index].next ||
            read_u16le(raw + 2) != input->things->projectiles[index].slot ||
            raw[4] != input->things->projectiles[index].kineticEnergy ||
            raw[5] != input->things->projectiles[index].attack ||
            read_u16le(raw + 6) != input->things->projectiles[index].eventIndex ||
            read_u16le(raw + 2) != input->associatedThing) return 1;
    } else if (input->kind == DM1_V1_F0248_LIVE_EFFECT_EXPLOSION_C15_PC34) {
        index = THING_GET_INDEX(input->rawThing);
        rawSize = 4u;
        if (THING_GET_TYPE(input->rawThing) != THING_TYPE_EXPLOSION || index < 0 ||
            index >= input->things->explosionCount ||
            input->things->thingCounts[THING_TYPE_EXPLOSION] !=
                input->things->explosionCount ||
            !input->things->rawThingData[THING_TYPE_EXPLOSION] ||
            !input->things->explosions) return 1;
        raw = input->things->rawThingData[THING_TYPE_EXPLOSION] +
              (size_t)index * rawSize;
        if (read_u16le(raw) != input->things->explosions[index].next ||
            (raw[2] & 0x7fu) != input->things->explosions[index].type ||
            ((raw[2] >> 7) & 1u) != input->things->explosions[index].centered ||
            raw[3] != input->things->explosions[index].attack ||
            (int)(raw[2] & 0x7fu) != input->explosionType ||
            (int)raw[3] != input->explosionAttack ||
            (int)((raw[2] >> 7) & 1u) != input->explosionCentered) return 1;
    } else {
        return 1;
    }

    receipt.valid = 1;
    receipt.noDraw = 0;
    receipt.saveReceiptBound = 1;
    receipt.rawThing = input->rawThing;
    receipt.associatedThing = input->kind == DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34
        ? input->associatedThing : THING_NONE;
    receipt.graphicIndex = input->expectedGraphicIndex;
    receipt.rawRecordFNV1a = dm1_v1_f0115_source_material_fnv1a_pc34(raw, rawSize);
    if (receipt.rawRecordFNV1a == 0u) {
        memset(&receipt, 0, sizeof(receipt));
        receipt.noDraw = 1;
        receipt.sourceAnchor = kF0248SourceAnchor;
    }
    *outReceipt = receipt;
    return 1;
}
