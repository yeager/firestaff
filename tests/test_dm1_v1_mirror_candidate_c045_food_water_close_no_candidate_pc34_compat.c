#include "firestaff/dm1/v1/mirror/dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_pc34_compat.h"

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
        printf("FAIL %s actual=%d expected=%d [%s]\n",
               message, actual, expected, anchor ? anchor : "(null)");
    }
}

static void check_u16_eq(uint16_t actual, uint16_t expected,
                         const char *message, const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=0x%04x expected=0x%04x [%s]\n",
               message, actual, expected, anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s [%s]\n",
               message, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static void test_evidence(void)
{
    const Dm1V1MirrorCandidateC045FoodWaterCloseEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_evidence_pc34();
    const char *text =
        dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_source_evidence_pc34();

    check_true(e != NULL, "evidence accessor", "CHEST.C F0333:30-67");
    check_contains(e->chestOpenAnchor, "F0333:30-67", "F0333 anchor",
                   e->chestOpenAnchor);
    check_contains(e->chestCloseAnchor, "F0334:113-132", "F0334 anchor",
                   e->chestCloseAnchor);
    check_contains(e->championAnchor, "F0297:243-298", "F0297 anchor",
                   e->championAnchor);
    check_contains(e->championAnchor, "F0298:270-298", "F0298 anchor",
                   e->championAnchor);
    check_contains(e->championAnchor, "F0300:511-515", "F0300 anchor",
                   e->championAnchor);
    check_contains(e->championAnchor, "F0301:606-614", "F0301 anchor",
                   e->championAnchor);
    check_contains(e->championAnchor, "F0302:662-714", "F0302 anchor",
                   e->championAnchor);
    check_contains(e->panelFoodWaterAnchor, "F0344:1493-1561",
                   "F0344 anchor", e->panelFoodWaterAnchor);
    check_contains(e->panelFoodWaterAnchor, "F0345:1563-1616",
                   "F0345 anchor", e->panelFoodWaterAnchor);
    check_contains(e->panelCloseAnchor, "F0354:2299-2352",
                   "F0354 anchor", e->panelCloseAnchor);
    check_contains(e->reviveOpenAnchor, "F0280:124-132", "F0280 anchor",
                   e->reviveOpenAnchor);
    check_contains(e->reviveC040Anchor, "F0282:744-806", "F0282 anchor",
                   e->reviveC040Anchor);
    check_contains(e->commandAnchor, "F0359:1985-1990", "F0359 anchor",
                   e->commandAnchor);
    check_contains(e->defsAnchor, "C040", "C040 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C045", "C045 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "M565", "M565 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "M568", "M568 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "G0425", "G0425 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "G0426", "G0426 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "M070", "M070 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "M516", "M516 defs", e->defsAnchor);
    check_contains(e->disjointness, "mirror_candidate_c040_*",
                   "disjoint c040", e->disjointness);
    check_contains(e->disjointness, "resurrect_*", "disjoint resurrect",
                   e->disjointness);
    check_contains(e->disjointness, "c545_*", "disjoint c545",
                   e->disjointness);
    check_contains(e->disjointness, "chest_close_*", "disjoint chest close",
                   e->disjointness);
    check_contains(e->disjointness, "full_chain_*", "disjoint full chain",
                   e->disjointness);
    check_contains(e->disjointness, "no_pending_resurrect_*",
                   "disjoint no pending", e->disjointness);
    check_contains(e->disjointness, "open_then_reselect_*",
                   "disjoint reselect", e->disjointness);
    check_contains(e->disjointness, "scroll_pickup_*",
                   "disjoint scroll pickup", e->disjointness);
    check_contains(text, "CHEST.C F0333:30-67", "source F0333", text);
    check_contains(text, "CHEST.C F0334:113-132", "source F0334", text);
    check_contains(text, "CHAMPION.C F0297:243-298", "source F0297", text);
    check_contains(text, "F0298:270-298", "source F0298", text);
    check_contains(text, "F0300:511-515", "source F0300", text);
    check_contains(text, "F0301:606-614", "source F0301", text);
    check_contains(text, "F0302:662-714", "source F0302", text);
    check_contains(text, "PANEL.C F0344:1493-1561", "source F0344", text);
    check_contains(text, "F0345:1563-1616", "source F0345", text);
    check_contains(text, "PANEL.C F0354:2299-2352", "source F0354", text);
    check_contains(text, "REVIVE.C F0280:124-132", "source F0280", text);
    check_contains(text, "F0282:744-806", "source F0282", text);
    check_contains(text, "COMMAND.C F0359:1985-1990", "source F0359", text);
    check_contains(text, "DEFS.H:2200 C040", "source C040", text);
    check_contains(text, "2205 C045", "source C045", text);
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat state;
    int i;

    dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_init_pc34(&state);
    check_int_eq(state.contractOnly, 1, "contract only", "asset-free");
    check_int_eq(state.inventoryChampionOrdinal, 1, "inventory ordinal",
                 "G0423");
    check_int_eq(state.leaderIndex, 0, "leader index", "M516");
    check_int_eq(state.sourceChestSlotIndex, 2, "source C30 slot", "C30");
    check_u16_eq(state.openChestThing, 0x6420u, "open chest", "G0426");
    check_u16_eq(state.sourceChestThing, 0x7330u, "source chest",
                 "CHEST.C F0333");
    check_u16_eq(state.foodThing, 0x0451u, "food thing", "C045");
    check_u16_eq(state.waterThing, 0x0452u, "water thing", "C045");
    check_u16_eq(state.sourceChestChain[2], state.foodThing,
                 "food in source chain", "CHEST.C F0333:30-67");
    check_u16_eq(state.g0425Slots[2], state.foodThing,
                 "food in G0425", "G0425");
    check_u16_eq(state.g0425Slots[5], state.waterThing,
                 "water in G0425", "PANEL.C F0345");
    check_u16_eq(state.championSwitchC30Thing, DM1_V1_MC_C045_FW_NONE_PC34,
                 "no transient C30", "CHAMPION.C F0300");
    check_int_eq(state.championSwitchSourceSlot, -1, "no source slot",
                 "CHAMPION.C F0301");
    check_int_eq(state.c018CloseCommand, 18, "C018 close command",
                 "C503/C018");
    check_int_eq(state.panelContent, 6, "initial chest panel", "M569");
    check_int_eq(state.panelGraphic, 25, "open chest graphic",
                 "CHEST.C F0333");
    check_int_eq(state.panelOpen, 1, "panel open", "G0426");
    check_int_eq(state.c040ResurrectPendingOrdinal, 4,
                 "resurrect pending held", "REVIVE.C F0280");
    check_int_eq(state.c040PanelOpened, 0, "C040 not opened", "C040");
    check_int_eq(state.f0282Entered, 0, "F0282 not entered",
                 "REVIVE.C F0282");
    check_int_eq(state.f0280CandidateGateChecked, 1, "F0280 gate",
                 "REVIVE.C F0280");
    check_int_eq(state.f0333OpenCount, 1, "F0333 open count",
                 "CHEST.C F0333");
    check_int_eq(state.f0334CloseCount, 0, "no close yet", "CHEST.C F0334");
    check_int_eq(state.preservedFoodBeforeClose, 1536, "food preserved seed",
                 "PANEL.C F0345");
    check_int_eq(state.preservedWaterBeforeClose, 2048,
                 "water preserved seed", "PANEL.C F0345");
    check_true(state.preHash != 0u, "pre hash nonzero", "determinism");
    for (i = 0; i < DM1_V1_MC_C045_FW_SLOT_COUNT_PC34; ++i) {
        check_u16_eq(state.sourceChestChain[i], state.g0425Slots[i],
                     "source/G0425 initial mirror", "CHEST.C F0333");
    }
}

static uint32_t test_run(void)
{
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat state;
    Dm1V1MirrorCandidateC045FoodWaterCloseResultPc34Compat result;
    int ok;
    int i;

    dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_init_pc34(&state);
    ok = dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_run_pc34(
        &state, &result);
    check_int_eq(ok, 1, "run accepted", "COMMAND.C F0359");
    check_int_eq(result.accepted, 1, "result accepted", "contract");
    check_int_eq(result.openedByC144Eye, 1, "opened by eye C144",
                 "CHEST.C F0333:30-67");
    check_int_eq(result.openedC045FoodWater, 1, "opened C045 food/water",
                 "PANEL.C F0345");
    check_int_eq(result.noC040OnOpen, 1, "no C040 on open",
                 "REVIVE.C F0280/F0282");
    check_int_eq(result.noF0282OnC045Close, 1, "no F0282 on C045 close",
                 "REVIVE.C F0282:744-806");
    check_int_eq(result.closeFromChestBoundState, 1,
                 "closed from chest-bound state", "CHEST.C F0334");
    check_int_eq(result.closeCommandC503C018, 1, "C503/C018 close",
                 "PANEL.C F0354");
    check_int_eq(result.releasedC30BackToSourceSlot, 1,
                 "released transient C30", "CHAMPION.C F0301");
    check_int_eq(result.sourceChainRestored, 1, "source chain restored",
                 "CHEST.C F0334");
    check_int_eq(result.foodWaterPanelStatePreserved, 1,
                 "food/water panel preserved", "PANEL.C F0344/F0345");
    check_int_eq(result.consumptionReadPreservedFood, 1,
                 "food consumption read preserved data", "PANEL.C F0345");
    check_int_eq(result.consumptionReadPreservedWater, 1,
                 "water value preserved", "PANEL.C F0345");
    check_int_eq(result.foodAfterConsumption, 1152, "food after consume",
                 "PANEL.C F0344");
    check_int_eq(result.waterAfterConsumption, 2048, "water after consume",
                 "PANEL.C F0344");
    check_int_eq(result.c040PendingStillPending, 1, "C040 pending retained",
                 "REVIVE.C F0280");
    check_int_eq(result.c040PanelStillClosed, 1, "C040 panel stayed closed",
                 "C040");
    check_int_eq(result.chestClosedBeforeFoodWaterDraw, 1,
                 "chest close before food/water draw", "PANEL.C F0345");
    check_int_eq(result.championSwitchSlotWasTransient, 1,
                 "transient champion switch C30", "CHAMPION.C F0300/F0301");
    check_int_eq(result.disjointFromC040CandidatePath, 1,
                 "disjoint from C040 path", "COMMAND.C F0359");
    check_int_eq(result.guardRejectsInvalidChest, 1, "invalid chest guard",
                 "CHEST.C F0333");
    check_int_eq(result.guardRejectsNoFoodThing, 1, "no food guard",
                 "PANEL.C F0345");
    check_int_eq(result.guardRejectsWrongPanel, 1, "wrong panel guard",
                 "M568");
    check_u16_eq(result.restoredThing, 0x0451u, "restored food thing",
                 "CHAMPION.C F0301");
    check_u16_eq(result.restoredChain[2], 0x0451u, "restored chain food",
                 "CHEST.C F0334");
    check_u16_eq(result.g0425AfterClose[2], 0x0451u, "G0425 restored food",
                 "G0425");
    check_u16_eq(result.g0425AfterClose[5], 0x0452u, "G0425 water stable",
                 "G0425");
    check_int_eq(state.c144EyeDispatches, 1, "C144 dispatch count",
                 "C144");
    check_int_eq(state.c503CloseDispatches, 1, "C503 dispatch count",
                 "C503");
    check_int_eq(state.c018CloseCommand, 18, "C018 retained", "C018");
    check_u16_eq(state.openChestThing, DM1_V1_MC_C045_FW_NONE_PC34,
                 "G0426 closed", "CHEST.C F0334");
    check_u16_eq(state.championSwitchC30Thing, DM1_V1_MC_C045_FW_NONE_PC34,
                 "transient C30 released", "CHAMPION.C F0301");
    check_int_eq(state.championSwitchSourceSlot, -1, "source slot cleared",
                 "CHAMPION.C F0301");
    check_int_eq(state.panelOpen, 0, "C045 closed", "PANEL.C F0354");
    check_int_eq(state.panelContent, 0, "panel content cleared",
                 "PANEL.C F0354");
    check_int_eq(state.panelGraphic, 0, "panel graphic cleared",
                 "PANEL.C F0354");
    check_int_eq(state.c040PanelOpened, 0, "C040 never opened", "C040");
    check_int_eq(state.f0282Entered, 0, "F0282 count zero",
                 "REVIVE.C F0282");
    check_int_eq(state.f0359C040DispatchCount, 0, "C040 dispatch count zero",
                 "COMMAND.C F0359");
    check_int_eq(state.f0334CloseCount, 1, "F0334 close count",
                 "CHEST.C F0334");
    check_int_eq(state.f0300RemoveSlotCount, 1, "F0300 remove count",
                 "CHAMPION.C F0300");
    check_int_eq(state.f0301AddSlotCount, 1, "F0301 add count",
                 "CHAMPION.C F0301");
    check_int_eq(state.f0345FoodWaterDrawCount, 1, "F0345 draw count",
                 "PANEL.C F0345");
    check_int_eq(state.f0344BarReadCount, 2, "F0344 reads both bars",
                 "PANEL.C F0344");
    check_int_eq(state.f0354CloseCount, 1, "F0354 close count",
                 "PANEL.C F0354");
    check_int_eq(state.f0297PutLeaderHandCount, 1, "F0297 consume count",
                 "CHAMPION.C F0297");
    check_int_eq(state.f0298RemoveLeaderHandCount, 1, "F0298 consume count",
                 "CHAMPION.C F0298");
    check_int_eq(state.closeReleasedC30ToChestChain, 1,
                 "release flag", "CHAMPION.C F0301");
    check_int_eq(state.preservedFoodBeforeClose, 1536,
                 "food before close preserved", "PANEL.C F0345");
    check_int_eq(state.preservedWaterBeforeClose, 2048,
                 "water before close preserved", "PANEL.C F0345");
    check_int_eq(state.consumedFoodAfterClose, 1152,
                 "food consumed from preserved panel", "PANEL.C F0344");
    check_int_eq(state.consumedWaterAfterClose, 2048,
                 "water unchanged after food consume", "PANEL.C F0344");
    check_true(state.preHash != 0u, "pre hash", "determinism");
    check_true(state.openHash != 0u, "open hash", "determinism");
    check_true(state.closeHash != 0u, "close hash", "determinism");
    check_true(state.consumeHash != 0u, "consume hash", "determinism");
    check_true(state.preHash != state.openHash, "open hash changes",
               "determinism");
    check_true(state.openHash != state.closeHash, "close hash changes",
               "determinism");
    check_true(state.closeHash != state.consumeHash, "consume hash changes",
               "determinism");
    check_true(result.hash != 0u, "result hash nonzero", "determinism");
    for (i = 0; i < DM1_V1_MC_C045_FW_SLOT_COUNT_PC34; ++i) {
        check_u16_eq(result.restoredChain[i], result.g0425AfterClose[i],
                     "restored chain mirrors G0425", "CHEST.C F0334");
    }
    return result.hash;
}

static void test_rejects(void)
{
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat state;
    Dm1V1MirrorCandidateC045FoodWaterCloseResultPc34Compat result;

    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_run_pc34(
            NULL, &result),
        0, "null state rejected", "guard");
    dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_init_pc34(&state);
    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_run_pc34(
            &state, NULL),
        0, "null result rejected", "guard");
    dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_init_pc34(&state);
    state.contractOnly = 0;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_run_pc34(
            &state, &result),
        0, "non-contract rejected", "asset-free");
    dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_init_pc34(&state);
    state.openChestThing = DM1_V1_MC_C045_FW_NONE_PC34;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_run_pc34(
            &state, &result),
        0, "closed chest rejected", "CHEST.C F0333");
    dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_init_pc34(&state);
    state.g0425Slots[2] = DM1_V1_MC_C045_FW_NONE_PC34;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_run_pc34(
            &state, &result),
        0, "missing food rejected", "PANEL.C F0345");
    dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_init_pc34(&state);
    state.c040PanelOpened = 1;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_run_pc34(
            &state, &result),
        0, "pre-open C040 rejected", "REVIVE.C F0282");
}

int main(void)
{
    uint32_t hash;

    printf("probe=dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_pc34_compat\n");
    printf("%s\n",
           dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_source_evidence_pc34());
    test_evidence();
    test_initial_state();
    hash = test_run();
    test_rejects();
    if (g_failures || g_assertions < 80) {
        printf("FAIL assertions=%d failures=%d hash=0x%08x\n",
               g_assertions, g_failures, hash);
        return 1;
    }
    printf("DM1_V1_MIRROR_CANDIDATE_C045_FOOD_WATER_CLOSE_NO_CANDIDATE_PC34_COMPAT_OK assertions=%d failures=0 hash=0x%08x\n",
           g_assertions, hash);
    return 0;
}
