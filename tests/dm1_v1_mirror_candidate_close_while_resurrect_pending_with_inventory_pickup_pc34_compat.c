/* ReDMCSB anchors: COMMAND.C F0359:1985-1990 gates C040 close dispatch;
 * REVIVE.C F0280:124-132 and F0282:744-806 open/consume the pending
 * candidate; CHAMPION.C F0297/F0298/F0300/F0301/F0302 move the C537/C30
 * pickup into leader hand; CHEST.C F0333/F0334 and PANEL.C F0344/F0345/
 * F0352/F0346/F0347 redraw C040 against G0425; DEFS.H:2088 covers
 * C30/G0425/G0426/M070/M516/C040 plus C537/M568/G0299.
 */
#include "firestaff/dm1/v1/mirror_candidate/close_while_resurrect_pending_with_inventory_pickup_pc34_compat.h"

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
    const Dm1V1MirrorCwrpipEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_cwrpip_evidence_pc34_compat();
    const char *source =
        dm1_v1_mirror_candidate_cwrpip_source_evidence_pc34_compat();

    CHECK_TRUE(e != NULL && e->contractOnly == 1,
               "evidence metadata is available",
               "metadata");
    CHECK_TRUE(strstr(e->commandC040Anchor, "COMMAND.C F0359:1985-1990") !=
                   NULL,
               "COMMAND.C F0359 anchor is cited",
               e->commandC040Anchor);
    CHECK_TRUE(strstr(e->reviveOpenAnchor, "REVIVE.C F0280:124-132") != NULL,
               "REVIVE.C F0280 anchor is cited",
               e->reviveOpenAnchor);
    CHECK_TRUE(strstr(e->reviveFinishAnchor, "REVIVE.C F0282:744-806") !=
                   NULL,
               "REVIVE.C F0282 anchor is cited",
               e->reviveFinishAnchor);
    CHECK_TRUE(strstr(e->championHandAnchor, "F0297") != NULL &&
                   strstr(e->championSlotAnchor, "F0300") != NULL &&
                   strstr(e->championSlotAnchor, "F0302") != NULL,
               "CHAMPION.C hand and C30 slot anchors are cited",
               "CHAMPION.C F0297/F0300/F0302");
    CHECK_TRUE(strstr(e->chestAnchor, "CHEST.C F0333") != NULL &&
                   strstr(e->chestAnchor, "F0334") != NULL,
               "CHEST.C F0333/F0334 anchors are cited",
               e->chestAnchor);
    CHECK_TRUE(strstr(e->panelAnchor, "PANEL.C") != NULL &&
                   strstr(e->panelAnchor, "F0346/F0347:1619-1657") != NULL,
               "PANEL.C C040 redraw anchors are cited",
               e->panelAnchor);
    CHECK_TRUE(strstr(e->defsAnchor, "C30") != NULL &&
                   strstr(e->defsAnchor, "G0425") != NULL &&
                   strstr(e->defsAnchor, "C040") != NULL &&
                   strstr(e->defsAnchor, "C537") != NULL &&
                   strstr(e->defsAnchor, "M568") != NULL &&
                   strstr(e->defsAnchor, "G0299") != NULL,
               "DEFS.H requested symbols are cited",
               e->defsAnchor);
    CHECK_TRUE(strstr(e->scope, "close-while-resurrect-pending") != NULL &&
                   strstr(e->scope, "pass732") != NULL &&
                   strstr(e->scope, "pass698") != NULL,
               "scope distinguishes this gate from existing coverage",
               e->scope);
    CHECK_TRUE(strstr(source, "COMMAND.C F0359:1985-1990") != NULL &&
                   strstr(source, "REVIVE.C F0282:744-806") != NULL,
               "source evidence string is populated",
               "source evidence");
}

