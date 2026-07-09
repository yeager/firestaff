#include "dm1_v1_layout_zones_pc34_compat.h"

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

static void assert_rect(DM1_V1_LayoutZoneRectPc34 rect,
                        int x,
                        int y,
                        int w,
                        int h,
                        const char* label) {
    ASSERT_EQ(rect.x, x, label);
    ASSERT_EQ(rect.y, y, label);
    ASSERT_EQ(rect.w, w, label);
    ASSERT_EQ(rect.h, h, label);
}

int main(void) {
    DM1_V1_LayoutZoneRectPc34 rect;
    int x = -1;
    int y = -1;

    ASSERT_TRUE(strstr(dm1_v1_layout_zones_source_evidence_pc34(),
                       "COORD.C F0635") != NULL,
                "source evidence names COORD.C");
    ASSERT_EQ(dm1_v1_screen_zone_id_pc34(), 2, "screen zone id");
    assert_rect(dm1_v1_screen_rect_pc34(), 0, 0, 320, 200, "screen rect");
    ASSERT_EQ(dm1_v1_screen_centered_dialog_zone_id_pc34(), 5,
              "centered dialog zone id");
    assert_rect(dm1_v1_screen_centered_dialog_rect_pc34(), 48, 32, 224, 136,
                "centered dialog rect");
    ASSERT_EQ(dm1_v1_explosion_pattern_d0c_zone_id_pc34(), 4,
              "explosion zone id");
    assert_rect(dm1_v1_explosion_pattern_d0c_rect_pc34(), 0, 0, 32, 29,
                "explosion rect");
    ASSERT_EQ(dm1_v1_viewport_centered_text_zone_id_pc34(), 6,
              "viewport centered text zone id");
    ASSERT_TRUE(dm1_v1_viewport_centered_text_rect_pc34(80, 16, &rect),
                "viewport centered text rect valid");
    assert_rect(rect, 72, 60, 80, 16, "viewport centered text rect");
    ASSERT_TRUE(!dm1_v1_viewport_centered_text_rect_pc34(0, 16, &rect),
                "viewport centered text rejects empty width");
    ASSERT_EQ(dm1_v1_message_area_zone_id_pc34(), 15, "message zone id");
    assert_rect(dm1_v1_message_area_rect_pc34(), 0, 173, 320, 27,
                "message rect");
    ASSERT_EQ(dm1_v1_viewport_zone_id_pc34(), 7, "viewport zone id");
    assert_rect(dm1_v1_viewport_rect_pc34(), 0, 33, 224, 136, "viewport rect");
    ASSERT_EQ(dm1_v1_leader_hand_object_name_zone_id_pc34(), 17,
              "leader hand object name zone id");
    assert_rect(dm1_v1_leader_hand_object_name_rect_pc34(), 233, 33, 87, 6,
                "leader hand object name rect");
    ASSERT_EQ(dm1_v1_champion_icon_zone_id_pc34(0), 113, "champion icon 0 id");
    ASSERT_EQ(dm1_v1_champion_icon_zone_id_pc34(3), 116, "champion icon 3 id");
    ASSERT_EQ(dm1_v1_champion_icon_zone_id_pc34(4), 0, "champion icon bad id");
    ASSERT_TRUE(dm1_v1_champion_icon_rect_pc34(2, &rect),
                "champion icon 2 rect valid");
    assert_rect(rect, 301, 15, 19, 14, "champion icon 2 rect");
    assert_rect(dm1_v1_inventory_backdrop_rect_pc34(), 0, 33, 224, 136,
                "inventory backdrop rect");
    ASSERT_EQ(dm1_v1_inventory_panel_zone_id_pc34(), 101, "panel zone id");
    assert_rect(dm1_v1_inventory_panel_rect_pc34(), 80, 52, 144, 73,
                "panel rect");
    ASSERT_EQ(dm1_v1_object_description_circle_zone_id_pc34(), 504,
              "object circle id");
    assert_rect(dm1_v1_object_description_circle_rect_pc34(), 103, 53, 32, 27,
                "object circle rect");
    ASSERT_EQ(dm1_v1_object_description_icon_zone_id_pc34(), 505,
              "object icon id");
    assert_rect(dm1_v1_object_description_icon_rect_pc34(), 111, 59, 16, 16,
                "object icon rect");
    ASSERT_EQ(dm1_v1_arrow_or_eye_zone_id_pc34(), 503, "arrow/eye id");
    assert_rect(dm1_v1_arrow_or_eye_rect_pc34(), 83, 57, 16, 9,
                "arrow/eye rect");
    ASSERT_EQ(dm1_v1_object_description_name_zone_id_pc34(), 506,
              "object name id");
    ASSERT_TRUE(dm1_v1_object_description_name_rect_for_text_pc34(48, 7, &rect),
                "object name rect valid");
    assert_rect(rect, 134, 64, 48, 7, "object name rect");
    ASSERT_TRUE(!dm1_v1_object_description_name_rect_for_text_pc34(0, 7, &rect),
                "object name rejects empty width");
    ASSERT_TRUE(dm1_v1_object_description_continuation_origin_pc34(&x, &y),
                "object continuation origin valid");
    ASSERT_EQ(x, 108, "object continuation x");
    ASSERT_EQ(y, 59, "object continuation y");

    if (g_fail) {
        fprintf(stderr, "dm1_v1_layout_zones_pc34_compat: %d failed, %d passed\n",
                g_fail, g_pass);
        return 1;
    }
    printf("dm1_v1_layout_zones_pc34_compat: %d passed\n", g_pass);
    return 0;
}
