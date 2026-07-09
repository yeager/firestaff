#include "dm1_v1_endgame_layout_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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

static void expect_rect(const char* label,
                        int ok,
                        const DM1_V1_EndgameRectPc34* r,
                        int x,
                        int y,
                        int w,
                        int h) {
    CHECK(ok == 1);
    if (!ok || !r) {
        fprintf(stderr, "FAIL: missing rect %s\n", label);
        return;
    }
    CHECK(r->x == x);
    CHECK(r->y == y);
    CHECK(r->w == w);
    CHECK(r->h == h);
}

static void test_the_end(void) {
    DM1_V1_EndgameRectPc34 r;
    memset(&r, 0, sizeof(r));
    expect_rect("the-end",
                dm1_v1_endgame_the_end_rect_pc34(&r),
                &r,
                120,
                95,
                80,
                14);
    CHECK(dm1_v1_endgame_the_end_rect_pc34(0) == 0);
}

static void test_champion_slots(void) {
    int slot;
    for (slot = 0; slot < 4; ++slot) {
        DM1_V1_EndgameRectPc34 mirror;
        DM1_V1_EndgameRectPc34 portrait;
        int x = 0;
        int y = 0;
        int skill = 0;
        CHECK(dm1_v1_endgame_champion_mirror_zone_id_pc34(slot) ==
              412 + slot);
        CHECK(dm1_v1_endgame_champion_portrait_zone_id_pc34(slot) ==
              416 + slot);
        expect_rect("mirror",
                    dm1_v1_endgame_champion_mirror_rect_pc34(slot,
                                                             &mirror),
                    &mirror,
                    19,
                    7 + slot * 48,
                    48,
                    43);
        expect_rect("portrait",
                    dm1_v1_endgame_champion_portrait_rect_pc34(slot,
                                                               &portrait),
                    &portrait,
                    27,
                    13 + slot * 48,
                    32,
                    29);
        CHECK(dm1_v1_endgame_champion_name_origin_pc34(slot, &x, &y) == 1);
        CHECK(x == 87);
        CHECK(y == 14 + slot * 48);
        for (skill = 0; skill < 4; ++skill) {
            CHECK(dm1_v1_endgame_champion_skill_origin_pc34(slot,
                                                            skill,
                                                            &x,
                                                            &y) == 1);
            CHECK(x == 105);
            CHECK(y == 23 + slot * 48 + skill * 8);
        }
    }
    CHECK(dm1_v1_endgame_champion_mirror_zone_id_pc34(-1) == 0);
    CHECK(dm1_v1_endgame_champion_mirror_zone_id_pc34(4) == 0);
    CHECK(dm1_v1_endgame_champion_portrait_zone_id_pc34(-1) == 0);
    CHECK(dm1_v1_endgame_champion_portrait_zone_id_pc34(4) == 0);
    CHECK(dm1_v1_endgame_champion_mirror_rect_pc34(-1, 0) == 0);
    CHECK(dm1_v1_endgame_champion_portrait_rect_pc34(4, 0) == 0);
    CHECK(dm1_v1_endgame_champion_name_origin_pc34(4, 0, 0) == 0);
    CHECK(dm1_v1_endgame_champion_skill_origin_pc34(0, -1, 0, 0) == 0);
    CHECK(dm1_v1_endgame_champion_skill_origin_pc34(0, 4, 0, 0) == 0);
}

static void test_buttons(void) {
    DM1_V1_EndgameRectPc34 r;
    expect_rect("restart-outer",
                dm1_v1_endgame_restart_box_pc34(0, &r),
                &r,
                103,
                140,
                115,
                15);
    expect_rect("restart-inner",
                dm1_v1_endgame_restart_box_pc34(1, &r),
                &r,
                105,
                142,
                111,
                11);
    expect_rect("quit-outer",
                dm1_v1_endgame_quit_box_pc34(0, &r),
                &r,
                127,
                165,
                67,
                15);
    expect_rect("quit-inner",
                dm1_v1_endgame_quit_box_pc34(1, &r),
                &r,
                129,
                167,
                63,
                11);
    CHECK(dm1_v1_endgame_restart_box_pc34(0, 0) == 0);
    CHECK(dm1_v1_endgame_quit_box_pc34(0, 0) == 0);
}

int main(void) {
    CHECK(dm1_v1_endgame_layout_source_evidence_pc34() != 0);
    test_the_end();
    test_champion_slots();
    test_buttons();
    printf("dm1_v1_endgame_layout: %d/%d assertions passed\n",
           g_assertions - g_failures,
           g_assertions);
    return g_failures == 0 ? 0 : 1;
}
