#include "dm1_v1_champion_panel_pressing_mouth_eye_statusbox_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int actual, int expected,
                      const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        fprintf(stderr, "FAIL %s got=%d expected=%d anchor=%s\n",
                label, actual, expected, anchor);
    }
}

static void check_contains(const char *label, const char *text,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!text || !strstr(text, needle)) {
        ++g_failures;
        fprintf(stderr, "FAIL %s missing='%s' anchor=%s\n",
                label, needle ? needle : "(null)", anchor);
    }
}

static int pixel_at(
    const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34 *frame,
    int x,
    int y)
{
    return frame->bytes[y * DM1_V1_CPMESB_STATUS_BOX_WIDTH_PC34 + x];
}

static void check_pixel(
    const char *label,
    const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34 *frame,
    int x,
    int y,
    int expected,
    const char *anchor)
{
    char full_label[128];

    snprintf(full_label, sizeof(full_label), "%s[%d,%d]", label, x, y);
    check_int(full_label, pixel_at(frame, x, y), expected, anchor);
}

static void check_frame_geometry(
    const char *prefix,
    const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34 *frame,
    int champion_index,
    int active_champion_index,
    int expected_ready_graphic,
    int expected_action_graphic)
{
    char label[128];
    int expected_zone;
    int expected_screen_x;

    expected_zone = 151 + champion_index;
    expected_screen_x = champion_index * 69;
    snprintf(label, sizeof(label), "%s.zone%d", prefix, champion_index);
    check_int(label, frame->zone_id, expected_zone, "DEFS.H:3783-3786");
    snprintf(label, sizeof(label), "%s.screenX%d", prefix, champion_index);
    check_int(label, frame->screen_x, expected_screen_x,
              "CHAMDRAW.C F0292:750 status-box stride");
    snprintf(label, sizeof(label), "%s.screenY%d", prefix, champion_index);
    check_int(label, frame->screen_y, 0, "CHAMDRAW.C F0292:773");
    snprintf(label, sizeof(label), "%s.width%d", prefix, champion_index);
    check_int(label, frame->width, 67, "CHAMDRAW.C F0292:773-775");
    snprintf(label, sizeof(label), "%s.height%d", prefix, champion_index);
    check_int(label, frame->height, 29, "CHAMDRAW.C F0292:773-775");
    snprintf(label, sizeof(label), "%s.redraw%d", prefix, champion_index);
    check_int(label, frame->redraws_status_box,
              champion_index == active_champion_index ? 1 : 0,
              "CHAMDRAW.C F0292:771-789");
    snprintf(label, sizeof(label), "%s.readyX%d", prefix, champion_index);
    check_int(label, frame->ready_hand_relative_x, 4,
              "layout-696 C211/C213/C215/C217");
    snprintf(label, sizeof(label), "%s.actionX%d", prefix, champion_index);
    check_int(label, frame->action_hand_relative_x, 24,
              "layout-696 C212/C214/C216/C218");
    snprintf(label, sizeof(label), "%s.handY%d", prefix, champion_index);
    check_int(label, frame->hand_relative_y, 10,
              "layout-696 C211..C218");
    if (champion_index == active_champion_index) {
        snprintf(label, sizeof(label), "%s.readyGraphic%d", prefix,
                 champion_index);
        check_int(label, frame->ready_hand_graphic, expected_ready_graphic,
                  "CHAMDRAW.C F0291:632-646");
        snprintf(label, sizeof(label), "%s.actionGraphic%d", prefix,
                 champion_index);
        check_int(label, frame->action_hand_graphic, expected_action_graphic,
                  "CHAMDRAW.C F0291:648-651");
        snprintf(label, sizeof(label), "%s.slotPixels%d", prefix,
                 champion_index);
        check_int(label, frame->slot_box_pixel_count, 648,
                  "two 18x18 status-hand slot boxes");
    }
}

