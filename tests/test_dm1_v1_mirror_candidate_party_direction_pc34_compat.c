#include "dm1_v1_mirror_candidate_party_direction_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* Chosen non-overlap scenario:
 * C040 resurrect/reincarnate panel is live (G0299 candidate owner), the party
 * receives five turn inputs that normally change G0308, then a champion
 * status-name click attempts to reach G0455 C159/C016 set-leader routing.
 * The gate proves the pending candidate, C040 panel pixels, and synthetic
 * C127 portrait identity are preserved; it does not press C160/C161/C162,
 * browse candidates by keyboard, cancel by click/deadzone, refresh icons,
 * open inventory/chest, or cast spells.
 *
 * ReDMCSB source-lock anchors:
 * COMMAND.C F0361/F0380:1709-1806,2045-2162; C160/C161/C162 tables:
 * 228-240,508-512; C007/C100/C111/C140/C145 G0299 gates:
 * 2180-2182,2302-2311,2336-2368; G0455/C159 rows:202-215,484-496.
 * CHAMPION.C F0297/F0300/F0301:243-268,489-585,587-625 and
 * CLIKCHAM.C F0368:51-72 for leader direction/redraw behavior.
 * CHAMPION.C F0331 time-effects guard:2333-2335.
 * DUNGEON.C:2608-2612 and DUNVIEW.C:3913-3928 for C127 portrait routing.
 * REVIVE.C F0280:124-132,176-182,272-276 and F0282:744-806.
 * PANEL.C F0346:1619-1635 for the C040 panel blit.
 */

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

static void test_fixture_starts_with_live_c040_candidate_panel(void)
{
    Dm1V1MirrorCandidatePartyDirectionStatePc34Compat state;
    const Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartyDirection_EvidencePc34Compat();

    DM1_V1_MirrorCandidatePartyDirection_InitPc34Compat(&state);

    CHECK_REDMCSB(state.candidateChampionOrdinal == 3u,
                  "fixture starts with G0299 candidate ordinal live",
                  e->g0299Anchor);
    CHECK_REDMCSB(state.panelContent ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_M568_PANEL_PC34_COMPAT &&
                      state.c040PanelOpen == 1,
                  "fixture starts with M568/C040 panel ownership",
                  e->revivePanelAnchor);
    CHECK_REDMCSB(state.inventoryChampionOrdinal == 3,
                  "candidate owns the active panel inventory ordinal",
                  e->revivePanelAnchor);
    CHECK_REDMCSB(state.candidateIdentityAnchor == 0x0420u,
                  "candidate identity anchor is initialized",
                  e->g0420Anchor);
    CHECK_REDMCSB(state.syntheticC127PortraitToken == 0xC1270420u,
                  "portrait identity is a synthetic C127 token",
                  e->dunviewPortraitAnchor);
    CHECK_REDMCSB(state.realAssetPortraitParityClaimed == 0,
                  "fixture rejects real-asset portrait parity",
                  e->contractScope);
}

static void test_c040_panel_pixels_use_real_c10_transparency_contract(void)
{
    Dm1V1MirrorCandidatePartyDirectionStatePc34Compat state;
    Dm1V1MirrorCandidatePartyDirectionResultPc34Compat result;
    const Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartyDirection_EvidencePc34Compat();

    DM1_V1_MirrorCandidatePartyDirection_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidatePartyDirection_RunFiveTurnC159ScenarioPc34Compat(
        &state, &result);

    CHECK_REDMCSB(state.panelSource[0] ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_C10_TRANSPARENT_PC34_COMPAT,
                  "C040 source contains a C10 transparent panel pixel",
                  e->panelPixelAnchor);
    CHECK_REDMCSB(result.c10TransparentPixelPreserved == 1,
                  "C10 transparent pixel preserves destination seed",
                  e->panelPixelAnchor);
    CHECK_REDMCSB(state.panelSource[1] ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_C040_GRAPHIC_PC34_COMPAT,
                  "C040 source contains an opaque panel marker pixel",
                  e->panelPixelAnchor);
    CHECK_REDMCSB(result.c040OpaquePanelPixelCopied == 1,
                  "opaque C040 panel pixel is copied to the destination",
                  e->panelPixelAnchor);
    CHECK_REDMCSB(result.panelPixelContractReal == 1,
                  "panel gate is backed by pixel copy evidence",
                  e->panelPixelAnchor);
}

