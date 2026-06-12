#include "firestaff/dm1/v1/mirror_candidate/c160_close_while_rotation_pending_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++g_assertions;
    if (!condition) {
        ++g_failures;
        printf("FAIL %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d [%s]\n", message, actual,
               expected, anchor ? anchor : "(null)");
    }
}

static void check_u32_eq(uint32_t actual, uint32_t expected,
                         const char *message, const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=0x%08x expected=0x%08x [%s]\n", message,
               actual, expected, anchor ? anchor : "(null)");
    }
}

static void check_u32_nonzero(uint32_t actual, const char *message,
                              const char *anchor)
{
    ++g_assertions;
    if (actual == 0u) {
        ++g_failures;
        printf("FAIL %s actual=0x%08x [%s]\n", message, actual,
               anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s [%s]\n", message,
               needle ? needle : "(null)", anchor ? anchor : "(null)");
    }
}

static void test_source_evidence(void)
{
    const Dm1V1MirrorCandidateC160CloseWhileRotationPendingEvidencePc34 *e =
        dm1_v1_mirror_candidate_c160_close_while_rotation_pending_evidence_pc34();
    const char *text =
        dm1_v1_mirror_candidate_c160_close_while_rotation_pending_source_evidence_pc34();

    check_true(e != NULL, "evidence accessor", "source-lock");
    check_contains(e->panelFoodWaterAnchor, "PANEL.C F0344:1493-1561",
                   "F0344 food/water anchor", e->panelFoodWaterAnchor);
    check_contains(e->panelFoodWaterAnchor, "F0345:1563-1617",
                   "F0345 food/water anchor", e->panelFoodWaterAnchor);
    check_contains(e->panelResurrectAnchor, "F0346:1619-1637",
                   "F0346 C040 anchor", e->panelResurrectAnchor);
    check_contains(e->panelResurrectAnchor, "F0347:1639-1693",
                   "F0347 C040 anchor", e->panelResurrectAnchor);
    check_contains(e->toggleCandidateGateAnchor, "F0355:2318-2322",
                   "F0355 !G0299 anchor", e->toggleCandidateGateAnchor);
    check_contains(e->toggleCandidateGateAnchor, "bypasses",
                   "F0355 bypass statement", e->toggleCandidateGateAnchor);
    check_contains(e->chestAnchor, "CHEST.C F0333/F0334",
                   "chest source anchor", e->chestAnchor);
    check_contains(e->chestAnchor, "idle", "chest idle contract",
                   e->chestAnchor);
    check_contains(e->championRotationAnchor, "F0297/F0298:243-298",
                   "leader hand anchor", e->championRotationAnchor);
    check_contains(e->championRotationAnchor, "F0300/F0301/F0302:511-714",
                   "F0300/F0301/F0302 anchor", e->championRotationAnchor);
    check_contains(e->commandClickAnchor, "COMMAND.C F0359:1452-1662",
                   "fresh click queue anchor", e->commandClickAnchor);
    check_contains(e->commandClickAnchor, "C160",
                   "fresh click C160 capture", e->commandClickAnchor);
    check_contains(e->commandPanelRouteAnchor, "F0378:1956-1993",
                   "F0378 M568 route", e->commandPanelRouteAnchor);
    check_contains(e->commandPanelRouteAnchor, "M568",
                   "F0378 M568 route label", e->commandPanelRouteAnchor);
    check_contains(e->commandQueueAnchor, "F0360:1692-1707",
                   "F0360 replay idle", e->commandQueueAnchor);
    check_contains(e->commandQueueAnchor, "F0380:2045-2184",
                   "F0380 drain", e->commandQueueAnchor);
    check_contains(e->clickChampionAnchor, "CLIKCHAM.C F0367/F0368:1-73",
                   "leader set/order anchor", e->clickChampionAnchor);
    check_contains(e->clickChampionAnchor, "after close",
                   "leader set after close", e->clickChampionAnchor);
    check_contains(e->defsAnchor, "C160..C162",
                   "C160..C162 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C040/C045/C030",
                   "C040/C045/C030 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "M565/M568",
                   "M565/M568 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C537..C545",
                   "C537..C545 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "G0299", "G0299 defs",
                   e->defsAnchor);
    check_contains(e->closeLandingAnchor,
                   "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
                   "floor/ceiling landing", e->closeLandingAnchor);
    check_contains(e->closeLandingAnchor,
                   "F0326_B_RefreshMousePointerInMainLoop",
                   "mouse pointer landing", e->closeLandingAnchor);

    check_contains(text, "PANEL.C F0344:1493-1561",
                   "source text F0344", text);
    check_contains(text, "F0345:1563-1617", "source text F0345", text);
    check_contains(text, "F0346:1619-1637", "source text F0346", text);
    check_contains(text, "F0347:1639-1693", "source text F0347", text);
    check_contains(text, "F0355:2318-2322", "source text F0355", text);
    check_contains(text, "CHEST.C F0333/F0334", "source text chest",
                   text);
    check_contains(text, "F0302:677-712", "source text F0302", text);
    check_contains(text, "COMMAND.C F0359:1452-1662",
                   "source text F0359", text);
    check_contains(text, "F0360:1692-1707", "source text F0360", text);
    check_contains(text, "F0378:1956-1993", "source text F0378", text);
    check_contains(text, "F0380:2045-2184", "source text F0380", text);
    check_contains(text, "CLIKCHAM.C F0367/F0368:1-73",
                   "source text CLIKCHAM", text);
    check_contains(text, "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
                   "source text F0098", text);
    check_contains(text, "F0326_B_RefreshMousePointerInMainLoop",
                   "source text F0326", text);
}

