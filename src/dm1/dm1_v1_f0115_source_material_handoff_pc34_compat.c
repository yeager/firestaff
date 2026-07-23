#include "dm1_v1_f0115_source_material_handoff_pc34_compat.h"

#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <string.h>

static const char *const kSourceAnchor =
    "ReDMCSB DUNVIEW.C F0115:4820-5078 (C2500/G0209/F0791 object piles), "
    "5668-5900 (C2900/M613 and F0142/G0209 projectiles)";

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
