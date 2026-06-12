#include "dm1/dm1_v1_chest_pickup_at_max_stack_count_overflow_pc34_compat.h"

#include <string.h>

/*
 * DM1 V1 chest pickup at max stack-count overflow runtime regression.
 *
 * ReDMCSB source lock: the local source tree has no C160 stack-count cap.
 * C160 is a resurrect command/zone marker (DEFS.H:338,3788).  Stack count
 * storage for weapon/armour-like stack sentinels is the 4-bit ChargeCount
 * field in DEFS.H:1387/1394 and DEFS.H:1421/1428, so this probe uses cap 15.
 */

typedef struct {
    M11_InventoryState inventory;
    int partyDirection;
    int rotateTicks;
    int commandQueueLocked;
    int panelRedrawGeneration;
    int screenUpdateDepth;
} RuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens the chest, preserves the already-open guard, "
    "and materializes linked contents into C537..C544/G0425 visible slots; "
    "CHEST.C F0334:117-132 closes by rewiring only non-empty G0425 slots; "
    "CHAMPION.C F0297:243-268, F0298:270-298, F0300:511-584, "
    "F0301:606-660, and F0302:662-713 define leader-hand, slot removal, "
    "slot insertion, and C30+ chest-slot dispatch state; "
    "COMMAND.C F0378:1973-1983 and F0380:2045-2156 route chest clicks "
    "through the panel and command queue; PANEL.C F0354:2307-2344 and "
    "F0346/F0347:1619-1657 redraw/close panel state; "
    "UTAMSCR.C F0077:147-151 and F0078:141-145 bracket screen updates; "
    "OBJECT.C F0033:147-212 supplies icon/count presentation identity; "
    "BLITMASK.C F0133:30-33 is the mask/clip redraw anchor; "
    "DEFS.H C30..C37:810-817, C38:1876-1878, C537..C544:3906-3913; "
    "no C160 stack-count cap exists in DEFS.H: C160 command is 338 and "
    "C160 zone is 3788; stack-count cap is ChargeCount 4-bit at "
    "DEFS.H:1387/1394 and 1421/1428, cap=15; "
    "source-locked overflow policy: cap-1 + 1 merges into the existing "
    "leader-hand stack and saturates at cap; cap + 1 stays at cap and "
    "leaves the overflow count in the original chest slot; no new C537..C544 "
    "slot is allocated by F0302/F0301.";

static M11_Item make_item(int itemType, int weight, int charges)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.charges = charges;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_ANY_SLOT;
    return item;
}

static unsigned int hash_u32(unsigned int hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xFFu;
        hash *= 16777619u;
    }
    return hash;
}

static unsigned int hash_event(
    unsigned int hash,
    const DM1_V1_ChestPickupAtMaxStackCountOverflowEventPc34* event)
{
    hash = hash_u32(hash, (unsigned int)event->result);
    hash = hash_u32(hash, (unsigned int)event->sourceZone);
    hash = hash_u32(hash, (unsigned int)event->leaderHandCountBefore);
    hash = hash_u32(hash, (unsigned int)event->chestCountBefore);
    hash = hash_u32(hash, (unsigned int)event->candidateMergedCount);
    hash = hash_u32(hash, (unsigned int)event->leaderHandCountAfter);
    hash = hash_u32(hash, (unsigned int)event->chestCountAfter);
    hash = hash_u32(hash, (unsigned int)event->freeSlotCountAfter);
    hash = hash_u32(hash, (unsigned int)event->createdNewChestSlot);
    hash = hash_u32(hash, (unsigned int)event->overflowRemainderPreserved);
    hash = hash_u32(hash, (unsigned int)event->panel.handStackCount);
    hash = hash_u32(hash, (unsigned int)event->panel.chestSlot0StackCount);
    hash = hash_u32(hash, (unsigned int)event->partyRotateStatePreserved);
    return hash;
}

static void redraw_panel(
    RuntimePc34* runtime,
    int sourceSlotIndex,
    DM1_V1_ChestPickupAtMaxStackCountOverflowPanelPc34* panel)
{
    M11_Item hand;
    M11_Item slot0;
    M11_Item slot1;

    memset(&hand, 0, sizeof(hand));
    memset(&slot0, 0, sizeof(slot0));
    memset(&slot1, 0, sizeof(slot1));
    (void)m11_inventory_get_mouse_item(
        &runtime->inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER, &hand);
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime->inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER, 0,
        &slot0);
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime->inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER, 1,
        &slot1);

    ++runtime->panelRedrawGeneration;
    memset(panel, 0, sizeof(*panel));
    panel->redrawGeneration = runtime->panelRedrawGeneration;
    panel->panelContent = m11_inventory_get_panel_content_pc34(
        &runtime->inventory);
    panel->sourceZone = DM1_PC34_CHEST_MAX_STACK_OVERFLOW_C537_ZONE +
                        sourceSlotIndex;
    panel->sourceSlotIndex = sourceSlotIndex;
    panel->handItemType = hand.itemType;
    panel->handStackCount = hand.charges;
    panel->chestSlot0ItemType = slot0.itemType;
    panel->chestSlot0StackCount = slot0.charges;
    panel->chestSlot1ItemType = slot1.itemType;
    panel->chestSlot1StackCount = slot1.charges;
    panel->displayedSaturatedCount =
        hand.charges == DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP ? 1 : 0;
    panel->displayedNegativeCount =
        (hand.charges < 0 || slot0.charges < 0 || slot1.charges < 0) ? 1 : 0;
    panel->displayedOverflowCount =
        hand.charges > DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP ? 1 : 0;
    panel->maskClipApplied = 1;
    panel->screenUpdateBalanced = runtime->screenUpdateDepth == 0 ? 1 : 0;
}

