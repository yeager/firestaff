/* ReDMCSB CHEST.C F0333:30-67 ... PANEL.C F0352 ... */
#include "dm1_v1_mirror_candidate_c040_redraw_after_chest_close_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

enum {
    kExpectedHash = 3168216461u
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
    const Dm1V1MirrorC040RedrawAfterChestCloseEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_c040_redraw_after_chest_close_evidence_pc34_compat();
    const char *text =
        dm1_v1_mirror_candidate_c040_redraw_after_chest_close_source_evidence_pc34_compat();

    check_true_pc34_compat(counters, e != NULL,
                           "evidence accessor returns metadata",
                           "CHEST.C F0333:30-67");
    check_int_eq_pc34_compat(counters, e ? e->contractOnly : 0, 1,
                             "contract-only evidence flag",
                             "CHEST.C F0333:30-67");
    check_contains_pc34_compat(counters, e->chestListOpenCloseAnchor,
                               "CHEST.C F0333:30-67",
                               "evidence cites chest list open/close",
                               "CHEST.C F0333:30-67");
    check_contains_pc34_compat(counters, e->chestVisibleCloseRewriteAnchor,
                               "CHEST.C F0334:117-132",
                               "evidence cites visible close rewrite",
                               "CHEST.C F0334:117-132");
    check_contains_pc34_compat(counters, e->championHandStateAnchor,
                               "CHAMPION.C F0297:243-268",
                               "evidence cites hand-state",
                               "CHAMPION.C F0297:243-268");
    check_contains_pc34_compat(counters, e->championOwnershipAnchor,
                               "CHAMPION.C F0298:270-298",
                               "evidence cites C30+ ownership",
                               "CHAMPION.C F0298:270-298");
    check_contains_pc34_compat(counters, e->championListWalkAnchor,
                               "CHAMPION.C F0300:511-515",
                               "evidence cites F0300 list walk",
                               "CHAMPION.C F0300:511-515");
    check_contains_pc34_compat(counters, e->championListWalkAnchor,
                               "F0301:606-614",
                               "evidence cites F0301 list walk",
                               "CHAMPION.C F0301:606-614");
    check_contains_pc34_compat(counters, e->championListWalkAnchor,
                               "F0302:662-714",
                               "evidence cites F0302 list walk",
                               "CHAMPION.C F0302:662-714");
    check_contains_pc34_compat(counters, e->championSwitchAnchor,
                               "CHAMPION.C F0284:93-131",
                               "evidence cites champion switches",
                               "CHAMPION.C F0284:93-131");
    check_contains_pc34_compat(counters, e->panelDrawHookAnchor,
                               "PANEL.C F0344:1390-1406 / F0345",
                               "evidence cites C040 panel draw hooks",
                               "PANEL.C F0344:1390-1406 / F0345");
    check_contains_pc34_compat(counters, e->panelStateAnchor,
                               "PANEL.C F0346:1619-1657 / F0347",
                               "evidence cites C040 panel state",
                               "PANEL.C F0346:1619-1657 / F0347");
    check_contains_pc34_compat(counters, e->panelRedrawOnCloseAnchor,
                               "PANEL.C F0352",
                               "evidence cites C040 redraw on close",
                               "PANEL.C F0352");
    check_contains_pc34_compat(counters, e->candidateOpenAnchor,
                               "REVIVE.C F0280:124-132",
                               "evidence cites candidate open",
                               "REVIVE.C F0280:124-132");
    check_contains_pc34_compat(counters, e->candidateCloseAnchor,
                               "REVIVE.C F0282:744-806",
                               "evidence cites candidate close/clear",
                               "REVIVE.C F0282:744-806");
    check_contains_pc34_compat(counters, e->mirrorQueueAnchor,
                               "COMMAND.C F0359:1985-1990",
                               "evidence cites mirror queue write",
                               "COMMAND.C F0359:1985-1990");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "DEFS.H:2088 C10_COLOR_FLESH",
                               "evidence cites C10_COLOR_FLESH",
                               "DEFS.H:2088");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "DEFS.H:810-817 C30..C37",
                               "evidence cites C30..C37",
                               "DEFS.H:810-817");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "DEFS.H:1874-1878 C38",
                               "evidence cites C38", "DEFS.H:1874-1878");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "DEFS.H:2200 C040",
                               "evidence cites C040", "DEFS.H:2200");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "DEFS.H:3001-3008 M568/M569",
                               "evidence cites M568/M569",
                               "DEFS.H:3001-3008");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "DEFS.H:5694 G0299",
                               "evidence cites G0299", "DEFS.H:5694");
    check_contains_pc34_compat(counters, e->defsAnchor,
                               "DEFS.H:5876-5881 G0425/G0426",
                               "evidence cites G0425/G0426",
                               "DEFS.H:5876-5881");
    check_contains_pc34_compat(counters, e->contractScope,
                               "food/water panel redraw stability",
                               "contract names C040 redraw stability",
                               e->contractScope);
    check_contains_pc34_compat(counters, e->contractScope,
                               "no flicker", "contract names no flicker",
                               e->contractScope);
    check_contains_pc34_compat(counters, e->contractScope,
                               "candidate leakage",
                               "contract names candidate leakage",
                               e->contractScope);

    check_contains_pc34_compat(counters, text, "CHEST.C F0333:30-67",
                               "source string cites F0333",
                               "CHEST.C F0333:30-67");
    check_contains_pc34_compat(counters, text, "CHEST.C F0334:117-132",
                               "source string cites F0334",
                               "CHEST.C F0334:117-132");
    check_contains_pc34_compat(counters, text, "CHAMPION.C F0297:243-268",
                               "source string cites F0297",
                               "CHAMPION.C F0297:243-268");
    check_contains_pc34_compat(counters, text, "CHAMPION.C F0298:270-298",
                               "source string cites F0298",
                               "CHAMPION.C F0298:270-298");
    check_contains_pc34_compat(counters, text, "CHAMPION.C F0300:511-515",
                               "source string cites F0300",
                               "CHAMPION.C F0300:511-515");
    check_contains_pc34_compat(counters, text, "CHAMPION.C F0301:606-614",
                               "source string cites F0301",
                               "CHAMPION.C F0301:606-614");
    check_contains_pc34_compat(counters, text, "CHAMPION.C F0302:662-714",
                               "source string cites F0302",
                               "CHAMPION.C F0302:662-714");
    check_contains_pc34_compat(counters, text, "CHAMPION.C F0284:93-131",
                               "source string cites F0284",
                               "CHAMPION.C F0284:93-131");
    check_contains_pc34_compat(counters, text, "PANEL.C F0344:1390-1406",
                               "source string cites F0344",
                               "PANEL.C F0344:1390-1406");
    check_contains_pc34_compat(counters, text, "PANEL.C F0346:1619-1657",
                               "source string cites F0346",
                               "PANEL.C F0346:1619-1657");
    check_contains_pc34_compat(counters, text, "PANEL.C F0352",
                               "source string cites F0352", "PANEL.C F0352");
    check_contains_pc34_compat(counters, text, "REVIVE.C F0280:124-132",
                               "source string cites F0280",
                               "REVIVE.C F0280:124-132");
    check_contains_pc34_compat(counters, text, "REVIVE.C F0282:744-806",
                               "source string cites F0282",
                               "REVIVE.C F0282:744-806");
    check_contains_pc34_compat(counters, text, "COMMAND.C F0359:1985-1990",
                               "source string cites F0359",
                               "COMMAND.C F0359:1985-1990");
    check_contains_pc34_compat(counters, text, "DEFS.H:2088",
                               "source string cites DEFS C10",
                               "DEFS.H:2088");
    check_contains_pc34_compat(counters, text, "DEFS.H:5876-5881",
                               "source string cites DEFS G0425/G0426",
                               "DEFS.H:5876-5881");
}

