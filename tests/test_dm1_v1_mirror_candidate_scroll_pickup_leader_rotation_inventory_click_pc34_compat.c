/* ReDMCSB source-lock evidence:
 * CHEST.C F0333:30-67 opens G0426 and materializes C30+/G0425 chest slots.
 * CHEST.C F0334:113-132 closes G0426 by relinking non-empty G0425 entries.
 * CHAMPION.C F0297:243-298 and F0298:270-298 own leader-hand identity.
 * CHAMPION.C F0302:662-710 owns occupied-slot and C30+ click dispatch.
 * CHAMPION.C F0284:93-130 cascades leader rotation across champions.
 * CHAMDRAW.C F0291:498-525 and F0293:1117-1143 redraw slots/states.
 * COMMAND.C F0359:1985-1990 dispatches pending M568/C040 panel input.
 * REVIVE.C F0280:124-132 opens and F0282:744-806 clears candidate state.
 */
#include "dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gAssertions;
static int gFailures;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++gAssertions;
    if (!condition) {
        ++gFailures;
        printf("FAIL: %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%d expected=%d [%s]\n",
               message, actual, expected, anchor ? anchor : "(null)");
    }
}

static void check_uint_eq(unsigned int actual, unsigned int expected,
                          const char *message, const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%u expected=%u [%s]\n",
               message, actual, expected, anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++gAssertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++gFailures;
        printf("FAIL: %s missing=%s [%s]\n",
               message, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static int expected_item_type_for_initial_index(int index)
{
    if (index == 2) {
        return DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_C040_PC34_COMPAT;
    }
    return 0x7100 + index;
}

static int expected_item_weight_for_initial_index(int index)
{
    if (index == 2) {
        return 9;
    }
    return 4 + index;
}

static void test_source_evidence(void)
{
    const char *evidence =
        dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_source_evidence_pc34();

    check_contains(evidence, "CHEST.C F0333:30-67",
                   "source evidence cites chest open",
                   "CHEST.C F0333:30-67");
    check_contains(evidence, "CHEST.C F0334:113-132",
                   "source evidence cites chest close",
                   "CHEST.C F0334:113-132");
    check_contains(evidence, "CHAMPION.C F0297:243-298",
                   "source evidence cites leader-hand put",
                   "CHAMPION.C F0297:243-298");
    check_contains(evidence, "F0298:270-298",
                   "source evidence cites leader-hand remove",
                   "CHAMPION.C F0298:270-298");
    check_contains(evidence, "CHAMPION.C F0302:662-710",
                   "source evidence cites slot dispatch",
                   "CHAMPION.C F0302:662-710");
    check_contains(evidence, "CHAMPION.C F0284:93-130",
                   "source evidence cites rotation cascade",
                   "CHAMPION.C F0284:93-130");
    check_contains(evidence, "CHAMDRAW.C F0291:498-525",
                   "source evidence cites slot render",
                   "CHAMDRAW.C F0291:498-525");
    check_contains(evidence, "F0293:1117-1143",
                   "source evidence cites champion-state redraw",
                   "CHAMDRAW.C F0293:1117-1143");
    check_contains(evidence, "COMMAND.C F0359:1985-1990",
                   "source evidence cites C040 dispatch",
                   "COMMAND.C F0359:1985-1990");
    check_contains(evidence, "REVIVE.C F0280:124-132",
                   "source evidence cites panel open",
                   "REVIVE.C F0280:124-132");
    check_contains(evidence, "F0282:744-806",
                   "source evidence cites panel clear",
                   "REVIVE.C F0282:744-806");
    check_contains(evidence, "DEFS.H C30/G0425/G0426/G0423/G0305/M070/M516/C040",
                   "source evidence cites required DEFS names",
                   "DEFS.H C30/G0425/G0426/G0423/G0305/M070/M516/C040");
}

static void test_spec_metadata(void)
{
    const DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Spec
        *spec =
            dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_spec_pc34();

    check_true(spec != NULL, "spec accessor returns data",
               "COMMAND.C F0359:1985-1990");
    check_int_eq(spec->partyCount, 4, "spec party count",
                 spec->defsAnchor);
    check_int_eq(spec->leaderIndex, 0, "spec leader index",
                 spec->leaderHandPutAnchor);
    check_uint_eq(spec->inventoryChampionOrdinal, 1u,
                  "spec inventory champion ordinal", spec->defsAnchor);
    check_uint_eq(spec->candidateChampionOrdinal, 3u,
                  "spec candidate champion ordinal", spec->candidatePanelOpenAnchor);
    check_int_eq(spec->pickedChestSlotIndex, 2,
                 "spec picks third visible chest slot", spec->slotDispatchAnchor);
    check_int_eq(spec->pickedPc34Slot, DM1_PC34_SLOT_CHEST_3,
                 "spec uses canonical C30+ chest slot", spec->defsAnchor);
    check_int_eq(spec->c040PanelGraphic,
                 DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_C040_PC34_COMPAT,
                 "spec uses C040 panel/hand sentinel", spec->candidateDispatchAnchor);
    check_int_eq(spec->c175PortraitZone,
                 DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_C175_PORTRAIT_PC34_COMPAT,
                 "spec targets the inventory portrait zone", spec->slotDispatchAnchor);
    check_int_eq(spec->directionBefore, 0, "spec starts north",
                 spec->leaderRotationAnchor);
    check_int_eq(spec->directionAfter, 1, "spec rotates east",
                 spec->leaderRotationAnchor);
    check_contains(spec->contractScope, "open chest",
                   "contract names open chest overlap", spec->contractScope);
    check_contains(spec->contractScope, "leader-hand pickup",
                   "contract names leader-hand pickup overlap",
                   spec->contractScope);
    check_contains(spec->contractScope, "inventory portrait click",
                   "contract names inventory portrait click overlap",
                   spec->contractScope);
}

static void check_expected_link_order(
    const DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Result
        *result,
    const char *anchor)
{
    int i;
    int expectedCount = 0;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];

        snprintf(label, sizeof(label), "expected link order slot %d", i);
        check_int_eq(result->expectedChestLinkOrder[i],
                     i == 7 ? 0 : (i < 2 ? 0x7100 + i : 0x7101 + i),
                     label, anchor);
    }
    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (result->expectedChestLinkOrder[i] != 0) {
            ++expectedCount;
        }
    }
    check_int_eq(result->expectedChestLinkCount, expectedCount,
                 "expected link count matches non-empty expected entries",
                 anchor);
}

