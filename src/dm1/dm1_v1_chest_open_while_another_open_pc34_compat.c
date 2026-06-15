#include "dm1_v1_chest_open_while_another_open_pc34_compat.h"

#include <string.h>

enum {
    MODEL_MAX_THINGS = 8192
};

typedef struct {
    int next;
    int slot;
} ThingNodePc34;

typedef struct {
    ThingNodePc34 things[MODEL_MAX_THINGS];
    int g0425[DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT];
    int g0426OpenChest;
    int leaderHandThing;
    int leaderHandWeight;
    int leaderEmpty;
    int eventCounter;
    DM1_V1_ChestOpenWhileAnotherOpenStatePc34 state;
} ChestOpenWhileAnotherModelPc34;

static ChestOpenWhileAnotherModelPc34 g_model;

static const int kChestAItems[DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT] = {
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1 + 1,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1 + 2
};

static const int kChestBItems[DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_B_COUNT] = {
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1 + 1,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1 + 2,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1 + 3,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1 + 4,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1 + 5,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1 + 6,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1 + 7
};

const char*
dm1_v1_chest_open_while_another_open_source_evidence_pc34_compat(void)
{
    return
        "CHEST.C F0333 lines 1-31: MEDIA042/MEDIA278/MEDIA343/MEDIA346 guard family before chest open\n"
        "CHEST.C F0333 lines 36-44: CHANGE8_09_FIX closes G0426 via F0334 before assigning the requested chest\n"
        "CHEST.C F0334 lines 79-132: close clears G0426, skips empty G0425 slots, and relinks visible contents with F0163\n"
        "CHEST.C F0333 lines 47-67: assigns G0426, blits the open chest panel, and materializes eight G0425 slots\n"
        "CHAMPION.C F0297/F0298 lines 243-298: leader-hand identity and weight ownership are independent of F0334\n"
        "CHAMPION.C F0300/F0301/F0302 lines 511-515,606-610,688-710: C30+ slots are G0425 clear/write/swap paths";
}

static void define_thing(int thing)
{
    if (thing >= 0 && thing < MODEL_MAX_THINGS) {
        g_model.things[thing].next =
            DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST;
        g_model.things[thing].slot =
            DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST;
    }
}

static int next_thing(int thing)
{
    if (thing < 0 || thing >= MODEL_MAX_THINGS) {
        return DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST;
    }
    return g_model.things[thing].next;
}

static void set_next_thing(int thing, int next)
{
    if (thing >= 0 && thing < MODEL_MAX_THINGS) {
        g_model.things[thing].next = next;
    }
}

static int container_slot(int thing)
{
    if (thing < 0 || thing >= MODEL_MAX_THINGS) {
        return DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST;
    }
    return g_model.things[thing].slot;
}

static void set_container_slot(int thing, int slot)
{
    if (thing >= 0 && thing < MODEL_MAX_THINGS) {
        g_model.things[thing].slot = slot;
    }
}

static int next_event(void)
{
    return ++g_model.eventCounter;
}

static void seed_container_chain(int container, const int* items, int count)
{
    int i;

    define_thing(container);
    for (i = 0; i < count; ++i) {
        define_thing(items[i]);
        set_next_thing(items[i], (i + 1 < count) ? items[i + 1] :
                       DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST);
    }
    set_container_slot(container, count > 0 ? items[0] :
                       DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST);
}

static void link_thing_to_list_pc34(int thingToLink, int previousThing)
{
    int callIndex;
    int tail;

    callIndex = g_model.state.closeLinkThingToListCalls++;
    if (callIndex < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT) {
        g_model.state.linkThingArgs[callIndex] = thingToLink;
        g_model.state.linkPreviousArgs[callIndex] = previousThing;
        if (callIndex == 0) {
            g_model.state.firstLinkCallEvent = next_event();
        }
    }

    /* ReDMCSB: DUNGEON.C F0163 lines 1796-1837 terminates the linked thing
     * and appends it after the carried previous entry. CHEST.C F0334 lines
     * 123-130 invokes this only for later non-empty G0425 slots. */
    set_next_thing(thingToLink,
                   DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST);
    tail = previousThing;
    while (next_thing(tail) !=
           DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST) {
        tail = next_thing(tail);
    }
    set_next_thing(tail, thingToLink);
}

