#include "dm1_v1_mirror_candidate_scroll_pickup_non_leader_panel_live_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* ReDMCSB source-lock evidence for this contract test:
 * CHEST.C F0333:30-67 and F0334:117-132 pin G0426/G0425;
 * CHAMPION.C F0297:243-268, F0298:270-298, F0300:511-584, F0301:606-660,
 * and F0302:662-713 pin the hand/C30+ exchange; COMMAND.C F0378:1973-1983
 * and F0380:2045-2159 pin scroll-pickup queue identity; REVIVE.C
 * F0280:124-132 and F0282:744-806 pin C040/G0299 open/cancel; PANEL.C
 * F0344/F0345 and F0346/F0347:1619-1657 pin panel click/redraw;
 * UTAMSCR.C F0077/F0078:141-150, OBJECT.C F0033:147-212, BLITMASK.C
 * F0133:30-33, and DEFS.H:338-340, 810-817, 1874-1878, 2085-2088,
 * 2088-2096, 2200, 3001-3008, 5694, 5876-5881 pin redraw and constants.
 */

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
               message,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
}

static void check_uint_eq(unsigned int actual, unsigned int expected,
                          const char *message, const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%u expected=%u [%s]\n",
               message,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++gAssertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++gFailures;
        printf("FAIL: %s missing=%s [%s]\n",
               message,
               needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static int expected_slot_before(int index)
{
    return index == 1 ? 0x7038 : 0x7200 + index;
}

static int expected_slot_after(int index)
{
    return index == 1 ? 0 : expected_slot_before(index);
}

static void test_source_evidence(void)
{
    const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveEvidencePc34Compat
        *e =
            DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_EvidencePc34Compat();
    const char *text =
        DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_SourceEvidencePc34Compat();

    check_true(e != NULL && e->contractOnly == 1,
               "evidence accessor returns contract-only metadata",
               "COMMAND.C F0380:2045-2159");
    check_contains(e->chestOpenAnchor, "CHEST.C F0333:30-67",
                   "evidence cites chest open", e->chestOpenAnchor);
    check_contains(e->chestCloseAnchor, "CHEST.C F0334:117-132",
                   "evidence cites chest close", e->chestCloseAnchor);
    check_contains(e->leaderHandPutAnchor, "CHAMPION.C F0297:243-268",
                   "evidence cites leader-hand put", e->leaderHandPutAnchor);
    check_contains(e->leaderHandRemoveAnchor, "CHAMPION.C F0298:270-298",
                   "evidence cites leader-hand remove",
                   e->leaderHandRemoveAnchor);
    check_contains(e->slotRemoveAnchor, "CHAMPION.C F0300:511-584",
                   "evidence cites slot remove", e->slotRemoveAnchor);
    check_contains(e->slotAddAnchor, "CHAMPION.C F0301:606-660",
                   "evidence cites slot add", e->slotAddAnchor);
    check_contains(e->slotDispatchAnchor, "CHAMPION.C F0302:662-713",
                   "evidence cites slot dispatch", e->slotDispatchAnchor);
    check_contains(e->scrollDispatchAnchor, "COMMAND.C F0378:1973-1983",
                   "evidence cites scroll dispatch", e->scrollDispatchAnchor);
    check_contains(e->queueAnchor, "COMMAND.C F0380:2045-2159",
                   "evidence cites queue identity", e->queueAnchor);
    check_contains(e->candidateOpenAnchor, "REVIVE.C F0280:124-132",
                   "evidence cites candidate open", e->candidateOpenAnchor);
    check_contains(e->candidateCancelAnchor, "REVIVE.C F0282:744-806",
                   "evidence cites candidate cancel",
                   e->candidateCancelAnchor);
    check_contains(e->panelClickAnchor, "PANEL.C F0344/F0345",
                   "evidence cites panel click/release", e->panelClickAnchor);
    check_contains(e->panelRedrawAnchor, "PANEL.C F0346/F0347:1619-1657",
                   "evidence cites C040 redraw", e->panelRedrawAnchor);
    check_contains(e->mouseAnchor, "UTAMSCR.C F0077/F0078:141-150",
                   "evidence cites mouse bracket", e->mouseAnchor);
    check_contains(e->objectAnchor, "OBJECT.C F0033:147-212",
                   "evidence cites object chain", e->objectAnchor);
    check_contains(e->blitMaskAnchor, "BLITMASK.C F0133:30-33",
                   "evidence cites blit mask", e->blitMaskAnchor);
    check_contains(e->defsAnchor, "DEFS.H:338-340",
                   "defs cites C162", e->defsAnchor);
    check_contains(e->defsAnchor, "810-817 C30..C37",
                   "defs cites C30..C37", e->defsAnchor);
    check_contains(e->defsAnchor, "1874-1878 C38",
                   "defs cites C38", e->defsAnchor);
    check_contains(e->defsAnchor, "2085-2088 G0305",
                   "defs cites G0305 party", e->defsAnchor);
    check_contains(e->defsAnchor, "2088-2096 G0423",
                   "defs cites G0423", e->defsAnchor);
    check_contains(e->defsAnchor, "2200 C040",
                   "defs cites C040", e->defsAnchor);
    check_contains(e->defsAnchor, "3001-3008 M568/M569",
                   "defs cites M568/M569", e->defsAnchor);
    check_contains(e->defsAnchor, "5694 G0299",
                   "defs cites G0299", e->defsAnchor);
    check_contains(e->defsAnchor, "5876-5881 G0425/G0426",
                   "defs cites G0425/G0426", e->defsAnchor);
    check_contains(e->contractScope, "C040/G0299 stays live",
                   "contract names live panel guard", e->contractScope);
    check_contains(e->contractScope, "pass686",
                   "contract distinguishes pass686", e->contractScope);

    check_contains(text, "CHEST.C F0333:30-67",
                   "source string cites F0333", e->chestOpenAnchor);
    check_contains(text, "CHEST.C F0334:117-132",
                   "source string cites F0334", e->chestCloseAnchor);
    check_contains(text, "CHAMPION.C F0297:243-268",
                   "source string cites F0297", e->leaderHandPutAnchor);
    check_contains(text, "CHAMPION.C F0298:270-298",
                   "source string cites F0298", e->leaderHandRemoveAnchor);
    check_contains(text, "CHAMPION.C F0300:511-584",
                   "source string cites F0300", e->slotRemoveAnchor);
    check_contains(text, "CHAMPION.C F0301:606-660",
                   "source string cites F0301", e->slotAddAnchor);
    check_contains(text, "CHAMPION.C F0302:662-713",
                   "source string cites F0302", e->slotDispatchAnchor);
    check_contains(text, "COMMAND.C F0378:1973-1983",
                   "source string cites F0378", e->scrollDispatchAnchor);
    check_contains(text, "COMMAND.C F0380:2045-2159",
                   "source string cites F0380", e->queueAnchor);
    check_contains(text, "REVIVE.C F0280:124-132",
                   "source string cites F0280", e->candidateOpenAnchor);
    check_contains(text, "REVIVE.C F0282:744-806",
                   "source string cites F0282", e->candidateCancelAnchor);
    check_contains(text, "PANEL.C F0344/F0345",
                   "source string cites F0344/F0345", e->panelClickAnchor);
    check_contains(text, "PANEL.C F0346/F0347:1619-1657",
                   "source string cites F0346/F0347", e->panelRedrawAnchor);
    check_contains(text, "UTAMSCR.C F0077/F0078:141-150",
                   "source string cites mouse bracket", e->mouseAnchor);
    check_contains(text, "OBJECT.C F0033:147-212",
                   "source string cites object chain", e->objectAnchor);
    check_contains(text, "BLITMASK.C F0133:30-33",
                   "source string cites blit mask", e->blitMaskAnchor);
    check_contains(text, "DEFS.H:338-340 C162",
                   "source string cites C162", e->defsAnchor);
    check_contains(text, "810-817 C30..C37",
                   "source string cites C30..C37", e->defsAnchor);
    check_contains(text, "1874-1878 C38",
                   "source string cites C38", e->defsAnchor);
    check_contains(text, "2085-2088 G0305 party",
                   "source string cites G0305", e->defsAnchor);
    check_contains(text, "2088-2096 G0423 chest",
                   "source string cites G0423", e->defsAnchor);
    check_contains(text, "2200 C040",
                   "source string cites C040", e->defsAnchor);
    check_contains(text, "3001-3008 M568/M569",
                   "source string cites M568/M569", e->defsAnchor);
    check_contains(text, "5694 G0299",
                   "source string cites G0299", e->defsAnchor);
    check_contains(text, "5876-5881 G0425/G0426",
                   "source string cites G0425/G0426", e->defsAnchor);
}

static void test_spec_metadata(void)
{
    const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveSpecPc34Compat
        *spec =
            DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_SpecPc34Compat();

    check_true(spec != NULL, "spec accessor returns metadata",
               "DEFS.H:338-340,810-817,1874-1878");
    check_int_eq(spec->partyCount, 4, "spec party count",
                 "DEFS.H:2085-2088 G0305");
    check_int_eq(spec->leaderIndex, 0, "spec leader index",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(spec->partyTailChampion, 1, "spec party tail champion",
                 "DEFS.H:2085-2088 G0305");
    check_uint_eq(spec->candidateOrdinal, 3u, "spec candidate ordinal",
                  "REVIVE.C F0280:124-132");
    check_int_eq(spec->leaderOpenChestThing, 0x6a40,
                 "spec leader open chest thing", "CHEST.C F0333:30-67");
    check_int_eq(spec->partyTailChestThing, 0x6b41,
                 "spec party tail chest differs", "CHEST.C F0333:30-67");
    check_true(spec->leaderOpenChestThing != spec->partyTailChestThing,
               "spec chest identities differ", "CHEST.C F0333:30-67");
    check_int_eq(spec->scrollThing, 0x7038, "spec C038 scroll thing",
                 "OBJECT.C F0033:147-212");
    check_int_eq(spec->nonLeaderSlotIndex, 1,
                 "spec non-leader slot index", "DEFS.H:810-817");
    check_int_eq(spec->nonLeaderSlotId, 31,
                 "spec non-leader slot id C31", "DEFS.H:810-817");
    check_int_eq(spec->nonLeaderSlotBox, 39,
                 "spec C38+ slot box", "DEFS.H:1874-1878 C38");
    check_int_eq(spec->nonLeaderDisplayZone, 538,
                 "spec C538 display zone", "DEFS.H:1874-1878 C38");
    check_int_eq(spec->c040PanelGraphic, 40, "spec C040 panel",
                 "DEFS.H:2200 C040");
    check_int_eq(spec->resurrectPanelId, 5, "spec M568 panel id",
                 "DEFS.H:3001-3008 M568/M569");
    check_int_eq(spec->chestPanelId, 4, "spec M569 panel id",
                 "DEFS.H:3001-3008 M568/M569");
    check_int_eq(spec->cancelCommand, 162, "spec C162 cancel",
                 "DEFS.H:338-340 C162");
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat state;
    int i;

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);

    check_int_eq(state.contractOnly, 1, "initial contract-only flag",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.partyCount, 4, "initial four champion party",
                 "DEFS.H:2085-2088 G0305");
    check_int_eq(state.leaderIndex, 0, "initial leader at slot 0",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.partyTailChampion, 1,
                 "initial party tail champion at slot 1",
                 "DEFS.H:2085-2088 G0305");
    check_uint_eq(state.candidateOrdinal, 3u,
                  "initial candidate ordinal metadata",
                  "REVIVE.C F0280:124-132");
    check_uint_eq(state.g0299CandidateOrdinal, 3u,
                  "initial G0299 candidate set",
                  "DEFS.H:5694 G0299");
    check_int_eq(state.panelOpen, 1, "initial C040 panel live",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.panelGraphic, 40, "initial C040 graphic",
                 "DEFS.H:2200 C040");
    check_int_eq(state.panelRedrawable, 1,
                 "initial C040 panel redrawable",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.leaderHandThing, 0, "initial leader hand empty",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.leaderOpenChestThing, 0x6a40,
                 "initial leader open chest", "CHEST.C F0333:30-67");
    check_int_eq(state.partyTailChestThing, 0x6b41,
                 "initial party tail different chest", "CHEST.C F0333:30-67");
    check_int_eq(state.openChestThing, state.leaderOpenChestThing,
                 "initial G0426 points to leader chest",
                 "DEFS.H:5876-5881 G0425/G0426");
    check_int_eq(state.inventoryChampionOrdinal, 2,
                 "initial G0423 inventory champion is non-leader",
                 "DEFS.H:2088-2096 G0423");
    check_int_eq(state.activeSlotBox, 39,
                 "initial active C538 slot box",
                 "DEFS.H:1874-1878 C38");
    check_int_eq(state.nonLeaderSlotThing, 0x7038,
                 "initial non-leader C538 slot has scroll",
                 "OBJECT.C F0033:147-212");
    check_int_eq(state.f0333OpenCount, 1, "initial F0333 open count",
                 "CHEST.C F0333:30-67");
    check_int_eq(state.f0280OpenCount, 1, "initial F0280 open count",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.panelRedrawCount, 1,
                 "initial panel redraw count",
                 "PANEL.C F0346/F0347:1619-1657");
    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[80];

        snprintf(label, sizeof(label), "initial C30+ chest slot %d", i);
        check_int_eq(state.chestSlots[i], expected_slot_before(i), label,
                     "CHEST.C F0333:30-67");
    }
}

static void check_run_result(
    const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveResultPc34Compat
        *result)
{
    const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveEvidencePc34Compat
        *e = result->evidence;
    int i;

    check_true(result->accepted, "full sequence accepted", e->contractScope);
    check_int_eq(result->f0333OpenCount, 1,
                 "F0333 open count remains initial live chest",
                 e->chestOpenAnchor);
    check_int_eq(result->f0334CloseCount, 0,
                 "F0334 close not run during guarded pickup/cancel",
                 e->chestCloseAnchor);
    check_int_eq(result->f0297PutCount, 1,
                 "F0297 leader-hand put count", e->leaderHandPutAnchor);
    check_int_eq(result->f0298RemoveCount, 0,
                 "F0298 remove not used without leader-hand swap",
                 e->leaderHandRemoveAnchor);
    check_int_eq(result->f0300SlotRemoveCount, 1,
                 "F0300 slot remove count", e->slotRemoveAnchor);
    check_int_eq(result->f0301SlotAddCount, 1,
                 "F0301 slot replacement/write-back count",
                 e->slotAddAnchor);
    check_int_eq(result->f0302DispatchCount, 1,
                 "F0302 slot dispatch count", e->slotDispatchAnchor);
    check_int_eq(result->f0378DispatchCount, 1,
                 "F0378 scroll dispatch count", e->scrollDispatchAnchor);
    check_int_eq(result->f0380QueueCount, 1,
                 "F0380 queue count", e->queueAnchor);
    check_int_eq(result->f0280OpenCount, 1,
                 "F0280 candidate open count preserved",
                 e->candidateOpenAnchor);
    check_int_eq(result->f0282CancelCount, 1,
                 "F0282 follow-up cancel count",
                 e->candidateCancelAnchor);
    check_int_eq(result->f0344PanelClickCount, 1,
                 "F0344/F0345 panel click count", e->panelClickAnchor);
    check_int_eq(result->panelRedrawCount, 3,
                 "panel redraw covers initial, pickup, cancel",
                 e->panelRedrawAnchor);
    check_int_eq(result->mouseEnableCount, 2,
                 "mouse enable covers pickup and cancel", e->mouseAnchor);
    check_int_eq(result->mouseDisableCount, 2,
                 "mouse disable covers pickup and cancel", e->mouseAnchor);
    check_int_eq(result->leaderHandThingAfter, 0x7038,
                 "leader hand now holds C038 scroll", e->leaderHandPutAnchor);
    check_int_eq(result->nonLeaderSlotCleared, 1,
                 "non-leader C538 slot cleared", e->slotRemoveAnchor);
    check_int_eq(result->nonLeaderSlotReplaced, 1,
                 "non-leader C538 slot replaced with empty sentinel",
                 e->slotAddAnchor);
    check_uint_eq(result->candidateOrdinal, 3u,
                  "result candidate ordinal exposes post-pickup G0299",
                  e->candidateOpenAnchor);
    check_int_eq(result->panelOpen, 1,
                 "result panelOpen exposes post-pickup C040 live",
                 e->panelRedrawAnchor);
    check_int_eq(result->openChestThing, 0x6a40,
                 "result openChestThing exposes leader open chest",
                 e->chestOpenAnchor);
    check_int_eq(result->partyTailChampion, 1,
                 "result party tail champion preserved", e->defsAnchor);
    check_int_eq(result->followUpCancelClearsCandidate, 1,
                 "follow-up C162 cancel clears candidate",
                 e->candidateCancelAnchor);
    check_int_eq(result->mutationGuardsOk, 1,
                 "mutation guards held during pickup", e->contractScope);

    check_int_eq(result->candidateOrdinalBefore, 3,
                 "candidate ordinal before pickup", e->candidateOpenAnchor);
    check_int_eq(result->candidateOrdinalAfterPickup, 3,
                 "candidate ordinal preserved after pickup",
                 e->queueAnchor);
    check_int_eq(result->candidateOrdinalAfterCancel, 0,
                 "candidate ordinal cleared after cancel",
                 e->candidateCancelAnchor);
    check_int_eq(result->panelOpenBefore, 1,
                 "panel open before pickup", e->panelRedrawAnchor);
    check_int_eq(result->panelOpenAfterPickup, 1,
                 "panel still open after pickup", e->panelRedrawAnchor);
    check_int_eq(result->panelOpenAfterCancel, 0,
                 "panel closed after cancel", e->candidateCancelAnchor);
    check_int_eq(result->panelRedrawableAfterPickup, 1,
                 "panel redrawable after pickup", e->panelRedrawAnchor);
    check_int_eq(result->openChestBefore, 0x6a40,
                 "open chest before pickup", e->chestOpenAnchor);
    check_int_eq(result->openChestAfterPickup, 0x6a40,
                 "open chest preserved after pickup", e->chestOpenAnchor);
    check_int_eq(result->openChestAfterCancel, 0x6a40,
                 "open chest not mutated by C162 candidate cancel",
                 e->chestCloseAnchor);
    check_int_eq(result->partyTailChampionBefore, 1,
                 "party tail before pickup", e->defsAnchor);
    check_int_eq(result->partyTailChampionAfterPickup, 1,
                 "party tail after pickup", e->defsAnchor);
    check_int_eq(result->leaderOpenChestBefore, 0x6a40,
                 "leader open chest before pickup", e->chestOpenAnchor);
    check_int_eq(result->leaderOpenChestAfterPickup, 0x6a40,
                 "leader open chest after pickup", e->chestOpenAnchor);
    check_int_eq(result->partyTailChestBefore, 0x6b41,
                 "tail chest before pickup", e->chestOpenAnchor);
    check_int_eq(result->partyTailChestAfterPickup, 0x6b41,
                 "tail chest after pickup", e->chestOpenAnchor);
    check_true(result->leaderOpenChestAfterPickup !=
                   result->partyTailChestAfterPickup,
               "leader chest did not mutate into tail chest",
               e->contractScope);
    check_int_eq(result->leaderHandBefore, 0,
                 "leader hand empty before pickup", e->leaderHandPutAnchor);
    check_int_eq(result->nonLeaderSlotBefore, 0x7038,
                 "non-leader slot before pickup has scroll",
                 e->objectAnchor);
    check_int_eq(result->nonLeaderSlotAfterPickup, 0,
                 "non-leader slot after pickup empty",
                 e->slotRemoveAnchor);
    check_int_eq(result->activeSlotBoxBefore, 39,
                 "active slot box before pickup C538",
                 e->defsAnchor);
    check_int_eq(result->activeSlotBoxAfterPickup, 39,
                 "active slot box after pickup preserved",
                 e->defsAnchor);
    check_int_eq(result->inventoryChampionOrdinalBefore, 2,
                 "inventory champion before pickup is non-leader",
                 e->defsAnchor);
    check_int_eq(result->inventoryChampionOrdinalAfterPickup, 2,
                 "inventory champion after pickup is non-leader",
                 e->defsAnchor);

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];

        snprintf(label, sizeof(label), "slot %d before pickup", i);
        check_int_eq(result->chestSlotsBefore[i],
                     expected_slot_before(i),
                     label,
                     e->chestOpenAnchor);
        snprintf(label, sizeof(label), "slot %d after pickup", i);
        check_int_eq(result->chestSlotsAfterPickup[i],
                     expected_slot_after(i),
                     label,
                     e->slotDispatchAnchor);
        if (i != 1) {
            snprintf(label, sizeof(label),
                     "slot %d unrelated preserved after pickup", i);
            check_int_eq(result->chestSlotsAfterPickup[i],
                         result->chestSlotsBefore[i],
                         label,
                         e->objectAnchor);
        }
    }
}

