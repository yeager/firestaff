#include "dm1_v1_inventory_chest_close_recompaction_pc34_compat.h"

#include <string.h>

enum {
    MODEL_MAX_THINGS = 2048,
    MODEL_PARENT_CHEST = 40
};

typedef struct {
    int next;
    int slot;
    int weight;
    int isContainer;
} RecompactionThingPc34;

typedef struct {
    RecompactionThingPc34 things[MODEL_MAX_THINGS];
    int chestSlots[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT];
    int openChest;
    int eventCounter;
    int linkThingToListCalls;
    int f0140Calls;
    int f0301Calls;
    int championLoad;
    DM1_V1_InventoryChestCloseRecompactionTracePc34* trace;
} RecompactionModelPc34;

static void model_init(RecompactionModelPc34* model)
{
    int i;

    memset(model, 0, sizeof(*model));
    model->openChest = DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE;
    for (i = 0; i < MODEL_MAX_THINGS; ++i) {
        model->things[i].next =
            DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST;
        model->things[i].slot =
            DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST;
    }
    for (i = 0; i < DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT; ++i) {
        model->chestSlots[i] =
            DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE;
    }
}

static void define_thing(RecompactionModelPc34* model, int thing, int weight,
                         int isContainer)
{
    if (!model || thing < 0 || thing >= MODEL_MAX_THINGS) {
        return;
    }
    model->things[thing].weight = weight;
    model->things[thing].isContainer = isContainer;
    model->things[thing].next =
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST;
    model->things[thing].slot =
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST;
}

static int thing_next(const RecompactionModelPc34* model, int thing)
{
    if (!model || thing < 0 || thing >= MODEL_MAX_THINGS) {
        return DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST;
    }
    return model->things[thing].next;
}

static void set_next(RecompactionModelPc34* model, int thing, int next)
{
    if (!model || thing < 0 || thing >= MODEL_MAX_THINGS) {
        return;
    }
    model->things[thing].next = next;
}

static void set_container_slot(RecompactionModelPc34* model, int thing,
                               int slot)
{
    if (!model || thing < 0 || thing >= MODEL_MAX_THINGS) {
        return;
    }
    model->things[thing].slot = slot;
}

static void link_thing_to_list_pc34(RecompactionModelPc34* model,
                                    int thingToLink, int thingInList)
{
    int callIndex;
    int lastThing;

    if (!model || thingToLink ==
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST) {
        return;
    }

    callIndex = model->linkThingToListCalls++;
    if (model->trace &&
        callIndex < DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT) {
        model->trace->linkThingArgs[callIndex] = thingToLink;
        model->trace->linkPreviousArgs[callIndex] = thingInList;
        model->trace->linkCallEvents[callIndex] = ++model->eventCounter;
        if (callIndex == 0) {
            model->trace->firstLinkCallEvent =
                model->trace->linkCallEvents[callIndex];
        }
    }

    /* ReDMCSB: DUNGEON.C F0163 lines 1796-1837 first terminates the object
     * being linked, then walks from the carried previous thing to append it. */
    set_next(model, thingToLink,
             DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST);
    lastThing = thingInList;
    while (thing_next(model, lastThing) !=
           DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST) {
        lastThing = thing_next(model, lastThing);
    }
    set_next(model, lastThing, thingToLink);
}

static int get_object_weight_pc34(RecompactionModelPc34* model, int thing)
{
    int weight;
    int child;
    int guard;

    if (!model || thing < 0 || thing >= MODEL_MAX_THINGS ||
        thing == DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE ||
        thing == DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST) {
        return 0;
    }
    ++model->f0140Calls;
    if (!model->things[thing].isContainer) {
        return model->things[thing].weight;
    }

    /* ReDMCSB: DUNGEON.C F0140 lines 1114-1120 gives every container a base
     * 50 units and recursively adds the linked objects in its Slot list. */
    weight = DM1_PC34_CHEST_CLOSE_RECOMPACTION_CONTAINER_BASE_WEIGHT;
    child = model->things[thing].slot;
    guard = 0;
    while (child != DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST &&
           guard++ < MODEL_MAX_THINGS) {
        weight += get_object_weight_pc34(model, child);
        child = thing_next(model, child);
    }
    return weight;
}

static void add_object_in_slot_pc34(RecompactionModelPc34* model, int thing)
{
    if (!model ||
        thing == DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE) {
        return;
    }
    /* ReDMCSB: CHAMPION.C F0301 lines 609-614 stores the object and adds
     * F0140_DUNGEON_GetObjectWeight to champion Load. */
    ++model->f0301Calls;
    model->championLoad += get_object_weight_pc34(model, thing);
}

