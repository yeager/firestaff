#include "firestaff/dm1/v1/mirror_candidate/c040_panel_browse_pickup_rotate_race_pc34_compat.h"

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
    const Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceEvidencePc34 *e =
        dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_evidence_pc34();
    const char *text =
        dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_source_evidence_pc34();

    check_true(e != NULL, "evidence accessor", "source-lock");
    check_contains(e->chestOpenAnchor, "CHEST.C F0333:30-67",
                   "chest open anchor", e->chestOpenAnchor);
    check_contains(e->chestOpenAnchor, "G0426", "G0426 open anchor",
                   e->chestOpenAnchor);
    check_contains(e->chestOpenAnchor, "G0425", "G0425 open anchor",
                   e->chestOpenAnchor);
    check_contains(e->chestCloseAnchor, "CHEST.C F0334:113-132",
                   "chest close anchor", e->chestCloseAnchor);
    check_contains(e->chestCloseAnchor, "untouched",
                   "chest close untouched claim", e->chestCloseAnchor);
    check_contains(e->championChainAnchor, "CHAMPION.C F0297/F0298:243-298",
                   "leader hand chain anchor", e->championChainAnchor);
    check_contains(e->championChainAnchor, "F0300/F0301/F0302:511-714",
                   "slot chain combined anchor", e->championChainAnchor);
    check_contains(e->championSlotAnchor, "F0300:511-515",
                   "F0300 slot clear", e->championSlotAnchor);
    check_contains(e->championSlotAnchor, "F0301:606-614",
                   "F0301 slot write", e->championSlotAnchor);
    check_contains(e->championSlotAnchor, "F0302:662-714",
                   "F0302 slot dispatch", e->championSlotAnchor);
    check_contains(e->panelAnchor, "PANEL.C F0346/F0347:1624-1657",
                   "C040 panel priority", e->panelAnchor);
    check_contains(e->panelAnchor, "F0354:2195-2242",
                   "status portrait redraw", e->panelAnchor);
    check_contains(e->reviveAnchor, "REVIVE.C F0280:124-132",
                   "candidate publish", e->reviveAnchor);
    check_contains(e->reviveAnchor, "F0282:744-806",
                   "candidate clear path", e->reviveAnchor);
    check_contains(e->commandClickAnchor, "COMMAND.C F0359:1452-1668",
                   "click queue", e->commandClickAnchor);
    check_contains(e->commandClickAnchor, "F0378:1956-1993",
                   "panel route", e->commandClickAnchor);
    check_contains(e->commandQueueAnchor, "F0361:1709-1813",
                   "queue write", e->commandQueueAnchor);
    check_contains(e->commandQueueAnchor, "F0380:2045-2178",
                   "queue dispatch", e->commandQueueAnchor);
    check_contains(e->commandLeaderAnchor, "CLIKCHAM.C F0367/F0368:20-73",
                   "leader identity", e->commandLeaderAnchor);
    check_contains(e->mouseWheelAnchor, "F0077:97-126",
                   "wheel queue write lineage", e->mouseWheelAnchor);
    check_contains(e->mouseWheelAnchor, "F0078:128-168",
                   "wheel queue read lineage", e->mouseWheelAnchor);
    check_contains(e->mouseWheelAnchor, "DEFS.H:6886-6895",
                   "local F0077/F0078 prototypes", e->mouseWheelAnchor);
    check_contains(e->defsAnchor, "DEFS.H:2088", "C10 defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C016..C065", "command defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C160..C162", "panel command defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "G0420/G0423/G0425/G0426",
                   "global defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C030", "C030 slot defs", e->defsAnchor);

    check_contains(text, "CHEST.C F0333:30-67",
                   "source text chest open", text);
    check_contains(text, "CHEST.C F0334:113-132",
                   "source text chest close", text);
    check_contains(text, "F0302:662-714", "source text F0302", text);
    check_contains(text, "PANEL.C F0346/F0347:1624-1657",
                   "source text panel priority", text);
    check_contains(text, "REVIVE.C F0280:124-132",
                   "source text F0280", text);
    check_contains(text, "F0282:744-806", "source text F0282", text);
    check_contains(text, "F0378:1956-1993", "source text F0378", text);
    check_contains(text, "F0361:1709-1813", "source text F0361", text);
    check_contains(text, "F0380:2045-2178", "source text F0380", text);
    check_contains(text, "CLIKCHAM.C F0367/F0368:20-73",
                   "source text CLIKCHAM", text);
    check_contains(text, "G0425/G0426", "source text globals", text);
}