static void test_non_overlap(void)
{
    const Dm1V1MirrorCandidateC160CloseWhileRotationPendingEvidencePc34 *e =
        dm1_v1_mirror_candidate_c160_close_while_rotation_pending_evidence_pc34();
    const char *siblings[] = {
        "live C040 mirror-candidate",
        "fresh C160 close click",
        "F0302 leader rotation is in-flight",
        "not C045 pending",
        "not C160 close replay",
        "not resurrect confirmation",
        "not chest open/close",
        "not chest pickup/drop",
        "not inventory toggle",
        "not save/load",
        "not teleporter",
        "not party turn/party-rotate"
    };
    int i;

    for (i = 0; i < (int)(sizeof(siblings) / sizeof(siblings[0])); ++i) {
        char message[128];

        snprintf(message, sizeof(message), "non-overlap marker %s",
                 siblings[i]);
        check_contains(e->nonOverlap, siblings[i], message, e->nonOverlap);
    }
    printf("NON_OVERLAP %s\n", e->nonOverlap);
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 state;
    int i;

    dm1_v1_mirror_candidate_c160_close_while_rotation_pending_init_pc34(
        &state, UINT32_C(0x786c160));
    check_int_eq(state.contractOnly, 1, "contract-only flag",
                 "contract-only source-lock");
    check_int_eq(state.noGameDataRequired, 1, "no game data flag",
                 "contract-only source-lock");
    check_int_eq(state.partyChampionCount, 4, "party count",
                 "DEFS.H M516");
    check_int_eq(state.leaderIndex, 0, "initial leader",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.pendingLeaderIndex, 1, "pending leader",
                 "CHAMPION.C F0302");
    check_int_eq(state.inventoryChampionOrdinal, 1, "inventory ordinal",
                 "DEFS.H G0423");
    check_int_eq(state.g0299CandidateOrdinal, 314, "G0299 live",
                 "DEFS.H G0299");
    check_int_eq(state.candidateOwnerIndex, 0, "candidate owner",
                 "PANEL.C F0346/F0347");
    check_int_eq(state.candidateChainIndex, 1, "candidate chain index",
                 "REVIVE.C F0280");
    check_int_eq(state.c040PanelLive, 1, "C040 panel live",
                 "PANEL.C F0346/F0347");
    check_int_eq(state.panelContent, 568, "M568 panel content",
                 "COMMAND.C F0378");
    check_int_eq(state.panelGraphic, 40, "C040 graphic",
                 "DEFS.H C040");
    check_int_eq(state.leaderHandEmpty, 1, "leader hand empty",
                 "COMMAND.C F0378");
    check_int_eq(state.f0346C040DrawCount, 1, "initial F0346 draw",
                 "PANEL.C F0346");
    check_int_eq(state.f0347PanelDrawCount, 1, "initial F0347 draw",
                 "PANEL.C F0347");
    check_int_eq(state.f0302RotationInFlight, 0,
                 "rotation not entered yet", "CHAMPION.C F0302");
    check_int_eq(state.f0302RotationCommitted, 0,
                 "rotation not committed yet", "CLIKCHAM.C F0368");
    check_int_eq(state.commandQueueDepth, 0, "initial queue empty",
                 "COMMAND.C F0380");
    check_int_eq(state.openChestThing, 0x6c60, "open chest sentinel",
                 "CHEST.C F0333");
    check_int_eq(state.g0426OpenChest, 0x6c60, "G0426 stable",
                 "CHEST.C F0333/F0334");
    check_int_eq(state.trace[0], 460, "initial trace",
                 "runtime trace");
    check_u32_nonzero(state.c040PanelPixelHashBefore,
                      "C040 pixel hash before", "stable pixels");
    check_u32_eq(state.c040PanelPixelHashAfterClose,
                 state.c040PanelPixelHashBefore,
                 "C040 pixel hash initially stable", "stable pixels");
    check_u32_nonzero(state.chestHashBefore, "chest hash before",
                      "CHEST.C F0333/F0334");
    check_u32_eq(state.chestHashAfter, state.chestHashBefore,
                 "chest hash initially stable", "CHEST.C F0333/F0334");
    check_u32_nonzero(state.beforeHash, "initial state hash",
                      "determinism");

    for (i = 0; i < DM1_V1_MC_C160_ROTATION_CHEST_SLOT_COUNT_PC34; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "initial chest slot C%d", 537 + i);
        check_int_eq(state.chestSlots[i], 0x7500 + i, label,
                     "DEFS.H C537..C545");
    }
    for (i = 0; i < DM1_V1_MC_C160_ROTATION_PARTY_COUNT_PC34; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "champion %d ordinal", i);
        check_int_eq(state.champions[i].championOrdinal, i + 1, label,
                     "DEFS.H M516");
        snprintf(label, sizeof(label), "champion %d alive", i);
        check_int_eq(state.champions[i].alive, 1, label,
                     "CLIKCHAM.C F0368");
    }
    check_int_eq(state.champions[0].leader, 1, "champion 0 leader",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.champions[1].leader, 0, "champion 1 not leader",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.champions[1].rotationPending, 1,
                 "champion 1 rotation pending", "CHAMPION.C F0302");
    check_int_eq(state.champions[0].c040CandidateOwner, 1,
                 "champion 0 owns C040 candidate", "PANEL.C F0346");
    check_int_eq(state.champions[1].c040CandidateOwner, 0,
                 "queued leader does not own C040 candidate",
                 "PANEL.C F0346");
}

