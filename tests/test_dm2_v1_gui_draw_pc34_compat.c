#include "dm2_v1_gui_draw_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_total  = 0;

#define RUN(fn) do { \
    tests_total++; \
    fn(); \
    tests_passed++; \
    printf("  PASS  %s\n", #fn); \
} while (0)

/* ── Null-safety tests ─────────────────────────────────────────────── */

static void test_draw_icon_pict_buff_null_cb(void) {
    DM2_V1_Rect r = {0,0,10,10};
    dm2_v1_draw_icon_pict_buff(NULL, 0, &r, 0, 0, 0, 0, NULL, NULL, NULL);
}

static void test_draw_icon_pict_entry_null_cb(void) {
    dm2_v1_draw_icon_pict_entry(0, 0, 0, 0, 0, 0, NULL, NULL);
}

static void test_draw_dialogue_progress_null_cb(void) {
    DM2_V1_DrawDialogueProgressReceipt r;
    memset(&r, 0xFF, sizeof(r));
    dm2_v1_draw_dialogue_progress(500, NULL, NULL, &r);
    assert(!r.progress_drawn);
}

static void test_draw_dialogue_progress_null_receipt(void) {
    DM2_V1_GuiDrawCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    dm2_v1_draw_dialogue_progress(500, &cb, NULL, NULL);
}

static void test_draw_wake_up_text_null_cb(void) {
    dm2_v1_draw_wake_up_text(NULL, NULL);
}

static void test_draw_player_3stat_health_bar_null_cb(void) {
    dm2_v1_draw_player_3stat_health_bar(0, NULL, NULL);
}

static void test_draw_cur_max_hms_null_cb(void) {
    dm2_v1_draw_cur_max_hms(0, 50, 100, NULL, NULL);
}

static void test_draw_player_3stat_text_null_cb(void) {
    dm2_v1_draw_player_3stat_text(0, NULL, NULL);
}

static void test_draw_player_name_at_cmdslot_null_cb(void) {
    dm2_v1_draw_player_name_at_cmdslot(NULL, NULL);
}

static void test_draw_player_damage_null_cb(void) {
    dm2_v1_draw_player_damage(0, NULL, NULL);
}

static void test_draw_chip_of_magic_map_null_cb(void) {
    dm2_v1_draw_chip_of_magic_map(NULL, 0, 0, 0, 0, NULL, NULL, NULL);
}

static void test_query_gdat_squad_icon_null_cb(void) {
    dm2_v1_query_gdat_squad_icon(NULL, 0, NULL, NULL, NULL);
}

static void test_draw_cryocell_lever_null_cb(void) {
    dm2_v1_draw_cryocell_lever(0, NULL, NULL);
}

static void test_draw_charsheet_option_icon_null_cb(void) {
    dm2_v1_draw_charsheet_option_icon(0, 0, 0, NULL, NULL);
}

static void test_money_box_survey_null_cb(void) {
    dm2_v1_money_box_survey(0, NULL, NULL);
}

static void test_draw_moneybox_null_cb(void) {
    dm2_v1_draw_moneybox(0, NULL, NULL);
}

static void test_draw_player_3stat_pane_null_cb(void) {
    dm2_v1_draw_player_3stat_pane(0, 0, NULL, NULL);
}

static void test_draw_cmd_slot_null_cb(void) {
    dm2_v1_draw_cmd_slot(0, 0, NULL, NULL);
}

static void test_draw_spell_to_be_cast_null_cb(void) {
    dm2_v1_draw_spell_to_be_cast(0, NULL, NULL);
}

static void test_draw_player_attack_dir_null_cb(void) {
    dm2_v1_draw_player_attack_dir(NULL, NULL);
}

static void test_draw_spell_panel_null_cb(void) {
    dm2_v1_draw_spell_panel(NULL, NULL);
}

static void test_show_attack_result_null_cb(void) {
    dm2_v1_show_attack_result(0, NULL, NULL);
}

static void test_draw_squad_spell_null_cb(void) {
    dm2_v1_draw_squad_spell_and_leader_icon(0, 0, NULL, NULL);
}

static void test_draw_food_water_poison_null_cb(void) {
    dm2_v1_draw_food_water_poison_panel(NULL, NULL);
}

