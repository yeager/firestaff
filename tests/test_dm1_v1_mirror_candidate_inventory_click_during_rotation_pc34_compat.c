#include "dm1_v1_mirror_candidate_inventory_click_during_rotation_pc34_compat.h"

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

static const char *anchor_for_index(size_t index)
{
    const Dm1V1MirrorCandidateInventoryClickDuringRotationCasePc34Compat
        *testCase = 0;

    if (dm1_v1_mirror_candidate_inventory_click_during_rotation_case_at(
            index, &testCase) &&
        testCase) {
        return testCase->sourceAnchor;
    }
    return "case index anchor unavailable";
}

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateInventoryClickDuringRotationEvidencePc34Compat
        *e = dm1_v1_mirror_candidate_inventory_click_during_rotation_evidence();

    CHECK_REDMCSB(e != 0,
                  "source-lock evidence is available",
                  "metadata");
    CHECK_REDMCSB(e->contractOnly == 1 &&
                      strstr(e->contractScope, "contract_only=1") != 0,
                  "fixture is explicitly contract-only",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->championPartyDirectionAnchor, "F0284") != 0 &&
                      strstr(e->championPartyDirectionAnchor, "93-131") != 0 &&
                      strstr(e->championPartyDirectionAnchor, "C156/C157") != 0,
                  "CHAMPION.C SetPartyDirection anchor is cited",
                  e->championPartyDirectionAnchor);
    CHECK_REDMCSB(strstr(e->commandPanelDispatchAnchor, "F0359") != 0 &&
                      strstr(e->commandPanelDispatchAnchor, "1985-1990") != 0 &&
                      strstr(e->commandPanelDispatchAnchor, "M568/C040") != 0,
                  "COMMAND.C mirror-panel dispatch anchor is cited",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(strstr(e->mouseClickQueueAnchor, "INPUT.C:641-664") != 0 &&
                      strstr(e->mouseClickQueueAnchor, "IO.C") != 0 &&
                      strstr(e->mouseClickQueueAnchor, "1102-1122") != 0,
                  "mouse/click queue and update guard anchors are cited",
                  e->mouseClickQueueAnchor);
    CHECK_REDMCSB(strstr(e->championRedrawAnchor, "F0293") != 0 &&
                      strstr(e->championRedrawAnchor, "1117-1143") != 0,
                  "CHAMDRAW.C champion-state redraw anchor is cited",
                  e->championRedrawAnchor);
    CHECK_REDMCSB(strstr(e->panelPortraitBlitAnchor, "F0354") != 0 &&
                      strstr(e->panelPortraitBlitAnchor, "2208-2240") != 0,
                  "PANEL.C portrait blit anchor is cited",
                  e->panelPortraitBlitAnchor);
}

static void test_static_fixture_table(void)
{
    size_t count = 0u;
    const Dm1V1MirrorCandidateInventoryClickDuringRotationCasePc34Compat
        *table =
            dm1_v1_mirror_candidate_inventory_click_during_rotation_table(
                &count);

    CHECK_REDMCSB(table != 0,
                  "rotation/click fixture table is available",
                  "table metadata");
    CHECK_REDMCSB(count == 8u,
                  "rotation/click fixture table has exactly 8 rows",
                  "table metadata");
    CHECK_REDMCSB(
        table ==
            dm1_v1_mirror_candidate_inventory_click_during_rotation_table(0),
        "fixture table accessor is NULL-count safe",
        "table metadata");
    CHECK_REDMCSB(
        table[0].rotationInProgress ==
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C156_TURN_LEFT_PC34_COMPAT &&
            table[0].clickType ==
                DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT &&
            table[0].clickTarget ==
                DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT,
        "row 0 covers left-click portrait during C156",
        anchor_for_index(0));
    CHECK_REDMCSB(
        table[1].clickType ==
            DM1_V1_MIRROR_CANDIDATE_CLICK_RIGHT_PC34_COMPAT &&
            table[1].clickTarget ==
                DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT,
        "row 1 covers right-click portrait during rotation",
        anchor_for_index(1));
    CHECK_REDMCSB(
        table[2].clickType ==
            DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT &&
            table[2].clickTarget ==
                DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_ACTION_HAND_SLOT_PC34_COMPAT,
        "row 2 covers left-click action-hand slot during rotation",
        anchor_for_index(2));
    CHECK_REDMCSB(
        table[3].rotationInProgress ==
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C157_TURN_RIGHT_PC34_COMPAT &&
            table[3].clickType ==
                DM1_V1_MIRROR_CANDIDATE_CLICK_RIGHT_PC34_COMPAT &&
            table[3].clickTarget ==
                DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_ACTION_HAND_SLOT_PC34_COMPAT,
        "row 3 covers right-click action-hand slot during C157",
        anchor_for_index(3));
    CHECK_REDMCSB(
        table[4].rotationInProgress ==
            DM1_V1_MIRROR_CANDIDATE_ROTATION_PAUSED_PC34_COMPAT,
        "row 4 covers click during paused rotation",
        anchor_for_index(4));
    CHECK_REDMCSB(
        table[5].rotationInProgress ==
            DM1_V1_MIRROR_CANDIDATE_ROTATION_COMPLETE_PC34_COMPAT,
        "row 5 covers click after rotation completes",
        anchor_for_index(5));
    CHECK_REDMCSB(
        table[6].clickTarget ==
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_CLOSED_INVENTORY_PC34_COMPAT,
        "row 6 covers closed-inventory click during rotation",
        anchor_for_index(6));
    CHECK_REDMCSB(
        table[7].clickTarget ==
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_OPEN_INVENTORY_PC34_COMPAT,
        "row 7 covers open-inventory click during rotation",
        anchor_for_index(7));
}

