#include "firestaff/dm1/v1/mirror_candidate/eye_slot_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++g_assertions;
    if (!condition) {
        ++g_failures;
        printf("FAIL %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int(int actual, int expected, const char *message,
                      const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d [%s]\n",
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

int main(void)
{
    DM1_V1_MirrorCandidateEyeSlotSwapResultPc34 result;
    const DM1_V1_MirrorCandidateEyeSlotSwapSpecPc34 *spec =
        dm1_v1_mirror_candidate_eye_slot_swap_spec_pc34();
    const char *source = dm1_v1_mirror_candidate_eye_slot_swap_source_evidence_pc34();
    const char *nonoverlap = dm1_v1_mirror_candidate_eye_slot_swap_non_overlap_pc34();

    check_true(spec != NULL, "spec is available",
               "PANEL.C F0352:2111-2159");
    check_contains(source, "PANEL.C F0352:2111-2159",
                   "source evidence cites eye press path",
                   spec->f0352Anchor);
    check_contains(source, "PANEL.C F0353:2162-2192",
                   "source evidence cites eye release path",
                   spec->f0353Anchor);
    check_contains(source, "PANEL.C F0342:1055-1180",
                   "source evidence cites F0342 object draw",
                   spec->f0342Anchor);
    check_contains(source, "PANEL.C F0347:1639-1693",
                   "source evidence cites F0347 always-close at 1650",
                   spec->f0347Anchor);
    check_contains(source, "PANEL.C F0346:1619-1637",
                   "source evidence cites C040 redraw",
                   spec->f0346Anchor);
    check_contains(source, "CHEST.C F0333:30-67",
                   "source evidence cites F0333 chest open",
                   spec->f0333Anchor);
    check_contains(source, "CHEST.C F0334:79-130",
                   "source evidence cites F0334 chest close",
                   spec->f0334Anchor);
    check_contains(nonoverlap, "C160 close",
                   "non-overlap names C160 disjoint",
                   "PANEL.C F0353:2162-2192");
    check_contains(nonoverlap, "C045",
                   "non-overlap names C045 disjoint",
                   "CHEST.C F0333:30-67");
    check_contains(nonoverlap, "F0319",
                   "non-overlap names F0319 leader death disjoint",
                   "CHEST.C F0334:79-130");
    check_contains(nonoverlap, "C040_eye_live_candidate",
                   "non-overlap names the existing eye-live sibling",
                   "CHEST.C F0333:30-67");
    check_int(spec->c040PanelContent, 568, "M568 panel content id",
              spec->f0346Anchor);
    check_int(spec->chestPanelContent, 569, "M569 panel content id",
              spec->f0347Anchor);
    check_int(spec->c546EyeZone, 546, "C546 eye zone id",
              spec->f0352Anchor);
    check_int(spec->eyeLookingGraphic, 203, "C203 eye looking id",
              spec->f0352Anchor);
    check_int(spec->eyeNotLookingGraphic, 202, "C202 eye not looking id",
              spec->f0353Anchor);
    check_int(spec->c144ClosedChestIcon, 144, "C144 closed chest icon id",
              spec->f0333Anchor);
    check_int(spec->c145OpenChestIcon, 145, "C145 open chest icon id",
              spec->f0333Anchor);
    check_int(spec->c09ThingTypeContainer, 9, "C09 container type id",
              spec->f0342Anchor);

    check_true(dm1_v1_mirror_candidate_eye_slot_swap_run_pc34(&result) == 1,
               "runtime contract returns accepted",
               "PANEL.C F0353:2162-2192");

    /* Assertion count guard. */
    check_int(result.assertionCount, 24, "model assertion count",
              "PANEL.C F0353:2162-2192");

    /* C040 candidate is byte-stable across press/release. */
    check_int(result.candidateOrdinalBefore, 3,
              "fixture starts with a live nonzero G0299",
              "REVIVE.C F0280:124-132");
    check_int(result.candidateOrdinalAfterPress,
              result.candidateOrdinalBefore,
              "eye press does not consume G0299",
              "PANEL.C F0352:2111-2159");
    check_int(result.candidateOrdinalAfterRelease,
              result.candidateOrdinalBefore,
              "eye release does not consume G0299",
              "PANEL.C F0353:2162-2192");

    /* G0424 panel-content slot-swap: 568 -> 569 -> 568. */
    check_int(result.panelContentBefore, 568, "fixture starts on C040",
              "PANEL.C F0346:1619-1637");
    check_int(result.panelContentAfterPress, 569,
              "eye press swaps panel content to CHEST",
              "PANEL.C F0342:1055-1180");
    check_int(result.panelContentAfterRelease, 568,
              "eye release restores C040 panel",
              "PANEL.C F0347:1639-1693");

    /* G0426 + v1OpenChestOpenedByEye slot-swap. */
    check_int(result.openChestBefore, 0x2639,
              "fixture starts with a different prior open chest",
              "CHEST.C F0334:79-130");
    check_int(result.openChestAfterPress, 0x2638,
              "eye press opens leader-hand chest",
              "CHEST.C F0333:30-67");
    check_int(result.openChestAfterRelease, -1,
              "eye release auto-closes chest via F0347->F0334",
              "PANEL.C F0347:1639-1693");
    check_int(result.openedByEyeAfterPress, 1,
              "eye press leaves v1OpenChestOpenedByEye = 1",
              "CHEST.C F0333:43-46");
    check_int(result.openedByEyeAfterRelease, 0,
              "eye release clears v1OpenChestOpenedByEye",
              "CHEST.C F0334:79-130");

    /* G0425 chest-slot chain. */
    check_int(result.chestSlotsPopulatedOnPress, 8,
              "F0333 populates 8 G0425 slots from the leader-hand chest",
              "CHEST.C F0333:30-67");
    check_true(result.chestSlotsClearedOnRelease,
               "F0347->F0334 clears G0425 slots on release",
               "CHEST.C F0334:79-130");

    /* C09 action-hand icon suppression. */
    check_int(result.c09IconBefore, 145,
              "fixture starts with prior chest open icon (C145)",
              "CHEST.C F0333:43-46");
    check_int(result.c09IconAfterPress, 144,
              "eye press suppresses C145 -> C144 (closed)",
              "CHEST.C F0333:43-46");
    check_int(result.c09IconAfterRelease, 144,
              "eye release keeps C144 (closed, no C145 redraw)",
              "CHEST.C F0333:43-46");

    /* Eye-zone icon redraw. */
    check_int(result.eyeIconBefore, 202, "fixture starts with C202",
              "PANEL.C F0353:2162-2192");
    check_int(result.eyeIconAfterPress, 203,
              "eye press draws C203 (looking)",
              "PANEL.C F0352:2111-2159");
    check_int(result.eyeIconAfterRelease, 202,
              "eye release draws C202 (not looking)",
              "PANEL.C F0353:2162-2192");

    /* G0331_B_PressingEye flag. */
    check_int(result.pressingEyeAfterPress, 1,
              "press path sets pressing-eye state",
              "PANEL.C F0352:2111-2159");
    check_int(result.pressingEyeAfterRelease, 0,
              "release path clears pressing-eye state",
              "PANEL.C F0353:2162-2192");

    /* Pointer/mouse ignore flags. */
    check_int(result.pointerHiddenAfterPress, 1,
              "press path hides pointer",
              "PANEL.C F0352:2111-2159");
    check_int(result.pointerHiddenAfterRelease, 0,
              "release path shows pointer",
              "PANEL.C F0353:2162-2192");
    check_int(result.ignoreMouseAfterPress, 1,
              "press path sets ignore-mouse-movements",
              "PANEL.C F0352:2111-2159");
    check_int(result.ignoreMouseAfterRelease, 0,
              "release path clears ignore-mouse-movements",
              "PANEL.C F0353:2162-2192");

    /* F0333 + F0334 dispatch counts. */
    check_true(result.f0333CalledOnce,
               "F0333_INVENTORY_OpenAndDrawChest fires once (on press)",
               "CHEST.C F0333:30-67");
    check_true(result.f0334CalledOnce,
               "F0334_INVENTORY_CloseChest fires twice (press prior + release)",
               "CHEST.C F0334:79-130");
    check_true(result.objectPanelDrawnOnPress,
               "F0342 object-panel draw fires once on press",
               "PANEL.C F0342:1055-1180");
    check_true(result.c040RedrawnOnRelease,
               "F0346 C040 redraw fires once on release",
               "PANEL.C F0346:1619-1637");
    check_true(result.leaderHandPreserved,
               "leader hand object is byte-stable across both legs",
               "PANEL.C F0352:2111-2159");
    check_int(result.viewportRedrawCount, 2,
              "F0352 + F0353 viewport redraws balance (2)",
              "PANEL.C F0353:2162-2192");

    if (g_failures) {
        printf("FAIL dm1_v1_mirror_candidate_eye_slot_swap_pc34_compat "
               "assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS dm1_v1_mirror_candidate_eye_slot_swap_pc34_compat "
           "assertions=%d\n",
           g_assertions);
    return 0;
}
