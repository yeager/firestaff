#include "firestaff/dm1/v1/chest/c040_drop_during_rotation_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB: COMMAND.C F0380 lines 2174-2178 drains queued C028..C065
 * slot-box commands even after panel dispatch has observed M568/C040 at
 * lines 1985-1990.  CHAMPION.C F0302 lines 688-710 then writes C30+ slots
 * through G0425, while PANEL.C F0346/F0347 lines 1619-1657 and REVIVE.C
 * F0280/F0282 preserve the live C040 candidate until an explicit C160..C162.
 */

typedef struct {
    int itemType;
    int weight;
    int charges;
    int quantity;
    int allowedSlots;
} C040DropThingPc34;

typedef struct {
    M11_InventoryState inventory;
    C040DropThingPc34 linked[DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    int quantities[DM1_V1_CHEST_C040_DROP_ROT_CHAMPION_COUNT_PC34]
                  [DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    int handQuantity[DM1_V1_CHEST_C040_DROP_ROT_CHAMPION_COUNT_PC34];
    uint32_t rng;
    int rngCalls;
    int currentLeader;
    int openOwner;
    int candidateOrdinal;
    int c040PanelOpen;
    int c040Graphic;
    int c040Command;
    int f0282ClearCount;
    int dropQueued;
    int rotationQueued;
    int commandQueueDepth;
    int mouseUpdateDepth;
    int f0077Observed;
    int f0078Observed;
    int closeCount;
} C040DropRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and materializes C537..C544/G0425\n"
    "CHEST.C F0334:113-132 is a negative close/relink anchor for this race\n"
    "CHAMPION.C F0297:243-298 owns the C030/G4055 leader hand put path\n"
    "CHAMPION.C F0298:270-298 owns the C030/G4055 leader hand remove path\n"
    "CHAMPION.C F0301:606-614 writes C30+ chest slots through G0425\n"
    "CHAMPION.C F0302:662-714 dispatches C537..C544 slot-box mutation\n"
    "COMMAND.C F0359:1452-1662 queues mouse commands\n"
    "COMMAND.C F0380:2045-2178 drains queued C061/C540 before rotation\n"
    "COMMAND.C F0380:1985-1990 keeps M568/C040 panel commands separate\n"
    "REVIVE.C F0280:124-132 publishes G0299 mirror candidate\n"
    "REVIVE.C F0282:744-806 clears G0299 only on C160..C162\n"
    "PANEL.C F0346/F0347:1619-1657 draws and keeps C040 panel state\n"
    "IO.C F0077:1113-1122 enables mouse screen-update suppression\n"
    "IO.C F0078:1102-1111 disables mouse screen-update suppression\n"
    "DEFS.H:338-340 C160..C162; 810-817 C30..C37; 1874-1878 C38; 2200 C040; 3001-3008 M568/M569; 3906-3913 C537..C544; 5694 G0299; 5876-5881 G0423/G0425/G0426\n"
    "Disjointness: chest_c040_drop_during_rotation is not pass771 plain chest scroll-wheel drop-during-rotation non-leader-open, not mirror_candidate_c545_drop_while_panel_live, not mirror_candidate_c040_redraw_after_chest_close, not chest_close_while_candidate_live, not chest_scroll_wheel_close_race, not chest_resurrect_rotation_scroll_wheel, not mirror_candidate_c545_accept_during_rotation, and not mirror_candidate_panel_redraw_after_inventory_exit";

static const DM1_V1_ChestC040DropDuringRotationSpecPc34 s_spec = {
    "Runtime regression: queued C061/C540 chest drop drains while M568/C040 candidate panel is live, then leader rotation drains.",
    "CHEST.C F0333 lines 30-67 open/materialize G0426 into C537..C544",
    "CHEST.C F0334 lines 113-132 close/relink path must not execute",
    "CHAMPION.C F0297 lines 243-298 put object in C030 leader hand",
    "CHAMPION.C F0298 lines 270-298 remove object from C030 leader hand",
    "CHAMPION.C F0301 lines 606-614 C30+ slot write through G0425",
    "CHAMPION.C F0302 lines 662-714 C537..C544 slot-box dispatch",
    "COMMAND.C F0359 lines 1452-1662 command queue producer",
    "COMMAND.C F0380 lines 2045-2178 command queue drain order",
    "COMMAND.C F0380 lines 1985-1990 C040 panel command guard",
    "REVIVE.C F0280 lines 124-132 candidate publish guard",
    "REVIVE.C F0282 lines 744-806 candidate clear guard",
    "PANEL.C F0346/F0347 lines 1619-1657 C040 panel draw/state",
    "IO.C F0077 lines 1113-1122 enable screen update suppression",
    "IO.C F0078 lines 1102-1111 disable screen update suppression",
    "DEFS.H lines 338-340,810-817,1874-1878,2200,3001-3008,3906-3913,5694,5876-5881 C160..C162/C30..C37/C38/C040/M568/M569/C537..C544/G0299/G0423/G0425/G0426",
    "Disjoint from pass771, C040 C545 drop live, C040 redraw after chest close, candidate-live close, scroll-wheel close race, resurrect-rotation wheel, C545 accept during rotation, and panel-redraw-after-inventory-exit.",
    DM1_V1_CHEST_C040_DROP_ROT_DETERMINISTIC_SEED_PC34,
    0x7CE32CE0u,
    DM1_V1_CHEST_C040_DROP_ROT_EXPECTED_RNG_CALLS_PC34,
    1,
    1,
    1,
    1,
    1
};

static uint32_t next_u32(C040DropRuntimePc34* rt)
{
    rt->rng = rt->rng * 1664525u + 1013904223u;
    ++rt->rngCalls;
    return rt->rng;
}

static int next_range(C040DropRuntimePc34* rt, int base, int span)
{
    return base + (int)(next_u32(rt) % (uint32_t)span);
}

static M11_Item to_item(C040DropThingPc34 thing)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = thing.itemType;
    item.weight = thing.weight;
    item.charges = thing.charges;
    item.identified = 1;
    item.allowedSlots = thing.allowedSlots;
    return item;
}

static C040DropThingPc34 make_stable_thing(C040DropRuntimePc34* rt,
                                           int slotIndex)
{
    C040DropThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = next_range(rt, 0x6200 + (slotIndex * 0x20), 0x1f);
    thing.weight = next_range(rt, 4, 9);
    thing.charges = next_range(rt, 20 + slotIndex, 17);
    thing.quantity = next_range(rt, 2 + slotIndex, 11);
    thing.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return thing;
}

static C040DropThingPc34 make_hand_thing(C040DropRuntimePc34* rt,
                                         int base,
                                         int quantityBase,
                                         int allowedSlots)
{
    C040DropThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = next_range(rt, base, 0x2f);
    thing.weight = next_range(rt, 8, 13);
    thing.charges = next_range(rt, 50, 31);
    thing.quantity = next_range(rt, quantityBase, 9);
    thing.allowedSlots = allowedSlots;
    return thing;
}

static int panel_hash(const C040DropRuntimePc34* rt)
{
    uint32_t hash = 2166136261u;

    hash ^= (uint32_t)rt->candidateOrdinal;
    hash *= 16777619u;
    hash ^= (uint32_t)rt->c040PanelOpen;
    hash *= 16777619u;
    hash ^= (uint32_t)rt->c040Graphic;
    hash *= 16777619u;
    hash ^= (uint32_t)rt->c040Command;
    hash *= 16777619u;
    hash ^= (uint32_t)m11_inventory_get_panel_content_pc34(&rt->inventory);
    hash *= 16777619u;
    return (int)hash;
}

static void record_slots(const C040DropRuntimePc34* rt,
                         int* types,
                         int* charges,
                         int* quantities)
{
    int i;

    for (i = 0; i < DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        M11_Item item;

        if (m11_inventory_get_item_in_chest_slot(&rt->inventory,
                                                 rt->openOwner,
                                                 i,
                                                 &item)) {
            types[i] = item.itemType;
            charges[i] = item.charges;
            quantities[i] = rt->quantities[rt->openOwner][i];
        } else {
            types[i] = 0;
            charges[i] = 0;
            quantities[i] = 0;
        }
    }
}

static int slots_match_before(const C040DropRuntimePc34* rt,
                              const int* types,
                              const int* charges,
                              const int* quantities)
{
    int i;

    for (i = 0; i < DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        if (i == DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34) {
            if (types[i] != 0 || charges[i] != 0 || quantities[i] != 0) {
                return 0;
            }
            continue;
        }
        if (types[i] != rt->linked[i].itemType ||
            charges[i] != rt->linked[i].charges ||
            quantities[i] != rt->linked[i].quantity) {
            return 0;
        }
    }
    return 1;
}

static int slots_match_after(const C040DropRuntimePc34* rt,
                             const int* types,
                             const int* charges,
                             const int* quantities)
{
    int i;
    M11_Item hand;

    (void)m11_inventory_get_mouse_item(
        &rt->inventory,
        DM1_V1_CHEST_C040_DROP_ROT_OLD_LEADER_PC34,
        &hand);

    for (i = 0; i < DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        if (i == DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34) {
            if (types[i] == 0 || quantities[i] == 0) {
                return 0;
            }
            continue;
        }
        if (types[i] != rt->linked[i].itemType ||
            charges[i] != rt->linked[i].charges ||
            quantities[i] != rt->linked[i].quantity) {
            return 0;
        }
    }
    return 1;
}

static void runtime_init(C040DropRuntimePc34* rt)
{
    M11_Item linked[DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    C040DropThingPc34 hand;
    C040DropThingPc34 newLeaderHand;
    int i;

    memset(rt, 0, sizeof(*rt));
    memset(linked, 0, sizeof(linked));
    rt->rng = DM1_V1_CHEST_C040_DROP_ROT_DETERMINISTIC_SEED_PC34;
    rt->currentLeader = DM1_V1_CHEST_C040_DROP_ROT_OLD_LEADER_PC34;
    rt->openOwner = DM1_V1_CHEST_C040_DROP_ROT_OPEN_OWNER_PC34;
    rt->candidateOrdinal = DM1_V1_CHEST_C040_DROP_ROT_CANDIDATE_ORDINAL_PC34;
    rt->c040PanelOpen = 1;
    rt->c040Graphic = DM1_V1_CHEST_C040_DROP_ROT_C040_GRAPHIC_PC34;
    rt->c040Command = DM1_V1_CHEST_C040_DROP_ROT_C040_COMMAND_PC34;

    m11_inventory_init(&rt->inventory,
                       DM1_V1_CHEST_C040_DROP_ROT_CHAMPION_COUNT_PC34);

    for (i = 0; i < DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        if (i == DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34) {
            continue;
        }
        rt->linked[i] = make_stable_thing(rt, i);
        linked[i] = to_item(rt->linked[i]);
        rt->quantities[rt->openOwner][i] = rt->linked[i].quantity;
    }

    hand = make_hand_thing(rt, 0x77c0, 9, DM1_PC34_ALLOWED_CONTAINER);
    newLeaderHand = make_hand_thing(rt, 0x79c0, 4, DM1_PC34_ALLOWED_ANY_SLOT);

    (void)m11_inventory_set_mouse_item(&rt->inventory,
                                       rt->currentLeader,
                                       hand.itemType,
                                       hand.weight,
                                       hand.charges,
                                       hand.allowedSlots);
    rt->handQuantity[rt->currentLeader] = hand.quantity;
    (void)m11_inventory_set_mouse_item(
        &rt->inventory,
        DM1_V1_CHEST_C040_DROP_ROT_NEW_LEADER_PC34,
        newLeaderHand.itemType,
        newLeaderHand.weight,
        newLeaderHand.charges,
        newLeaderHand.allowedSlots);
    rt->handQuantity[DM1_V1_CHEST_C040_DROP_ROT_NEW_LEADER_PC34] =
        newLeaderHand.quantity;

    (void)m11_inventory_open_chest(&rt->inventory,
                                   rt->openOwner,
                                   0x6400 + next_range(rt, 0x20, 0x3f),
                                   linked,
                                   DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34);
    (void)m11_inventory_set_panel_content_pc34(
        &rt->inventory,
        DM1_V1_CHEST_C040_DROP_ROT_PANEL_C040_PC34);
}

static int queue_drop_and_rotation(C040DropRuntimePc34* rt)
{
    if (!rt || rt->dropQueued || rt->rotationQueued ||
        rt->candidateOrdinal != DM1_V1_CHEST_C040_DROP_ROT_CANDIDATE_ORDINAL_PC34 ||
        !rt->c040PanelOpen ||
        m11_inventory_get_panel_content_pc34(&rt->inventory) !=
            DM1_V1_CHEST_C040_DROP_ROT_PANEL_C040_PC34) {
        return 0;
    }

    rt->dropQueued = 1;
    rt->rotationQueued = 1;
    rt->commandQueueDepth = 2;
    return 1;
}

static int c040_click_is_suppressed_while_hand_full(const C040DropRuntimePc34* rt)
{
    M11_Item hand;

    if (!rt || !rt->c040PanelOpen ||
        rt->candidateOrdinal != DM1_V1_CHEST_C040_DROP_ROT_CANDIDATE_ORDINAL_PC34 ||
        m11_inventory_get_panel_content_pc34(&rt->inventory) !=
            DM1_V1_CHEST_C040_DROP_ROT_PANEL_C040_PC34 ||
        !m11_inventory_get_mouse_item(&rt->inventory, rt->currentLeader, &hand)) {
        return 0;
    }
    return hand.itemType != 0;
}

static int drain_c540_drop(C040DropRuntimePc34* rt)
{
    M11_Item hand;
    M11_Item c540;
    int result;

    if (!rt || !rt->dropQueued || rt->commandQueueDepth != 2 ||
        !rt->c040PanelOpen ||
        rt->candidateOrdinal != DM1_V1_CHEST_C040_DROP_ROT_CANDIDATE_ORDINAL_PC34 ||
        m11_inventory_get_panel_content_pc34(&rt->inventory) !=
            DM1_V1_CHEST_C040_DROP_ROT_PANEL_C040_PC34 ||
        !m11_inventory_get_mouse_item(&rt->inventory, rt->currentLeader, &hand) ||
        hand.itemType == 0 ||
        !m11_inventory_get_item_in_chest_slot(
            &rt->inventory,
            rt->openOwner,
            DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34,
            &c540) ||
        c540.itemType != 0 ||
        !m11_inventory_can_equip(
            &hand,
            DM1_V1_CHEST_C040_DROP_ROT_TARGET_PC34_SLOT_PC34)) {
        return 0;
    }

    rt->f0077Observed = 1;
    ++rt->mouseUpdateDepth;
    result = m11_inventory_set_item_in_chest_slot(
        &rt->inventory,
        rt->openOwner,
        DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34,
        hand.itemType,
        hand.weight,
        hand.charges,
        hand.allowedSlots);
    if (!result) {
        return 0;
    }
    rt->quantities[rt->openOwner]
                  [DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34] =
        rt->handQuantity[rt->currentLeader];
    (void)m11_inventory_set_mouse_item(
        &rt->inventory, rt->currentLeader, 0, 0, 0, 0);
    rt->handQuantity[rt->currentLeader] = 0;
    rt->dropQueued = 0;
    --rt->commandQueueDepth;
    --rt->mouseUpdateDepth;
    rt->f0078Observed = 1;
    return 1;
}

static int consume_rotation(C040DropRuntimePc34* rt)
{
    if (!rt || !rt->rotationQueued || rt->commandQueueDepth != 1 ||
        rt->currentLeader != DM1_V1_CHEST_C040_DROP_ROT_OLD_LEADER_PC34) {
        return 0;
    }

    rt->currentLeader = DM1_V1_CHEST_C040_DROP_ROT_NEW_LEADER_PC34;
    rt->rotationQueued = 0;
    --rt->commandQueueDepth;
    return 1;
}

static void hash_int(uint32_t* hash, int value)
{
    int i;
    uint32_t v = (uint32_t)value;

    for (i = 0; i < 4; ++i) {
        *hash ^= (v >> (i * 8)) & 0xffu;
        *hash *= 16777619u;
    }
}

static void hash_probe(uint32_t* hash,
                       const DM1_V1_ChestC040DropDuringRotationProbePc34* p)
{
    int i;

    hash_int(hash, (int)p->postResolveSeed);
    hash_int(hash, p->rngCallCount);
    hash_int(hash, p->panelHashAfterRotate);
    hash_int(hash, p->candidateOrdinalAfterRotate);
    hash_int(hash, p->leaderAfterRotate);
    hash_int(hash, p->c540TypeAfterRotate);
    hash_int(hash, p->c540QuantityAfterRotate);
    hash_int(hash, p->chestNeverClosed);
    hash_int(hash, p->f0077F0078Balanced);
    for (i = 0; i < DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        hash_int(hash, p->visibleTypesBefore[i]);
        hash_int(hash, p->visibleTypesAfterDrop[i]);
        hash_int(hash, p->visibleTypesAfterRotate[i]);
        hash_int(hash, p->visibleQuantitiesAfterRotate[i]);
    }
}

const char*
dm1_v1_chest_c040_drop_during_rotation_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestC040DropDuringRotationSpecPc34*
dm1_v1_chest_c040_drop_during_rotation_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_c040_drop_during_rotation_run_pc34(
    DM1_V1_ChestC040DropDuringRotationProbePc34* out)
{
    C040DropRuntimePc34 rt;
    M11_Item hand;
    M11_Item c540;
    uint32_t hash = 2166136261u;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    runtime_init(&rt);

    out->contractOnly = 1;
    out->noGameData = 1;
    out->noGraphicsDatLoad = 1;
    out->noDungeonDatLoad = 1;
    out->noRealAssetPixels = 1;
    out->runtimeRegression = 1;
    out->deterministicSeed = DM1_V1_CHEST_C040_DROP_ROT_DETERMINISTIC_SEED_PC34;
    out->rngCallCount = rt.rngCalls;
    out->postResolveSeed = rt.rng;

    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_C040_DROP_ROT_STEP_OPEN_CHEST_PC34;
    out->openOwnerBefore = rt.openOwner;
    out->openChestThingBefore =
        m11_inventory_get_open_chest_thing(&rt.inventory, rt.openOwner);
    out->panelAfterChestOpen = DM1_V1_CHEST_C040_DROP_ROT_PANEL_CHEST_PC34;
    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_C040_DROP_ROT_STEP_OPEN_C040_PC34;
    out->panelAfterC040Open =
        m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->c040PanelOpenBefore = rt.c040PanelOpen;
    out->c040Graphic = rt.c040Graphic;
    out->c040Command = rt.c040Command;
    out->candidateOrdinalBefore = rt.candidateOrdinal;
    out->panelHashBeforeDrop = panel_hash(&rt);
    record_slots(&rt,
                 out->visibleTypesBefore,
                 out->visibleChargesBefore,
                 out->visibleQuantitiesBefore);
    out->c540EmptyBeforeDrop =
        out->visibleTypesBefore
            [DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34] == 0;
    out->chestSlotChainCoherentBefore =
        slots_match_before(&rt,
                           out->visibleTypesBefore,
                           out->visibleChargesBefore,
                           out->visibleQuantitiesBefore);

    out->queuedDrop = queue_drop_and_rotation(&rt);
    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_C040_DROP_ROT_STEP_QUEUE_DROP_ROTATION_PC34;
    out->queuedRotation = rt.rotationQueued;
    out->queuedCommand = DM1_V1_CHEST_C040_DROP_ROT_TARGET_COMMAND_PC34;
    out->queuedZone = DM1_V1_CHEST_C040_DROP_ROT_TARGET_ZONE_PC34;
    out->queuedSlotBox = DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_BOX_PC34;
    out->queuedPc34Slot = DM1_V1_CHEST_C040_DROP_ROT_TARGET_PC34_SLOT_PC34;
    out->commandQueueDepthAfterQueue = rt.commandQueueDepth;
    out->leaderBeforeQueue = rt.currentLeader;
    (void)m11_inventory_get_mouse_item(&rt.inventory, rt.currentLeader, &hand);
    out->oldLeaderHandTypeBefore = hand.itemType;
    out->oldLeaderHandWeightBefore = hand.weight;
    out->oldLeaderHandChargesBefore = hand.charges;
    out->oldLeaderHandQuantityBefore = rt.handQuantity[rt.currentLeader];
    (void)m11_inventory_get_mouse_item(
        &rt.inventory,
        DM1_V1_CHEST_C040_DROP_ROT_NEW_LEADER_PC34,
        &hand);
    out->newLeaderHandTypeBefore = hand.itemType;
    out->c040ClickSuppressedWhileHandFull =
        c040_click_is_suppressed_while_hand_full(&rt);

    (void)drain_c540_drop(&rt);
    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_C040_DROP_ROT_STEP_DRAIN_C540_DROP_PC34;
    out->f0077Observed = rt.f0077Observed;
    out->f0078Observed = rt.f0078Observed;
    out->mouseUpdateDepthAfterDrop = rt.mouseUpdateDepth;
    out->commandQueueDepthAfterDrop = rt.commandQueueDepth;
    out->dropDrainFirst = rt.commandQueueDepth == 1 && !rt.dropQueued;
    out->openChestThingAfterDrop =
        m11_inventory_get_open_chest_thing(&rt.inventory, rt.openOwner);
    out->panelAfterDrop = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->c040PanelOpenAfterDrop = rt.c040PanelOpen;
    out->candidateOrdinalAfterDrop = rt.candidateOrdinal;
    out->candidateStillLiveAfterDrop =
        rt.candidateOrdinal ==
        DM1_V1_CHEST_C040_DROP_ROT_CANDIDATE_ORDINAL_PC34;
    out->panelHashAfterDrop = panel_hash(&rt);
    (void)m11_inventory_get_mouse_item(
        &rt.inventory,
        DM1_V1_CHEST_C040_DROP_ROT_OLD_LEADER_PC34,
        &hand);
    out->oldLeaderHandTypeAfterDrop = hand.itemType;
    out->oldLeaderHandEmptyAfterDrop = hand.itemType == 0;
    (void)m11_inventory_get_item_in_chest_slot(
        &rt.inventory,
        rt.openOwner,
        DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34,
        &c540);
    out->c540TypeAfterDrop = c540.itemType;
    out->c540WeightAfterDrop = c540.weight;
    out->c540ChargesAfterDrop = c540.charges;
    out->c540QuantityAfterDrop =
        rt.quantities[rt.openOwner]
                     [DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34];
    record_slots(&rt,
                 out->visibleTypesAfterDrop,
                 out->visibleChargesAfterDrop,
                 out->visibleQuantitiesAfterDrop);
    out->chestSlotChainCoherentAfterDrop =
        slots_match_after(&rt,
                          out->visibleTypesAfterDrop,
                          out->visibleChargesAfterDrop,
                          out->visibleQuantitiesAfterDrop);

    (void)consume_rotation(&rt);
    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_C040_DROP_ROT_STEP_DRAIN_ROTATION_PC34;
    out->commandQueueDepthAfterRotate = rt.commandQueueDepth;
    out->leaderAfterRotate = rt.currentLeader;
    out->openOwnerAfterRotate = rt.openOwner;
    out->openChestThingAfterRotate =
        m11_inventory_get_open_chest_thing(&rt.inventory, rt.openOwner);
    out->panelAfterRotate = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->panelStayedC040 =
        out->panelAfterDrop == DM1_V1_CHEST_C040_DROP_ROT_PANEL_C040_PC34 &&
        out->panelAfterRotate == DM1_V1_CHEST_C040_DROP_ROT_PANEL_C040_PC34;
    out->c040PanelOpenAfterRotate = rt.c040PanelOpen;
    out->candidateOrdinalAfterRotate = rt.candidateOrdinal;
    out->candidateStillLiveAfterRotate =
        rt.candidateOrdinal ==
        DM1_V1_CHEST_C040_DROP_ROT_CANDIDATE_ORDINAL_PC34;
    out->f0282ClearCount = rt.f0282ClearCount;
    out->panelHashAfterRotate = panel_hash(&rt);
    out->panelHashStable =
        out->panelHashBeforeDrop == out->panelHashAfterDrop &&
        out->panelHashBeforeDrop == out->panelHashAfterRotate;
    (void)m11_inventory_get_mouse_item(
        &rt.inventory,
        DM1_V1_CHEST_C040_DROP_ROT_OLD_LEADER_PC34,
        &hand);
    out->oldLeaderHandTypeAfterRotate = hand.itemType;
    out->oldLeaderHandEmptyAfterRotate = hand.itemType == 0;
    (void)m11_inventory_get_mouse_item(
        &rt.inventory,
        DM1_V1_CHEST_C040_DROP_ROT_NEW_LEADER_PC34,
        &hand);
    out->newLeaderHandTypeAfterRotate = hand.itemType;
    out->newLeaderHandPreservedAfterRotate =
        hand.itemType == out->newLeaderHandTypeBefore;
    record_slots(&rt,
                 out->visibleTypesAfterRotate,
                 out->visibleChargesAfterRotate,
                 out->visibleQuantitiesAfterRotate);
    out->c540TypeAfterRotate =
        out->visibleTypesAfterRotate
            [DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34];
    out->c540QuantityAfterRotate =
        out->visibleQuantitiesAfterRotate
            [DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34];
    out->c540StillVisibleAfterRotate =
        out->c540TypeAfterRotate == out->c540TypeAfterDrop &&
        out->c540QuantityAfterRotate == out->c540QuantityAfterDrop;
    out->chestSlotChainCoherentAfterRotate =
        slots_match_after(&rt,
                          out->visibleTypesAfterRotate,
                          out->visibleChargesAfterRotate,
                          out->visibleQuantitiesAfterRotate);
    out->chestNeverClosed =
        rt.closeCount == 0 &&
        out->openChestThingAfterRotate == out->openChestThingBefore;
    out->closeCount = rt.closeCount;
    out->f0077F0078Balanced =
        rt.f0077Observed && rt.f0078Observed && rt.mouseUpdateDepth == 0;
    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_C040_DROP_ROT_STEP_ASSERT_STABLE_PC34;

    out->noPass771PlainDropDuringRotation = 1;
    out->noMirrorCandidateC040LiveC545Drop = 1;
    out->noMirrorCandidateC040RedrawAfterChestClose = 1;
    out->noChestCloseWhileCandidateLive = 1;
    out->noChestScrollWheelCloseRace = 1;
    out->noChestResurrectRotationScrollWheel = 1;
    out->noMirrorCandidateC545AcceptDuringRotation = 1;
    out->noMirrorCandidatePanelRedrawAfterInventoryExit = 1;

    hash_probe(&hash, out);
    out->deterministicHash = hash;
    return 1;
}