static void test_run_sequence(void)
{
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat state;
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveResultPc34Compat result;
    int ok;

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    ok = DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_RunPc34Compat(
        &state, &result);

    check_int_eq(ok, 1, "run returns accepted",
                 "COMMAND.C F0380:2045-2159");
    check_run_result(&result);
    check_int_eq(state.leaderHandThing, 0x7038,
                 "state leader hand holds scroll after run",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.nonLeaderSlotThing, 0,
                 "state non-leader slot cleared after run",
                 "CHAMPION.C F0300:511-584");
    check_uint_eq(state.g0299CandidateOrdinal, 0u,
                  "state G0299 cleared by follow-up cancel",
                  "REVIVE.C F0282:744-806");
    check_int_eq(state.panelOpen, 0,
                 "state panel closed by follow-up cancel",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.openChestThing, 0x6a40,
                 "state open chest remains leader chest",
                 "CHEST.C F0333:30-67");
    check_int_eq(state.partyTailChampion, 1,
                 "state party tail champion preserved",
                 "DEFS.H:2085-2088 G0305");
    check_int_eq(state.f0334CloseCount, 0,
                 "state close count remains zero", "CHEST.C F0334:117-132");
    check_int_eq(state.followUpCancelRequested, 162,
                 "state records C162 cancel command",
                 "DEFS.H:338-340 C162");
    check_int_eq(state.candidateClearedByCancel, 1,
                 "state records candidate cleanly cleared",
                 "REVIVE.C F0282:744-806");
}

