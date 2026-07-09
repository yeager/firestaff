#include "dm1_v1_dialog_layout_pc34_compat.h"

#include <stdio.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check(int cond, const char* expr, const char* file, int line) {
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s:%d %s\n", file, line, expr);
    }
}

#define CHECK(c) check((c), #c, __FILE__, __LINE__)

static void expect_rect(int ok,
                        const DM1_V1_DialogRectPc34* r,
                        int x,
                        int y,
                        int w,
                        int h) {
    CHECK(ok == 1);
    CHECK(r != 0);
    CHECK(r->x == x);
    CHECK(r->y == y);
    CHECK(r->w == w);
    CHECK(r->h == h);
}

static void test_dialog_header_and_patch(void) {
    DM1_V1_DialogPatchPc34 p;
    int x = 0;
    int y = 0;
    CHECK(dm1_v1_dialog_layout_source_evidence_pc34() != 0);
    CHECK(dm1_v1_dialog_version_text_origin_pc34(&x, &y) == 1);
    CHECK(x == 192);
    CHECK(y == 40);

    CHECK(dm1_v1_dialog_choice_patch_pc34(1, &p) == 1);
    CHECK(p.srcX == 0 && p.srcY == 14 && p.w == 224 && p.h == 75);
    CHECK(p.dstX == 0 && p.dstY == 51);
    CHECK(dm1_v1_dialog_choice_patch_pc34(2, &p) == 1);
    CHECK(p.srcX == 102 && p.srcY == 52 && p.w == 21 && p.h == 37);
    CHECK(p.dstX == 102 && p.dstY == 89);
    CHECK(dm1_v1_dialog_choice_patch_pc34(3, &p) == 0);
    CHECK(dm1_v1_dialog_choice_patch_pc34(4, &p) == 1);
    CHECK(p.srcX == 102 && p.srcY == 99 && p.w == 21 && p.h == 36);
    CHECK(p.dstX == 102 && p.dstY == 62);
    CHECK(dm1_v1_dialog_choice_patch_pc34(4, 0) == 0);
}

static void test_message_geometry(void) {
    DM1_V1_DialogRectPc34 r;
    expect_rect(dm1_v1_dialog_message_rect_pc34(1, &r),
                &r,
                112,
                49,
                77,
                25);
    expect_rect(dm1_v1_dialog_message_rect_pc34(2, &r),
                &r,
                112,
                32,
                77,
                5);
    CHECK(dm1_v1_dialog_message_width_pc34(1) == 77);
    CHECK(dm1_v1_dialog_message_width_pc34(4) == 77);
    CHECK(dm1_v1_dialog_single_choice_message_text_y_pc34(1) == 96);
    CHECK(dm1_v1_dialog_single_choice_message_text_y_pc34(2) == 92);
    CHECK(dm1_v1_dialog_multi_choice_message_text_y_pc34(1) == 70);
    CHECK(dm1_v1_dialog_multi_choice_message_text_y_pc34(2) == 66);
    CHECK(dm1_v1_dialog_message_rect_pc34(1, 0) == 0);
}

static void test_choice_zones(void) {
    DM1_V1_DialogRectPc34 r;
    CHECK(dm1_v1_dialog_choice_text_zone_id_pc34(1, 0) == 462);
    expect_rect(dm1_v1_dialog_choice_text_rect_pc34(1, 0, &r),
                &r,
                16,
                110,
                192,
                7);
    CHECK(dm1_v1_dialog_choice_text_zone_id_pc34(2, 0) == 463);
    CHECK(dm1_v1_dialog_choice_text_zone_id_pc34(2, 1) == 462);
    CHECK(dm1_v1_dialog_choice_text_zone_id_pc34(3, 1) == 466);
    CHECK(dm1_v1_dialog_choice_text_zone_id_pc34(3, 2) == 467);
    CHECK(dm1_v1_dialog_choice_text_zone_id_pc34(4, 0) == 464);
    CHECK(dm1_v1_dialog_choice_text_zone_id_pc34(4, 1) == 465);
    expect_rect(dm1_v1_dialog_choice_text_rect_pc34(4, 3, &r),
                &r,
                123,
                110,
                86,
                7);
    CHECK(dm1_v1_dialog_choice_text_rect_pc34(2, 2, &r) == 0);

    CHECK(dm1_v1_dialog_choice_button_zone_id_pc34(1, 0) == 456);
    CHECK(dm1_v1_dialog_choice_button_zone_id_pc34(2, 0) == 457);
    CHECK(dm1_v1_dialog_choice_button_zone_id_pc34(3, 1) == 460);
    CHECK(dm1_v1_dialog_choice_button_zone_id_pc34(4, 3) == 461);
    expect_rect(dm1_v1_dialog_choice_hit_rect_pc34(1, 0, &r),
                &r,
                16,
                104,
                192,
                17);
    expect_rect(dm1_v1_dialog_choice_hit_rect_pc34(4, 3, &r),
                &r,
                123,
                104,
                86,
                17);
    CHECK(dm1_v1_dialog_choice_hit_rect_pc34(4, 4, &r) == 0);
}

static void test_hit_text_alignment(void) {
    int choiceCount;
    int i;
    for (choiceCount = 1; choiceCount <= 4; ++choiceCount) {
        for (i = 0; i < choiceCount; ++i) {
            DM1_V1_DialogRectPc34 text;
            DM1_V1_DialogRectPc34 hit;
            CHECK(dm1_v1_dialog_choice_text_rect_pc34(choiceCount, i, &text));
            CHECK(dm1_v1_dialog_choice_hit_rect_pc34(choiceCount, i, &hit));
            CHECK(hit.x == text.x);
            CHECK(hit.y + 6 == text.y);
            CHECK(hit.w == text.w);
            CHECK(hit.h == 17);
        }
    }
}

int main(void) {
    test_dialog_header_and_patch();
    test_message_geometry();
    test_choice_zones();
    test_hit_text_alignment();
    printf("dm1_v1_dialog_layout: %d/%d assertions passed\n",
           g_assertions - g_failures,
           g_assertions);
    return g_failures == 0 ? 0 : 1;
}
