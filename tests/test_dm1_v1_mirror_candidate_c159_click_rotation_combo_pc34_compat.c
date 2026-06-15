#include "dm1_v1_mirror_candidate_c159_click_rotation_combo_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateC159ClickRotationComboEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateC159ClickRotationCombo_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "source-lock evidence is available",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(e->contractOnly == 1 &&
                      strstr(e->contractScope, "contract_only=1") != NULL,
                  "fixture is explicitly contract-only",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->commandNameRowAnchor, "484-488") != NULL &&
                      strstr(e->commandNameRowAnchor, "C159") != NULL,
                  "COMMAND.C C159 name-row mapping is cited",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(strstr(e->commandC040DispatchAnchor, "1985-1990") != NULL &&
                      strstr(e->commandC040DispatchAnchor, "F0282") != NULL,
                  "COMMAND.C C040 panel dispatch anchor is cited",
                  e->commandC040DispatchAnchor);
    CHECK_REDMCSB(strstr(e->commandStatusInventoryGuardAnchor,
                         "2159-2181") != NULL &&
                      strstr(e->commandStatusInventoryGuardAnchor,
                             "!G0299") != NULL,
                  "COMMAND.C status/inventory !G0299 guard is cited",
                  e->commandStatusInventoryGuardAnchor);
    CHECK_REDMCSB(strstr(e->commandSpellActionGuardAnchor,
                         "2302-2311") != NULL &&
                      strstr(e->commandSpellActionGuardAnchor,
                             "!G0299") != NULL,
                  "COMMAND.C spell/action !G0299 guard is cited",
                  e->commandSpellActionGuardAnchor);
    CHECK_REDMCSB(strstr(e->commandSaveGuardAnchor, "2366-2370") != NULL &&
                      strstr(e->commandSaveGuardAnchor, "!G0299") != NULL,
                  "COMMAND.C save !G0299 guard is cited",
                  e->commandSaveGuardAnchor);
    CHECK_REDMCSB(strstr(e->reviveCandidatePublishGateAnchor,
                         "124-132") != NULL,
                  "REVIVE.C F0280 publish gate is cited",
                  e->reviveCandidatePublishGateAnchor);
    CHECK_REDMCSB(strstr(e->reviveCandidatePublishAnchor,
                         "1501-1503") != NULL &&
                      strstr(e->reviveCandidatePublishAnchor,
                             "272-276") != NULL,
                  "MOVESENS to REVIVE publish path is cited",
                  e->reviveCandidatePublishAnchor);
    CHECK_REDMCSB(strstr(e->reviveCandidateClearAnchor, "744-758") != NULL &&
                      strstr(e->reviveCandidateClearAnchor,
                             "785-806") != NULL,
                  "REVIVE.C F0282 candidate-clear paths are cited",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(strstr(e->movesensPublishAnchor, "1501-1503") != NULL,
                  "MOVESENS.C C127 publish source is cited",
                  e->movesensPublishAnchor);
    CHECK_REDMCSB(strstr(e->championLeaderHandAnchor, "243-285") != NULL &&
                      strstr(e->championLeaderHandAnchor,
                             "662-706") != NULL,
                  "CHAMPION.C leader-hand and slot routes are cited",
                  e->championLeaderHandAnchor);
    CHECK_REDMCSB(strstr(e->panelPortraitNameZoneAnchor,
                         "2208-2240") != NULL &&
                      strstr(e->panelPortraitNameZoneAnchor,
                             "484-488") != NULL,
                  "PANEL.C C175 portrait and C159 name-zone anchors are cited",
                  e->panelPortraitNameZoneAnchor);
    CHECK_REDMCSB(strstr(e->defsPanelCommandAnchor, "338-340") != NULL,
                  "DEFS.H C160/C161/C162 panel commands are cited",
                  e->defsPanelCommandAnchor);
    CHECK_REDMCSB(strstr(e->defsNameZoneAnchor, "3787-3790") != NULL,
                  "DEFS.H C159..C162 champion-name zones are cited",
                  e->defsNameZoneAnchor);
    CHECK_REDMCSB(strstr(e->defsPanelZoneAnchor, "3941-3943") != NULL &&
                      strstr(e->defsPanelZoneAnchor, "3980-3982") != NULL,
                  "local DEFS.H M664/M665/M666 panel zones are cited",
                  e->defsPanelZoneAnchor);
    CHECK_REDMCSB(strstr(e->defsPanelZoneAnchor, "4041-4042") != NULL &&
                      strstr(e->defsPanelZoneAnchor, "viewport wall") != NULL,
                  "requested DEFS.H:4041-4042 mismatch is documented",
                  e->defsPanelZoneAnchor);
}

