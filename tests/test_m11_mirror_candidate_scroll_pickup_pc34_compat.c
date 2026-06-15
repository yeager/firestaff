/* ReDMCSB source-lock evidence:
 * CHEST.C F0333:31-67 opens the chest and materializes C537..C544/G0425.
 * PANEL.C F0342:1122-1134 closes prior chest state, dispatches scrolls to
 * F0341, and dispatches containers through CHEST.C F0333.
 * Mirror-candidate state path: COMMAND.C F0359:1985-1990 keeps C040/G0299
 * pending while panel refreshes route through the candidate mirror.
 */
#include "m11_mirror_candidate_scroll_pickup_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    kThingNone = M11_MIRROR_CANDIDATE_SCROLL_PICKUP_NONE_PC34_COMPAT,
    kOpenChestThing = 0x0901,
    kFullHandThing = 0x0501,
    kScrollThing = M11_MIRROR_CANDIDATE_SCROLL_PICKUP_C151_SCROLL_PC34_COMPAT,
    kCandidateOrdinal = 3,
    kPanelC040 = 40
};

typedef struct ScrollPickupRuntimePc34Compat {
    int mirrorState;
    int c040PanelOpen;
    int panelContent;
    int candidateChampionOrdinal;
    int openChestThing;
    int g0425[M11_MIRROR_CANDIDATE_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int floorScrollThing;
    int actionHandThing;
    int walkedOverScroll;
    int mirrorRefreshCount;
    int candidateIconRefreshCount;
    int scrollClickCount;
    int pickupCount;
    int closeCount;
    int closePreservedActionHand;
    int candidatePreserved;
    int refusedFullHandPickup;
} ScrollPickupRuntimePc34Compat;

static int gAssertions;
static int gFailures;

#define ANCHOR_SCROLL_PICKUP \
    "ReDMCSB CHEST.C F0333:31-67 C537..C544/G0425; " \
    "PANEL.C F0342:1122-1134 scroll/container panel dispatch; " \
    "mirror-candidate state path COMMAND.C F0359:1985-1990 C040/G0299"

static const M11MirrorCandidateScrollPickupProbePc34Compat s_probe = {
    M11_MIRROR_CANDIDATE_SCROLL_PICKUP_MIRROR_CHEST_OPEN_PC34_COMPAT,
    { 0x1537, 0x1538, 0x1539, 0x153A, 0x153B, 0x153C, 0x153D, 0x153E },
    M11_MIRROR_CANDIDATE_SCROLL_PICKUP_C151_SCROLL_PC34_COMPAT,
    1,
    M11_MIRROR_CANDIDATE_SCROLL_PICKUP_MIRROR_CLOSED_PC34_COMPAT,
    M11_MIRROR_CANDIDATE_SCROLL_PICKUP_C151_SCROLL_PC34_COMPAT,
    ANCHOR_SCROLL_PICKUP
};

static int anchor_mentions_required_paths(const char *anchor)
{
    return anchor && strstr(anchor, "CHEST.C F0333") &&
           strstr(anchor, "PANEL.C F0342") &&
           strstr(anchor, "mirror-candidate state path");
}

static void check_int(const char *label, int got, int want, const char *anchor)
{
    ++gAssertions;
    if (!anchor_mentions_required_paths(anchor)) {
        ++gFailures;
        printf("FAIL %s missing required anchor: %s\n",
               label, anchor ? anchor : "(null)");
        return;
    }
    if (got != want) {
        ++gFailures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
    }
}

static void check_true(const char *label, int condition, const char *anchor)
{
    check_int(label, condition ? 1 : 0, 1, anchor);
}

static void init_runtime(
    const M11MirrorCandidateScrollPickupProbePc34Compat *probe,
    ScrollPickupRuntimePc34Compat *runtime)
{
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->mirrorState = probe->initialMirrorState;
    runtime->c040PanelOpen = 1;
    runtime->panelContent = kPanelC040;
    runtime->candidateChampionOrdinal = kCandidateOrdinal;
    runtime->openChestThing = kOpenChestThing;
    runtime->floorScrollThing = probe->scrollOnFloor;
    runtime->actionHandThing = kThingNone;
    runtime->candidatePreserved = 1;
    for (i = 0; i < M11_MIRROR_CANDIDATE_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        runtime->g0425[i] = probe->initialChestSlots[i];
    }
}

static void mirror_refresh(ScrollPickupRuntimePc34Compat *runtime)
{
    ++runtime->mirrorRefreshCount;
    if (runtime->c040PanelOpen &&
        runtime->candidateChampionOrdinal == kCandidateOrdinal) {
        ++runtime->candidateIconRefreshCount;
    } else {
        runtime->candidatePreserved = 0;
    }
}

static void walk_party_over_scroll(ScrollPickupRuntimePc34Compat *runtime)
{
    runtime->walkedOverScroll = 1;
    mirror_refresh(runtime);
}

static int click_scroll_in_mirror(ScrollPickupRuntimePc34Compat *runtime)
{
    ++runtime->scrollClickCount;
    if (runtime->floorScrollThing != kScrollThing) {
        return 0;
    }
    if (runtime->actionHandThing != kThingNone) {
        runtime->refusedFullHandPickup = 1;
        mirror_refresh(runtime);
        return 0;
    }

    runtime->actionHandThing = runtime->floorScrollThing;
    runtime->floorScrollThing = kThingNone;
    ++runtime->pickupCount;
    runtime->mirrorState =
        M11_MIRROR_CANDIDATE_SCROLL_PICKUP_MIRROR_SCROLL_IN_HAND_PC34_COMPAT;
    mirror_refresh(runtime);
    return 1;
}

static void close_mirror(ScrollPickupRuntimePc34Compat *runtime)
{
    ++runtime->closeCount;
    runtime->mirrorState =
        M11_MIRROR_CANDIDATE_SCROLL_PICKUP_MIRROR_CLOSED_PC34_COMPAT;
    runtime->c040PanelOpen = 0;
    if (runtime->actionHandThing == kScrollThing) {
        runtime->closePreservedActionHand = 1;
    }
}

static void test_probe_metadata(
    const M11MirrorCandidateScrollPickupProbePc34Compat *probe)
{
    int i;

    check_int("initial mirror is chest-open",
              probe->initialMirrorState,
              M11_MIRROR_CANDIDATE_SCROLL_PICKUP_MIRROR_CHEST_OPEN_PC34_COMPAT,
              probe->anchorString);
    for (i = 0; i < M11_MIRROR_CANDIDATE_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[80];

        snprintf(label, sizeof(label), "initial C%d materialized",
                 M11_MIRROR_CANDIDATE_SCROLL_PICKUP_C537_PC34_COMPAT + i);
        check_int(label, probe->initialChestSlots[i], 0x1537 + i,
                  probe->anchorString);
    }
    check_int("scroll C151 is on the floor",
              probe->scrollOnFloor,
              M11_MIRROR_CANDIDATE_SCROLL_PICKUP_C151_SCROLL_PC34_COMPAT,
              probe->anchorString);
    check_int("expected pickup succeeds",
              probe->expectedPickup, 1, probe->anchorString);
    check_int("expected mirror closes after pickup path",
              probe->expectedMirrorStateAfter,
              M11_MIRROR_CANDIDATE_SCROLL_PICKUP_MIRROR_CLOSED_PC34_COMPAT,
              probe->anchorString);
    check_int("expected action hand receives scroll",
              probe->expectedActionHand,
              M11_MIRROR_CANDIDATE_SCROLL_PICKUP_C151_SCROLL_PC34_COMPAT,
              probe->anchorString);
}

static void test_scroll_pickup_route(
    const M11MirrorCandidateScrollPickupProbePc34Compat *probe)
{
    ScrollPickupRuntimePc34Compat runtime;
    int accepted;

    init_runtime(probe, &runtime);
    check_int("setup C040 panel open", runtime.c040PanelOpen, 1,
              probe->anchorString);
    check_int("setup panel content is mirror candidate C040",
              runtime.panelContent, kPanelC040, probe->anchorString);
    check_int("setup candidate ordinal is published",
              runtime.candidateChampionOrdinal, kCandidateOrdinal,
              probe->anchorString);
    check_int("setup leader action hand empty",
              runtime.actionHandThing, kThingNone, probe->anchorString);
    check_int("setup open chest thing set",
              runtime.openChestThing, kOpenChestThing, probe->anchorString);

    walk_party_over_scroll(&runtime);
    accepted = click_scroll_in_mirror(&runtime);
    close_mirror(&runtime);

    check_int("walk over scroll triggers mirror refresh",
              runtime.walkedOverScroll, 1, probe->anchorString);
    check_int("scroll click accepted", accepted, probe->expectedPickup,
              probe->anchorString);
    check_int("scroll pickup count", runtime.pickupCount, 1,
              probe->anchorString);
    check_int("floor scroll removed after pickup",
              runtime.floorScrollThing, kThingNone, probe->anchorString);
    check_int("action hand receives picked scroll",
              runtime.actionHandThing, probe->expectedActionHand,
              probe->anchorString);
    check_int("mirror refreshes after walk and after scroll click",
              runtime.mirrorRefreshCount, 2, probe->anchorString);
    check_int("candidate icon refreshes twice",
              runtime.candidateIconRefreshCount, 2, probe->anchorString);
    check_int("candidate remains published through pickup",
              runtime.candidateChampionOrdinal, kCandidateOrdinal,
              probe->anchorString);
    check_true("mirror candidate state not dropped",
               runtime.candidatePreserved, probe->anchorString);
    check_int("mirror close path dispatched once",
              runtime.closeCount, 1, probe->anchorString);
    check_int("mirror state after close",
              runtime.mirrorState, probe->expectedMirrorStateAfter,
              probe->anchorString);
    check_true("close path keeps picked-up scroll in action hand",
               runtime.closePreservedActionHand, probe->anchorString);
}

static void test_full_hand_negative(
    const M11MirrorCandidateScrollPickupProbePc34Compat *probe)
{
    ScrollPickupRuntimePc34Compat runtime;
    int accepted;

    init_runtime(probe, &runtime);
    runtime.actionHandThing = kFullHandThing;

    walk_party_over_scroll(&runtime);
    accepted = click_scroll_in_mirror(&runtime);

    check_int("full-hand scroll pickup is rejected",
              accepted, 0, probe->anchorString);
    check_int("full-hand negative records refusal",
              runtime.refusedFullHandPickup, 1, probe->anchorString);
    check_int("full-hand action item preserved",
              runtime.actionHandThing, kFullHandThing, probe->anchorString);
    check_int("full-hand floor scroll not consumed",
              runtime.floorScrollThing, kScrollThing, probe->anchorString);
    check_int("full-hand mirror stays chest-open",
              runtime.mirrorState,
              M11_MIRROR_CANDIDATE_SCROLL_PICKUP_MIRROR_CHEST_OPEN_PC34_COMPAT,
              probe->anchorString);
    check_int("full-hand candidate remains published",
              runtime.candidateChampionOrdinal, kCandidateOrdinal,
              probe->anchorString);
    check_true("full-hand does not clobber mirror candidate state",
               runtime.candidatePreserved, probe->anchorString);
    check_int("full-hand still refreshes candidate icon",
              runtime.candidateIconRefreshCount, 2, probe->anchorString);
}

int M11_MirrorCandidateScrollPickup_RunPc34Compat(
    const M11MirrorCandidateScrollPickupProbePc34Compat *probe,
    int *passed,
    int *failed)
{
    int startAssertions;
    int startFailures;

    if (!probe) {
        return 0;
    }

    startAssertions = gAssertions;
    startFailures = gFailures;
    test_probe_metadata(probe);
    test_scroll_pickup_route(probe);
    test_full_hand_negative(probe);

    if (passed) {
        *passed = (gAssertions - startAssertions) - (gFailures - startFailures);
    }
    if (failed) {
        *failed = gFailures - startFailures;
    }
    return (gFailures == startFailures) ? 1 : 0;
}

int main(void)
{
    int passed = 0;
    int failed = 0;
    int ok = M11_MirrorCandidateScrollPickup_RunPc34Compat(
        &s_probe, &passed, &failed);

    if (!ok || failed != 0) {
        printf("m11_mirror_candidate_scroll_pickup_pc34_compat failed: "
               "%d passed, %d failed\n",
               passed, failed);
        return 1;
    }

    printf("m11_mirror_candidate_scroll_pickup_pc34_compat passed: "
           "%d assertions\n",
           passed);
    return 0;
}
