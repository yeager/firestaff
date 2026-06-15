#include "dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CHEST.C F0333:30-32 same-open early return leaves G0426/G0425 intact\n"
    "CHEST.C F0333:53-76 materializes C537..C544 from the chest link order\n"
    "CHAMPION.C F0284:93-130 applies party direction deltas and redraws icons\n"
    "CHAMPION.C F0284:131-178 carries the alternate media party-rotate body\n"
    "CLIKCHAM.C F0368:51-68 switches leader/load ownership without touching the open chest links\n"
    "CHAMPION.C F0302:677-699 reads leader hand and selected C30+ slot, returning before a rejected exchange\n"
    "DEFS.H C30 and C537..C544 define the visible chest source slots";

const DM1_V1_ChestPickupAfterPartyRotateSpecPc34
    dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_pc34_spec = {
        "Runtime gate: party/leader rotation during a pending non-leader-open chest pickup rejects the stale pickup and preserves C537..C544 order.",
        DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PARTY_COUNT,
        DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_OLD_LEADER,
        DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_NEW_LEADER,
        DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_INVENTORY_CHAMPION,
        DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PICKED_INDEX,
        DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PICKED_PC34_SLOT,
        DM1_PC34_SLOT_CHEST_1,
        DM1_PC34_SLOT_CHEST_8,
        DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT,
        "CHEST.C F0333 lines 30-32 same-open early return",
        "CHEST.C F0333 lines 53-76 C537..C544 materialization",
        "CHAMPION.C F0284 lines 93-130 party direction rotation",
        "CHAMPION.C F0284 lines 131-178 alternate party rotation; CLIKCHAM.C F0368 lines 51-68 leader switch",
        "CHAMPION.C F0302 lines 677-699 pre-exchange reject gates",
        "DEFS.H C30 and C537..C544 slot definitions"
    };

typedef struct {
    int leaderIndex;
    int partyDirection;
    int championDirection[DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PARTY_COUNT];
    int championCell[DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PARTY_COUNT];
} DM1_V1_ChestPickupAfterPartyRotatePartyStatePc34;

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

static void seed_chest(M11_Item* items)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT; ++i) {
        items[i] = make_item(
            DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_FIRST_ITEM + i,
            5 + i,
            20 + i);
    }
}

static int normalize_direction(int value)
{
    return value & 3;
}

static void apply_party_direction(
    DM1_V1_ChestPickupAfterPartyRotatePartyStatePc34* party,
    int newDirection)
{
    int delta;
    int i;

    if (!party || party->partyDirection == newDirection) {
        return;
    }

    /* ReDMCSB: CHAMPION.C F0284 lines 117-130 applies the direction delta to
     * every champion Cell/Direction, stores G0308, then redraws changed icons. */
    delta = newDirection - party->partyDirection;
    if (delta < 0) {
        delta += 4;
    }
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PARTY_COUNT; ++i) {
        party->championCell[i] =
            normalize_direction(party->championCell[i] + delta);
        party->championDirection[i] =
            normalize_direction(party->championDirection[i] + delta);
    }
    party->partyDirection = newDirection;
}

static void switch_leader_and_drop_old_hand(
    M11_InventoryState* inventory,
    DM1_V1_ChestPickupAfterPartyRotatePartyStatePc34* party,
    int newLeader)
{
    int oldLeader;

    if (!inventory || !party || newLeader == party->leaderIndex ||
        newLeader < 0 ||
        newLeader >= DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PARTY_COUNT) {
        return;
    }

    oldLeader = party->leaderIndex;
    /* ReDMCSB: CLIKCHAM.C F0368 lines 51-68 clears the old leader, assigns the
     * new leader, and rebalances the global leader-hand load. This Firestaff
     * PC34 slice binds that global-hand handoff as dropping stale per-champion
     * old-leader hand state while preserving the already-full new leader hand. */
    (void)m11_inventory_set_mouse_item(inventory, oldLeader, 0, 0, 0, 0);
    party->leaderIndex = newLeader;
}

static int copy_chest_fields(const M11_InventoryState* state,
                             int champion,
                             int* types,
                             int* weights)
{
    int i;

    if (!state || !types || !weights) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, champion, i, &item)) {
            return 0;
        }
        types[i] = item.itemType;
        weights[i] = item.weight;
    }
    return 1;
}

static int count_type(const int* types, int itemType)
{
    int i;
    int count = 0;

    if (!types || itemType == 0) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT; ++i) {
        if (types[i] == itemType) {
            ++count;
        }
    }
    return count;
}

