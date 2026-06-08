#include "dm1_v1_mirror_candidate_left_click_rotation_pc34_compat.h"

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
    const Dm1V1MirrorCandidateLeftClickRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateLeftClickRotation_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "source-lock evidence is available",
                  "metadata");
    CHECK_REDMCSB(e->contractOnly == 1 &&
                      strstr(e->contractScope, "contract_only=1") != NULL,
                  "fixture is explicitly contract-only",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->commandPanelDispatchAnchor, "1985-1990") != NULL &&
                      strstr(e->commandPanelDispatchAnchor, "F0282") != NULL,
                  "COMMAND.C C040 mirror dispatch anchor is cited",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(strstr(e->commandClickRoutingAnchor, "F0282") != NULL &&
                      strstr(e->commandClickRoutingAnchor, "508-511") != NULL,
                  "click-to-C040 routing metadata is cited",
                  e->commandClickRoutingAnchor);
    CHECK_REDMCSB(strstr(e->championPartyDirectionAnchor, "F0284") != NULL &&
                      strstr(e->championPartyDirectionAnchor, "93-131") != NULL,
                  "CHAMPION.C F0284 rotation anchor is cited",
                  e->championPartyDirectionAnchor);
    CHECK_REDMCSB(strstr(e->championLeaderHandPutAnchor, "F0297") != NULL &&
                      strstr(e->championLeaderHandPutAnchor, "243-298") != NULL,
                  "leader-hand put route is cited",
                  e->championLeaderHandPutAnchor);
    CHECK_REDMCSB(strstr(e->championLeaderHandRemoveAnchor, "F0298") != NULL &&
                      strstr(e->championLeaderHandRemoveAnchor, "270-298") != NULL,
                  "leader-hand remove route is cited",
                  e->championLeaderHandRemoveAnchor);
    CHECK_REDMCSB(strstr(e->championSlotWriteClearAnchor, "F0300/F0301") != NULL &&
                      strstr(e->championSlotWriteClearAnchor, "511-515") != NULL &&
                      strstr(e->championSlotWriteClearAnchor, "606-614") != NULL,
                  "slot write/clear route is cited",
                  e->championSlotWriteClearAnchor);
    CHECK_REDMCSB(strstr(e->panelPortraitRedrawAnchor, "F0354") != NULL &&
                      strstr(e->panelPortraitRedrawAnchor, "2208-2240") != NULL,
                  "PANEL.C portrait/redraw anchor is cited",
                  e->panelPortraitRedrawAnchor);
    CHECK_REDMCSB(strstr(e->defsColorAnchor, "DEFS.H:2088") != NULL &&
                      strstr(e->defsColorAnchor, "C10") != NULL,
                  "DEFS.H C10 anchor is cited",
                  e->defsColorAnchor);
    CHECK_REDMCSB(strstr(e->defsHandSlotAnchor, "M070") != NULL,
                  "DEFS.H M070 hand-slot anchor is cited",
                  e->defsHandSlotAnchor);
    CHECK_REDMCSB(strstr(e->defsRosterAnchor, "M516") != NULL,
                  "DEFS.H M516 roster/rotation-mask anchor is cited",
                  e->defsRosterAnchor);
    CHECK_REDMCSB(strstr(e->defsGlobalsAnchor, "G0425") != NULL &&
                      strstr(e->defsGlobalsAnchor, "G0426") != NULL &&
                      strstr(e->defsGlobalsAnchor, "G0423") != NULL &&
                      strstr(e->defsGlobalsAnchor, "G0305") != NULL,
                  "DEFS.H chest/inventory/party globals are cited",
                  e->defsGlobalsAnchor);
    CHECK_REDMCSB(strstr(e->g0299GuardAnchor, "!G0299") != NULL &&
                      strstr(e->g0299GuardAnchor, "2302-2311") != NULL &&
                      strstr(e->g0299GuardAnchor, "2366-2370") != NULL,
                  "!G0299 guarded routes are cited",
                  e->g0299GuardAnchor);
    CHECK_REDMCSB(strstr(e->nonOverlapNote, "left-click candidate rotation") != NULL &&
                      strstr(e->nonOverlapNote, "C159 click combo") != NULL,
                  "metadata records non-overlap with existing gates",
                  e->nonOverlapNote);
}

