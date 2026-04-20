#ifndef REDMCSB_BOOT_PROGRAM_RUNTIME_PC34_COMPAT_H
#define REDMCSB_BOOT_PROGRAM_RUNTIME_PC34_COMPAT_H

struct BootProgramStep_Compat {
    unsigned int viewportGraphicIndex;
};

struct BootProgramRuntimeResult_Compat {
    unsigned int requestedStepCount;
    unsigned int completedStepCount;
    unsigned int skippedStepCount;
    unsigned int skippedPlaceholderStepCount;
    unsigned int skippedSpecialStepCount;
    unsigned int publishedFrameCount;
    unsigned int firstFrameNumber;
    int bootProgramCompleted;
};

int F9007_RUNTIME_RunBootProgram_Compat(
    const char* graphicsDatPath,
    const char* outputPrefix,
    unsigned int dialogGraphicIndex,
    unsigned int firstFrameNumber,
    const struct BootProgramStep_Compat* steps,
    unsigned int stepCount,
    struct BootProgramRuntimeResult_Compat* outResult);

#endif
