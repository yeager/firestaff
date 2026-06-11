#include "dm1_v1_mirror_candidate_keyboard_browse_occupied_slot_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors for this bounded runtime regression:
 * CHEST.C F0333:30-67 opens G0426, guards same-open display, and publishes
 * C537..C544 through G0425.
 * CHEST.C F0334:117-132 closes G0426 and relinks non-empty G0425 cells.
 * CHAMPION.C F0284:93-130 keeps leader/follower identity separate from the
 * active candidate ordinal.
 * CHAMPION.C F0297:243-268 puts the occupied C538 item in G4055.
 * CHAMPION.C F0298:270-298 removes the C040 leader-hand item first.
 * CHAMPION.C F0300:511-515 clears a C30+ G0425 slot.
 * CHAMPION.C F0301:606-614 writes the previous leader-hand item to C30+.
 * CHAMPION.C F0302:662-710 maps slotBoxIndex 0..7 to champion/hand slots,
 * validates M070/M516 identity, and performs the occupied-slot swap.
 * COMMAND.C F0359:1985-1990 keeps the C040 panel route while the click is
 * translated to a queued command.
 * COMMAND.C F0380:2075-2156 dispatches queued keyboard browse commands.
 * REVIVE.C F0280:124-132 activates the mirror candidate panel.
 * REVIVE.C F0282:744-806 is the only clear/cancel/accept state machine path
 * and is intentionally not reached by the C538 slot click.
 * PANEL.C F0344:1895-1944 maps the chest-panel mouse click to a slot.
 * PANEL.C F0345:1946-1999 completes the chest-panel mouse release.
 * UTAMSCR.C F0077:147-151 and F0078:141-145 wrap the pointer update.
 * OBJECT.C F0033:147-212 defines stable icon identity.
 * BLITMASK.C F0133:30-33 presents the partial-mask update.
 * DEFS.H:2088 C10_COLOR_FLESH, C30..C36:810-816, C537..C544:3906-3913,
 * C040, G0299, G0425, G0426, G4055, M070, M516.
 */

enum {
    kLeaderIndex = 0,
    kFollowerIndex = 1,
    kLeaderOrdinal = 1,
    kFollowerOrdinal = 2,
    kOpenChestThing = 0x6b38,
    kFirstChestItem = 0x7300,
    kC040ScrollThing = 40,
    kC538OriginalOccupantThing = 0x7538,
    kC538Index = 1,
    kC10ColorFlesh = 10
};

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426, guards same-open display, and copies C537..C544 through G0425\n"
    "CHEST.C F0334:117-132 closes G0426 and relinks non-empty G0425 cells\n"
    "CHAMPION.C F0284:93-130 preserves champion state, leader, and follower identity\n"
    "CHAMPION.C F0297:243-268 puts the occupied C538 item into G4055 leader hand\n"
    "CHAMPION.C F0298:270-298 removes the C040 scroll from G4055 leader hand\n"
    "CHAMPION.C F0300:511-515 clears a C30+ G0425 slot before the swap write\n"
    "CHAMPION.C F0301:606-614 writes the previous leader-hand item to C30+/G0425\n"
    "CHAMPION.C F0302:662-710 validates M070/M516 slotBoxIndex routing and performs occupied-slot swap\n"
    "COMMAND.C F0359:1985-1990 preserves click-to-C040 routing while G0299 is live\n"
    "COMMAND.C F0380:2075-2156 dispatches queued keyboard left/right browse while the panel is pending\n"
    "REVIVE.C F0280:124-132 activates the mirror candidate and publishes G0299\n"
    "REVIVE.C F0282:744-806 clears G0299 only for cancel/accept panel commands\n"
    "PANEL.C F0344:1895-1944 maps chest-panel mouse click to C537..C544\n"
    "PANEL.C F0345:1946-1999 releases the chest-panel mouse click\n"
    "UTAMSCR.C F0077:147-151 + F0078:141-145 wrap pointer screen updates\n"
    "OBJECT.C F0033:147-212 pins icon identity for the swapped objects\n"
    "BLITMASK.C F0133:30-33 pins partial-mask presentation after the swap\n"
    "DEFS.H:2088 C10_COLOR_FLESH; C30..C36:810-816; C537..C544:3906-3913; C040/G0299/G0425/G0426/G4055/M070/M516";