static void check_rejects_mutation(
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat state,
    const char *message)
{
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveResultPc34Compat result;

    check_int_eq(
        DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_RunPc34Compat(
            &state, &result),
        0,
        message,
        "COMMAND.C F0380:2045-2159");
}

static void test_rejects_non_contract_state(void)
{
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat state;
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveResultPc34Compat result;

    check_int_eq(
        DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_RunPc34Compat(
            NULL, &result),
        0,
        "run rejects null state",
        "COMMAND.C F0380:2045-2159");
    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    check_int_eq(
        DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_RunPc34Compat(
            &state, NULL),
        0,
        "run rejects null result",
        "COMMAND.C F0380:2045-2159");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.contractOnly = 0;
    check_rejects_mutation(state, "rejects non-contract state");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.panelOpen = 0;
    check_rejects_mutation(state, "rejects closed C040 panel");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.g0299CandidateOrdinal = 0;
    check_rejects_mutation(state, "rejects missing G0299 candidate");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.g0299CandidateOrdinal = 4;
    check_rejects_mutation(state, "rejects wrong G0299 candidate");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.partyCount = 3;
    check_rejects_mutation(state, "rejects non-four-champion party");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.leaderIndex = 1;
    check_rejects_mutation(state, "rejects non-zero leader index");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.partyTailChampion = 2;
    check_rejects_mutation(state, "rejects changed party tail champion");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.openChestThing = state.partyTailChestThing;
    check_rejects_mutation(state, "rejects G0426 already mutated to tail chest");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.leaderOpenChestThing = state.partyTailChestThing;
    check_rejects_mutation(state, "rejects equal leader/tail chest identity");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.leaderHandThing = 0x7777;
    check_rejects_mutation(state, "rejects occupied leader hand swap state");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.nonLeaderSlotThing = 0;
    check_rejects_mutation(state, "rejects empty non-leader C538 slot");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.chestSlots[1] = 0;
    check_rejects_mutation(state, "rejects missing C038 in G0425 C538 slot");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.inventoryChampionOrdinal = 1;
    check_rejects_mutation(state, "rejects leader inventory champion");

    DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
        &state);
    state.activeSlotBox = 40;
    check_rejects_mutation(state, "rejects wrong active C38 slot box");
}

int main(void)
{
    printf("=== DM1 V1 mirror-candidate non-leader C538 scroll pickup C040 live gate ===\n");
    test_source_evidence();
    test_spec_metadata();
    test_initial_state();
    test_run_sequence();
    test_rejects_non_contract_state();
    if (gFailures) {
        printf("FAIL: %d assertion(s) failed out of %d\n",
               gFailures,
               gAssertions);
        return 1;
    }
    printf("PASS: assertions=%d failures=0\n", gAssertions);
    return 0;
}
