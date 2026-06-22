/* ReDMCSB COMMAND.C F0380:2361-2364 dispatches C146_COMMAND_WAKE_UP
 * unconditionally and CHAMPION.C F0314_CHAMPION_WakeUp:1382-1414 owns
 * the wake-up body. The wake-up path clears G0300_B_PartyIsResting,
 * calls F0098_DUNGEONVIEW_DrawFloorAndCeiling, restores G0441..G0444
 * input handlers, calls F0357_COMMAND_DiscardAllInput, and calls
 * F0457_START_DrawEnabledMenus_CPSF. None of these touch G0299 or
 * the C040 panel rectangle. The fixture pins that contract end-to-end.
 */
#include "dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

enum {
    kExpectedHash = 1369256640u
};

typedef struct TestCounters {
    int assertions;
    int failures;
} TestCounters;

static void check_true_pc34_compat(TestCounters *counters, int condition,
                                   const char *message, const char *anchor)
{
    ++counters->assertions;
    if (!condition) {
        ++counters->failures;
        printf("FAIL: %s [%s]\n", message, anchor ? anchor : "(null)");
    }
    assert(condition);
}

static void check_int_eq_pc34_compat(TestCounters *counters, int actual,
                                     int expected, const char *message,
                                     const char *anchor)
{
    ++counters->assertions;
    if (actual != expected) {
        ++counters->failures;
        printf("FAIL: %s actual=%d expected=%d [%s]\n",
               message,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
    assert(actual == expected);
}

static void check_uint_eq_pc34_compat(TestCounters *counters,
                                      unsigned int actual,
                                      unsigned int expected,
                                      const char *message,
                                      const char *anchor)
{
    ++counters->assertions;
    if (actual != expected) {
        ++counters->failures;
        printf("FAIL: %s actual=%u expected=%u [%s]\n",
               message,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
    assert(actual == expected);
}

static void check_contains_pc34_compat(TestCounters *counters,
                                       const char *haystack,
                                       const char *needle,
                                       const char *message,
                                       const char *anchor)
{
    int contains = haystack && needle && strstr(haystack, needle);

    ++counters->assertions;
    if (!contains) {
        ++counters->failures;
        printf("FAIL: %s missing=%s [%s]\n",
               message,
               needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
    assert(contains);
}

static int expected_visible_slot_pc34_compat(int index)
{
    return 0x5370 + index;
}

static int expected_chest_list_slot_pc34_compat(int index)
{
    return 0x4250 + index;
}

static void test_source_metadata_pc34_compat(TestCounters *counters)
{
    const Dm1V1MirrorC146SleepWakeupRepaintGateEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_evidence_pc34_compat();
    const char *text =
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_source_evidence_pc34_compat();

    check_true_pc34_compat(counters, e != NULL,
                           "evidence accessor returns metadata",
                           "COMMAND.C F0380:2361-2364");
    check_int_eq_pc34_compat(counters, e ? e->contractOnly : 0, 1,
                             "contract-only evidence flag",
                             "COMMAND.C F0380:2361-2364");
    check_contains_pc34_compat(counters, e->wakeUpDispatchAnchor,
                               "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:"
                               "2361-2364",
                               "evidence cites wake-up dispatch",
                               "COMMAND.C F0380:2361-2364");
    check_contains_pc34_compat(counters, e->wakeUpDispatchAnchor,
                               "C146_COMMAND_WAKE_UP",
                               "evidence cites C146 command",
                               "DEFS.H:335");
    check_contains_pc34_compat(counters, e->wakeUpBodyAnchor,
                               "CHAMPION.C F0314_CHAMPION_WakeUp:1382-1414",
                               "evidence cites wake-up body",
                               "CHAMPION.C F0314:1382-1414");
    check_contains_pc34_compat(counters, e->wakeUpBodyAnchor,
                               "G0300_B_PartyIsResting",
                               "evidence cites party resting clear",
                               "DEFS.H:5695");
    check_contains_pc34_compat(counters, e->menuDisableAnchor,
                               "F0456_START_DrawDisabledMenus:335-385",
                               "evidence cites F0456 disable",
                               "STARTUP2.C F0456:335-385");
    check_contains_pc34_compat(counters, e->menuEnableAnchor,
                               "F0457_START_DrawEnabledMenus_CPSF:388-441",
                               "evidence cites F0457 enable",
                               "STARTUP2.C F0457:388-441");
    check_contains_pc34_compat(counters, e->restScreenDrawAnchor,
                               "F0379_COMMAND_DrawRestScreen:1996-2034",
                               "evidence cites F0379 rest screen",
                               "COMMAND.C F0379:1996-2034");
    check_contains_pc34_compat(counters, e->floorCeilingAnchor,
                               "F0098_DUNGEONVIEW_DrawFloorAndCeiling:"
                               "2962-2997",
                               "evidence cites F0098 floor/ceiling",
                               "DUNVIEW.C F0098:2962-2997");
    check_contains_pc34_compat(counters, e->movementArrowsAnchor,
                               "F0395_MENUS_DrawMovementArrows",
                               "evidence cites F0395 movement arrows",
                               "PANEL.C F0395");
    check_contains_pc34_compat(counters, e->restGateAnchor,
                               "CHANGE2_15_FIX",
                               "evidence cites CHANGE2_15_FIX rest gate",
                               "COMMAND.C F0380:2336-2358");
    check_contains_pc34_compat(counters, e->candidateOpenAnchor,
                               "REVIVE.C F0280:124-132",
                               "evidence cites F0280 candidate open",
                               "REVIVE.C F0280:124-132");
    check_contains_pc34_compat(counters, e->candidateCloseAnchor,
                               "REVIVE.C F0282:744-806",
                               "evidence cites F0282 candidate close",
                               "REVIVE.C F0282:744-806");
    check_contains_pc34_compat(counters, e->panelResurrectReincarnateAnchor,
                               "F0346_INVENTORY_DrawPanel_ResurrectReincarnate:"
                               "1619-1637",
                               "evidence cites F0346 resurrect panel blit",
                               "PANEL.C F0346:1619-1637");
    check_contains_pc34_compat(counters, e->panelDrawRouterAnchor,
                               "F0347_INVENTORY_DrawPanel:1639-1693",
                               "evidence cites F0347 panel router",
                               "PANEL.C F0347:1639-1693");
    check_contains_pc34_compat(counters, e->actingChampionClearAnchor,
                               "F0506_ui_ActingChampionOrdinal",
                               "evidence cites acting champion ordinal",
                               "CHAMPION.C F0506");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "DEFS.H:335 C146_COMMAND_WAKE_UP",
                               "evidence cites DEFS C146",
                               "DEFS.H:335");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "DEFS.H:5694 G0299",
                               "evidence cites DEFS G0299",
                               "DEFS.H:5694");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "DEFS.H:5695 G0300",
                               "evidence cites DEFS G0300",
                               "DEFS.H:5695");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "DEFS.H:3001-3008",
                               "evidence cites DEFS panel content ids",
                               "DEFS.H:3001-3008");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "G0032_ai_Graphic562_Box_Panel",
                               "evidence cites panel box rect",
                               "DEFS.H:5326");
    check_contains_pc34_compat(counters, e->contractScope,
                               "C146_COMMAND_WAKE_UP",
                               "contract names C146 wake-up",
                               e->contractScope);
    check_contains_pc34_compat(counters, e->contractScope,
                               "panel rectangle bytes stable",
                               "contract names panel rectangle stability",
                               e->contractScope);
    check_contains_pc34_compat(counters, e->contractScope,
                               "candidate ordinal live",
                               "contract names candidate live",
                               e->contractScope);
    check_contains_pc34_compat(counters, e->contractScope,
                               "M568_PANEL_RESURRECT_REINCARNATE",
                               "contract names M568 panel content",
                               e->contractScope);

    check_contains_pc34_compat(counters, text,
                               "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:"
                               "2361-2364",
                               "source string cites wake-up dispatch",
                               "COMMAND.C F0380:2361-2364");
    check_contains_pc34_compat(counters, text,
                               "CHAMPION.C F0314_CHAMPION_WakeUp:1382-1414",
                               "source string cites wake-up body",
                               "CHAMPION.C F0314:1382-1414");
    check_contains_pc34_compat(counters, text,
                               "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
                               "source string cites F0098",
                               "DUNVIEW.C F0098:2962-2997");
    check_contains_pc34_compat(counters, text,
                               "F0457_START_DrawEnabledMenus_CPSF",
                               "source string cites F0457",
                               "STARTUP2.C F0457:388-441");
    check_contains_pc34_compat(counters, text,
                               "F0357_COMMAND_DiscardAllInput",
                               "source string cites F0357 input discard",
                               "COMMAND.C F0357");
    check_contains_pc34_compat(counters, text,
                               "CHANGE2_15_FIX",
                               "source string cites CHANGE2_15_FIX",
                               "COMMAND.C F0380:2336-2358");
    check_contains_pc34_compat(counters, text,
                               "REVIVE.C F0280:124-132",
                               "source string cites F0280",
                               "REVIVE.C F0280:124-132");
    check_contains_pc34_compat(counters, text,
                               "REVIVE.C F0282:744-806",
                               "source string cites F0282",
                               "REVIVE.C F0282:744-806");
    check_contains_pc34_compat(counters, text,
                               "PANEL.C F0346_INVENTORY_DrawPanel_Resurrect"
                               "Reincarnate",
                               "source string cites F0346 resurrect panel",
                               "PANEL.C F0346:1619-1637");
    check_contains_pc34_compat(counters, text,
                               "PANEL.C F0347_INVENTORY_DrawPanel",
                               "source string cites F0347 panel router",
                               "PANEL.C F0347:1639-1699");
    check_contains_pc34_compat(counters, text,
                               "DEFS.H:335 C146_COMMAND_WAKE_UP",
                               "source string cites DEFS C146",
                               "DEFS.H:335");
    check_contains_pc34_compat(counters, text,
                               "DEFS.H:5694",
                               "source string cites DEFS G0299",
                               "DEFS.H:5694");
}

static void test_spec_metadata_pc34_compat(TestCounters *counters)
{
    const Dm1V1MirrorC146SleepWakeupRepaintGateSpecPc34Compat *spec =
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_spec_pc34_compat();

    check_true_pc34_compat(counters, spec != NULL,
                           "spec accessor returns metadata",
                           "CHAMPION.C F0314:1382-1414");
    check_uint_eq_pc34_compat(counters, spec->deterministicSeed, 0xC1460531u,
                              "spec deterministic seed",
                              "DEFS.H:335");
    check_int_eq_pc34_compat(counters, spec->leaderIndex, 0,
                             "spec leader index",
                             "DEFS.H:5694");
    check_int_eq_pc34_compat(counters, spec->partyChampionCount, 4,
                             "spec party count",
                             "REVIVE.C F0280:124-132");
    check_uint_eq_pc34_compat(counters, spec->candidateOrdinal, 4,
                              "spec candidate ordinal",
                              "DEFS.H:5694");
    check_int_eq_pc34_compat(counters, spec->c146WakeUpCommand, 146,
                             "spec C146 wake-up command",
                             "DEFS.H:335");
    check_int_eq_pc34_compat(counters, spec->c145RestCommand, 145,
                             "spec C145 rest command",
                             "DEFS.H:334");
    check_int_eq_pc34_compat(counters, spec->c147FreezeGameCommand, 147,
                             "spec C147 freeze game command",
                             "DEFS.H:336");
    check_int_eq_pc34_compat(counters, spec->c148UnfreezeGameCommand, 148,
                             "spec C148 unfreeze game command",
                             "DEFS.H:337");
    check_int_eq_pc34_compat(counters, spec->c040PanelGraphic, 40,
                             "spec C040 panel graphic",
                             "DEFS.H:2200");
    check_int_eq_pc34_compat(counters, spec->c10PanelColor, 10,
                             "spec C10 color",
                             "DEFS.H:2088");
    check_int_eq_pc34_compat(counters, spec->m568CandidatePanel, 568,
                             "spec M568 candidate panel",
                             "DEFS.H:3001-3008");
    check_int_eq_pc34_compat(counters, spec->m565FoodWaterPanel, 565,
                             "spec M565 food/water panel",
                             "DEFS.H:3001-3008");
    check_int_eq_pc34_compat(counters, spec->m643ScrollPanel, 643,
                             "spec M643 scroll panel",
                             "DEFS.H:3001-3008");
    check_int_eq_pc34_compat(counters, spec->m569ChestPanel, 569,
                             "spec M569 chest panel",
                             "DEFS.H:3001-3008");
    check_int_eq_pc34_compat(counters, spec->c070MouthCommand, 70,
                             "spec C070 mouth command",
                             "DEFS.H:303");
    check_int_eq_pc34_compat(counters, spec->c537VisibleBase, 537,
                             "spec C537 visible base",
                             "DEFS.H:2200");
    check_int_eq_pc34_compat(counters, spec->c544VisibleTop, 544,
                             "spec C544 visible top",
                             "DEFS.H:2200");
    check_int_eq_pc34_compat(counters, spec->panelBoxXMin, 80,
                             "spec panel box xMin",
                             "DEFS.H:5326");
    check_int_eq_pc34_compat(counters, spec->panelBoxXMax, 223,
                             "spec panel box xMax",
                             "DEFS.H:5326");
    check_int_eq_pc34_compat(counters, spec->panelBoxYMin, 52,
                             "spec panel box yMin",
                             "DEFS.H:5326");
    check_int_eq_pc34_compat(counters, spec->panelBoxYMax, 124,
                             "spec panel box yMax",
                             "DEFS.H:5326");
    check_uint_eq_pc34_compat(counters, spec->restEntryVerticalBlanks, 10u,
                              "spec MEDIA009 vertical blanks",
                              "CHAMPION.C F0314:1395-1398");
}

static void test_initial_state_pc34_compat(TestCounters *counters)
{
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat state;
    int i;

    dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_init_pc34_compat(
        &state);

    check_int_eq_pc34_compat(counters, state.contractOnly, 1,
                             "initial contract-only flag",
                             "COMMAND.C F0380:2361-2364");
    check_uint_eq_pc34_compat(counters, state.deterministicSeed, 0xC1460531u,
                              "initial deterministic seed",
                              "DEFS.H:335");
    check_int_eq_pc34_compat(counters, state.leaderIndex, 0,
                             "initial leader index",
                             "DEFS.H:5694");
    check_int_eq_pc34_compat(counters, state.partyChampionCount, 4,
                             "initial party count",
                             "REVIVE.C F0280:124-132");
    check_uint_eq_pc34_compat(counters, state.candidateOrdinal, 4,
                              "initial candidate ordinal",
                              "REVIVE.C F0280:124-132");
    check_uint_eq_pc34_compat(counters, state.g0299CandidateOrdinal, 4,
                              "initial G0299 candidate ordinal",
                              "DEFS.H:5694");
    check_int_eq_pc34_compat(counters, state.c040PanelOpen, 1,
                             "initial C040 panel open",
                             "PANEL.C F0346:1619-1637 / F0347:1639-1693");
    check_int_eq_pc34_compat(counters, state.c040PanelGraphic, 40,
                             "initial C040 panel graphic",
                             "DEFS.H:2200");
    check_int_eq_pc34_compat(counters, state.c040PanelCommand, 568,
                             "initial M568 command",
                             "DEFS.H:3001-3008");
    check_int_eq_pc34_compat(counters, state.c040PanelColor, 10,
                             "initial flesh color",
                             "DEFS.H:2088");
    check_int_eq_pc34_compat(counters, state.c040PanelOwnerSlot, 30,
                             "initial C30 owner slot",
                             "DEFS.H:810-817");
    check_int_eq_pc34_compat(counters, state.c038SlotBox, 38,
                             "initial C38 slot box",
                             "DEFS.H:1874-1878");
    check_int_eq_pc34_compat(counters, state.c030HandSlot, 30,
                             "initial C30 hand slot",
                             "DEFS.H:810-817");
    check_int_eq_pc34_compat(counters, state.c037HandSlot, 37,
                             "initial C37 hand slot",
                             "DEFS.H:810-817");
    check_int_eq_pc34_compat(counters, state.g0424PanelContent, 568,
                             "initial M568 panel content",
                             "PANEL.C F0346:1626");
    check_int_eq_pc34_compat(counters, state.g0423InventoryChampionOrdinal, 0,
                             "initial inventory champion ordinal",
                             "DEFS.H:5876");
    check_int_eq_pc34_compat(counters, state.g0300PartyIsResting, 1,
                             "initial party is resting",
                             "DEFS.H:5695");
    check_int_eq_pc34_compat(counters, state.g0506ActingChampionOrdinal, 0,
                             "initial acting champion ordinal",
                             "CHAMPION.C F0506");
    check_uint_eq_pc34_compat(counters, state.g0318WaitVerticalBlanks, 0u,
                              "initial wait vertical blanks",
                              "CHAMPION.C F0314:1395");
    check_int_eq_pc34_compat(counters, state.c146WakeUpCommand, 146,
                             "initial C146 wake-up command",
                             "DEFS.H:335");
    check_int_eq_pc34_compat(counters, state.c145RestCommand, 145,
                             "initial C145 rest command",
                             "DEFS.H:334");
    check_int_eq_pc34_compat(counters, state.c147FreezeCommand, 147,
                             "initial C147 freeze command",
                             "DEFS.H:336");
    check_int_eq_pc34_compat(counters, state.c148UnfreezeCommand, 148,
                             "initial C148 unfreeze command",
                             "DEFS.H:337");
    check_int_eq_pc34_compat(counters, state.mouthRouteZone, 545,
                             "initial mouth route zone",
                             "DEFS.H:1874");
    check_int_eq_pc34_compat(counters, state.mouthRouteCommand, 70,
                             "initial mouth route command",
                             "DEFS.H:303");
    check_int_eq_pc34_compat(counters, state.f0280CandidateOpenCount, 1,
                             "initial candidate open count",
                             "REVIVE.C F0280:124-132");
    check_uint_eq_pc34_compat(counters, state.panelHashBeforeWakeUp,
                              state.panelHashAfterWakeUp,
                              "initial panel hash stable",
                              "PANEL.C F0346:1619-1637 / F0347:1639-1693");
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];

        snprintf(label, sizeof(label), "initial visible C%d slot",
                 537 + i);
        check_int_eq_pc34_compat(counters, state.visibleC537ToC544[i],
                                 expected_visible_slot_pc34_compat(i),
                                 label, "DEFS.H:2200");
        snprintf(label, sizeof(label), "initial G0425 list slot %d", i);
        check_int_eq_pc34_compat(counters, state.g0425ChestList[i],
                                 expected_chest_list_slot_pc34_compat(i),
                                 label, "DEFS.H:5876-5881");
        snprintf(label, sizeof(label), "initial champion hand slot %d", i);
        check_int_eq_pc34_compat(counters, state.championHandC537ToC544[i],
                                 expected_visible_slot_pc34_compat(i),
                                 label, "CHAMPION.C F0297:243-268");
    }
}

