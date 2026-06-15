#include "dm1/dm1_v1_chest_empty_party_group_cleanup_pc34_compat.h"

#include <string.h>

static const char s_f0333_anchor[] =
    "CHEST.C:F0333_INVENTORY_OpenAndDrawChest:53-76 copies up to eight "
    "linked objects into G0425_aT_ChestSlots and fills the rest with NONE.";

static const char s_f0334_anchor[] =
    "CHEST.C:F0334_INVENTORY_CloseChest:113-132 returns when no G0426 chest "
    "is open, sets Container->Slot to ENDOFLIST, skips NONE slots, and "
    "clears G0425 entries while closing.";

static const char s_f0163_anchor[] =
    "DUNGEON.C:F0163_DUNGEON_LinkThingToList:1796-1837 clears the linked "
    "thing Next to ENDOFLIST, then appends it to a square/list tail.";

static const char s_f0164_anchor[] =
    "DUNGEON.C:F0164_DUNGEON_UnlinkThingFromList:1879-1918 ignores "
    "ENDOFLIST, clears cell bits, removes a last square thing by clearing "
    "the square-list-present bit and decrementing cumulative counts, or "
    "splices head/tail links otherwise.";

static const char s_f0172_anchor[] =
    "DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2517-2523,2670-2721 reads the "
    "square first thing after list cleanup and publishes M550_FIRST_THING.";

static const char s_f0302_anchor[] =
    "CHAMPION.C:F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:688-710 "
    "uses the current leader hand and C30+ G0425 chest slots as the routed "
    "open-chest surface.";

static const DM1_V1_ChestEmptyPartyGroupCleanupSpecPc34 s_spec = {
    1,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_NONE,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_NOT_ON_SQUARE,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_FLOOR_THING,
    s_f0333_anchor,
    s_f0334_anchor,
    s_f0163_anchor,
    s_f0164_anchor,
    s_f0172_anchor,
    s_f0302_anchor,
    "contract_only=1; synthetic empty-party-square cleanup gate for chest "
    "close/list teardown, without real-asset or original-DOS parity claims."
};

typedef struct {
    int thing;
    int next;
} ThingNodePc34;

typedef struct {
    int squareListPresent;
    int firstThing;
    int squareFirstThingCount;
    int cumulativeColumnCounts[4];
    ThingNodePc34 nodes[3];
    int removedNextAfterUnlink;
    int cellBitsCleared;
} SquareListPc34;