static uint32_t run_one(
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingResultPc34 *result,
    uint32_t seed)
{
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 state;
    int accepted;
    int i;

    dm1_v1_mirror_candidate_c160_close_while_rotation_pending_init_pc34(
        &state, seed);
    accepted =
        dm1_v1_mirror_candidate_c160_close_while_rotation_pending_run_pc34(
            &state, result);
    check_int_eq(accepted, 1, "runtime accepted",
                 "C160 rotation-pending runtime regression");
    check_int_eq(result->accepted, 1, "result accepted",
                 "C160 rotation-pending runtime regression");
    check_int_eq(result->contractOnly, 1, "contract-only result",
                 "contract-only source-lock");
    check_int_eq(result->noGameDataRequired, 1, "no game data result",
                 "contract-only source-lock");
    check_int_eq(result->initialLeaderIndex, 0, "initial leader",
                 "CLIKCHAM.C F0368");
    check_int_eq(result->pendingLeaderIndexBeforeClose, 1,
                 "pending leader before close", "CHAMPION.C F0302");
    check_int_eq(result->leaderIndexDuringClose, 0,
                 "leader unchanged during close", "ordering");
    check_int_eq(result->finalLeaderIndex, 1, "leader after commit",
                 "CLIKCHAM.C F0368");
    check_int_eq(result->pendingLeaderIndexAfterClose, 1,
                 "pending leader survives close", "ordering");
    check_int_eq(result->pendingLeaderIndexAfterCommit, -1,
                 "pending leader consumed after close", "ordering");
    check_int_eq(result->g0299BeforeClose, 314, "G0299 before close",
                 "DEFS.H G0299");
    check_int_eq(result->g0299AfterClose, 0, "G0299 cleared by C160",
                 "COMMAND.C F0378/F0282");
    check_int_eq(result->candidateOwnerBeforeClose, 0,
                 "candidate owner before close", "PANEL.C F0346");
    check_int_eq(result->candidateOwnerAfterClose, 0,
                 "candidate owner stable after close", "PANEL.C F0346");
    check_int_eq(result->c040PanelLiveBeforeClose, 1,
                 "C040 panel live before close", "PANEL.C F0346/F0347");
    check_int_eq(result->c040PanelLiveAfterClose, 0,
                 "C040 panel closed after C160", "COMMAND.C F0378");
    check_int_eq(result->panelContentBeforeClose, 568,
                 "M568 before close", "COMMAND.C F0378");
    check_int_eq(result->panelContentAfterClose, 0,
                 "panel content closed after C160", "COMMAND.C F0378");
    check_int_eq(result->panelGraphicBeforeClose, 40,
                 "C040 graphic before close", "DEFS.H C040");
    check_int_eq(result->panelGraphicAfterClose, 0,
                 "panel graphic closed after C160", "COMMAND.C F0378");
    check_int_eq(result->commandQueueDepthAfterClose, 0,
                 "queue drained after close", "COMMAND.C F0380");
    check_int_eq(result->commandQueueDepthAfterCommit, 0,
                 "queue still drained after commit", "COMMAND.C F0380");
    check_int_eq(result->f0302RotationInFlightBeforeClose, 1,
                 "F0302 in-flight before close", "CHAMPION.C F0302");
    check_int_eq(result->f0302RotationInFlightAfterClose, 1,
                 "F0302 still in-flight after close", "ordering");
    check_int_eq(result->f0302RotationCommittedAfterClose, 0,
                 "rotation not committed before close", "ordering");
    check_int_eq(result->f0302RotationCommittedAfterCommit, 1,
                 "rotation committed after close", "ordering");
    check_int_eq(result->f0282C160ClearCount, 1,
                 "one C160 candidate clear", "F0282 C160");
    check_int_eq(result->f0282NonC160ClearCount, 0,
                 "no non-C160 candidate clear", "F0282");
    check_int_eq(result->f0302EnterCount, 1, "one F0302 entry",
                 "CHAMPION.C F0302");
    check_int_eq(result->f0302LeaderCommitCount, 1,
                 "one F0302 leader commit", "CHAMPION.C F0302");
    check_int_eq(result->f0333OpenCount, 0, "no F0333 open",
                 "CHEST.C F0333");
    check_int_eq(result->f0334CloseCount, 0, "no F0334 close",
                 "CHEST.C F0334");
    check_int_eq(result->f0345FoodWaterDrawCount, 0,
                 "no F0345 food/water draw", "PANEL.C F0345");
    check_int_eq(result->f0346C040DrawCount, 1,
                 "only initial F0346 draw", "PANEL.C F0346");
    check_int_eq(result->f0347PanelDrawCount, 1,
                 "only initial F0347 draw", "PANEL.C F0347");
    check_int_eq(result->f0355ToggleSuppressedByCandidateCount, 0,
                 "F0355 gate did not suppress C160", "PANEL.C F0355");
    check_int_eq(result->f0359FreshClickCount, 1,
                 "fresh C160 click captured", "COMMAND.C F0359");
    check_int_eq(result->f0360PendingReplayCount, 0,
                 "no pending replay", "COMMAND.C F0360");
    check_int_eq(result->f0367StatusBoxClickCount, 1,
                 "leader path after close", "CLIKCHAM.C F0367");
    check_int_eq(result->f0368SetLeaderCount, 1,
                 "leader set after close", "CLIKCHAM.C F0368");
    check_int_eq(result->f0378PanelRouteCount, 1,
                 "one panel route", "COMMAND.C F0378");
    check_int_eq(result->f0380DrainCount, 1,
                 "one queue drain", "COMMAND.C F0380");
    check_int_eq(result->f0098FloorCeilingDrawCount, 1,
                 "floor/ceiling draw", "F0098");
    check_int_eq(result->f0326MousePointerRefreshCount, 1,
                 "mouse pointer refresh", "F0326");
    check_int_eq(result->saveLoadCount, 0, "no save/load",
                 "COMMAND.C F0380");
    check_int_eq(result->teleporterCount, 0, "no teleporter",
                 "non-overlap");
    check_int_eq(result->partyRotateCount, 0, "no party rotate",
                 "non-overlap");
    check_int_eq(result->c040PanelRerenderDuringCloseCount, 0,
                 "no C040 re-render during close", "PANEL.C F0346/F0347");
    check_int_eq(result->c160ClearsG0299DespiteRotationPending, 1,
                 "C160 clears G0299 with rotation pending", "race target");
    check_int_eq(result->closeBypassesF0355CandidateGate, 1,
                 "C160 bypasses F0355 gate", "PANEL.C F0355");
    check_int_eq(result->noC040RerenderDuringClose, 1,
                 "F0346/F0347 not re-run during close", "PANEL.C F0346/F0347");
    check_int_eq(result->rotationCommitsAfterClose, 1,
                 "rotation commits after close", "ordering");
    check_int_eq(result->noChestOpenOrClose, 1,
                 "no chest open or close", "CHEST.C F0333/F0334");
    check_int_eq(result->noExtraCandidateClear, 1,
                 "no extra candidate clear", "F0282");
    check_int_eq(result->noSaveLoadTeleporterPartyRotate, 1,
                 "no unrelated side effects", "COMMAND.C F0380");
    check_int_eq(result->closeLandsInDungeonRefresh, 1,
                 "close lands in dungeon refresh", "F0098/F0326");
    check_int_eq(result->c040PanelPixelsStable, 1,
                 "C040 panel pixels stable", "stable pixels");
    check_int_eq(result->chestStatePreserved, 1,
                 "chest state preserved", "CHEST.C F0333/F0334");
    check_int_eq(result->sourceLockAnchorsPresent, 1,
                 "source-lock anchors present", "ReDMCSB");
    check_int_eq(result->guardRejectsNullState, 1,
                 "null state guard", "guard");
    check_int_eq(result->guardRejectsNullResult, 1,
                 "null result guard", "guard");

    for (i = 0; i < DM1_V1_MC_C160_ROTATION_CHEST_SLOT_COUNT_PC34; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "before chest slot C%d", 537 + i);
        check_int_eq(result->chestSlotsBefore[i], 0x7500 + i, label,
                     "DEFS.H C537..C545");
        snprintf(label, sizeof(label), "after chest slot C%d", 537 + i);
        check_int_eq(result->chestSlotsAfter[i], 0x7500 + i, label,
                     "DEFS.H C537..C545");
        snprintf(label, sizeof(label), "stable chest slot C%d", 537 + i);
        check_int_eq(result->chestSlotsAfter[i], result->chestSlotsBefore[i],
                     label, "CHEST.C F0333/F0334");
    }
    for (i = 0; i < DM1_V1_MC_C160_ROTATION_TRACE_COUNT_PC34; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "trace marker %d", i);
        check_int_eq(result->trace[i], 460 + i, label, "runtime trace");
    }

    check_u32_nonzero(result->c040PanelPixelHashBefore,
                      "C040 pixel hash before", "stable pixels");
    check_u32_eq(result->c040PanelPixelHashAfterClose,
                 result->c040PanelPixelHashBefore,
                 "C040 pixel hash stable after close", "stable pixels");
    check_u32_nonzero(result->chestHashBefore, "chest hash before",
                      "CHEST.C F0333/F0334");
    check_u32_eq(result->chestHashAfter, result->chestHashBefore,
                 "chest hash stable", "CHEST.C F0333/F0334");
    check_u32_nonzero(result->beforeHash, "before hash", "determinism");
    check_u32_nonzero(result->afterF0302PendingHash,
                      "after F0302 pending hash", "determinism");
    check_u32_nonzero(result->afterCloseHash, "after close hash",
                      "determinism");
    check_u32_nonzero(result->afterRotationCommitHash,
                      "after rotation commit hash", "determinism");
    check_u32_nonzero(result->deterministicHash, "deterministic hash",
                      "determinism");
    check_true(result->beforeHash != result->afterF0302PendingHash,
               "F0302 pending mutates hash", "determinism");
    check_true(result->afterF0302PendingHash != result->afterCloseHash,
               "C160 close mutates hash", "determinism");
    check_true(result->afterCloseHash != result->afterRotationCommitHash,
               "post-close rotation commit mutates hash", "determinism");

    check_int_eq(state.leaderIndex, 1, "state final leader",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.pendingLeaderIndex, -1, "state pending clear",
                 "ordering");
    check_int_eq(state.g0299CandidateOrdinal, 0, "state G0299 clear",
                 "F0282 C160");
    check_int_eq(state.f0302RotationInFlight, 0,
                 "state rotation no longer in-flight", "CHAMPION.C F0302");
    check_int_eq(state.f0302RotationCommitted, 1,
                 "state rotation committed", "CLIKCHAM.C F0368");
    check_int_eq(state.c040PanelLive, 0, "state C040 closed",
                 "F0282 C160");
    check_int_eq(state.champions[0].leader, 0, "old leader cleared",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.champions[1].leader, 1, "new leader set",
                 "CLIKCHAM.C F0368");
    return result->deterministicHash;
}