static void test_draw_item_stats_bar_null_cb(void) {
    dm2_v1_draw_item_stats_bar(0, 0, 100, 'A', 5, NULL, NULL);
}

static void test_draw_item_in_hand_null_cb(void) {
    int dummy;
    dm2_v1_draw_item_in_hand(&dummy, NULL, NULL);
}

static void test_draw_container_panel_null_cb(void) {
    dm2_v1_draw_container_panel(0, 0, NULL, NULL);
}

typedef struct ContainerPanelFixture {
    int image_queries;
    int blits;
    int8_t classes[2];
    int8_t subs[2];
    int8_t indices[2];
} ContainerPanelFixture;

static void *container_panel_image(void *ctx, int8_t cls, int8_t sub,
                                   int8_t idx) {
    ContainerPanelFixture *fixture = ctx;
    int slot = fixture->image_queries++;
    if (slot < 2) {
        fixture->classes[slot] = cls;
        fixture->subs[slot] = sub;
        fixture->indices[slot] = idx;
    }
    return fixture;
}

static DM2_V1_Rect *container_panel_blit_rect(void *ctx, void *bmp,
                                               DM2_V1_Rect *out,
                                               int16_t id, int16_t *srcx,
                                               int16_t *srcy) {
    (void)ctx;
    (void)bmp;
    (void)id;
    *srcx = 0;
    *srcy = 0;
    *out = (DM2_V1_Rect){0, 0, 16, 16};
    return out;
}

static int16_t container_panel_bmp_width(void *ctx, void *bmp) {
    (void)ctx;
    (void)bmp;
    return 16;
}

static void *container_panel_bmp(void *ctx, int16_t dbidx) {
    (void)dbidx;
    return ctx;
}

static void container_panel_blit(void *ctx, void *src, void *dst,
                                 DM2_V1_Rect *rect, int16_t srcx,
                                 int16_t srcy, int16_t srcw, int16_t dstw,
                                 int16_t alpha, int32_t mode, int32_t srcbpp,
                                 int32_t dstbpp, void *palette) {
    ContainerPanelFixture *fixture = ctx;
    (void)src;
    (void)dst;
    (void)rect;
    (void)srcx;
    (void)srcy;
    (void)srcw;
    (void)dstw;
    (void)alpha;
    (void)mode;
    (void)srcbpp;
    (void)dstbpp;
    (void)palette;
    fixture->blits++;
}

static void test_draw_container_panel_keeps_frame_without_cls2_query(void) {
    ContainerPanelFixture fixture = {0};
    DM2_V1_GuiDrawCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.query_gdat_image = container_panel_image;
    cb.query_blit_rect = container_panel_blit_rect;
    cb.get_bmp_width = container_panel_bmp_width;
    cb.get_bmp = container_panel_bmp;
    cb.blit = container_panel_blit;

    dm2_v1_draw_container_panel(0, 1, &cb, &fixture);

    assert(fixture.image_queries == 2);
    assert(fixture.blits == 2);
    assert(fixture.classes[0] == 20 && fixture.classes[1] == 20);
    assert(fixture.subs[0] == 0 && fixture.subs[1] == 0);
    assert(fixture.indices[0] == 0x10 && fixture.indices[1] == 0x12);
}

typedef struct ItemStatsBarFixture {
    int16_t rect_id;
    int16_t scale_num;
    int16_t scale_den;
    int scale_calls;
} ItemStatsBarFixture;

static DM2_V1_Rect *item_stats_bar_rect(void *ctx, int16_t id,
                                       DM2_V1_Rect *out) {
    (void)ctx;
    (void)id;
    *out = (DM2_V1_Rect){10, 20, 40, 5};
    return out;
}

static DM2_V1_Rect *item_stats_bar_scale(void *ctx, int16_t id,
                                         int16_t num, int16_t den,
                                         DM2_V1_Rect *out) {
    ItemStatsBarFixture *fixture = ctx;
    fixture->rect_id = id;
    fixture->scale_num = num;
    fixture->scale_den = den;
    fixture->scale_calls++;
    *out = (DM2_V1_Rect){10, 20, 30, 5};
    return out;
}