static void check_positive_result(
    const DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Result
        *result)
{
    const DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Spec
        *spec = result->spec;
    int i;

    check_int_eq(result->path,
                 DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PATH_PORTRAIT_CLICK_PC34_COMPAT,
                 "positive path id", spec->candidateDispatchAnchor);
    check_true(result->accepted, "positive path accepted",
               spec->contractScope);
    check_true(result->chestOpenDispatched, "chest open dispatched",
               spec->chestOpenAnchor);
    check_true(result->scrollPickupDispatched, "scroll pickup dispatched",
               spec->slotDispatchAnchor);
    check_true(result->resurrectPanelOpened, "C040 panel opened",
               spec->candidatePanelOpenAnchor);
    check_true(result->leaderRotationDispatched, "leader rotation dispatched",
               spec->leaderRotationAnchor);
    check_true(result->inventoryPortraitClickDispatched,
               "inventory portrait click dispatched", spec->slotDispatchAnchor);
    check_true(!result->chestCloseDispatched,
               "positive path keeps chest open", spec->chestCloseAnchor);
    check_true(result->portraitClickRejectedByCandidate,
               "portrait click is rejected by candidate guard",
               spec->candidateDispatchAnchor);
    check_true(result->candidatePanelVisible,
               "candidate panel remains visible", spec->candidateDispatchAnchor);
    check_true(result->candidateStatePreserved,
               "candidate ordinal/graphic preserved", spec->candidateDispatchAnchor);
    check_true(result->candidateNoReinit,
               "candidate panel is not reinitialized", spec->candidatePanelOpenAnchor);
    check_true(result->leaderHandStillC040,
               "leader hand still carries C040 pickup", spec->leaderHandPutAnchor);
    check_true(result->leaderHandStatePreservedThroughRotation,
               "leader hand is preserved through rotation", spec->leaderRotationAnchor);
    check_true(result->chestLinkOrderPreserved,
               "chest visible link order preserved", spec->chestOpenAnchor);
    check_true(result->c30SlotTypesWeightsByteEqualPrePost,
               "C30+ slot type/weight arrays byte-equal across rotation/click",
               spec->slotRenderAnchor);
    check_true(result->championRotationCascadeApplied,
               "champion rotation cascade applied", spec->leaderRotationAnchor);

    check_uint_eq(result->candidateOrdinalBefore, spec->candidateChampionOrdinal,
                  "candidate before ordinal", spec->candidatePanelOpenAnchor);
    check_uint_eq(result->candidateOrdinalAfter, spec->candidateChampionOrdinal,
                  "candidate after ordinal", spec->candidateDispatchAnchor);
    check_int_eq(result->candidatePanelOpenCountBeforeFinal, 1,
                 "candidate panel opened once before final click",
                 spec->candidatePanelOpenAnchor);
    check_int_eq(result->candidatePanelOpenCountAfterFinal, 1,
                 "candidate panel open count not incremented",
                 spec->candidateDispatchAnchor);
    check_int_eq(result->candidatePanelGraphicBefore, spec->c040PanelGraphic,
                 "candidate graphic before is C040", spec->candidateDispatchAnchor);
    check_int_eq(result->candidatePanelGraphicAfter, spec->c040PanelGraphic,
                 "candidate graphic after is C040", spec->candidateDispatchAnchor);

    check_int_eq(result->leaderHandTypeBeforeRotation, spec->c040PanelGraphic,
                 "leader hand before rotation is C040", spec->leaderHandPutAnchor);
    check_int_eq(result->leaderHandTypeAfterRotation, spec->c040PanelGraphic,
                 "leader hand after rotation is C040", spec->leaderRotationAnchor);
    check_int_eq(result->leaderHandTypeAfterFinal, spec->c040PanelGraphic,
                 "leader hand after portrait click is C040", spec->slotDispatchAnchor);
    check_int_eq(result->leaderHandWeightBeforeRotation, 9,
                 "leader hand weight before rotation", spec->leaderHandPutAnchor);
    check_int_eq(result->leaderHandWeightAfterRotation, 9,
                 "leader hand weight after rotation", spec->leaderRotationAnchor);
    check_int_eq(result->leaderHandWeightAfterFinal, 9,
                 "leader hand weight after final click", spec->slotDispatchAnchor);
    check_int_eq(result->leaderHandChargesBeforeRotation, 1,
                 "leader hand charges before rotation", spec->leaderHandPutAnchor);
    check_int_eq(result->leaderHandChargesAfterRotation, 1,
                 "leader hand charges after rotation", spec->leaderRotationAnchor);
    check_int_eq(result->leaderHandChargesAfterFinal, 1,
                 "leader hand charges after final click", spec->slotDispatchAnchor);
    check_int_eq(result->leaderHandAllowedSlotsBeforeRotation,
                 DM1_PC34_ALLOWED_ANY_SLOT,
                 "leader hand allowed slots before rotation", spec->defsAnchor);
    check_int_eq(result->leaderHandAllowedSlotsAfterRotation,
                 DM1_PC34_ALLOWED_ANY_SLOT,
                 "leader hand allowed slots after rotation", spec->defsAnchor);
    check_int_eq(result->leaderHandAllowedSlotsAfterFinal,
                 DM1_PC34_ALLOWED_ANY_SLOT,
                 "leader hand allowed slots after final click", spec->defsAnchor);

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];
        int expectedType = i == spec->pickedChestSlotIndex
                               ? 0
                               : expected_item_type_for_initial_index(i);
        int expectedWeight = i == spec->pickedChestSlotIndex
                                 ? 0
                                 : expected_item_weight_for_initial_index(i);

        snprintf(label, sizeof(label), "positive pre C%d type",
                 DM1_PC34_SLOT_CHEST_1 + i);
        check_int_eq(result->c30SlotTypesPre[i], expectedType, label,
                     spec->slotDispatchAnchor);
        snprintf(label, sizeof(label), "positive pre C%d weight",
                 DM1_PC34_SLOT_CHEST_1 + i);
        check_int_eq(result->c30SlotWeightsPre[i], expectedWeight, label,
                     spec->slotDispatchAnchor);
        snprintf(label, sizeof(label), "positive post C%d type",
                 DM1_PC34_SLOT_CHEST_1 + i);
        check_int_eq(result->c30SlotTypesPost[i], expectedType, label,
                     spec->slotRenderAnchor);
        snprintf(label, sizeof(label), "positive post C%d weight",
                 DM1_PC34_SLOT_CHEST_1 + i);
        check_int_eq(result->c30SlotWeightsPost[i], expectedWeight, label,
                     spec->slotRenderAnchor);
    }

    check_expected_link_order(result, spec->chestOpenAnchor);
    check_int_eq(result->chestLinkCount, result->expectedChestLinkCount,
                 "positive link count", spec->chestOpenAnchor);
    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];

        snprintf(label, sizeof(label), "positive chest link order %d", i);
        check_int_eq(result->chestLinkOrder[i],
                     result->expectedChestLinkOrder[i],
                     label, spec->chestOpenAnchor);
    }

    check_int_eq(result->partyDirectionBefore, spec->directionBefore,
                 "party direction before rotation", spec->leaderRotationAnchor);
    check_int_eq(result->partyDirectionAfter, spec->directionAfter,
                 "party direction after rotation", spec->leaderRotationAnchor);
    for (i = 0; i < spec->partyCount; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "champion %d cell before", i);
        check_int_eq(result->championCellsBefore[i], i, label,
                     spec->leaderRotationAnchor);
        snprintf(label, sizeof(label), "champion %d cell after", i);
        check_int_eq(result->championCellsAfter[i], (i + 1) % 4, label,
                     spec->leaderRotationAnchor);
        snprintf(label, sizeof(label), "champion %d direction before", i);
        check_int_eq(result->championDirectionsBefore[i], 0, label,
                     spec->leaderRotationAnchor);
        snprintf(label, sizeof(label), "champion %d direction after", i);
        check_int_eq(result->championDirectionsAfter[i], 1, label,
                     spec->leaderRotationAnchor);
    }

    check_int_eq(result->f0333OpenCount, 1, "F0333 open count",
                 spec->chestOpenAnchor);
    check_int_eq(result->f0334CloseCount, 0, "F0334 close count",
                 spec->chestCloseAnchor);
    check_int_eq(result->f0297PutLeaderHandCount, 1, "F0297 put count",
                 spec->leaderHandPutAnchor);
    check_int_eq(result->f0298RemoveLeaderHandCount, 0, "F0298 remove count",
                 spec->leaderHandRemoveAnchor);
    check_int_eq(result->f0302SlotDispatchCount, 1, "F0302 slot dispatch count",
                 spec->slotDispatchAnchor);
    check_int_eq(result->f0284RotationCount, 1, "F0284 rotation count",
                 spec->leaderRotationAnchor);
    check_int_eq(result->f0291SlotRenderCount, 1, "F0291 slot render count",
                 spec->slotRenderAnchor);
    check_int_eq(result->f0293ChampionStateRedrawCount, 1,
                 "F0293 redraw count", spec->championStateRedrawAnchor);
    check_int_eq(result->f0359C040DispatchGuardCount, 1,
                 "F0359 C040 guard count", spec->candidateDispatchAnchor);
    check_int_eq(result->f0280PanelOpenCount, 1, "F0280 panel open count",
                 spec->candidatePanelOpenAnchor);
    check_int_eq(result->f0282PanelClearCount, 0, "F0282 panel clear count",
                 spec->candidatePanelClearAnchor);
    check_int_eq(result->closedChestCount, 0,
                 "positive path does not close chest", spec->chestCloseAnchor);
    check_int_eq(result->openChestThingAfterFinal, spec->openChestThing,
                 "positive path keeps G0426 open", spec->chestOpenAnchor);
}

