#include "entrance_frontend_pc34_compat.h"
#include <string.h>

#define ENTRANCE_COMPAT_SCREEN_WIDTH 320u
#define ENTRANCE_COMPAT_SCREEN_HEIGHT 200u
#define ENTRANCE_COMPAT_DOOR_SCREEN_Y 28u
#define ENTRANCE_COMPAT_RUNTIME_VIEWPORT_SOURCE_X 0u
#define ENTRANCE_COMPAT_RUNTIME_VIEWPORT_SOURCE_Y 33u
#define ENTRANCE_COMPAT_DUNGEON_VIEW_X 0u
#define ENTRANCE_COMPAT_DUNGEON_VIEW_Y 31u
#define ENTRANCE_COMPAT_DUNGEON_VIEW_WIDTH 224u
#define ENTRANCE_COMPAT_DUNGEON_VIEW_HEIGHT 136u

static int entrance_copy_rect(unsigned char* dst,
                              unsigned int dstW,
                              unsigned int dstH,
                              unsigned int dstX,
                              unsigned int dstY,
                              const unsigned char* src,
                              unsigned int srcW,
                              unsigned int srcH,
                              unsigned int srcX,
                              unsigned int srcY,
                              unsigned int w,
                              unsigned int h) {
    unsigned int row;
    if (!dst || !src || dstW == 0u || dstH == 0u ||
        srcW == 0u || srcH == 0u || w == 0u || h == 0u) {
        return 0;
    }
    if (srcX >= srcW || srcY >= srcH || dstX >= dstW || dstY >= dstH) {
        return 0;
    }
    if (w > srcW - srcX) w = srcW - srcX;
    if (h > srcH - srcY) h = srcH - srcY;
    if (w > dstW - dstX) w = dstW - dstX;
    if (h > dstH - dstY) h = dstH - dstY;
    if (w == 0u || h == 0u) {
        return 0;
    }
    for (row = 0u; row < h; ++row) {
        memcpy(dst + (size_t)(dstY + row) * (size_t)dstW + (size_t)dstX,
               src + (size_t)(srcY + row) * (size_t)srcW + (size_t)srcX,
               (size_t)w);
    }
    return 1;
}

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

unsigned int ENTRANCE_Compat_GetRuntimeDelayMs(const EntranceCompatSourceAnimationStep* step) {
    if (!step) return 0u;
    /* ReDMCSB ENTRANCE.C PC/F20 timing source-lock:
     * - ENTRANCE.C:850-883 waits on M526_WaitVerticalBlank() while the
     *   entrance waits for input.
     * - ENTRANCE.C:935 calls F0022_MAIN_Delay(20) before door opening.
     * - ENTRANCE.C:239 guards each F0438 door animation step with one
     *   M526_WaitVerticalBlank().
     * Runtime expresses those source ticks as 20 ms vblank slots, matching
     * the TITLE frontend cadence helper used for TITLE.C. */
    if (step->delayTicks > 0u) {
        return step->delayTicks * 20u;
    }
    if (step->vblankLoopCount > 0u) {
        return step->vblankLoopCount * 20u;
    }
    return 0u;
}

const char* ENTRANCE_Compat_GetSourceAnimationEvidence(void) {
    return "ReDMCSB ENTRANCE.C PC/F20 path: draw entrance micro-dungeon, fade/curtain to entrance palette, draw C004 entrance screen, wait on VBlank/input loop, play switch sound, F0022_MAIN_Delay(20), hide pointer, then F0438 opens doors in source animation steps 1..31 with a BUG0_71 one-VBlank guard per step; rattle sound fires when step%3==1; door boxes move 4px per step from DATA.C left {0,100,0,160} and right {109,231,0,160}.";
}

