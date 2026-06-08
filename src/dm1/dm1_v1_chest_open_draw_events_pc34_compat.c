#include "dm1_v1_chest_open_draw_events_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CHEST.C F0333:30-32 same-open early return emits no chest panel redraws\n"
    "CHEST.C F0333:43-48 writes G0426, conditionally draws C145 in C09, then blits C025 open-chest panel\n"
    "CHEST.C F0333:53-76 draws C38..C45 chest slot icons, clearing empty slots with C0xFFFF_ICON_NONE\n"
    "PANEL.C F0347/F0354 route open chest panel content through the current inventory panel draw";

static M11_Item make_item(int itemType)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = 1;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static void add_event(DM1_V1_ChestOpenDrawCasePc34* out,
                      int eventKind,
                      int slotBox,
                      int graphicOrIcon)
{
    DM1_V1_ChestOpenDrawEventPc34* event;

    if (!out || out->eventCount >= DM1_PC34_CHEST_OPEN_DRAW_EVENT_MAX) {
        return;
    }
    event = &out->events[out->eventCount++];
    event->eventKind = eventKind;
    event->slotBox = slotBox;
    event->graphicOrIcon = graphicOrIcon;
}

static void summarize_events(DM1_V1_ChestOpenDrawCasePc34* out)
{
    int i;

    if (!out) {
        return;
    }
    out->firstSlotBox = 0;
    out->lastSlotBox = 0;
    out->firstFilledIcon = 0;
    out->lastClearedIcon = 0;
    for (i = 0; i < out->eventCount; ++i) {
        const DM1_V1_ChestOpenDrawEventPc34* event = &out->events[i];

        if (event->eventKind == DM1_PC34_CHEST_OPEN_DRAW_EVENT_ACTION_ICON) {
            if (event->slotBox == DM1_PC34_CHEST_OPEN_DRAW_SLOT_ACTION_HAND &&
                event->graphicOrIcon ==
                DM1_PC34_CHEST_OPEN_DRAW_ICON_OPEN_CHEST) {
                ++out->actionHandOpenIconCount;
            }
        } else if (event->eventKind ==
                   DM1_PC34_CHEST_OPEN_DRAW_EVENT_PANEL_BLIT) {
            if (event->graphicOrIcon ==
                DM1_PC34_CHEST_OPEN_DRAW_GRAPHIC_OPEN_CHEST_PANEL) {
                ++out->panelBlitCount;
            }
        } else if (event->eventKind ==
                   DM1_PC34_CHEST_OPEN_DRAW_EVENT_SLOT_ICON) {
            ++out->slotIconCount;
            if (out->firstSlotBox == 0) {
                out->firstSlotBox = event->slotBox;
            }
            out->lastSlotBox = event->slotBox;
            if (event->graphicOrIcon ==
                DM1_PC34_CHEST_OPEN_DRAW_ICON_NONE) {
                ++out->clearedSlotIconCount;
                out->lastClearedIcon = event->graphicOrIcon;
            } else {
                ++out->filledSlotIconCount;
                if (out->firstFilledIcon == 0) {
                    out->firstFilledIcon = event->graphicOrIcon;
                }
            }
        }
    }
}

static int run_case(int linkedItemCount,
                    int pressingEye,
                    int sameChestBeforeOpen,
                    DM1_V1_ChestOpenDrawCasePc34* out)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_SLOT_COUNT];
    int i;

    if (!out || linkedItemCount < 0 ||
        linkedItemCount > DM1_PC34_CHEST_SLOT_COUNT) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    m11_inventory_init(&state, 1);
    out->sourceLockedContractOnly = 1;
    out->openChestThing = DM1_PC34_CHEST_OPEN_DRAW_THING;
    out->pressingEye = pressingEye;
    out->sameChestBeforeOpen = sameChestBeforeOpen;
    out->linkedItemCount = linkedItemCount;

    for (i = 0; i < linkedItemCount; ++i) {
        linked[i] = make_item(DM1_PC34_CHEST_OPEN_DRAW_ITEM_FIRST + i);
    }

    if (sameChestBeforeOpen) {
        if (!m11_inventory_open_chest(&state, 0, out->openChestThing,
                                      linked, linkedItemCount)) {
            return 0;
        }
        /* ReDMCSB CHEST.C F0333 lines 30-32 returns before the action-hand
         * icon draw, C025 panel blit, or C38..C45 slot-icon redraws. */
        out->openResult = 1;
        summarize_events(out);
        return 1;
    }

    out->openResult = m11_inventory_open_chest(
        &state, 0, out->openChestThing, linked, linkedItemCount);
    if (!out->openResult) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0333 lines 43-48: when the eye is not being pressed,
     * C09 receives C145 before the C025 open-chest panel is blitted. */
    if (!pressingEye) {
        add_event(out, DM1_PC34_CHEST_OPEN_DRAW_EVENT_ACTION_ICON,
                  DM1_PC34_CHEST_OPEN_DRAW_SLOT_ACTION_HAND,
                  DM1_PC34_CHEST_OPEN_DRAW_ICON_OPEN_CHEST);
    }
    add_event(out, DM1_PC34_CHEST_OPEN_DRAW_EVENT_PANEL_BLIT, 0,
              DM1_PC34_CHEST_OPEN_DRAW_GRAPHIC_OPEN_CHEST_PANEL);

    /* ReDMCSB CHEST.C F0333 lines 53-76 walks the linked list in order, draws
     * occupied C38..C45 icons, then clears the rest with C0xFFFF_ICON_NONE. */
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        const int icon =
            i < linkedItemCount ?
            DM1_PC34_CHEST_OPEN_DRAW_ITEM_FIRST + i :
            DM1_PC34_CHEST_OPEN_DRAW_ICON_NONE;

        add_event(out, DM1_PC34_CHEST_OPEN_DRAW_EVENT_SLOT_ICON,
                  DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_FIRST + i, icon);
    }
    summarize_events(out);
    return 1;
}

const char* dm1_v1_chest_open_draw_events_source_evidence_pc34(void)
{
    return s_source_evidence;
}

int dm1_v1_chest_open_draw_events_run_pc34(
    DM1_V1_ChestOpenDrawProbePc34* out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->sourceLockedContractOnly = 1;
    out->c09ActionHandSlotBox = DM1_PC34_CHEST_OPEN_DRAW_SLOT_ACTION_HAND;
    out->c38ChestFirstSlotBox = DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_FIRST;
    out->c45ChestLastSlotBox = DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_LAST;
    out->c025OpenChestPanelGraphic =
        DM1_PC34_CHEST_OPEN_DRAW_GRAPHIC_OPEN_CHEST_PANEL;
    out->c145OpenChestIcon = DM1_PC34_CHEST_OPEN_DRAW_ICON_OPEN_CHEST;

    if (!run_case(3, 0, 0, &out->normalOpen)) {
        return 0;
    }
    if (!run_case(3, 1, 0, &out->pressingEyeOpen)) {
        return 0;
    }
    if (!run_case(3, 0, 1, &out->sameChestNoop)) {
        return 0;
    }
    return 1;
}
