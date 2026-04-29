#ifndef REDMCSB_ENTRANCE_FRONTEND_PC34_COMPAT_H
#define REDMCSB_ENTRANCE_FRONTEND_PC34_COMPAT_H

typedef enum EntranceCompatSourceEventKind {
    ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_MICRO_DUNGEON = 0,
    ENTRANCE_COMPAT_SOURCE_EVENT_FADE_TO_BLACK = 1,
    ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_ENTRANCE_SCREEN = 2,
    ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT = 3,
    ENTRANCE_COMPAT_SOURCE_EVENT_SWITCH_SOUND = 4,
    ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY = 5,
    ENTRANCE_COMPAT_SOURCE_EVENT_OPEN_DOOR_STEP = 6,
    ENTRANCE_COMPAT_SOURCE_EVENT_FINAL_DUNGEON_VIEW = 7
} EntranceCompatSourceEventKind;

typedef struct EntranceCompatDoorStep {
    unsigned int animationStep;
    unsigned int soundRattle;
    unsigned int vblankBeforeCopy;
    unsigned int leftBoxX;
    unsigned int leftBoxY;
    unsigned int leftBoxW;
    unsigned int leftBoxH;
    unsigned int rightBoxX;
    unsigned int rightBoxY;
    unsigned int rightBoxW;
    unsigned int rightBoxH;
    unsigned int leftSourceX;
    unsigned int rightSourceX;
    const char* sourceLineEvidence;
} EntranceCompatDoorStep;

typedef struct EntranceCompatSourceAnimationStep {
    unsigned int sourceStepOrdinal;
    EntranceCompatSourceEventKind kind;
    unsigned int delayTicks;
    unsigned int vblankLoopCount;
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    const char* sourceLineEvidence;
} EntranceCompatSourceAnimationStep;

unsigned int ENTRANCE_Compat_GetDoorAnimationStepCount(void);
int ENTRANCE_Compat_GetDoorAnimationStep(unsigned int animationStep,
                                         EntranceCompatDoorStep* outStep);
unsigned int ENTRANCE_Compat_GetSourceAnimationStepCount(void);
int ENTRANCE_Compat_GetSourceAnimationStep(unsigned int sourceStepOrdinal,
                                           EntranceCompatSourceAnimationStep* outStep);
const char* ENTRANCE_Compat_GetSourceAnimationEvidence(void);

#endif
