#include <stdio.h>

#include "memory_graphics_dat_special_pc34_compat.h"

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

static int test_dialog_box_special_preload_allocates_by_decompressed_size(void) {
    const char* path = "./test_graphics_dat_special.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatSpecials_Compat specials = {0};
    int i;

    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(path, &fileState, &runtimeState)) {
        return 0;
    }
    if (!F0479_MEMORY_PreloadDialogBoxGraphic_Compat(path, &runtimeState, &fileState, 1, &specials)) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (specials.dialogBoxGraphic == 0 || specials.dialogBoxAllocatedByteCount != 25 || specials.dialogBoxLoadedByteCount != 5) {
        F0479_MEMORY_FreeSpecialGraphics_Compat(&specials);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    for (i = 0; i < 5; i++) {
        if (specials.dialogBoxGraphic[i] != (unsigned char)(0x20 + i)) {
            F0479_MEMORY_FreeSpecialGraphics_Compat(&specials);
            F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
            return 0;
        }
    }
    for (i = 5; i < 25; i++) {
        if (specials.dialogBoxGraphic[i] != 0) {
            F0479_MEMORY_FreeSpecialGraphics_Compat(&specials);
            F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
            return 0;
        }
    }
    if (!F0479_MEMORY_PreloadDialogBoxGraphic_Compat(path, &runtimeState, &fileState, 1, &specials)) {
        F0479_MEMORY_FreeSpecialGraphics_Compat(&specials);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    F0479_MEMORY_FreeSpecialGraphics_Compat(&specials);
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 1;
}

int main(void) {
    if (!test_dialog_box_special_preload_allocates_by_decompressed_size()) {
        fprintf(stderr, "test_dialog_box_special_preload_allocates_by_decompressed_size failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
