/* DM1 V1 mirror-candidate reselect-after-deposit with party-rotate gate.
 *
 * Source-lock anchors:
 * - CHEST.C F0333:30-67 opens/draws a panel/chest and seeds G0425.
 * - CHEST.C F0334:117-132 clears G0426 and rewrites/reset G0425 slots.
 * - CHAMPION.C F0297:243-268 puts an object in leader hand.
 * - CHAMPION.C F0298:270-298 removes the leader-hand object.
 * - CHAMPION.C F0300:511-584 removes a champion/chest slot object.
 * - CHAMPION.C F0301:606-660 adds a champion/chest slot object.
 * - CHAMPION.C F0302:662-713 dispatches slot-box transfers.
 * - COMMAND.C F0378:1973-1983 handles mirror/candidate panel input.
 * - COMMAND.C F0380:2045-2156 dispatches queued turns and slot commands.
 * - REVIVE.C F0280:124-132 publishes only when hand is empty and party has room.
 * - REVIVE.C F0282:744-806 clears G0299/C040 after candidate decisions.
 * - PANEL.C F0346/F0347:1619-1657 redraws C040 when G0299 is set.
 * - UTAMSCR.C F0077:147-151 + F0078:141-145 bracket pointer updates.
 * - OBJECT.C F0033:147-212 resolves icons with party-direction-sensitive compass.
 * - BLITMASK.C F0133:30-33 anchors masked redraw.
 * - DEFS.H:338-340 C162; 810-817 C30..C37; 1874-1878 C38; 2200 C040;
 *   3001-3008 M568/M569; 5694 G0299; 5876-5881 G0425/G0426.
 * - COMMAND.C F0380:2150-2156 routes C001/C002 turns; CLIKMENU.C
 *   F0365:142-174 calls CHAMPION.C F0284:93-131 to rotate party direction.
 */
