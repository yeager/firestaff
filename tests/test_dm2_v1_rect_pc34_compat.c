#include "dm2_v1_rect_pc34_compat.h"
#include <assert.h>
#include <stdio.h>

static void test_init(void)
{
    DM2_V1_Rect r;
    dm2_v1_rect_init(&r);
    assert(r.x == 0 && r.y == 0 && r.w == 0 && r.h == 0);
    printf("test_init OK\n");
}

static void test_set(void)
{
    DM2_V1_Rect r;
    dm2_v1_rect_set(&r, 10, 20, 100, 50);
    assert(r.x == 10 && r.y == 20 && r.w == 100 && r.h == 50);
    printf("test_set OK\n");
}

static void test_set_origin(void)
{
    DM2_V1_Rect r;
    dm2_v1_rect_set_origin(&r, 320, 200);
    assert(r.x == 0 && r.y == 0 && r.w == 320 && r.h == 200);
    printf("test_set_origin OK\n");
}

static void test_inflate(void)
{
    DM2_V1_Rect r;
    dm2_v1_rect_set(&r, 10, 20, 100, 50);
    dm2_v1_rect_inflate(&r, 5, 3);
    assert(r.x == 5 && r.y == 17 && r.w == 110 && r.h == 56);
    printf("test_inflate OK\n");
}

static void test_contains_inside(void)
{
    DM2_V1_Rect r;
    dm2_v1_rect_set(&r, 10, 20, 100, 50);
    assert(dm2_v1_rect_contains(&r, 50, 40));
    assert(dm2_v1_rect_contains(&r, 10, 20));
    assert(dm2_v1_rect_contains(&r, 109, 69));
    printf("test_contains_inside OK\n");
}

static void test_contains_outside(void)
{
    DM2_V1_Rect r;
    dm2_v1_rect_set(&r, 10, 20, 100, 50);
    assert(!dm2_v1_rect_contains(&r, 9, 40));
    assert(!dm2_v1_rect_contains(&r, 110, 40));
    assert(!dm2_v1_rect_contains(&r, 50, 19));
    assert(!dm2_v1_rect_contains(&r, 50, 70));
    printf("test_contains_outside OK\n");
}

static void test_intersect_full_overlap(void)
{
    DM2_V1_Rect r, clip;
    dm2_v1_rect_set(&r, 10, 20, 50, 30);
    dm2_v1_rect_set(&clip, 0, 0, 320, 200);
    int16_t ox, oy;
    assert(dm2_v1_rect_intersect(&r, &clip, &ox, &oy));
    assert(ox == 0 && oy == 0);
    assert(r.x == 10 && r.y == 20 && r.w == 50 && r.h == 30);
    printf("test_intersect_full_overlap OK\n");
}

static void test_intersect_partial(void)
{
    DM2_V1_Rect r, clip;
    dm2_v1_rect_set(&r, -5, 10, 20, 30);
    dm2_v1_rect_set(&clip, 0, 0, 320, 200);
    int16_t ox, oy;
    assert(dm2_v1_rect_intersect(&r, &clip, &ox, &oy));
    assert(ox == 5);
    assert(oy == 0);
    assert(r.x == 0 && r.w == 15);
    printf("test_intersect_partial OK\n");
}

static void test_intersect_no_overlap(void)
{
    DM2_V1_Rect r, clip;
    dm2_v1_rect_set(&r, 400, 10, 20, 30);
    dm2_v1_rect_set(&clip, 0, 0, 320, 200);
    int16_t ox, oy;
    assert(!dm2_v1_rect_intersect(&r, &clip, &ox, &oy));
    printf("test_intersect_no_overlap OK\n");
}

static void test_intersect_right_clip(void)
{
    DM2_V1_Rect r, clip;
    dm2_v1_rect_set(&r, 300, 50, 40, 20);
    dm2_v1_rect_set(&clip, 0, 0, 320, 200);
    int16_t ox, oy;
    assert(dm2_v1_rect_intersect(&r, &clip, &ox, &oy));
    assert(r.w == 20);
    printf("test_intersect_right_clip OK\n");
}

static void test_center_in(void)
{
    DM2_V1_Rect r, container;
    dm2_v1_rect_set(&container, 0, 0, 320, 200);
    dm2_v1_rect_center_in(&r, &container, 100, 50);
    assert(r.x == 110 && r.y == 75);
    assert(r.w == 100 && r.h == 50);
    printf("test_center_in OK\n");
}

static void test_tmprects_ringbuffer(void)
{
    DM2_V1_TempRects t;
    dm2_v1_tmprects_init(&t);
    DM2_V1_Rect *r1 = dm2_v1_tmprects_alloc(&t, 1, 2, 3, 4);
    DM2_V1_Rect *r2 = dm2_v1_tmprects_alloc(&t, 5, 6, 7, 8);
    assert(r1 != r2);
    assert(r1->x == 1 && r2->x == 5);

    /* Allocate 2 more to wrap around */
    dm2_v1_tmprects_alloc(&t, 0, 0, 0, 0);
    dm2_v1_tmprects_alloc(&t, 0, 0, 0, 0);
    DM2_V1_Rect *r5 = dm2_v1_tmprects_alloc(&t, 99, 99, 99, 99);
    assert(r5 == r1);
    assert(r5->x == 99);
    printf("test_tmprects_ringbuffer OK\n");
}

static void test_tmprects_origin(void)
{
    DM2_V1_TempRects t;
    dm2_v1_tmprects_init(&t);
    DM2_V1_Rect *r = dm2_v1_tmprects_alloc_origin(&t, 320, 200);
    assert(r->x == 0 && r->y == 0 && r->w == 320 && r->h == 200);
    printf("test_tmprects_origin OK\n");
}

static void test_viewport_rect(void)
{
    DM2_V1_Rect r;
    dm2_v1_rect_set(&r, 21, 8, 182, 110);
    assert(dm2_v1_rect_contains(&r, 21, 8));
    assert(dm2_v1_rect_contains(&r, 202, 117));
    assert(!dm2_v1_rect_contains(&r, 20, 8));
    printf("test_viewport_rect OK\n");
}

int main(void)
{
    test_init();
    test_set();
    test_set_origin();
    test_inflate();
    test_contains_inside();
    test_contains_outside();
    test_intersect_full_overlap();
    test_intersect_partial();
    test_intersect_no_overlap();
    test_intersect_right_clip();
    test_center_in();
    test_tmprects_ringbuffer();
    test_tmprects_origin();
    test_viewport_rect();
    printf("All dm2_v1_rect tests passed.\n");
    return 0;
}