static void test_guards(void)
{
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 state;
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingResultPc34 result;

    dm1_v1_mirror_candidate_c160_close_while_rotation_pending_init_pc34(
        &state, UINT32_C(0x160));
    check_int_eq(
        dm1_v1_mirror_candidate_c160_close_while_rotation_pending_run_pc34(
            NULL, &result),
        0, "null state rejected", "guard");
    check_int_eq(
        dm1_v1_mirror_candidate_c160_close_while_rotation_pending_run_pc34(
            &state, NULL),
        0, "null result rejected", "guard");

    dm1_v1_mirror_candidate_c160_close_while_rotation_pending_init_pc34(
        &state, UINT32_C(0x160));
    state.g0299CandidateOrdinal = 0;
    check_int_eq(
        dm1_v1_mirror_candidate_c160_close_while_rotation_pending_run_pc34(
            &state, &result),
        0, "no candidate rejected", "DEFS.H G0299");

    dm1_v1_mirror_candidate_c160_close_while_rotation_pending_init_pc34(
        &state, UINT32_C(0x160));
    state.pendingLeaderIndex = -1;
    check_int_eq(
        dm1_v1_mirror_candidate_c160_close_while_rotation_pending_run_pc34(
            &state, &result),
        0, "no pending rotation rejected", "CHAMPION.C F0302");

    dm1_v1_mirror_candidate_c160_close_while_rotation_pending_init_pc34(
        &state, UINT32_C(0x160));
    state.panelContent = 565;
    check_int_eq(
        dm1_v1_mirror_candidate_c160_close_while_rotation_pending_run_pc34(
            &state, &result),
        0, "wrong panel rejected", "PANEL.C F0345/F0346");
}

int main(void)
{
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingResultPc34 first;
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingResultPc34 second;
    uint32_t firstHash;
    uint32_t secondHash;

    test_source_evidence();
    test_non_overlap();
    test_initial_state();
    firstHash = run_one(&first, UINT32_C(0x786c160));
    secondHash = run_one(&second, UINT32_C(0x786c160));
    check_u32_eq(secondHash, firstHash, "two-run deterministic hash stable",
                 "determinism");
    test_guards();

    if (g_failures || g_assertions < 150) {
        printf("FAIL test_dm1_v1_mirror_candidate_c160_close_while_rotation_pending_pc34_compat assertions=%d failures=%d hash=0x%08x rerun=0x%08x\n",
               g_assertions, g_failures, firstHash, secondHash);
        return 1;
    }
    printf("PASS test_dm1_v1_mirror_candidate_c160_close_while_rotation_pending_pc34_compat assertions=%d failures=0 hash=0x%08x rerun=0x%08x\n",
           g_assertions, firstHash, secondHash);
    return 0;
}
