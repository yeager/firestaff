#include <stdio.h>
#include <string.h>

#include "memory_load_pipeline_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_pipeline_temporary_mode_returns_bytecount_and_output(void) {
    unsigned short languageSpecificGraphicIndices[4] = {0, 1, 2, 3};
    unsigned short byteCounts[4] = {8, 12, 16, 20};
    unsigned char loadedGraphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char destination[16] = {0};
    unsigned char* outGraphic = 0;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    struct MemoryLoadPipelineResult_Compat result = {0};
    return MEMORY_LOAD_RunPipeline_Compat(MEMORY_LOAD_PIPELINE_MODE_TEMPORARY, 0, languageSpecificGraphicIndices, byteCounts, loadedGraphic, sizeof(loadedGraphic), &outGraphic, destination, &sizeInfo, &result) == 1 &&
           result.byteCount == 8 && result.output == destination && outGraphic == destination &&
           result.transaction.session.usedViewportBuffer == 0;
}

static int test_pipeline_endgame_mode_returns_bitmap_and_transaction(void) {
    unsigned char loadedGraphic[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char storage[6] = {0};
    unsigned char* bitmap = storage + 4;
    unsigned char* outGraphic = 0;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    struct MemoryLoadPipelineResult_Compat result = {0};
    return MEMORY_LOAD_RunPipeline_Compat(MEMORY_LOAD_PIPELINE_MODE_ENDGAME_BITMAP, 0, 0, 0, loadedGraphic, sizeof(loadedGraphic), &outGraphic, bitmap, &sizeInfo, &result) == 1 &&
           result.byteCount == 8 && result.output == bitmap && outGraphic == bitmap && bitmap[0] == 0x22 && bitmap[1] == 0x00 &&
           result.transaction.session.usedViewportBuffer == 1;
}

static int test_pipeline_rejects_unknown_mode(void) {
    struct MemoryLoadPipelineResult_Compat result = {0};
    return MEMORY_LOAD_RunPipeline_Compat(999, 0, 0, 0, 0, 0, 0, 0, 0, &result) == 0;
}

int main(void) {
    if (!test_pipeline_temporary_mode_returns_bytecount_and_output()) {
        fprintf(stderr, "test_pipeline_temporary_mode_returns_bytecount_and_output failed\n");
        return 1;
    }
    if (!test_pipeline_endgame_mode_returns_bitmap_and_transaction()) {
        fprintf(stderr, "test_pipeline_endgame_mode_returns_bitmap_and_transaction failed\n");
        return 1;
    }
    if (!test_pipeline_rejects_unknown_mode()) {
        fprintf(stderr, "test_pipeline_rejects_unknown_mode failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
