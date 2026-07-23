#include "dm1_v1_f0069_f0070_f0073_mouse_source_receipt_pc34_compat.h"
#include "dm1_v1_graphics_loader_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int planar_c028_to_indexed(const DM1_V1_GFX_BitmapPc34 *bitmap,
                                  uint8_t *outPixels, size_t outPixelCount)
{
    unsigned int x;
    unsigned int y;
    size_t needed = (size_t)DM1_V1_F0070_C028_WIDTH_PC34 *
                    DM1_V1_F0070_C028_HEIGHT_PC34;

    if (!bitmap || !bitmap->data || !outPixels ||
        bitmap->width != DM1_V1_F0070_C028_WIDTH_PC34 ||
        bitmap->height != DM1_V1_F0070_C028_HEIGHT_PC34 ||
        outPixelCount != needed) return 0;
    for (y = 0U; y < bitmap->height; ++y) {
        for (x = 0U; x < bitmap->width; ++x) {
            unsigned int plane;
            uint8_t color = 0U;
            size_t row = (size_t)y * bitmap->byte_width * 4U;
            uint8_t bit = (uint8_t)(0x80U >> (x & 7U));
            for (plane = 0U; plane < 4U; ++plane) {
                if (bitmap->data[row + plane * bitmap->byte_width + (x >> 3)] & bit) {
                    color |= (uint8_t)(1U << plane);
                }
            }
            outPixels[(size_t)y * bitmap->width + x] = color;
        }
    }
    return 1;
}

int main(void)
{
    const char *graphicsPath = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    DM1_V1_F0069F0070RuntimeInputPc34 input;
    DM1_V1_F0069F0070F0073ReceiptPc34 receipt;
    uint8_t indexed[DM1_V1_F0070_C028_WIDTH_PC34 * DM1_V1_F0070_C028_HEIGHT_PC34];

    memset(&input, 0, sizeof(input));
    input.leaderEmptyHanded = 1;
    input.leaderIndex = -1;
    input.targetChampionIconIndex = 0;
    input.championIndexByCell[0] = 0;
    input.championIndexByCell[1] = -1;
    input.championIndexByCell[2] = -1;
    input.championIndexByCell[3] = -1;
    check(!dm1_v1_f0069_f0070_f0073_mouse_runtime_receipt_pc34(&input, &receipt) &&
              receipt.suppressSyntheticUi && receipt.f0073SyntheticScreenAreaSuppressed,
          "missing authenticated C028 fails closed without synthetic UI");

    if (graphicsPath && graphicsPath[0] != '\0') {
        DM1_V1_GFX_LoaderStatePc34 loader;
        DM1_V1_GFX_BitmapPc34 bitmap;
        DM1_V1_GFX_InitPc34Compat(&loader);
        memset(&bitmap, 0, sizeof(bitmap));
        check(DM1_V1_GFX_OpenDatPc34Compat(&loader, graphicsPath),
              "real GRAPHICS.DAT opens");
        if (loader.loaded) {
            if (!DM1_V1_GFX_LoadBitmapPc34Compat(
                    &loader, DM1_V1_F0070_C028_GRAPHIC_PC34, &bitmap)) {
                check(!dm1_v1_f0069_f0070_f0073_mouse_runtime_receipt_pc34(
                          &input, &receipt) && receipt.suppressSyntheticUi,
                      "unavailable real C028 decode remains fail closed");
            }
            if (bitmap.allocated) {
                check(planar_c028_to_indexed(&bitmap, indexed, sizeof(indexed)),
                      "real C028 planar material converts to indexed pixels");
                input.championIcons.graphicsDatAuthenticated = 1;
                input.championIcons.graphicIndex = DM1_V1_F0070_C028_GRAPHIC_PC34;
                input.championIcons.width = DM1_V1_F0070_C028_WIDTH_PC34;
                input.championIcons.height = DM1_V1_F0070_C028_HEIGHT_PC34;
                input.championIcons.indexedPixels = indexed;
                input.championIcons.indexedPixelCount = sizeof(indexed);
                input.championIcons.indexedPixelsFnv1a =
                    dm1_v1_f0069_f0070_f0073_fnv1a_pc34(indexed, sizeof(indexed));
                check(dm1_v1_f0069_f0070_f0073_mouse_runtime_receipt_pc34(
                          &input, &receipt),
                      "real C028 enables F0069/F0070 runtime receipt");
                check(receipt.valid && receipt.f0069Pointer == DM1_V1_F0069_POINTER_CHAMPION_PC34 &&
                          receipt.f0070PickedUp && receipt.nextHeldChampionIconOrdinal == 1U &&
                          receipt.f0073Pc34ScreenAreaUnavailable &&
                          receipt.f0073SyntheticScreenAreaSuppressed,
                      "receipt preserves source pointer transition and PC34 F0073 boundary");
                input.heldChampionIconOrdinal = receipt.nextHeldChampionIconOrdinal;
                check(dm1_v1_f0069_f0070_f0073_mouse_runtime_receipt_pc34(
                          &input, &receipt) && receipt.f0070Dropped &&
                          receipt.nextHeldChampionIconOrdinal == 0U,
                      "authenticated C028 supports the F0070 drop transition");
                DM1_V1_GFX_FreeBitmapPc34Compat(&bitmap);
            }
            DM1_V1_GFX_ClosePc34Compat(&loader);
        }
    }
    return failures ? 1 : 0;
}
