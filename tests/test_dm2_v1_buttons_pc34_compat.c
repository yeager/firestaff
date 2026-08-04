/*
 * test_dm2_v1_buttons_pc34_compat.c — unit tests for DM2 button group management.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_buttons_pc34_compat.h"

static int passed = 0, failed = 0;

#define RUN(fn) do { fn(); passed++; } while(0)

static void test_button_init(void)
{
    DM2_V1_Button b;
    memset(&b, 0xFF, sizeof(b));
    dm2_v1_button_init(&b);
    assert(b.dbidx == -1);
    assert(b.r.x == 0 && b.r.y == 0 && b.r.w == 0 && b.r.h == 0);
    assert(b.groupsize == 0);
    printf("  PASS: button_init\n");
}

static void test_buttongroup_init(void)
{
    DM2_V1_ButtonGroup bg;
    memset(&bg, 0xFF, sizeof(bg));
    dm2_v1_buttongroup_init(&bg);
    assert(bg.button.dbidx == -1);
    assert(bg.button.groupsize == 0);
    for (int i = 0; i < 5; i++) {
        assert(bg.rects[i].x == 0);
        assert(bg.rects[i].w == 0);
    }
    printf("  PASS: buttongroup_init\n");
}

static void test_init_global_buttongroups(void)
{
    DM2_V1_ButtonGroup bg1, bg2;
    memset(&bg1, 0xFF, sizeof(bg1));
    memset(&bg2, 0xFF, sizeof(bg2));
    dm2_v1_init_global_buttongroups(&bg1, &bg2);
    assert(bg1.button.dbidx == -1);
    assert(bg2.button.dbidx == -1);
    printf("  PASS: init_global_buttongroups\n");
}

static void test_offset_rect(void)
{
    DM2_V1_ButtonGroup bg;
    dm2_v1_buttongroup_init(&bg);
    bg.button.r.x = 10; bg.button.r.y = 20;

    DM2_V1_ButtonRect src = {15, 25, 30, 40};
    DM2_V1_OffsetRectReceipt r = dm2_v1_offset_rect(&bg, &src);
    assert(r.result.x == 5);
    assert(r.result.y == 5);
    assert(r.result.w == 30);
    assert(r.result.h == 40);
    printf("  PASS: offset_rect\n");
}

static void test_adjust_first_rect(void)
{
    DM2_V1_ButtonGroup bg;
    dm2_v1_buttongroup_init(&bg);
    bg.button.r.x = 0; bg.button.r.y = 0;
    bg.button.r.w = 100; bg.button.r.h = 100;

    DM2_V1_ButtonRect rect = {10, 10, 20, 20};
    DM2_V1_AdjustButtonGroupReceipt r = dm2_v1_adjust_buttongroup_rects(&bg, &rect);
    assert(r.adjusted);
    assert(bg.button.groupsize == 1);
    assert(bg.rects[0].x == 10);
    assert(bg.rects[0].y == 10);
    printf("  PASS: adjust_first_rect\n");
}

static void test_adjust_contained(void)
{
    DM2_V1_ButtonGroup bg;
    dm2_v1_buttongroup_init(&bg);
    bg.button.r = (DM2_V1_ButtonRect){0, 0, 100, 100};
    bg.button.groupsize = 1;
    bg.rects[0] = (DM2_V1_ButtonRect){5, 5, 50, 50};

    /* Input is inside existing rect */
    DM2_V1_ButtonRect rect = {10, 10, 20, 20};
    DM2_V1_AdjustButtonGroupReceipt r = dm2_v1_adjust_buttongroup_rects(&bg, &rect);
    assert(r.already_contained);
    assert(bg.button.groupsize == 1);
    printf("  PASS: adjust_contained\n");
}