static void test_draw_item_stats_bar_maps_helper_arguments(void) {
    ItemStatsBarFixture fixture = {0};
    DM2_V1_GuiDrawCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.query_expanded_rect = item_stats_bar_rect;
    cb.scale_rect = item_stats_bar_scale;

    dm2_v1_draw_item_stats_bar(0x1f0, 75, 100, 'K', 5, &cb, &fixture);

    assert(fixture.scale_calls == 1);
    assert(fixture.rect_id == 0x1f0);
    assert(fixture.scale_num == 7500);
    assert(fixture.scale_den == 10000);
}

static void test_draw_item_icon_null_cb(void) {
    dm2_v1_draw_item_icon(0, 0, 0, 0, 0, NULL, NULL);
}

static void test_draw_container_survey_null_cb(void) {
    int dummy;
    dm2_v1_draw_container_survey(&dummy, NULL, NULL);
}

static void test_draw_eye_mouth_null_cb(void) {
    dm2_v1_draw_eye_mouth_colored_rectangle(0, 0, NULL, NULL);
}

static void test_draw_scroll_text_null_cb(void) {
    dm2_v1_draw_scroll_text(0, NULL, NULL);
}

static void test_draw_item_survey_null_cb(void) {
    DM2_V1_DrawItemSurveyReceipt r;
    memset(&r, 0xFF, sizeof(r));
    int32_t res = dm2_v1_draw_item_survey(0, 0, NULL, NULL, &r);
    assert(res == 0);
    assert(!r.survey_drawn);
}

static void test_draw_hand_action_icons_null_cb(void) {
    dm2_v1_draw_hand_action_icons(0, 0, 0, NULL, NULL);
}

