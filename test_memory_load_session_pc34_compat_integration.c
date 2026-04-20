#include <stdio.h>
#include <string.h>

#include "memory_load_session_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_not_expanded_loads_directly_to_destination(void) {
    unsigned char loadedGraphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char viewportBuffer[8] = {0};
    unsigned char destination[8] = {0};
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    struct MemoryLoadSessionResult_Compat result = {0};
    F0490_MEMORY_LoadGraphicSession_Compat(
        MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED | MEMORY_LOAD_EXPAND_FLAG_DO_NOT_COPY_DIMENSIONS,
        loadedGraphic,
        sizeof(loadedGraphic),
        viewportBuffer,
        destination,
        &sizeInfo,
        &result);
    return memcmp(destination, loadedGraphic, sizeof(loadedGraphic)) == 0 &&
           result.loadTarget == destination && result.usedViewportBuffer == 0 && result.drawFloorAndCeilingRequested == 0 &&
           memcmp(viewportBuffer, (unsigned char[8]){0}, 8) == 0;
}

static int test_expanded_load_uses_viewport_buffer_and_sets_draw_flag(void) {
    unsigned char loadedGraphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char viewportBuffer[8] = {0};
    unsigned char bitmapStorage[6] = {0};
    unsigned char* bitmap = bitmapStorage + 4;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    struct MemoryLoadSessionResult_Compat result = {0};
    F0490_MEMORY_LoadGraphicSession_Compat(
        0,
        loadedGraphic,
        sizeof(loadedGraphic),
        viewportBuffer,
        bitmap,
        &sizeInfo,
        &result);
    return memcmp(viewportBuffer, loadedGraphic, sizeof(loadedGraphic)) == 0 &&
           bitmapStorage[0] == 0x02 && bitmapStorage[1] == 0x00 && bitmapStorage[2] == 0x01 && bitmapStorage[3] == 0x00 &&
           bitmap[0] == 0x22 && bitmap[1] == 0x00 &&
           result.loadTarget == viewportBuffer && result.usedViewportBuffer == 1 && result.drawFloorAndCeilingRequested == 1;
}

static int test_expanded_load_respects_do_not_copy_dimensions(void) {
    unsigned char loadedGraphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char viewportBuffer[8] = {0};
    unsigned char bitmapStorage[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0x00, 0x00};
    unsigned char* bitmap = bitmapStorage + 4;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    struct MemoryLoadSessionResult_Compat result = {0};
    F0490_MEMORY_LoadGraphicSession_Compat(
        MEMORY_LOAD_EXPAND_FLAG_DO_NOT_COPY_DIMENSIONS,
        loadedGraphic,
        sizeof(loadedGraphic),
        viewportBuffer,
        bitmap,
        &sizeInfo,
        &result);
    return bitmapStorage[0] == 0xAA && bitmapStorage[1] == 0xBB && bitmapStorage[2] == 0xCC && bitmapStorage[3] == 0xDD &&
           bitmap[0] == 0x22 && bitmap[1] == 0x00 &&
           result.usedViewportBuffer == 1 && result.drawFloorAndCeilingRequested == 1;
}

int main(void) {
    if (!test_not_expanded_loads_directly_to_destination()) {
        fprintf(stderr, "test_not_expanded_loads_directly_to_destination failed\n");
        return 1;
    }
    if (!test_expanded_load_uses_viewport_buffer_and_sets_draw_flag()) {
        fprintf(stderr, "test_expanded_load_uses_viewport_buffer_and_sets_draw_flag failed\n");
        return 1;
    }
    if (!test_expanded_load_respects_do_not_copy_dimensions()) {
        fprintf(stderr, "test_expanded_load_respects_do_not_copy_dimensions failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