static int collect_chain(const RecompactionModelPc34* model, int head,
                         int* outChain, int maxOut, int* outTerminator,
                         int* outContainsNone)
{
    int count = 0;
    int thing = head;

    if (outContainsNone) {
        *outContainsNone = 0;
    }
    while (thing != DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST &&
           count < maxOut) {
        if (thing == DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE &&
            outContainsNone) {
            *outContainsNone = 1;
            break;
        }
        outChain[count++] = thing;
        thing = thing_next(model, thing);
    }
    if (outTerminator) {
        *outTerminator = thing;
    }
    return count;
}

static int chain_contains(const int* chain, int count, int thing)
{
    int i;

    if (!chain || count < 0) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (chain[i] == thing) {
            return 1;
        }
    }
    return 0;
}

static int close_chest_pc34(RecompactionModelPc34* model)
{
    DM1_V1_InventoryChestCloseRecompactionTracePc34* trace;
    int processFirstChestSlot = 1;
    int previousThing =
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST;
    int containerThing;
    int i;

    if (!model) {
        return 0;
    }
    trace = model->trace;

    /* ReDMCSB: CHEST.C F0334 lines 112-114 initializes the first-slot guard
     * before returning when no G0426_T_OpenChest is active. */
    if (model->openChest ==
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE) {
        if (trace) {
            trace->secondCloseNoOp = 1;
            trace->secondCloseLinkCallDelta = 0;
            trace->openChestAfterSecondClose = model->openChest;
            trace->containerSlotAfterSecondClose =
                model->things[MODEL_PARENT_CHEST].slot;
        }
        return 1;
    }

    containerThing = model->openChest;
    if (trace) {
        trace->containerSlotBeforeClear =
            model->things[containerThing].slot;
    }

    model->openChest =
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE;
    if (trace) {
        trace->openChestAfterReset = model->openChest;
        trace->openChestResetEvent = ++model->eventCounter;
    }

    set_container_slot(model, containerThing,
                       DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST);
    if (trace) {
        trace->containerSlotAfterEndMarker =
            model->things[containerThing].slot;
        trace->endMarkerEvent = ++model->eventCounter;
    }

    for (i = 0; i < DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT; ++i) {
        int thing = model->chestSlots[i];

        if (trace) {
            trace->inputSlots[i] = thing;
            trace->processFirstBefore[i] = processFirstChestSlot;
        }
        if (thing != DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE) {
            model->chestSlots[i] =
                DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE;
            if (processFirstChestSlot) {
                processFirstChestSlot = 0;
                if (trace) {
                    trace->firstThing = thing;
                    trace->firstThingNextBefore = thing_next(model, thing);
                    trace->processFirstFalseSlot = i;
                    trace->processFirstFalseEvent = ++model->eventCounter;
                }
                set_next(model, thing,
                         DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST);
                if (trace) {
                    trace->firstThingNextAfterSentinel =
                        thing_next(model, thing);
                    trace->firstNextSentinelEvent = ++model->eventCounter;
                }
                set_container_slot(model, containerThing, thing);
                previousThing = thing;
                if (trace) {
                    trace->containerSlotAfterFirstHead =
                        model->things[containerThing].slot;
                    trace->headAssignEvent = ++model->eventCounter;
                }
            } else {
                int callIndex = model->linkThingToListCalls;

                if (trace &&
                    callIndex < DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT) {
                    trace->previousBeforeLink[callIndex] = previousThing;
                }
                link_thing_to_list_pc34(model, thing, previousThing);
                previousThing = thing;
                if (trace &&
                    callIndex < DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT) {
                    trace->previousAfterLink[callIndex] = previousThing;
                }
            }
        }
        if (trace) {
            trace->processFirstAfter[i] = processFirstChestSlot;
        }
    }

    if (trace) {
        trace->chainHead = model->things[containerThing].slot;
        trace->chainCount = collect_chain(
            model, trace->chainHead, trace->chain,
            DM1_PC34_CHEST_CLOSE_RECOMPACTION_CHAIN_CAPACITY,
            &trace->chainTerminator, &trace->chainContainsNone);
        trace->linkCallCount = model->linkThingToListCalls;
        if (trace->firstThing >= 0 && trace->firstThing < MODEL_MAX_THINGS) {
            trace->firstThingNextAfterClose =
                thing_next(model, trace->firstThing);
        }
        if (trace->nestedThing > 0) {
            trace->nestedThingNextAfterClose =
                thing_next(model, trace->nestedThing);
            trace->nestedThingInChain =
                chain_contains(trace->chain, trace->chainCount,
                               trace->nestedThing);
        }
    }
    return 1;
}

static void setup_open_chest(RecompactionModelPc34* model, const int* slots,
                             int slotCount)
{
    int i;

    define_thing(model, MODEL_PARENT_CHEST, 0, 1);
    set_container_slot(model, MODEL_PARENT_CHEST, 777);
    model->openChest = MODEL_PARENT_CHEST;
    for (i = 0; i < slotCount &&
         i < DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT; ++i) {
        model->chestSlots[i] = slots[i];
        if (slots[i] != DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE) {
            define_thing(model, slots[i], 1 + i, 0);
        }
    }
}

