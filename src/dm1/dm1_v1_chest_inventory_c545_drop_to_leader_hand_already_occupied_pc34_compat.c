#include "dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    kLeaderIndex = 0,
    kPartyCount = 1,
    kPanelContentChampion = 5,
    kScrollWeight = 1,
    kOtherThingBase = 0x7200,
    kExpectedHash = 0x329fdb3au
};

typedef struct {
    unsigned char bytes[DM1_PC34_C545_LEADER_HAND_OCCUPIED_SURFACE_BYTES];
} GuardSurface;

typedef struct {
    M11_InventoryState runtime;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int leaderIndex;
    int partyCount;
    int g0305PartyChampionCount;
    int g0423InventoryChampionOrdinal;
    int g0426OpenChestThing;
    int activeSlotIndex;
    int activeC30Slot;
    int activeC537Zone;
    int c040PanelGraphic;
    int c040PanelContent;
    int c040PanelStable;
    int c040PanelRedrawCount;
    int pickupRollbackCount;
    int f0333OpenInitCount;
    int f0334SlotDropSinkCount;
    int f0378C545PanelDispatchCount;
    int f0380QueueDispatchCount;
    int f0380PreservedQueuedCommandIdentity;
    int f0077MouthRouteRedrawCount;
    int f0078MouthRouteRedrawCount;
    int f0033ObjectIdentityCount;
    int f0133RedrawIdentityCount;
    int c545DispatchPathHit;
    int chestSlotFreed;
    int leaderHandStackGrew;
    int c30OwnershipBefore[DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT];
    int c30OwnershipAfter[DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT];
    int chestTypesBefore[DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT];
    int chestCountsBefore[DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT];
    int chestTypesAfter[DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT];
    int chestCountsAfter[DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT];
    int leaderHandTypeBefore;
    int leaderHandIconBefore;
    int leaderHandCountBefore;
    int leaderHandTypeAfter;
    int leaderHandIconAfter;
    int leaderHandCountAfter;
    int chestSlotTypeBefore;
    int chestSlotIconBefore;
    int chestSlotCountBefore;
    int chestSlotTypeAfter;
    int chestSlotIconAfter;
    int chestSlotCountAfter;
    int rejectedGuardSurfaceUnchangedCount;
    int rejectedGuardMutationCount;
    uint32_t deterministic_hash;
} C545Fixture;

static Dm1V1ChestInventoryC545DropToLeaderHandAlreadyOccupiedStatsPc34Compat
    s_lastStats;

static const Dm1V1ChestInventoryC545DropToLeaderHandAlreadyOccupiedEvidencePc34Compat
    s_evidence = {
        1,
        1,
        1,
        "CHEST.C F0333:30-67 chest open/init materializes G0425",
        "CHEST.C F0334:117-132 chest slot/drop sink/C545 dispatch",
        "CHAMPION.C F0297:243-268 leader hand state",
        "CHAMPION.C F0298:270-298 champion hand put/get",
        "CHAMPION.C F0300:511-584 C30..C37 slot removal",
        "CHAMPION.C F0301:606-660 C30 ownership tracking",
        "CHAMPION.C F0302:662-713 party/champion panel slot dispatch",
        "COMMAND.C F0378:1973-1983 C545/panel input dispatch",
        "COMMAND.C F0380:2045-2159 queue dispatch preserves identity",
        "PANEL.C F0354:2307-2344 C040 panel redraw",
        "UTAMSCR.C F0077:147-151/F0078:141-145 mouth-route redraw",
        "OBJECT.C F0033:147-212 object identity",
        "BLITMASK.C F0133:30-33 redraw identity",
        "DEFS.H:810-816 C30..C36 + 3906-3913 C537..C544 + "
            "1874-1878 C38 + 2200 C040 + 2085-2088 G0305 party + "
            "2088-2096 G0423 chest + 5876-5881 G0425/G0426",
        "source_locked_contract_only=1; no_real_asset_bitmap_parity=1; "
            "no_game_data_load=1; pass718 C545 same-icon C038 chest drop "
            "to already occupied leader hand, distinct from C537-C544 runs, "
            "pass706 occupied-slot swap, pass711 panel-live non-leader C538 "
            "pickup, and pass715 C040 close-after-candidate-open"
    };

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 chest open/init materializes G0425\n"
    "CHEST.C F0334:117-132 chest slot / drop sink / C545 dispatch\n"
    "CHAMPION.C F0297:243-268 leader hand state\n"
    "CHAMPION.C F0298:270-298 champion hand put/get\n"
    "CHAMPION.C F0300:511-584 C30..C37 slots\n"
    "CHAMPION.C F0301:606-660 C30 ownership tracking\n"
    "CHAMPION.C F0302:662-713 party/champion panel\n"
    "COMMAND.C F0378:1973-1983 C545/panel input dispatch\n"
    "COMMAND.C F0380:2045-2159 queue dispatch preserves identity\n"
    "PANEL.C F0354:2307-2344 C040 panel redraw\n"
    "UTAMSCR.C F0077:147-151/F0078:141-145 mouth-route redraw\n"
    "OBJECT.C F0033:147-212 object identity\n"
    "BLITMASK.C F0133:30-33 redraw identity\n"
    "DEFS.H:810-816 C30..C36 + 3906-3913 C537..C544 + "
    "1874-1878 C38 + 2200 C040 + 2085-2088 G0305 party + "
    "2088-2096 G0423 chest + 5876-5881 G0425/G0426\n"
    "source_locked_contract_only=1; no_real_asset_bitmap_parity=1; "
    "no_game_data_load=1";