static void test_non_overlap(void)
{
    const Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceEvidencePc34 *e =
        dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_evidence_pc34();
    const char *siblings[] = {
        "mirror_candidate_click_cancel_with_rotation",
        "click_cancel",
        "resurrect_confirmation",
        "rotation_during_resurrect_confirmation",
        "c159_click_rotation_combo",
        "c040_chrome_inventory_owner_swap",
        "c040_redraw_after_chest_close",
        "c040_close_non_leader_scroll_pickup",
        "c045_close_after_non_candidate_transition",
        "c045_food_water_close_no_candidate",
        "c545_pickup_while_panel_live",
        "c545_drop_while_panel_live",
        "chest_close_leader_hand_pickup",
        "chest_close_pending_panel",
        "chest_open_during_pending",
        "inventory_click_during_rotation",
        "keyboard_rotation_combo",
        "left_click_rotation",
        "save_load",
        "teleporter_survival",
        "scroll_pickup_leader_rotation_inventory_click",
        "scroll_pickup_with_party_rotate_in_progress",
        "scroll_pickup_non_leader_panel_live",
        "pending_hand_during_chest_pickup_race",
        "pending_hand_queue"
    };
    int i;

    check_contains(e->nonOverlap, "live C040 panel browse",
                   "non-overlap names fresh panel browse race",
                   e->nonOverlap);
    check_contains(e->nonOverlap, "same-window wheel leader rotation",
                   "non-overlap names wheel rotation", e->nonOverlap);
    check_contains(e->nonOverlap, "chest-slot click rejected by C040 routing",
                   "non-overlap names rejected chest click", e->nonOverlap);
    for (i = 0; i < (int)(sizeof(siblings) / sizeof(siblings[0])); ++i) {
        char message[128];

        snprintf(message, sizeof(message), "non-overlap excludes %s",
                 siblings[i]);
        check_contains(e->nonOverlap, siblings[i], message, e->nonOverlap);
    }
    printf("NON_OVERLAP %s\n", e->nonOverlap);
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 state;
    int i;

    dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_init_pc34(
        &state);
    check_int_eq(state.partyChampionCount, 4, "party count",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.leaderIndex, 0, "initial leader",
                 "CLIKCHAM.C F0368:51-73");
    check_int_eq(state.pendingLeaderIndex, 1, "queued leader",
                 "COMMAND.C F0380:2045-2178");
    check_int_eq(state.inventoryChampionOrdinal, 1, "inventory champion",
                 "DEFS.H G0423");
    check_int_eq(state.c040PanelOpen, 1, "C040 panel open",
                 "PANEL.C F0346/F0347:1624-1657");
    check_int_eq(state.panelContent, 568, "M568 panel content",
                 "COMMAND.C F0378:1956-1993");
    check_int_eq(state.panelGraphic, 40, "C040 panel graphic",
                 "DEFS.H C040");
    check_int_eq(state.candidateOwnerIndex, 0, "candidate owner",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.candidateChainIndex, 1, "candidate chain index",
                 "COMMAND.C F0361:1709-1813");
    check_int_eq(state.candidateChainCount, 3, "candidate chain count",
                 "CHAMPION.C F0297/F0298");
    check_int_eq(state.candidateChainOrdinals[0], 310,
                 "candidate chain ordinal 0", "REVIVE.C F0280");
    check_int_eq(state.candidateChainOrdinals[1], 311,
                 "candidate chain ordinal 1", "REVIVE.C F0280");
    check_int_eq(state.candidateChainOrdinals[2], 312,
                 "candidate chain ordinal 2", "REVIVE.C F0280");
    check_int_eq(state.g0299CandidateOrdinal, 311,
                 "G0299 points at browsed candidate", "G0299");
    check_int_eq(state.selectedCandidateOrdinal, 311,
                 "selected candidate matches G0299", "G0299");
    check_int_eq(state.leaderHandThing, 0xffff, "leader hand empty thing",
                 "CHAMPION.C F0297/F0298");
    check_int_eq(state.leaderHandEmpty, 1, "leader hand empty flag",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.openChestThing, 0x6c40, "open chest thing sentinel",
                 "CHEST.C F0333:30-67");
    check_int_eq(state.g0426OpenChest, 0x6c40, "G0426 open chest",
                 "DEFS.H G0426");
    check_int_eq(state.f0280CandidatePublishCount, 1,
                 "one F0280 candidate publish", "REVIVE.C F0280");
    check_int_eq(state.f0333OpenCount, 1, "one F0333 open",
                 "CHEST.C F0333");
    check_int_eq(state.f0346C040DrawCount, 1, "one C040 draw",
                 "PANEL.C F0346");
    check_int_eq(state.f0347CandidatePriorityCount, 1,
                 "F0347 candidate priority", "PANEL.C F0347:1654-1657");
    check_int_eq(state.trace[0], 100, "initial trace marker",
                 "runtime trace");
    check_u32_nonzero(state.chainHash, "initial chain hash", "determinism");
    check_u32_nonzero(state.chestHash, "initial chest hash", "determinism");
    check_u32_nonzero(state.stateHash, "initial state hash", "determinism");

    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_CHEST_SLOT_COUNT_PC34; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "initial chest slot C%d",
                 537 + i);
        check_int_eq(state.chestSlots[i], 0x7400 + i, label,
                     "CHEST.C F0333:30-67");
    }
    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_PARTY_COUNT_PC34; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "champion %d ordinal", i);
        check_int_eq(state.champions[i].championOrdinal, i + 1, label,
                     "M516");
        snprintf(label, sizeof(label), "champion %d alive", i);
        check_int_eq(state.champions[i].alive, 1, label, "M516");
        snprintf(label, sizeof(label), "champion %d load", i);
        check_int_eq(state.champions[i].load, 20 + i, label,
                     "CLIKCHAM.C F0368:55-69");
    }
    check_int_eq(state.champions[0].leader, 1, "champion 0 leader",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.champions[1].leader, 0, "champion 1 not leader yet",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.champions[0].c040ChainLinked, 1,
                 "old leader owns C040 chain", "REVIVE.C F0280");
    check_int_eq(state.champions[1].c040ChainLinked, 0,
                 "queued leader does not own C040 chain",
                 "CHAMPION.C F0302");
}