static M11_Item make_item(int itemType, int weight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static void init_square(SquareListPc34* square)
{
    memset(square, 0, sizeof(*square));
    square->firstThing = DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END;
    square->removedNextAfterUnlink =
        DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END;
}

static ThingNodePc34* node_for_thing(SquareListPc34* square, int thing)
{
    int i;

    if (!square) {
        return 0;
    }
    for (i = 0; i < 3; ++i) {
        if (square->nodes[i].thing == thing) {
            return &square->nodes[i];
        }
    }
    return 0;
}

static void seed_square_only(SquareListPc34* square, int firstThing)
{
    init_square(square);
    square->squareListPresent = 1;
    square->firstThing = firstThing;
    square->squareFirstThingCount = 1;
    square->cumulativeColumnCounts[1] = 1;
    square->cumulativeColumnCounts[2] = 1;
    square->nodes[0].thing = firstThing;
    square->nodes[0].next = DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END;
}

static void seed_square_pair(SquareListPc34* square, int firstThing, int secondThing)
{
    seed_square_only(square, firstThing);
    square->nodes[0].next = secondThing;
    square->nodes[1].thing = secondThing;
    square->nodes[1].next = DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END;
}

static int link_thing_to_empty_square(SquareListPc34* square, int thing)
{
    if (!square || thing == DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END) {
        return 0;
    }

    /* ReDMCSB DUNGEON.C F0163 lines 1796-1837 clears Next to ENDOFLIST
     * before appending or becoming a square/list head. */
    seed_square_only(square, thing);
    return 1;
}

static int unlink_thing_from_square(SquareListPc34* square, int thing)
{
    ThingNodePc34* node;
    ThingNodePc34* prev;
    int previousThing;

    if (!square || thing == DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END) {
        /* ReDMCSB DUNGEON.C F0164 lines 1879-1880 returns for ENDOFLIST. */
        return 0;
    }
    square->cellBitsCleared = 1;
    node = node_for_thing(square, thing);
    if (!node || !square->squareListPresent) {
        /* ReDMCSB DUNGEON.C F0164 lines 1909-1918 scans the current list;
         * the synthetic contract reports no mutation when the thing is absent. */
        return 0;
    }

    if (square->firstThing == thing &&
        node->next == DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END) {
        /* ReDMCSB DUNGEON.C F0164 lines 1886-1895 clears the square list
         * present bit, removes the square-first entry, and decrements later
         * cumulative column counts when the removed thing was the last one. */
        square->squareListPresent = 0;
        square->firstThing = DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END;
        square->squareFirstThingCount = 0;
        square->cumulativeColumnCounts[1] = 0;
        square->cumulativeColumnCounts[2] = 0;
        square->removedNextAfterUnlink = node->next;
        node->next = DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END;
        return 1;
    }

    if (square->firstThing == thing) {
        /* ReDMCSB DUNGEON.C F0164 lines 1900-1902 replaces a head thing
         * with its Next when other things remain on the square. */
        square->firstThing = node->next;
        square->removedNextAfterUnlink = node->next;
        node->next = DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END;
        return 1;
    }

    previousThing = square->firstThing;
    prev = node_for_thing(square, previousThing);
    while (prev && prev->next != DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END &&
           prev->next != thing) {
        previousThing = prev->next;
        prev = node_for_thing(square, previousThing);
    }
    if (!prev || prev->next != thing) {
        return 0;
    }

    /* ReDMCSB DUNGEON.C F0164 lines 1909-1918 walks from the list head and
     * splices the previous thing's Next around the removed tail/interior node. */
    prev->next = node->next;
    square->removedNextAfterUnlink = node->next;
    node->next = DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END;
    return 1;
}

static int cleanup_if_empty_close(int closeCount, SquareListPc34* square)
{
    if (closeCount != 0) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0334 lines 117-132 rewrites no visible links when all
     * G0425 slots are NONE; DUNGEON.C F0164 lines 1886-1895 is then the
     * contract-only square cleanup modeled for this empty party-square slice. */
    return unlink_thing_from_square(
        square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING);
}

const char*
dm1_v1_chest_empty_party_group_cleanup_source_evidence_pc34(void)
{
    return
        "CHEST.C F0333:53-76 open materialization and trailing NONE fill\n"
        "CHEST.C F0334:113-132 close no-open return, visible-slot rewrite, "
        "G0425 clear, and empty close count\n"
        "DUNGEON.C F0163:1796-1837 append/Next reset model\n"
        "DUNGEON.C F0164:1879-1918 last-square thing cleanup and head/tail "
        "splice model\n"
        "DUNGEON.C F0172:2517-2523,2670-2721 first-thing aspect read after "
        "cleanup\n"
        "CHAMPION.C F0302:688-710 C30+ G0425 route ownership";
}

const DM1_V1_ChestEmptyPartyGroupCleanupSpecPc34*
dm1_v1_chest_empty_party_group_cleanup_spec_pc34(void)
{
    return &s_spec;
}

static int run_empty_close_case(
    DM1_V1_ChestEmptyPartyGroupCleanupEmptyClosePc34* out)
{
    M11_InventoryState state;
    M11_Item item;
    M11_Item closed[DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT];
    SquareListPc34 square;

    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    m11_inventory_init(&state, 1);
    seed_square_only(&square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING);

    /* ReDMCSB CHEST.C F0334 lines 113-114 returns when no G0426 chest is open. */
    out->noOpenCloseCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT);

    /* ReDMCSB CHEST.C F0333 lines 53-76 opens an empty container by filling
     * every visible G0425 chest slot with NONE. */
    out->openResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING, 0, 0);
    out->openThingAfterOpen = m11_inventory_get_open_chest_thing(&state, 0);
    if (!m11_inventory_get_item_in_chest_slot(&state, 0, 0, &item)) {
        return 0;
    }
    out->emptyVisibleSlot0 = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT - 1,
            &item)) {
        return 0;
    }
    out->emptyVisibleSlot7 = item.itemType;

    out->emptyCloseSquarePresentBefore = square.squareListPresent;
    out->emptyCloseFirstThingBefore = square.firstThing;
    out->emptyCloseSquareFirstThingCountBefore = square.squareFirstThingCount;
    out->emptyCloseColumn1Before = square.cumulativeColumnCounts[1];
    out->emptyCloseColumn2Before = square.cumulativeColumnCounts[2];

    /* ReDMCSB CHEST.C F0334 lines 117-132 closes G0426 and rewrites only
     * non-empty visible slots; an empty visible set returns count zero here. */
    out->closeCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT);
    out->openThingAfterClose = m11_inventory_get_open_chest_thing(&state, 0);
    out->getClosedSlotResult = m11_inventory_get_item_in_chest_slot(
        &state, 0, 0, &item);

    out->emptyCloseCleanupApplied =
        cleanup_if_empty_close(out->closeCount, &square);
    out->emptyCloseSquarePresentAfter = square.squareListPresent;
    out->emptyCloseFirstThingAfter = square.firstThing;
    out->emptyCloseSquareFirstThingCountAfter = square.squareFirstThingCount;
    out->emptyCloseColumn1After = square.cumulativeColumnCounts[1];
    out->emptyCloseColumn2After = square.cumulativeColumnCounts[2];
    out->emptyCloseRemovedNextAfterUnlink = square.removedNextAfterUnlink;
    out->emptyCloseCellBitsCleared = square.cellBitsCleared;

    out->closeAgainCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT);
    return 1;
}