static void record_check(int condition, const char *label, const char *anchor)
{
    ++s_lastStats.assertions;
    if (!condition) {
        ++s_lastStats.failures;
        printf("FAIL %s anchor=%s\n", label, anchor ? anchor : "(null)");
    }
}

static void record_int_eq(int actual, int expected, const char *label,
                          const char *anchor)
{
    ++s_lastStats.assertions;
    if (actual != expected) {
        ++s_lastStats.failures;
        printf("FAIL %s actual=%d expected=%d anchor=%s\n",
               label,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
}

static void record_u32_eq(uint32_t actual, uint32_t expected,
                          const char *label, const char *anchor)
{
    ++s_lastStats.assertions;
    if (actual != expected) {
        ++s_lastStats.failures;
        printf("FAIL %s actual=0x%08x expected=0x%08x anchor=%s\n",
               label,
               (unsigned int)actual,
               (unsigned int)expected,
               anchor ? anchor : "(null)");
    }
}

static void record_contains(const char *haystack, const char *needle,
                            const char *label, const char *anchor)
{
    ++s_lastStats.assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++s_lastStats.failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label,
               needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static M11_Item make_item(int thing, int count)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = thing;
    item.weight = kScrollWeight * count;
    item.charges = count;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static int icon_for_thing(const M11_Item *item)
{
    if (!item || item->itemType == 0) {
        return DM1_PC34_C545_LEADER_HAND_OCCUPIED_NONE;
    }
    if (item->itemType == DM1_PC34_C545_LEADER_HAND_OCCUPIED_SCROLL_THING) {
        return DM1_PC34_C545_LEADER_HAND_OCCUPIED_C038_SCROLL_ICON;
    }
    return item->itemType & 0xff;
}

static void fill_surface(GuardSurface *surface, unsigned int salt)
{
    int i;

    for (i = 0; i < DM1_PC34_C545_LEADER_HAND_OCCUPIED_SURFACE_BYTES; ++i) {
        surface->bytes[i] = (unsigned char)((salt + (unsigned int)(i * 17)) &
                                            0xffu);
    }
}

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t deterministic_hash(const C545Fixture *fixture)
{
    uint32_t hash = 2166136261u;
    int i;

    hash = hash_u32(hash, (uint32_t)fixture->leaderHandTypeAfter);
    hash = hash_u32(hash, (uint32_t)fixture->leaderHandCountAfter);
    hash = hash_u32(hash, (uint32_t)fixture->chestSlotTypeAfter);
    hash = hash_u32(hash, (uint32_t)fixture->chestSlotCountAfter);
    hash = hash_u32(hash, (uint32_t)fixture->c040PanelRedrawCount);
    hash = hash_u32(hash, (uint32_t)fixture->pickupRollbackCount);
    hash = hash_u32(hash, (uint32_t)fixture->f0334SlotDropSinkCount);
    hash = hash_u32(hash, (uint32_t)fixture->f0378C545PanelDispatchCount);
    hash = hash_u32(hash, (uint32_t)fixture->f0380QueueDispatchCount);
    hash = hash_u32(hash,
                    (uint32_t)fixture->f0380PreservedQueuedCommandIdentity);
    for (i = 0; i < DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT; ++i) {
        hash = hash_u32(hash, (uint32_t)fixture->chestTypesAfter[i]);
        hash = hash_u32(hash, (uint32_t)fixture->chestCountsAfter[i]);
        hash = hash_u32(hash, (uint32_t)fixture->c30OwnershipAfter[i]);
    }
    return hash;
}

static int copy_chest(C545Fixture *fixture, int before)
{
    int i;

    for (i = 0; i < DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(
                &fixture->runtime, kLeaderIndex, i, &item)) {
            return 0;
        }
        if (before) {
            fixture->chestTypesBefore[i] = item.itemType;
            fixture->chestCountsBefore[i] = item.charges;
            fixture->c30OwnershipBefore[i] = item.itemType;
        } else {
            fixture->chestTypesAfter[i] = item.itemType;
            fixture->chestCountsAfter[i] = item.charges;
            fixture->c30OwnershipAfter[i] = item.itemType;
        }
    }
    return 1;
}

static int seed_fixture(C545Fixture *fixture)
{
    M11_Item linked[DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT];
    int i;

    memset(fixture, 0, sizeof(*fixture));
    fixture->source_locked_contract_only = 1;
    fixture->no_real_asset_bitmap_parity = 1;
    fixture->no_game_data_load = 1;
    fixture->leaderIndex = kLeaderIndex;
    fixture->partyCount = kPartyCount;
    fixture->g0305PartyChampionCount = kPartyCount;
    fixture->g0423InventoryChampionOrdinal = 1;
    fixture->g0426OpenChestThing =
        DM1_PC34_C545_LEADER_HAND_OCCUPIED_OPEN_CHEST_THING;
    fixture->activeSlotIndex =
        DM1_PC34_C545_LEADER_HAND_OCCUPIED_ACTIVE_INDEX;
    fixture->activeC30Slot =
        DM1_PC34_C545_LEADER_HAND_OCCUPIED_C30_FIRST +
        fixture->activeSlotIndex;
    fixture->activeC537Zone =
        DM1_PC34_C545_LEADER_HAND_OCCUPIED_C537_FIRST +
        fixture->activeSlotIndex;
    fixture->c040PanelGraphic = DM1_PC34_C545_LEADER_HAND_OCCUPIED_C040_PANEL;
    fixture->c040PanelContent = kPanelContentChampion;
    fixture->c040PanelStable = 1;
    fixture->f0333OpenInitCount = 1;

    m11_inventory_init(&fixture->runtime, kPartyCount);
    for (i = 0; i < DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT; ++i) {
        linked[i] = make_item(kOtherThingBase + i, 1);
    }
    linked[fixture->activeSlotIndex] =
        make_item(DM1_PC34_C545_LEADER_HAND_OCCUPIED_SCROLL_THING,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_CHEST_COUNT);
    if (!m11_inventory_open_chest(
            &fixture->runtime,
            kLeaderIndex,
            fixture->g0426OpenChestThing,
            linked,
            DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT)) {
        return 0;
    }
    if (!m11_inventory_set_mouse_item(
            &fixture->runtime,
            kLeaderIndex,
            DM1_PC34_C545_LEADER_HAND_OCCUPIED_SCROLL_THING,
            kScrollWeight * DM1_PC34_C545_LEADER_HAND_OCCUPIED_LEADER_COUNT,
            DM1_PC34_C545_LEADER_HAND_OCCUPIED_LEADER_COUNT,
            DM1_PC34_ALLOWED_CONTAINER)) {
        return 0;
    }
    if (!copy_chest(fixture, 1)) {
        return 0;
    }
    return 1;
}

static int c545_ready(const C545Fixture *fixture, int command)
{
    M11_Item hand;
    M11_Item slot;

    if (!fixture || !fixture->source_locked_contract_only ||
        !fixture->no_real_asset_bitmap_parity || !fixture->no_game_data_load ||
        command != DM1_PC34_C545_LEADER_HAND_OCCUPIED_COMMAND ||
        fixture->leaderIndex != kLeaderIndex ||
        fixture->partyCount != kPartyCount ||
        fixture->g0305PartyChampionCount != kPartyCount ||
        fixture->g0423InventoryChampionOrdinal != 1 ||
        fixture->g0426OpenChestThing !=
            DM1_PC34_C545_LEADER_HAND_OCCUPIED_OPEN_CHEST_THING ||
        fixture->activeSlotIndex < 0 ||
        fixture->activeSlotIndex >=
            DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT ||
        fixture->c040PanelGraphic !=
            DM1_PC34_C545_LEADER_HAND_OCCUPIED_C040_PANEL ||
        fixture->c040PanelContent != kPanelContentChampion ||
        !fixture->c040PanelStable ||
        fixture->pickupRollbackCount != 0) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(&fixture->runtime, kLeaderIndex, &hand) ||
        !m11_inventory_get_item_in_chest_slot(
            &fixture->runtime,
            kLeaderIndex,
            fixture->activeSlotIndex,
            &slot)) {
        return 0;
    }
    return hand.itemType ==
               DM1_PC34_C545_LEADER_HAND_OCCUPIED_SCROLL_THING &&
           slot.itemType ==
               DM1_PC34_C545_LEADER_HAND_OCCUPIED_SCROLL_THING &&
           icon_for_thing(&hand) == icon_for_thing(&slot) &&
           hand.charges == DM1_PC34_C545_LEADER_HAND_OCCUPIED_LEADER_COUNT &&
           slot.charges == DM1_PC34_C545_LEADER_HAND_OCCUPIED_CHEST_COUNT;
}

static int c545_drop_to_leader_hand_same_icon(C545Fixture *fixture,
                                              int command)
{
    M11_Item hand;
    M11_Item slot;
    int newCount;

    if (!c545_ready(fixture, command)) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(&fixture->runtime, kLeaderIndex, &hand) ||
        !m11_inventory_get_item_in_chest_slot(
            &fixture->runtime,
            kLeaderIndex,
            fixture->activeSlotIndex,
            &slot)) {
        return 0;
    }

    fixture->leaderHandTypeBefore = hand.itemType;
    fixture->leaderHandIconBefore = icon_for_thing(&hand);
    fixture->leaderHandCountBefore = hand.charges;
    fixture->chestSlotTypeBefore = slot.itemType;
    fixture->chestSlotIconBefore = icon_for_thing(&slot);
    fixture->chestSlotCountBefore = slot.charges;

    ++fixture->f0380QueueDispatchCount;
    ++fixture->f0378C545PanelDispatchCount;
    fixture->f0380PreservedQueuedCommandIdentity =
        command == DM1_PC34_C545_LEADER_HAND_OCCUPIED_COMMAND;
    ++fixture->f0334SlotDropSinkCount;
    ++fixture->f0033ObjectIdentityCount;
    ++fixture->f0133RedrawIdentityCount;
    fixture->c545DispatchPathHit = 1;

    newCount = hand.charges + slot.charges;
    if (!m11_inventory_set_mouse_item(&fixture->runtime,
                                      kLeaderIndex,
                                      hand.itemType,
                                      kScrollWeight * newCount,
                                      newCount,
                                      DM1_PC34_ALLOWED_CONTAINER) ||
        !m11_inventory_set_item_in_chest_slot(
            &fixture->runtime,
            kLeaderIndex,
            fixture->activeSlotIndex,
            0,
            0,
            0,
            DM1_PC34_ALLOWED_CONTAINER) ||
        !m11_inventory_get_mouse_item(&fixture->runtime,
                                      kLeaderIndex,
                                      &hand) ||
        !m11_inventory_get_item_in_chest_slot(
            &fixture->runtime,
            kLeaderIndex,
            fixture->activeSlotIndex,
            &slot) ||
        !copy_chest(fixture, 0)) {
        return 0;
    }

    fixture->leaderHandTypeAfter = hand.itemType;
    fixture->leaderHandIconAfter = icon_for_thing(&hand);
    fixture->leaderHandCountAfter = hand.charges;
    fixture->chestSlotTypeAfter = slot.itemType;
    fixture->chestSlotIconAfter = icon_for_thing(&slot);
    fixture->chestSlotCountAfter = slot.charges;
    fixture->leaderHandStackGrew =
        hand.charges == DM1_PC34_C545_LEADER_HAND_OCCUPIED_EXPECTED_COUNT;
    fixture->chestSlotFreed = slot.itemType == 0 && slot.charges == 0;
    fixture->deterministic_hash = deterministic_hash(fixture);
    return 1;
}