#include "dm1/dm1_v1_mirror_candidate_reselect_after_deposit_with_party_rotate_pc34_compat.h"

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
        printf("FAIL: %s actual=0x%X expected=0x%X [%s]\n",
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

static int chest_slots_are_none(
    const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state)
{
    int i;

    for (i = 0; i < DM1_V1_MCRADPR_CHEST_SLOT_COUNT_PC34; ++i) {
        if (state->chestSlots[i] != DM1_V1_MCRADPR_THING_NONE_PC34) {
            return 0;
        }
    }
    return 1;
}

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_EvidencePc34Compat();

    check_true(e != NULL, "evidence accessor returns metadata", "metadata");
    check_int_eq(e->contractOnly, 1, "contract-only marker",
                 "contract_only=1");
    check_contains(e->chestOpenAnchor, "CHEST.C F0333:30-67",
                   "F0333 open/seed range is cited", e->chestOpenAnchor);
    check_contains(e->chestOpenAnchor, "G0425",
                   "F0333 cites G0425 seeding", e->chestOpenAnchor);
    check_contains(e->chestCloseAnchor, "CHEST.C F0334:117-132",
                   "F0334 close/reset range is cited", e->chestCloseAnchor);
    check_contains(e->chestCloseAnchor, "G0426",
                   "F0334 cites G0426 reset", e->chestCloseAnchor);
    check_contains(e->championPutAnchor, "CHAMPION.C F0297:243-268",
                   "F0297 put range is cited", e->championPutAnchor);
    check_contains(e->championRemoveAnchor, "CHAMPION.C F0298:270-298",
                   "F0298 remove range is cited", e->championRemoveAnchor);
    check_contains(e->championSlotRemoveAnchor, "F0300:511-584",
                   "F0300 slot remove range is cited",
                   e->championSlotRemoveAnchor);
    check_contains(e->championSlotAddAnchor, "F0301:606-660",
                   "F0301 slot add range is cited",
                   e->championSlotAddAnchor);
    check_contains(e->championSlotClickAnchor, "F0302:662-713",
                   "F0302 slot click range is cited",
                   e->championSlotClickAnchor);
    check_contains(e->commandPanelAnchor, "COMMAND.C F0378:1973-1983",
                   "F0378 panel dispatch is cited", e->commandPanelAnchor);
    check_contains(e->commandQueueAnchor, "COMMAND.C F0380:2045-2156",
                   "F0380 queue dispatch is cited", e->commandQueueAnchor);
    check_contains(e->reviveOpenAnchor, "REVIVE.C F0280:124-132",
                   "F0280 open gate is cited", e->reviveOpenAnchor);
    check_contains(e->reviveOpenAnchor, "empty-hand",
                   "F0280 empty-hand gate is named", e->reviveOpenAnchor);
    check_contains(e->reviveClickAnchor, "REVIVE.C F0282:744-806",
                   "F0282 clear range is cited", e->reviveClickAnchor);
    check_contains(e->panelRedrawAnchor, "PANEL.C F0346/F0347:1619-1657",
                   "C040 redraw range is cited", e->panelRedrawAnchor);
    check_contains(e->mouseAnchor, "F0077:147-151",
                   "mouse enable range is cited", e->mouseAnchor);
    check_contains(e->mouseAnchor, "F0078:141-145",
                   "mouse disable range is cited", e->mouseAnchor);
    check_contains(e->objectAnchor, "OBJECT.C F0033:147-212",
                   "object icon range is cited", e->objectAnchor);
    check_contains(e->objectAnchor, "party-direction",
                   "object anchor mentions direction-sensitive compass",
                   e->objectAnchor);
    check_contains(e->blitmaskAnchor, "BLITMASK.C F0133:30-33",
                   "masked redraw range is cited", e->blitmaskAnchor);
    check_contains(e->defsAnchor, "DEFS.H:338-340 C162",
                   "C162 is cited", e->defsAnchor);
    check_contains(e->defsAnchor, "DEFS.H:810-817 C30..C37",
                   "C30..C37 are cited", e->defsAnchor);
    check_contains(e->defsAnchor, "DEFS.H:1874-1878 C38",
                   "C38 is cited", e->defsAnchor);
    check_contains(e->defsAnchor, "DEFS.H:2200 C040",
                   "C040 is cited", e->defsAnchor);
    check_contains(e->defsAnchor, "DEFS.H:3001-3008 M568/M569",
                   "M568/M569 are cited", e->defsAnchor);
    check_contains(e->defsAnchor, "DEFS.H:5694 G0299",
                   "G0299 is cited", e->defsAnchor);
    check_contains(e->defsAnchor, "DEFS.H:5876-5881 G0425/G0426",
                   "G0425/G0426 are cited", e->defsAnchor);
    check_contains(e->partyRotateAnchor, "COMMAND.C F0380:2150-2156",
                   "queue turn dispatch is cited", e->partyRotateAnchor);
    check_contains(e->partyRotateAnchor, "CLIKMENU.C F0365:142-174",
                   "turn-party handler is cited", e->partyRotateAnchor);
    check_contains(e->partyRotateAnchor, "CHAMPION.C F0284:93-131",
                   "party direction rotation handler is cited",
                   e->partyRotateAnchor);
    check_contains(e->partyRotateAnchor, "party direction rotate",
                   "party rotate scope is named", e->partyRotateAnchor);
    check_contains(e->nonOverlapScope, "deposit-then-party-rotate-then-reopen",
                   "non-overlap names this scenario", e->nonOverlapScope);
    check_contains(e->nonOverlapScope, "pass702",
                   "non-overlap excludes pass702", e->nonOverlapScope);
    check_contains(e->nonOverlapScope, "pass707",
                   "non-overlap excludes pass707", e->nonOverlapScope);
    check_contains(e->nonOverlapScope, "pass736",
                   "non-overlap excludes pass736", e->nonOverlapScope);
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat state;
    int i;

    DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_InitPc34Compat(
        &state);

    check_int_eq(state.contractOnly, 1, "initial contract-only flag",
                 "contract_only=1");
    check_int_eq(state.partyChampionCount, 3, "initial party has room",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.leaderIndex, 0, "initial leader index",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.partyDirection, 0, "initial party direction",
                 "CHAMPION.C F0284:93-131");
    check_int_eq(state.candidateChampionIndex, 3, "candidate row index",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.candidateOrdinal, 4, "candidate ordinal",
                 "DEFS.H:5694");
    check_uint_eq(state.leaderHandThing, DM1_V1_MCRADPR_THING_NONE_PC34,
                  "leader hand starts empty thing",
                  "CHAMPION.C F0298:270-298");
    check_int_eq(state.leaderHandEmpty, 1, "leader empty flag starts true",
                 "REVIVE.C F0280:124-132");
    check_uint_eq(state.pendingDepositThing, DM1_V1_MCRADPR_THING_NONE_PC34,
                  "no pending deposit at init", "CHAMPION.C F0297:243-268");
    check_int_eq(state.depositPending, 0, "pending flag starts clear",
                 "COMMAND.C F0380:2045-2156");
    check_uint_eq(state.candidateSlotThing, 0x4C20u,
                  "candidate slot starts with seed thing",
                  "CHAMPION.C F0300:511-584");
    check_int_eq(state.candidateSlotFilled, 1, "candidate slot marked filled",
                 "CHAMPION.C F0301:606-660");
    check_uint_eq(state.depositedThing, DM1_V1_MCRADPR_THING_NONE_PC34,
                  "no deposited thing at init", "CHAMPION.C F0301:606-660");
    check_int_eq(state.g0299CandidateOrdinal, 0, "G0299 starts clear",
                 "DEFS.H:5694");
    check_int_eq(state.c040PanelOpen, 0, "C040 starts closed",
                 "DEFS.H:2200 C040");
    check_int_eq(state.panelContent, 0, "panel content starts clear",
                 "DEFS.H:3001-3008 M568/M569");
    check_int_eq(state.c040Graphic, 0, "C040 graphic starts clear",
                 "DEFS.H:2200 C040");
    check_int_eq(state.candidateReferenceDirection, -1,
                 "candidate reference is unbound before open",
                 "CHAMPION.C F0284:93-131");
    check_int_eq(state.reopenedReferenceDirection, -1,
                 "reopen reference is unbound before reopen",
                 "CHAMPION.C F0284:93-131");
    check_int_eq(state.openChestThing, 0x6401, "fixture starts with G0426 set",
                 "DEFS.H:5876-5881 G0425/G0426");
    check_int_eq(state.noCrashGuard, 1, "no-crash guard starts armed",
                 "COMMAND.C F0380:2045-2156");
    check_int_eq(state.f0280OpenCount, 0, "open count starts zero",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.f0282ClickCount, 0, "candidate click count starts zero",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.f0334CloseCount, 0, "chest close count starts zero",
                 "CHEST.C F0334:117-132");
    check_int_eq(state.f0297PutCount, 0, "leader put count starts zero",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.f0298RemoveCount, 0, "leader remove count starts zero",
                 "CHAMPION.C F0298:270-298");
    check_int_eq(state.objectTransferCount, 0, "object transfer starts zero",
                 "OBJECT.C F0033:147-212");
    check_int_eq(state.depositFireCount, 0, "deposit fire starts zero",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.doubleFireCount, 0, "double-fire count starts zero",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.staleDepositRejectCount, 0, "stale reject starts zero",
                 "COMMAND.C F0380:2045-2156");
    check_int_eq(state.chestSlots[0], 0x7100, "G0425 first slot seeded",
                 "CHEST.C F0333:30-67");
    check_int_eq(state.chestSlots[7], 0x7107, "G0425 last slot seeded",
                 "CHEST.C F0333:30-67");
    for (i = 0; i < state.partyChampionCount; ++i) {
        check_int_eq(state.championDirection[i], 0,
                     "champion direction starts at party direction",
                     "CHAMPION.C F0284:93-131");
    }
}

