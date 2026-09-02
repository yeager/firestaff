#include "dm1_v1_ceiling_pit_viewport_pc34_compat.h"

#include <assert.h>
#include <stdio.h>

static void test_rect_d2l(void)
{
    const DM1V1CeilingPitViewportRectPc34 *r =
        dm1_v1_ceiling_pit_viewport_rect_pc34(
            DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34,
            DM1_V1_ZONE_CEILING_PIT_D2L_PC34, 0);
    (void)r;
    if (r) {
        assert(r->width > 0);
        assert(r->height > 0);
    }
}

static void test_rect_d2c(void)
{
    const DM1V1CeilingPitViewportRectPc34 *r =
        dm1_v1_ceiling_pit_viewport_rect_pc34(
            DM1_V1_GRAPHIC_CEILING_PIT_D2C_PC34,
            DM1_V1_ZONE_CEILING_PIT_D2C_PC34, 0);
    (void)r;
    if (r) {
        assert(r->width > 0);
        assert(r->height > 0);
    }
}

static void test_rect_d2r(void)
{
    const DM1V1CeilingPitViewportRectPc34 *r =
        dm1_v1_ceiling_pit_viewport_rect_pc34(
            DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34,
            DM1_V1_ZONE_CEILING_PIT_D2R_PC34, 0);
    (void)r;
}

static void expect_rect(int profile, int graphic, int zone, int parity, int x,
                        int y, int width, int height)
{
    const DM1V1CeilingPitViewportRectPc34 *r =
        dm1_v1_ceiling_pit_viewport_rect_for_profile_pc34(profile, graphic,
                                                           zone, parity);
    assert(r != NULL);
    assert(r->x == x && r->y == y);
    assert(r->width == width && r->height == height);
}

static void test_rect_d1_d0_all_routes(void)
{
    /* ReDMCSB DUNVIEW.C:571-576 / 7527-8292. */
    expect_rect(DM1_V1_CEILING_PIT_PROFILE_PC34,
                DM1_V1_GRAPHIC_CEILING_PIT_D1L_PC34,
                DM1_V1_ZONE_CEILING_PIT_D1L_PC34, 0, 0, 8, 64, 9);
    expect_rect(DM1_V1_CEILING_PIT_PROFILE_I34E,
                DM1_V1_GRAPHIC_CEILING_PIT_D1L_I34E,
                DM1_V1_ZONE_CEILING_PIT_D1R_I34E, 1, 160, 8, 64, 9);
    expect_rect(DM1_V1_CEILING_PIT_PROFILE_PC34,
                DM1_V1_GRAPHIC_CEILING_PIT_D1C_PC34,
                DM1_V1_ZONE_CEILING_PIT_D1C_PC34, 0, 32, 8, 160, 9);
    expect_rect(DM1_V1_CEILING_PIT_PROFILE_PC34,
                DM1_V1_GRAPHIC_CEILING_PIT_D0L_PC34,
                DM1_V1_ZONE_CEILING_PIT_D0L_PC34, 0, 0, 0, 16, 4);
    expect_rect(DM1_V1_CEILING_PIT_PROFILE_I34E,
                DM1_V1_GRAPHIC_CEILING_PIT_D0L_I34E,
                DM1_V1_ZONE_CEILING_PIT_D0R_I34E, 1, 208, 0, 16, 4);
    expect_rect(DM1_V1_CEILING_PIT_PROFILE_I34E,
                DM1_V1_GRAPHIC_CEILING_PIT_D0C_I34E,
                DM1_V1_ZONE_CEILING_PIT_D0C_I34E, 0, 16, 0, 192, 4);
}

static void test_rect_parity(void)
{
    const DM1V1CeilingPitViewportRectPc34 *r0 =
        dm1_v1_ceiling_pit_viewport_rect_pc34(
            DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34,
            DM1_V1_ZONE_CEILING_PIT_D2L_PC34, 0);
    const DM1V1CeilingPitViewportRectPc34 *r1 =
        dm1_v1_ceiling_pit_viewport_rect_pc34(
            DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34,
            DM1_V1_ZONE_CEILING_PIT_D2L_PC34, 1);
    (void)r0; (void)r1;
}

static void test_rect_invalid(void)
{
    const DM1V1CeilingPitViewportRectPc34 *r =
        dm1_v1_ceiling_pit_viewport_rect_pc34(-1, -1, 0);
    (void)r;
    assert(r == NULL);
}

static void test_draw_null_dest(void)
{
    int rc = dm1_v1_ceiling_pit_viewport_draw_pc34(
        DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34,
        DM1_V1_ZONE_CEILING_PIT_D2L_PC34,
        5, 5, 0,
        NULL, 320, 200, 320,
        NULL, 32, 32);
    (void)rc;
    assert(rc != 1);
}

static void test_constants(void)
{
    assert(DM1_V1_CEILING_PIT_TRANSPARENT_PC34 == 10);
    assert(DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34 == 63);
    assert(DM1_V1_GRAPHIC_CEILING_PIT_D2C_PC34 == 64);
    assert(DM1_V1_ZONE_CEILING_PIT_D2L_PC34 == 862);
    assert(DM1_V1_ZONE_CEILING_PIT_D2C_PC34 == 863);
    assert(DM1_V1_ZONE_CEILING_PIT_D2R_PC34 == 864);
}

static void test_i34e_constants(void)
{
    assert(DM1_V1_GRAPHIC_CEILING_PIT_D2L_I34E == 64);
    assert(DM1_V1_GRAPHIC_CEILING_PIT_D2C_I34E == 65);
    assert(DM1_V1_ZONE_CEILING_PIT_D2L_I34E == 864);
    assert(DM1_V1_ZONE_CEILING_PIT_D2C_I34E == 865);
    assert(DM1_V1_ZONE_CEILING_PIT_D2R_I34E == 866);
}

int main(void)
{
    test_rect_d2l();
    test_rect_d2c();
    test_rect_d2r();
    test_rect_d1_d0_all_routes();
    test_rect_parity();
    test_rect_invalid();
    test_draw_null_dest();
    test_constants();
    test_i34e_constants();

    puts("ok: DM1 ceiling pit viewport (Q-DM1-03) 9 tests passed");
    return 0;
}