static void check_negative_close_result(
    const DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Result
        *result)
{
    const DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Spec
        *spec = result->spec;
    int i;

    check_int_eq(result->path,
                 DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PATH_CLOSE_CHEST_PC34_COMPAT,
                 "negative path id", spec->chestCloseAnchor);
    check_true(result->accepted, "negative close path accepted",
               spec->contractScope);
    check_true(result->chestOpenDispatched, "negative chest open dispatched",
               spec->chestOpenAnchor);
    check_true(result->scrollPickupDispatched, "negative scroll pickup dispatched",
               spec->slotDispatchAnchor);
    check_true(result->resurrectPanelOpened, "negative panel opened",
               spec->candidatePanelOpenAnchor);
    check_true(result->leaderRotationDispatched, "negative rotation dispatched",
               spec->leaderRotationAnchor);
    check_true(!result->inventoryPortraitClickDispatched,
               "negative path skips portrait click", spec->slotDispatchAnchor);
    check_true(result->chestCloseDispatched,
               "negative path closes chest at step 4", spec->chestCloseAnchor);
    check_true(result->candidatePanelVisible,
               "negative close keeps candidate panel visible",
               spec->candidateDispatchAnchor);
    check_true(result->candidateStatePreserved,
               "negative close keeps candidate state", spec->candidateDispatchAnchor);
    check_true(result->candidateNoReinit,
               "negative close does not reinit panel", spec->candidatePanelOpenAnchor);
    check_true(result->leaderHandStillC040,
               "negative close leaves leader hand C040", spec->leaderHandPutAnchor);
    check_true(result->leaderHandStatePreservedThroughRotation,
               "negative close preserves hand through rotation",
               spec->leaderRotationAnchor);
    check_true(result->chestLinkOrderPreserved,
               "negative close preserves collapsed chest link order",
               spec->chestCloseAnchor);
    check_true(!result->c30SlotTypesWeightsByteEqualPrePost,
               "negative close clears C30+ window instead of preserving it",
               spec->chestCloseAnchor);

    check_uint_eq(result->candidateOrdinalBefore, spec->candidateChampionOrdinal,
                  "negative candidate before", spec->candidatePanelOpenAnchor);
    check_uint_eq(result->candidateOrdinalAfter, spec->candidateChampionOrdinal,
                  "negative candidate after", spec->candidateDispatchAnchor);
    check_int_eq(result->candidatePanelOpenCountBeforeFinal, 1,
                 "negative panel open count before", spec->candidatePanelOpenAnchor);
    check_int_eq(result->candidatePanelOpenCountAfterFinal, 1,
                 "negative panel open count after", spec->candidateDispatchAnchor);
    check_int_eq(result->leaderHandTypeAfterFinal, spec->c040PanelGraphic,
                 "negative leader hand final C040", spec->leaderHandPutAnchor);
    check_int_eq(result->leaderHandWeightAfterFinal, 9,
                 "negative leader hand final weight", spec->leaderHandPutAnchor);
    check_int_eq(result->leaderHandChargesAfterFinal, 1,
                 "negative leader hand final charges", spec->leaderHandPutAnchor);

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];
        int expectedType = i == spec->pickedChestSlotIndex
                               ? 0
                               : expected_item_type_for_initial_index(i);
        int expectedWeight = i == spec->pickedChestSlotIndex
                                 ? 0
                                 : expected_item_weight_for_initial_index(i);

        snprintf(label, sizeof(label), "negative pre C%d type",
                 DM1_PC34_SLOT_CHEST_1 + i);
        check_int_eq(result->c30SlotTypesPre[i], expectedType, label,
                     spec->slotDispatchAnchor);
        snprintf(label, sizeof(label), "negative pre C%d weight",
                 DM1_PC34_SLOT_CHEST_1 + i);
        check_int_eq(result->c30SlotWeightsPre[i], expectedWeight, label,
                     spec->slotDispatchAnchor);
        snprintf(label, sizeof(label), "negative post C%d type clear",
                 DM1_PC34_SLOT_CHEST_1 + i);
        check_int_eq(result->c30SlotTypesPost[i], 0, label,
                     spec->chestCloseAnchor);
        snprintf(label, sizeof(label), "negative post C%d weight clear",
                 DM1_PC34_SLOT_CHEST_1 + i);
        check_int_eq(result->c30SlotWeightsPost[i], 0, label,
                     spec->chestCloseAnchor);
    }

    check_expected_link_order(result, spec->chestCloseAnchor);
    check_int_eq(result->chestLinkCount, result->expectedChestLinkCount,
                 "negative closed link count", spec->chestCloseAnchor);
    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];

        snprintf(label, sizeof(label), "negative chest link order %d", i);
        check_int_eq(result->chestLinkOrder[i],
                     result->expectedChestLinkOrder[i],
                     label, spec->chestCloseAnchor);
    }

    check_int_eq(result->f0333OpenCount, 1, "negative F0333 open count",
                 spec->chestOpenAnchor);
    check_int_eq(result->f0334CloseCount, 1, "negative F0334 close count",
                 spec->chestCloseAnchor);
    check_int_eq(result->f0297PutLeaderHandCount, 1, "negative F0297 put count",
                 spec->leaderHandPutAnchor);
    check_int_eq(result->f0298RemoveLeaderHandCount, 0,
                 "negative F0298 remove count", spec->leaderHandRemoveAnchor);
    check_int_eq(result->f0302SlotDispatchCount, 1,
                 "negative F0302 dispatch count", spec->slotDispatchAnchor);
    check_int_eq(result->f0284RotationCount, 1, "negative F0284 count",
                 spec->leaderRotationAnchor);
    check_int_eq(result->f0359C040DispatchGuardCount, 0,
                 "negative path has no portrait guard click",
                 spec->candidateDispatchAnchor);
    check_int_eq(result->f0280PanelOpenCount, 1,
                 "negative F0280 open count", spec->candidatePanelOpenAnchor);
    check_int_eq(result->f0282PanelClearCount, 0,
                 "negative F0282 clear count", spec->candidatePanelClearAnchor);
    check_int_eq(result->closedChestCount, result->expectedChestLinkCount,
                 "negative close returns compacted count", spec->chestCloseAnchor);
    check_int_eq(result->openChestThingAfterFinal, 0,
                 "negative close clears G0426", spec->chestCloseAnchor);
}

int main(void)
{
    DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Result
        positive;
    DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Result
        negative;
    int positiveAccepted;
    int negativeAccepted;

    test_source_evidence();
    test_spec_metadata();

    positiveAccepted =
        dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_simulate_pc34(
            DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PATH_PORTRAIT_CLICK_PC34_COMPAT,
            &positive);
    check_int_eq(positiveAccepted, 1, "positive simulator return",
                 "COMMAND.C F0359:1985-1990");
    check_positive_result(&positive);

    negativeAccepted =
        dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_simulate_pc34(
            DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PATH_CLOSE_CHEST_PC34_COMPAT,
            &negative);
    check_int_eq(negativeAccepted, 1, "negative simulator return",
                 "CHEST.C F0334:113-132");
    check_negative_close_result(&negative);

    printf("dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_pc34_compat: %d assertions, %d failures\n",
           gAssertions, gFailures);
    if (gAssertions < 80) {
        printf("FAIL: expected at least 80 assertions\n");
        return 1;
    }
    return gFailures == 0 ? 0 : 1;
}