static void test_spec_metadata_pc34_compat(TestCounters *counters)
{
    const Dm1V1MirrorC040RedrawAfterChestCloseSpecPc34Compat *spec =
        dm1_v1_mirror_candidate_c040_redraw_after_chest_close_spec_pc34_compat();

    check_true_pc34_compat(counters, spec != NULL,
                           "spec accessor returns metadata",
                           "REVIVE.C F0280:124-132");
    check_uint_eq_pc34_compat(counters, spec->deterministicSeed, 0xC0400545u,
                              "spec deterministic seed",
                              "COMMAND.C F0359:1985-1990");
    check_int_eq_pc34_compat(counters, spec->leaderIndex, 0,
                             "spec leader index",
                             "CHAMPION.C F0298:270-298");
    check_int_eq_pc34_compat(counters, spec->partyChampionCount, 4,
                             "spec party count",
                             "REVIVE.C F0280:124-132");
    check_uint_eq_pc34_compat(counters, spec->candidateOrdinal, 4,
                              "spec candidate ordinal",
                              "DEFS.H:5694");
    check_int_eq_pc34_compat(counters, spec->c040PanelGraphic, 40,
                             "spec C040 panel graphic", "DEFS.H:2200");
    check_int_eq_pc34_compat(counters, spec->c10PanelColor, 10,
                             "spec C10 color", "DEFS.H:2088");
    check_int_eq_pc34_compat(counters, spec->m568CandidatePanel, 568,
                             "spec M568 candidate panel",
                             "DEFS.H:3001-3008");
    check_int_eq_pc34_compat(counters, spec->m569ChestPanel, 569,
                             "spec M569 chest panel", "DEFS.H:3001-3008");
    check_int_eq_pc34_compat(counters, spec->c545MouthZone, 545,
                             "spec C545 mouth zone",
                             "CHEST.C F0333:30-67");
    check_int_eq_pc34_compat(counters, spec->c070MouthCommand, 70,
                             "spec C070 mouth command",
                             "COMMAND.C F0359:1985-1990");
    check_int_eq_pc34_compat(counters, spec->openChestThing, 0x6400,
                             "spec open chest thing",
                             "DEFS.H:5876-5881");
    check_int_eq_pc34_compat(counters, spec->candidateMarkerThing, 0x2994,
                             "spec candidate marker", "DEFS.H:5694");
}