static void check_active_pixels(
    const char *prefix,
    const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34 *frame,
    int ready_graphic,
    int action_graphic)
{
    char label[128];

    snprintf(label, sizeof(label), "%s.fill.topleft", prefix);
    check_pixel(label, frame, 0, 0, 12, "CHAMDRAW.C F0292:784-789");
    snprintf(label, sizeof(label), "%s.fill.topright", prefix);
    check_pixel(label, frame, 66, 0, 12, "CHAMDRAW.C F0292:773-786");
    snprintf(label, sizeof(label), "%s.fill.bottomleft", prefix);
    check_pixel(label, frame, 0, 28, 12, "CHAMDRAW.C F0292:773-786");
    snprintf(label, sizeof(label), "%s.fill.bottomright", prefix);
    check_pixel(label, frame, 66, 28, 12, "CHAMDRAW.C F0292:773-786");
    snprintf(label, sizeof(label), "%s.fill.center", prefix);
    check_pixel(label, frame, 55, 14, 12, "CHAMDRAW.C F0292:784-789");

    snprintf(label, sizeof(label), "%s.ready.topleft", prefix);
    check_pixel(label, frame, 4, 10, ready_graphic,
                "CHAMDRAW.C F0291:632-646");
    snprintf(label, sizeof(label), "%s.ready.center", prefix);
    check_pixel(label, frame, 12, 18, ready_graphic,
                "CHAMDRAW.C F0291:653-673");
    snprintf(label, sizeof(label), "%s.ready.bottomright", prefix);
    check_pixel(label, frame, 21, 27, ready_graphic,
                "layout-696 C211/C213/C215/C217");
    snprintf(label, sizeof(label), "%s.action.topleft", prefix);
    check_pixel(label, frame, 24, 10, action_graphic,
                "CHAMDRAW.C F0291:648-651");
    snprintf(label, sizeof(label), "%s.action.center", prefix);
    check_pixel(label, frame, 32, 18, action_graphic,
                "CHAMDRAW.C F0291:653-673");
    snprintf(label, sizeof(label), "%s.action.bottomright", prefix);
    check_pixel(label, frame, 41, 27, action_graphic,
                "layout-696 C212/C214/C216/C218");
}

static void check_inactive_pixels(
    const char *prefix,
    const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34 *frame)
{
    char label[128];

    snprintf(label, sizeof(label), "%s.inactive.0.0", prefix);
    check_pixel(label, frame, 0, 0, 0, "inactive status box untouched");
    snprintf(label, sizeof(label), "%s.inactive.4.10", prefix);
    check_pixel(label, frame, 4, 10, 0, "inactive status box untouched");
    snprintf(label, sizeof(label), "%s.inactive.24.10", prefix);
    check_pixel(label, frame, 24, 10, 0, "inactive status box untouched");
    snprintf(label, sizeof(label), "%s.inactive.66.28", prefix);
    check_pixel(label, frame, 66, 28, 0, "inactive status box untouched");
}

static DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPlanPc34 build_or_fail(
    const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxInputPc34 *input,
    const char *anchor)
{
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPlanPc34 plan;

    if (!dm1_v1_champion_panel_pressing_mouth_eye_statusbox_plan_pc34(
            input, &plan)) {
        memset(&plan, 0, sizeof(plan));
        ++g_assertions;
        ++g_failures;
        fprintf(stderr, "FAIL build_or_fail anchor=%s\n", anchor);
    } else {
        ++g_assertions;
    }
    return plan;
}