static void publish_live_candidate(
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state)
{
    int published;
    const Dm1V1MirrorCandidateC159ClickRotationComboEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateC159ClickRotationCombo_EvidencePc34Compat();

    DM1_V1_MirrorCandidateC159ClickRotationCombo_InitPc34Compat(state);
    published =
        DM1_V1_MirrorCandidateC159ClickRotationCombo_PublishCandidatePc34Compat(
            state);

    CHECK_REDMCSB(published == 1,
                  "portrait sensor publishes one mirror candidate",
                  e->movesensPublishAnchor);
    CHECK_REDMCSB(state->candidateChampionOrdinal == 3u,
                  "G0299 candidate ordinal becomes the appended champion",
                  e->reviveCandidatePublishAnchor);
    CHECK_REDMCSB(state->partyChampionCount == 3u,
                  "candidate publish increments party champion count",
                  e->reviveCandidatePublishAnchor);
    CHECK_REDMCSB(state->c040PanelOpen == 1 &&
                      state->panelKind ==
                          DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_M568_PANEL_PC34_COMPAT,
                  "C040/M568 panel owns the live candidate",
                  e->commandC040DispatchAnchor);
    CHECK_REDMCSB(state->leaderEmptyHanded == 1,
                  "publish fixture satisfies the leader-empty guard",
                  e->reviveCandidatePublishGateAnchor);
}