static uint32_t run_one(
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceResultPc34 *result)
{
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 state;
    int accepted;
    int i;

    dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_init_pc34(
        &state);
    accepted =
        dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_run_pc34(
            &state, result);
    check_int_eq(accepted, 1, "runtime accepted",
                 "pass768 runtime regression");
    check_int_eq(result->accepted, 1, "result accepted",
                 "pass768 runtime regression");
    check_int_eq(result->sameTickWindow, 1, "same tick window",
                 "COMMAND.C F0359/F0361");
    check_int_eq(result->initialLeaderIndex, 0, "initial leader",
                 "CLIKCHAM.C F0368");
    check_int_eq(result->finalLeaderIndex, 1, "final leader",
                 "CLIKCHAM.C F0368");
    check_int_eq(result->pendingLeaderIndexBefore, 1,
                 "pending leader before", "COMMAND.C F0380");
    check_int_eq(result->pendingLeaderIndexAfter, -1,
                 "pending leader consumed", "COMMAND.C F0380");
    check_int_eq(result->c040PanelOpenBefore, 1, "C040 before",
                 "PANEL.C F0346/F0347");
    check_int_eq(result->c040PanelOpenAfter, 1, "C040 after",
                 "PANEL.C F0346/F0347");
    check_int_eq(result->panelContentBefore, 568, "panel M568 before",
                 "COMMAND.C F0378");
    check_int_eq(result->panelContentAfter, 568, "panel M568 after",
                 "COMMAND.C F0378");
    check_int_eq(result->panelGraphicBefore, 40, "panel C040 before",
                 "DEFS.H C040");
    check_int_eq(result->panelGraphicAfter, 40, "panel C040 after",
                 "DEFS.H C040");
    check_int_eq(result->candidateOwnerBefore, 0, "owner before",
                 "REVIVE.C F0280");
    check_int_eq(result->candidateOwnerAfter, 0, "owner after",
                 "REVIVE.C F0280");
    check_int_eq(result->candidateIndexBefore, 1, "candidate index before",
                 "COMMAND.C F0361");
    check_int_eq(result->candidateIndexAfter, 1, "candidate index after",
                 "COMMAND.C F0361");
    check_int_eq(result->g0299Before, 311, "G0299 before", "G0299");
    check_int_eq(result->g0299After, 311, "G0299 after", "G0299");
    check_int_eq(result->selectedCandidateBefore, 311,
                 "selected candidate before", "G0299");
    check_int_eq(result->selectedCandidateAfter, 311,
                 "selected candidate after", "G0299");
    check_int_eq(result->g0426Before, 0x6c40, "G0426 before",
                 "CHEST.C F0333");
    check_int_eq(result->g0426After, 0x6c40, "G0426 after",
                 "CHEST.C F0333");
    check_int_eq(result->openChestThingBefore, 0x6c40,
                 "open chest before", "CHEST.C F0333");
    check_int_eq(result->openChestThingAfter, 0x6c40,
                 "open chest after", "CHEST.C F0333");
    check_int_eq(result->wheelQueuedByF0077, 1, "wheel queued",
                 "MOUSE.C F0077:97-126");
    check_int_eq(result->wheelReadByF0078, 1, "wheel read",
                 "MOUSE.C F0078:128-168");
    check_int_eq(result->wheelQueueDepthAfterRead, 0,
                 "wheel queue drained", "COMMAND.C F0380");
    check_int_eq(result->f0361QueueWriteCount, 1, "F0361 queue write",
                 "COMMAND.C F0361:1709-1813");
    check_int_eq(result->f0380DispatchCount, 1, "F0380 dispatch",
                 "COMMAND.C F0380:2045-2178");
    check_int_eq(result->f0368SetLeaderCount, 1, "F0368 leader set",
                 "CLIKCHAM.C F0368:51-73");
    check_int_eq(result->f0359PanelClickCount, 1, "F0359 panel click",
                 "COMMAND.C F0359:1452-1668");
    check_int_eq(result->f0378PanelRouteCount, 1, "F0378 panel route",
                 "COMMAND.C F0378:1956-1993");
    check_int_eq(result->f0302ChestPickupCount, 0, "F0302 pickup skipped",
                 "CHAMPION.C F0302:662-714");
    check_int_eq(result->f0334CloseCount, 0, "F0334 close skipped",
                 "CHEST.C F0334:113-132");
    check_int_eq(result->f0282CandidateClearCount, 0,
                 "F0282 candidate clear skipped", "REVIVE.C F0282:744-806");
    check_int_eq(result->c040RouteRejectedChestPickup, 1,
                 "C040 route rejected chest pickup",
                 "PANEL.C F0346/F0347:1624-1657");
    check_int_eq(result->chestStatePreserved, 1, "chest state preserved",
                 "CHEST.C F0333/F0334");
    check_int_eq(result->candidateStatePreserved, 1,
                 "candidate state preserved", "REVIVE.C F0280");
    check_int_eq(result->championChainPreserved, 1,
                 "champion chain preserved", "CHAMPION.C F0297/F0298");
    check_int_eq(result->candidateIndexPreserved, 1,
                 "candidate index preserved", "COMMAND.C F0361");
    check_int_eq(result->selectedCandidatePreserved, 1,
                 "selected candidate preserved", "G0299");
    check_int_eq(result->g0426Preserved, 1, "G0426 preserved",
                 "DEFS.H G0426");
    check_int_eq(result->panelStayedC040, 1, "panel stayed C040",
                 "PANEL.C F0346/F0347");
    check_int_eq(result->leaderRotationConsumed, 1,
                 "leader rotation consumed", "COMMAND.C F0380");
    check_int_eq(result->noChestClose, 1, "no chest close",
                 "CHEST.C F0334");
    check_int_eq(result->noCandidateClear, 1, "no candidate clear",
                 "REVIVE.C F0282");
    check_int_eq(result->noSaveLoadTeleporterResurrectCommit, 1,
                 "no save/load/teleporter/resurrect commit", "non-overlap");
    check_int_eq(result->sourceLockAnchorsPresent, 1,
                 "source-lock anchors present", "ReDMCSB");

    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_CHEST_SLOT_COUNT_PC34; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "before chest slot C%d", 537 + i);
        check_int_eq(result->chestSlotsBefore[i], 0x7400 + i, label,
                     "CHEST.C F0333:30-67");
        snprintf(label, sizeof(label), "after chest slot C%d", 537 + i);
        check_int_eq(result->chestSlotsAfter[i], 0x7400 + i, label,
                     "CHEST.C F0333:30-67");
        snprintf(label, sizeof(label), "stable chest slot C%d", 537 + i);
        check_int_eq(result->chestSlotsAfter[i], result->chestSlotsBefore[i],
                     label, "CHEST.C F0334:113-132");
    }

    check_int_eq(result->trace[0], 100, "trace init", "runtime trace");
    check_int_eq(result->trace[1], 101, "trace queue wheel",
                 "MOUSE.C F0077");
    check_int_eq(result->trace[2], 102, "trace queue click",
                 "COMMAND.C F0359");
    check_int_eq(result->trace[3], 103, "trace rotate",
                 "COMMAND.C F0380");
    check_int_eq(result->trace[4], 104, "trace C040 reject",
                 "COMMAND.C F0378");
    check_int_eq(result->trace[5], 105, "trace stable",
                 "CHEST.C/REVIVE.C invariants");

    check_u32_nonzero(result->chainHashBefore, "chain hash before",
                      "determinism");
    check_u32_nonzero(result->chainHashAfter, "chain hash after",
                      "determinism");
    check_u32_nonzero(result->chestHashBefore, "chest hash before",
                      "determinism");
    check_u32_nonzero(result->chestHashAfter, "chest hash after",
                      "determinism");
    check_u32_nonzero(result->beforeHash, "before hash", "determinism");
    check_u32_nonzero(result->afterQueueHash, "after queue hash",
                      "determinism");
    check_u32_nonzero(result->afterRotationHash, "after rotation hash",
                      "determinism");
    check_u32_nonzero(result->afterClickHash, "after click hash",
                      "determinism");
    check_u32_nonzero(result->deterministicHash, "deterministic hash",
                      "determinism");
    check_u32_eq(result->chainHashAfter, result->chainHashBefore,
                 "chain hash stable", "CHAMPION.C F0297/F0298");
    check_u32_eq(result->chestHashAfter, result->chestHashBefore,
                 "chest hash stable", "CHEST.C F0333/F0334");
    check_true(result->beforeHash != result->afterQueueHash,
               "queue mutates runtime hash", "COMMAND.C F0361");
    check_true(result->afterQueueHash != result->afterRotationHash,
               "rotation mutates runtime hash", "COMMAND.C F0380");
    check_true(result->afterRotationHash != result->afterClickHash,
               "C040 route reject mutates runtime hash", "COMMAND.C F0378");

    check_int_eq(state.leaderIndex, 1, "state leader after run",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.pendingLeaderIndex, -1, "state pending leader clear",
                 "COMMAND.C F0380");
    check_int_eq(state.g0299CandidateOrdinal, 311, "state G0299 kept",
                 "REVIVE.C F0280/F0282");
    check_int_eq(state.selectedCandidateOrdinal, 311,
                 "state selected candidate kept", "COMMAND.C F0361");
    check_int_eq(state.g0426OpenChest, 0x6c40, "state G0426 kept",
                 "CHEST.C F0333");
    check_int_eq(state.chestSlots[1], 0x7401, "state C538 kept",
                 "CHEST.C F0333/F0334");
    check_int_eq(state.leaderHandEmpty, 1, "leader hand still empty",
                 "CHAMPION.C F0302");
    check_int_eq(state.f0302ChestPickupCount, 0,
                 "state F0302 count still zero", "CHAMPION.C F0302");
    return result->deterministicHash;
}