static void test_adjust_containing(void)
{
    DM2_V1_ButtonGroup bg;
    dm2_v1_buttongroup_init(&bg);
    bg.button.r = (DM2_V1_ButtonRect){0, 0, 100, 100};
    bg.button.groupsize = 1;
    bg.rects[0] = (DM2_V1_ButtonRect){10, 10, 20, 20};

    /* Input contains existing rect — replaces it */
    DM2_V1_ButtonRect rect = {5, 5, 50, 50};
    DM2_V1_AdjustButtonGroupReceipt r = dm2_v1_adjust_buttongroup_rects(&bg, &rect);
    assert(r.adjusted);
    assert(bg.rects[0].x == 5);
    assert(bg.rects[0].w == 50);
    assert(bg.button.groupsize == 1);
    printf("  PASS: adjust_containing\n");
}

static void test_adjust_clipping(void)
{
    DM2_V1_ButtonGroup bg;
    dm2_v1_buttongroup_init(&bg);
    bg.button.r = (DM2_V1_ButtonRect){10, 10, 80, 80};

    /* Rect extends beyond right edge */
    DM2_V1_ButtonRect rect = {50, 20, 60, 30};
    DM2_V1_AdjustButtonGroupReceipt r = dm2_v1_adjust_buttongroup_rects(&bg, &rect);
    assert(r.adjusted);
    assert(r.clipped);
    /* Right edge: 10+80-1=89, rect right: 50+60-1=109, delta=-20, w becomes 40 */
    assert(bg.rects[0].w == 40);
    printf("  PASS: adjust_clipping\n");
}

static void test_adjust_fully_outside(void)
{
    DM2_V1_ButtonGroup bg;
    dm2_v1_buttongroup_init(&bg);
    bg.button.r = (DM2_V1_ButtonRect){50, 50, 20, 20};

    /* Rect entirely left of button bounds */
    DM2_V1_ButtonRect rect = {10, 10, 5, 5};
    DM2_V1_AdjustButtonGroupReceipt r = dm2_v1_adjust_buttongroup_rects(&bg, &rect);
    assert(r.removed);
    assert(bg.button.groupsize == 0);
    printf("  PASS: adjust_fully_outside\n");
}

static void test_adjust_overflow(void)
{
    DM2_V1_ButtonGroup bg;
    dm2_v1_buttongroup_init(&bg);
    bg.button.r = (DM2_V1_ButtonRect){0, 0, 200, 200};

    /* Fill all 5 slots with non-overlapping rects */
    for (int i = 0; i < 5; i++) {
        DM2_V1_ButtonRect rect = {(int16_t)(i * 30), 0, 20, 20};
        dm2_v1_adjust_buttongroup_rects(&bg, &rect);
    }
    assert(bg.button.groupsize == 5);

    /* 6th rect should overflow */
    DM2_V1_ButtonRect rect = {160, 0, 20, 20};
    DM2_V1_AdjustButtonGroupReceipt r = dm2_v1_adjust_buttongroup_rects(&bg, &rect);
    assert(r.overflow);
    printf("  PASS: adjust_overflow\n");
}

static void test_null_safety(void)
{
    dm2_v1_button_init(NULL);
    dm2_v1_buttongroup_init(NULL);
    dm2_v1_offset_rect(NULL, NULL);
    dm2_v1_adjust_buttongroup_rects(NULL, NULL);
    printf("  PASS: null_safety\n");
}

int main(void)
{
    printf("test_dm2_v1_buttons_pc34_compat:\n");
    RUN(test_button_init);
    RUN(test_buttongroup_init);
    RUN(test_init_global_buttongroups);
    RUN(test_offset_rect);
    RUN(test_adjust_first_rect);
    RUN(test_adjust_contained);
    RUN(test_adjust_containing);
    RUN(test_adjust_clipping);
    RUN(test_adjust_fully_outside);
    RUN(test_adjust_overflow);
    RUN(test_null_safety);
    printf("  %d passed, %d failed\n", passed, failed);
    return failed;
}
