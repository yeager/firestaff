#ifndef REDMCSB_SWSH_FRONTEND_PC34_COMPAT_H
#define REDMCSB_SWSH_FRONTEND_PC34_COMPAT_H

typedef enum SWSH_CompatSourceEventKind {
    SWSH_COMPAT_SOURCE_EVENT_LOAD_LOGO_BITMAP = 0,
    SWSH_COMPAT_SOURCE_EVENT_START_SOUND = 1,
    SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR = 2,
    SWSH_COMPAT_SOURCE_EVENT_WAIT_VBLANKS = 3,
    SWSH_COMPAT_SOURCE_EVENT_RUN_START_PROGRAM = 4
} SWSH_CompatSourceEventKind;

typedef struct SWSH_CompatSourceAnimationStep {
    unsigned int sourceStepOrdinal;
    SWSH_CompatSourceEventKind kind;
    unsigned int colorIndex;
    unsigned int colorValue;
    unsigned int vblankCount;
    unsigned int sourceLine;
    const char* sourceLineEvidence;
} SWSH_CompatSourceAnimationStep;

typedef struct SWSH_CompatSourceTiming {
    unsigned int paletteCommandCount;
    unsigned int paletteColorSetCount;
    unsigned int paletteWaitCommandCount;
    unsigned int paletteWaitVblankCount;
    unsigned int soundRegisterWriteCount;
    unsigned int soundWaitCommandCount;
    unsigned int soundWaitVblankCount;
    const char* sourceFile;
    const char* sourceFunction;
    const char* evidenceNote;
} SWSH_CompatSourceTiming;

void SWSH_Compat_ExpandLogoToBitmap(const unsigned char* graphic, unsigned char* bitmap);
unsigned int SWSH_Compat_GetSourceAnimationStepCount(void);
int SWSH_Compat_GetSourceAnimationStep(unsigned int sourceStepOrdinal,
                                       SWSH_CompatSourceAnimationStep* outStep);
SWSH_CompatSourceTiming SWSH_Compat_GetSourceTimingEvidence(void);
const char* SWSH_Compat_GetSourceAnimationEvidence(void);

#define SWSH_COMPAT_SOURCE_PALETTE_COMMAND_COUNT 26u
#define SWSH_COMPAT_SOURCE_PALETTE_COLOR_SET_COUNT 17u
#define SWSH_COMPAT_SOURCE_PALETTE_WAIT_COMMAND_COUNT 9u
#define SWSH_COMPAT_SOURCE_PALETTE_WAIT_VBLANK_COUNT 21u
#define SWSH_COMPAT_SOURCE_SOUND_REGISTER_WRITE_COUNT 17u
#define SWSH_COMPAT_SOURCE_SOUND_WAIT_COMMAND_COUNT 10u
#define SWSH_COMPAT_SOURCE_SOUND_WAIT_VBLANK_COUNT 20u

#endif