static int guarded_rejection_preserves_surface(const C545Fixture *base,
                                               int guardKind)
{
    C545Fixture probe;
    GuardSurface before;
    GuardSurface after;
    int accepted;

    if (!base) {
        return 0;
    }
    probe = *base;
    fill_surface(&before, (unsigned int)(0x45u + (unsigned int)guardKind));
    after = before;
    switch (guardKind) {
    case 0:
        probe.source_locked_contract_only = 0;
        break;
    case 1:
        probe.no_real_asset_bitmap_parity = 0;
        break;
    case 2:
        probe.no_game_data_load = 0;
        break;
    case 3:
        probe.c040PanelGraphic = 0;
        break;
    case 4:
        probe.c040PanelStable = 0;
        break;
    case 5:
        probe.pickupRollbackCount = 1;
        break;
    case 6:
        probe.g0426OpenChestThing = 0;
        break;
    case 7:
        probe.activeSlotIndex = DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT;
        break;
    case 8:
        if (!m11_inventory_set_mouse_item(
                &probe.runtime,
                kLeaderIndex,
                0x1234,
                1,
                1,
                DM1_PC34_ALLOWED_CONTAINER)) {
            return 0;
        }
        break;
    case 9:
        if (!m11_inventory_set_item_in_chest_slot(
                &probe.runtime,
                kLeaderIndex,
                DM1_PC34_C545_LEADER_HAND_OCCUPIED_ACTIVE_INDEX,
                0x2234,
                1,
                1,
                DM1_PC34_ALLOWED_CONTAINER)) {
            return 0;
        }
        break;
    case 10:
        probe.g0305PartyChampionCount = 0;
        break;
    case 11:
        probe.g0423InventoryChampionOrdinal = 0;
        break;
    default:
        return 0;
    }

    accepted = c545_drop_to_leader_hand_same_icon(
        &probe, DM1_PC34_C545_LEADER_HAND_OCCUPIED_COMMAND);
    if (accepted) {
        after.bytes[0] ^= 0x5au;
    }
    return !accepted &&
           memcmp(before.bytes, after.bytes, sizeof(before.bytes)) == 0;
}

