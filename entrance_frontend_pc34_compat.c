#include "entrance_frontend_pc34_compat.h"
#include <string.h>

unsigned int ENTRANCE_Compat_GetDoorAnimationStepCount(void) {
    return 31u;
}

int ENTRANCE_Compat_GetDoorAnimationStep(unsigned int animationStep,
                                         EntranceCompatDoorStep* outStep) {
    EntranceCompatDoorStep step;
    unsigned int leftRight;
    unsigned int rightLeft;
    if (!outStep || animationStep < 1u || animationStep >= 32u) return 0;
    memset(&step, 0, sizeof(step));
    step.animationStep = animationStep;
    step.soundRattle = ((animationStep % 3u) == 1u) ? 1u : 0u;
    step.vblankBeforeCopy = 1u;
    leftRight = 100u - 4u * (animationStep - 1u);
    rightLeft = 109u + 4u * (animationStep - 1u);
    if ((int)leftRight >= 0) {
        step.leftBoxX = 0u;
        step.leftBoxY = 0u;
        step.leftBoxW = leftRight + 1u;
        step.leftBoxH = 161u;
        step.leftSourceX = (animationStep & 0x00fcu) << 2;
    }
    if (rightLeft <= 231u) {
        step.rightBoxX = rightLeft;
        step.rightBoxY = 0u;
        step.rightBoxW = 231u - rightLeft + 1u;
        step.rightBoxH = 161u;
        step.rightSourceX = (animationStep & 0x0003u) << 2;
    }
    step.sourceLineEvidence = "ENTRANCE.C:142-304 loops animationStep=1..31; lines 152-168 rattle every step%3==1; lines 189-231 move door boxes by 4px; line 239 waits one VBlank per step";
    *outStep = step;
    return 1;
}

static const char* entrance_event_line(EntranceCompatSourceEventKind kind) {
    switch (kind) {
    case ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_MICRO_DUNGEON:
        return "ENTRANCE.C:409-421 builds 5x5 micro dungeon behind doors";
    case ENTRANCE_COMPAT_SOURCE_EVENT_FADE_TO_BLACK:
        return "ENTRANCE.C:426-443 fades/curtains to entrance black/palette";
    case ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_ENTRANCE_SCREEN:
        return "ENTRANCE.C:446-595 draws C004 entrance screen, door bitmaps, buttons, palette";
    case ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT:
        return "ENTRANCE.C:850-883 draw entrance, discard input, wait on VBlank loop until command";
    case ENTRANCE_COMPAT_SOURCE_EVENT_SWITCH_SOUND:
        return "ENTRANCE.C:906-934 plays switch sound after Enter/command";
    case ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY:
        return "ENTRANCE.C:935 waits F0022_MAIN_Delay(20), then hides pointer";
    case ENTRANCE_COMPAT_SOURCE_EVENT_OPEN_DOOR_STEP:
        return "ENTRANCE.C:142-304 opens doors in 31 source steps with one M526_WaitVerticalBlank() guard each";
    case ENTRANCE_COMPAT_SOURCE_EVENT_FINAL_DUNGEON_VIEW:
        return "ENTRANCE.C:362-368 waits sound complete/draws micro dungeon or final viewport after door loop";
    }
    return "ENTRANCE.C source event";
}

unsigned int ENTRANCE_Compat_GetSourceAnimationStepCount(void) {
    return 38u;
}

int ENTRANCE_Compat_GetSourceAnimationStep(unsigned int sourceStepOrdinal,
                                           EntranceCompatSourceAnimationStep* outStep) {
    EntranceCompatSourceAnimationStep step;
    if (!outStep || sourceStepOrdinal == 0u || sourceStepOrdinal > ENTRANCE_Compat_GetSourceAnimationStepCount()) return 0;
    memset(&step, 0, sizeof(step));
    step.sourceStepOrdinal = sourceStepOrdinal;
    if (sourceStepOrdinal == 1u) {
        step.kind = ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_MICRO_DUNGEON;
        step.x = 0u; step.y = 3u; step.width = 224u; step.height = 136u;
    } else if (sourceStepOrdinal == 2u) {
        step.kind = ENTRANCE_COMPAT_SOURCE_EVENT_FADE_TO_BLACK;
    } else if (sourceStepOrdinal == 3u) {
        step.kind = ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_ENTRANCE_SCREEN;
        step.x = 0u; step.y = 0u; step.width = 320u; step.height = 200u;
    } else if (sourceStepOrdinal == 4u) {
        step.kind = ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT;
        step.vblankLoopCount = 1u;
    } else if (sourceStepOrdinal == 5u) {
        step.kind = ENTRANCE_COMPAT_SOURCE_EVENT_SWITCH_SOUND;
    } else if (sourceStepOrdinal == 6u) {
        step.kind = ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY;
        step.delayTicks = 20u;
    } else if (sourceStepOrdinal >= 7u && sourceStepOrdinal <= 37u) {
        EntranceCompatDoorStep door;
        (void)ENTRANCE_Compat_GetDoorAnimationStep(sourceStepOrdinal - 6u, &door);
        step.kind = ENTRANCE_COMPAT_SOURCE_EVENT_OPEN_DOOR_STEP;
        step.vblankLoopCount = 1u;
        step.x = 0u; step.y = 28u; step.width = 228u; step.height = 161u;
    } else {
        step.kind = ENTRANCE_COMPAT_SOURCE_EVENT_FINAL_DUNGEON_VIEW;
        step.x = 0u; step.y = 33u; step.width = 224u; step.height = 136u;
    }
    step.sourceLineEvidence = entrance_event_line(step.kind);
    *outStep = step;
    return 1;
}

const char* ENTRANCE_Compat_GetSourceAnimationEvidence(void) {
    return "ReDMCSB ENTRANCE.C PC/F20 path: draw entrance micro-dungeon, fade/curtain to entrance palette, draw C004 entrance screen, wait on VBlank/input loop, play switch sound, F0022_MAIN_Delay(20), hide pointer, then F0438 opens doors in source animation steps 1..31 with a BUG0_71 one-VBlank guard per step; rattle sound fires when step%3==1; door boxes move 4px per step from DATA.C left {0,100,0,160} and right {109,231,0,160}.";
}
