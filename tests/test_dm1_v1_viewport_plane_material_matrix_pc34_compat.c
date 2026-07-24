/* Real PC34 F0094/F0098 floor-set material matrix regression. */

#include "dm1_v1_viewport_plane_material_matrix_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int data_path(char *out, size_t outSize)
{
    const char *root = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char *home;
    if (root && root[0]) {
        snprintf(out, outSize, "%s/GRAPHICS.DAT", root);
        return 1;
    }
    home = getenv("HOME");
    if (!home || !home[0]) return 0;
    snprintf(out, outSize, "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    return 1;
}

int main(void)
{
    char graphicsPath[1200];
    M11_AssetLoader loader;
    DM1_V1_ViewportPlaneBlitPc34 planes[2];
    DM1_V1_ViewportPlaneMaterialMatrixPc34 matrix;
    DM1_V1_ViewportPlaneMaterialReceiptPc34 receipt;
    unsigned char framebuffer[320 * 200];
    int ok;

    if (!data_path(graphicsPath, sizeof(graphicsPath)) ||
        !M11_AssetLoader_Init(&loader, graphicsPath)) {
        if (getenv("FIRESTAFF_DM1_DATA_DIR")) {
            fputs("FAIL: requested PC34 GRAPHICS.DAT could not be opened\n", stderr);
            return 1;
        }
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    memset(planes, 0, sizeof(planes));
    planes[0].kind = DM1_V1_VIEWPORT_PLANE_FLOOR_PC34;
    planes[0].dstX = 8; planes[0].dstY = 60;
    planes[0].width = 16; planes[0].height = 16;
    planes[0].transparentColor = -1;
    planes[1].kind = DM1_V1_VIEWPORT_PLANE_CEILING_PC34;
    planes[1].dstX = 32; planes[1].dstY = 8;
    planes[1].width = 16; planes[1].height = 16;
    planes[1].transparentColor = -1;
    memset(framebuffer, 0xff, sizeof(framebuffer));

    ok = dm1_v1_viewport_plane_material_matrix_decode_pc34(
             &loader, 0, planes, 2, &matrix, &receipt) &&
         matrix.valid && receipt.valid && matrix.planeCount == 2 &&
         matrix.surfaces[0].graphicIndex == 78 &&
         matrix.surfaces[1].graphicIndex == 79 &&
         dm1_v1_viewport_plane_material_matrix_render_pc34(
             &loader, 0, planes, 2, &matrix, &receipt, framebuffer, 320, 200) &&
         framebuffer[60 * 320 + 8] == matrix.surfaces[0].indexedPixels[0] &&
         framebuffer[8 * 320 + 32] == matrix.surfaces[1].indexedPixels[0];
    if (ok) {
        /* A changed rectangle/palette cannot reuse an authenticated receipt. */
        planes[0].dstX = 9;
        ok = !dm1_v1_viewport_plane_material_matrix_render_pc34(
            &loader, 0, planes, 2, &matrix, &receipt, framebuffer, 320, 200);
    }
    if (ok) {
        planes[0].dstX = 8;
        planes[1].paletteMapValid = 1;
        memset(planes[1].paletteMap, 0, sizeof(planes[1].paletteMap));
        planes[1].paletteMap[1] = 2;
        ok = !dm1_v1_viewport_plane_material_matrix_render_pc34(
            &loader, 0, planes, 2, &matrix, &receipt, framebuffer, 320, 200);
    }
    M11_AssetLoader_Shutdown(&loader);
    if (!ok) return 1;
    puts("ok: real PC34 F0094 floor/ceiling material matrix is source-bound");
    return 0;
}