static void assert_evidence(void)
{
    const char *text =
        dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_source_evidence_pc34_compat();
    const Dm1V1ChestInventoryC545DropToLeaderHandAlreadyOccupiedEvidencePc34Compat
        *e =
            dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_evidence_pc34_compat();

    record_check(e != NULL, "evidence pointer", "COMMAND.C F0380:2045-2159");
    record_int_eq(e->source_locked_contract_only, 1,
                  "source_locked_contract_only",
                  "COMMAND.C F0380:2045-2159");
    record_int_eq(e->no_real_asset_bitmap_parity, 1,
                  "no_real_asset_bitmap_parity",
                  "BLITMASK.C F0133:30-33");
    record_int_eq(e->no_game_data_load, 1,
                  "no_game_data_load", "CHEST.C F0333:30-67");
    record_contains(text, "CHEST.C F0333:30-67",
                    "source cites F0333", e->chestOpenInitAnchor);
    record_contains(text, "CHEST.C F0334:117-132",
                    "source cites F0334", e->chestSlotDropSinkAnchor);
    record_contains(text, "CHAMPION.C F0297:243-268",
                    "source cites F0297", e->leaderHandStateAnchor);
    record_contains(text, "CHAMPION.C F0298:270-298",
                    "source cites F0298", e->championHandPutGetAnchor);
    record_contains(text, "CHAMPION.C F0300:511-584",
                    "source cites F0300", e->c30SlotAnchor);
    record_contains(text, "CHAMPION.C F0301:606-660",
                    "source cites F0301", e->c30OwnershipAnchor);
    record_contains(text, "CHAMPION.C F0302:662-713",
                    "source cites F0302", e->partyChampionPanelAnchor);
    record_contains(text, "COMMAND.C F0378:1973-1983",
                    "source cites F0378", e->c545PanelInputAnchor);
    record_contains(text, "COMMAND.C F0380:2045-2159",
                    "source cites F0380", e->queueDispatchAnchor);
    record_contains(text, "PANEL.C F0354:2307-2344",
                    "source cites F0354", e->c040PanelRedrawAnchor);
    record_contains(text, "UTAMSCR.C F0077:147-151/F0078:141-145",
                    "source cites F0077/F0078", e->mouthRouteRedrawAnchor);
    record_contains(text, "OBJECT.C F0033:147-212",
                    "source cites F0033", e->objectIdentityAnchor);
    record_contains(text, "BLITMASK.C F0133:30-33",
                    "source cites F0133", e->redrawIdentityAnchor);
    record_contains(text, "DEFS.H:810-816 C30..C36",
                    "defs cites C30..C36", e->defsAnchor);
    record_contains(text, "3906-3913 C537..C544",
                    "defs cites C537..C544", e->defsAnchor);
    record_contains(text, "1874-1878 C38",
                    "defs cites C38", e->defsAnchor);
    record_contains(text, "2200 C040",
                    "defs cites C040", e->defsAnchor);
    record_contains(text, "2085-2088 G0305",
                    "defs cites G0305", e->defsAnchor);
    record_contains(text, "2088-2096 G0423 chest",
                    "defs cites G0423", e->defsAnchor);
    record_contains(text, "5876-5881 G0425/G0426",
                    "defs cites G0425/G0426", e->defsAnchor);
    record_contains(text, "source_locked_contract_only=1",
                    "source declares contract-only", e->scope);
    record_contains(text, "no_real_asset_bitmap_parity=1",
                    "source declares no bitmap parity", e->scope);
    record_contains(text, "no_game_data_load=1",
                    "source declares no game data", e->scope);
    record_contains(e->scope, "pass706",
                    "scope distinguishes occupied-slot swap", e->scope);
    record_contains(e->scope, "pass711",
                    "scope distinguishes pass711", e->scope);
    record_contains(e->scope, "pass715",
                    "scope distinguishes pass715", e->scope);
}