static void test_five_turns_change_party_direction_but_keep_candidate_owner(void)
{
    Dm1V1MirrorCandidatePartyDirectionStatePc34Compat state;
    Dm1V1MirrorCandidatePartyDirectionResultPc34Compat result;
    int returned;
    const Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartyDirection_EvidencePc34Compat();

    DM1_V1_MirrorCandidatePartyDirection_InitPc34Compat(&state);
    returned =
        DM1_V1_MirrorCandidatePartyDirection_RunFiveTurnC159ScenarioPc34Compat(
            &state, &result);

    CHECK_REDMCSB(returned == 1,
                  "combined party-direction/C159 scenario returns preserved",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(result.commandQueuedCount == 6 &&
                      result.commandDequeuedCount == 6,
                  "five turns plus one C012 status click are queued/dequeued",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(result.turnDispatchCount == 5,
                  "all five party-direction inputs dispatch as turns",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(result.partyDirectionBefore == 0 &&
                      result.partyDirectionAfter == 3,
                  "turn sequence changes G0308 party direction",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(result.candidateDirectionBefore == 0 &&
                      result.candidateDirectionAfter == 0,
                  "blocked C159 route does not restamp candidate direction",
                  e->championLeaderAnchor);
    CHECK_REDMCSB(result.g0299AnchorPreserved == 1,
                  "G0299 candidate ordinal survives party-direction input",
                  e->g0299Anchor);
    CHECK_REDMCSB(result.g0420IdentityPreserved == 1,
                  "G0420 candidate identity marker survives input",
                  e->g0420Anchor);
    CHECK_REDMCSB(result.panelOwnerPreserved == 1,
                  "C040 panel owner remains the pending candidate",
                  e->revivePanelAnchor);
}

static void test_c159_name_click_is_blocked_before_nested_g0455_route(void)
{
    Dm1V1MirrorCandidatePartyDirectionStatePc34Compat state;
    Dm1V1MirrorCandidatePartyDirectionResultPc34Compat result;
    const Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartyDirection_EvidencePc34Compat();

    DM1_V1_MirrorCandidatePartyDirection_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidatePartyDirection_RunFiveTurnC159ScenarioPc34Compat(
        &state, &result);

    CHECK_REDMCSB(result.statusCommandDequeued ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_C012_PC34_COMPAT,
                  "status-box command is the outer C012 route",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(result.statusGateBlockedByG0299 == 1,
                  "G0299 blocks F0367 before C159 can be scanned",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(result.c159NestedReachedBefore == 0 &&
                      result.c159NestedReachedAfter == 0,
                  "nested G0455 C159/C016 route is not reached",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(result.f0367StatusDispatchCountBefore == 0 &&
                      result.f0367StatusDispatchCountAfter == 0,
                  "F0367 status-box dispatcher is not called",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(result.f0368SetLeaderCountBefore == 0 &&
                      result.f0368SetLeaderCountAfter == 0,
                  "F0368 set-leader path is not called",
                  e->championLeaderAnchor);
    CHECK_REDMCSB(result.leaderIndexBefore == 0 &&
                      result.leaderIndexAfter == 0,
                  "leader index is preserved",
                  e->championLeaderAnchor);
}

static void test_no_duplicate_no_panel_command_no_guard_side_effects(void)
{
    Dm1V1MirrorCandidatePartyDirectionStatePc34Compat state;
    Dm1V1MirrorCandidatePartyDirectionResultPc34Compat result;
    const Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartyDirection_EvidencePc34Compat();

    DM1_V1_MirrorCandidatePartyDirection_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidatePartyDirection_RunFiveTurnC159ScenarioPc34Compat(
        &state, &result);

    CHECK_REDMCSB(result.noDuplicateCandidate == 1,
                  "scenario does not append a duplicate candidate",
                  e->revivePanelAnchor);
    CHECK_REDMCSB(result.partyChampionCountBefore == 3u &&
                      result.partyChampionCountAfter == 3u,
                  "party champion count remains stable",
                  e->revivePanelAnchor);
    CHECK_REDMCSB(result.resurrectDispatchCountBefore == 0 &&
                      result.resurrectDispatchCountAfter == 0,
                  "C160 resurrect is not triggered",
                  e->revivePanelAnchor);
    CHECK_REDMCSB(result.reincarnateDispatchCountBefore == 0 &&
                      result.reincarnateDispatchCountAfter == 0,
                  "C161 reincarnate is not triggered",
                  e->revivePanelAnchor);
    CHECK_REDMCSB(result.cancelDispatchCountBefore == 0 &&
                      result.cancelDispatchCountAfter == 0,
                  "C162 cancel is not triggered",
                  e->revivePanelAnchor);
    CHECK_REDMCSB(result.restDispatchCountBefore == 0 &&
                      result.restDispatchCountAfter == 0,
                  "C145 rest guard has no side effect in this scenario",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(result.saveDispatchCountBefore == 0 &&
                      result.saveDispatchCountAfter == 0,
                  "C140 save guard has no side effect in this scenario",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(result.portraitContractOnly == 1,
                  "portrait identity remains deterministic contract-only",
                  e->contractScope);
}

static void test_source_lock_and_non_overlap_metadata(void)
{
    const Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartyDirection_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "evidence metadata is available",
                  "metadata");
    CHECK_REDMCSB(strstr(e->commandRouteAnchor, "1709-1806") != NULL &&
                      strstr(e->commandRouteAnchor, "2158-2162") != NULL,
                  "evidence cites COMMAND.C keyboard/queue/status gates",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(strstr(e->commandRouteAnchor, "2180-2182") != NULL &&
                      strstr(e->commandRouteAnchor, "2302-2311") != NULL &&
                      strstr(e->commandRouteAnchor, "2336-2368") != NULL,
                  "evidence cites inventory/spell/rest/save sibling gates",
                  e->commandRouteAnchor);
    CHECK_REDMCSB(strstr(e->championLeaderAnchor, "F0297") != NULL &&
                      strstr(e->championLeaderAnchor, "F0300") != NULL &&
                      strstr(e->championLeaderAnchor, "F0301") != NULL,
                  "evidence cites champion object/weight paths",
                  e->championLeaderAnchor);
    CHECK_REDMCSB(strstr(e->championLeaderAnchor, "F0331") != NULL,
                  "evidence cites CHAMPION.C F0331 guard",
                  e->championLeaderAnchor);
    CHECK_REDMCSB(strstr(e->dunviewPortraitAnchor, "DUNVIEW.C") != NULL &&
                      strstr(e->dunviewPortraitAnchor, "3913-3928") != NULL,
                  "evidence cites DUNVIEW.C portrait draw route",
                  e->dunviewPortraitAnchor);
    CHECK_REDMCSB(strstr(e->revivePanelAnchor, "F0280") != NULL &&
                      strstr(e->revivePanelAnchor, "F0282") != NULL,
                  "evidence cites REVIVE.C add and panel paths",
                  e->revivePanelAnchor);
    CHECK_REDMCSB(strstr(e->panelPixelAnchor, "C040") != NULL &&
                      strstr(e->panelPixelAnchor, "1619-1635") != NULL,
                  "evidence cites C040 panel blit",
                  e->panelPixelAnchor);
    CHECK_REDMCSB(strstr(e->nonOverlapNote, "five turn") != NULL &&
                      strstr(e->nonOverlapNote, "does not browse") != NULL,
                  "evidence records non-overlap with browse/click gates",
                  e->nonOverlapNote);
    CHECK_REDMCSB(strstr(e->contractScope, "synthetic C127") != NULL &&
                      strstr(e->contractScope, "no real-asset portrait parity") != NULL,
                  "contract scope rejects real-asset portrait parity",
                  e->contractScope);
}

int main(void)
{
    test_fixture_starts_with_live_c040_candidate_panel();
    test_c040_panel_pixels_use_real_c10_transparency_contract();
    test_five_turns_change_party_direction_but_keep_candidate_owner();
    test_c159_name_click_is_blocked_before_nested_g0455_route();
    test_no_duplicate_no_panel_command_no_guard_side_effects();
    test_source_lock_and_non_overlap_metadata();

    printf("PASS dm1_v1_mirror_candidate_party_direction_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