static void test_stepwise_sequence(void)
{
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat state;
    int ok;

    DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_InitPc34Compat(
        &state);

    ok = DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_OpenCandidatePc34Compat(
        &state);
    check_int_eq(ok, 1, "initial candidate opens", "REVIVE.C F0280:124-132");
    check_int_eq(state.g0299CandidateOrdinal, 4, "G0299 tracks candidate",
                 "DEFS.H:5694");
    check_int_eq(state.c040PanelOpen, 1, "C040 opens", "DEFS.H:2200 C040");
    check_int_eq(state.panelContent, DM1_V1_MCRADPR_M568_PANEL_PC34,
                 "M568 panel content set", "DEFS.H:3001-3008 M568/M569");
    check_int_eq(state.candidateReferenceDirection, 0,
                 "initial candidate reference uses initial party direction",
                 "CHAMPION.C F0284:93-131");

    ok = DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_PutDepositInLeaderHandPc34Compat(
        &state,
        0x6D40u);
    check_int_eq(ok, 1, "deposit object is put in leader hand",
                 "CHAMPION.C F0297:243-268");
    check_uint_eq(state.leaderHandThing, 0x6D40u, "leader hand holds deposit",
                  "CHAMPION.C F0297:243-268");
    check_int_eq(state.leaderHandEmpty, 0, "leader empty flag clears",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.depositPending, 1, "pending deposit is armed",
                 "COMMAND.C F0380:2045-2156");
    check_uint_eq(state.pendingDepositThing, 0x6D40u,
                  "pending deposit records object",
                  "CHAMPION.C F0297:243-268");
    check_int_eq(state.c040PanelOpen, 1,
                 "mirror candidate remains open with deposit pending",
                 "PANEL.C F0346/F0347:1619-1657");

    ok = DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_DepositViaMirrorPc34Compat(
        &state);
    check_int_eq(ok, 1, "deposit through mirror fires",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.depositFireCount, 1, "deposit fires exactly once",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.objectTransferCount, 1, "object transfer count advances",
                 "OBJECT.C F0033:147-212");
    check_uint_eq(state.depositedThing, 0x6D40u,
                  "deposited thing records leader-hand object",
                  "CHAMPION.C F0301:606-660");
    check_uint_eq(state.candidateSlotThing, 0x6D40u,
                  "candidate slot receives deposit",
                  "CHAMPION.C F0301:606-660");
    check_int_eq(state.c040PanelOpen, 0, "mirror closes after deposit",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.panelContent, 0, "panel content cleared after deposit",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.g0299CandidateOrdinal, 0, "G0299 clears after deposit",
                 "DEFS.H:5694");
    check_uint_eq(state.leaderHandThing, DM1_V1_MCRADPR_THING_NONE_PC34,
                  "leader hand object clears after deposit",
                  "CHAMPION.C F0298:270-298");
    check_int_eq(state.leaderHandEmpty, 1, "leader hand is empty after deposit",
                 "CHAMPION.C F0298:270-298");
    check_true(chest_slots_are_none(&state), "G0425 slots reset after deposit",
               "DEFS.H:5876-5881 G0425/G0426");
    check_int_eq(state.openChestThing, DM1_V1_MCRADPR_THING_NONE_PC34,
                 "G0426 resets after deposit",
                 "DEFS.H:5876-5881 G0425/G0426");
    check_int_eq(state.f0334CloseCount, 1, "F0334 close ran once",
                 "CHEST.C F0334:117-132");
    check_int_eq(state.f0282ClickCount, 1, "F0282 click ran once",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.f0302SlotClickCount, 1, "F0302 slot click counted",
                 "CHAMPION.C F0302:662-713");

    ok = DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_RotatePartyPc34Compat(
        &state,
        DM1_V1_MCRADPR_C002_TURN_RIGHT_PC34);
    check_int_eq(ok, 1, "party rotates after deposit",
                 "CLIKMENU.C F0365:142-174");
    check_int_eq(state.partyDirection, 1, "party direction becomes east",
                 "CHAMPION.C F0284:93-131");
    check_int_eq(state.championCell[0], 1, "leader cell rotates",
                 "CHAMPION.C F0284:93-131");
    check_int_eq(state.championCell[1], 2, "second champion cell rotates",
                 "CHAMPION.C F0284:93-131");
    check_int_eq(state.championCell[2], 3, "third champion cell rotates",
                 "CHAMPION.C F0284:93-131");
    check_int_eq(state.championDirection[0], 1, "leader direction rotates",
                 "CHAMPION.C F0284:93-131");
    check_int_eq(state.f0365TurnPartyCount, 1, "F0365 turn count advances",
                 "CLIKMENU.C F0365:142-174");
    check_int_eq(state.f0284SetPartyDirectionCount, 1,
                 "F0284 direction count advances",
                 "CHAMPION.C F0284:93-131");

    ok = DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_OpenCandidatePc34Compat(
        &state);
    check_int_eq(ok, 1, "candidate reopens after rotate",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.g0299CandidateOrdinal, 4,
                 "G0299 republishes candidate on reopen",
                 "DEFS.H:5694");
    check_int_eq(state.c040PanelOpen, 1, "C040 reopens cleanly",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.panelContent, DM1_V1_MCRADPR_M568_PANEL_PC34,
                 "reopened panel is M568", "DEFS.H:3001-3008 M568/M569");
    check_int_eq(state.candidateReferenceDirection, 1,
                 "reopen uses rotated party direction as reference",
                 "CHAMPION.C F0284:93-131");
    check_int_eq(state.depositPending, 0, "no deposit pending after reopen",
                 "REVIVE.C F0282:744-806");
    check_uint_eq(state.pendingDepositThing, DM1_V1_MCRADPR_THING_NONE_PC34,
                  "pending deposit object remains clear",
                  "CHAMPION.C F0298:270-298");
    check_uint_eq(state.leaderHandThing, DM1_V1_MCRADPR_THING_NONE_PC34,
                  "leader hand stays empty on reopen",
                  "REVIVE.C F0280:124-132");
    check_int_eq(state.depositFireCount, 1,
                 "reopen does not fire deposit again",
                 "REVIVE.C F0282:744-806");
}