static void assert_initial_fixture(const C545Fixture *fixture)
{
    int i;

    record_int_eq(fixture->source_locked_contract_only, 1,
                  "initial contract flag", "COMMAND.C F0380:2045-2159");
    record_int_eq(fixture->no_real_asset_bitmap_parity, 1,
                  "initial bitmap flag", "BLITMASK.C F0133:30-33");
    record_int_eq(fixture->no_game_data_load, 1,
                  "initial data-load flag", "CHEST.C F0333:30-67");
    record_int_eq(fixture->leaderIndex, kLeaderIndex,
                  "initial leader index", "CHAMPION.C F0297:243-268");
    record_int_eq(fixture->partyCount, kPartyCount,
                  "initial party count", "DEFS.H:2085-2088 G0305 party");
    record_int_eq(fixture->g0305PartyChampionCount, kPartyCount,
                  "initial G0305 party", "DEFS.H:2085-2088 G0305 party");
    record_int_eq(fixture->g0423InventoryChampionOrdinal, 1,
                  "initial G0423 inventory champion",
                  "DEFS.H:2088-2096 G0423 chest");
    record_int_eq(fixture->g0426OpenChestThing,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_OPEN_CHEST_THING,
                  "initial G0426 open chest", "CHEST.C F0333:30-67");
    record_int_eq(fixture->activeSlotIndex,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_ACTIVE_INDEX,
                  "initial active slot index", "DEFS.H:3906-3913 C537..C544");
    record_int_eq(fixture->activeC30Slot,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_C30_FIRST,
                  "initial active C30", "DEFS.H:810-816 C30..C36");
    record_int_eq(fixture->activeC537Zone,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_C537_FIRST,
                  "initial active C537", "DEFS.H:3906-3913 C537..C544");
    record_int_eq(fixture->c040PanelGraphic,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_C040_PANEL,
                  "initial C040 panel", "DEFS.H:2200 C040");
    record_int_eq(fixture->c040PanelContent, kPanelContentChampion,
                  "initial panel content", "PANEL.C F0354:2307-2344");
    record_int_eq(fixture->c040PanelStable, 1,
                  "initial panel stable", "PANEL.C F0354:2307-2344");
    record_int_eq(fixture->c040PanelRedrawCount, 0,
                  "initial C040 redraw count", "PANEL.C F0354:2307-2344");
    record_int_eq(fixture->pickupRollbackCount, 0,
                  "initial pickup rollback", "CHAMPION.C F0300:511-584");
    record_int_eq(fixture->f0333OpenInitCount, 1,
                  "initial F0333 count", "CHEST.C F0333:30-67");
    record_int_eq(fixture->f0334SlotDropSinkCount, 0,
                  "initial F0334 count", "CHEST.C F0334:117-132");
    record_int_eq(fixture->f0378C545PanelDispatchCount, 0,
                  "initial F0378 count", "COMMAND.C F0378:1973-1983");
    record_int_eq(fixture->f0380QueueDispatchCount, 0,
                  "initial F0380 count", "COMMAND.C F0380:2045-2159");
    for (i = 0; i < DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT; ++i) {
        record_int_eq(fixture->c30OwnershipBefore[i],
                      fixture->chestTypesBefore[i],
                      "initial C30 ownership mirrors G0425",
                      "CHAMPION.C F0301:606-660");
        record_int_eq(fixture->chestCountsBefore[i], 1,
                      "initial visible stack count",
                      "CHEST.C F0333:30-67");
        record_int_eq(fixture->chestTypesBefore[i] != 0, 1,
                      "initial visible slot occupied",
                      "DEFS.H:5876-5881 G0425/G0426");
    }
    record_int_eq(
        fixture->chestTypesBefore[fixture->activeSlotIndex],
        DM1_PC34_C545_LEADER_HAND_OCCUPIED_SCROLL_THING,
        "initial active chest slot C038 scroll", "OBJECT.C F0033:147-212");
}