static void test_left_click_rotates_candidate_view_only(void)
{
    Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat state;
    Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat result;
    const Dm1V1MirrorCandidateLeftClickRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateLeftClickRotation_EvidencePc34Compat();
    int ok;

    DM1_V1_MirrorCandidateLeftClickRotation_InitPc34Compat(&state);
    ok = DM1_V1_MirrorCandidateLeftClickRotation_ApplyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(ok == 1 && result.rotationViewOnly == 1,
                  "left-click rotation contract succeeds",
                  e->contractScope);
    CHECK_REDMCSB(result.eventDispatchedToMirrorCandidateHandler == 1 &&
                      result.leftClickDispatchCountAfter ==
                          result.leftClickDispatchCountBefore + 1 &&
                      result.mirrorCandidateHandlerCountAfter ==
                          result.mirrorCandidateHandlerCountBefore + 1,
                  "left-click is dispatched to the mirror-candidate handler",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.rotationDispatchCountAfter ==
                      result.rotationDispatchCountBefore + 1,
                  "rotation dispatcher runs exactly once",
                  e->commandClickRoutingAnchor);
    CHECK_REDMCSB(result.candidateAdvancedByOne == 1 &&
                      result.visibleCandidateIndexBefore == 1 &&
                      result.visibleCandidateIndexAfter == 2,
                  "candidate index advances by one",
                  e->defsRosterAnchor);
    CHECK_REDMCSB(result.candidateStayedWithinVisibleSet == 1 &&
                      result.visibleCandidateCountBefore == 4 &&
                      result.visibleCandidateCountAfter == 4,
                  "candidate remains within the visible candidate set",
                  e->defsRosterAnchor);
    CHECK_REDMCSB(result.candidateOrdinalBefore == 12u &&
                      result.candidateOrdinalAfter == 13u &&
                      result.g0299After == 13u,
                  "G0299 follows the newly visible candidate ordinal",
                  e->defsGlobalsAnchor);
    CHECK_REDMCSB(result.noLeaderHandSwap == 1 &&
                      result.leaderHandThingBefore ==
                          DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_LEADER_HAND_THING_PC34_COMPAT &&
                      result.leaderHandThingAfter ==
                          result.leaderHandThingBefore,
                  "leader-hand content is preserved",
                  e->championLeaderHandPutAnchor);
    CHECK_REDMCSB(result.leaderHandPutCountAfter ==
                      result.leaderHandPutCountBefore &&
                      result.leaderHandRemoveCountAfter ==
                          result.leaderHandRemoveCountBefore,
                  "rotation enters no F0297/F0298 leader-hand route",
                  e->championLeaderHandRemoveAnchor);
    CHECK_REDMCSB(result.noSlotWriteOrClear == 1,
                  "rotation does not write or clear C30/chest slots",
                  e->championSlotWriteClearAnchor);
    CHECK_REDMCSB(result.noC159NameRowSideEffect == 1 &&
                      result.c159NameRowDispatchCountAfter == 0 &&
                      result.c159SetLeaderCountAfter == 0,
                  "rotation triggers no C159/name-row side effect",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(result.noG0299GuardedSideEffect == 1 &&
                      result.statusBoxDispatchCountAfter == 0 &&
                      result.spellRuneDispatchCountAfter == 0 &&
                      result.saveDispatchCountAfter == 0,
                  "status-box, spell-rune, and save guards do not fire",
                  e->g0299GuardAnchor);
    CHECK_REDMCSB(result.leaderIndexBefore == result.leaderIndexAfter,
                  "view-only rotation does not switch the leader",
                  e->championPartyDirectionAnchor);
    CHECK_REDMCSB(result.c040PanelStillOpen == 1,
                  "C040 remains the owner after view-only rotation",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.portraitRedrawCountAfter ==
                      result.portraitRedrawCountBefore + 1,
                  "rotation records one portrait refresh",
                  e->panelPortraitRedrawAnchor);
}

static void test_left_click_wraps_within_visible_candidates(void)
{
    Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat state;
    Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat result;
    const Dm1V1MirrorCandidateLeftClickRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateLeftClickRotation_EvidencePc34Compat();
    int ok;

    DM1_V1_MirrorCandidateLeftClickRotation_InitPc34Compat(&state);
    state.visibleCandidateIndex = 3;
    state.g0299CandidateChampionOrdinal = state.visibleCandidateOrdinals[3];

    ok = DM1_V1_MirrorCandidateLeftClickRotation_ApplyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(ok == 1 && result.visibleCandidateIndexBefore == 3,
                  "wrap fixture starts at the final visible candidate",
                  e->defsRosterAnchor);
    CHECK_REDMCSB(result.candidateAdvancedByOne == 1 &&
                      result.visibleCandidateIndexAfter == 0,
                  "left-click wraps from last candidate to first candidate",
                  e->defsRosterAnchor);
    CHECK_REDMCSB(result.candidateOrdinalBefore == 14u &&
                      result.candidateOrdinalAfter == 11u,
                  "wrapped rotation swaps only the viewed candidate ordinal",
                  e->defsRosterAnchor);
    CHECK_REDMCSB(result.rotationViewOnly == 1 &&
                      result.noLeaderHandSwap == 1 &&
                      result.noC159NameRowSideEffect == 1,
                  "wrapped rotation remains view-only",
                  e->nonOverlapNote);
}

static void test_non_left_click_does_not_rotate(void)
{
    Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat state;
    Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat result;
    const Dm1V1MirrorCandidateLeftClickRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateLeftClickRotation_EvidencePc34Compat();
    int ok;

    DM1_V1_MirrorCandidateLeftClickRotation_InitPc34Compat(&state);
    ok = DM1_V1_MirrorCandidateLeftClickRotation_ApplyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_MOUSE_RIGHT_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(ok == 0,
                  "right-click is outside the left-click rotation contract",
                  e->nonOverlapNote);
    CHECK_REDMCSB(result.eventDispatchedToMirrorCandidateHandler == 0 &&
                      result.rotationDispatchCountAfter ==
                          result.rotationDispatchCountBefore,
                  "right-click does not dispatch the rotation handler",
                  e->commandClickRoutingAnchor);
    CHECK_REDMCSB(result.visibleCandidateIndexBefore ==
                      result.visibleCandidateIndexAfter &&
                      result.candidateOrdinalBefore ==
                          result.candidateOrdinalAfter,
                  "right-click leaves the visible candidate unchanged",
                  e->nonOverlapNote);
    CHECK_REDMCSB(result.noLeaderHandSwap == 1 &&
                      result.noC159NameRowSideEffect == 1 &&
                      result.noG0299GuardedSideEffect == 1,
                  "right-click negative control has no side effects",
                  e->nonOverlapNote);
}

static void test_closed_panel_blocks_left_click_rotation(void)
{
    Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat state;
    Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat result;
    const Dm1V1MirrorCandidateLeftClickRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateLeftClickRotation_EvidencePc34Compat();
    int ok;

    DM1_V1_MirrorCandidateLeftClickRotation_InitPc34Compat(&state);
    state.c040PanelOpen = 0;

    ok = DM1_V1_MirrorCandidateLeftClickRotation_ApplyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(ok == 0,
                  "closed C040 blocks the left-click rotation hand-off",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.leftClickDispatchCountAfter ==
                      result.leftClickDispatchCountBefore &&
                      result.mirrorCandidateHandlerCountAfter ==
                          result.mirrorCandidateHandlerCountBefore,
                  "closed panel reaches no mirror-candidate handler",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.candidateOrdinalBefore ==
                      result.candidateOrdinalAfter &&
                      result.g0299Before == result.g0299After,
                  "closed panel leaves candidate state unchanged",
                  e->commandPanelDispatchAnchor);
}

int main(void)
{
    test_source_lock_metadata();
    test_left_click_rotates_candidate_view_only();
    test_left_click_wraps_within_visible_candidates();
    test_non_left_click_does_not_rotate();
    test_closed_panel_blocks_left_click_rotation();

    printf("PASS dm1_v1_mirror_candidate_left_click_rotation_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