int ENTRANCE_Compat_CompositeDoorOpeningFrame(unsigned char* framebuffer,
                                              unsigned int framebufferWidth,
                                              unsigned int framebufferHeight,
                                              const EntranceCompatCompositePixels* pixels,
                                              const EntranceCompatDoorStep* door) {
    int drewDoor = 0;
    if (!framebuffer || !pixels || !door || !pixels->entranceScreen || !pixels->dungeonFrame) {
        return 0;
    }
    if (pixels->entranceWidth < ENTRANCE_COMPAT_SCREEN_WIDTH ||
        pixels->entranceHeight < ENTRANCE_COMPAT_SCREEN_HEIGHT ||
        pixels->dungeonFrameWidth < ENTRANCE_COMPAT_RUNTIME_VIEWPORT_SOURCE_X + ENTRANCE_COMPAT_DUNGEON_VIEW_WIDTH ||
        pixels->dungeonFrameHeight < ENTRANCE_COMPAT_RUNTIME_VIEWPORT_SOURCE_Y + ENTRANCE_COMPAT_DUNGEON_VIEW_HEIGHT) {
        return 0;
    }

    /* ReDMCSB ENTRANCE.C:175 copies the saved C004+door background into the
     * composite bitmap, line 184 blits only G0296_puc_Bitmap_Viewport behind
     * the doors, and lines 189-231 overlay moving door strips.  Firestaff's
     * runtime dungeon frame is a full 320x200 image, so this source-locked
     * composite extracts just the 224x136 viewport from y=33 into the entrance
     * door aperture at y=28+3. */
    if (!entrance_copy_rect(framebuffer,
                            framebufferWidth,
                            framebufferHeight,
                            0u,
                            0u,
                            pixels->entranceScreen,
                            pixels->entranceWidth,
                            pixels->entranceHeight,
                            0u,
                            0u,
                            ENTRANCE_COMPAT_SCREEN_WIDTH,
                            ENTRANCE_COMPAT_SCREEN_HEIGHT)) {
        return 0;
    }
    if (!entrance_copy_rect(framebuffer,
                            framebufferWidth,
                            framebufferHeight,
                            ENTRANCE_COMPAT_DUNGEON_VIEW_X,
                            ENTRANCE_COMPAT_DUNGEON_VIEW_Y,
                            pixels->dungeonFrame,
                            pixels->dungeonFrameWidth,
                            pixels->dungeonFrameHeight,
                            ENTRANCE_COMPAT_RUNTIME_VIEWPORT_SOURCE_X,
                            ENTRANCE_COMPAT_RUNTIME_VIEWPORT_SOURCE_Y,
                            ENTRANCE_COMPAT_DUNGEON_VIEW_WIDTH,
                            ENTRANCE_COMPAT_DUNGEON_VIEW_HEIGHT)) {
        return 0;
    }
    if (door->leftBoxW > 0u) {
        if (!pixels->leftDoor ||
            pixels->leftDoorWidth < door->leftSourceX + door->leftBoxW ||
            pixels->leftDoorHeight < door->leftBoxY + door->leftBoxH) {
            return 0;
        }
        if (!entrance_copy_rect(framebuffer,
                                framebufferWidth,
                                framebufferHeight,
                                door->leftBoxX,
                                ENTRANCE_COMPAT_DOOR_SCREEN_Y + door->leftBoxY,
                                pixels->leftDoor,
                                pixels->leftDoorWidth,
                                pixels->leftDoorHeight,
                                door->leftSourceX,
                                door->leftBoxY,
                                door->leftBoxW,
                                door->leftBoxH)) {
            return 0;
        }
        drewDoor = 1;
    }
    if (door->rightBoxW > 0u) {
        if (!pixels->rightDoor ||
            pixels->rightDoorWidth < door->rightSourceX + door->rightBoxW ||
            pixels->rightDoorHeight < door->rightBoxY + door->rightBoxH) {
            return 0;
        }
        if (!entrance_copy_rect(framebuffer,
                                framebufferWidth,
                                framebufferHeight,
                                door->rightBoxX,
                                ENTRANCE_COMPAT_DOOR_SCREEN_Y + door->rightBoxY,
                                pixels->rightDoor,
                                pixels->rightDoorWidth,
                                pixels->rightDoorHeight,
                                door->rightSourceX,
                                door->rightBoxY,
                                door->rightBoxW,
                                door->rightBoxH)) {
            return 0;
        }
        drewDoor = 1;
    }
    return drewDoor || door->animationStep > 0u;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602 — Remaining ENTRANCE.C function citations for parity
 *
 *   ENTRANCE.C:600 F0440_STARTEND_G
 *   ENTRANCE.C:966 F0442_STARTEND_P
 *   ENTRANCE.C:694 F0707_R
 *   ENTRANCE.C:166 F0709_S
 *   ENTRANCE.C:363 F0710_W
 *   ENTRANCE.C:1079 F0712_A
 *   ENTRANCE.C:733 F0741_MUSIC_P (platform-specific, not implemented for PC-34)
 *   ENTRANCE.C:688 F0771_FILE_C (platform-specific, not implemented for PC-34)
 *   ENTRANCE.C:674 F0772_FILE_R (platform-specific, not implemented for PC-34)
 *   ENTRANCE.C:672 F0774_FILE_S (platform-specific, not implemented for PC-34)
 *   ENTRANCE.C:671 F0775_FILE_G (platform-specific, not implemented for PC-34)
 *   ENTRANCE.C:666 F0780_FILE_I (platform-specific, not implemented for PC-34)
 *   ENTRANCE.C:727 F0785_S
 *   ENTRANCE.C:842 F0813_P
 *   ENTRANCE.C:923 F0814_TR
 *   ENTRANCE.C:955 F0815_S
 *   ENTRANCE.C:1017 F1028_F
 *   ENTRANCE.C:1074 F2158_G
 *   ENTRANCE.C:424 F2163_S
 * ══════════════════════════════════════════════════════════════════════ */