static int run_square_cases(
    DM1_V1_ChestEmptyPartyGroupCleanupSquareCasesPc34* out)
{
    SquareListPc34 square;
    ThingNodePc34* node;

    memset(out, 0, sizeof(*out));
    seed_square_pair(&square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING);
    out->headPresentBefore = square.squareListPresent;
    out->headFirstBefore = square.firstThing;
    out->headCountBefore = square.squareFirstThingCount;
    node = node_for_thing(
        &square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING);
    out->headChestNextBefore = node ? node->next : 0;
    out->headCleanupApplied = unlink_thing_from_square(
        &square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING);
    out->headPresentAfter = square.squareListPresent;
    out->headFirstAfter = square.firstThing;
    out->headCountAfter = square.squareFirstThingCount;
    node = node_for_thing(
        &square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING);
    out->headChestNextAfter = node ? node->next : 0;
    node = node_for_thing(
        &square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING);
    out->headSensorNextAfter = node ? node->next : 0;

    seed_square_pair(&square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING);
    out->tailPresentBefore = square.squareListPresent;
    out->tailFirstBefore = square.firstThing;
    out->tailCountBefore = square.squareFirstThingCount;
    node = node_for_thing(
        &square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING);
    out->tailSensorNextBefore = node ? node->next : 0;
    out->tailCleanupApplied = unlink_thing_from_square(
        &square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING);
    out->tailPresentAfter = square.squareListPresent;
    out->tailFirstAfter = square.firstThing;
    out->tailCountAfter = square.squareFirstThingCount;
    node = node_for_thing(
        &square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING);
    out->tailSensorNextAfter = node ? node->next : 0;
    node = node_for_thing(
        &square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING);
    out->tailChestNextAfter = node ? node->next : 0;

    seed_square_only(&square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING);
    out->missingCleanupResult = unlink_thing_from_square(
        &square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING);
    out->endOfListCleanupResult = unlink_thing_from_square(
        &square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END);
    return 1;
}

static int run_open_close_cycle(
    DM1_V1_ChestEmptyPartyGroupCleanupOpenClosePc34* out)
{
    M11_InventoryState state;
    M11_Item linked[2];
    M11_Item closed[DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT];
    SquareListPc34 square;
    int i;

    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    m11_inventory_init(&state, 1);
    linked[0] = make_item(DM1_PC34_CHEST_EMPTY_PARTY_GROUP_ITEM0, 4);
    linked[1] = make_item(DM1_PC34_CHEST_EMPTY_PARTY_GROUP_ITEM1, 9);
    seed_square_only(&square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING);

    /* ReDMCSB CHEST.C F0333 lines 53-76 materializes only visible linked
     * chest objects, and F0334 lines 117-132 preserves non-empty slots. */
    out->openResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING, linked, 2);
    out->closeCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT);
    for (i = 0; i < DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT; ++i) {
        out->closedTypes[i] = closed[i].itemType;
        out->closedWeights[i] = closed[i].weight;
    }
    out->cleanupAttempted = cleanup_if_empty_close(out->closeCount, &square);
    out->squarePresentAfterClose = square.squareListPresent;
    out->firstThingAfterClose = square.firstThing;
    out->closeClearsOpenThing =
        m11_inventory_get_open_chest_thing(&state, 0) == 0 ? 1 : 0;

    /* ReDMCSB DUNGEON.C F0163 lines 1796-1837 can re-link the same synthetic
     * chest thing before another CHEST.C F0333/F0334 empty open-close cycle. */
    if (!link_thing_to_empty_square(
            &square, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING)) {
        return 0;
    }
    out->reopenEmptyResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_REOPEN_THING, 0, 0);
    out->reopenEmptyCloseCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT);
    out->reopenEmptyCleanupApplied =
        cleanup_if_empty_close(out->reopenEmptyCloseCount, &square);
    out->reopenEmptySquarePresentAfter = square.squareListPresent;
    out->reopenEmptyFirstThingAfter = square.firstThing;
    return 1;
}

int dm1_v1_chest_empty_party_group_cleanup_pc34(
    DM1_V1_ChestEmptyPartyGroupCleanupProbePc34* out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->contractOnly = 1;
    return run_empty_close_case(&out->emptyClose) &&
           run_square_cases(&out->squareCases) &&
           run_open_close_cycle(&out->openClose);
}
