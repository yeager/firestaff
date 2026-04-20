#include <stdio.h>
#include <string.h>

#include "memory_load_wrappers_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_get_decompressed_byte_count_uses_language_index(void) {
    unsigned short languageSpecificGraphicIndices[4] = {2, 0, 3, 1};
    unsigned short byteCounts[4] = {10, 20, 30, 40};
    return F0494_MEMORY_GetGraphicDecompressedByteCount_Compat(1, languageSpecificGraphicIndices, byteCounts) == 10 &&
           F0494_MEMORY_GetGraphicDecompressedByteCount_Compat(2, languageSpecificGraphicIndices, byteCounts) == 40;
}

static int test_load_temporary_graphic_returns_byte_count_and_raw_copy(void) {
    unsigned short languageSpecificGraphicIndices[4] = {0, 1, 2, 3};
    unsigned short byteCounts[4] = {8, 12, 16, 20};
    unsigned char loadedGraphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char destination[16] = {0};
    unsigned char* outGraphic = 0;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    long result = F0440_STARTEND_LoadTemporaryGraphic_Compat(0, languageSpecificGraphicIndices, byteCounts, loadedGraphic, &outGraphic, destination, &sizeInfo);
    return result == 8 && outGraphic == destination && memcmp(destination, loadedGraphic, 8) == 0;
}

static int test_load_endgame_bitmap_expanded_returns_bitmap(void) {
    unsigned char loadedGraphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char storage[6] = {0};
    unsigned char* bitmap = storage + 4;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    unsigned char* result = F0763_LoadEndgameBitmapExpanded_Compat(loadedGraphic, sizeof(loadedGraphic), bitmap, &sizeInfo);
    return result == bitmap && storage[0] == 0x02 && storage[1] == 0x00 && storage[2] == 0x01 && storage[3] == 0x00 && bitmap[0] == 0x22 && bitmap[1] == 0x00;
}

int main(void) {
    if (!test_get_decompressed_byte_count_uses_language_index()) {
        fprintf(stderr, "test_get_decompressed_byte_count_uses_language_index failed\n");
        return 1;
    }
    if (!test_load_temporary_graphic_returns_byte_count_and_raw_copy()) {
        fprintf(stderr, "test_load_temporary_graphic_returns_byte_count_and_raw_copy failed\n");
        return 1;
    }
    if (!test_load_endgame_bitmap_expanded_returns_bitmap()) {
        fprintf(stderr, "test_load_endgame_bitmap_expanded_returns_bitmap failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