static void check_run_result_pc34_compat(
    TestCounters *counters,
    const Dm1V1MirrorC146SleepWakeupRepaintGateResultPc34Compat *result)
{
    int i;
    const Dm1V1MirrorC146SleepWakeupRepaintGateEvidencePc34Compat *e =
        result->evidence;

    check_true_pc34_compat(counters, result->accepted,
                           "runtime wake-up mutation accepted",
                           e->contractScope);
    check_uint_eq_pc34_compat(counters, result->deterministicHash,
                              kExpectedHash,
                              "pinned deterministic FNV-1a hash",
                              "COMMAND.C F0380:2361-2364");
    check_uint_eq_pc34_compat(counters, result->initialPanelHash,
                              result->finalPanelHash,
                              "panel hash stable across wake-up tick",
                              e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->initialPanelOpen, 1,
                             "initial panel open captured",
                             e->panelResurrectReincarnateAnchor);
    check_int_eq_pc34_compat(counters, result->finalPanelOpen, 1,
                             "final panel still open after wake-up",
                             e->panelResurrectReincarnateAnchor);
    check_int_eq_pc34_compat(counters, result->initialPanelGraphic, 40,
                             "initial C040 graphic captured",
                             e->defsAnchor);
    check_int_eq_pc34_compat(counters, result->finalPanelGraphic, 40,
                             "final C040 graphic preserved",
                             e->defsAnchor);
    check_int_eq_pc34_compat(counters, result->initialPanelCommand, 568,
                             "initial M568 command captured",
                             e->defsAnchor);
    check_int_eq_pc34_compat(counters, result->finalPanelCommand, 568,
                             "final M568 command preserved",
                             e->defsAnchor);
    check_uint_eq_pc34_compat(counters, result->initialCandidateOrdinal, 4,
                              "initial G0299 candidate captured",
                              e->candidateOpenAnchor);
    check_uint_eq_pc34_compat(counters, result->finalCandidateOrdinal, 4,
                              "G0299 candidate preserved after wake-up",
                              e->candidateOpenAnchor);
    check_int_eq_pc34_compat(counters, result->initialPartyIsResting, 1,
                             "initial G0300 party resting captured",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->finalPartyIsResting, 0,
                             "G0300 party resting cleared by wake-up",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->initialPanelContent, 568,
                             "initial M568 panel content captured",
                             e->panelResurrectReincarnateAnchor);
    check_int_eq_pc34_compat(counters, result->finalPanelContent, 568,
                             "M568 panel content preserved across wake-up",
                             e->panelResurrectReincarnateAnchor);
    check_int_eq_pc34_compat(counters,
                             result->initialInventoryChampionOrdinal, 0,
                             "initial G0423 inventory captured",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->finalInventoryChampionOrdinal,
                             0,
                             "G0423 inventory preserved across wake-up",
                             e->wakeUpBodyAnchor);
    check_uint_eq_pc34_compat(counters, result->finalWaitVerticalBlanks, 10u,
                              "G0318 vertical blanks armed after wake-up",
                              e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters,
                             result->initialActingChampionOrdinal, 0,
                             "initial G0506 captured",
                             e->actingChampionClearAnchor);
    check_int_eq_pc34_compat(counters, result->finalActingChampionOrdinal, 0,
                             "G0506 preserved across wake-up",
                             e->actingChampionClearAnchor);
    check_int_eq_pc34_compat(counters, result->c146WakeUpDispatched, 1,
                             "C146 wake-up was dispatched",
                             "COMMAND.C F0380:2361-2364");
    check_int_eq_pc34_compat(counters, result->c145RestBlockedByCandidate, 1,
                             "C145 rest is blocked by candidate gate",
                             e->restGateAnchor);
    check_int_eq_pc34_compat(counters, result->panelHashStable, 1,
                             "panel hash stable boolean",
                             e->panelResurrectReincarnateAnchor);
    check_int_eq_pc34_compat(counters, result->candidateStillLive, 1,
                             "candidate remains live after wake-up",
                             e->candidateOpenAnchor);
    check_int_eq_pc34_compat(counters, result->noPanelFlicker, 1,
                             "no panel flicker observed",
                             e->panelResurrectReincarnateAnchor);
    check_int_eq_pc34_compat(counters, result->noRedrawClobber, 1,
                             "no redraw clobber observed",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->noCandidateLeakage, 1,
                             "no candidate marker leakage observed",
                             e->candidateCloseAnchor);
    check_int_eq_pc34_compat(counters, result->partyRestingCleared, 1,
                             "G0300 cleared by wake-up",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->waitVerticalBlanksArmed, 1,
                             "G0318 armed by wake-up",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->inputDiscardFired, 1,
                             "F0357 input discard fired",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->menuRedrawFired, 1,
                             "F0457 menu redraw fired",
                             e->menuEnableAnchor);
    check_int_eq_pc34_compat(counters, result->panelRedrawSkipped, 1,
                             "F0346/F0347 panel redraw skipped",
                             e->panelResurrectReincarnateAnchor);
    check_int_eq_pc34_compat(counters, result->candidateCloseSkipped, 1,
                             "F0282 candidate close skipped",
                             e->candidateCloseAnchor);
    check_int_eq_pc34_compat(counters, result->candidateOpenStable, 1,
                             "F0280 candidate open stable at 1",
                             e->candidateOpenAnchor);
    check_int_eq_pc34_compat(counters, result->actingChampionOrdinalStable, 1,
                             "G0506 stable across wake-up",
                             e->actingChampionClearAnchor);
    check_int_eq_pc34_compat(counters, result->inventoryChampionOrdinalStable,
                             1,
                             "G0423 stable across wake-up",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->panelContentStable, 1,
                             "G0424 panel content stable across wake-up",
                             e->panelResurrectReincarnateAnchor);
    check_int_eq_pc34_compat(counters, result->visibleSlotsCleared, 0,
                             "visible C537..C544 slots not cleared by "
                             "wake-up",
                             e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(counters, result->chestListStable, 1,
                             "G0425 chest list remains stable",
                             e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(counters, result->championHandStateStable, 1,
                             "champion C537..C544 hand state stable",
                             e->panelHandRedrawAnchor);
    check_int_eq_pc34_compat(counters, result->f0314WakeUpCount, 1,
                             "F0314 wake-up fired exactly once",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->f0098FloorCeilingCount, 1,
                             "F0098 floor/ceiling fired exactly once",
                             e->floorCeilingAnchor);
    check_int_eq_pc34_compat(counters, result->f0456DisabledMenusCount, 0,
                             "F0456 disabled menus fired zero times",
                             e->menuDisableAnchor);
    check_int_eq_pc34_compat(counters, result->f0457EnabledMenusCount, 1,
                             "F0457 enabled menus fired exactly once",
                             e->menuEnableAnchor);
    check_int_eq_pc34_compat(counters, result->f0379RestScreenCount, 0,
                             "F0379 rest screen fired zero times",
                             e->restScreenDrawAnchor);
    check_int_eq_pc34_compat(counters,
                             result->f0346PanelResurrectReincarnateCount, 0,
                             "F0346 resurrect panel blit fired zero times",
                             e->panelResurrectReincarnateAnchor);
    check_int_eq_pc34_compat(counters, result->f0347PanelDrawRouterCount, 0,
                             "F0347 panel router fired zero times",
                             e->panelDrawRouterAnchor);
    check_int_eq_pc34_compat(counters, result->f0280CandidateOpenCount, 1,
                             "F0280 candidate open remains at initial 1",
                             e->candidateOpenAnchor);
    check_int_eq_pc34_compat(counters, result->f0282CandidateCloseCount, 0,
                             "F0282 candidate close did not run",
                             e->candidateCloseAnchor);
    check_int_eq_pc34_compat(counters, result->f0357InputDiscardCount, 1,
                             "F0357 input discard fired exactly once",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->f0292ChampionDrawStateCount, 0,
                             "F0292 champion draw state fired zero times",
                             e->panelHandRedrawAnchor);
    check_int_eq_pc34_compat(counters,
                             result->f0293ChampionDrawAllStatesCount, 0,
                             "F0293 champion draw all states fired zero "
                             "times",
                             e->panelHandRedrawAnchor);
    check_int_eq_pc34_compat(counters,
                             result->f0457ReenteredRestBranchCount, 0,
                             "F0457 did not re-enter the rest branch",
                             e->menuEnableAnchor);
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];

        snprintf(label, sizeof(label), "visible before C%d", 537 + i);
        check_int_eq_pc34_compat(counters, result->visibleBefore[i],
                                 expected_visible_slot_pc34_compat(i),
                                 label, e->chestListOpenCloseAnchor);
        snprintf(label, sizeof(label), "visible after C%d", 537 + i);
        check_int_eq_pc34_compat(counters, result->visibleAfter[i],
                                 expected_visible_slot_pc34_compat(i),
                                 label,
                                 e->chestListOpenCloseAnchor);
        snprintf(label, sizeof(label), "G0425 before slot %d", i);
        check_int_eq_pc34_compat(counters, result->chestListBefore[i],
                                 expected_chest_list_slot_pc34_compat(i),
                                 label, e->chestListOpenCloseAnchor);
        snprintf(label, sizeof(label), "G0425 after slot %d", i);
        check_int_eq_pc34_compat(counters, result->chestListAfter[i],
                                 expected_chest_list_slot_pc34_compat(i),
                                 label, e->chestListOpenCloseAnchor);
        snprintf(label, sizeof(label), "champion hand before slot %d", i);
        check_int_eq_pc34_compat(counters, result->championHandBefore[i],
                                 expected_visible_slot_pc34_compat(i),
                                 label, e->panelHandRedrawAnchor);
        snprintf(label, sizeof(label), "champion hand after slot %d", i);
        check_int_eq_pc34_compat(counters, result->championHandAfter[i],
                                 expected_visible_slot_pc34_compat(i),
                                 label, e->panelHandRedrawAnchor);
    }

    check_int_eq_pc34_compat(counters, result->rejectsNullState, 1,
                             "guard rejects null state",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsNullResult, 1,
                             "guard rejects null result",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsNonContract, 1,
                             "guard rejects non-contract state",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsNoPanel, 1,
                             "guard rejects no C040 panel",
                             e->panelResurrectReincarnateAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsNoCandidate, 1,
                             "guard rejects no G0299 candidate",
                             e->candidateOpenAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsPartyNotResting, 1,
                             "guard rejects not-resting entry",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsFrozenGame, 1,
                             "guard rejects frozen-game state",
                             e->wakeUpBodyAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsWrongWakeUpCommand, 1,
                             "guard rejects wrong wake-up command id",
                             e->defsAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsWrongPanelContent, 1,
                             "guard rejects wrong panel content",
                             e->panelResurrectReincarnateAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsMouthRouteOpen, 1,
                             "guard rejects open mouth route",
                             e->panelHandRedrawAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsCandidateLeakPreload, 1,
                             "guard rejects preloaded candidate leak",
                             e->candidateCloseAnchor);
    check_int_eq_pc34_compat(counters, result->mutationGuardsOk, 1,
                             "mutation guard matrix passed",
                             e->wakeUpBodyAnchor);
}