static void assert_after_drop(const C545Fixture *fixture)
{
    int i;

    record_int_eq(fixture->c545DispatchPathHit, 1,
                  "C545 dispatch path hit", "COMMAND.C F0378:1973-1983");
    record_int_eq(fixture->f0334SlotDropSinkCount, 1,
                  "F0334 chest drop sink hit", "CHEST.C F0334:117-132");
    record_int_eq(fixture->f0378C545PanelDispatchCount, 1,
                  "F0378 C545 dispatch count", "COMMAND.C F0378:1973-1983");
    record_int_eq(fixture->f0380QueueDispatchCount, 1,
                  "F0380 queue count", "COMMAND.C F0380:2045-2159");
    record_int_eq(fixture->f0380PreservedQueuedCommandIdentity, 1,
                  "F0380 preserved queued command identity",
                  "COMMAND.C F0380:2045-2159");
    record_int_eq(fixture->leaderHandTypeBefore,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_SCROLL_THING,
                  "leader hand before type", "CHAMPION.C F0297:243-268");
    record_int_eq(fixture->leaderHandIconBefore,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_C038_SCROLL_ICON,
                  "leader hand before icon", "OBJECT.C F0033:147-212");
    record_int_eq(fixture->leaderHandCountBefore,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_LEADER_COUNT,
                  "leader hand before count", "CHAMPION.C F0297:243-268");
    record_int_eq(fixture->chestSlotTypeBefore,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_SCROLL_THING,
                  "chest slot before type", "CHEST.C F0334:117-132");
    record_int_eq(fixture->chestSlotIconBefore,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_C038_SCROLL_ICON,
                  "chest slot before icon", "OBJECT.C F0033:147-212");
    record_int_eq(fixture->chestSlotCountBefore,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_CHEST_COUNT,
                  "chest slot before count", "CHEST.C F0334:117-132");
    record_int_eq(fixture->leaderHandTypeAfter,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_SCROLL_THING,
                  "leader hand after type", "CHAMPION.C F0298:270-298");
    record_int_eq(fixture->leaderHandIconAfter,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_C038_SCROLL_ICON,
                  "leader hand after icon", "OBJECT.C F0033:147-212");
    record_int_eq(fixture->leaderHandCountAfter,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_EXPECTED_COUNT,
                  "leader hand stack grew by one", "CHAMPION.C F0298:270-298");
    record_int_eq(fixture->leaderHandStackGrew, 1,
                  "leader hand grew flag", "CHAMPION.C F0298:270-298");
    record_int_eq(fixture->chestSlotTypeAfter, 0,
                  "chest slot type freed", "CHAMPION.C F0300:511-584");
    record_int_eq(fixture->chestSlotIconAfter, 0,
                  "chest slot icon freed", "OBJECT.C F0033:147-212");
    record_int_eq(fixture->chestSlotCountAfter, 0,
                  "chest slot count freed", "CHEST.C F0334:117-132");
    record_int_eq(fixture->chestSlotFreed, 1,
                  "chest slot freed flag", "CHEST.C F0334:117-132");
    record_int_eq(fixture->c040PanelGraphic,
                  DM1_PC34_C545_LEADER_HAND_OCCUPIED_C040_PANEL,
                  "C040 panel graphic stable", "PANEL.C F0354:2307-2344");
    record_int_eq(fixture->c040PanelContent, kPanelContentChampion,
                  "C040 panel content stable", "PANEL.C F0354:2307-2344");
    record_int_eq(fixture->c040PanelRedrawCount, 0,
                  "C040 panel redraw count exactly zero",
                  "PANEL.C F0354:2307-2344");
    record_int_eq(fixture->pickupRollbackCount, 0,
                  "scroll-pickup rollback count zero",
                  "CHAMPION.C F0300:511-584");
    record_int_eq(fixture->f0077MouthRouteRedrawCount, 0,
                  "mouth route enable redraw zero",
                  "UTAMSCR.C F0077:147-151/F0078:141-145");
    record_int_eq(fixture->f0078MouthRouteRedrawCount, 0,
                  "mouth route disable redraw zero",
                  "UTAMSCR.C F0077:147-151/F0078:141-145");
    record_int_eq(fixture->f0033ObjectIdentityCount, 1,
                  "object identity consulted once", "OBJECT.C F0033:147-212");
    record_int_eq(fixture->f0133RedrawIdentityCount, 1,
                  "redraw identity consulted once", "BLITMASK.C F0133:30-33");
    for (i = 0; i < DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT; ++i) {
        const int wasActive = i == fixture->activeSlotIndex;
        record_int_eq(fixture->chestTypesAfter[i],
                      wasActive ? 0 : fixture->chestTypesBefore[i],
                      "after G0425 slot type",
                      "DEFS.H:5876-5881 G0425/G0426");
        record_int_eq(fixture->chestCountsAfter[i],
                      wasActive ? 0 : fixture->chestCountsBefore[i],
                      "after G0425 slot count",
                      "DEFS.H:5876-5881 G0425/G0426");
        record_int_eq(fixture->c30OwnershipAfter[i],
                      fixture->chestTypesAfter[i],
                      "after C30 ownership tracks visible slot",
                      "CHAMPION.C F0301:606-660");
        record_int_eq(fixture->c30OwnershipAfter[i] == 0,
                      wasActive ? 1 : 0,
                      "only active C30 ownership freed",
                      "CHAMPION.C F0301:606-660");
    }
    record_u32_eq(fixture->deterministic_hash, kExpectedHash,
                  "deterministic_hash stable",
                  "COMMAND.C F0380:2045-2159");
}

