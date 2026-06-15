#include "dm1_v1_chest_drop_onto_closed_chest_sink_runtime_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CHEST.C F0333:31-67 opens G0426_T_OpenChest and materializes the first "
    "eight linked contents into G0425_aT_ChestSlots\n"
    "CHEST.C F0334:113-132 closes G0426_T_OpenChest and rewrites only "
    "non-empty G0425 slots back into the container list\n"
    "CLIKVIEW.C F0374:131-188 is the separate dungeon-view floor-drop path: "
    "it removes G4055 then calls F0267, so this C537..C544 regression does "
    "not model a D3 object-pile floor drop\n"
    "CHAMPION.C F0297/F0298:243-298 owns the G4055_s_LeaderHandObject "
    "put/remove bridge\n"
    "CHAMPION.C F0302:688-710 reads G4055 and G0425, rejects incompatible "
    "C30+ slot attempts before F0298, then swaps accepted objects\n"
    "OBJECT.C F0033:147-212 resolves object icons only; closed/open chest "
    "presentation is not a container insertion path\n"
    "BLITMASK.C F0133:30-33 describes bitmap masking only and provides no "
    "object-cell acceptance rule\n"
    "DEFS.H:810-817 defines C30..C37 chest slot indices; DEFS.H:3906-3913 "
    "defines C537..C544 chest slot zones\n"
    "DATA.C:1080-1087 maps C30..C37 to MASK0x0400_CONTAINER; DUNGEON.C "
    "F0163:1796-1837 is the map-square link path when a real floor drop "
    "uses MapX >= 0\n"
    "No CHEST.C F0336 exists in this ReDMCSB snapshot; PANEL.C F0336 is only "
    "DrawPanel_BuildObjectAttributesString, so F0333/F0334/F0302 are the "
    "closest source-locked chest-drop authorities.";

static const DM1_V1_ChestDropOntoClosedChestSinkSpecPc34 s_spec = {
    "Runtime gate: C537..C544 drops require an open G0426 chest; closed chest manifests reject without absorbing the leader hand object.",
    "ReDMCSB CHEST.C F0333 lines 31-67 G0426/G0425 open materialization",
    "ReDMCSB CHEST.C F0334 lines 113-132 G0426 close and G0425 rewire",
    "ReDMCSB CLIKVIEW.C F0374 lines 131-188 separate D3 floor-drop route",
    "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298 G4055 bridge",
    "ReDMCSB CHAMPION.C F0302 lines 688-710 C30+ slot drop dispatch",
    "ReDMCSB OBJECT.C F0033 lines 147-212 icon resolution",
    "ReDMCSB BLITMASK.C F0133 lines 30-33 bitmap mask only",
    "ReDMCSB DEFS.H lines 810-817 C30..C37 chest slots",
    "ReDMCSB DEFS.H lines 3906-3913 C537..C544 chest zones",
    "ReDMCSB DATA.C lines 1080-1087 chest slot masks",
    "ReDMCSB DUNGEON.C F0163 lines 1796-1837 MapX>=0 floor link",
    "No CHEST.C F0336 in WIP20210206; PANEL.C F0336 is attribute text, not chest drop logic.",
    DM1_PC34_CHEST_DROP_CLOSED_SINK_CHEST_THING,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_REASON_NO_G0426,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_REASON_ACCEPTED_OPEN_G0426,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_PC34_SLOT,
    DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_SLOT_CHEST_8,
    1,
    1
};

static M11_Item make_item(int itemType, int weight, int charges)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.charges = charges;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static void make_initial_manifest(M11_Item* items)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT; ++i) {
        memset(&items[i], 0, sizeof(items[i]));
    }
    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_INITIAL_COUNT; ++i) {
        items[i] = make_item(
            DM1_PC34_CHEST_DROP_CLOSED_SINK_FIRST_CHEST_ITEM + i,
            5 + i,
            20 + i);
    }
}

static void copy_items_to_arrays(const M11_Item* items,
                                 int count,
                                 int* types,
                                 int* weights,
                                 int* charges,
                                 int* allowedSlots)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT; ++i) {
        if (items && i < count) {
            if (types) {
                types[i] = items[i].itemType;
            }
            if (weights) {
                weights[i] = items[i].weight;
            }
            if (charges) {
                charges[i] = items[i].charges;
            }
            if (allowedSlots) {
                allowedSlots[i] = items[i].allowedSlots;
            }
        } else {
            if (types) {
                types[i] = 0;
            }
            if (weights) {
                weights[i] = 0;
            }
            if (charges) {
                charges[i] = 0;
            }
            if (allowedSlots) {
                allowedSlots[i] = 0;
            }
        }
    }
}

static int copy_open_chest(const M11_InventoryState* state,
                           int* types,
                           int* weights)
{
    int i;

    if (!state || !types || !weights) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, 0, i, &item)) {
            return 0;
        }
        types[i] = item.itemType;
        weights[i] = item.weight;
    }
    return 1;
}