static void test_contract_and_evidence(void)
{
    const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPc34Contract *contract;
    const char *evidence;

    contract = dm1_v1_champion_panel_pressing_mouth_eye_statusbox_contract_pc34();
    evidence =
        dm1_v1_champion_panel_pressing_mouth_eye_statusbox_source_evidence_pc34();

    check_int("contract.only", contract->contract_only, 1,
              "contract-only no game data");
    check_int("contract.champions", contract->champion_count, 4,
              "DEFS.H C151..C154");
    check_int("contract.firstZone", contract->status_box_first_zone, 151,
              "DEFS.H:3783");
    check_int("contract.lastZone", contract->status_box_last_zone, 154,
              "DEFS.H:3786");
    check_int("contract.width", contract->status_box_width, 67,
              "CHAMDRAW.C F0292:773-775");
    check_int("contract.height", contract->status_box_height, 29,
              "CHAMDRAW.C F0292:773-775");
    check_int("contract.stride", contract->status_box_stride, 69,
              "CHAMDRAW.C F0292:750");
    check_int("contract.fill", contract->status_box_fill_color, 12,
              "CHAMDRAW.C F0292:784-789");
    check_int("contract.readySlot", contract->ready_hand_slot, 0,
              "DEFS.H slot C00");
    check_int("contract.actionSlot", contract->action_hand_slot, 1,
              "DEFS.H slot C01");
    check_int("contract.slotSize", contract->slot_box_size, 18,
              "layout-696 C211..C218");
    check_int("contract.normalGfx", contract->slot_box_normal_graphic, 33,
              "DEFS.H:2193");
    check_int("contract.woundedGfx", contract->slot_box_wounded_graphic, 34,
              "DEFS.H:2194");
    check_int("contract.actingGfx", contract->slot_box_acting_graphic, 35,
              "DEFS.H:2195");
    check_int("contract.poisonGfx", contract->poison_label_graphic, 32,
              "PANEL.C F0345:1601-1606");
    check_int("contract.poisonZone", contract->poison_label_zone, 502,
              "PANEL.C F0345:1606");
    check_int("contract.mouth", contract->mouth_command, 70,
              "DEFS.H:303");
    check_int("contract.eye", contract->eye_command, 71,
              "DEFS.H:304");
    check_contains("contract.f0291", contract->draw_slot_anchor, "498-677",
                   "CHAMDRAW.C F0291");
    check_contains("contract.f0292", contract->draw_state_anchor, "1060-1087",
                   "CHAMDRAW.C F0292");
    check_contains("contract.f0302", contract->slotbox_route_anchor, "677-711",
                   "CHAMPION.C F0302");
    check_contains("contract.f0284", contract->leader_rotation_anchor,
                   "117-130", "CHAMPION.C F0284");
    check_contains("contract.poison", contract->poison_overlay_anchor,
                   "1601-1606", "PANEL.C F0345");
    check_contains("contract.defs", contract->defs_anchor, "C151-C154",
                   "DEFS.H");
    check_contains("source.f0291", evidence, "F0291:498-677",
                   "source evidence");
    check_contains("source.f0292", evidence, "F0292:771-789",
                   "source evidence");
    check_contains("source.g0331", evidence, "G0333 mouth/G0331 eye",
                   "source evidence");
    check_contains("source.f0302", evidence, "F0302:677-711",
                   "source evidence");
    check_contains("source.f0284", evidence, "F0284:117-130",
                   "source evidence");
    check_contains("source.text", evidence, "TEXT.C F0041:1254-1264",
                   "source evidence");
    check_contains("source.poison", evidence, "PANEL.C F0345:1601-1606",
                   "source evidence");
    check_contains("source.defs", evidence, "DEFS.H:3783-3786",
                   "source evidence");
}

