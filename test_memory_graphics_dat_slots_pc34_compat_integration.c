#include <stdio.h>

#include "memory_graphics_dat_slots_pc34_compat.h"

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

static int test_preload_slot_loads_once_and_reuses_pointer(void) {
    const char* path = "./test_graphics_dat_slots.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatSlots_Compat slots;
    struct MemoryGraphicsDatSelection_Compat selection = {0};
    unsigned char* first;
    unsigned char* second;
    int i;

    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(path, &fileState, &runtimeState)) {
        return 0;
    }
    if (!F0484_MEMORY_InitializeGraphicSlots_Compat(runtimeState.graphicCount, &slots)) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    first = F0484_MEMORY_PreloadGraphicSlot_Compat(path, &runtimeState, &fileState, 1, &slots, &selection);
    if (first == 0 || selection.offset != 23 || selection.compressedByteCount != 5) {
        F0484_MEMORY_FreeGraphicSlots_Compat(&slots);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    for (i = 0; i < 5; i++) {
        if (first[i] != (unsigned char)(0x20 + i)) {
            F0484_MEMORY_FreeGraphicSlots_Compat(&slots);
            F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
            return 0;
        }
    }
    second = F0484_MEMORY_PreloadGraphicSlot_Compat(path, &runtimeState, &fileState, 1, &slots, 0);
    if (second != first || slots.loadedByteCounts[1] != 5 || slots.graphics[1] != first) {
        F0484_MEMORY_FreeGraphicSlots_Compat(&slots);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    F0484_MEMORY_FreeGraphicSlots_Compat(&slots);
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 1;
}

int main(void) {
    if (!test_preload_slot_loads_once_and_reuses_pointer()) {
        fprintf(stderr, "test_preload_slot_loads_once_and_reuses_pointer failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
