#include "dm1_v1_chest_scroll_wheel_full_leader_partial_chest_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int check_int(const char* label, int got, int want, const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int check_contains(const char* label,
                          const char* haystack,
                          const char* needle,
                          const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || !haystack || !needle ||
        !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, anchor);
    return 1;
}

#define CHECK_INT(label, got, want, anchor) \
    check_int((label), (int)(got), (int)(want), (anchor))
#define CHECK_CONTAINS(label, haystack, needle, anchor) \
    check_contains((label), (haystack), (needle), (anchor))

static const char* const k_f0302_anchor =
    "ReDMCSB CHAMPION.C F0302:662-710 snapshots G4055, reads C30+ chest slots, "
    "and suppresses empty/empty or disallowed movement.";
static const char* const k_f0297_anchor =
    "ReDMCSB CHAMPION.C F0297:243-268 and F0298:270-298 own leader-hand "
    "put/remove, pointer/icon identity, and leader load.";
static const char* const k_chest_anchor =
    "ReDMCSB CHEST.C F0333:30-67 materializes partial C537/C538 chest slots; "
    "F0334:117-132 rewrites non-empty visible cells on close.";
static const char* const k_panel_anchor =
    "ReDMCSB PANEL.C F0344:1895-1944 + F0345:1946-1999 and COMMAND.C "
    "F0359:1985-1990 route panel clicks and scroll-wheel highlight focus.";
static const char* const k_mouse_anchor =
    "ReDMCSB MOUSE.C F0077:97-126 + F0078:128-168 wheel queue lineage.";

static int test_source_evidence(void)
{
    const char* evidence =
        dm1_v1_chest_scroll_wheel_composite_source_evidence_pc34();
    int ok = 1;

    ok &= CHECK_CONTAINS("evidence F0333", evidence, "CHEST.C F0333:30-67",
                         k_chest_anchor);
    ok &= CHECK_CONTAINS("evidence F0334", evidence, "F0334:117-132",
                         k_chest_anchor);
    ok &= CHECK_CONTAINS("evidence F0297", evidence, "F0297:243-268",
                         k_f0297_anchor);
    ok &= CHECK_CONTAINS("evidence F0298", evidence, "F0298:270-298",
                         k_f0302_anchor);
    ok &= CHECK_CONTAINS("evidence F0302", evidence, "F0302:662-710",
                         k_f0302_anchor);
    ok &= CHECK_CONTAINS("evidence PANEL", evidence, "PANEL.C F0344:1895-1944",
                         k_panel_anchor);
    ok &= CHECK_CONTAINS("evidence COMMAND", evidence,
                         "COMMAND.C F0359:1985-1990", k_panel_anchor);
    ok &= CHECK_CONTAINS("evidence MOUSE", evidence, "MOUSE.C F0077:97-126",
                         k_mouse_anchor);
    ok &= CHECK_CONTAINS("evidence OBJECT", evidence, "OBJECT.C F0033:147-212",
                         k_f0302_anchor);
    ok &= CHECK_CONTAINS("evidence BLITMASK", evidence,
                         "BLITMASK.C F0133:30-33", k_panel_anchor);
    ok &= CHECK_CONTAINS("evidence DEFS", evidence, "DEFS.H:810",
                         k_chest_anchor);
    return ok;
}