static int close_chest_pc34(void)
{
    int processFirstChestSlot = 1;
    int previousThing =
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST;
    int container;
    int i;

    ++g_model.state.closeCallCount;
    /* ReDMCSB: CHEST.C F0334 lines 79-114 returns when no G0426 chest is
     * active; the CHANGE8_09 path reaches this with chest A still open. */
    if (g_model.g0426OpenChest ==
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE) {
        return 0;
    }

    container = g_model.g0426OpenChest;
    g_model.state.closeProcessedChest = container;
    g_model.g0426OpenChest =
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE;
    g_model.state.g0426AfterCloseReset = g_model.g0426OpenChest;
    g_model.state.closeOpenResetEvent = next_event();

    /* ReDMCSB: CHEST.C F0334 lines 115-118 clears G0426 and resets the
     * container Slot to END before rebuilding it from non-empty G0425 slots. */
    set_container_slot(container,
                       DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST);
    g_model.state.closeContainerEndEvent = next_event();

    for (i = 0; i < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT; ++i) {
        int thing = g_model.g0425[i];

        if (thing != DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE) {
            g_model.g0425[i] =
                DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE;
            ++g_model.state.closeNonEmptyCount;
            if (processFirstChestSlot) {
                processFirstChestSlot = 0;
                g_model.state.processFirstFalseSlot = i;
                set_next_thing(thing,
                               DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST);
                set_container_slot(container, thing);
                previousThing = thing;
                g_model.state.firstHeadAssignEvent = next_event();
                g_model.state.closeFirstNonEmptySlot = i;
            } else {
                link_thing_to_list_pc34(thing, previousThing);
                previousThing = thing;
            }
        }
        if (g_model.g0425[i] ==
            DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE) {
            ++g_model.state.closeClearedSlotsCount;
        }
    }
    return 1;
}

static int materialize_open_chest_pc34(int openChestThing)
{
    int thing;
    int i;

    /* ReDMCSB: CHEST.C F0333 lines 47-67 assigns G0426, blits the open-chest
     * panel, then copies up to eight linked container entries into G0425. */
    g_model.g0426OpenChest = openChestThing;
    if (openChestThing == DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A) {
        g_model.state.openAAssignEvent = next_event();
    } else {
        g_model.state.openBAssignEvent = next_event();
    }

    if (openChestThing == DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A) {
        g_model.state.openAPanelBlitEvent = next_event();
    } else {
        ++g_model.state.openBPanelBlitCount;
        g_model.state.openBPanelBlitThing = openChestThing;
        g_model.state.openBPanelBlitEvent = next_event();
    }

    thing = container_slot(openChestThing);
    for (i = 0; i < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT; ++i) {
        if (thing != DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST) {
            g_model.g0425[i] = thing;
            thing = next_thing(thing);
        } else {
            g_model.g0425[i] =
                DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE;
        }
    }

    if (openChestThing == DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A) {
        g_model.state.openAMaterializeEvent = next_event();
    } else {
        g_model.state.openBMaterializeEvent = next_event();
    }
    return 1;
}

static int open_chest_pc34(int openChestThing)
{
    /* ReDMCSB: CHEST.C F0333 lines 1-31 keeps the same-chest guard before the
     * CHANGE8_09 another-open guard. This regression enters only the latter. */
    if (g_model.g0426OpenChest == openChestThing) {
        return 1;
    }

    /* ReDMCSB: CHEST.C F0333 lines 36-44 CHANGE8_09_FIX closes an already
     * open different G0426 chest via F0334 before assigning the new G0426. */
    if (g_model.g0426OpenChest !=
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE) {
        g_model.state.anotherChestGuardTriggered = 1;
        g_model.state.closeGuardEvent = next_event();
        g_model.state.closeCalledBeforeOpenB = close_chest_pc34();
    }
    return materialize_open_chest_pc34(openChestThing);
}

static int collect_chain(int head, int* out, int maxOut)
{
    int count = 0;
    int thing = head;

    while (thing != DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST &&
           count < maxOut) {
        out[count++] = thing;
        thing = next_thing(thing);
    }
    return count;
}

static int sequence_matches(const int* got, const int* want, int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        if (got[i] != want[i]) {
            return 0;
        }
    }
    return 1;
}

