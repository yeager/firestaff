#include "dm1_v1_mirror_candidate_resurrect_reselect_with_inventory_pickup_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gAssertions;
static int gFailures;

#define CHECK_TRUE(cond, msg, anchor) do { \
    ++gAssertions; \
    if (!(cond)) { \
        ++gFailures; \
        printf("FAIL: %s [%s]\n", (msg), (anchor)); \
    } \
} while (0)

static void check_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateRripEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateRrip_EvidencePc34Compat();
    const char *source = DM1_V1_MirrorCandidateRrip_SourceEvidencePc34Compat();

    CHECK_TRUE(e != NULL && e->contractOnly == 1,
               "evidence metadata is available",
               "metadata");
    CHECK_TRUE(strstr(e->championDirectionAnchor,
                      "CHAMPION.C F0284:93-131") != NULL,
               "CHAMPION.C F0284 anchor is cited",
               e->championDirectionAnchor);
    CHECK_TRUE(strstr(e->championLeaderHandAnchor,
                      "CHAMPION.C F0297:243-268") != NULL,
               "CHAMPION.C F0297 anchor is cited",
               e->championLeaderHandAnchor);
    CHECK_TRUE(strstr(e->championSlotBoxAnchor,
                      "CHAMPION.C F0302:662-713") != NULL,
               "CHAMPION.C F0302 anchor is cited",
               e->championSlotBoxAnchor);
    CHECK_TRUE(strstr(e->reviveOpenAnchor,
                      "REVIVE.C F0280:124-132") != NULL,
               "REVIVE.C F0280 anchor is cited",
               e->reviveOpenAnchor);
    CHECK_TRUE(strstr(e->reviveFinishAnchor,
                      "REVIVE.C F0282:744-806") != NULL,
               "REVIVE.C F0282 anchor is cited",
               e->reviveFinishAnchor);
    CHECK_TRUE(strstr(e->panelDrawAnchor, "PANEL.C F0344/F0345") != NULL &&
                   strstr(e->panelDrawAnchor, "F0346/F0347:1619-1657") != NULL,
               "PANEL.C F0344/F0345/F0346/F0347 anchors are cited",
               e->panelDrawAnchor);
    CHECK_TRUE(strstr(e->commandPanelAnchor,
                      "COMMAND.C F0359:1985-1990") != NULL,
               "COMMAND.C F0359 anchor is cited",
               e->commandPanelAnchor);
    CHECK_TRUE(strstr(e->commandQueueAnchor,
                      "COMMAND.C F0380:2045-2156") != NULL,
               "COMMAND.C F0380 anchor is cited",
               e->commandQueueAnchor);
    CHECK_TRUE(strstr(e->chestOpenAnchor, "CHEST.C F0333") != NULL &&
                   strstr(e->chestCloseAnchor, "CHEST.C F0334") != NULL,
               "CHEST.C F0333/F0334 anchors are cited",
               "CHEST.C F0333/F0334");
    CHECK_TRUE(strstr(e->defsAnchor, "C038") != NULL &&
                   strstr(e->defsAnchor, "C30") != NULL &&
                   strstr(e->defsAnchor, "C040") != NULL &&
                   strstr(e->defsAnchor, "G0425") != NULL &&
                   strstr(e->defsAnchor, "G0426") != NULL &&
                   strstr(e->defsAnchor, "M070") != NULL &&
                   strstr(e->defsAnchor, "M516") != NULL,
               "DEFS.H C30/C040/G0425/G0426/M070/M516 anchors are cited",
               e->defsAnchor);
    CHECK_TRUE(strstr(e->nonDuplicationScope, "reselect") != NULL &&
                   strstr(e->nonDuplicationScope, "inventory") != NULL,
               "non-duplication note names this new overlap path",
               e->nonDuplicationScope);
    CHECK_TRUE(strstr(source, "REVIVE.C F0280:124-132") != NULL &&
                   strstr(source, "COMMAND.C F0380:2045-2156") != NULL,
               "source evidence string is populated",
               "source evidence");
}

