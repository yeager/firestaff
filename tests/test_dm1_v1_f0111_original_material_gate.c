/* Real PC34 F0111 source-material gate. No M11 framebuffer or substitute
 * texture participates: every D1/D2/D3 frame and panel comes from the
 * installed GRAPHICS.DAT byte stream or the test fails closed. */

#include "dm1_v1_center_door_render_pc34_compat.h"
#include "dm1_v1_side_door_render_pc34_compat.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kDoorGraphicCount = 9 };

static const int kDoorGraphics[kDoorGraphicCount] = {
    87, 88, 89, 90, 91, 92, 246, 247, 248
};

static const char* graphics_path(void)
{
    static char fallback[1024];
    const char* configured = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    const char* home;
    if (configured && configured[0]) {
        return configured;
    }
    home = getenv("HOME");
    if (!home || !home[0]) {
        return 0;
    }
    snprintf(fallback, sizeof(fallback), "%s/.firestaff/data/dm1/GRAPHICS.DAT",
             home);
    return fallback;
}

int main(void)
{
    const char* path = graphics_path();
    M11_AssetLoader loader;
    DM1_V1_DoorSourceMaterialPc34 materials[kDoorGraphicCount];
    int i;

    memset(&loader, 0, sizeof(loader));
    if (!M11_AssetLoader_Init(&loader, path)) {
        if (getenv("FIRESTAFF_DM1_GRAPHICS_DAT")) {
            fputs("configured PC34 GRAPHICS.DAT is unavailable\n", stderr);
            return 1;
        }
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    memset(materials, 0, sizeof(materials));
    for (i = 0; i < kDoorGraphicCount; ++i) {
        const M11_AssetSlot* slot = M11_AssetLoader_Load(
            &loader, (unsigned int)kDoorGraphics[i]);
        if (!slot || !slot->loaded || !slot->pixels ||
            slot->width == 0 || slot->height == 0) {
            fputs("PC34 F0111 graphic failed indexed decode\n", stderr);
            goto fail;
        }
        materials[i].graphicsDatOwned = 1;
        materials[i].graphicIndex = kDoorGraphics[i];
        materials[i].width = slot->width;
        materials[i].height = slot->height;
        materials[i].pixels = slot->pixels;
        materials[i].pixelByteCount = (int)slot->width * (int)slot->height;
        materials[i].pixelsFNV1a = DM1_V1_DoorSourcePixelsFNV1aPc34(
            slot->pixels, materials[i].pixelByteCount);
        if (materials[i].pixelsFNV1a == 0u) {
            fputs("PC34 F0111 graphic has no source fingerprint\n", stderr);
            goto fail;
        }
    }
    for (i = 0; i < 3; ++i) {
        DM1_CenterDoorOriginalMaterialReceiptPc34 receipt;
        int panel = dm1_v1_door_panel_graphic_for_set_depth_pc34(0, i);
        if (!dm1_v1_center_door_original_material_receipt_pc34(
                i, 4, 1, panel, materials, kDoorGraphicCount, &receipt) ||
            !receipt.plan.valid || !receipt.plan.panelVisible ||
            receipt.sourceMaterialCount != receipt.plan.blitCount) {
            fputs("F0111 center door admitted incomplete source material\n", stderr);
            goto fail;
        }
    }
    for (i = 0; i < dm1_v1_side_door_render_plan_count_pc34(); ++i) {
        DM1_SideDoorRenderPlanPc34 plan;
        DM1_SideDoorOriginalMaterialReceiptPc34 receipt;
        if (!dm1_v1_side_door_render_plan_at_pc34(i, &plan) ||
            !dm1_v1_side_door_original_material_receipt_pc34(
                plan.relForward, plan.relSide, 4, 1, plan.panel.graphicIndex,
                materials, kDoorGraphicCount, &receipt) ||
            !receipt.valid || !receipt.panelVisible || receipt.blitCount <= 0) {
            fprintf(stderr, "F0111 side door %d admitted incomplete source material\n",
                    i);
            goto fail;
        }
    }
    {
        DM1_CenterDoorOriginalMaterialReceiptPc34 receipt;
        ++materials[0].pixelsFNV1a;
        if (dm1_v1_center_door_original_material_receipt_pc34(
                0, 4, 1, 248, materials, kDoorGraphicCount, &receipt)) {
            fputs("F0111 accepted drifted GRAPHICS.DAT material\n", stderr);
            goto fail;
        }
    }
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0111 D1/D2/D3 center and side material is source-bound");
    return 0;

fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