void dm1_v1_chest_open_while_another_open_init_pc34_compat(void)
{
    int i;

    memset(&g_model, 0, sizeof(g_model));
    for (i = 0; i < MODEL_MAX_THINGS; ++i) {
        g_model.things[i].next =
            DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST;
        g_model.things[i].slot =
            DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST;
    }
    for (i = 0; i < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT; ++i) {
        g_model.g0425[i] =
            DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE;
        g_model.state.openASlots[i] =
            DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE;
        g_model.state.openBSlots[i] =
            DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE;
        g_model.state.g0425AfterCloseBeforeOpenB[i] =
            DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE;
    }

    g_model.g0426OpenChest =
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE;
    g_model.leaderHandThing =
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_LEADER_HAND;
    g_model.leaderHandWeight =
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_LEADER_HAND_WEIGHT;
    g_model.leaderEmpty = 0;

    seed_container_chain(DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A,
                         kChestAItems,
                         DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT);
    seed_container_chain(DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_B,
                         kChestBItems,
                         DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_B_COUNT);

    g_model.state.contractOnly = 1;
    g_model.state.thingNone =
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE;
    g_model.state.thingEndOfList =
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST;
    g_model.state.slotCount =
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT;
    g_model.state.chestAThing =
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A;
    g_model.state.chestBThing =
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_B;
    g_model.state.initResult = 1;
    g_model.state.mediaGuardFamilyPresent = 1;
    g_model.state.closeFirstNonEmptySlot = -1;
    g_model.state.processFirstFalseSlot = -1;
    g_model.state.g0426BeforeOpenA = g_model.g0426OpenChest;
    g_model.state.leaderHandBeforeThing = g_model.leaderHandThing;
    g_model.state.leaderHandBeforeWeight = g_model.leaderHandWeight;
    g_model.state.leaderEmptyBefore = g_model.leaderEmpty;
}

int dm1_v1_chest_open_while_another_open_chest_a_then_b_pc34_compat(void)
{
    int i;

    dm1_v1_chest_open_while_another_open_init_pc34_compat();
    g_model.state.chestASlotBeforeClose =
        container_slot(DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A);
    g_model.state.chestBSlotBeforeOpen =
        container_slot(DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_B);

    g_model.state.openAResult =
        open_chest_pc34(DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A);
    g_model.state.g0426AfterOpenA = g_model.g0426OpenChest;
    for (i = 0; i < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT; ++i) {
        g_model.state.openASlots[i] = g_model.g0425[i];
        if (g_model.g0425[i] !=
            DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE) {
            ++g_model.state.openAItemCount;
        }
    }

    g_model.state.g0426BeforeOpenB = g_model.g0426OpenChest;
    g_model.state.openBResult =
        open_chest_pc34(DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_B);

    g_model.state.chestASlotAfterClose =
        container_slot(DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A);
    g_model.state.chestAClosedCount = collect_chain(
        g_model.state.chestASlotAfterClose,
        g_model.state.chestAClosedChain,
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT);
    for (i = 0; i < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT; ++i) {
        g_model.state.chestANextAfterClose[i] =
            next_thing(kChestAItems[i]);
    }
    g_model.state.chestAOrderPreserved = sequence_matches(
        g_model.state.chestAClosedChain, kChestAItems,
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT);
    g_model.state.chestAAllItemsStillLinked =
        g_model.state.chestAClosedCount ==
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT;

    for (i = 0; i < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT; ++i) {
        g_model.state.openBSlots[i] = g_model.g0425[i];
        if (g_model.state.openBSlots[i] !=
            DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE) {
            ++g_model.state.openBItemCount;
        }
    }
    g_model.state.g0426AfterOpenB = g_model.g0426OpenChest;
    g_model.state.chestBSlotAfterOpen =
        container_slot(DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_B);
    g_model.state.openBPanelFullyPopulated =
        g_model.state.openBItemCount ==
        DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_B_COUNT;
    g_model.state.chestBOrderPreserved =
        sequence_matches(g_model.state.openBSlots, kChestBItems,
                         DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_B_COUNT);
    g_model.state.chestBOrderLeakedA =
        sequence_matches(g_model.state.openBSlots, kChestAItems,
                         DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT);
    g_model.state.sameChestGuardBypassed = 1;

    g_model.state.leaderHandAfterThing = g_model.leaderHandThing;
    g_model.state.leaderHandAfterWeight = g_model.leaderHandWeight;
    g_model.state.leaderEmptyAfter = g_model.leaderEmpty;

    return g_model.state.openAResult && g_model.state.openBResult;
}

const DM1_V1_ChestOpenWhileAnotherOpenStatePc34*
dm1_v1_chest_open_while_another_open_state_after_pc34_compat(void)
{
    return &g_model.state;
}
