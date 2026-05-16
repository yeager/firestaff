#include <stdio.h>
#include <string.h>
#include "dialog_frontend_pc34_compat.h"

static const char* kind_name(DialogCompatSourceEventKind kind) {
    switch (kind) {
    case DIALOG_COMPAT_SOURCE_EVENT_EXPAND_DIALOG_GRAPHIC: return "EXPAND_DIALOG_GRAPHIC";
    case DIALOG_COMPAT_SOURCE_EVENT_PRINT_VERSION: return "PRINT_VERSION";
    case DIALOG_COMPAT_SOURCE_EVENT_COUNT_CHOICES: return "COUNT_CHOICES";
    case DIALOG_COMPAT_SOURCE_EVENT_FADE_OR_CLEAR: return "FADE_OR_CLEAR";
    case DIALOG_COMPAT_SOURCE_EVENT_PATCH_CHOICE_GRAPHIC: return "PATCH_CHOICE_GRAPHIC";
    case DIALOG_COMPAT_SOURCE_EVENT_PRINT_CHOICES: return "PRINT_CHOICES";
    case DIALOG_COMPAT_SOURCE_EVENT_LAYOUT_MESSAGES: return "LAYOUT_MESSAGES";
    case DIALOG_COMPAT_SOURCE_EVENT_BLIT_OR_DRAW_VIEWPORT: return "BLIT_OR_DRAW_VIEWPORT";
    case DIALOG_COMPAT_SOURCE_EVENT_FADE_IN: return "FADE_IN";
    case DIALOG_COMPAT_SOURCE_EVENT_GET_CHOICE_LOOP: return "GET_CHOICE_LOOP";
    case DIALOG_COMPAT_SOURCE_EVENT_CHOICE_FEEDBACK: return "CHOICE_FEEDBACK";
    case DIALOG_COMPAT_SOURCE_EVENT_RESTORE_INPUT: return "RESTORE_INPUT";
    }
    return "UNKNOWN";
}

int main(void) {
    unsigned int i, c, s;
    int ok = 1;
    unsigned int patchEvents = 0u;
    DialogCompatLayoutRecord rec;
    printf("probe=firestaff_dialog_source_schedule\n");
    printf("sourceScheduleEvidence=%s\n", DIALOG_Compat_GetSourceScheduleEvidence());
    printf("dialogSourceEventCount=%u\n", DIALOG_Compat_GetSourceEventCount());
    if (DIALOG_Compat_GetSourceEventCount() != 14u) ok = 0;
    for (i = 1u; i <= DIALOG_Compat_GetSourceEventCount(); ++i) {
        DialogCompatSourceEvent event;
        if (!DIALOG_Compat_GetSourceEvent(i, &event)) { ok = 0; continue; }
        if (event.kind == DIALOG_COMPAT_SOURCE_EVENT_PATCH_CHOICE_GRAPHIC) patchEvents++;
        printf("dialogSourceEvent[%u]=kind:%s choiceCount:%u zone:%u patchZone:%u negGraphic:%u delay:%u vblank:%u evidence:%s\n",
               i, kind_name(event.kind), event.choiceCount, event.zoneIndex, event.patchZoneIndex,
               event.negativeGraphicIndex, event.delayTicks, event.vblankCount,
               event.sourceLineEvidence ? event.sourceLineEvidence : "");
    }
    if (patchEvents != 4u) ok = 0;
    for (c = 1u; c <= 4u; ++c) {
        printf("choiceLayoutCount[%u]=%u\n", c, DIALOG_Compat_GetChoiceLayoutCount(c));
        if (DIALOG_Compat_GetChoiceLayoutCount(c) != c) ok = 0;
        for (s = 1u; s <= c; ++s) {
            DialogCompatChoiceLayout layout;
            if (!DIALOG_Compat_GetChoiceLayout(c, s, &layout)) { ok = 0; continue; }
            printf("choiceLayout[%u,%u]=zone:%u center:%u,%u evidence:%s\n",
                   c, s, layout.textZoneIndex, layout.centerX, layout.centerY,
                   layout.sourceLineEvidence ? layout.sourceLineEvidence : "");
        }
    }
    if (!DIALOG_Compat_GetLayoutRecord(450u, &rec) || rec.x != 192u || rec.y != 7u) ok = 0;
    if (!DIALOG_Compat_GetLayoutRecord(469u, &rec) || rec.baseIndex != 468u || rec.x != 112u || rec.y != 49u) ok = 0;
    if (!DIALOG_Compat_GetLayoutRecord(471u, &rec) || rec.baseIndex != 470u || rec.x != 112u || rec.y != 32u) ok = 0;
    for (i = 450u; i <= 471u; ++i) {
        if (DIALOG_Compat_GetLayoutRecord(i, &rec)) {
            printf("dialogLayoutRecord[%u]=kind:%u base:%u xy:%u,%u evidence:%s\n",
                   i, rec.layoutKind, rec.baseIndex, rec.x, rec.y,
                   rec.sourceLineEvidence ? rec.sourceLineEvidence : "");
        }
    }
    printf("dialogSourceScheduleInvariantOk=%d\n", ok);
    return ok ? 0 : 1;
}