static void check_manual_close_while_pending_flow(void)
{
    Dm1V1MirrorCwrpipStatePc34Compat state;
    Dm1V1MirrorCwrpipResultPc34Compat pickup;
    Dm1V1MirrorCwrpipResultPc34Compat close;
    Dm1V1MirrorCwrpipResultPc34Compat reopen;

    dm1_v1_mirror_candidate_cwrpip_init_pc34_compat(&state);

    CHECK_TRUE(state.currentStep ==
                   DM1_V1_MIRROR_CWRPIP_STEP_RESURRECT_CANDIDATE_PENDING_PC34_COMPAT,
               "fixture starts at STEP_RESURRECT_CANDIDATE_PENDING",
               "REVIVE.C F0280:124-132");
    CHECK_TRUE(state.dominantStep ==
                   DM1_V1_MIRROR_CWRPIP_STEP_RESURRECT_CANDIDATE_PENDING_PC34_COMPAT &&
                   state.pendingCandidateOrdinal ==
                       state.originalCandidateOrdinal,
               "pending resurrect candidate is dominant before pickup",
               "REVIVE.C F0280:124-132");
    CHECK_TRUE(state.c30Chain[0] != DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT &&
                   state.leaderEmptyHanded == 1,
               "C537/C30 source exists and leader hand starts empty",
               "CHAMPION.C F0302:662-713");

    CHECK_TRUE(
        dm1_v1_mirror_candidate_cwrpip_c537_pickup_before_close_pc34_compat(
            &state, &pickup) == 1,
        "C537 pickup fires before C040 close",
        "CHAMPION.C F0302:662-713");
    CHECK_TRUE(pickup.dominantPending == 1 &&
                   pickup.pendingCandidateNotConsumed == 1,
               "STEP_RESURRECT_CANDIDATE_PENDING remains dominant after pickup",
               "REVIVE.C F0282:744-806");
    CHECK_TRUE(pickup.pickupLandedInLeaderHandFromC30 == 1 &&
                   pickup.leaderHandAfter == pickup.c30Slot0Before &&
                   pickup.c30Slot0After ==
                       DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT,
               "picked thing lands in leader-hand C30 chain",
               "CHAMPION.C F0297/F0300/F0302; DEFS.H C30/C537");
    CHECK_TRUE(pickup.candidateAfter == state.originalCandidateOrdinal &&
                   state.f0282CandidateConsumeCount == 0,
               "pickup does not consume the pending resurrect candidate",
               "REVIVE.C F0282:744-806");

    CHECK_TRUE(
        dm1_v1_mirror_candidate_cwrpip_c040_close_while_pending_pc34_compat(
            &state, &close) == 1,
        "C040 close is driven after pickup while candidate is pending",
        "COMMAND.C F0359:1985-1990");
    CHECK_TRUE(close.closeBlockedByLeaderHand == 1 &&
                   close.pendingCandidateNotConsumed == 1 &&
                   close.dominantPending == 1,
               "occupied leader hand blocks F0282 and preserves pending state",
               "COMMAND.C F0359:1985-1990");
    CHECK_TRUE(close.panelRedrewAgainstPickupModifiedChain == 1 &&
                   state.panelRedrawSawLeaderHandThing == pickup.leaderHandAfter &&
                   state.panelRedrawSawC30Slot0Thing ==
                       DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT,
               "C040 panel chrome redraws against pickup-modified C30 chain",
               "PANEL.C F0346/F0347:1619-1657; CHEST.C F0334");
    CHECK_TRUE(state.c040PanelOpen == 1 &&
                   state.panelContent ==
                       DM1_V1_MIRROR_CWRPIP_M568_PANEL_PC34_COMPAT &&
                   state.panelGraphic ==
                       DM1_V1_MIRROR_CWRPIP_C040_PANEL_PC34_COMPAT,
               "C040 chrome remains the active panel after blocked close",
               "PANEL.C F0346/F0347:1619-1657");

    CHECK_TRUE(
        dm1_v1_mirror_candidate_cwrpip_next_open_refire_pc34_compat(
            &state, &reopen) == 1,
        "next mirror-candidate open is driven",
        "REVIVE.C F0280:124-132");
    CHECK_TRUE(reopen.reopenRefiredOriginalChampion == 1 &&
                   reopen.candidateAfter == state.originalCandidateOrdinal &&
                   state.nextOpenRefiredOrdinal ==
                       (int)state.originalCandidateOrdinal,
               "pending resurrect re-fires with the original champion",
               "REVIVE.C F0280:124-132");
    CHECK_TRUE(state.f0280CandidateOpenCount == 2 &&
                   state.f0282CandidateConsumeCount == 0 &&
                   state.candidateConsumedByCloseCount == 0,
               "only F0280 re-open count advances; F0282 never consumes",
               "REVIVE.C F0280/F0282");
}

int main(void)
{
    int ok;
    int selfAssertions;
    int selfFailures;
    unsigned int hash;

    check_source_lock_metadata();
    check_manual_close_while_pending_flow();

    ok = dm1_v1_mirror_candidate_cwrpip_run_self_test_pc34_compat();
    selfAssertions = dm1_v1_mirror_candidate_cwrpip_assertions_pc34_compat();
    selfFailures = dm1_v1_mirror_candidate_cwrpip_failures_pc34_compat();
    hash = dm1_v1_mirror_candidate_cwrpip_hash_pc34_compat();

    CHECK_TRUE(ok == 1 && selfFailures == 0,
               "library self-test passes",
               "self-test");
    CHECK_TRUE(selfAssertions >= 20,
               "library self-test covers pickup, close, and re-open",
               "state-machine driver");
    CHECK_TRUE(hash != 0u,
               "deterministic hash is non-zero",
               "deterministic hash");

    if (gFailures != 0 || selfFailures != 0) {
        printf("FAIL dm1_v1_mirror_candidate_close_while_resurrect_pending_with_inventory_pickup_pc34_compat assertions=%d failures=%d self_assertions=%d self_failures=%d hash=0x%08X\n",
               gAssertions,
               gFailures,
               selfAssertions,
               selfFailures,
               hash);
        return 1;
    }

    printf("PASS dm1_v1_mirror_candidate_close_while_resurrect_pending_with_inventory_pickup_pc34_compat assertions=%d failures=0 self_assertions=%d self_failures=0 hash=0x%08X\n",
           gAssertions,
           selfAssertions,
           hash);
    return 0;
}