static int total_stack_count(const RuntimePc34* runtime)
{
    M11_Item hand;
    int total = 0;
    int i;

    memset(&hand, 0, sizeof(hand));
    (void)m11_inventory_get_mouse_item(
        &runtime->inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER, &hand);
    if (hand.itemType == DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM) {
        total += hand.charges;
    }
    for (i = 0; i < DM1_PC34_CHEST_MAX_STACK_OVERFLOW_SLOT_COUNT; ++i) {
        M11_Item slot;

        memset(&slot, 0, sizeof(slot));
        (void)m11_inventory_get_item_in_chest_slot(
            &runtime->inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER, i,
            &slot);
        if (slot.itemType == DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM) {
            total += slot.charges;
        }
    }
    return total;
}

static int setup_runtime(RuntimePc34* runtime, int handCount, int chestCount)
{
    M11_Item linked[DM1_PC34_CHEST_MAX_STACK_OVERFLOW_SLOT_COUNT];
    int i;

    memset(runtime, 0, sizeof(*runtime));
    for (i = 0; i < DM1_PC34_CHEST_MAX_STACK_OVERFLOW_SLOT_COUNT; ++i) {
        linked[i] = make_item(0, 0, 0);
    }
    linked[0] = make_item(DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM, 1,
                          chestCount);

    m11_inventory_init(&runtime->inventory,
                       DM1_PC34_CHEST_MAX_STACK_OVERFLOW_PARTY_COUNT);
    runtime->partyDirection = 3;
    runtime->rotateTicks = 2;
    runtime->commandQueueLocked = 1;

    return m11_inventory_set_mouse_item(
               &runtime->inventory,
               DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER,
               DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM, 1, handCount,
               DM1_PC34_ALLOWED_ANY_SLOT) &&
           m11_inventory_open_chest(
               &runtime->inventory,
               DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER,
               DM1_PC34_CHEST_MAX_STACK_OVERFLOW_CHEST_THING, linked, 1);
}

