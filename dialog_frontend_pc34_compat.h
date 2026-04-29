#ifndef REDMCSB_DIALOG_FRONTEND_PC34_COMPAT_H
#define REDMCSB_DIALOG_FRONTEND_PC34_COMPAT_H

typedef enum DialogCompatSourceEventKind {
    DIALOG_COMPAT_SOURCE_EVENT_EXPAND_DIALOG_GRAPHIC = 0,
    DIALOG_COMPAT_SOURCE_EVENT_PRINT_VERSION = 1,
    DIALOG_COMPAT_SOURCE_EVENT_COUNT_CHOICES = 2,
    DIALOG_COMPAT_SOURCE_EVENT_FADE_OR_CLEAR = 3,
    DIALOG_COMPAT_SOURCE_EVENT_PATCH_CHOICE_GRAPHIC = 4,
    DIALOG_COMPAT_SOURCE_EVENT_PRINT_CHOICES = 5,
    DIALOG_COMPAT_SOURCE_EVENT_LAYOUT_MESSAGES = 6,
    DIALOG_COMPAT_SOURCE_EVENT_BLIT_OR_DRAW_VIEWPORT = 7,
    DIALOG_COMPAT_SOURCE_EVENT_FADE_IN = 8,
    DIALOG_COMPAT_SOURCE_EVENT_GET_CHOICE_LOOP = 9,
    DIALOG_COMPAT_SOURCE_EVENT_CHOICE_FEEDBACK = 10,
    DIALOG_COMPAT_SOURCE_EVENT_RESTORE_INPUT = 11
} DialogCompatSourceEventKind;

typedef struct DialogCompatSourceEvent {
    unsigned int ordinal;
    DialogCompatSourceEventKind kind;
    unsigned int choiceCount;
    unsigned int zoneIndex;
    unsigned int patchZoneIndex;
    unsigned int negativeGraphicIndex;
    unsigned int delayTicks;
    unsigned int vblankCount;
    const char* sourceLineEvidence;
} DialogCompatSourceEvent;

typedef struct DialogCompatChoiceLayout {
    unsigned int choiceCount;
    unsigned int slotOrdinal;
    unsigned int textZoneIndex;
    unsigned int centerX;
    unsigned int centerY;
    const char* sourceLineEvidence;
} DialogCompatChoiceLayout;

typedef struct DialogCompatLayoutRecord {
    unsigned int zoneIndex;
    unsigned int layoutKind;
    unsigned int baseIndex;
    unsigned int x;
    unsigned int y;
    const char* sourceLineEvidence;
} DialogCompatLayoutRecord;

unsigned int DIALOG_Compat_GetSourceEventCount(void);
int DIALOG_Compat_GetSourceEvent(unsigned int ordinal, DialogCompatSourceEvent* outEvent);
unsigned int DIALOG_Compat_GetChoiceLayoutCount(unsigned int choiceCount);
int DIALOG_Compat_GetChoiceLayout(unsigned int choiceCount, unsigned int slotOrdinal, DialogCompatChoiceLayout* outLayout);
int DIALOG_Compat_GetLayoutRecord(unsigned int zoneIndex, DialogCompatLayoutRecord* outRecord);
const char* DIALOG_Compat_GetSourceScheduleEvidence(void);

struct GraphicWidthHeight_Compat;
void F0427_DIALOG_DrawBackdrop_Compat(
    const unsigned char* graphic,
    unsigned char* viewportBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo);

#endif
