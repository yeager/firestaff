#include "main_loop_m11.h"
#include "render_sdl_m11.h"

#include <stdio.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check_int(const char* label, int got, int expected) {
    ++g_assertions;
    if (got != expected) {
        ++g_failures;
        fprintf(stderr, "FAIL %s: got %d expected %d\n", label, got, expected);
    }
}

int main(void) {
    check_int("v1 forces nearest from linear",
              M11_ResolveGameScaleFilterForPresentation(
                  M12_PRESENTATION_V1_ORIGINAL,
                  M11_SCALE_FILTER_LINEAR),
              M11_SCALE_FILTER_NEAREST);
    check_int("v1 keeps nearest",
              M11_ResolveGameScaleFilterForPresentation(
                  M12_PRESENTATION_V1_ORIGINAL,
                  M11_SCALE_FILTER_NEAREST),
              M11_SCALE_FILTER_NEAREST);
    check_int("v2 filtered forces nearest from linear for source glyphs",
              M11_ResolveGameScaleFilterForPresentation(
                  M12_PRESENTATION_V20_FILTERED,
                  M11_SCALE_FILTER_LINEAR),
              M11_SCALE_FILTER_NEAREST);
    check_int("v2 filtered keeps nearest",
              M11_ResolveGameScaleFilterForPresentation(
                  M12_PRESENTATION_V20_FILTERED,
                  M11_SCALE_FILTER_NEAREST),
              M11_SCALE_FILTER_NEAREST);
    check_int("v2 upscaled keeps linear",
              M11_ResolveGameScaleFilterForPresentation(
                  M12_PRESENTATION_V21_UPSCALED,
                  M11_SCALE_FILTER_LINEAR),
              M11_SCALE_FILTER_LINEAR);
    check_int("invalid filter clamps nearest",
              M11_ResolveGameScaleFilterForPresentation(
                  M12_PRESENTATION_V22_MODERN,
                  99),
              M11_SCALE_FILTER_NEAREST);

    printf("m11_v1_presentation_filter_pc34_compat: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