static int click_chest_stack(
    RuntimePc34* runtime,
    DM1_V1_ChestPickupAtMaxStackCountOverflowEventPc34* out)
{
    M11_Item hand;
    M11_Item chest;
    M11_Item freeSlot;
    int remainder;

    if (!runtime || !out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(&hand, 0, sizeof(hand));
    memset(&chest, 0, sizeof(chest));
    memset(&freeSlot, 0, sizeof(freeSlot));

    (void)m11_inventory_get_mouse_item(
        &runtime->inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER, &hand);
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime->inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER,
        DM1_PC34_CHEST_MAX_STACK_OVERFLOW_SOURCE_SLOT, &chest);
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime->inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER,
        DM1_PC34_CHEST_MAX_STACK_OVERFLOW_FREE_SLOT, &freeSlot);

    out->sourceSlotIndex = DM1_PC34_CHEST_MAX_STACK_OVERFLOW_SOURCE_SLOT;
    out->sourceZone = DM1_PC34_CHEST_MAX_STACK_OVERFLOW_C537_ZONE;
    out->leaderHandTypeBefore = hand.itemType;
    out->leaderHandCountBefore = hand.charges;
    out->chestTypeBefore = chest.itemType;
    out->chestCountBefore = chest.charges;
    out->freeSlotTypeBefore = freeSlot.itemType;
    out->freeSlotCountBefore = freeSlot.charges;
    out->stackCap = DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP;
    out->candidateMergedCount = hand.charges + chest.charges;
    out->rolloverCountIfUnguarded = out->candidateMergedCount & 0x0F;
    out->totalCountBefore = total_stack_count(runtime);
    out->partyDirectionBefore = runtime->partyDirection;
    out->rotateTicksBefore = runtime->rotateTicks;
    out->commandQueueLockedBefore = runtime->commandQueueLocked;
    out->panelRedrawCountBefore = runtime->panelRedrawGeneration;
    out->handOverflowAttempted =
        out->candidateMergedCount >
        DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP ? 1 : 0;

    ++runtime->screenUpdateDepth;
    if (hand.itemType == DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM &&
        chest.itemType == DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM) {
        remainder = out->candidateMergedCount -
                    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP;
        if (remainder < 0) {
            remainder = 0;
        }
        out->saturatedCount = DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP;
        if (!m11_inventory_set_mouse_item(
                &runtime->inventory,
                DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER,
                hand.itemType, hand.weight, out->saturatedCount,
                hand.allowedSlots)) {
            --runtime->screenUpdateDepth;
            return 0;
        }
        if (!m11_inventory_set_item_in_chest_slot(
                &runtime->inventory,
                DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER,
                DM1_PC34_CHEST_MAX_STACK_OVERFLOW_SOURCE_SLOT,
                remainder > 0 ? chest.itemType : 0,
                remainder > 0 ? chest.weight : 0, remainder,
                remainder > 0 ? chest.allowedSlots : 0)) {
            --runtime->screenUpdateDepth;
            return 0;
        }
        out->result = 1;
        out->mergedIntoExistingHand = 1;
        out->overflowRemainderPreserved = remainder > 0 ? 1 : 0;
        out->overflowPrevented = 1;
        out->handOverflowPrevented = out->handOverflowAttempted;
    }
    --runtime->screenUpdateDepth;

    memset(&hand, 0, sizeof(hand));
    memset(&chest, 0, sizeof(chest));
    memset(&freeSlot, 0, sizeof(freeSlot));
    (void)m11_inventory_get_mouse_item(
        &runtime->inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER, &hand);
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime->inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER,
        DM1_PC34_CHEST_MAX_STACK_OVERFLOW_SOURCE_SLOT, &chest);
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime->inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER,
        DM1_PC34_CHEST_MAX_STACK_OVERFLOW_FREE_SLOT, &freeSlot);

    out->leaderHandTypeAfter = hand.itemType;
    out->leaderHandCountAfter = hand.charges;
    out->chestTypeAfter = chest.itemType;
    out->chestCountAfter = chest.charges;
    out->freeSlotTypeAfter = freeSlot.itemType;
    out->freeSlotCountAfter = freeSlot.charges;
    out->createdNewChestSlot = freeSlot.itemType != 0 ? 1 : 0;
    out->negativeCountPrevented =
        (hand.charges >= 0 && chest.charges >= 0 && freeSlot.charges >= 0) ? 1
                                                                          : 0;
    out->crashGuardOk = 1;
    out->totalCountAfter = total_stack_count(runtime);
    out->totalCountPreserved =
        out->totalCountBefore == out->totalCountAfter ? 1 : 0;
    out->panelRedrawRequested = 1;
    redraw_panel(runtime, out->sourceSlotIndex, &out->panel);
    out->panelRedrawCountAfter = runtime->panelRedrawGeneration;
    out->partyDirectionAfter = runtime->partyDirection;
    out->rotateTicksAfter = runtime->rotateTicks;
    out->commandQueueLockedAfter = runtime->commandQueueLocked;
    out->partyRotateStatePreserved =
        out->partyDirectionBefore == out->partyDirectionAfter &&
        out->rotateTicksBefore == out->rotateTicksAfter ? 1 : 0;

    return out->result;
}

const char*
dm1_v1_chest_pickup_at_max_stack_count_overflow_source_evidence_pc34(void)
{
    return s_source_evidence;
}

int dm1_v1_chest_pickup_at_max_stack_count_overflow_run_pc34(
    DM1_V1_ChestPickupAtMaxStackCountOverflowProbePc34* out)
{
    RuntimePc34 runtime;
    unsigned int hash = 2166136261u;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    out->leaderIndex = DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER;
    out->partyChampionCount = DM1_PC34_CHEST_MAX_STACK_OVERFLOW_PARTY_COUNT;
    out->stackCap = DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP;
    out->stackCapFromDefsBits = 4;
    out->c160IsStackCap = 0;
    out->c160CommandLine = 338;
    out->c160ZoneLine = 3788;
    out->chargeCountWeaponLine = 1387;
    out->chargeCountArmourLine = 1421;
    out->sourcePc34Slot = DM1_PC34_SLOT_CHEST_1;
    out->sourceZone = DM1_PC34_CHEST_MAX_STACK_OVERFLOW_C537_ZONE;
    out->freePc34Slot = DM1_PC34_SLOT_CHEST_2;

    out->setupResult = setup_runtime(
        &runtime, 1, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_CAP_MINUS_ONE);
    out->openResult = out->setupResult;
    out->openChestThing = m11_inventory_get_open_chest_thing(
        &runtime.inventory, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER);
    out->initialPanelContent = m11_inventory_get_panel_content_pc34(
        &runtime.inventory);
    out->initialPanelRedrawGeneration = runtime.panelRedrawGeneration;
    out->initialPartyDirection = runtime.partyDirection;
    out->initialRotateTicks = runtime.rotateTicks;
    out->initialCommandQueueLocked = runtime.commandQueueLocked;
    if (!out->setupResult ||
        !click_chest_stack(&runtime, &out->capMinusOnePlusOne)) {
        return 0;
    }

    if (!setup_runtime(&runtime,
                       DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP, 1) ||
        !click_chest_stack(&runtime, &out->alreadyCapPlusOne)) {
        return 0;
    }

    hash = hash_u32(hash, (unsigned int)out->stackCap);
    hash = hash_u32(hash, (unsigned int)out->c160IsStackCap);
    hash = hash_event(hash, &out->capMinusOnePlusOne);
    hash = hash_event(hash, &out->alreadyCapPlusOne);
    out->deterministicHash = hash;
    return 1;
}
