#include "csb_v1_viewport_surface_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;

static int expect_int(const char *label, int got, int expected)
{
    ++assertions;
    if (got != expected) {
        fprintf(stderr, "FAIL %s got=%d expected=%d\n", label, got, expected);
        return 0;
    }
    return 1;
}

int main(void)
{
    uint8_t screen[320 * 200];
    uint8_t *viewport = NULL;
    int stride = 0;
    int ok = 1;

    memset(screen, 0, sizeof(screen));
    ok &= expect_int("bind.full-page",
                     csb_v1_viewport_screen_surface_pc34(
                         screen, sizeof(screen), 320, 200,
                         &viewport, &stride), 1);
    ok &= expect_int("origin.x", CSB_V1_VIEWPORT_SCREEN_X_PC34, 48);
    ok &= expect_int("origin.y", CSB_V1_VIEWPORT_SCREEN_Y_PC34, 33);
    ok &= expect_int("size.width", CSB_V1_VIEWPORT_SCREEN_WIDTH_PC34, 224);
    ok &= expect_int("size.height", CSB_V1_VIEWPORT_SCREEN_HEIGHT_PC34, 136);
    ok &= expect_int("origin.offset", (int)(viewport - screen), 33 * 320 + 48);
    ok &= expect_int("origin.stride", stride, 320);

    if (viewport) {
        viewport[0] = 1;
        viewport[(CSB_V1_VIEWPORT_SCREEN_HEIGHT_PC34 - 1) * stride +
                 CSB_V1_VIEWPORT_SCREEN_WIDTH_PC34 - 1] = 2;
    }
    ok &= expect_int("hud.above-preserved", screen[32 * 320 + 48], 0);
    ok &= expect_int("hud.left-preserved", screen[33 * 320 + 47], 0);
    ok &= expect_int("viewport.first-write", screen[33 * 320 + 48], 1);
    ok &= expect_int("viewport.last-write", screen[168 * 320 + 271], 2);
    ok &= expect_int("hud.below-preserved", screen[169 * 320 + 48], 0);

    viewport = (uint8_t *)1;
    stride = -1;
    ok &= expect_int("reject.short-page",
                     csb_v1_viewport_screen_surface_pc34(
                         screen, sizeof(screen) - 1u, 320, 200,
                         &viewport, &stride), 0);
    ok &= expect_int("reject.null-output", viewport == NULL, 1);
    ok &= expect_int("reject.zero-stride", stride, 0);
    ok &= expect_int("evidence",
                     strstr(csb_v1_viewport_screen_surface_source_evidence_pc34(),
                            "VIEWPORT.C") != NULL, 1);

    printf("checks=%d failures=%d\n", assertions, ok ? 0 : 1);
    return ok ? 0 : 1;
}
