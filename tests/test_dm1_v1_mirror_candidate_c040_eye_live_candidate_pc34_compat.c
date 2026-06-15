#include "firestaff/dm1/v1/mirror/dm1_v1_mirror_candidate_c040_eye_live_candidate_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gAssertions;
static int gFailures;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++gAssertions;
    if (!condition) {
        ++gFailures;
        printf("FAIL %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int(int actual, int expected, const char *message,
                      const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL %s actual=%d expected=%d [%s]\n",
               message, actual, expected, anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++gAssertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++gFailures;
        printf("FAIL %s missing=%s [%s]\n",
               message, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

int main(void)
{
    DM1_V1_MirrorCandidateC040EyeLiveCandidateResultPc34 result;
    const DM1_V1_MirrorCandidateC040EyeLiveCandidateSpecPc34 *spec =
        dm1_v1_mirror_candidate_c040_eye_live_candidate_spec_pc34();

    check_true(spec != NULL, "spec is available",
               "REVIVE.C F0280:124-132");
    check_contains(spec->sourceEvidence, "REVIVE.C F0280:124-132",
                   "source evidence cites C040 candidate activation",
                   spec->f0280Anchor);
    check_contains(spec->sourceEvidence, "PANEL.C F0352:2123-2159",
                   "source evidence cites eye press",
                   spec->f0352Anchor);
    check_contains(spec->sourceEvidence, "PANEL.C F0353:2174-2192",
                   "source evidence cites eye release",
                   spec->f0353Anchor);
    check_contains(spec->sourceEvidence, "PANEL.C F0347:1651-1656",
                   "source evidence cites C040 redraw priority",
                   spec->f0347Anchor);
    check_contains(spec->sourceEvidence, "CHEST.C F0334:112-132",
                   "source evidence cites close-chest clear",
                   spec->f0334Anchor);
    check_contains(spec->sourceEvidence, "COMMAND.C F0359:1985-1990",
                   "source evidence cites direct C040 dispatch boundary",
                   spec->f0359Anchor);
    check_contains(spec->nonOverlap, "C546 eye press/release",
                   "non-overlap names the eye-route slice",
                   spec->f0352Anchor);
    check_int(spec->c040PanelContent, 568, "C040 panel content id",
              spec->f0347Anchor);
    check_int(spec->c040PanelGraphic, 40, "C040 graphic id",
              spec->f0347Anchor);
    check_int(spec->c546EyeZone, 546, "C546 eye zone id",
              spec->f0352Anchor);
    check_int(spec->eyeLookingGraphic, 203, "pressed-eye graphic id",
              spec->f0352Anchor);
    check_int(spec->eyeNotLookingGraphic, 202, "released-eye graphic id",
              spec->f0353Anchor);

    check_true(dm1_v1_mirror_candidate_c040_eye_live_candidate_run_pc34(
                   &result) == 1,
               "runtime contract returns accepted",
               spec->f0353Anchor);
    check_int(result.assertionCount, 12, "model assertion count",
              spec->f0353Anchor);
    check_int(result.candidateOrdinalBefore, 3,
              "fixture starts with a live nonzero C040 candidate",
              spec->f0280Anchor);
    check_int(result.candidateOrdinalAfterPress,
              result.candidateOrdinalBefore,
              "eye press does not consume C040 candidate",
              spec->f0352Anchor);
    check_int(result.candidateOrdinalAfterRelease,
              result.candidateOrdinalBefore,
              "eye release does not consume C040 candidate",
              spec->f0353Anchor);
    check_int(result.panelContentBefore, spec->c040PanelContent,
              "fixture starts on C040 panel", spec->f0347Anchor);
    check_true(result.panelContentAfterPress != spec->c040PanelContent,
               "eye press may show object panel while button is down",
               spec->f0352Anchor);
    check_int(result.panelContentAfterRelease, spec->c040PanelContent,
              "eye release redraws C040 panel", spec->f0347Anchor);
    check_int(result.panelGraphicAfterRelease, spec->c040PanelGraphic,
              "C040 graphic restored after release", spec->f0347Anchor);
    check_int(result.pressingEyeAfterPress, 1,
              "press path sets pressing-eye state", spec->f0352Anchor);
    check_int(result.pressingEyeAfterRelease, 0,
              "release path clears pressing-eye state", spec->f0353Anchor);
    check_int(result.pointerHiddenAfterPress, 1,
              "press path hides pointer", spec->f0352Anchor);
    check_int(result.pointerHiddenAfterRelease, 0,
              "release path shows pointer", spec->f0353Anchor);
    check_true(result.openChestBefore != -1, "fixture starts with open chest",
               spec->f0334Anchor);
    check_int(result.openChestAfterRelease, -1,
              "F0347 release closes prior chest", spec->f0334Anchor);
    check_true(result.chestClosedOnRelease,
               "release calls close-chest once", spec->f0347Anchor);
    check_true(result.chestSlotsCleared,
               "close-chest clears visible G0425 slots", spec->f0334Anchor);
    check_true(result.f0347RedrewC040,
               "F0347 gives live C040 priority after close",
               spec->f0347Anchor);
    check_true(result.f0282NotDispatched,
               "C546 does not route through C040 confirm/cancel",
               spec->f0359Anchor);
    check_true(result.leaderHandPreserved,
               "eye toggle preserves leader hand object",
               spec->f0352Anchor);
    check_true(result.objectPanelDrawnDuringPress,
               "occupied leader hand draws object eye panel during press",
               spec->f0352Anchor);
    check_true(result.viewportRedrawnForPressAndRelease,
               "press and release both redraw the viewport",
               spec->f0353Anchor);
    check_true(result.accepted, "all C040 eye-live-candidate checks accepted",
               spec->f0353Anchor);

    if (gFailures) {
        printf("FAIL dm1_v1_mirror_candidate_c040_eye_live_candidate_pc34_compat "
               "assertions=%d failures=%d\n",
               gAssertions, gFailures);
        return 1;
    }
    printf("PASS dm1_v1_mirror_candidate_c040_eye_live_candidate_pc34_compat "
           "assertions=%d\n",
           gAssertions);
    return 0;
}