const Dm1V1ChestInventoryC545DropToLeaderHandAlreadyOccupiedEvidencePc34Compat *
dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_evidence_pc34_compat(
    void)
{
    return &s_evidence;
}

const char *
dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_source_evidence_pc34_compat(
    void)
{
    return s_source_evidence;
}

int run_dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_pc34_compat_self_test(
    void)
{
    C545Fixture fixture;
    int ok;
    int i;

    memset(&s_lastStats, 0, sizeof(s_lastStats));
    assert_evidence();
    ok = seed_fixture(&fixture);
    record_int_eq(ok, 1, "fixture seeded", "CHEST.C F0333:30-67");
    if (!ok) {
        return 0;
    }
    assert_initial_fixture(&fixture);
    for (i = 0; i < 12; ++i) {
        const int guardOk = guarded_rejection_preserves_surface(&fixture, i);
        record_int_eq(guardOk, 1,
                      "guarded rejection preserves caller-owned surface",
                      "BLITMASK.C F0133:30-33");
        if (guardOk) {
            ++fixture.rejectedGuardSurfaceUnchangedCount;
        } else {
            ++fixture.rejectedGuardMutationCount;
        }
    }
    record_int_eq(fixture.rejectedGuardSurfaceUnchangedCount, 12,
                  "all guard surfaces unchanged", "BLITMASK.C F0133:30-33");
    record_int_eq(fixture.rejectedGuardMutationCount, 0,
                  "no guard mutated caller surface", "BLITMASK.C F0133:30-33");
    ok = c545_drop_to_leader_hand_same_icon(
        &fixture, DM1_PC34_C545_LEADER_HAND_OCCUPIED_COMMAND);
    record_int_eq(ok, 1, "C545 same-icon drop accepted",
                  "COMMAND.C F0378:1973-1983");
    if (ok) {
        assert_after_drop(&fixture);
    }
    record_check(s_lastStats.assertions >= 150,
                 "minimum assertion count", "COMMAND.C F0380:2045-2159");
    s_lastStats.deterministic_hash = fixture.deterministic_hash;
    return s_lastStats.failures == 0;
}

Dm1V1ChestInventoryC545DropToLeaderHandAlreadyOccupiedStatsPc34Compat
dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_last_stats_pc34_compat(
    void)
{
    return s_lastStats;
}