static void test_null_guards(void)
{
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 state;
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceResultPc34 result;

    dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_init_pc34(
        &state);
    check_int_eq(
        dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_run_pc34(
            NULL, &result),
        0, "null state rejected", "guard");
    check_int_eq(
        dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_run_pc34(
            &state, NULL),
        0, "null result rejected", "guard");
    state.c040PanelOpen = 0;
    check_int_eq(
        dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_run_pc34(
            &state, &result),
        0, "missing C040 panel rejected", "non-overlap");
}

int main(void)
{
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceResultPc34 first;
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceResultPc34 second;
    uint32_t firstHash;
    uint32_t secondHash;

    test_source_evidence();
    test_non_overlap();
    test_initial_state();
    firstHash = run_one(&first);
    secondHash = run_one(&second);
    check_u32_eq(secondHash, firstHash, "two-run deterministic hash stable",
                 "determinism");
    test_null_guards();

    if (g_failures || g_assertions < 150) {
        printf("FAIL test_dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_pc34_compat assertions=%d failures=%d hash=0x%08x rerun=0x%08x\n",
               g_assertions, g_failures, firstHash, secondHash);
        return 1;
    }
    printf("PASS test_dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_pc34_compat assertions=%d failures=0 hash=0x%08x rerun=0x%08x\n",
           g_assertions, firstHash, secondHash);
    return 0;
}
