#include <stdio.h>
#include <string.h>

#include "memory_graphics_dat_viewport_path_pc34_compat.h"
#include "memory_load_expand_pc34_compat.h"

static int gApplyCallCount;
static int gLastFlags;
static unsigned long gLastLoadedByteCount;
static struct GraphicWidthHeight_Compat gLastSizeInfo;

void F0490_MEMORY_ApplyLoadedGraphic_Compat(
    int graphicIndexFlags,
    const unsigned char* loadedGraphic,
    unsigned long loadedByteCount,
    unsigned char* destinationBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo) {
    gApplyCallCount++;
    gLastFlags = graphicIndexFlags;
    gLastLoadedByteCount = loadedByteCount;
    gLastSizeInfo = *sizeInfo;
    memcpy(destinationBitmap, loadedGraphic, (size_t)loadedByteCount);
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

static int test_viewport_path_loads_selected_graphic_by_index(void) {
    const char* path = "./test_graphics_dat_viewport_path.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatTransactionResult_Compat result = {0};
    struct MemoryGraphicsDatSelection_Compat selection = {0};
    unsigned char viewport[32] = {0};
    unsigned char destination[32] = {0};
    int i;

    gApplyCallCount = 0;
    gLastFlags = 0;
    gLastLoadedByteCount = 0;
    memset(&gLastSizeInfo, 0, sizeof(gLastSizeInfo));
    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(path, &fileState, &runtimeState)) {
        return 0;
    }
    if (!F0490_MEMORY_LoadViewportGraphicByIndex_Compat(
            path,
            &runtimeState,
            &fileState,
            1,
            0,
            viewport,
            destination,
            &result,
            &selection)) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (selection.offset != 23 || selection.compressedByteCount != 5 || selection.widthHeight.Width != 9 || selection.widthHeight.Height != 10) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (!result.graphicsOpened || !result.graphicsClosed || !result.session.usedViewportBuffer) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (gApplyCallCount != 1 || gLastFlags != 0 || gLastLoadedByteCount != 5 || gLastSizeInfo.Width != 9 || gLastSizeInfo.Height != 10) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    for (i = 0; i < 5; i++) {
        if (viewport[i] != (unsigned char)(0x20 + i) || destination[i] != viewport[i]) {
            F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
            return 0;
        }
    }
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 1;
}

int main(void) {
    if (!test_viewport_path_loads_selected_graphic_by_index()) {
        fprintf(stderr, "test_viewport_path_loads_selected_graphic_by_index failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
