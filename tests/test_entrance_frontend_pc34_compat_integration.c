#include <stdio.h>
#include <string.h>
#include "entrance_frontend_pc34_compat.h"

static const char* kind_name(EntranceCompatSourceEventKind kind) {
    switch (kind) {
    case ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_MICRO_DUNGEON: return "DRAW_MICRO_DUNGEON";
    case ENTRANCE_COMPAT_SOURCE_EVENT_FADE_TO_BLACK: return "FADE_TO_BLACK";
    case ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_ENTRANCE_SCREEN: return "DRAW_ENTRANCE_SCREEN";
    case ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT: return "WAIT_FOR_INPUT";
    case ENTRANCE_COMPAT_SOURCE_EVENT_SWITCH_SOUND: return "SWITCH_SOUND";
    case ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY: return "PRE_OPEN_DELAY";
    case ENTRANCE_COMPAT_SOURCE_EVENT_OPEN_DOOR_STEP: return "OPEN_DOOR_STEP";
    case ENTRANCE_COMPAT_SOURCE_EVENT_FINAL_DUNGEON_VIEW: return "FINAL_DUNGEON_VIEW";
    }
    return "UNKNOWN";
}

int main(void) {
    unsigned int i;
    unsigned int rattleCount = 0u;
    unsigned int vblankDoorCount = 0u;
    int ok = 1;
    EntranceCompatDoorStep first;
    EntranceCompatDoorStep last;
    memset(&first, 0, sizeof(first));
    memset(&last, 0, sizeof(last));
    printf("probe=firestaff_entrance_source_animation_schedule\n");
    printf("sourceAnimationEvidence=%s\n", ENTRANCE_Compat_GetSourceAnimationEvidence());
    printf("doorAnimationStepCount=%u\n", ENTRANCE_Compat_GetDoorAnimationStepCount());
    if (ENTRANCE_Compat_GetDoorAnimationStepCount() != 31u) ok = 0;
    for (i = 1u; i <= ENTRANCE_Compat_GetDoorAnimationStepCount(); ++i) {
        EntranceCompatDoorStep step;
        if (!ENTRANCE_Compat_GetDoorAnimationStep(i, &step)) { ok = 0; continue; }
        if (i == 1u) first = step;
        if (i == 31u) last = step;
        if (step.soundRattle) rattleCount++;
        if (step.vblankBeforeCopy) vblankDoorCount++;
        printf("doorStep[%u]=rattle:%u vblank:%u left:%u,%u,%u,%u right:%u,%u,%u,%u src:%u,%u evidence:%s\n",
               i, step.soundRattle, step.vblankBeforeCopy,
               step.leftBoxX, step.leftBoxY, step.leftBoxW, step.leftBoxH,
               step.rightBoxX, step.rightBoxY, step.rightBoxW, step.rightBoxH,
               step.leftSourceX, step.rightSourceX,
               step.sourceLineEvidence ? step.sourceLineEvidence : "");
    }
    printf("doorRattleCount=%u\n", rattleCount);
    printf("doorVblankCount=%u\n", vblankDoorCount);
    if (rattleCount != 11u || vblankDoorCount != 31u) ok = 0;
    if (first.leftBoxW != 101u || first.rightBoxX != 109u || first.rightBoxW != 123u) ok = 0;
    if (last.leftBoxW != 0u || last.rightBoxX != 229u || last.rightBoxW != 3u) ok = 0;
    printf("sourceAnimationStepCount=%u\n", ENTRANCE_Compat_GetSourceAnimationStepCount());
    if (ENTRANCE_Compat_GetSourceAnimationStepCount() != 38u) ok = 0;
    for (i = 1u; i <= ENTRANCE_Compat_GetSourceAnimationStepCount(); ++i) {
        EntranceCompatSourceAnimationStep step;
        if (!ENTRANCE_Compat_GetSourceAnimationStep(i, &step)) { ok = 0; continue; }
        printf("sourceAnimationStep[%u]=kind:%s delay:%u vblankLoop:%u box:%u,%u,%u,%u evidence:%s\n",
               i, kind_name(step.kind), step.delayTicks, step.vblankLoopCount,
               step.x, step.y, step.width, step.height,
               step.sourceLineEvidence ? step.sourceLineEvidence : "");
    }
    printf("entranceSourceAnimationScheduleInvariantOk=%d\n", ok);
    return ok ? 0 : 1;
}
