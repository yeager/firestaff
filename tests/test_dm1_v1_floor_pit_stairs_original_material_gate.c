/* Real PC34 F0104/F0114-adjacent floor, pit and stair material gate.
 * No framebuffer renderer is linked: every acceptance is based solely on
 * decoded GRAPHICS.DAT pixels, ReDMCSB geometry and the native palette. */

#include "asset_loader_m11.h"
#include "dm1_v1_floor_pit_pc34_compat.h"
#include "dm1_v1_stairs_render_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kMaterialCount = 33 };

static const int kGraphics[kMaterialCount] = {
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125
};

static const char* graphics_path(void)
{
    static char fallback[1024];
    const char* configured = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    const char* home;
    if (configured && configured[0]) return configured;
    home = getenv("HOME");
    if (!home || !home[0]) return 0;
    snprintf(fallback, sizeof(fallback), "%s/.firestaff/data/dm1/GRAPHICS.DAT",
             home);
    return fallback;
}

int main(void)
{
    M11_AssetLoader loader;
    DM1_V1_FloorFeatureSourceMaterialPc34 materials[kMaterialCount];
    int i;

    memset(&loader, 0, sizeof(loader));
    memset(materials, 0, sizeof(materials));
    if (!M11_AssetLoader_Init(&loader, graphics_path())) {
        if (getenv("FIRESTAFF_DM1_GRAPHICS_DAT")) {
            fputs("configured PC34 GRAPHICS.DAT is unavailable\n", stderr);
            return 1;
        }
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    for (i = 0; i < kMaterialCount; ++i) {
        const M11_AssetSlot* slot = M11_AssetLoader_Load(
            &loader, (unsigned int)kGraphics[i]);
        if (!slot || !slot->loaded || !slot->pixels ||
            slot->width == 0 || slot->height == 0) {
            fputs("PC34 floor/pit/stairs source decode failed\n", stderr);
            goto fail;
        }
        materials[i].graphicsDatOwned = 1;
        materials[i].graphicIndex = kGraphics[i];
        materials[i].width = slot->width;
        materials[i].height = slot->height;
        materials[i].indexedPixels = slot->pixels;
        materials[i].indexedPixelCount = (int)slot->width * (int)slot->height;
        materials[i].pixelsFNV1a = DM1_V1_FloorFeatureFNV1aPc34(
            slot->pixels, materials[i].indexedPixelCount);
        if (materials[i].pixelsFNV1a == 0u) goto fail;
    }
    for (i = 0; i < dm1_v1_floor_pit_plan_count_pc34(); ++i) {
        DM1_FloorPitRenderPlanPc34 plan;
        DM1_V1_FloorFeatureMaterialReceiptPc34 visible;
        DM1_V1_FloorFeatureMaterialReceiptPc34 invisible;
        if (!dm1_v1_floor_pit_render_plan_at_pc34(i, &plan) ||
            !dm1_v1_floor_pit_original_material_receipt_pc34(
                plan.relForward, plan.relSide, 0x08, materials, kMaterialCount,
                &visible) || !visible.valid ||
            visible.graphicIndex != plan.visibleBlit.graphicIndex ||
            visible.paletteRoute != DM1_V1_FLOOR_FEATURE_PALETTE_NATIVE_PC34) {
            fprintf(stderr, "visible floor pit plan %d rejected real source\n", i);
            goto fail;
        }
        if (plan.hasInvisibleBlit &&
            (!dm1_v1_floor_pit_original_material_receipt_pc34(
                 plan.relForward, plan.relSide, 0x0c, materials, kMaterialCount,
                 &invisible) || !invisible.valid ||
             invisible.graphicIndex != plan.invisibleBlit.graphicIndex)) {
            fprintf(stderr, "invisible floor pit plan %d rejected real source\n", i);
            goto fail;
        }
    }
    for (i = 0; i < dm1_v1_stairs_render_plan_count_pc34(); ++i) {
        DM1_StairsRenderPlanPc34 plan;
        DM1_V1_FloorFeatureMaterialReceiptPc34 up;
        DM1_V1_FloorFeatureMaterialReceiptPc34 down;
        int square;
        int downSquare;
        if (!dm1_v1_stairs_render_plan_at_pc34(i, &plan)) {
            fprintf(stderr, "stairs plan %d is unavailable\n", i);
            goto fail;
        }
        square = plan.frontOnly ? 0x0c : 0x04;
        downSquare = square & ~0x04;
        if (
            !dm1_v1_stairs_original_material_receipt_pc34(
                plan.relForward, plan.relSide, square, 0, materials,
                kMaterialCount, &up) || !up.valid ||
            up.graphicIndex != plan.upBlit.graphicIndex ||
            !dm1_v1_stairs_original_material_receipt_pc34(
                plan.relForward, plan.relSide, downSquare, 0, materials,
                kMaterialCount, &down) || !down.valid ||
            down.graphicIndex != plan.downBlit.graphicIndex) {
            fprintf(stderr, "stairs plan %d rejected real source\n", i);
            goto fail;
        }
    }
    ++materials[0].pixelsFNV1a;
    {
        DM1_V1_FloorFeatureMaterialReceiptPc34 receipt;
        if (dm1_v1_floor_pit_original_material_receipt_pc34(
                3, -2, 0x08, materials, kMaterialCount, &receipt)) {
            fputs("floor pit admitted drifted GRAPHICS.DAT material\n", stderr);
            goto fail;
        }
    }
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 floor, pit and stairs material/geometry/palette gate");
    return 0;

fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