static void test_c159_cancel_combo(void)
{
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat state;
    Dm1V1MirrorCandidateC159ClickRotationComboResultPc34Compat result;
    const Dm1V1MirrorCandidateC159ClickRotationComboEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateC159ClickRotationCombo_EvidencePc34Compat();
    int ok;

    publish_live_candidate(&state);
    ok = DM1_V1_MirrorCandidateC159ClickRotationCombo_RunPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C162_CANCEL_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(ok == 1,
                  "C159 plus C162 cancel combo satisfies contract",
                  e->commandC040DispatchAnchor);
    CHECK_REDMCSB(result.c159Clicked == 1 &&
                      result.c159MapsToC016 == 1,
                  "C159 row click maps to the C016 leader route",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(result.c159BlockedByG0299 == 1 &&
                      result.c159SetLeaderSkipped == 1,
                  "G0299 blocks the C159 set-leader dispatch",
                  e->commandStatusInventoryGuardAnchor);
    CHECK_REDMCSB(result.f0367StatusDispatchCountAfter ==
                      result.f0367StatusDispatchCountBefore &&
                      result.f0368SetLeaderCountAfter ==
                          result.f0368SetLeaderCountBefore,
                  "F0367/F0368 are not reached while G0299 is live",
                  e->commandStatusInventoryGuardAnchor);
    CHECK_REDMCSB(result.panelStillOwnedAfterC159 == 1 &&
                      result.c040PanelOpenAfterC159 == 1,
                  "C159 does not steal the C040 panel owner",
                  e->commandC040DispatchAnchor);
    CHECK_REDMCSB(result.panelCommandDispatchedAfterC159 == 1 &&
                      state.cancelDispatchCount == 1,
                  "C162 cancel still dispatches through the C040 panel",
                  e->commandC040DispatchAnchor);
    CHECK_REDMCSB(result.candidateClearedByPanelCommand == 1 &&
                      result.candidateOrdinalAfterPanel == 0u,
                  "C162 cancel clears G0299",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.cancelRemovedCandidateChampion == 1 &&
                      state.partyChampionCount == 2u,
                  "C162 cancel removes the appended candidate champion",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.nonPanelInputsBlockedByG0299 == 1,
                  "spell/action/save probes remain blocked while G0299 is live",
                  e->commandSpellActionGuardAnchor);
    CHECK_REDMCSB(result.leaderIndexBefore == result.leaderIndexAfter,
                  "blocked C159 click does not change leader",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(result.noLeaderHandRoutes == 1 &&
                      result.leaderHandPreserved == 1,
                  "leader-hand put/remove routes are not entered",
                  e->championLeaderHandAnchor);
    CHECK_REDMCSB(result.noSlotRoutes == 1,
                  "slot-box route is not entered",
                  e->championLeaderHandAnchor);
}

static void test_c159_accept_combo(int panelCommand)
{
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat state;
    Dm1V1MirrorCandidateC159ClickRotationComboResultPc34Compat result;
    const Dm1V1MirrorCandidateC159ClickRotationComboEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateC159ClickRotationCombo_EvidencePc34Compat();
    int ok;

    publish_live_candidate(&state);
    ok = DM1_V1_MirrorCandidateC159ClickRotationCombo_RunPc34Compat(
        &state, panelCommand, &result);

    CHECK_REDMCSB(ok == 1,
                  "C159 plus C160/C161 accept combo satisfies contract",
                  e->commandC040DispatchAnchor);
    CHECK_REDMCSB(result.panelCommandValid == 1,
                  "panel command belongs to C160/C161/C162",
                  e->defsPanelCommandAnchor);
    CHECK_REDMCSB(result.c159BlockedByG0299 == 1,
                  "G0299 blocks C159 before the panel command",
                  e->commandStatusInventoryGuardAnchor);
    CHECK_REDMCSB(result.panelCommandDispatchedAfterC159 == 1,
                  "accept command still dispatches after blocked C159",
                  e->commandC040DispatchAnchor);
    CHECK_REDMCSB(result.candidateClearedByPanelCommand == 1,
                  "accept command clears G0299 through F0282",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.acceptedCandidateChampionRemains == 1 &&
                      state.partyChampionCount == 3u,
                  "accepted candidate remains in the party count",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.c040PanelOpenAfterPanel == 0,
                  "C040 panel closes after accept command",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.nonPanelInputsBlockedByG0299 == 1,
                  "non-panel inputs are blocked before F0282 clears G0299",
                  e->commandSaveGuardAnchor);
    CHECK_REDMCSB(result.noLeaderHandRoutes == 1 &&
                      result.noSlotRoutes == 1,
                  "leader hand and slot routes remain untouched",
                  e->championLeaderHandAnchor);
}

static void test_c159_baseline_without_g0299_reaches_leader_route(void)
{
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat state;
    Dm1V1MirrorCandidateC159ClickRotationComboResultPc34Compat result;
    const Dm1V1MirrorCandidateC159ClickRotationComboEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateC159ClickRotationCombo_EvidencePc34Compat();
    int ok;

    DM1_V1_MirrorCandidateC159ClickRotationCombo_InitPc34Compat(&state);
    state.leaderIndex = 1;
    ok = DM1_V1_MirrorCandidateC159ClickRotationCombo_RunPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C162_CANCEL_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(ok == 0,
                  "no-G0299 baseline is not the live-candidate combo",
                  e->contractScope);
    CHECK_REDMCSB(result.c159Clicked == 1 &&
                      result.c159MapsToC016 == 1,
                  "baseline C159 click still maps to C016",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(result.c159BlockedByG0299 == 0,
                  "baseline has no G0299 block",
                  e->commandStatusInventoryGuardAnchor);
    CHECK_REDMCSB(result.f0367StatusDispatchCountAfter ==
                      result.f0367StatusDispatchCountBefore + 1 &&
                      result.f0368SetLeaderCountAfter ==
                          result.f0368SetLeaderCountBefore + 1,
                  "baseline reaches F0367/F0368 set-leader route",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(state.leaderIndex == 0,
                  "baseline C159 route sets champion-0 leader",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(state.panelDispatchCount == 0,
                  "baseline does not dispatch C040 panel command",
                  e->commandC040DispatchAnchor);
}

int main(void)
{
    test_source_lock_metadata();
    test_c159_cancel_combo();
    test_c159_accept_combo(
        DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C160_RESURRECT_PC34_COMPAT);
    test_c159_accept_combo(
        DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C161_REINCARNATE_PC34_COMPAT);
    test_c159_baseline_without_g0299_reaches_leader_route();

    printf("PASS dm1_v1_mirror_candidate_c159_click_rotation_combo_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
