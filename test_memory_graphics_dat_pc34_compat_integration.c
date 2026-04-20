#include <stdio.h>
#include <string.h>

#include "memory_graphics_dat_pc34_compat.h"

static int write_test_file(const char* path) {
    FILE* f;
    int i;


    f = fopen(path, "wb");
    if (f == 0) {
        return 0;
    }
    for (i = 0; i < 2500; i++) {
        unsigned char b = (unsigned char)(i & 0xFF);
        if (fwrite(&b, 1, 1, f) != 1) {
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    return 1;
}

static int test_open_close_reference_count(void) {
    const char* path = "./test_graphics_dat.bin";
    struct MemoryGraphicsDatState_Compat state = {0};
    if (!write_test_file(path)) {
        return 0;
    }
    if (!F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(path, &state)) {
        return 0;
    }
    if (!F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(path, &state)) {
        return 0;
    }
    if (state.referenceCount != 2 || state.file == 0 || state.fileSize != 2500) {
        return 0;
    }
    if (!F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&state)) {
        return 0;
    }
    if (state.referenceCount != 1 || state.file == 0) {
        return 0;
    }
    if (!F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&state)) {
        return 0;
    }
    return state.referenceCount == 0 && state.file == 0;
}

static int test_load_graphic_reads_across_chunk_boundary(void) {
    const char* path = "./test_graphics_dat.bin";
    struct MemoryGraphicsDatState_Compat state = {0};
    unsigned char buffer[40] = {0};
    int i;
    if (!write_test_file(path)) {
        return 0;
    }
    if (!F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(path, &state)) {
        return 0;
    }
    if (!F0474_MEMORY_LoadGraphic_CPSDF_Compat(1010, 40, &state, buffer)) {
        return 0;
    }
    for (i = 0; i < 40; i++) {
        if (buffer[i] != (unsigned char)((1010 + i) & 0xFF)) {
            return 0;
        }
    }
    F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&state);
    return 1;
}

static int test_load_graphic_reuses_cached_chunk_for_same_area(void) {
    const char* path = "./test_graphics_dat.bin";
    struct MemoryGraphicsDatState_Compat state = {0};
    unsigned char buffer1[8] = {0};
    unsigned char buffer2[8] = {0};
    if (!write_test_file(path)) {
        return 0;
    }
    if (!F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(path, &state)) {
        return 0;
    }
    if (!F0474_MEMORY_LoadGraphic_CPSDF_Compat(100, 8, &state, buffer1)) {
        return 0;
    }
    if (state.cachedChunkIndex != 0) {
        return 0;
    }
    if (!F0474_MEMORY_LoadGraphic_CPSDF_Compat(120, 8, &state, buffer2)) {
        return 0;
    }
    if (state.cachedChunkIndex != 0) {
        return 0;
    }
    F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&state);
    return buffer1[0] == (unsigned char)(100 & 0xFF) && buffer2[0] == (unsigned char)(120 & 0xFF);
}

int main(void) {
    if (!test_open_close_reference_count()) {
        fprintf(stderr, "test_open_close_reference_count failed\n");
        return 1;
    }
    if (!test_load_graphic_reads_across_chunk_boundary()) {
        fprintf(stderr, "test_load_graphic_reads_across_chunk_boundary failed\n");
        return 1;
    }
    if (!test_load_graphic_reuses_cached_chunk_for_same_area()) {
        fprintf(stderr, "test_load_graphic_reuses_cached_chunk_for_same_area failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