static void test_draw_map_chip_null_cb(void) {
    dm2_v1_draw_map_chip(0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
}

static void test_draw_majic_map_null_cb(void) {
    DM2_V1_DrawMajicMapReceipt r;
    memset(&r, 0xFF, sizeof(r));
    dm2_v1_draw_majic_map(0, NULL, NULL, &r);
    assert(!r.map_drawn);
}

static void test_display_hint_new_line_null_cb(void) {
    dm2_v1_display_hint_new_line(NULL, NULL);
}

static void test_display_taken_item_name_null_cb(void) {
    dm2_v1_display_taken_item_name(0, NULL, NULL);
}

static void test_highlight_arrow_panel_null_cb(void) {
    dm2_v1_highlight_arrow_panel(0, 0, 0, NULL, NULL);
}

static void test_display_right_panel_squad_hands_null_cb(void) {
    dm2_v1_display_right_panel_squad_hands(NULL, NULL);
}

static void test_refresh_player_stat_disp_null_cb(void) {
    DM2_V1_RefreshPlayerStatDispReceipt r;
    memset(&r, 0xFF, sizeof(r));
    dm2_v1_refresh_player_stat_disp(0, NULL, NULL, &r);
    assert(!r.stats_refreshed);
}

static void test_guidraw_29ee_000f_null_cb(void) {
    DM2_V1_GuiDraw000fReceipt r;
    memset(&r, 0xFF, sizeof(r));
    int32_t res = dm2_v1_guidraw_29ee_000f(NULL, NULL, &r);
    assert(res == 0);
    assert(!r.panel_drawn);
}

static void test_guidraw_24a5_1798_null_cb(void) {
    DM2_V1_GuiDraw1798Receipt r;
    memset(&r, 0xFF, sizeof(r));
    dm2_v1_guidraw_24a5_1798(0, NULL, NULL, &r);
    assert(!r.executed);
}

static void test_update_right_panel_null_cb(void) {
    DM2_V1_UpdateRightPanelReceipt r;
    memset(&r, 0xFF, sizeof(r));
    dm2_v1_update_right_panel(0, NULL, NULL, &r);
    assert(!r.panel_updated);
}

/* ── Internal function null safety ─────────────────────────────────── */

static void test_guidraw_0b36_0c52_null_cb(void) {
    dm2_v1_guidraw_0b36_0c52(0, 0, 0, NULL, NULL);
}

static void test_guidraw_29ee_00a3_null_cb(void) {
    dm2_v1_guidraw_29ee_00a3(0, NULL, NULL);
}

static void test_guidraw_0b36_105b_null_cb(void) {
    DM2_V1_Rect r = {0,0,10,10};
    dm2_v1_guidraw_0b36_105b(0, &r, 0, NULL, NULL);
}

static void test_guidraw_24a5_0e82_null_cb(void) {
    dm2_v1_guidraw_24a5_0e82(0, 0, 0, 0, 100, 0, NULL, NULL);
}

static void test_guidraw_2405_014a_null_cb(void) {
    int8_t res = dm2_v1_guidraw_2405_014a(0, 0, 0, NULL, NULL);
    assert(res == 0x18);
}

static void test_guidraw_2405_011f_null_cb(void) {
    DM2_V1_Rect out;
    dm2_v1_guidraw_2405_011f(0, &out, NULL, NULL);
}

static void test_guidraw_2405_00ec_null_cb(void) {
    DM2_V1_Rect out;
    dm2_v1_guidraw_2405_00ec(0, &out, NULL, NULL);
}

static void test_guidraw_2e62_03b5_null_cb(void) {
    int32_t res = dm2_v1_guidraw_2e62_03b5(0, 0, 0, NULL, NULL);
    assert(res == 0);
}

static void test_guidraw_29ee_1d03_null_cb(void) {
    dm2_v1_guidraw_29ee_1d03(0, NULL, NULL);
}

static void test_guidraw_29ee_1946_null_cb(void) {
    dm2_v1_guidraw_29ee_1946(0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
}

/* ── Data table verification ───────────────────────────────────────── */

static void test_table1d275a_entries(void) {
    assert(dm2_guidraw_table1d275a[0][0] == -2);
    assert(dm2_guidraw_table1d275a[0][1] == 10);
    assert(dm2_guidraw_table1d275a[16][0] == 0);
    assert(dm2_guidraw_table1d275a[16][1] == -11);
}

static void test_table1d69d0_entries(void) {
    assert(dm2_guidraw_table1d69d0[0] == 0x08);
    assert(dm2_guidraw_table1d69d0[3] == 0x02);
}

static void test_table1d67d9_entries(void) {
    assert(dm2_guidraw_table1d67d9[0] == 0);
    assert(dm2_guidraw_table1d67d9[1] == 1);
}

static void test_v1d1124_slash(void) {
    assert(dm2_guidraw_v1d1124[0] == '/');
    assert(dm2_guidraw_v1d1124[1] == '\0');
}

/* ── Callback dispatch tests ───────────────────────────────────────── */

static bool mock_v1e0200_called = false;
static bool mock_get_v1e0200(void *ctx) {
    (void)ctx;
    mock_v1e0200_called = true;
    return false;
}

static void test_dialogue_progress_callback_dispatch(void) {
    DM2_V1_GuiDrawCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.get_v1e0200 = mock_get_v1e0200;
    mock_v1e0200_called = false;

    DM2_V1_DrawDialogueProgressReceipt r;
    dm2_v1_draw_dialogue_progress(500, &cb, NULL, &r);
    assert(mock_v1e0200_called);
    assert(!r.progress_drawn);
}

static int16_t mock_get_hero_hp(void *ctx, int32_t idx) {
    (void)ctx; (void)idx;
    return 50;
}

static int16_t mock_get_hero_max_hp(void *ctx, int32_t idx) {
    (void)ctx; (void)idx;
    return 100;
}

static void test_refresh_stat_disp_callback_dispatch(void) {
    DM2_V1_GuiDrawCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.get_hero_hp = mock_get_hero_hp;
    cb.get_hero_max_hp = mock_get_hero_max_hp;

    DM2_V1_RefreshPlayerStatDispReceipt r;
    dm2_v1_refresh_player_stat_disp(0, &cb, NULL, &r);
    assert(r.stats_refreshed);
}

static int16_t mock_get_v1e0288(void *ctx) { (void)ctx; return 0; }
static int16_t mock_get_heros(void *ctx) { (void)ctx; return 0; }

static void test_update_right_panel_no_heroes(void) {
    DM2_V1_GuiDrawCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.get_v1e0288 = mock_get_v1e0288;
    cb.get_heros_in_party = mock_get_heros;

    DM2_V1_UpdateRightPanelReceipt r;
    dm2_v1_update_right_panel(0, &cb, NULL, &r);
    assert(r.panel_updated);
}

static void test_guidraw_2405_014a_no_flags(void) {
    DM2_V1_GuiDrawCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    int8_t res = dm2_v1_guidraw_2405_014a(0, 0, 0, &cb, NULL);
    assert(res == 0x18);
}

int main(void) {
    printf("test_dm2_v1_gui_draw_pc34_compat\n");

    /* Null-safety: public functions */
    RUN(test_draw_icon_pict_buff_null_cb);
    RUN(test_draw_icon_pict_entry_null_cb);
    RUN(test_draw_dialogue_progress_null_cb);
    RUN(test_draw_dialogue_progress_null_receipt);
    RUN(test_draw_wake_up_text_null_cb);
    RUN(test_draw_player_3stat_health_bar_null_cb);
    RUN(test_draw_cur_max_hms_null_cb);
    RUN(test_draw_player_3stat_text_null_cb);
    RUN(test_draw_player_name_at_cmdslot_null_cb);
    RUN(test_draw_player_damage_null_cb);
    RUN(test_draw_chip_of_magic_map_null_cb);
    RUN(test_query_gdat_squad_icon_null_cb);
    RUN(test_draw_cryocell_lever_null_cb);
    RUN(test_draw_charsheet_option_icon_null_cb);
    RUN(test_money_box_survey_null_cb);
    RUN(test_draw_moneybox_null_cb);
    RUN(test_draw_player_3stat_pane_null_cb);
    RUN(test_draw_cmd_slot_null_cb);
    RUN(test_draw_spell_to_be_cast_null_cb);
    RUN(test_draw_player_attack_dir_null_cb);
    RUN(test_draw_spell_panel_null_cb);
    RUN(test_show_attack_result_null_cb);
    RUN(test_draw_squad_spell_null_cb);
    RUN(test_draw_food_water_poison_null_cb);
    RUN(test_draw_item_stats_bar_null_cb);
    RUN(test_draw_item_in_hand_null_cb);
    RUN(test_draw_container_panel_null_cb);
    RUN(test_draw_container_panel_keeps_frame_without_cls2_query);
    RUN(test_draw_item_stats_bar_maps_helper_arguments);
    RUN(test_draw_item_icon_null_cb);
    RUN(test_draw_container_survey_null_cb);
    RUN(test_draw_eye_mouth_null_cb);
    RUN(test_draw_scroll_text_null_cb);
    RUN(test_draw_item_survey_null_cb);
    RUN(test_draw_hand_action_icons_null_cb);
    RUN(test_draw_map_chip_null_cb);
    RUN(test_draw_majic_map_null_cb);
    RUN(test_display_hint_new_line_null_cb);
    RUN(test_display_taken_item_name_null_cb);
    RUN(test_highlight_arrow_panel_null_cb);
    RUN(test_display_right_panel_squad_hands_null_cb);
    RUN(test_refresh_player_stat_disp_null_cb);
    RUN(test_guidraw_29ee_000f_null_cb);
    RUN(test_guidraw_24a5_1798_null_cb);
    RUN(test_update_right_panel_null_cb);

    /* Null-safety: internal functions */
    RUN(test_guidraw_0b36_0c52_null_cb);
    RUN(test_guidraw_29ee_00a3_null_cb);
    RUN(test_guidraw_0b36_105b_null_cb);
    RUN(test_guidraw_24a5_0e82_null_cb);
    RUN(test_guidraw_2405_014a_null_cb);
    RUN(test_guidraw_2405_011f_null_cb);
    RUN(test_guidraw_2405_00ec_null_cb);
    RUN(test_guidraw_2e62_03b5_null_cb);
    RUN(test_guidraw_29ee_1d03_null_cb);
    RUN(test_guidraw_29ee_1946_null_cb);

    /* Data tables */
    RUN(test_table1d275a_entries);
    RUN(test_table1d69d0_entries);
    RUN(test_table1d67d9_entries);
    RUN(test_v1d1124_slash);

    /* Callback dispatch */
    RUN(test_dialogue_progress_callback_dispatch);
    RUN(test_refresh_stat_disp_callback_dispatch);
    RUN(test_update_right_panel_no_heroes);
    RUN(test_guidraw_2405_014a_no_flags);

    printf("\n%d / %d tests passed\n", tests_passed, tests_total);
    return (tests_passed == tests_total) ? 0 : 1;
}