static int run_trace(const int* slots, int nestedHost, int nestedThing,
                     DM1_V1_InventoryChestCloseRecompactionTracePc34* out)
{
    RecompactionModelPc34 model;
    int linkCallsAfterFirstClose;

    if (!slots || !out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->nestedThing = nestedThing;
    model_init(&model);
    model.trace = out;
    setup_open_chest(&model, slots,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT);
    if (nestedHost != DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE &&
        nestedThing != DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE) {
        define_thing(&model, nestedThing, 19, 0);
        set_next(&model, nestedHost, nestedThing);
        set_next(&model, nestedThing,
                 DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST);
    }

    if (!close_chest_pc34(&model)) {
        return 0;
    }
    linkCallsAfterFirstClose = model.linkThingToListCalls;
    if (!close_chest_pc34(&model)) {
        return 0;
    }
    out->secondCloseLinkCallDelta =
        model.linkThingToListCalls - linkCallsAfterFirstClose;
    return 1;
}

static int run_base_weight_probe(
    DM1_V1_InventoryChestCloseRecompactionProbePc34* out)
{
    RecompactionModelPc34 model;
    DM1_V1_InventoryChestCloseRecompactionTracePc34 trace;
    int slots[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT] = {
        900, 901,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE
    };

    if (!out) {
        return 0;
    }
    memset(&trace, 0, sizeof(trace));
    model_init(&model);
    model.trace = &trace;
    setup_open_chest(&model, slots,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT);

    define_thing(&model, 900, 0, 1);
    define_thing(&model, 901, 0, 1);
    define_thing(&model, 910, 7, 0);
    define_thing(&model, 911, 11, 0);
    set_container_slot(&model, 900, 910);
    set_container_slot(&model, 901, 911);
    set_next(&model, 910,
             DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST);
    set_next(&model, 911,
             DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST);

    if (!close_chest_pc34(&model)) {
        return 0;
    }
    out->baseWeightCloseCount = trace.chainCount;
    out->baseWeightFirstChildWeight = get_object_weight_pc34(&model, 900);
    out->baseWeightSecondChildWeight = get_object_weight_pc34(&model, 901);
    out->baseWeightContentsOnly = 7 + 11;
    out->baseWeightParentContainerWeight =
        get_object_weight_pc34(&model, MODEL_PARENT_CHEST);
    out->baseWeightExpectedChampionLoad =
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_CONTAINER_BASE_WEIGHT +
        out->baseWeightFirstChildWeight +
        out->baseWeightSecondChildWeight;
    add_object_in_slot_pc34(&model, MODEL_PARENT_CHEST);
    out->baseWeightChampionLoadAfterF0301 = model.championLoad;
    out->baseWeightF0301Calls = model.f0301Calls;
    out->baseWeightF0140Calls = model.f0140Calls;
    return 1;
}

const char* dm1_inventory_chest_close_recompaction_source_evidence_pc34(void)
{
    return
        "CHEST.C:112-132 F0334 resets G0426, writes Container->Slot END, "
        "sets the first non-empty Generic->Next END, then appends via F0163\n"
        "DUNGEON.C:1796-1837 F0163 terminates the appended thing and links it "
        "after the carried previous thing\n"
        "DUNGEON.C:1114-1120 F0140 gives containers base weight 50 plus linked "
        "contents\n"
        "CHAMPION.C:609-614 F0301 adds F0140 object weight to champion Load";
}

int m11_inventory_pc34_probe_chest_close_recompaction(
    DM1_V1_InventoryChestCloseRecompactionProbePc34* out)
{
    const int fullSlots[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT] = {
        305, 101, 404, 202, 808, 707, 606, 505
    };
    const int sparseSlots[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT] = {
        110,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        220,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        330,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        440,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE
    };
    const int guardSlots[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT] = {
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        510,
        520,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        530,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE
    };
    const int overfullSlots[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT] = {
        610, 620, 630, 640, 650, 660, 670, 680
    };
    const int emptySlots[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT] = {
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
        DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE
    };

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    if (!run_trace(fullSlots,
                   DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
                   DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
                   &out->fullOrder)) {
        return 0;
    }
    if (!run_trace(sparseSlots,
                   DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
                   DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
                   &out->sparseOrder)) {
        return 0;
    }
    if (!run_trace(guardSlots,
                   DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
                   DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
                   &out->guardOrder)) {
        return 0;
    }
    if (!run_trace(overfullSlots, 610, 690, &out->overfullOrder)) {
        return 0;
    }
    if (!run_trace(emptySlots,
                   DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
                   DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
                   &out->emptyOrder)) {
        return 0;
    }
    return run_base_weight_probe(out);
}