static int arrays_equal(const int* a, const int* b)
{
    int i;

    if (!a || !b) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int count_visible(const int* types)
{
    int i;
    int count = 0;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int process_in_flight_pickup_after_rotation(
    const DM1_V1_ChestPickupAfterPartyRotatePartyStatePc34* party,
    int leaderAtPointerDown,
    const M11_Item* currentLeaderHand,
    int* refusedLeaderChanged,
    int* refusedHandFull)
{
    if (refusedLeaderChanged) {
        *refusedLeaderChanged = 0;
    }
    if (refusedHandFull) {
        *refusedHandFull = 0;
    }
    if (!party || !currentLeaderHand) {
        return 0;
    }

    /* ReDMCSB: CHAMPION.C F0302 lines 688-699 snapshots G4055 leader-hand and
     * the selected C30+ G0425 slot before any F0298/F0300/F0297 exchange.
     * If leader identity changed while the non-leader chest panel was pending
     * and the new leader hand is already occupied, this regression takes the
     * early-return side of that pre-exchange gate and leaves G0425 untouched. */
    if (party->leaderIndex != leaderAtPointerDown) {
        if (refusedLeaderChanged) {
            *refusedLeaderChanged = 1;
        }
        if (currentLeaderHand->itemType != 0) {
            if (refusedHandFull) {
                *refusedHandFull = 1;
            }
            return 0;
        }
    }
    return 1;
}

static void hash_int(uint32_t* hash, int value)
{
    int i;
    uint32_t v = (uint32_t)value;

    for (i = 0; i < 4; ++i) {
        *hash ^= (v >> (i * 8)) & 0xFFu;
        *hash *= 16777619u;
    }
}

static void hash_case(
    uint32_t* hash,
    const DM1_V1_ChestPickupAfterPartyRotateCasePc34* c)
{
    int i;

    hash_int(hash, c->mode);
    hash_int(hash, c->leaderBeforeRotation);
    hash_int(hash, c->leaderAfterRotation);
    hash_int(hash, c->partyDirectionAfter);
    hash_int(hash, c->inFlightPickupResult);
    hash_int(hash, c->chestLinkStateIntact);
    hash_int(hash, c->reopenOrderPreserved);
    hash_int(hash, c->newLeaderHandAfterType);
    hash_int(hash, c->oldLeaderHandAfterType);
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT; ++i) {
        hash_int(hash, c->reopenedTypes[i]);
        hash_int(hash, c->reopenedWeights[i]);
    }
}

static int run_case(DM1_V1_ChestPickupAfterPartyRotateCasePc34* out,
                    int mode)
{
    M11_InventoryState inventory;
    M11_Item linked[DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT];
    M11_Item oldLeaderHandBefore;
    M11_Item oldLeaderHandAfter;
    M11_Item newLeaderHandBefore;
    M11_Item newLeaderHandAfter;
    DM1_V1_ChestPickupAfterPartyRotatePartyStatePc34 party;
    int inventoryChampion =
        DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_INVENTORY_CHAMPION;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(&party, 0, sizeof(party));
    out->mode = mode;
    out->sourceLockedContractOnly = 0;
    out->partyCount = DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PARTY_COUNT;
    out->oldLeaderIndex = DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_OLD_LEADER;
    out->newLeaderIndex = DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_NEW_LEADER;
    out->inventoryChampionIndex = inventoryChampion;
    out->leaderBeforeRotation = out->oldLeaderIndex;
    out->pickedItemType = DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_FIRST_ITEM +
                          DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PICKED_INDEX;

    for (i = 0; i < DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PARTY_COUNT; ++i) {
        party.championDirection[i] = i;
        party.championCell[i] = i;
    }
    party.leaderIndex = out->oldLeaderIndex;
    party.partyDirection = 0;

    seed_chest(linked);
    m11_inventory_init(&inventory,
                       DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PARTY_COUNT);
    if (!m11_inventory_set_mouse_item(
            &inventory, out->oldLeaderIndex,
            DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_OLD_HAND_ITEM,
            17, 1, DM1_PC34_ALLOWED_CONTAINER) ||
        !m11_inventory_set_mouse_item(
            &inventory, out->newLeaderIndex,
            DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_NEW_HAND_ITEM,
            23, 2, DM1_PC34_ALLOWED_CONTAINER) ||
        !m11_inventory_get_mouse_item(
            &inventory, out->oldLeaderIndex, &oldLeaderHandBefore) ||
        !m11_inventory_get_mouse_item(
            &inventory, out->newLeaderIndex, &newLeaderHandBefore)) {
        return 0;
    }
    out->oldLeaderHandBeforeType = oldLeaderHandBefore.itemType;
    out->newLeaderHandBeforeType = newLeaderHandBefore.itemType;

    /* ReDMCSB: CHEST.C F0333 lines 53-76 copies the linked object order into
     * C537..C544/G0425 for the non-leader inventory champion. */
    out->openResult = m11_inventory_open_chest(
        &inventory, inventoryChampion,
        DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_CHEST_THING,
        linked, DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT);
    out->openChestThing =
        m11_inventory_get_open_chest_thing(&inventory, inventoryChampion);
    if (!out->openResult ||
        !copy_chest_fields(&inventory, inventoryChampion,
                           out->initialTypes, out->initialWeights)) {
        return 0;
    }

    if (mode == DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_MODE_DIRECTION) {
        apply_party_direction(&party, 1);
    } else {
        apply_party_direction(&party, 2);
    }
    switch_leader_and_drop_old_hand(&inventory, &party,
                                    out->newLeaderIndex);
    out->leaderAfterRotation = party.leaderIndex;
    out->partyDirectionBefore = 0;
    out->partyDirectionAfter = party.partyDirection;
    out->oldLeaderDirectionAfter =
        party.championDirection[out->oldLeaderIndex];
    out->newLeaderDirectionAfter =
        party.championDirection[out->newLeaderIndex];
    out->oldLeaderCellAfter = party.championCell[out->oldLeaderIndex];
    out->newLeaderCellAfter = party.championCell[out->newLeaderIndex];

    if (!m11_inventory_get_mouse_item(
            &inventory, out->oldLeaderIndex, &oldLeaderHandAfter) ||
        !m11_inventory_get_mouse_item(
            &inventory, out->newLeaderIndex, &newLeaderHandAfter)) {
        return 0;
    }
    out->oldLeaderHandAfterType = oldLeaderHandAfter.itemType;
    out->newLeaderHandAfterType = newLeaderHandAfter.itemType;

    out->inFlightPickupResult = process_in_flight_pickup_after_rotation(
        &party, out->leaderBeforeRotation, &newLeaderHandAfter,
        &out->refusedBecauseLeaderChanged,
        &out->refusedBecauseNewLeaderHandFull);
    out->rejectedBeforeSlotSwap = out->inFlightPickupResult == 0 ? 1 : 0;
    if (out->inFlightPickupResult) {
        return 0;
    }

    if (!copy_chest_fields(&inventory, inventoryChampion,
                           out->afterRejectTypes,
                           out->afterRejectWeights)) {
        return 0;
    }
    out->chestLinkStateIntact =
        arrays_equal(out->afterRejectTypes, out->initialTypes) &&
        arrays_equal(out->afterRejectWeights, out->initialWeights) ? 1 : 0;

    /* ReDMCSB: CHEST.C F0333 lines 30-32 returns when the same chest is still
     * G0426_T_OpenChest, so the next click sees the same visible C537..C544
     * order rather than rematerializing a shifted or swapped chain. */
    out->sameOpenResult = m11_inventory_open_chest(
        &inventory, inventoryChampion,
        DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_CHEST_THING,
        linked, DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT);
    out->sameOpenChestThing =
        m11_inventory_get_open_chest_thing(&inventory, inventoryChampion);
    if (!out->sameOpenResult ||
        !copy_chest_fields(&inventory, inventoryChampion,
                           out->reopenedTypes,
                           out->reopenedWeights)) {
        return 0;
    }
    out->reopenOrderPreserved =
        arrays_equal(out->reopenedTypes, out->initialTypes) &&
        arrays_equal(out->reopenedWeights, out->initialWeights) ? 1 : 0;
    out->visibleCountAfterReject = count_visible(out->afterRejectTypes);
    out->oldLeaderHandDropped = out->oldLeaderHandAfterType == 0 ? 1 : 0;
    out->newLeaderHandPreserved =
        out->newLeaderHandAfterType == out->newLeaderHandBeforeType ? 1 : 0;
    out->pickedItemCountAfterReject =
        count_type(out->afterRejectTypes, out->pickedItemType);
    out->newLeaderHandItemCountAfterReject =
        count_type(out->afterRejectTypes, out->newLeaderHandAfterType) +
        (out->newLeaderHandAfterType ==
         DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_NEW_HAND_ITEM ? 1 : 0);

    return out->chestLinkStateIntact && out->reopenOrderPreserved &&
           out->oldLeaderHandDropped && out->newLeaderHandPreserved &&
           out->pickedItemCountAfterReject == 1 &&
           out->newLeaderHandItemCountAfterReject == 1;
}

const char*
dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestPickupAfterPartyRotateSpecPc34*
dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_spec_pc34(void)
{
    return &dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_pc34_spec;
}

int dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_run_pc34(
    DM1_V1_ChestPickupAfterPartyRotateProbePc34* out)
{
    uint32_t hash = 2166136261u;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->caseCount = 2;
    if (!run_case(&out->directionCase,
                  DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_MODE_DIRECTION) ||
        !run_case(&out->leaderSwapCase,
                  DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_MODE_LEADER_SWAP)) {
        return 0;
    }
    hash_case(&hash, &out->directionCase);
    hash_case(&hash, &out->leaderSwapCase);
    out->deterministicHash = hash;
    return 1;
}
