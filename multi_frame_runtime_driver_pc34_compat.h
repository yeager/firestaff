#ifndef REDMCSB_MULTI_FRAME_RUNTIME_DRIVER_PC34_COMPAT_H
#define REDMCSB_MULTI_FRAME_RUNTIME_DRIVER_PC34_COMPAT_H

struct MultiFrameRuntimeDriverResult_Compat {
    unsigned int requestedFrameCount;
    unsigned int completedFrameCount;
    unsigned int skippedFrameCount;
    unsigned int skippedPlaceholderFrameCount;
    unsigned int skippedSpecialFrameCount;
    unsigned int publishedFrameCount;
    unsigned int firstFrameNumber;
    const char* outputPrefix;
};

int F9004_RUNTIME_RunStartupFrameSequence_Compat(
    const char* graphicsDatPath,
    const char* outputPrefix,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned int firstFrameNumber,
    unsigned int frameCount,
    struct MultiFrameRuntimeDriverResult_Compat* outResult);

#endif
