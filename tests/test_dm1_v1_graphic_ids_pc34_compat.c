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
    DM1_V1_ObjectIconSourceZonePc34 portrait;
    DM1_V1_ObjectIconSourceZonePc34 zone;
    ASSERT_TRUE(strstr(dm1_v1_graphic_ids_source_evidence_pc34(),
                       "DEFS.H C006") != NULL,
                "source evidence names DEFS.H");
    ASSERT_TRUE(strstr(dm1_v1_graphic_ids_source_evidence_pc34(),
                       "C093..C107") != NULL,
                "source evidence names wall graphics");
    ASSERT_EQ(dm1_v1_graphic_the_end_pc34(), 6, "the end");
    ASSERT_EQ(dm1_v1_graphic_dialog_box_pc34(), 17, "dialog box");
    ASSERT_EQ(dm1_v1_graphic_inventory_backdrop_pc34(), 17, "inventory backdrop");
    ASSERT_EQ(dm1_v1_graphic_panel_empty_pc34(), 20, "panel empty");
    ASSERT_EQ(dm1_v1_graphic_panel_open_scroll_pc34(), 23, "open scroll");
    ASSERT_EQ(dm1_v1_graphic_champion_portraits_pc34(), 26, "portraits");
    ASSERT_TRUE(dm1_v1_graphic_champion_portrait_source_zone_pc34(
                    13, &portrait),
                "C026 portrait ordinal resolves");
    ASSERT_EQ(portrait.graphic_index, 26, "C026 portrait graphic");
    ASSERT_EQ(portrait.x, 160, "C026 ordinal 13 source x");
    ASSERT_EQ(portrait.y, 29, "C026 ordinal 13 source y");
    ASSERT_EQ(portrait.w, 32, "C026 portrait source width");
    ASSERT_EQ(portrait.h, 29, "C026 portrait source height");
    ASSERT_TRUE(!dm1_v1_graphic_champion_portrait_source_zone_pc34(
                     -1, &portrait) &&
                    !dm1_v1_graphic_champion_portrait_source_zone_pc34(
                     DM1_V1_CHAMPION_PORTRAIT_COUNT_PC34, &portrait),
                "C026 portrait rejects out-of-range ordinal");
    ASSERT_TRUE(dm1_v1_graphic_validate_champion_portrait_atlas_pc34(256, 87) &&
                    !dm1_v1_graphic_validate_champion_portrait_atlas_pc34(255, 87) &&
                    !dm1_v1_graphic_validate_champion_portrait_atlas_pc34(256, 86),
                "C026 portrait requires native 8x3 atlas dimensions");
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
    ASSERT_EQ(dm1_v1_graphic_wallset0_index_pc34(0), 93, "wall d0r graphic");
    ASSERT_EQ(dm1_v1_graphic_wallset0_index_pc34(4), 97, "wall d1c graphic");
    ASSERT_EQ(dm1_v1_graphic_wallset0_index_pc34(14), 107, "wall d3c graphic");
    ASSERT_EQ(dm1_v1_graphic_wallset0_index_pc34(-1), -1, "negative wall rejected");
    ASSERT_EQ(dm1_v1_graphic_wallset0_index_pc34(15), -1, "past wall rejected");
    ASSERT_EQ(dm1_v1_graphic_materialized_wallset_index_pc34(0, 86),
              86, "wallset0 materialized first");
    ASSERT_EQ(dm1_v1_graphic_materialized_wallset_index_pc34(2, 97),
              177, "wallset2 d1c materialized");
    ASSERT_EQ(dm1_v1_graphic_materialized_wallset_index_pc34(-1, 97),
              97, "negative map wallset clamps");
    ASSERT_EQ(dm1_v1_graphic_materialized_wallset_index_pc34(3, 70),
              70, "non-wallset graphic passes through");
    ASSERT_EQ(dm1_v1_graphic_endgame_champion_mirror_pc34(), 346,
              "endgame champion mirror");
    ASSERT_TRUE(dm1_v1_object_icon_source_zone_pc34(0, &zone),
                "object icon 0 source zone");
    ASSERT_EQ(zone.graphic_index, 42, "object icon 0 graphic");
    ASSERT_EQ(zone.x, 0, "object icon 0 x");
    ASSERT_EQ(zone.y, 0, "object icon 0 y");
    ASSERT_EQ(zone.w, 16, "object icon w");
    ASSERT_EQ(zone.h, 16, "object icon h");
    ASSERT_TRUE(dm1_v1_object_icon_source_zone_pc34(31, &zone),
                "object icon 31 source zone");
    ASSERT_EQ(zone.graphic_index, 42, "object icon 31 graphic");
    ASSERT_EQ(zone.x, 240, "object icon 31 x");
    ASSERT_EQ(zone.y, 16, "object icon 31 y");
    ASSERT_TRUE(dm1_v1_object_icon_source_zone_pc34(32, &zone),
                "object icon 32 source zone");
    ASSERT_EQ(zone.graphic_index, 43, "object icon 32 graphic");
    ASSERT_EQ(zone.x, 0, "object icon 32 x");
    ASSERT_EQ(zone.y, 0, "object icon 32 y");
    ASSERT_TRUE(!dm1_v1_object_icon_source_zone_pc34(-1, &zone),
                "negative icon rejected");
    ASSERT_TRUE(!dm1_v1_object_icon_source_zone_pc34(0, NULL),
                "null zone rejected");

    if (g_fail) {
        fprintf(stderr, "dm1_v1_graphic_ids_pc34_compat: %d failed, %d passed\n",
                g_fail, g_pass);
        return 1;
    }
    printf("dm1_v1_graphic_ids_pc34_compat: %d passed\n", g_pass);
    return 0;
}
