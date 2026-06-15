/*
 * test_dm1_v1_f0128_viewport_pc34_compat.c
 *
 * DM1 V1 BUG-118 — F0128 viewport-crop readiness regression
 * gate.  Source-locked per ReDMCSB DUNVIEW.C F0128
 * (F0128_DUNGEONVIEW_Draw_CPSF) + F0674_F0128_sub.
 *
 * The gate roots the pass434 chain.  Before this fix the
 * chain failed at pass434 with
 * FAIL_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS.  v1
 * wires the bounded F0128 compose path and exposes a
 * readiness flag that the M11 caller drives after every
 * party-tuple change.
 */
#include "m11_dm1_v1_f0128_viewport_pc34_compat.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    printf("=== DM1 V1 BUG-118 — F0128 viewport-crop readiness (v2.7.21) ===\n");

    /* Default: not ready. */
    CHECK(m11_dm1_v1_f0128_viewport_crop_ready() == 0,
          "default: viewport not yet ready");

    /* G0076 default: disabled. */
    CHECK(m11_dm1_v1_f0128_g0076_get() == 0,
          "G0076 default disabled (DM1 PC 3.4 baseline)");

    /* Compose with arbitrary tuple coords. */
    m11_dm1_v1_f0128_compose_viewport_for_tuple(1, 3, 0);
    CHECK(m11_dm1_v1_f0128_viewport_crop_ready() == 1,
          "after compose (1,3,map0): viewport ready");

    /* Re-compose (idempotent: stays ready). */
    m11_dm1_v1_f0128_compose_viewport_for_tuple(1, 4, 0);
    CHECK(m11_dm1_v1_f0128_viewport_crop_ready() == 1,
          "after re-compose (1,4,map0): viewport still ready");

    /* G0076 toggles. */
    m11_dm1_v1_f0128_g0076_set(1);
    CHECK(m11_dm1_v1_f0128_g0076_get() == 1,
          "G0076 enabled (CSB-style flip alternation)");
    m11_dm1_v1_f0128_g0076_set(0);
    CHECK(m11_dm1_v1_f0128_g0076_get() == 0,
          "G0076 disabled");

    /* Negative tuple coords are accepted (defensive). */
    m11_dm1_v1_f0128_compose_viewport_for_tuple(-1, -1, -1);
    CHECK(m11_dm1_v1_f0128_viewport_crop_ready() == 1,
          "compose with negative tuple: still ready (defensive)");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