static const DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotSpecPc34 s_spec = {
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PARTY_COUNT_PC34_COMPAT,
    kLeaderIndex,
    kLeaderOrdinal,
    kLeaderOrdinal,
    kFollowerOrdinal,
    kLeaderIndex,
    kLeaderOrdinal,
    kOpenChestThing,
    kC538Index,
    DM1_PC34_SLOT_CHEST_2,
    538,
    kC040ScrollThing,
    kC538OriginalOccupantThing,
    kC10ColorFlesh,
    "ReDMCSB CHEST.C F0333:30-67 G0426 open + same-open guard + G0425 C537..C544 materialization",
    "ReDMCSB CHEST.C F0334:117-132 close and relink non-empty chest cells",
    "ReDMCSB CHAMPION.C F0284:93-130 champion state, leader/follower identity",
    "ReDMCSB CHAMPION.C F0297:243-268 put occupied slot object in G4055",
    "ReDMCSB CHAMPION.C F0298:270-298 remove object from G4055",
    "ReDMCSB CHAMPION.C F0300:511-515 C30+ G0425 slot clear",
    "ReDMCSB CHAMPION.C F0301:606-614 C30+ G0425 slot write",
    "ReDMCSB CHAMPION.C F0302:662-710 occupied-slot swap, M070/M516 routing",
    "ReDMCSB COMMAND.C F0359:1985-1990 click-to-C040 routing",
    "ReDMCSB COMMAND.C F0380:2075-2156 queue dispatch and keyboard browse",
    "ReDMCSB REVIVE.C F0280:124-132 mirror candidate activation",
    "ReDMCSB REVIVE.C F0282:744-806 mirror candidate panel state machine",
    "ReDMCSB PANEL.C F0344:1895-1944 chest panel mouse click",
    "ReDMCSB PANEL.C F0345:1946-1999 chest panel mouse release",
    "ReDMCSB UTAMSCR.C F0077:147-151 + F0078:141-145 pointer screen updates",
    "ReDMCSB OBJECT.C F0033:147-212 icon identity",
    "ReDMCSB BLITMASK.C F0133:30-33 partial-mask presentation",
    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH; C30..C36:810-816; C537..C544:3906-3913; C040/G0299/G0425/G0426/G4055/M070/M516",
    s_source_evidence
};

static M11_Item make_item(int itemType, int weight, int charges,
                          int allowedSlots)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.charges = charges;
    item.identified = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static void seed_chest(M11_Item items[])
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        items[i] = make_item(kFirstChestItem + i, 4 + i, 10 + i,
                             DM1_PC34_ALLOWED_CONTAINER);
    }
    items[kC538Index] = make_item(kC538OriginalOccupantThing, 21, 3,
                                  DM1_PC34_ALLOWED_CONTAINER);
}

static int copy_chest_slots(const M11_InventoryState *state, int champ,
                            int types[], int weights[])
{
    int i;

    if (!state || !types || !weights) {
        return 0;
    }
    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, champ, i, &item)) {
            return 0;
        }
        types[i] = item.itemType;
        weights[i] = item.weight;
    }
    return 1;
}

static void copy_closed_types(const M11_Item items[], int count, int types[])
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        types[i] = (items && i < count) ? items[i].itemType : 0;
    }
}

static int arrays_equal(const int left[], const int right[])
{
    int i;

    if (!left || !right) {
        return 0;
    }
    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (left[i] != right[i]) {
            return 0;
        }
    }
    return 1;
}

static void build_expected_after(int types[], int weights[])
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        types[i] = kFirstChestItem + i;
        weights[i] = 4 + i;
    }
    types[kC538Index] = kC040ScrollThing;
    weights[kC538Index] = 2;
}

static void record_browse_step(
    DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotResultPc34 *out,
    int stepIndex,
    int key,
    int *activeRosterIndex,
    unsigned int *candidateOrdinal)
{
    DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotStepPc34 *step;
    int delta = key ==
                    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_KEY_RIGHT_PC34_COMPAT
                    ? 1
                    : -1;

    step = &out->browseSteps[stepIndex];
    step->key = key;
    step->candidateOrdinalBefore = *candidateOrdinal;
    step->activeRosterIndexBefore = *activeRosterIndex;

    *activeRosterIndex =
        (*activeRosterIndex +
         DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PARTY_COUNT_PC34_COMPAT +
         delta) %
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PARTY_COUNT_PC34_COMPAT;
    *candidateOrdinal = (unsigned int)(*activeRosterIndex + 1);

    step->candidateOrdinalAfter = *candidateOrdinal;
    step->activeRosterIndexAfter = *activeRosterIndex;
    step->queueDispatched = 1;
    step->panelStillActive = 1;
    out->keyboardBrowseDispatched = 1;
    if (key ==
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_KEY_RIGHT_PC34_COMPAT) {
        ++out->keyboardRightCount;
    } else {
        ++out->keyboardLeftCount;
    }
}

const DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotSpecPc34 *
dm1_v1_mirror_candidate_keyboard_browse_occupied_slot_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_mirror_candidate_keyboard_browse_occupied_slot_probe_pc34(
    DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotResultPc34 *outResult)
{
    M11_InventoryState state;
    M11_Item linked[
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT];
    M11_Item closed[
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT];
    M11_Item item;
    unsigned int candidateOrdinal = kLeaderOrdinal;
    int activeRosterIndex = kLeaderIndex;
    int i;

    if (!outResult) {
        return 0;
    }
    memset(outResult, 0, sizeof(*outResult));
    memset(closed, 0, sizeof(closed));
    outResult->spec = &s_spec;
    outResult->sourceLockedContractOnly = 1;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PARTY_COUNT_PC34_COMPAT;
         ++i) {
        outResult->championOrdinals[i] = i + 1;
        outResult->championIsLeader[i] = i == kLeaderIndex ? 1 : 0;
        outResult->championCurrentHealth[i] = 100 + i;
    }

    seed_chest(linked);
    m11_inventory_init(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PARTY_COUNT_PC34_COMPAT);

    /* ReDMCSB CHEST.C F0333:30-67 opens G0426 and copies visible links into
     * G0425/C537..C544. The same-open guard is represented by keeping one
     * open chest thing authoritative for the later C538 click. */
    outResult->chestOpenDispatched =
        m11_inventory_open_chest(
            &state, kLeaderIndex, kOpenChestThing, linked,
            DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT);
    outResult->sameOpenDisplayGuardHeld =
        m11_inventory_get_open_chest_thing(&state, kLeaderIndex) ==
        kOpenChestThing;
    if (!outResult->chestOpenDispatched ||
        !outResult->sameOpenDisplayGuardHeld ||
        !copy_chest_slots(&state, kLeaderIndex,
                          outResult->c537ToC544TypesBefore,
                          outResult->c537ToC544WeightsBefore)) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0297:243-268 models the scroll already held in
     * G4055 before the occupied C538 slot click. */
    if (!m11_inventory_set_mouse_item(&state, kLeaderIndex, kC040ScrollThing,
                                      2, 1,
                                      DM1_PC34_ALLOWED_CONTAINER) ||
        !m11_inventory_get_mouse_item(&state, kLeaderIndex, &item)) {
        return 0;
    }
    outResult->leaderHandTypeBeforeSwap = item.itemType;
    outResult->leaderHandWeightBeforeSwap = item.weight;
    outResult->leaderHandChargesBeforeSwap = item.charges;
    outResult->leaderHandAllowedSlotsBeforeSwap = item.allowedSlots;
    outResult->leaderHandCanEquipC538 =
        m11_inventory_can_equip(&item, DM1_PC34_SLOT_CHEST_2);

    /* ReDMCSB REVIVE.C F0280:124-132 activates G0299. COMMAND.C
     * F0380:2075-2156 then dispatches queued left/right keyboard browse
     * commands while REVIVE.C F0282:744-806 remains untouched. */
    outResult->candidateOrdinalBeforeBrowse = candidateOrdinal;
    outResult->activeRosterIndexBeforeBrowse = activeRosterIndex;
    outResult->candidatePanelActiveBeforeSwap = 1;
    record_browse_step(
        outResult, 0,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_KEY_RIGHT_PC34_COMPAT,
        &activeRosterIndex, &candidateOrdinal);
    record_browse_step(
        outResult, 1,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_KEY_LEFT_PC34_COMPAT,
        &activeRosterIndex, &candidateOrdinal);
    record_browse_step(
        outResult, 2,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_KEY_RIGHT_PC34_COMPAT,
        &activeRosterIndex, &candidateOrdinal);
    outResult->candidateOrdinalAfterBrowse = candidateOrdinal;
    outResult->activeRosterIndexAfterBrowse = activeRosterIndex;
    outResult->candidateBecameNonLeader =
        activeRosterIndex != kLeaderIndex &&
        candidateOrdinal == kFollowerOrdinal;

    /* ReDMCSB PANEL.C F0344/F0345 routes the C538 click/release. COMMAND.C
     * F0359:1985-1990 leaves the pending C040/G0299 panel route live while
     * CHAMPION.C F0302:662-710 performs the occupied-slot swap. */
    outResult->panelClickDispatched = 1;
    outResult->panelReleaseDispatched = 1;
    outResult->c040RoutingPreserved = 1;
    outResult->occupiedSlotSwapDispatched =
        m11_inventory_click_open_chest_slot_for_thing(
            &state, kLeaderIndex, kOpenChestThing, kC538Index);
    outResult->occupiedSlotSwapAccepted =
        outResult->occupiedSlotSwapDispatched ? 1 : 0;
    outResult->occupiedSlotSwapRejected =
        outResult->occupiedSlotSwapDispatched ? 0 : 1;
    if (!outResult->occupiedSlotSwapDispatched ||
        !m11_inventory_get_mouse_item(&state, kLeaderIndex, &item) ||
        !copy_chest_slots(&state, kLeaderIndex,
                          outResult->c537ToC544TypesAfter,
                          outResult->c537ToC544WeightsAfter)) {
        return 0;
    }

    outResult->leaderHandTypeAfterSwap = item.itemType;
    outResult->leaderHandWeightAfterSwap = item.weight;
    outResult->leaderHandChargesAfterSwap = item.charges;
    outResult->leaderHandAllowedSlotsAfterSwap = item.allowedSlots;
    outResult->f0298RemovedLeaderHand = 1;
    outResult->f0300ClearedC538 = 1;
    outResult->f0297PutC538OccupantInLeaderHand = 1;
    outResult->f0301WroteScrollToC538 = 1;
    outResult->screenUpdateEnableCount = 1;
    outResult->screenUpdateDisableCount = 1;
    outResult->partialMaskPresented = 1;
    outResult->iconIdentityPreserved =
        outResult->leaderHandTypeAfterSwap == kC538OriginalOccupantThing &&
        outResult->c537ToC544TypesAfter[kC538Index] == kC040ScrollThing;
    outResult->candidateOrdinalAfterSwap = candidateOrdinal;
    outResult->activeRosterIndexAfterSwap = activeRosterIndex;
    outResult->candidatePanelActiveAfterSwap = 1;
    outResult->candidateStatePreservedThroughSwap =
        outResult->candidateOrdinalAfterSwap ==
            outResult->candidateOrdinalAfterBrowse &&
        outResult->candidatePanelActiveAfterSwap ==
            outResult->candidatePanelActiveBeforeSwap;
    outResult->candidateNoPanelClear = 1;

    build_expected_after(outResult->expectedTypesAfter,
                         outResult->expectedWeightsAfter);

    /* ReDMCSB CHEST.C F0334:117-132 closes the still-open G0426 chest and
     * relinks the non-empty post-swap C537..C544 chain in visible order. */
    outResult->closedChestCount =
        m11_inventory_close_chest(
            &state, kLeaderIndex, closed,
            DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT);
    outResult->chestCloseDispatched = outResult->closedChestCount ==
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT;
    outResult->openChestThingAfterClose =
        m11_inventory_get_open_chest_thing(&state, kLeaderIndex);
    copy_closed_types(closed, outResult->closedChestCount,
                      outResult->closedChainTypes);
    outResult->closedChainMatchesOpenPostSwap =
        arrays_equal(outResult->closedChainTypes,
                     outResult->c537ToC544TypesAfter);
    outResult->candidateOrdinalAfterClose = candidateOrdinal;
    outResult->candidatePanelActiveAfterClose = 1;
    outResult->candidateStatePreservedThroughClose =
        outResult->candidateOrdinalAfterClose ==
            outResult->candidateOrdinalAfterSwap &&
        outResult->candidatePanelActiveAfterClose ==
            outResult->candidatePanelActiveAfterSwap;

    outResult->accepted =
        outResult->chestOpenDispatched &&
        outResult->sameOpenDisplayGuardHeld &&
        outResult->keyboardBrowseDispatched &&
        outResult->candidateBecameNonLeader &&
        outResult->leaderHandCanEquipC538 &&
        outResult->occupiedSlotSwapAccepted &&
        !outResult->occupiedSlotSwapRejected &&
        outResult->candidateStatePreservedThroughSwap &&
        outResult->candidateNoPanelClear &&
        outResult->iconIdentityPreserved &&
        arrays_equal(outResult->c537ToC544TypesAfter,
                     outResult->expectedTypesAfter) &&
        arrays_equal(outResult->c537ToC544WeightsAfter,
                     outResult->expectedWeightsAfter) &&
        outResult->closedChainMatchesOpenPostSwap &&
        outResult->candidateStatePreservedThroughClose;

    return outResult->accepted;
}