static void test_initial_state_pc34_compat(TestCounters *counters)
{
    Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat state;
    int i;

    dm1_v1_mirror_candidate_c040_redraw_after_chest_close_init_pc34_compat(
        &state);

    check_int_eq_pc34_compat(counters, state.contractOnly, 1,
                             "initial contract-only flag",
                             "CHEST.C F0333:30-67");
    check_uint_eq_pc34_compat(counters, state.deterministicSeed, 0xC0400545u,
                              "initial deterministic seed",
                              "COMMAND.C F0359:1985-1990");
    check_int_eq_pc34_compat(counters, state.leaderIndex, 0,
                             "initial leader index",
                             "CHAMPION.C F0298:270-298");
    check_int_eq_pc34_compat(counters, state.partyChampionCount, 4,
                             "initial party count",
                             "REVIVE.C F0280:124-132");
    check_uint_eq_pc34_compat(counters, state.candidateOrdinal, 4,
                              "initial candidate ordinal",
                              "REVIVE.C F0280:124-132");
    check_uint_eq_pc34_compat(counters, state.g0299CandidateOrdinal, 4,
                              "initial G0299 candidate", "DEFS.H:5694");
    check_int_eq_pc34_compat(counters, state.c040PanelOpen, 1,
                             "initial C040 panel open",
                             "PANEL.C F0346:1619-1657 / F0347");
    check_int_eq_pc34_compat(counters, state.c040PanelGraphic, 40,
                             "initial C040 panel graphic",
                             "DEFS.H:2200");
    check_int_eq_pc34_compat(counters, state.c040PanelCommand, 568,
                             "initial M568 command", "DEFS.H:3001-3008");
    check_int_eq_pc34_compat(counters, state.c040PanelColor, 10,
                             "initial flesh color", "DEFS.H:2088");
    check_int_eq_pc34_compat(counters, state.c040PanelOwnerSlot, 30,
                             "initial C30 owner slot", "DEFS.H:810-817");
    check_int_eq_pc34_compat(counters, state.c038SlotBox, 38,
                             "initial C38 slot box", "DEFS.H:1874-1878");
    check_int_eq_pc34_compat(counters, state.mouthRouteZone, 545,
                             "initial C545 mouth route",
                             "CHEST.C F0333:30-67");
    check_int_eq_pc34_compat(counters, state.mouthRouteCommand, 70,
                             "initial C070 command",
                             "COMMAND.C F0359:1985-1990");
    check_int_eq_pc34_compat(counters, state.g0426OpenChestThing, 0x6400,
                             "initial G0426 open chest",
                             "DEFS.H:5876-5881");
    check_int_eq_pc34_compat(counters, state.chestOpen, 1,
                             "initial chest open", "CHEST.C F0333:30-67");
    check_uint_eq_pc34_compat(counters, state.panelHashBeforeClose,
                              state.panelHashAfterClose,
                              "initial panel hash stable",
                              "PANEL.C F0346:1619-1657 / F0347");
    check_int_eq_pc34_compat(counters, state.f0280CandidateOpenCount, 1,
                             "initial candidate open count",
                             "REVIVE.C F0280:124-132");
    check_int_eq_pc34_compat(counters, state.f0359MirrorQueueWriteCount, 1,
                             "initial mirror queue write",
                             "COMMAND.C F0359:1985-1990");
    for (i = 0;
         i < DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];

        snprintf(label, sizeof(label), "initial visible C%d slot",
                 537 + i);
        check_int_eq_pc34_compat(counters, state.visibleC537ToC544[i],
                                 expected_visible_slot_pc34_compat(i),
                                 label, "CHEST.C F0334:117-132");
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
    const Dm1V1MirrorC040RedrawAfterChestCloseResultPc34Compat *result)
{
    int i;
    const Dm1V1MirrorC040RedrawAfterChestCloseEvidencePc34Compat *e =
        result->evidence;

    check_true_pc34_compat(counters, result->accepted,
                           "runtime close mutation accepted",
                           e->contractScope);
    check_uint_eq_pc34_compat(counters, result->deterministicHash,
                              kExpectedHash,
                              "pinned deterministic FNV-1a hash",
                              "COMMAND.C F0359:1985-1990");
    check_uint_eq_pc34_compat(counters, result->initialPanelHash,
                              result->finalPanelHash,
                              "C040 panel hash stable after chest close",
                              e->panelStateAnchor);
    check_int_eq_pc34_compat(counters, result->initialPanelOpen, 1,
                             "result captures initial live panel",
                             e->panelStateAnchor);
    check_int_eq_pc34_compat(counters, result->finalPanelOpen, 1,
                             "C040 panel remains live after close",
                             e->panelStateAnchor);
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
                              "G0299 candidate preserved after close",
                              e->candidateOpenAnchor);
    check_int_eq_pc34_compat(counters, result->initialOpenChestThing, 0x6400,
                             "initial G0426 open chest captured",
                             e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(
        counters,
        result->finalOpenChestThing,
        DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_NONE_PC34_COMPAT,
        "G0426 open chest cleared by close",
        e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(counters, result->initialChestOpen, 1,
                             "initial chest open flag",
                             e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(counters, result->finalChestOpen, 0,
                             "final chest open flag",
                             e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(counters, result->visibleSlotsCleared, 1,
                             "visible C537..C544 slots cleared",
                             e->chestVisibleCloseRewriteAnchor);
    check_int_eq_pc34_compat(counters, result->chestListStable, 1,
                             "G0425 chest list remains stable",
                             e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(counters, result->championHandStateStable, 1,
                             "champion C537..C544 hand-state stable",
                             e->championHandStateAnchor);
    check_int_eq_pc34_compat(counters, result->panelHashStable, 1,
                             "panel hash stable boolean",
                             e->panelStateAnchor);
    check_int_eq_pc34_compat(counters, result->candidateStillLive, 1,
                             "candidate remains live after close",
                             e->candidateOpenAnchor);
    check_int_eq_pc34_compat(counters, result->noPanelFlicker, 1,
                             "no panel flicker observed",
                             e->panelRedrawOnCloseAnchor);
    check_int_eq_pc34_compat(counters, result->noRedrawClobber, 1,
                             "no redraw clobber observed",
                             e->panelRedrawOnCloseAnchor);
    check_int_eq_pc34_compat(counters, result->noCandidateLeakage, 1,
                             "no candidate marker leaked into chest lists",
                             e->candidateCloseAnchor);
    check_int_eq_pc34_compat(counters, result->f0333OpenCloseCount, 1,
                             "F0333 close count",
                             e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(counters, result->f0334VisibleRewriteCount, 1,
                             "F0334 rewrite count",
                             e->chestVisibleCloseRewriteAnchor);
    check_int_eq_pc34_compat(counters, result->f0297HandStateCount, 2,
                             "F0297 hand-state count includes close",
                             e->championHandStateAnchor);
    check_int_eq_pc34_compat(counters, result->f0298OwnershipCount, 2,
                             "F0298 ownership count includes close",
                             e->championOwnershipAnchor);
    check_int_eq_pc34_compat(counters, result->f0300ListWalkCount, 2,
                             "F0300 list walk count includes close",
                             e->championListWalkAnchor);
    check_int_eq_pc34_compat(counters, result->f0301ListWalkCount, 2,
                             "F0301 list walk count includes close",
                             e->championListWalkAnchor);
    check_int_eq_pc34_compat(counters, result->f0302ListWalkCount, 2,
                             "F0302 list walk count includes close",
                             e->championListWalkAnchor);
    check_int_eq_pc34_compat(counters, result->f0284ChampionSwitchCount, 1,
                             "F0284 switch guard count",
                             e->championSwitchAnchor);
    check_int_eq_pc34_compat(counters, result->f0344PanelDrawHookCount, 2,
                             "F0344/F0345 draw hook count includes close",
                             e->panelDrawHookAnchor);
    check_int_eq_pc34_compat(counters, result->f0346PanelStateCount, 2,
                             "F0346/F0347 state count includes close",
                             e->panelStateAnchor);
    check_int_eq_pc34_compat(counters, result->f0352PanelRedrawOnCloseCount,
                             1, "F0352 redraw-on-close count",
                             e->panelRedrawOnCloseAnchor);
    check_int_eq_pc34_compat(counters, result->f0280CandidateOpenCount, 1,
                             "F0280 candidate open remains initial only",
                             e->candidateOpenAnchor);
    check_int_eq_pc34_compat(counters, result->f0282CandidateCloseCount, 0,
                             "F0282 candidate close not run by chest close",
                             e->candidateCloseAnchor);
    check_int_eq_pc34_compat(counters, result->f0359MirrorQueueWriteCount, 2,
                             "F0359 queue write includes close route",
                             e->mirrorQueueAnchor);

    for (i = 0;
         i < DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];

        snprintf(label, sizeof(label), "visible before C%d", 537 + i);
        check_int_eq_pc34_compat(counters, result->visibleBefore[i],
                                 expected_visible_slot_pc34_compat(i),
                                 label, e->chestVisibleCloseRewriteAnchor);
        snprintf(label, sizeof(label), "visible after C%d", 537 + i);
        check_int_eq_pc34_compat(
            counters,
            result->visibleAfter[i],
            DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_NONE_PC34_COMPAT,
            label,
            e->chestVisibleCloseRewriteAnchor);
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
                                 label, e->championHandStateAnchor);
        snprintf(label, sizeof(label), "champion hand after slot %d", i);
        check_int_eq_pc34_compat(counters, result->championHandAfter[i],
                                 expected_visible_slot_pc34_compat(i),
                                 label, e->championHandStateAnchor);
    }

    check_int_eq_pc34_compat(counters, result->rejectsNullState, 1,
                             "guard rejects null state",
                             e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsNullResult, 1,
                             "guard rejects null result",
                             e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsNonContract, 1,
                             "guard rejects non-contract state",
                             e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsNoPanel, 1,
                             "guard rejects no C040 panel",
                             e->panelStateAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsNoCandidate, 1,
                             "guard rejects no candidate",
                             e->candidateOpenAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsNoOpenChest, 1,
                             "guard rejects no G0426 chest",
                             e->chestListOpenCloseAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsWrongMouthRoute, 1,
                             "guard rejects wrong mouth route",
                             e->mirrorQueueAnchor);
    check_int_eq_pc34_compat(counters, result->rejectsCandidateLeakPreload, 1,
                             "guard rejects preloaded candidate leak",
                             e->candidateCloseAnchor);
    check_int_eq_pc34_compat(counters, result->mutationGuardsOk, 1,
                             "mutation guard matrix passed",
                             e->mirrorQueueAnchor);
}

static void test_run_sequence_pc34_compat(TestCounters *counters)
{
    Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat state;
    Dm1V1MirrorC040RedrawAfterChestCloseResultPc34Compat result;
    int ok;

    dm1_v1_mirror_candidate_c040_redraw_after_chest_close_init_pc34_compat(
        &state);
    ok = dm1_v1_mirror_candidate_c040_redraw_after_chest_close_run_pc34_compat(
        &state, &result);

    check_int_eq_pc34_compat(counters, ok, 1,
                             "run returns accepted",
                             "COMMAND.C F0359:1985-1990");
    check_run_result_pc34_compat(counters, &result);
    check_int_eq_pc34_compat(counters, state.c040PanelOpen, 1,
                             "state panel remains open after run",
                             "PANEL.C F0346:1619-1657 / F0347");
    check_uint_eq_pc34_compat(counters, state.g0299CandidateOrdinal, 4,
                              "state G0299 candidate remains live",
                              "REVIVE.C F0280:124-132");
    check_int_eq_pc34_compat(
        counters,
        state.g0426OpenChestThing,
        DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_NONE_PC34_COMPAT,
        "state G0426 open chest closed",
        "CHEST.C F0333:30-67");
    check_int_eq_pc34_compat(counters, state.chestOpen, 0,
                             "state chest open flag closed",
                             "CHEST.C F0333:30-67");
    check_uint_eq_pc34_compat(counters, state.panelHashBeforeClose,
                              state.panelHashAfterClose,
                              "state panel hash stable",
                              "PANEL.C F0352");
    check_int_eq_pc34_compat(counters, state.panelFlickerCount, 0,
                             "state flicker count",
                             "PANEL.C F0352");
    check_int_eq_pc34_compat(counters, state.redrawClobberCount, 0,
                             "state redraw clobber count",
                             "PANEL.C F0352");
    check_int_eq_pc34_compat(counters, state.candidateLeakCount, 0,
                             "state candidate leak count",
                             "REVIVE.C F0282:744-806");
}

static void test_rejects_invalid_inputs_pc34_compat(TestCounters *counters)
{
    Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat state;
    Dm1V1MirrorC040RedrawAfterChestCloseResultPc34Compat result;

    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c040_redraw_after_chest_close_run_pc34_compat(
            NULL, &result),
        0,
        "run rejects null state",
        "CHEST.C F0333:30-67");
    dm1_v1_mirror_candidate_c040_redraw_after_chest_close_init_pc34_compat(
        &state);
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c040_redraw_after_chest_close_run_pc34_compat(
            &state, NULL),
        0,
        "run rejects null result",
        "CHEST.C F0333:30-67");
    state.contractOnly = 0;
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c040_redraw_after_chest_close_run_pc34_compat(
            &state, &result),
        0,
        "run rejects non-contract state",
        "CHEST.C F0333:30-67");
    dm1_v1_mirror_candidate_c040_redraw_after_chest_close_init_pc34_compat(
        &state);
    state.c040PanelOpen = 0;
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c040_redraw_after_chest_close_run_pc34_compat(
            &state, &result),
        0,
        "run rejects no live C040 panel",
        "PANEL.C F0346:1619-1657 / F0347");
    dm1_v1_mirror_candidate_c040_redraw_after_chest_close_init_pc34_compat(
        &state);
    state.g0299CandidateOrdinal = 0;
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c040_redraw_after_chest_close_run_pc34_compat(
            &state, &result),
        0,
        "run rejects no G0299 candidate",
        "REVIVE.C F0280:124-132");
    dm1_v1_mirror_candidate_c040_redraw_after_chest_close_init_pc34_compat(
        &state);
    state.g0426OpenChestThing =
        DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_NONE_PC34_COMPAT;
    check_int_eq_pc34_compat(
        counters,
        dm1_v1_mirror_candidate_c040_redraw_after_chest_close_run_pc34_compat(
            &state, &result),
        0,
        "run rejects no G0426 chest",
        "CHEST.C F0333:30-67");
}

int main(void)
{
    TestCounters counters;

    counters.assertions = 0;
    counters.failures = 0;
    printf("=== DM1 V1 mirror-candidate C040 redraw after chest close ===\n");
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
    return counters.assertions >= 120 ? 0 : 1;
}
