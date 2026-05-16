#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "host_video_pgm_backend_pc34_compat.h"
#include "screen_bitmap_export_pgm_pc34_compat.h"

int F9002_HOSTVIDEO_PublishFrameToPgm_Compat(
const unsigned char*                       screenBitmap SEPARATOR
unsigned int                               frameNumber  SEPARATOR
const char*                                outputPath   SEPARATOR
struct HostVideoPgmBackendResult_Compat*   outResult    FINAL_SEPARATOR
{
        struct ScreenBitmapExportPgmResult_Compat exportResult;


        memset(&exportResult, 0, sizeof(exportResult));
        if (!F9001_SCREEN_ExportBitmapToPgm_Compat(screenBitmap, outputPath, &exportResult)) {
                return 0;
        }
        if (outResult != 0) {
                memset(outResult, 0, sizeof(*outResult));
                outResult->frameNumber = frameNumber;
                outResult->width = exportResult.width;
                outResult->height = exportResult.height;
                outResult->pixelCount = exportResult.pixelCount;
                outResult->outputPath = outputPath;
        }
        return 1;
}
