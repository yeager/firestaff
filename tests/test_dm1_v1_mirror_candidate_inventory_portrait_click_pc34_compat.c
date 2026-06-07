#include "dm1_v1_mirror_candidate_inventory_portrait_click_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* ReDMCSB source-lock anchors tested here:
 * COMMAND.C F0380:2159-2181 !G0299 dispatch guard.
 * COMMAND.C:484-488 G0455 C159..C162 champion name-row route.
 * DEFS.H:338-340 panel commands C160/C161/C162 and DEFS.H:3787-3790
 * C159/C160/C161/C162 champion-name zones; local DEFS.H:4041-4042 is not
 * the C160/C161/C162 framing requested by the pass note.
 * PANEL.C F0354:2208-2240 C175+champion portrait-box rectangle.
 * CHAMDRAW.C F0292:810-812 inventory portrait dispatch to F0354.
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

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateInventoryPortraitClick_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL, "evidence metadata is available",
                  "COMMAND.C F0380:2159-2181");
    CHECK_REDMCSB(strstr(e->commandGuardAnchor, "2159-2181") != NULL,
                  "COMMAND.C !G0299 guard anchor is present",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(strstr(e->c159NameRouteAnchor, "484-488") != NULL,
                  "COMMAND.C C159 G0455 anchor is present",
                  e->c159NameRouteAnchor);
    CHECK_REDMCSB(strstr(e->defsPanelCommandAnchor, "338-340") != NULL,
                  "actual local DEFS.H C160/C161/C162 command anchor is present",
                  e->defsPanelCommandAnchor);
    CHECK_REDMCSB(strstr(e->defsPanelCommandAnchor, "4041-4042") != NULL &&
                      strstr(e->defsPanelCommandAnchor, "viewport") != NULL,
                  "mismatched requested DEFS.H:4041-4042 citation is documented",
                  e->defsPanelCommandAnchor);
    CHECK_REDMCSB(strstr(e->defsNameZoneAnchor, "3787-3790") != NULL,
                  "DEFS.H C159..C162 name-zone anchor is present",
                  e->defsNameZoneAnchor);
    CHECK_REDMCSB(strstr(e->defsNameZoneAnchor, "3793") != NULL,
                  "DEFS.H C175 portrait-zone anchor is present",
                  e->defsNameZoneAnchor);
    CHECK_REDMCSB(strstr(e->panelPortraitBoxAnchor, "2208-2240") != NULL,
                  "PANEL.C F0354 portrait-box anchor is present",
                  e->panelPortraitBoxAnchor);
    CHECK_REDMCSB(strstr(e->chamdrawPortraitDispatchAnchor, "810-812") != NULL,
                  "CHAMDRAW.C F0292 portrait dispatch anchor is present",
                  e->chamdrawPortraitDispatchAnchor);
    CHECK_REDMCSB(strstr(e->contractScope, "contract-only") != NULL,
                  "contract scope rejects real asset claims",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->nonOverlapNote, "C175+0") != NULL &&
                      strstr(e->nonOverlapNote, "C159") != NULL,
                  "non-overlap evidence names C175 and C159",
                  e->nonOverlapNote);
}