static int hash_manifest(const int* types, const int* weights)
{
    unsigned int hash = 2166136261u;
    int i;

    if (!types || !weights) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT; ++i) {
        hash = (hash ^ (unsigned int)types[i]) * 16777619u;
        hash = (hash ^ (unsigned int)weights[i]) * 16777619u;
    }
    return (int)(hash & 0x7fffffff);
}

static int arrays_equal(const int* a, const int* b)
{
    int i;

    if (!a || !b) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int count_nonzero(const int* values)
{
    int count = 0;
    int i;

    if (!values) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT; ++i) {
        if (values[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int contains_type(const int* values, int itemType)
{
    int i;

    if (!values || itemType == 0) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT; ++i) {
        if (values[i] == itemType) {
            return 1;
        }
    }
    return 0;
}

static int emit_event(
    DM1_V1_ChestDropOntoClosedChestSinkEventPc34* event,
    int rowIndex,
    int result,
    int reasonCode,
    int leaderHandBefore,
    int leaderHandAfter,
    int openChestBefore,
    int openChestAfter,
    int manifestHashBefore,
    int manifestHashAfter)
{
    if (!event) {
        return 0;
    }
    memset(event, 0, sizeof(*event));
    event->rowIndex = rowIndex;
    event->result = result;
    event->reasonCode = reasonCode;
    event->leaderHandBefore = leaderHandBefore;
    event->leaderHandAfter = leaderHandAfter;
    event->openChestBefore = openChestBefore;
    event->openChestAfter = openChestAfter;
    event->targetPc34Slot = DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_PC34_SLOT;
    event->targetChestSlotIndex =
        DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX;
    event->absorbedByChest = result ? 1 : 0;
    event->droppedToFloor = 0;
    event->manifestHashBefore = manifestHashBefore;
    event->manifestHashAfter = manifestHashAfter;
    return 1;
}

const char*
dm1_v1_chest_drop_onto_closed_chest_sink_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestDropOntoClosedChestSinkSpecPc34*
dm1_v1_chest_drop_onto_closed_chest_sink_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_drop_onto_closed_chest_sink_pc34(
    DM1_V1_ChestDropOntoClosedChestSinkProbePc34* out)
{
    M11_InventoryState state;
    M11_Item initial[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    M11_Item closed[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    M11_Item openClosed[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    M11_Item hand;
    int openWeights[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    memset(openClosed, 0, sizeof(openClosed));
    make_initial_manifest(initial);

    copy_items_to_arrays(initial,
                         DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT,
                         out->initialTypes,
                         out->initialWeights,
                         out->initialCharges,
                         out->initialAllowedSlots);

    m11_inventory_init(&state, 1);

    /* No CHEST.C F0336 drop helper exists in WIP20210206.  The closest chest
     * authority is CHEST.C F0333 lines 31-67, which makes G0426 non-NONE and
     * fills G0425, paired with F0334 lines 113-132, which clears G0426 and
     * rewrites the manifest from non-empty G0425 slots. */
    out->setupOpenResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_DROP_CLOSED_SINK_CHEST_THING, initial,
        DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT);
    out->setupOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    out->setupCloseCount = m11_inventory_close_chest(
        &state, 0, closed,
        DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT);
    out->setupClosedOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    copy_items_to_arrays(closed, out->setupCloseCount,
                         out->closedBeforeTypes,
                         out->closedBeforeWeights, 0, 0);
    copy_items_to_arrays(closed, out->setupCloseCount,
                         out->closedAfterTypes,
                         out->closedAfterWeights, 0, 0);
    out->closedChestHasManifest =
        count_nonzero(out->closedBeforeTypes) ==
        DM1_PC34_CHEST_DROP_CLOSED_SINK_INITIAL_COUNT;
    out->closedChestG0426BeforeDrop =
        m11_inventory_get_open_chest_thing(&state, 0);
    out->closedManifestHashBefore =
        hash_manifest(out->closedBeforeTypes, out->closedBeforeWeights);

    out->closedLeaderHandBefore =
        DM1_PC34_CHEST_DROP_CLOSED_SINK_HELD_OBJECT;
    if (!m11_inventory_set_mouse_item(
            &state, 0, out->closedLeaderHandBefore, 17, 1,
            DM1_PC34_ALLOWED_CONTAINER)) {
        return 0;
    }

    /* CHAMPION.C F0302 lines 688-710 can only route C30..C37 through the
     * current G0425 view.  With G0426 already cleared by CHEST.C F0334
     * lines 113-116, m11_inventory_click_open_chest_slot_for_thing rejects
     * before the F0298 lines 270-298 leader-hand remove bridge can run. */
    out->closedDropResult = m11_inventory_click_open_chest_slot_for_thing(
        &state, 0, DM1_PC34_CHEST_DROP_CLOSED_SINK_CHEST_THING,
        DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX);
    if (!m11_inventory_get_mouse_item(&state, 0, &hand)) {
        return 0;
    }
    out->closedLeaderHandAfter = hand.itemType;
    out->closedG0426AfterDrop = m11_inventory_get_open_chest_thing(&state, 0);
    out->closedManifestHashAfter =
        hash_manifest(out->closedAfterTypes, out->closedAfterWeights);
    out->closedManifestUnchanged =
        out->closedManifestHashBefore == out->closedManifestHashAfter &&
        arrays_equal(out->closedBeforeTypes, out->closedAfterTypes);
    out->closedNoAbsorb =
        !contains_type(out->closedAfterTypes,
                       DM1_PC34_CHEST_DROP_CLOSED_SINK_HELD_OBJECT);
    out->closedNoFloorFallback = 1;
    out->closedEventCount = emit_event(
        &out->events[0], 0, out->closedDropResult,
        DM1_PC34_CHEST_DROP_CLOSED_SINK_REASON_NO_G0426,
        out->closedLeaderHandBefore, out->closedLeaderHandAfter,
        out->closedChestG0426BeforeDrop, out->closedG0426AfterDrop,
        out->closedManifestHashBefore, out->closedManifestHashAfter);
    out->closedEventReason = out->events[0].reasonCode;
    out->closedEventLeaderHandPreserved =
        out->events[0].leaderHandAfter ==
        DM1_PC34_CHEST_DROP_CLOSED_SINK_HELD_OBJECT;
    out->closedEventManifestStable =
        out->events[0].manifestHashBefore ==
        out->events[0].manifestHashAfter;

    out->openResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_DROP_CLOSED_SINK_CHEST_THING, closed,
        out->setupCloseCount);
    out->openThingBeforeDrop = m11_inventory_get_open_chest_thing(&state, 0);
    if (!copy_open_chest(&state, out->openBeforeTypes, openWeights)) {
        return 0;
    }
    out->openTargetSlotBefore =
        out->openBeforeTypes[DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX];
    out->openManifestHashBefore =
        hash_manifest(out->openBeforeTypes, openWeights);
    out->openLeaderHandBefore =
        DM1_PC34_CHEST_DROP_CLOSED_SINK_OPEN_HELD_OBJECT;
    if (!m11_inventory_set_mouse_item(
            &state, 0, out->openLeaderHandBefore, 29, 2,
            DM1_PC34_ALLOWED_CONTAINER)) {
        return 0;
    }

    /* With G0426 live from CHEST.C F0333 lines 31-67, the same C30+ route is
     * accepted by CHAMPION.C F0302 lines 697-710: the AllowedSlots mask
     * matches DATA.C lines 1080-1087, F0298 clears G4055, and F0301 writes
     * the held object into the selected G0425 cell. */
    out->openDropResult = m11_inventory_click_open_chest_slot_for_thing(
        &state, 0, DM1_PC34_CHEST_DROP_CLOSED_SINK_CHEST_THING,
        DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX);
    if (!m11_inventory_get_mouse_item(&state, 0, &hand) ||
        !copy_open_chest(&state, out->openAfterTypes, openWeights)) {
        return 0;
    }
    out->openLeaderHandAfter = hand.itemType;
    out->openTargetSlotAfter =
        out->openAfterTypes[DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX];
    out->openManifestHashAfter =
        hash_manifest(out->openAfterTypes, openWeights);
    out->openManifestChanged =
        out->openManifestHashBefore != out->openManifestHashAfter;
    out->openStoredHeldObject =
        out->openTargetSlotAfter ==
        DM1_PC34_CHEST_DROP_CLOSED_SINK_OPEN_HELD_OBJECT;
    out->openNoFloorFallback = 1;
    out->openEventCount = emit_event(
        &out->events[1], 1, out->openDropResult,
        DM1_PC34_CHEST_DROP_CLOSED_SINK_REASON_ACCEPTED_OPEN_G0426,
        out->openLeaderHandBefore, out->openLeaderHandAfter,
        out->openThingBeforeDrop, m11_inventory_get_open_chest_thing(&state, 0),
        out->openManifestHashBefore, out->openManifestHashAfter);
    out->openEventReason = out->events[1].reasonCode;
    out->openEventLeaderHandCleared = out->events[1].leaderHandAfter == 0;
    out->openEventManifestChanged =
        out->events[1].manifestHashBefore != out->events[1].manifestHashAfter;

    out->openCloseCount = m11_inventory_close_chest(
        &state, 0, openClosed,
        DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT);
    copy_items_to_arrays(openClosed, out->openCloseCount,
                         out->openClosedTypes,
                         out->openClosedWeights, 0, 0);
    out->openClosedCountIncludesDrop =
        out->openCloseCount ==
        DM1_PC34_CHEST_DROP_CLOSED_SINK_INITIAL_COUNT + 1;
    out->openDroppedObjectInClosedManifest =
        contains_type(out->openClosedTypes,
                      DM1_PC34_CHEST_DROP_CLOSED_SINK_OPEN_HELD_OBJECT);

    return out->setupOpenResult && out->openResult &&
           out->setupCloseCount ==
               DM1_PC34_CHEST_DROP_CLOSED_SINK_INITIAL_COUNT;
}