static void test_eye_statusbox_pre_post(void)
{
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxInputPc34 input;
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPlanPc34 plan;

    memset(&input, 0, sizeof(input));
    input.active_champion_index = 1;
    input.inventory_champion_index = 1;
    input.acting_champion_index = 1;
    input.leader_empty_handed = 1;
    input.poison_banner_champion_index = -1;
    input.press = DM1_V1_CPMESB_PRESS_EYE_PC34;

    plan = build_or_fail(&input, "CHAMDRAW.C F0292:1064-1078 G0331 eye");
    check_int("eye.valid", plan.valid, 1, "plan valid");
    check_int("eye.active", plan.active_champion_index, 1, "C152 active");
    check_int("eye.g0331", plan.g0331_presses_eye, 1,
              "CHAMDRAW.C F0292:1064");
    check_int("eye.g0333", plan.g0333_presses_mouth, 0,
              "CHAMDRAW.C F0292:1061");
    check_int("eye.route", plan.panel_redraw_route, 351,
              "CHAMDRAW.C F0292:1065-1067");
    check_int("eye.viewport", plan.viewport_redraw_requested, 1,
              "CHAMDRAW.C F0292:1078");
    check_int("eye.status", plan.status_box_redraw_requested, 1,
              "CHAMDRAW.C F0292:771-789");
    check_int("eye.changed", plan.post_tick_changed_byte_count, 0,
              "one tick later press-held status box stable");
    check_int("eye.poisonOff", plan.poison_banner_active, 0,
              "no poison overlay in eye-only case");
    check_int("eye.poisonChampion", plan.poison_banner_champion_index, -1,
              "no poison overlay champion");

    check_frame_geometry("eye.pre", &plan.pre_tick[1], 1, 1, 33, 35);
    check_frame_geometry("eye.post", &plan.post_tick[1], 1, 1, 33, 35);
    check_active_pixels("eye.pre", &plan.pre_tick[1], 33, 35);
    check_active_pixels("eye.post", &plan.post_tick[1], 33, 35);
    check_inactive_pixels("eye.pre.champ0", &plan.pre_tick[0]);
    check_inactive_pixels("eye.pre.champ3", &plan.pre_tick[3]);
}

static void test_mouth_statusbox_pre_post(void)
{
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxInputPc34 input;
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPlanPc34 plan;

    memset(&input, 0, sizeof(input));
    input.active_champion_index = 0;
    input.inventory_champion_index = 0;
    input.acting_champion_index = 0;
    input.leader_empty_handed = 1;
    input.poison_banner_champion_index = -1;
    input.press = DM1_V1_CPMESB_PRESS_MOUTH_PC34;

    plan = build_or_fail(&input, "CHAMDRAW.C F0292:1061-1063 G0333 mouth");
    check_int("mouth.valid", plan.valid, 1, "plan valid");
    check_int("mouth.active", plan.active_champion_index, 0, "C151 active");
    check_int("mouth.g0331", plan.g0331_presses_eye, 0,
              "CHAMDRAW.C F0292:1064");
    check_int("mouth.g0333", plan.g0333_presses_mouth, 1,
              "CHAMDRAW.C F0292:1061");
    check_int("mouth.route", plan.panel_redraw_route, 345,
              "CHAMDRAW.C F0292:1061-1063");
    check_int("mouth.changed", plan.post_tick_changed_byte_count, 0,
              "one tick later press-held status box stable");

    check_frame_geometry("mouth.pre", &plan.pre_tick[0], 0, 0, 33, 35);
    check_frame_geometry("mouth.post", &plan.post_tick[0], 0, 0, 33, 35);
    check_active_pixels("mouth.pre", &plan.pre_tick[0], 33, 35);
    check_active_pixels("mouth.post", &plan.post_tick[0], 33, 35);
    check_inactive_pixels("mouth.pre.champ1", &plan.pre_tick[1]);
    check_inactive_pixels("mouth.post.champ2", &plan.post_tick[2]);
}

