#ifndef REDMCSB_ENTRANCE_FRONTEND_PC34_COMPAT_H
#define REDMCSB_ENTRANCE_FRONTEND_PC34_COMPAT_H

#include <stddef.h>

typedef enum EntranceCompatRuntimeCommandId {
    ENTRANCE_COMPAT_RUNTIME_COMMAND_NONE = 0,
    ENTRANCE_COMPAT_RUNTIME_COMMAND_ENTER_DUNGEON = 200,
    ENTRANCE_COMPAT_RUNTIME_COMMAND_ENTER_BONUS_DUNGEON = 201,
    ENTRANCE_COMPAT_RUNTIME_COMMAND_RESUME = 202,
    ENTRANCE_COMPAT_RUNTIME_COMMAND_DRAW_CREDITS = 203,
    ENTRANCE_COMPAT_RUNTIME_COMMAND_QUIT = 216
} EntranceCompatRuntimeCommandId;

typedef enum EntranceCompatCommandPath {
    ENTRANCE_COMPAT_COMMAND_PATH_QUIT = -1,
    ENTRANCE_COMPAT_COMMAND_PATH_NONE = 0,
    ENTRANCE_COMPAT_COMMAND_PATH_ENTER = 1,
    ENTRANCE_COMPAT_COMMAND_PATH_RESUME = 2,
    ENTRANCE_COMPAT_COMMAND_PATH_CREDITS = 3
} EntranceCompatCommandPath;

typedef enum EntranceCompatKey {
    ENTRANCE_COMPAT_KEY_OTHER = 0,
    ENTRANCE_COMPAT_KEY_RETURN = 1,
    ENTRANCE_COMPAT_KEY_KEYPAD_RETURN = 2,
    ENTRANCE_COMPAT_KEY_ESCAPE = 3,
    ENTRANCE_COMPAT_KEY_Q = 4,
    ENTRANCE_COMPAT_KEY_SPACE = 5
} EntranceCompatKey;

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

typedef struct EntranceCompatCompositePixels {
    const unsigned char* entranceScreen;
    unsigned int entranceWidth;
    unsigned int entranceHeight;
    const unsigned char* dungeonFrame;
    unsigned int dungeonFrameWidth;
    unsigned int dungeonFrameHeight;
    const unsigned char* leftDoor;
    unsigned int leftDoorWidth;
    unsigned int leftDoorHeight;
    const unsigned char* rightDoor;
    unsigned int rightDoorWidth;
    unsigned int rightDoorHeight;
} EntranceCompatCompositePixels;

unsigned int ENTRANCE_Compat_GetDoorAnimationStepCount(void);
int ENTRANCE_Compat_GetDoorAnimationStep(unsigned int animationStep,
                                         EntranceCompatDoorStep* outStep);
unsigned int ENTRANCE_Compat_GetSourceAnimationStepCount(void);
int ENTRANCE_Compat_GetSourceAnimationStep(unsigned int sourceStepOrdinal,
                                           EntranceCompatSourceAnimationStep* outStep);
unsigned int ENTRANCE_Compat_GetRuntimeDelayMs(const EntranceCompatSourceAnimationStep* step);
unsigned int ENTRANCE_Compat_GetVblankDelayMs(void);
unsigned int ENTRANCE_Compat_GetCreditsWaitTicks(void);
int ENTRANCE_Compat_DispatchKeyCommand(EntranceCompatKey key);
EntranceCompatCommandPath ENTRANCE_Compat_CommandPathFromSourceCommand(int commandId);
int ENTRANCE_Compat_ShouldAutoEnterForTimeout(int allowHeadlessTimeout,
                                              int autoEnterAfterMs,
                                              unsigned long long elapsedMs);
int ENTRANCE_Compat_ResolveDm1ResumeSavePath(const char* sourceId,
                                             int quickResumeAvailable,
                                             const char* quickResumeGameId,
                                             const char* quickResumeSavePath,
                                             char* outPath,
                                             size_t outPathBytes);
const char* ENTRANCE_Compat_GetSourceAnimationEvidence(void);
const char* ENTRANCE_Compat_GetRuntimeCommandEvidence(void);
int ENTRANCE_Compat_CompositeDoorOpeningFrame(unsigned char* framebuffer,
                                              unsigned int framebufferWidth,
                                              unsigned int framebufferHeight,
                                              const EntranceCompatCompositePixels* pixels,
                                              const EntranceCompatDoorStep* door);

#endif