static void test_run_sequence_pc34_compat(TestCounters *counters)
{
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat state;
    Dm1V1MirrorC146SleepWakeupRepaintGateResultPc34Compat result;
    int ok;

    dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_init_pc34_compat(
        &state);
    ok = dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
        &state, &result);

    check_int_eq_pc34_compat(counters, ok, 1,
                             "run returns accepted",
                             "COMMAND.C F0380:2361-2364");
    check_run_result_pc34_compat(counters, &result);
    check_int_eq_pc34_compat(counters, state.c040PanelOpen, 1,
                             "state panel remains open after run",
                             "PANEL.C F0346:1619-1637 / F0347:1639-1693");
    check_uint_eq_pc34_compat(counters, state.g0299CandidateOrdinal, 4,
                              "state G0299 candidate remains live",
                              "REVIVE.C F0280:124-132");
    check_int_eq_pc34_compat(counters, state.g0300PartyIsResting, 0,
                             "state G0300 cleared after run",
                             "CHAMPION.C F0314:1393");
    check_uint_eq_pc34_compat(counters, state.g0318WaitVerticalBlanks, 10u,
                              "state G0318 armed after run",
                              "CHAMPION.C F0314:1395");
    check_int_eq_pc34_compat(counters, state.g0424PanelContent, 568,
                             "state G0424 preserved after run",
                             "PANEL.C F0346:1626");
    check_int_eq_pc34_compat(counters, state.g0423InventoryChampionOrdinal, 0,
                             "state G0423 preserved after run",
                             "DEFS.H:5876");
    check_int_eq_pc34_compat(counters, state.g0506ActingChampionOrdinal, 0,
                             "state G0506 preserved after run",
                             "CHAMPION.C F0506");
    check_uint_eq_pc34_compat(counters, state.panelHashBeforeWakeUp,
                              state.panelHashAfterWakeUp,
                              "state panel hash stable",
                              "CHAMPION.C F0314:1382-1414");
    check_int_eq_pc34_compat(counters, state.panelFlickerCount, 0,
                             "state flicker count",
                             "PANEL.C F0346:1619-1637");
    check_int_eq_pc34_compat(counters, state.redrawClobberCount, 0,
                             "state redraw clobber count",
                             "PANEL.C F0098:2962-2997");
    check_int_eq_pc34_compat(counters, state.candidateLeakCount, 0,
                             "state candidate leak count",
                             "REVIVE.C F0282:744-806");
    check_int_eq_pc34_compat(counters, state.f0314WakeUpCount, 1,
                             "state F0314 fired exactly once",
                             "CHAMPION.C F0314:1382-1414");
    check_int_eq_pc34_compat(counters, state.f0098FloorCeilingCount, 1,
                             "state F0098 fired exactly once",
                             "DUNVIEW.C F0098:2962-2997");
    check_int_eq_pc34_compat(counters, state.f0457EnabledMenusCount, 1,
                             "state F0457 fired exactly once",
                             "STARTUP2.C F0457:388-441");
    check_int_eq_pc34_compat(counters, state.f0357InputDiscardCount, 1,
                             "state F0357 fired exactly once",
                             "COMMAND.C F0357");
    check_int_eq_pc34_compat(counters, state.f0282CandidateCloseCount, 0,
                             "state F0282 did not run",
                             "REVIVE.C F0282:744-806");
    check_int_eq_pc34_compat(counters,
                             state.f0346PanelResurrectReincarnateCount, 0,
                             "state F0346 did not run",
                             "PANEL.C F0346:1619-1637");
    check_int_eq_pc34_compat(counters, state.f0347PanelDrawRouterCount, 0,
                             "state F0347 did not run",
                             "PANEL.C F0347:1639-1693");
    check_int_eq_pc34_compat(counters, state.f0292ChampionDrawStateCount, 0,
                             "state F0292 did not run",
                             "CHAMPION.C F0292:771-839");
    check_int_eq_pc34_compat(counters, state.f0293ChampionDrawAllStatesCount, 0,
                             "state F0293 did not run",
                             "CHAMPION.C F0293:1117-1143");
}