static void test_eye_with_other_champion_poison_overlay(void)
{
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxInputPc34 input;
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPlanPc34 plan;

    memset(&input, 0, sizeof(input));
    input.active_champion_index = 1;
    input.inventory_champion_index = 1;
    input.acting_champion_index = 1;
    input.leader_empty_handed = 1;
    input.poison_banner_active = 1;
    input.poison_banner_champion_index = 3;
    input.press = DM1_V1_CPMESB_PRESS_EYE_PC34;

    plan = build_or_fail(&input,
                         "PANEL.C F0345:1601-1606 poison on champion 3");
    check_int("poison.valid", plan.valid, 1, "plan valid");
    check_int("poison.active", plan.active_champion_index, 1, "C152 active");
    check_int("poison.champion", plan.poison_banner_champion_index, 3,
              "different champion poison overlay");
    check_int("poison.activeFlag", plan.poison_banner_active, 1,
              "PANEL.C F0345:1601");
    check_int("poison.zone", plan.poison_overlay_zone, 502,
              "PANEL.C F0345:1606");
    check_int("poison.graphic", plan.poison_overlay_graphic, 32,
              "DEFS.H C032 and PANEL.C F0345:1606");
    check_int("poison.different", plan.poison_overlay_on_different_champion, 1,
              "test fixture requires different champion");
    check_int("poison.noStatusBytes",
              plan.poison_overlay_touches_status_box_bytes, 0,
              "C502 overlay is outside C151..C154");
    check_int("poison.changed", plan.post_tick_changed_byte_count, 0,
              "one tick later poison overlay isolated");

    check_frame_geometry("poison.active.pre", &plan.pre_tick[1], 1, 1, 33, 35);
    check_active_pixels("poison.active.pre", &plan.pre_tick[1], 33, 35);
    check_active_pixels("poison.active.post", &plan.post_tick[1], 33, 35);
    check_frame_geometry("poison.other.pre", &plan.pre_tick[3], 3, 1, 33, 35);
    check_inactive_pixels("poison.other.pre", &plan.pre_tick[3]);
    check_inactive_pixels("poison.other.post", &plan.post_tick[3]);
}

static void test_validation_guards(void)
{
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxInputPc34 input;
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPlanPc34 plan;

    memset(&input, 0, sizeof(input));
    input.active_champion_index = 4;
    input.inventory_champion_index = 0;
    input.acting_champion_index = 0;
    input.poison_banner_champion_index = -1;
    input.press = DM1_V1_CPMESB_PRESS_EYE_PC34;
    check_int("invalidChampion",
              dm1_v1_champion_panel_pressing_mouth_eye_statusbox_plan_pc34(
                  &input, &plan),
              0, "synthetic champion domain 0..3");
    check_int("invalidChampionFlag", plan.rejected_invalid_champion, 1,
              "synthetic champion domain 0..3");

    input.active_champion_index = 0;
    input.press =
        (DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPressPc34)999;
    check_int("invalidPress",
              dm1_v1_champion_panel_pressing_mouth_eye_statusbox_plan_pc34(
                  &input, &plan),
              0, "C070/C071 only");
    check_int("invalidPressFlag", plan.rejected_invalid_press, 1,
              "C070/C071 only");

    input.press = DM1_V1_CPMESB_PRESS_EYE_PC34;
    input.poison_banner_active = 1;
    input.poison_banner_champion_index = 0;
    check_int("samePoisonChampion",
              dm1_v1_champion_panel_pressing_mouth_eye_statusbox_plan_pc34(
                  &input, &plan),
              0, "poison overlay fixture must use a different champion");
    check_int("samePoisonChampionFlag", plan.rejected_same_poison_champion, 1,
              "poison overlay fixture must use a different champion");

    check_int("nullOutput",
              dm1_v1_champion_panel_pressing_mouth_eye_statusbox_plan_pc34(
                  &input, NULL),
              0, "null output guard");
}

int main(void)
{
    printf("== DM1 V1 champion panel pressing mouth/eye status-box slice ==\n");
    test_contract_and_evidence();
    test_eye_statusbox_pre_post();
    test_mouth_statusbox_pre_post();
    test_eye_with_other_champion_poison_overlay();
    test_validation_guards();

    if (g_assertions < 80) {
        fprintf(stderr, "FAIL assertion_count got=%d expected>=80\n",
                g_assertions);
        return 1;
    }
    if (g_failures != 0) {
        fprintf(stderr, "FAILURES %d / %d assertions\n",
                g_failures, g_assertions);
        return 1;
    }
    printf("PASS test_dm1_v1_champion_panel_pressing_mouth_eye_statusbox_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
