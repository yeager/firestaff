#include <stdio.h>
#include <string.h>

#include "memory_graphics_dat_dialog_path_pc34_compat.h"

static int gDialogCallCount;
static const unsigned char* gLastGraphic;
static unsigned char* gLastViewport;
static struct GraphicWidthHeight_Compat gLastSizeInfo;

void F0427_DIALOG_DrawBackdrop_Compat(
    const unsigned char* graphic,
    unsigned char* viewportBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo) {
    gDialogCallCount++;
    gLastGraphic = graphic;
    gLastViewport = viewportBitmap;
    gLastSizeInfo = *sizeInfo;
    memcpy(viewportBitmap, graphic, 5);
}

static int write_u16(FILE* f, unsigned short value) {
    unsigned char bytes[2];
    bytes[0] = (unsigned char)(value & 0xFF);
    bytes[1] = (unsigned char)((value >> 8) & 0xFF);
    return fwrite(bytes, 1, 2, f) == 2;
}

static int write_format1_file(const char* path) {
    FILE* f = fopen(path, "wb");
    unsigned char payloadA[3] = {0x10, 0x11, 0x12};
    unsigned char payloadB[5] = {0x20, 0x21, 0x22, 0x23, 0x24};
    if (f == 0) {
        return 0;
    }
    if (!write_u16(f, 0x8001)
     || !write_u16(f, 2)
     || !write_u16(f, 3)
     || !write_u16(f, 5)
     || !write_u16(f, 13)
     || !write_u16(f, 25)
     || !write_u16(f, 7)
     || !write_u16(f, 8)
     || !write_u16(f, 9)
     || !write_u16(f, 10)
     || fwrite(payloadA, 1, sizeof(payloadA), f) != sizeof(payloadA)
     || fwrite(payloadB, 1, sizeof(payloadB), f) != sizeof(payloadB)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static int test_dialog_path_preloads_then_draws_backdrop(void) {
    const char* path = "./test_graphics_dat_dialog_path.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatSpecials_Compat specials = {0};
    unsigned char viewport[32] = {0};
    int i;

    gDialogCallCount = 0;
    gLastGraphic = 0;
    gLastViewport = 0;
    memset(&gLastSizeInfo, 0, sizeof(gLastSizeInfo));
    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(path, &fileState, &runtimeState)) {
        return 0;
    }
    if (!F0427_DIALOG_DrawPreloadedBackdrop_Compat(path, &runtimeState, &fileState, 1, &specials, viewport)) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (gDialogCallCount != 1 || gLastGraphic != specials.dialogBoxGraphic || gLastViewport != viewport) {
        F0479_MEMORY_FreeSpecialGraphics_Compat(&specials);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (gLastSizeInfo.Width != 9 || gLastSizeInfo.Height != 10) {
        F0479_MEMORY_FreeSpecialGraphics_Compat(&specials);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (specials.dialogBoxAllocatedByteCount != 25 || specials.dialogBoxLoadedByteCount != 5) {
        F0479_MEMORY_FreeSpecialGraphics_Compat(&specials);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    for (i = 0; i < 5; i++) {
        if (viewport[i] != (unsigned char)(0x20 + i)) {
            F0479_MEMORY_FreeSpecialGraphics_Compat(&specials);
            F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
            return 0;
        }
    }
    F0479_MEMORY_FreeSpecialGraphics_Compat(&specials);
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 1;
}

int main(void) {
    if (!test_dialog_path_preloads_then_draws_backdrop()) {
        fprintf(stderr, "test_dialog_path_preloads_then_draws_backdrop failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
