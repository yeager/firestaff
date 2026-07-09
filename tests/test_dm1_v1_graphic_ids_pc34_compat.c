#include "dm1_v1_graphic_ids_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass;
static int g_fail;

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

#define ASSERT_TRUE(expr, msg) do { \
    if (expr) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

int main(void) {
    ASSERT_TRUE(strstr(dm1_v1_graphic_ids_source_evidence_pc34(),
                       "DEFS.H C006") != NULL,
                "source evidence names DEFS.H");
    ASSERT_EQ(dm1_v1_graphic_the_end_pc34(), 6, "the end");
    ASSERT_EQ(dm1_v1_graphic_dialog_box_pc34(), 17, "dialog box");
    ASSERT_EQ(dm1_v1_graphic_inventory_backdrop_pc34(), 17, "inventory backdrop");
    ASSERT_EQ(dm1_v1_graphic_panel_empty_pc34(), 20, "panel empty");
    ASSERT_EQ(dm1_v1_graphic_panel_open_scroll_pc34(), 23, "open scroll");
    ASSERT_EQ(dm1_v1_graphic_champion_portraits_pc34(), 26, "portraits");
    ASSERT_EQ(dm1_v1_graphic_champion_icons_pc34(), 28, "champion icons");
    ASSERT_EQ(dm1_v1_graphic_object_description_circle_pc34(), 29, "circle");
    ASSERT_EQ(dm1_v1_graphic_food_label_pc34(), 30, "food label");
    ASSERT_EQ(dm1_v1_graphic_water_label_pc34(), 31, "water label");
    ASSERT_EQ(dm1_v1_graphic_poisoned_label_pc34(), 32, "poison label");
    ASSERT_EQ(dm1_v1_graphic_slot_box_normal_pc34(), 33, "slot normal");
    ASSERT_EQ(dm1_v1_graphic_slot_box_wounded_pc34(), 34, "slot wounded");
    ASSERT_EQ(dm1_v1_graphic_slot_box_acting_hand_pc34(), 35, "slot acting");
    ASSERT_EQ(dm1_v1_graphic_party_shield_border_pc34(), 37, "party shield");
    ASSERT_EQ(dm1_v1_graphic_fire_shield_border_pc34(), 38, "fire shield");
    ASSERT_EQ(dm1_v1_graphic_spell_shield_border_pc34(), 39, "spell shield");
    ASSERT_EQ(dm1_v1_graphic_arrow_or_eye_pc34(0), 18, "panel arrow");
    ASSERT_EQ(dm1_v1_graphic_arrow_or_eye_pc34(1), 19, "panel eye");
    ASSERT_EQ(dm1_v1_graphic_creature_damage_pc34(), 14, "creature damage");
    ASSERT_EQ(dm1_v1_graphic_champion_damage_small_pc34(), 15, "small damage");
    ASSERT_EQ(dm1_v1_graphic_champion_damage_big_pc34(), 16, "big damage");
    ASSERT_EQ(dm1_v1_graphic_endgame_champion_mirror_pc34(), 346,
              "endgame champion mirror");

    if (g_fail) {
        fprintf(stderr, "dm1_v1_graphic_ids_pc34_compat: %d failed, %d passed\n",
                g_fail, g_pass);
        return 1;
    }
    printf("dm1_v1_graphic_ids_pc34_compat: %d passed\n", g_pass);
    return 0;
}
