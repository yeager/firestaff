#include <stdio.h>
#include <string.h>

#include "memory_graphics_dat_transaction_pc34_compat.h"
#include "memory_load_expand_pc34_compat.h"

static int gApplyCallCount;

void F0490_MEMORY_ApplyLoadedGraphic_Compat(
    int graphicIndexFlags,
    const unsigned char* loadedGraphic,
    unsigned long loadedByteCount,
    unsigned char* destinationBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo) {
    (void)graphicIndexFlags;
    (void)sizeInfo;
    gApplyCallCount++;
    memcpy(destinationBitmap, loadedGraphic, (size_t)loadedByteCount);
}

static int write_test_file(const char* path) {
    FILE* f;
    int i;


    f = fopen(path, "wb");
    if (f == 0) {
        return 0;
    }
    for (i = 0; i < 4096; i++) {
        unsigned char b = (unsigned char)(i & 0xFF);
        if (fwrite(&b, 1, 1, f) != 1) {
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    return 1;
}

static int test_not_expanded_loads_directly_to_destination(void) {
    const char* path = "./test_graphics_dat_transaction.bin";
    struct MemoryGraphicsDatState_Compat state = {0};
    struct MemoryGraphicsDatTransactionResult_Compat result = {0};
    struct GraphicWidthHeight_Compat sizeInfo = {8, 1};
    unsigned char viewport[32] = {0};
    unsigned char destination[32] = {0};
    int i;

    gApplyCallCount = 0;
    if (!write_test_file(path)) {
        return 0;
    }
    if (!F0490_MEMORY_RunGraphicsDatTransaction_Compat(
            path,
            100,
            8,
            MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED,
            &state,
            viewport,
            destination,
            &sizeInfo,
            &result)) {
        return 0;
    }
    if (!result.graphicsOpened || !result.graphicsClosed) {
        return 0;
    }
    if (gApplyCallCount != 0) {
        return 0;
    }
    if (result.session.loadTarget != destination || result.session.usedViewportBuffer) {
        return 0;
    }
    for (i = 0; i < 8; i++) {
        if (destination[i] != (unsigned char)((100 + i) & 0xFF)) {
            return 0;
        }
    }
    return 1;
}

static int test_expanded_loads_via_viewport_then_applies(void) {
    const char* path = "./test_graphics_dat_transaction.bin";
    struct MemoryGraphicsDatState_Compat state = {0};
    struct MemoryGraphicsDatTransactionResult_Compat result = {0};
    struct GraphicWidthHeight_Compat sizeInfo = {4, 2};
    unsigned char viewport[32] = {0};
    unsigned char destination[32] = {0};
    int i;

    gApplyCallCount = 0;
    if (!write_test_file(path)) {
        return 0;
    }
    if (!F0490_MEMORY_RunGraphicsDatTransaction_Compat(
            path,
            512,
            8,
            0,
            &state,
            viewport,
            destination,
            &sizeInfo,
            &result)) {
        return 0;
    }
    if (!result.session.usedViewportBuffer || result.session.loadTarget != viewport) {
        return 0;
    }
    if (gApplyCallCount != 1) {
        return 0;
    }
    for (i = 0; i < 8; i++) {
        if (viewport[i] != (unsigned char)((512 + i) & 0xFF)) {
            return 0;
        }
        if (destination[i] != viewport[i]) {
            return 0;
        }
    }
    return 1;
}

int main(void) {
    if (!test_not_expanded_loads_directly_to_destination()) {
        fprintf(stderr, "test_not_expanded_loads_directly_to_destination failed\n");
        return 1;
    }
    if (!test_expanded_loads_via_viewport_then_applies()) {
        fprintf(stderr, "test_expanded_loads_via_viewport_then_applies failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
