#include <stdio.h>
#include <string.h>

#include "memory_load_wrappers_transaction_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_temporary_graphic_transaction_returns_bytecount_and_direct_load(void) {
    unsigned short languageSpecificGraphicIndices[4] = {0, 1, 2, 3};
    unsigned short byteCounts[4] = {8, 12, 16, 20};
    unsigned char loadedGraphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char destination[16] = {0};
    unsigned char* outGraphic = 0;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    struct MemoryLoadTransactionResult_Compat result = {0};
    long byteCount = F0440_STARTEND_LoadTemporaryGraphicTransaction_Compat(0, languageSpecificGraphicIndices, byteCounts, loadedGraphic, &outGraphic, destination, &sizeInfo, &result);
    return byteCount == 8 && outGraphic == destination && memcmp(destination, loadedGraphic, 8) == 0 &&
           result.musicStopped == 1 && result.graphicsOpened == 1 && result.graphicsClosed == 1 && result.session.usedViewportBuffer == 0;
}

static int test_endgame_transaction_returns_bitmap_and_uses_viewport_path(void) {
    unsigned char loadedGraphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char storage[6] = {0};
    unsigned char* bitmap = storage + 4;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    struct MemoryLoadTransactionResult_Compat result = {0};
    unsigned char* out = F0763_LoadEndgameBitmapExpandedTransaction_Compat(loadedGraphic, sizeof(loadedGraphic), bitmap, &sizeInfo, &result);
    return out == bitmap && storage[0] == 0x02 && storage[1] == 0x00 && storage[2] == 0x01 && storage[3] == 0x00 &&
           bitmap[0] == 0x22 && bitmap[1] == 0x00 && result.session.usedViewportBuffer == 1 && result.session.drawFloorAndCeilingRequested == 1;
}

int main(void) {
    if (!test_temporary_graphic_transaction_returns_bytecount_and_direct_load()) {
        fprintf(stderr, "test_temporary_graphic_transaction_returns_bytecount_and_direct_load failed\n");
        return 1;
    }
    if (!test_endgame_transaction_returns_bitmap_and_uses_viewport_path()) {
        fprintf(stderr, "test_endgame_transaction_returns_bitmap_and_uses_viewport_path failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
