#include <stdio.h>
#include <string.h>

#include "endgame_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;


static const char* end_kind_name(EndgameCompatSourceEventKind kind) {
    switch (kind) {
    case ENDGAME_COMPAT_SOURCE_EVENT_OPTIONAL_LOSS_SCREAM_DELAY: return "OPTIONAL_LOSS_SCREAM_DELAY";
    case ENDGAME_COMPAT_SOURCE_EVENT_LOAD_END_ASSETS: return "LOAD_END_ASSETS";
    case ENDGAME_COMPAT_SOURCE_EVENT_FADE_TO_BLACK: return "FADE_TO_BLACK";
    case ENDGAME_COMPAT_SOURCE_EVENT_CHAMPION_SUMMARY_RENDER: return "CHAMPION_SUMMARY_RENDER";
    case ENDGAME_COMPAT_SOURCE_EVENT_CHAMPION_SUMMARY_WAIT_INPUT: return "CHAMPION_SUMMARY_WAIT_INPUT";
    case ENDGAME_COMPAT_SOURCE_EVENT_THE_END_BLIT: return "THE_END_BLIT";
    case ENDGAME_COMPAT_SOURCE_EVENT_THE_END_PALETTE_FADE: return "THE_END_PALETTE_FADE";
    case ENDGAME_COMPAT_SOURCE_EVENT_RESTART_DELAY: return "RESTART_DELAY";
    case ENDGAME_COMPAT_SOURCE_EVENT_RESTART_BUTTONS_RENDER: return "RESTART_BUTTONS_RENDER";
    case ENDGAME_COMPAT_SOURCE_EVENT_RESTART_WAIT: return "RESTART_WAIT";
    case ENDGAME_COMPAT_SOURCE_EVENT_CREDITS_FADE: return "CREDITS_FADE";
    }
    return "UNKNOWN";
}

static int test_endgame_source_animation_schedule(void) {
    unsigned int i;
    int ok = 1;
    EndgameCompatSourceAnimationStep theEnd;
    EndgameCompatSourceAnimationStep restartDelay;
    EndgameCompatSourceAnimationStep restartWait;
    memset(&theEnd, 0, sizeof(theEnd));
    memset(&restartDelay, 0, sizeof(restartDelay));
    memset(&restartWait, 0, sizeof(restartWait));
    printf("probe=firestaff_endgame_source_animation_schedule\n");
    printf("sourceAnimationEvidence=%s\n", ENDGAME_Compat_GetSourceAnimationEvidence());
    printf("sourceAnimationStepCount=%u\n", ENDGAME_Compat_GetSourceAnimationStepCount());
    if (ENDGAME_Compat_GetSourceAnimationStepCount() != 11u) ok = 0;
    for (i = 1u; i <= ENDGAME_Compat_GetSourceAnimationStepCount(); ++i) {
        EndgameCompatSourceAnimationStep step;
        if (!ENDGAME_Compat_GetSourceAnimationStep(i, &step)) {
            ok = 0;
            continue;
        }
        printf("sourceAnimationStep[%u]=kind:%s delay:%u vblankLoop:%u box:%u,%u,%u,%u evidence:%s\n",
               i,
               end_kind_name(step.kind),
               step.delayTicks,
               step.vblankLoopCount,
               step.x,
               step.y,
               step.width,
               step.height,
               step.sourceLineEvidence ? step.sourceLineEvidence : "");
        if (step.kind == ENDGAME_COMPAT_SOURCE_EVENT_THE_END_BLIT) theEnd = step;
        if (step.kind == ENDGAME_COMPAT_SOURCE_EVENT_RESTART_DELAY) restartDelay = step;
        if (step.kind == ENDGAME_COMPAT_SOURCE_EVENT_RESTART_WAIT) restartWait = step;
    }
    if (theEnd.x != 120u || theEnd.y != 95u || theEnd.width != 80u || theEnd.height != 14u) ok = 0;
    if (restartDelay.delayTicks != 300u) ok = 0;
    if (restartWait.vblankLoopCount != 900u) ok = 0;
    printf("endgameSourceAnimationScheduleInvariantOk=%d\n", ok);
    return ok;
}

static int test_endgame_credits_same_stride(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char dst[2] = {0x00, 0x00};
    ENDGAME_Compat_ExpandCreditsToScreenBitmap(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x00;
}

static int test_endgame_credits_padded(void) {
    unsigned char src[9] = {0x03, 0x00, 0x02, 0x00, 0x12, 0x34, 0x56, 0x91, 0xE1};
    unsigned char dst[4] = {0x00, 0x00, 0x00, 0x00};
    ENDGAME_Compat_ExpandCreditsToScreenBitmap(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x20 && dst[2] == 0x22 && dst[3] == 0x20;
}

int main(void) {
    if (!test_endgame_credits_same_stride()) {
        fprintf(stderr, "test_endgame_credits_same_stride failed\n");
        return 1;
    }
    if (!test_endgame_credits_padded()) {
        fprintf(stderr, "test_endgame_credits_padded failed\n");
        return 1;
    }
    if (!test_endgame_source_animation_schedule()) {
        fprintf(stderr, "test_endgame_source_animation_schedule failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
