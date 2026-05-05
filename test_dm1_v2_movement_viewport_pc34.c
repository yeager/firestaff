#include "dm1_v2_movement_engine_pc34.h"
#include "dm1_v2_viewport_renderer_pc34.h"

#include <stdio.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static void test_movement_basics(void) {
    DM1_V2_PlayerPos p;
    DM1_V2_MoveParams params = {256, 1, 256, 0, 0};
    int8_t map[9] = {
        0, 0, 0,
        0, 1, 0,
        0, 0, 0,
    };

    dm1_v2_pos_init(&p, 2, 3, 0);
    CHECK(dm1_v2_get_x(&p) == 2 * DM1_V2_SUBPIXEL_SCALE);
    CHECK(dm1_v2_get_y(&p) == 3 * DM1_V2_SUBPIXEL_SCALE);
    CHECK(p.facingDir == 0);
    CHECK(!dm1_v2_has_moved(&p));

    dm1_v2_set_subpixel(&p, -5, DM1_V2_SUBPIXEL_SCALE + 9);
    CHECK(p.xSub == 0);
    CHECK(p.ySub == DM1_V2_SUBPIXEL_SCALE - 1);

    dm1_v2_turn(&p, -1);
    CHECK(p.facingDir == 7);
    dm1_v2_turn(&p, 1);
    CHECK(p.facingDir == 0);

    dm1_v2_move_step(&p, &params, 0, 1000);
    CHECK(dm1_v2_has_moved(&p));
    CHECK(dm1_v2_collides_at(1, 1, map, 3, 3) == 1);
    CHECK(dm1_v2_collides_at(0, 0, map, 3, 3) == 0);
    CHECK(dm1_v2_collides_at(-1, 0, map, 3, 3) == 1);
    CHECK(dm1_v2_collides_at(0, 0, NULL, 3, 3) == 1);
}

static void test_viewport_basics(void) {
    DM1_V2_ViewportState vp;
    DM1_V2_Color c;

    dm1_v2_vp_init(&vp);
    CHECK(dm1_v2_vp_is_dirty(&vp));
    CHECK(!dm1_v2_vp_is_scrolling(&vp));
    CHECK(vp.light.fogDensity[0] == 0);
    CHECK(vp.light.fogDensity[3] == 192);

    dm1_v2_vp_clear(&vp, 10, 20, 30);
    c = dm1_v2_vp_get_pixel(&vp, 5, 6);
    CHECK(c.r == 10 && c.g == 20 && c.b == 30 && c.a == 255);

    dm1_v2_vp_set_pixel(&vp, 5, 6, 100, 110, 120, 130);
    c = dm1_v2_vp_get_pixel(&vp, 5, 6);
    CHECK(c.r == 100 && c.g == 110 && c.b == 120 && c.a == 130);

    c = dm1_v2_vp_get_pixel(&vp, -1, -1);
    CHECK(c.r == 0 && c.g == 0 && c.b == 0 && c.a == 255);

    dm1_v2_vp_present(&vp, 1234);
    CHECK(!dm1_v2_vp_is_dirty(&vp));
    CHECK(vp.frameCount == 1);
    CHECK(vp.lastRenderMs == 1234);

    dm1_v2_vp_begin_scroll(&vp, 8, 0, 16);
    CHECK(dm1_v2_vp_is_scrolling(&vp));
    dm1_v2_vp_tick_scroll(&vp, 250);
    CHECK(vp.scroll.scrollOffX == 4);
    CHECK(vp.scroll.scrollOffX <= vp.scroll.scrollTargetX);
    dm1_v2_vp_tick_scroll(&vp, 250);
    CHECK(vp.scroll.scrollOffX == 8);
    CHECK(!dm1_v2_vp_is_scrolling(&vp));

    dm1_v2_vp_begin_scroll(&vp, -8, 0, 16);
    CHECK(dm1_v2_vp_is_scrolling(&vp));
    dm1_v2_vp_tick_scroll(&vp, 250);
    CHECK(vp.scroll.scrollOffX == 4);
    dm1_v2_vp_tick_scroll(&vp, 250);
    CHECK(vp.scroll.scrollOffX == 0);
    CHECK(!dm1_v2_vp_is_scrolling(&vp));
}

int main(void) {
    test_movement_basics();
    test_viewport_basics();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_movement_viewport_pc34: ok");
    return 0;
}
