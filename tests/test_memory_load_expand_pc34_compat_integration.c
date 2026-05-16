#include <stdio.h>
#include <string.h>

#include "memory_load_expand_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_apply_loaded_graphic_expands_and_copies_dimensions(void) {
    unsigned char graphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char storage[6] = {0};
    unsigned char* bitmap = storage + 4;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    F0490_MEMORY_ApplyLoadedGraphic_Compat(0, graphic, sizeof(graphic), bitmap, &sizeInfo);
    return storage[0] == 0x02 && storage[1] == 0x00 && storage[2] == 0x01 && storage[3] == 0x00 &&
           bitmap[0] == 0x22 && bitmap[1] == 0x00;
}

static int test_apply_loaded_graphic_expands_without_copying_dimensions(void) {
    unsigned char graphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char storage[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0x00, 0x00};
    unsigned char* bitmap = storage + 4;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    F0490_MEMORY_ApplyLoadedGraphic_Compat(MEMORY_LOAD_EXPAND_FLAG_DO_NOT_COPY_DIMENSIONS, graphic, sizeof(graphic), bitmap, &sizeInfo);
    return storage[0] == 0xAA && storage[1] == 0xBB && storage[2] == 0xCC && storage[3] == 0xDD &&
           bitmap[0] == 0x22 && bitmap[1] == 0x00;
}

static int test_apply_loaded_graphic_without_expand_copies_raw_graphic(void) {
    unsigned char graphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char destination[8] = {0};
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    F0490_MEMORY_ApplyLoadedGraphic_Compat(MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED | MEMORY_LOAD_EXPAND_FLAG_DO_NOT_COPY_DIMENSIONS, graphic, sizeof(graphic), destination, &sizeInfo);
    return memcmp(destination, graphic, sizeof(graphic)) == 0;
}

int main(void) {
    if (!test_apply_loaded_graphic_expands_and_copies_dimensions()) {
        fprintf(stderr, "test_apply_loaded_graphic_expands_and_copies_dimensions failed\n");
        return 1;
    }
    if (!test_apply_loaded_graphic_expands_without_copying_dimensions()) {
        fprintf(stderr, "test_apply_loaded_graphic_expands_without_copying_dimensions failed\n");
        return 1;
    }
    if (!test_apply_loaded_graphic_without_expand_copies_raw_graphic()) {
        fprintf(stderr, "test_apply_loaded_graphic_without_expand_copies_raw_graphic failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