static void test_full_runtime_result_and_hash(void)
{
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat state;
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat state2;
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat result;
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat result2;
    unsigned int recomputedHash;
    int ok;
    int ok2;

    DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_InitPc34Compat(
        &state);
    ok = DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_RunPc34Compat(
        &state,
        &result);

    check_int_eq(ok, 1, "full sequence returns success",
                 result.evidence->nonOverlapScope);
    check_int_eq(result.ok, 1, "result ok bit is set",
                 result.evidence->nonOverlapScope);
    check_true(result.evidence ==
                   DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_EvidencePc34Compat(),
               "result carries evidence pointer", "metadata");
    check_int_eq(result.openedInitialCandidate, 1,
                 "initial candidate opened", result.evidence->reviveOpenAnchor);
    check_int_eq(result.putDepositInLeaderHand, 1,
                 "leader hand deposit was armed",
                 result.evidence->championPutAnchor);
    check_uint_eq(result.leaderHandThingBeforeDeposit, 0x6D40u,
                  "leader hand held deposit before firing",
                  result.evidence->championPutAnchor);
    check_int_eq(result.depositFired, 1, "deposit fired through mirror",
                 result.evidence->reviveClickAnchor);
    check_int_eq(result.mirrorClosedAfterDeposit, 1,
                 "mirror closes after deposit", result.evidence->reviveClickAnchor);
    check_int_eq(result.objectTransferred, 1,
                 "deposit object transferred", result.evidence->championSlotAddAnchor);
    check_uint_eq(result.depositedThing, 0x6D40u,
                  "result records deposited object",
                  result.evidence->championSlotAddAnchor);
    check_int_eq(result.g0425ResetAfterDeposit, 1,
                 "G0425 reset after deposit", result.evidence->chestCloseAnchor);
    check_int_eq(result.g0426ResetAfterDeposit, 1,
                 "G0426 reset after deposit", result.evidence->chestCloseAnchor);
    check_int_eq(result.g0299ResetAfterDeposit, 1,
                 "G0299 reset after deposit", result.evidence->reviveClickAnchor);
    check_int_eq(result.c040ResetAfterDeposit, 1,
                 "C040 reset after deposit", result.evidence->panelRedrawAnchor);
    check_int_eq(result.leaderHandEmptyAfterDeposit, 1,
                 "leader hand empty after deposit",
                 result.evidence->championRemoveAnchor);
    check_uint_eq(result.leaderHandThingAfterDeposit,
                  DM1_V1_MCRADPR_THING_NONE_PC34,
                  "leader hand thing clear after deposit",
                  result.evidence->championRemoveAnchor);
    check_int_eq(result.noPendingDepositAfterDeposit, 1,
                 "pending deposit clear after deposit",
                 result.evidence->reviveClickAnchor);
    check_int_eq(result.noLeftoverC040BeforeReopen, 1,
                 "no C040 leftover before reopen", result.evidence->panelRedrawAnchor);
    check_int_eq(result.noLeftoverChestBeforeReopen, 1,
                 "no chest leftover before reopen", result.evidence->chestCloseAnchor);
    check_int_eq(result.partyRotated, 1, "party rotated after deposit",
                 result.evidence->partyRotateAnchor);
    check_int_eq(result.partyDirectionBeforeRotate, 0,
                 "rotate starts from initial direction",
                 result.evidence->partyRotateAnchor);
    check_int_eq(result.partyDirectionAfterRotate, 1,
                 "rotate ends at new direction", result.evidence->partyRotateAnchor);
    check_int_eq(result.championCellsRotated, 1,
                 "champion cells follow party rotate",
                 result.evidence->partyRotateAnchor);
    check_int_eq(result.championDirectionsRotated, 1,
                 "champion directions follow party rotate",
                 result.evidence->partyRotateAnchor);
    check_int_eq(result.reopenedCandidate, 1,
                 "candidate reopens after rotate", result.evidence->reviveOpenAnchor);
    check_int_eq(result.reopenG0299CandidateOrdinal, 4,
                 "reopen republishes G0299 candidate",
                 result.evidence->defsAnchor);
    check_int_eq(result.reopenUsesNewDirection, 1,
                 "reopen uses new party direction",
                 result.evidence->partyRotateAnchor);
    check_int_eq(result.candidateReferenceDirectionBeforeDeposit, 0,
                 "first reference frame is old direction",
                 result.evidence->partyRotateAnchor);
    check_int_eq(result.candidateReferenceDirectionAfterReopen, 1,
                 "second reference frame is rotated direction",
                 result.evidence->partyRotateAnchor);
    check_int_eq(result.reopenHasCleanDepositState, 1,
                 "reopen has no leftover deposit state",
                 result.evidence->reviveClickAnchor);
    check_int_eq(result.reopenHasCleanPanelState, 1,
                 "reopen draws a fresh C040 panel", result.evidence->panelRedrawAnchor);
    check_int_eq(result.noCrash, 1, "no crash guard survived sequence",
                 result.evidence->commandQueueAnchor);
    check_int_eq(result.closeAfterReopen, 1,
                 "close after reopen succeeds", result.evidence->reviveClickAnchor);
    check_int_eq(result.closeAfterReopenNoDoubleFire, 1,
                 "close after reopen does not double-fire deposit",
                 result.evidence->reviveClickAnchor);
    check_int_eq(result.secondCloseNoop, 1,
                 "second close is a no-op", result.evidence->reviveClickAnchor);
    check_int_eq(result.doubleFireCountAfterClose, 0,
                 "double-fire count remains zero", result.evidence->reviveClickAnchor);
    check_int_eq(result.f0280OpenCountAfterReopen, 2,
                 "F0280 open ran for initial open and reopen",
                 result.evidence->reviveOpenAnchor);
    check_int_eq(result.f0282ClickCountAfterDeposit, 1,
                 "F0282 deposit click ran once", result.evidence->reviveClickAnchor);
    check_int_eq(result.f0334CloseCountAfterDeposit, 1,
                 "F0334 close ran once at deposit",
                 result.evidence->chestCloseAnchor);
    check_int_eq(result.f0378DispatchCountAfterDeposit, 1,
                 "F0378 dispatched deposit panel click once",
                 result.evidence->commandPanelAnchor);
    check_int_eq(result.f0380DispatchCountAfterRotate, 2,
                 "F0380 dispatched deposit and rotate",
                 result.evidence->commandQueueAnchor);
    check_int_eq(result.f0346PanelDrawCountAfterReopen, 2,
                 "C040 draw ran once per open", result.evidence->panelRedrawAnchor);
    check_int_eq(result.f0347PanelRedrawCountAfterReopen, 3,
                 "panel redraw includes open, close, reopen",
                 result.evidence->panelRedrawAnchor);
    check_int_eq(result.contractOnly, 1, "result is contract-only",
                 result.evidence->nonOverlapScope);
    check_int_eq(result.noAssetsOrPixelParity, 1,
                 "result makes no asset or pixel parity claim",
                 result.evidence->nonOverlapScope);
    check_int_eq(state.depositFireCount, 1,
                 "state deposit fire count is one after close",
                 result.evidence->reviveClickAnchor);
    check_int_eq(state.objectTransferCount, 1,
                 "state object transfer count is one after close",
                 result.evidence->objectAnchor);
    check_int_eq(state.doubleFireCount, 0,
                 "state double-fire count is zero",
                 result.evidence->reviveClickAnchor);
    check_int_eq(state.closeNoopCount, 1,
                 "state second close no-op counted once",
                 result.evidence->reviveClickAnchor);
    check_int_eq(state.g0299CandidateOrdinal, 0,
                 "final G0299 is clear after close",
                 result.evidence->defsAnchor);
    check_int_eq(state.c040PanelOpen, 0,
                 "final C040 is closed after close",
                 result.evidence->defsAnchor);
    recomputedHash =
        DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_HashPc34Compat(
            &state,
            &result);
    check_uint_eq(result.hash, recomputedHash, "reported hash is recomputable",
                  "deterministic hash");
    check_true(result.hash != 0u, "reported hash is non-zero",
               "deterministic hash");

    DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_InitPc34Compat(
        &state2);
    ok2 = DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_RunPc34Compat(
        &state2,
        &result2);
    check_int_eq(ok2, 1, "second sequence returns success",
                 "deterministic hash");
    check_uint_eq(result2.hash, result.hash,
                  "second run produces same stable hash",
                  "deterministic hash");
}

int main(void)
{
    unsigned int hash = 0u;

    test_source_lock_metadata();
    test_initial_state();
    test_stepwise_sequence();
    test_full_runtime_result_and_hash();

    if (gFailures == 0) {
        Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat state;
        Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat result;

        DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_InitPc34Compat(
            &state);
        (void)DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_RunPc34Compat(
            &state,
            &result);
        hash = result.hash;
    }

    if (gFailures) {
        printf("FAIL test_dm1_v1_mirror_candidate_reselect_after_deposit_with_party_rotate_pc34_compat "
               "assertions=%d failures=%d hash=0x%08X\n",
               gAssertions,
               gFailures,
               hash);
        return 1;
    }

    printf("PASS test_dm1_v1_mirror_candidate_reselect_after_deposit_with_party_rotate_pc34_compat "
           "assertions=%d failures=0 hash=0x%08X\n",
           gAssertions,
           hash);
    return 0;
}