static void test_rejects_invalid_inputs_pc34_compat(TestCounters *counters)
{
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat state;
    Dm1V1MirrorC146SleepWakeupRepaintGateResultPc34Compat result;

    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
            NULL, &result),
        0,
        "run rejects null state",
        "CHAMPION.C F0314:1382-1414");
    dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_init_pc34_compat(
        &state);
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
            &state, NULL),
        0,
        "run rejects null result",
        "CHAMPION.C F0314:1382-1414");
    state.contractOnly = 0;
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
            &state, &result),
        0,
        "run rejects non-contract state",
        "CHAMPION.C F0314:1382-1414");
    dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_init_pc34_compat(
        &state);
    state.c040PanelOpen = 0;
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
            &state, &result),
        0,
        "run rejects no live C040 panel",
        "PANEL.C F0346:1619-1637 / F0347:1639-1693");
    dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_init_pc34_compat(
        &state);
    state.g0299CandidateOrdinal = 0;
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
            &state, &result),
        0,
        "run rejects no G0299 candidate",
        "REVIVE.C F0280:124-132");
    dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_init_pc34_compat(
        &state);
    state.g0300PartyIsResting = 0;
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
            &state, &result),
        0,
        "run rejects not-resting entry",
        "CHAMPION.C F0314:1393");
    dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_init_pc34_compat(
        &state);
    state.c146WakeUpCommand = 0;
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
            &state, &result),
        0,
        "run rejects wrong C146 wake-up command id",
        "DEFS.H:335");
    dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_init_pc34_compat(
        &state);
    state.c147FreezeFilledViewport = 1;
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
            &state, &result),
        0,
        "run rejects frozen-game state",
        "CHAMPION.C F0314:1382-1414");
}

int main(void)
{
    TestCounters counters;

    counters.assertions = 0;
    counters.failures = 0;
    printf("=== DM1 V1 mirror-candidate C146 sleep/wake-up repaint gate "
           "===\n");
    test_source_metadata_pc34_compat(&counters);
    test_spec_metadata_pc34_compat(&counters);
    test_initial_state_pc34_compat(&counters);
    test_run_sequence_pc34_compat(&counters);
    test_rejects_invalid_inputs_pc34_compat(&counters);
    if (counters.failures) {
        printf("FAIL: assertions=%d failures=%d\n",
               counters.assertions,
               counters.failures);
        return 1;
    }
    printf("PASS: assertions=%d failures=0\n", counters.assertions);
    return counters.assertions >= 130 ? 0 : 1;
}