static void test_inventory_open_then_c040_open_fixture(void)
{
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat state;
    const Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateInventoryPortraitClick_EvidencePc34Compat();
    int openedInventory;
    int openedPanel;

    DM1_V1_MirrorCandidateInventoryPortraitClick_InitPc34Compat(&state);
    openedInventory =
        DM1_V1_MirrorCandidateInventoryPortraitClick_OpenInventoryPc34Compat(
            &state, 1u);
    openedPanel =
        DM1_V1_MirrorCandidateInventoryPortraitClick_OpenC040FromResurrectClickPc34Compat(
            &state, 2u);

    CHECK_REDMCSB(openedInventory == 1,
                  "inventory opens for champion-0 before C040",
                  e->chamdrawPortraitDispatchAnchor);
    CHECK_REDMCSB(state.inventoryChampionOrdinal == 1u,
                  "G0423-style inventory leader ordinal is champion-0",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(state.inventoryOpenCount == 1,
                  "inventory open count records one open",
                  e->chamdrawPortraitDispatchAnchor);
    CHECK_REDMCSB(openedPanel == 1,
                  "resurrect click opens C040 candidate panel",
                  e->defsPanelCommandAnchor);
    CHECK_REDMCSB(state.c040PanelOpen == 1,
                  "C040 panel is live after resurrect click",
                  e->defsPanelCommandAnchor);
    CHECK_REDMCSB(state.c040PanelGraphic ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C040_PANEL_GRAPHIC_PC34_COMPAT,
                  "C040 panel graphic identifier is recorded",
                  e->defsPanelCommandAnchor);
    CHECK_REDMCSB(state.candidateChampionOrdinal == 2u,
                  "G0299 pending candidate ordinal is published",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(state.resurrectClickOpenCount == 1,
                  "resurrect click open count records one C040 publish",
                  e->defsPanelCommandAnchor);
    CHECK_REDMCSB(state.leaderIndex == 0,
                  "active leader starts as champion-0",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(state.leaderHandThingOrdinal == 0x4d2,
                  "inventory leader-hand item fixture is non-empty",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(state.frontD1cMirrorChampionOrdinal == 1,
                  "front mirror route starts on champion-0 ordinal",
                  e->panelPortraitBoxAnchor);
}

static void test_c175_portrait_click_ignored_while_c040_live(void)
{
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat state;
    Dm1V1MirrorCandidateInventoryPortraitClickResultPc34Compat result;
    const Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateInventoryPortraitClick_EvidencePc34Compat();
    int returned;

    DM1_V1_MirrorCandidateInventoryPortraitClick_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateInventoryPortraitClick_OpenInventoryPc34Compat(
        &state, 1u);
    (void)DM1_V1_MirrorCandidateInventoryPortraitClick_OpenC040FromResurrectClickPc34Compat(
        &state, 2u);

    returned =
        DM1_V1_MirrorCandidateInventoryPortraitClick_ProcessPortraitZonePc34Compat(
            &state,
            DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C175_FIRST_PORTRAIT_ZONE_PC34_COMPAT,
            &result);

    CHECK_REDMCSB(returned == 0,
                  "C175+0 portrait click returns ignored while G0299 is set",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.requestedZone ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C175_FIRST_PORTRAIT_ZONE_PC34_COMPAT,
                  "requested zone is C175+0 portrait-box",
                  e->panelPortraitBoxAnchor);
    CHECK_REDMCSB(result.requestedChampionIndex == 0,
                  "C175+0 resolves to champion-0 portrait",
                  e->panelPortraitBoxAnchor);
    CHECK_REDMCSB(result.isPortraitZone == 1,
                  "click is classified as a portrait-box zone",
                  e->panelPortraitBoxAnchor);
    CHECK_REDMCSB(result.isChampionZeroPortraitZone == 1,
                  "click is specifically champion-0 portrait-box",
                  e->panelPortraitBoxAnchor);
    CHECK_REDMCSB(result.isC159NameZone == 0,
                  "portrait-box click is not the C159 name zone",
                  e->c159NameRouteAnchor);
    CHECK_REDMCSB(result.c159NonOverlap == 1,
                  "C175+0 is explicitly non-overlapping with C159",
                  e->nonOverlapNote);
    CHECK_REDMCSB(result.commandGuardChecked == 1,
                  "COMMAND.C !G0299 guard is checked",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.rejectedByG0299 == 1,
                  "G0299 rejects the portrait click",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.ignored == 1,
                  "portrait click records ignored outcome",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.acceptedLeaderSwitch == 0,
                  "rejected portrait click does not switch leader",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.inventoryOpenBefore == 1 &&
                      result.inventoryOpenAfter == 1,
                  "inventory remains open around ignored click",
                  e->chamdrawPortraitDispatchAnchor);
    CHECK_REDMCSB(result.c040PanelOpenBefore == 1 &&
                      result.c040PanelOpenAfter == 1,
                  "C040 panel remains open around ignored click",
                  e->defsPanelCommandAnchor);
    CHECK_REDMCSB(result.candidateOrdinalBefore == 2u &&
                      result.candidateOrdinalAfter == 2u,
                  "pending C040 candidate identity is preserved",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.inventoryOrdinalBefore == 1u &&
                      result.inventoryOrdinalAfter == 1u,
                  "inventory leader ordinal is preserved",
                  e->chamdrawPortraitDispatchAnchor);
    CHECK_REDMCSB(result.leaderIndexBefore == 0 &&
                      result.leaderIndexAfter == 0,
                  "active leader stays champion-0",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.leaderHandThingBefore == 0x4d2 &&
                      result.leaderHandThingAfter == 0x4d2,
                  "inventory leader-hand item is preserved",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.frontMirrorOrdinalBefore == 1 &&
                      result.frontMirrorOrdinalAfter == 1,
                  "front mirror route ordinal is preserved",
                  e->panelPortraitBoxAnchor);
    CHECK_REDMCSB(result.portraitClickRejectCountAfter ==
                      result.portraitClickRejectCountBefore + 1,
                  "portrait rejection count increments once",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.portraitClickAcceptCountAfter ==
                      result.portraitClickAcceptCountBefore,
                  "portrait accept count does not increment",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.c159NameRouteCountAfter ==
                      result.c159NameRouteCountBefore,
                  "C159 route count is untouched by C175 portrait click",
                  e->c159NameRouteAnchor);
    CHECK_REDMCSB(result.commandGuardRejectCountAfter ==
                      result.commandGuardRejectCountBefore + 1,
                  "command guard rejection count increments once",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.leaderPreserved == 1,
                  "result reports preserved leader",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.candidatePreserved == 1,
                  "result reports preserved candidate",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.inventoryLeaderPreserved == 1,
                  "result reports preserved inventory leader",
                  e->chamdrawPortraitDispatchAnchor);
    CHECK_REDMCSB(result.leaderHandPreserved == 1,
                  "result reports preserved leader hand",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.mirrorRoutePreserved == 1,
                  "result reports preserved mirror route",
                  e->panelPortraitBoxAnchor);
    CHECK_REDMCSB(state.leaderIndex == 0,
                  "state leader remains champion-0 after ignored click",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(state.candidateChampionOrdinal == 2u,
                  "state G0299 remains on pending candidate",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(state.inventoryChampionOrdinal == 1u,
                  "state inventory ordinal remains champion-0",
                  e->chamdrawPortraitDispatchAnchor);
    CHECK_REDMCSB(state.leaderHandThingOrdinal == 0x4d2,
                  "state leader-hand item remains unchanged",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(state.frontD1cMirrorChampionOrdinal == 1,
                  "state front mirror route remains champion-0 ordinal",
                  e->panelPortraitBoxAnchor);
    CHECK_REDMCSB(state.portraitClickRejectCount == 1,
                  "state has one portrait rejection",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(state.portraitClickAcceptCount == 0,
                  "state has zero portrait accepts",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(state.c159NameRouteCount == 0,
                  "state never enters C159 name-route",
                  e->c159NameRouteAnchor);
}

static void test_c159_non_overlap_zone_is_distinct(void)
{
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat state;
    Dm1V1MirrorCandidateInventoryPortraitClickResultPc34Compat result;
    const Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateInventoryPortraitClick_EvidencePc34Compat();
    int returned;

    DM1_V1_MirrorCandidateInventoryPortraitClick_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateInventoryPortraitClick_OpenInventoryPc34Compat(
        &state, 1u);
    (void)DM1_V1_MirrorCandidateInventoryPortraitClick_OpenC040FromResurrectClickPc34Compat(
        &state, 2u);
    returned =
        DM1_V1_MirrorCandidateInventoryPortraitClick_ProcessPortraitZonePc34Compat(
            &state,
            DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C159_NAME_ZONE_0_PC34_COMPAT,
            &result);

    CHECK_REDMCSB(returned == 0,
                  "C159 route sample is not processed as a portrait click",
                  e->c159NameRouteAnchor);
    CHECK_REDMCSB(result.isC159NameZone == 1,
                  "sample zone is C159 champion-name zone",
                  e->c159NameRouteAnchor);
    CHECK_REDMCSB(result.isPortraitZone == 0,
                  "C159 is not a C175 portrait-box zone",
                  e->panelPortraitBoxAnchor);
    CHECK_REDMCSB(result.isChampionZeroPortraitZone == 0,
                  "C159 does not identify champion-0 portrait-box",
                  e->nonOverlapNote);
    CHECK_REDMCSB(result.c159NonOverlap == 0,
                  "C159 sample cannot claim C175+0 non-overlap flag",
                  e->nonOverlapNote);
    CHECK_REDMCSB(result.requestedChampionIndex ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_NONE_PC34_COMPAT,
                  "C159 sample has no portrait champion index",
                  e->c159NameRouteAnchor);
    CHECK_REDMCSB(result.c159NameRouteCountAfter ==
                      result.c159NameRouteCountBefore + 1,
                  "C159 route count increments only for C159 sample",
                  e->c159NameRouteAnchor);
    CHECK_REDMCSB(result.portraitClickRejectCountAfter ==
                      result.portraitClickRejectCountBefore,
                  "C159 sample does not increment portrait rejection count",
                  e->panelPortraitBoxAnchor);
    CHECK_REDMCSB(state.c159NameRouteCount == 1,
                  "state records the C159 route separately",
                  e->c159NameRouteAnchor);
    CHECK_REDMCSB(state.portraitClickRejectCount == 0,
                  "state records no portrait rejection for C159 route",
                  e->nonOverlapNote);
}

static void test_c040_closed_portrait_click_switches_leader(void)
{
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat state;
    Dm1V1MirrorCandidateInventoryPortraitClickResultPc34Compat result;
    const Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateInventoryPortraitClick_EvidencePc34Compat();
    int returned;

    DM1_V1_MirrorCandidateInventoryPortraitClick_InitPc34Compat(&state);
    state.leaderIndex = 1;
    state.frontD1cMirrorChampionOrdinal = 2;
    (void)DM1_V1_MirrorCandidateInventoryPortraitClick_OpenInventoryPc34Compat(
        &state, 2u);

    returned =
        DM1_V1_MirrorCandidateInventoryPortraitClick_ProcessPortraitZonePc34Compat(
            &state,
            DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C175_FIRST_PORTRAIT_ZONE_PC34_COMPAT,
            &result);

    CHECK_REDMCSB(returned == 1,
                  "C040-closed baseline allows C175+0 portrait click",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.c040PanelOpenBefore == 0 &&
                      result.c040PanelOpenAfter == 0,
                  "baseline starts and ends with C040 closed",
                  e->defsPanelCommandAnchor);
    CHECK_REDMCSB(result.candidateOrdinalBefore == 0u &&
                      result.candidateOrdinalAfter == 0u,
                  "baseline has no G0299 pending candidate",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.commandGuardChecked == 1,
                  "baseline still checks the command guard",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.rejectedByG0299 == 0,
                  "baseline is not rejected by G0299",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.acceptedLeaderSwitch == 1,
                  "baseline accepts leader switch",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.leaderIndexBefore == 1 &&
                      result.leaderIndexAfter == 0,
                  "baseline switches from champion-1 to champion-0",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(state.leaderIndex == 0,
                  "state leader is champion-0 after baseline click",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.inventoryOrdinalBefore == 2u &&
                      result.inventoryOrdinalAfter == 2u,
                  "baseline preserves already-open inventory ordinal",
                  e->chamdrawPortraitDispatchAnchor);
    CHECK_REDMCSB(result.leaderHandThingBefore == 0x4d2 &&
                      result.leaderHandThingAfter == 0x4d2,
                  "baseline preserves leader-hand item",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.frontMirrorOrdinalBefore == 2 &&
                      result.frontMirrorOrdinalAfter == 2,
                  "baseline does not rewrite mirror route ordinal",
                  e->panelPortraitBoxAnchor);
    CHECK_REDMCSB(result.portraitClickAcceptCountAfter ==
                      result.portraitClickAcceptCountBefore + 1,
                  "baseline portrait accept count increments once",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.portraitClickRejectCountAfter ==
                      result.portraitClickRejectCountBefore,
                  "baseline portrait reject count does not increment",
                  e->commandGuardAnchor);
}

static void test_run_entrypoint(void)
{
    int passed = 0;
    int failed = 0;
    int ok = dm1_v1_mirror_candidate_inventory_portrait_click_run(
        &passed, &failed);

    CHECK_REDMCSB(ok == 1,
                  "run entrypoint self-check succeeds",
                  "COMMAND.C F0380:2159-2181");
    CHECK_REDMCSB(failed == 0,
                  "run entrypoint reports zero failed assertions",
                  "COMMAND.C F0380:2159-2181");
    CHECK_REDMCSB(passed >= 28,
                  "run entrypoint reports its assertion count",
                  "COMMAND.C F0380:2159-2181");
}

int main(void)
{
    test_source_lock_metadata();
    test_inventory_open_then_c040_open_fixture();
    test_c175_portrait_click_ignored_while_c040_live();
    test_c159_non_overlap_zone_is_distinct();
    test_c040_closed_portrait_click_switches_leader();
    test_run_entrypoint();

    printf("PASS dm1_v1_mirror_candidate_inventory_portrait_click_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