static int test_fixture_table(void)
{
    const DM1_V1_ChestScrollWheelCompositeCasePc34* cases;
    size_t count = 0;
    int fullEmpty = 0;
    int fullOccupied = 0;
    int emptyEmpty = 0;
    int emptyOccupied = 0;
    size_t i;
    int ok = 1;

    cases = dm1_v1_chest_scroll_wheel_composite_cases(&count);
    ok &= CHECK_INT("fixture count", count, 16, k_chest_anchor);
    ok &= CHECK_INT("fixture null-count pointer stable",
                     dm1_v1_chest_scroll_wheel_composite_cases(NULL) == cases,
                     1, k_chest_anchor);
    ok &= CHECK_INT("C30 base", DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_C30, 30,
                    k_f0302_anchor);
    ok &= CHECK_INT("C537 zone", DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_C537, 537,
                    k_chest_anchor);
    ok &= CHECK_INT("C538 zone", DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_C538, 538,
                    k_chest_anchor);
    ok &= CHECK_INT("C544 zone", DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_C544, 544,
                    k_chest_anchor);

    for (i = 0; i < count; ++i) {
        char label[96];
        DM1_V1_ChestScrollWheelCompositeDecision decision =
            dm1_v1_chest_scroll_wheel_composite_decide(
                cases[i].leaderHandFull,
                cases[i].chestSlotCount,
                cases[i].chestSlotOccupied,
                cases[i].scrollDirection,
                cases[i].targetChampionIndex);

        snprintf(label, sizeof(label), "fixture case %u", (unsigned)i);
        ok &= CHECK_INT(label, decision, cases[i].expected, k_f0302_anchor);
        fullEmpty += cases[i].leaderHandFull && !cases[i].chestSlotOccupied;
        fullOccupied += cases[i].leaderHandFull && cases[i].chestSlotOccupied;
        emptyEmpty += !cases[i].leaderHandFull && !cases[i].chestSlotOccupied;
        emptyOccupied +=
            !cases[i].leaderHandFull && cases[i].chestSlotOccupied;
    }
    ok &= CHECK_INT("leader-full slot-N cases", fullEmpty, 4, k_f0297_anchor);
    ok &= CHECK_INT("leader-full slot-Y cases", fullOccupied, 4,
                    k_f0297_anchor);
    ok &= CHECK_INT("leader-empty slot-N cases", emptyEmpty, 4,
                    k_f0302_anchor);
    ok &= CHECK_INT("leader-empty slot-Y cases", emptyOccupied, 4,
                    k_f0302_anchor);
    return ok;
}

static int test_composite_routes(void)
{
    DM1_V1_ChestScrollWheelCompositeDecision first;
    DM1_V1_ChestScrollWheelCompositeDecision second;
    int ok = 1;

    ok &= CHECK_INT("leader-full empty up",
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        1, 2, 0, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 0),
                    DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL,
                    k_f0297_anchor);
    ok &= CHECK_INT("leader-full occupied down",
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        1, 2, 1, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 1),
                    DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL,
                    k_f0297_anchor);
    ok &= CHECK_INT("leader-full full chest none",
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        1, 8, 1, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_NONE, 0),
                    DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL,
                    k_f0297_anchor);
    ok &= CHECK_INT("leader-empty chest-full occupied",
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        0, 8, 1, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 0),
                    DM1_V1_CHEST_SCROLL_WHEEL_FOCUS_CHEST_SLOT,
                    k_chest_anchor);
    ok &= CHECK_INT("leader-empty partial occupied",
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        0, 2, 1, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 0),
                    DM1_V1_CHEST_SCROLL_WHEEL_DISPATCH_PICKUP,
                    k_f0302_anchor);
    ok &= CHECK_INT("leader-empty partial occupied down",
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        0, 2, 1, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 1),
                    DM1_V1_CHEST_SCROLL_WHEEL_DISPATCH_PICKUP,
                    k_f0302_anchor);
    ok &= CHECK_INT("leader-empty chest-empty scroll-up",
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        0, 0, 0, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 0),
                    DM1_V1_CHEST_SCROLL_WHEEL_SCROLL_TO_NEXT_SLOT,
                    k_mouse_anchor);
    ok &= CHECK_INT("leader-empty chest-empty scroll-down",
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        0, 0, 0, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 1),
                    DM1_V1_CHEST_SCROLL_WHEEL_SCROLL_TO_NEXT_SLOT,
                    k_mouse_anchor);
    ok &= CHECK_INT("leader-empty chest-empty scroll-none",
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        0, 0, 0, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_NONE, 0),
                    DM1_V1_CHEST_SCROLL_WHEEL_FOCUS_CHEST_SLOT,
                    k_panel_anchor);
    ok &= CHECK_INT("leader-empty partial occupied scroll-none",
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        0, 2, 1, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_NONE, 1),
                    DM1_V1_CHEST_SCROLL_WHEEL_DISPATCH_PICKUP,
                    k_f0302_anchor);
    first = dm1_v1_chest_scroll_wheel_composite_decide(
        0, 2, 1, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 1);
    second = dm1_v1_chest_scroll_wheel_composite_decide(
        0, 2, 1, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 1);
    ok &= CHECK_INT("same tuple repeatable", first, second, k_f0302_anchor);
    ok &= CHECK_INT("champion 0 same as champion 1",
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        0, 2, 1, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 0),
                    dm1_v1_chest_scroll_wheel_composite_decide(
                        0, 2, 1, DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 1),
                    k_chest_anchor);
    return ok;
}

int main(void)
{
    int ok = 1;

    ok &= test_source_evidence();
    ok &= test_fixture_table();
    ok &= test_composite_routes();

    printf("SUMMARY assertions=%d failures=%d\n", g_assertions, g_failures);
    return (ok && g_failures == 0) ? 0 : 1;
}
