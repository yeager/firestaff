#include "main_loop_m11.h"

#include <stdio.h>

/* IMG3 globals are required by firestaff_m10 when this focused gate links
 * the full runtime libraries through main_loop_m11.c. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int expect_eq(const char* name, int got, int want) {
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", name, got, want);
        return 0;
    }
    printf("%s=%d\n", name, got);
    return 1;
}

int main(void) {
    int ok = 1;

    ok &= expect_eq("unbounded_run_duration_never_expires",
                    M11_RunDurationHasElapsed(-1, 10U, 0U), 0);
    ok &= expect_eq("zero_run_duration_expires_immediately",
                    M11_RunDurationHasElapsed(0, 10U, 10U), 1);
    ok &= expect_eq("finite_run_duration_before_deadline",
                    M11_RunDurationHasElapsed(10, 100U, 109U), 0);
    ok &= expect_eq("finite_run_duration_at_deadline",
                    M11_RunDurationHasElapsed(10, 100U, 110U), 1);
    ok &= expect_eq("finite_run_duration_handles_tick_wrap",
                    M11_RunDurationHasElapsed(32, UINT32_C(0xfffffff0),
                                              UINT32_C(0x00000010)), 1);

    /* ReDMCSB ENTRANCE.C:850-883 drains prior input and waits in
     * C099_MODE_WAITING_ON_ENTRANCE until a fresh entrance command is
     * processed.  The launcher Launch key/click must not be reused as C200. */
    ok &= expect_eq("interactive_ignores_1200ms_auto_enter",
                    M11_Entrance_ShouldAutoEnterForTimeout(0, 1200, 1201), 0);
    ok &= expect_eq("interactive_still_waits_after_headless_timeout",
                    M11_Entrance_ShouldAutoEnterForTimeout(0, 1200, 6000), 0);

    /* Headless/autotest remains deterministic: use the short explicit handoff
     * timeout when supplied, otherwise the conservative 5s escape hatch. */
    ok &= expect_eq("headless_waits_before_short_timeout",
                    M11_Entrance_ShouldAutoEnterForTimeout(1, 1200, 1200), 0);
    ok &= expect_eq("headless_enters_after_short_timeout",
                    M11_Entrance_ShouldAutoEnterForTimeout(1, 1200, 1201), 1);
    ok &= expect_eq("headless_waits_before_default_timeout",
                    M11_Entrance_ShouldAutoEnterForTimeout(1, 0, 5000), 0);
    ok &= expect_eq("headless_enters_after_default_timeout",
                    M11_Entrance_ShouldAutoEnterForTimeout(1, 0, 5001), 1);

    return ok ? 0 : 1;
}
