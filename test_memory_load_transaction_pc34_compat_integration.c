#include <stdio.h>
#include <string.h>

#include "memory_load_transaction_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_transaction_wraps_direct_not_expanded_path(void) {
    unsigned char loadedGraphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char viewportBuffer[8] = {0};
    unsigned char destination[8] = {0};
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    struct MemoryLoadTransactionResult_Compat result = {0};
    F0490_MEMORY_RunLoadGraphicTransaction_Compat(
        MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED | MEMORY_LOAD_EXPAND_FLAG_DO_NOT_COPY_DIMENSIONS,
        loadedGraphic,
        sizeof(loadedGraphic),
        viewportBuffer,
        destination,
        &sizeInfo,
        &result);
    return result.musicStopped == 1 && result.graphicsOpened == 1 && result.graphicsClosed == 1 &&
           result.session.usedViewportBuffer == 0 && result.session.drawFloorAndCeilingRequested == 0 &&
           memcmp(destination, loadedGraphic, sizeof(loadedGraphic)) == 0;
}

static int test_transaction_wraps_expanded_viewport_path(void) {
    unsigned char loadedGraphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char viewportBuffer[8] = {0};
    unsigned char bitmapStorage[6] = {0};
    unsigned char* bitmap = bitmapStorage + 4;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    struct MemoryLoadTransactionResult_Compat result = {0};
    F0490_MEMORY_RunLoadGraphicTransaction_Compat(
        0,
        loadedGraphic,
        sizeof(loadedGraphic),
        viewportBuffer,
        bitmap,
        &sizeInfo,
        &result);
    return result.musicStopped == 1 && result.graphicsOpened == 1 && result.graphicsClosed == 1 &&
           result.session.usedViewportBuffer == 1 && result.session.drawFloorAndCeilingRequested == 1 &&
           memcmp(viewportBuffer, loadedGraphic, sizeof(loadedGraphic)) == 0 && bitmap[0] == 0x22 && bitmap[1] == 0x00;
}

int main(void) {
    if (!test_transaction_wraps_direct_not_expanded_path()) {
        fprintf(stderr, "test_transaction_wraps_direct_not_expanded_path failed\n");
        return 1;
    }
    if (!test_transaction_wraps_expanded_viewport_path()) {
        fprintf(stderr, "test_transaction_wraps_expanded_viewport_path failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