static void test_table_decisions(void)
{
    size_t i;
    size_t count = 0u;
    const Dm1V1MirrorCandidateInventoryClickDuringRotationCasePc34Compat
        *table =
            dm1_v1_mirror_candidate_inventory_click_during_rotation_table(
                &count);

    for (i = 0u; i < count; ++i) {
        Dm1V1MirrorCandidateInventoryClickDecisionPc34Compat decision =
            dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
                table[i].rotationInProgress,
                table[i].clickType,
                table[i].clickTarget);
        CHECK_REDMCSB(decision == table[i].expectedDecision,
                      table[i].caseName,
                      table[i].sourceAnchor);
    }
}

static void test_required_runtime_edges(void)
{
    Dm1V1MirrorCandidateInventoryClickDecisionPc34Compat first;
    Dm1V1MirrorCandidateInventoryClickDecisionPc34Compat second;

    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C156_TURN_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT) ==
            DM1_V1_MIRROR_CANDIDATE_SUPPRESS_DURING_ROTATION_PC34_COMPAT,
        "left-click portrait during in-progress C156 is suppressed",
        "CHAMPION.C F0284:93-131; PANEL.C F0354:2208-2240");
    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C157_TURN_RIGHT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_RIGHT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_ACTION_HAND_SLOT_PC34_COMPAT) ==
            DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT,
        "right-click action-hand slot during C157 dispatches normally",
        "INPUT.C:663-664; COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C157_TURN_RIGHT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_RIGHT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT) ==
            DM1_V1_MIRROR_CANDIDATE_QUEUE_FOR_AFTER_ROTATION_PC34_COMPAT,
        "right-click portrait during in-progress rotation is queued",
        "INPUT.C:663-664; PANEL.C F0354:2208-2240");
    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C156_TURN_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_CLOSED_INVENTORY_PC34_COMPAT) ==
            DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT,
        "left-click closed inventory during rotation dispatches normally",
        "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
            DM1_V1_MIRROR_CANDIDATE_ROTATION_COMPLETE_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT) ==
            DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT,
        "left-click after rotation completes dispatches normally",
        "CHAMPION.C F0284:129-130");
    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
            DM1_V1_MIRROR_CANDIDATE_ROTATION_COMPLETE_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_RIGHT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_OPEN_INVENTORY_PC34_COMPAT) ==
            DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT,
        "right-click after rotation completes dispatches normally",
        "CHAMPION.C F0284:129-130; COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
            DM1_V1_MIRROR_CANDIDATE_ROTATION_COMPLETE_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_MIDDLE_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_ACTION_HAND_SLOT_PC34_COMPAT) ==
            DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT,
        "middle-click after rotation completes dispatches normally",
        "CHAMPION.C F0284:129-130; COMMAND.C F0359:1985-1990");

    first = dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
        DM1_V1_MIRROR_CANDIDATE_ROTATION_C157_TURN_RIGHT_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CLICK_RIGHT_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT);
    second = dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
        DM1_V1_MIRROR_CANDIDATE_ROTATION_C157_TURN_RIGHT_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CLICK_RIGHT_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT);
    CHECK_REDMCSB(first == second,
                  "same rotation/click tuple returns same decision",
                  "COMMAND.C F0359:1985-1990 deterministic dispatch");
}

static void test_safety_and_rejection(void)
{
    const Dm1V1MirrorCandidateInventoryClickDuringRotationCasePc34Compat
        *testCase = 0;

    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_case_at(
            0u, 0) == 0,
        "case accessor rejects NULL output pointer",
        "table metadata");
    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_case_at(
            99u, &testCase) == 0 &&
            testCase == 0,
        "case accessor rejects out-of-range index",
        "table metadata");
    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_case_at(
            0u, &testCase) == 1 &&
            testCase != 0,
        "case accessor returns first row safely",
        anchor_for_index(0));
    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C156_TURN_LEFT_PC34_COMPAT,
            0,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT) ==
            DM1_V1_MIRROR_CANDIDATE_REJECT_INVALID_CLICK_TYPE_PC34_COMPAT,
        "clickType below LEFT_CLICK is rejected",
        "INPUT.C:641-664");
    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C156_TURN_LEFT_PC34_COMPAT,
            4,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT) ==
            DM1_V1_MIRROR_CANDIDATE_REJECT_INVALID_CLICK_TYPE_PC34_COMPAT,
        "clickType above MIDDLE_CLICK is rejected",
        "INPUT.C:641-664");
    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C157_TURN_RIGHT_PC34_COMPAT,
            -7,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_ACTION_HAND_SLOT_PC34_COMPAT) ==
            DM1_V1_MIRROR_CANDIDATE_REJECT_INVALID_CLICK_TYPE_PC34_COMPAT,
        "negative clickType is rejected",
        "INPUT.C:641-664");
}

int main(void)
{
    test_source_lock_metadata();
    test_static_fixture_table();
    test_table_decisions();
    test_required_runtime_edges();
    test_safety_and_rejection();

    printf("PASS dm1_v1_mirror_candidate_inventory_click_during_rotation_"
           "pc34_compat %d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
