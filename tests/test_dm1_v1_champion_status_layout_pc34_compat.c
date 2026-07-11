#include "dm1_v1_champion_status_layout_pc34_compat.h"

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
                        const DM1_V1_ChampionStatusRectPc34* r,
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

static void test_status_boxes(void) {
    DM1_V1_ChampionStatusRectPc34 r;
    int slot;
    for (slot = 0; slot < 4; ++slot) {
        CHECK(dm1_v1_champion_status_box_zone_id_pc34(slot) == 151 + slot);
        expect_rect(dm1_v1_champion_status_box_rect_pc34(slot, &r),
                    &r,
                    slot * 69,
                    0,
                    67,
                    29);
        CHECK(dm1_v1_champion_status_bar_graph_zone_id_pc34(slot) ==
              187 + slot);
        CHECK(dm1_v1_champion_status_hand_parent_zone_id_pc34(slot) ==
              207 + slot);
        CHECK(dm1_v1_champion_status_name_clear_zone_id_pc34(slot) ==
              159 + slot);
        CHECK(dm1_v1_champion_status_name_text_zone_id_pc34(slot) ==
              163 + slot);
    }
    CHECK(dm1_v1_champion_status_box_zone_id_pc34(-1) == 0);
    CHECK(dm1_v1_champion_status_box_zone_id_pc34(4) == 0);
}

static void test_bars_hands_and_names(void) {
    DM1_V1_ChampionStatusRectPc34 r;
    CHECK(dm1_v1_champion_status_bar_zone_id_pc34(0) == 195);
    CHECK(dm1_v1_champion_status_bar_zone_id_pc34(1) == 199);
    CHECK(dm1_v1_champion_status_bar_zone_id_pc34(2) == 203);
    CHECK(dm1_v1_champion_status_bar_zone_id_pc34(3) == 0);
    CHECK(dm1_v1_champion_status_bar_value_zone_id_pc34(3, 1) == 202);
    expect_rect(dm1_v1_champion_status_bar_rect_pc34(2, 1, &r),
                &r,
                191,
                0,
                4,
                25);

    CHECK(dm1_v1_champion_status_hand_zone_id_pc34(3, 0) == 217);
    CHECK(dm1_v1_champion_status_hand_zone_id_pc34(3, 1) == 218);
    expect_rect(dm1_v1_champion_status_hand_rect_pc34(3, 1, &r),
                &r,
                231,
                10,
                16,
                16);
    expect_rect(dm1_v1_champion_status_hand_icon_rect_pc34(3, 1, &r),
                &r,
                232,
                11,
                16,
                16);
    expect_rect(dm1_v1_champion_status_hand_slot_box_rect_pc34(3, 1, &r),
                &r,
                231,
                10,
                18,
                18);
    expect_rect(dm1_v1_champion_status_name_rect_pc34(3, &r),
                &r,
                207,
                0,
                43,
                7);
    expect_rect(dm1_v1_champion_status_name_text_rect_pc34(3, &r),
                &r,
                208,
                0,
                42,
                7);
}

static void test_colors_and_graphics(void) {
    int gfx[3] = {0, 0, 0};
    CHECK(dm1_v1_champion_status_name_color_pc34(0, 10, 0) == -1);
    CHECK(dm1_v1_champion_status_name_color_pc34(1, 0, 0) == 13);
    CHECK(dm1_v1_champion_status_name_color_pc34(1, 10, 1) == 11);
    CHECK(dm1_v1_champion_status_name_color_pc34(1, 10, 0) == 9);
    CHECK(dm1_v1_champion_status_name_clear_color_pc34() == 1);
    CHECK(dm1_v1_champion_status_box_fill_color_pc34() == 12);
    CHECK(dm1_v1_champion_status_box_graphic_pc34() == 7);
    CHECK(dm1_v1_champion_dead_status_box_graphic_pc34() == 8);
    CHECK(dm1_v1_champion_status_box_base_graphic_pc34(0, 10) == 0);
    CHECK(dm1_v1_champion_status_box_base_graphic_pc34(1, 10) == 0);
    CHECK(dm1_v1_champion_status_box_base_graphic_pc34(1, 0) == 8);

    CHECK(dm1_v1_champion_status_hand_slot_graphic_pc34(0, 0, 0) == 33);
    CHECK(dm1_v1_champion_status_hand_slot_graphic_pc34(1, 0, 1) == 35);
    CHECK(dm1_v1_champion_status_hand_slot_graphic_pc34(1, 2, 0) == 34);
    CHECK(dm1_v1_champion_status_hand_slot_graphic_pc34(6, 0, 0) == -1);

    CHECK(dm1_v1_champion_status_shield_border_graphics_pc34(0, 0, 0, gfx) ==
          0);
    CHECK(dm1_v1_champion_status_shield_border_graphics_pc34(7, 5, 3, gfx) ==
          3);
    CHECK(gfx[0] == 37 && gfx[1] == 39 && gfx[2] == 38);
}

static void test_damage_and_poison(void) {
    DM1_V1_ChampionStatusRectPc34 r;
    expect_rect(dm1_v1_champion_poison_label_rect_pc34(3, 30, 6, &r),
                &r,
                225,
                29,
                30,
                6);
    CHECK(dm1_v1_champion_damage_indicator_zone_id_pc34(3) == 170);
    CHECK(dm1_v1_champion_inventory_damage_indicator_zone_id_pc34(3) == 182);
    expect_rect(dm1_v1_champion_damage_indicator_rect_pc34(3, 45, 7, &r),
                &r,
                218,
                11,
                45,
                7);
    expect_rect(dm1_v1_champion_inventory_damage_indicator_rect_pc34(3,
                                                                     32,
                                                                     29,
                                                                     &r),
                &r,
                214,
                0,
                32,
                29);
    expect_rect(dm1_v1_champion_damage_number_origin_pc34(3, &r),
                &r,
                236,
                11,
                0,
                0);
    expect_rect(dm1_v1_champion_damage_number_origin_variant_pc34(3,
                                                                  77,
                                                                  1,
                                                                  &r),
                &r,
                225,
                16,
                0,
                0);
    CHECK(dm1_v1_champion_damage_indicator_rect_pc34(4, 45, 7, &r) == 0);
    CHECK(dm1_v1_champion_poison_label_rect_pc34(0, 0, 6, &r) == 0);
}

int main(void) {
    CHECK(dm1_v1_champion_status_layout_source_evidence_pc34() != 0);
    test_status_boxes();
    test_bars_hands_and_names();
    test_colors_and_graphics();
    test_damage_and_poison();
    printf("dm1_v1_champion_status_layout: %d/%d assertions passed\n",
           g_assertions - g_failures,
           g_assertions);
    return g_failures == 0 ? 0 : 1;
}
