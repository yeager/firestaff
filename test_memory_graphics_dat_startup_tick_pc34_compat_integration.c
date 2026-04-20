#include <stdio.h>
#include <string.h>

#include "memory_graphics_dat_startup_tick_pc34_compat.h"
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

static int test_startup_tick_mini_completes_after_main_loop_entry(void) {
    const char* path = "./test_graphics_dat_startup_tick.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatStartupTickResult_Compat tick;
    unsigned char viewportGraphicBuffer[32] = {0};
    unsigned char viewportBitmap[32] = {0};
    int i;

    gApplyCallCount = 0;
    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_RunStartupTickMini_Compat(
            path,
            &fileState,
            1,
            0,
            viewportGraphicBuffer,
            viewportBitmap,
            &tick)) {
        return 0;
    }
    if (!tick.startupTickReady || !tick.startupTickCompleted || tick.stage != MEMORY_GRAPHICS_DAT_STARTUP_TICK_STAGE_STARTUP_TICK_COMPLETE) {
        F0479_MEMORY_FreeStartupTickMini_Compat(&tick);
        return 0;
    }
    if (!tick.mainLoop.mainLoopEntered || tick.mainLoop.stage != MEMORY_GRAPHICS_DAT_MAIN_LOOP_STAGE_MAIN_LOOP_ENTERED) {
        F0479_MEMORY_FreeStartupTickMini_Compat(&tick);
        return 0;
    }
    if (gApplyCallCount != 1) {
        F0479_MEMORY_FreeStartupTickMini_Compat(&tick);
        return 0;
    }
    for (i = 0; i < 3; i++) {
        if (viewportGraphicBuffer[i] != (unsigned char)(0x10 + i) || viewportBitmap[i] != viewportGraphicBuffer[i]) {
            F0479_MEMORY_FreeStartupTickMini_Compat(&tick);
            return 0;
        }
    }
    F0479_MEMORY_FreeStartupTickMini_Compat(&tick);
    return 1;
}

int main(void) {
    if (!test_startup_tick_mini_completes_after_main_loop_entry()) {
        fprintf(stderr, "test_startup_tick_mini_completes_after_main_loop_entry failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