static void check_manual_confirm_flow(void)
{
    Dm1V1MirrorCandidateRripStatePc34Compat state;
    Dm1V1MirrorCandidateRripResultPc34Compat reselect;
    Dm1V1MirrorCandidateRripResultPc34Compat inventory;
    Dm1V1MirrorCandidateRripResultPc34Compat finish;

    DM1_V1_MirrorCandidateRrip_InitPc34Compat(&state);

    CHECK_TRUE(state.candidateChampionOrdinal == 2 &&
                   state.selectedChampionOrdinal == 2,
               "fixture starts with a C040 candidate bound to champion 2",
               "REVIVE.C F0280:124-132");
    CHECK_TRUE(state.panelContent ==
                   DM1_V1_MIRROR_CANDIDATE_RRIP_M568_PANEL_PC34_COMPAT &&
                   state.panelGraphic ==
                       DM1_V1_MIRROR_CANDIDATE_RRIP_C040_GRAPHIC_PC34_COMPAT,
               "fixture starts with M568/C040 panel",
               "PANEL.C F0346/F0347:1619-1657");
    CHECK_TRUE(state.sourceC30Thing == 0x7038 &&
                   state.leaderHandThing == 0,
               "source C30 scroll and hand state are known before overlap",
               "DEFS.H C30:810; CHAMPION.C F0297:243-268");

    CHECK_TRUE(
        DM1_V1_MirrorCandidateRrip_ReselectSameChampionPc34Compat(
            &state, &reselect) == 1,
        "same portrait reselect reissues resurrect candidate",
        "REVIVE.C F0280:124-132");
    CHECK_TRUE(reselect.candidateBoundToSelectedChampion == 1 &&
                   reselect.candidateAfter == 2,
               "candidate remains bound to champion 2 after reselect",
               "REVIVE.C F0280:124-132");

    CHECK_TRUE(
        DM1_V1_MirrorCandidateRrip_InventoryClickDuringReselectPc34Compat(
            &state, &inventory) == 0,
        "in-flight C038 inventory click is refused while candidate is alive",
        "CHAMPION.C F0302:662-713");
    CHECK_TRUE(inventory.blocked == 1 &&
                   inventory.queued == 1 &&
                   state.blockedInventoryClicks == 1,
               "refused inventory click is retained as queued work",
               "COMMAND.C F0380:2045-2156");
    CHECK_TRUE(state.queuedInventoryCommand ==
                   DM1_V1_MIRROR_CANDIDATE_RRIP_C038_SCROLL_PICKUP_PC34_COMPAT,
               "queued click preserves C038 identity",
               "DEFS.H C038:275");
    CHECK_TRUE(state.queuedInventorySlot ==
                   DM1_V1_MIRROR_CANDIDATE_RRIP_C30_CHEST_SLOT_PC34_COMPAT,
               "queued click preserves C30 destination/source slot",
               "DEFS.H C30:810; CHAMPION.C F0302:689-690");
    CHECK_TRUE(state.sourceC30Thing == 0x7038 &&
                   state.leaderHandThing == 0,
               "refusal leaves champion hand and C30 thing untouched",
               "CHAMPION.C F0297:243-268; F0302:704-709");

    CHECK_TRUE(
        DM1_V1_MirrorCandidateRrip_FinishCandidatePc34Compat(
            &state,
            DM1_V1_MIRROR_CANDIDATE_RRIP_FINISH_CONFIRM_PC34_COMPAT,
            &finish) == 1,
        "confirming resurrect clears candidate through C040 panel route",
        "COMMAND.C F0359:1985-1990; REVIVE.C F0282:744-806");
    CHECK_TRUE(finish.candidateAfter == 0 &&
                   state.candidateChampionOrdinal == 0,
               "candidate is cleared after confirm",
               "REVIVE.C F0282:785");
    CHECK_TRUE(finish.dispatched == 1 &&
                   finish.dispatchWaitedForCandidateFinish == 1,
               "queued inventory click dispatches only after candidate finish",
               "COMMAND.C F0380:2045-2156");
    CHECK_TRUE(state.chestSlot0Thing == 0x7038,
               "queued inventory click is dispatched to chest/mirror C30",
               "CHEST.C F0333:30-67; CHAMPION.C F0302:689-709");
    CHECK_TRUE(state.leaderHandThing == 0 &&
                   state.sourceC30Thing == 0x7038 &&
                   state.handPreservedCount == 1,
               "champion hand state preserves whatever was in C30",
               "CHAMPION.C F0297:243-268; DEFS.H M516:873/876");
}

int main(void)
{
    int ok;
    int selfAssertions;
    int selfFailures;
    int hash;

    check_source_lock_metadata();
    check_manual_confirm_flow();

    ok = DM1_V1_MirrorCandidateRrip_RunSelfTestPc34Compat();
    selfAssertions = DM1_V1_MirrorCandidateRrip_AssertionsPc34Compat();
    selfFailures = DM1_V1_MirrorCandidateRrip_FailuresPc34Compat();
    hash = DM1_V1_MirrorCandidateRrip_DeterministicHashPc34Compat();

    CHECK_TRUE(ok == 1 && selfFailures == 0,
               "library self-test passes",
               "self-test");
    CHECK_TRUE(selfAssertions >= 40,
               "library self-test covers confirm and cancel paths",
               "REVIVE.C F0282:744-806");
    CHECK_TRUE(hash != 0,
               "deterministic hash is non-zero",
               "deterministic hash");

    if (gFailures != 0 || selfFailures != 0) {
        printf("FAIL dm1_v1_mirror_candidate_resurrect_reselect_with_"
               "inventory_pickup_pc34_compat assertions=%d failures=%d "
               "self_assertions=%d self_failures=%d hash=0x%08X\n",
               gAssertions,
               gFailures,
               selfAssertions,
               selfFailures,
               (unsigned int)hash);
        return 1;
    }

    printf("PASS dm1_v1_mirror_candidate_resurrect_reselect_with_"
           "inventory_pickup_pc34_compat assertions=%d failures=0 "
           "self_assertions=%d self_failures=0 hash=0x%08X\n",
           gAssertions,
           selfAssertions,
           (unsigned int)hash);
    return 0;
}
