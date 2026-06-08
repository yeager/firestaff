#ifndef FIRESTAFF_DM1_V1_CHEST_OPEN_DRAW_EVENTS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_OPEN_DRAW_EVENTS_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_OPEN_DRAW_EVENT_MAX = 10,
    DM1_PC34_CHEST_OPEN_DRAW_LINKED_ITEM_MAX = 10,
    DM1_PC34_CHEST_OPEN_DRAW_EVENT_ACTION_ICON = 1,
    DM1_PC34_CHEST_OPEN_DRAW_EVENT_PANEL_BLIT = 2,
    DM1_PC34_CHEST_OPEN_DRAW_EVENT_SLOT_ICON = 3,
    DM1_PC34_CHEST_OPEN_DRAW_SLOT_ACTION_HAND = 9,
    DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_FIRST = 38,
    DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_LAST = 45,
    DM1_PC34_CHEST_OPEN_DRAW_GRAPHIC_OPEN_CHEST_PANEL = 25,
    DM1_PC34_CHEST_OPEN_DRAW_ICON_NONE = -1,
    DM1_PC34_CHEST_OPEN_DRAW_ICON_OPEN_CHEST = 145,
    DM1_PC34_CHEST_OPEN_DRAW_THING = 0x7B10,
    DM1_PC34_CHEST_OPEN_DRAW_ITEM_FIRST = 930
};

typedef struct {
    int eventKind;
    int slotBox;
    int graphicOrIcon;
} DM1_V1_ChestOpenDrawEventPc34;

typedef struct {
    int sourceLockedContractOnly;
    int openChestThing;
    int pressingEye;
    int sameChestBeforeOpen;
    int linkedItemCount;
    int openResult;
    int eventCount;
    DM1_V1_ChestOpenDrawEventPc34
        events[DM1_PC34_CHEST_OPEN_DRAW_EVENT_MAX];
    int actionHandOpenIconCount;
    int panelBlitCount;
    int slotIconCount;
    int filledSlotIconCount;
    int clearedSlotIconCount;
    int firstSlotBox;
    int lastSlotBox;
    int firstFilledIcon;
    int lastFilledIcon;
    int lastClearedIcon;
    int materializedSlotCount;
    int overflowInputCount;
    int overflowTailMaterialized;
} DM1_V1_ChestOpenDrawCasePc34;

typedef struct {
    int sourceLockedContractOnly;
    int c09ActionHandSlotBox;
    int c38ChestFirstSlotBox;
    int c45ChestLastSlotBox;
    int c025OpenChestPanelGraphic;
    int c145OpenChestIcon;
    DM1_V1_ChestOpenDrawCasePc34 normalOpen;
    DM1_V1_ChestOpenDrawCasePc34 pressingEyeOpen;
    DM1_V1_ChestOpenDrawCasePc34 sameChestNoop;
    DM1_V1_ChestOpenDrawCasePc34 overflowOpen;
} DM1_V1_ChestOpenDrawProbePc34;

const char* dm1_v1_chest_open_draw_events_source_evidence_pc34(void);
int dm1_v1_chest_open_draw_events_run_pc34(
    DM1_V1_ChestOpenDrawProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_OPEN_DRAW_EVENTS_PC34_COMPAT_H */
