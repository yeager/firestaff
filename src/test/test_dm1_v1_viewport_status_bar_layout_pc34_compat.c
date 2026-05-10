#include "dm1_v1_viewport_status_bar_layout_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL:%d: %s\n", __LINE__, #expr); \
        failures++; \
    } \
} while (0)

int main(void) {
    int failures = 0;
    int c;

    CHECK(DM1_V1_STATUS_BOX_SPACING == 69);
    CHECK(DM1_V1_STATUS_BAR_WIDTH == 4);
    CHECK(DM1_V1_STATUS_BAR_HEIGHT == 25);
    CHECK(DM1_V1_STATUS_BAR_TOP_Y == 2);
    CHECK(DM1_V1_STATUS_BAR_FIRST_X == 46);
    CHECK(DM1_V1_STATUS_BAR_STAT_SPACING == 7);
    CHECK(DM1_V1_STATUS_BAR_BLANK_COLOR == 12);
    CHECK(DM1_V1_CHAMPION_COLOR_0 == 7);
    CHECK(DM1_V1_CHAMPION_COLOR_1 == 11);
    CHECK(DM1_V1_CHAMPION_COLOR_2 == 8);
    CHECK(DM1_V1_CHAMPION_COLOR_3 == 14);

    for (c = 0; c < DM1_V1_PARTY_CHAMPION_COUNT; ++c) {
        CHECK(dm1_v1_status_bar_graph_zone_id(c) == 187 + c);
        CHECK(dm1_v1_status_bar_value_zone_id(c, DM1_V1_STATUS_BAR_HEALTH) == 195 + c);
        CHECK(dm1_v1_status_bar_value_zone_id(c, DM1_V1_STATUS_BAR_STAMINA) == 199 + c);
        CHECK(dm1_v1_status_bar_value_zone_id(c, DM1_V1_STATUS_BAR_MANA) == 203 + c);
        CHECK(dm1_v1_status_bar_x(c, DM1_V1_STATUS_BAR_HEALTH) == c * 69 + 46);
        CHECK(dm1_v1_status_bar_x(c, DM1_V1_STATUS_BAR_STAMINA) == c * 69 + 53);
        CHECK(dm1_v1_status_bar_x(c, DM1_V1_STATUS_BAR_MANA) == c * 69 + 60);
    }

    CHECK(dm1_v1_status_bar_graph_zone_id(-1) == 0);
    CHECK(dm1_v1_status_bar_value_zone_id(4, 0) == 0);
    CHECK(dm1_v1_status_bar_value_zone_id(0, 3) == 0);
    CHECK(dm1_v1_status_bar_x(0, 3) == -1);

    CHECK(dm1_v1_status_bar_fill_height(0, 100) == 0);
    CHECK(dm1_v1_status_bar_fill_height(1, 1000) == 1);
    CHECK(dm1_v1_status_bar_fill_height(50, 100) == 12);
    CHECK(dm1_v1_status_bar_blank_height(50, 100) == 13);
    CHECK(dm1_v1_status_bar_fill_height(100, 100) == 25);
    CHECK(dm1_v1_status_bar_fill_height(125, 100) == 25);

    CHECK(DM1_V1_HEALTH_VALUE_ZONE == 550);
    CHECK(DM1_V1_STAMINA_VALUE_SOURCE_ZONE == 551);
    CHECK(DM1_V1_MANA_VALUE_SOURCE_ZONE == 552);
    CHECK(DM1_V1_HEALTH_VALUE_X == 95 && DM1_V1_HEALTH_VALUE_Y == 116);
    CHECK(DM1_V1_STAMINA_VALUE_X == 95 && DM1_V1_STAMINA_VALUE_Y == 124);
    CHECK(DM1_V1_MANA_VALUE_X == 95 && DM1_V1_MANA_VALUE_Y == 132);

    CHECK(DM1_V1_SKILL_VALUE_ZONE == 557);
    CHECK(DM1_V1_SKILL_VALUE_REL_X == 28);
    CHECK(DM1_V1_SKILL_VALUE_REL_Y == 6);
    CHECK(DM1_V1_SKILL_LINE_HEIGHT == 7);
    CHECK(DM1_V1_SKILL_LEVEL_MIN_VISIBLE == 2);
    CHECK(DM1_V1_SKILL_LEVEL_MAX_CLAMP == 16);
    CHECK(dm1_v1_skill_level_text_y(0) == 6);
    CHECK(dm1_v1_skill_level_text_y(1) == 13);
    CHECK(dm1_v1_skill_level_text_y(2) == 20);
    CHECK(dm1_v1_skill_level_text_y(3) == 27);
    CHECK(dm1_v1_skill_level_text_y(4) == -1);

    CHECK(DM1_V1_XP_BAR_PRESENT == 0);
    CHECK(DM1_V1_XP_BAR_WIDTH == 0);
    CHECK(DM1_V1_XP_BAR_HEIGHT == 0);

    if (failures) {
        fprintf(stderr, "DM1 V1 viewport status bar layout gate failed: %d assertion(s)\n", failures);
        return EXIT_FAILURE;
    }
    puts("PASS: DM1 V1 viewport status bar layout source-lock invariants");
    return EXIT_SUCCESS;
}
