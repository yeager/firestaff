#include "firestaff/csb/v1/startup_entrance_pointer_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_passed;
static int g_failed;

unsigned int ENTRANCE_Compat_GetMouseRouteCount(void)
{
    return 5u;
}

int ENTRANCE_Compat_GetMouseRoute(unsigned int ordinal,
                                  EntranceMouseRouteCompat *outRoute)
{
    (void)ordinal;
    if (outRoute) {
        memset(outRoute, 0, sizeof(*outRoute));
    }
    return 0;
}

int ENTRANCE_Compat_HitTestMouseRoute(int screenX,
                                      int screenY,
                                      unsigned int buttonMask,
                                      EntranceMouseRouteCompat *outRoute)
{
    if (!outRoute) {
        return 0;
    }
    memset(outRoute, 0, sizeof(*outRoute));
    if (screenX == 244 && screenY == 45 &&
        buttonMask == ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT) {
        outRoute->commandId = 200u;
        return 1;
    }
    if (screenX == 244 && screenY == 45 &&
        buttonMask == ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT) {
        outRoute->commandId = 201u;
        return 1;
    }
    if (screenX == 244 && screenY == 76 &&
        buttonMask == ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT) {
        outRoute->commandId = 202u;
        return 1;
    }
    if (screenX == 248 && screenY == 186 &&
        buttonMask == ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT) {
        outRoute->commandId = 203u;
        return 1;
    }
    if (screenX == 243 && screenY == 110 &&
        buttonMask == ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT) {
        outRoute->commandId = 216u;
        return 1;
    }
    return 0;
}

const char *ENTRANCE_Compat_GetMouseRouteEvidence(void)
{
    return "test stub";
}

static void check(int condition, const char *message)
{
    if (condition) {
        ++g_passed;
        printf("PASS %s\n", message);
    } else {
        ++g_failed;
        printf("FAIL %s\n", message);
    }
}

static void expect_action(const char *message,
                          int x,
                          int y,
                          unsigned int mask,
                          CSB_V1_StartupEntrancePointerAction_PC34 expected,
                          int expectedCommand)
{
    CSB_V1_StartupEntrancePointerAction_PC34 action =
        CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34;
    int hit = csb_v1_startup_entrance_pointer_action_pc34(
        0, x, y, mask, &action);
    check(hit && action == expected &&
              csb_v1_startup_entrance_command_for_pointer_action_pc34(
                  action) == expectedCommand,
          message);
}

int main(void)
{
    CSB_V1_StartupEntrancePointerAction_PC34 action =
        CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_QUIT_PC34;
    CSB_V1_StartupTickState_PC34 tick;
    CSB_V1_StartupTickResult_PC34 result;
    CSB_V1_StartupRenderState_PC34 render_state;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupCommandState_PC34 command_state;
    CSB_V1_StartupEntranceDecision_PC34 decision;
    char phase[64];
    int startup_active;
    int startup_frame;
    int i;

    expect_action("enter route becomes CSB startup Enter",
                  244, 45,
                  ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                  CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_ENTER_DUNGEON_PC34,
                  200);
    expect_action("bonus route becomes CSB startup Bonus",
                  244, 45,
                  ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT,
                  CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_ENTER_BONUS_DUNGEON_PC34,
                  201);
    expect_action("resume route becomes CSB startup Resume",
                  244, 76,
                  ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                  CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_RESUME_PC34,
                  202);
    expect_action("credits route becomes CSB startup Credits",
                  248, 186,
                  ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                  CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_DRAW_CREDITS_PC34,
                  203);
    expect_action("quit route becomes CSB startup Quit",
                  243, 110,
                  ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                  CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_QUIT_PC34,
                  216);

    check(!csb_v1_startup_entrance_pointer_action_pc34(
              0, 1, 1, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, &action) &&
              action == CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34,
          "outside route is ignored");
    action = CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_QUIT_PC34;
    check(csb_v1_startup_entrance_pointer_action_pc34(
              1, 244, 45, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, &action) &&
              action == CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34,
          "credits-active click is consumed as dismiss");
    check(csb_v1_startup_entrance_pointer_action_pc34(
              0, 244, 45, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, NULL) == 0,
          "NULL output action is rejected");

    memset(&tick, 0, sizeof(tick));
    tick.title_active = 1;
    tick.title_frame = csb_v1_startup_title_total_ticks_pc34() - 1;
    check(csb_v1_startup_advance_tick_pc34(&tick, &result) &&
              result.redraw &&
              result.title_finished &&
              !tick.title_active &&
              tick.title_frame == csb_v1_startup_title_total_ticks_pc34() &&
              tick.entrance_source_step == 1 &&
              tick.entrance_frame == 0,
          "startup tick hands title to entrance source step");

    memset(&tick, 0, sizeof(tick));
    tick.entrance_source_step = 1;
    for (i = 0; i < csb_v1_startup_entrance_wait_stage_pc34() - 1; ++i) {
        (void)csb_v1_startup_advance_tick_pc34(&tick, &result);
    }
    check(tick.entrance_source_step ==
              csb_v1_startup_entrance_wait_stage_pc34() &&
              result.reached_entrance_wait,
          "startup tick advances entrance prelude to wait stage");

    memset(&tick, 0, sizeof(tick));
    tick.credits_active = 1;
    tick.credits_remaining_ticks = 1;
    check(csb_v1_startup_advance_tick_pc34(&tick, &result) &&
              result.credits_finished &&
              !tick.credits_active &&
              tick.credits_remaining_ticks == 0,
          "startup tick owns credits timeout");

    memset(&tick, 0, sizeof(tick));
    tick.opening_active = 1;
    tick.opening_delay_ticks =
        csb_v1_startup_entrance_pre_open_delay_ticks_pc34();
    tick.opening_step = 1;
    tick.door_step_count = 3;
    for (i = 0; i < csb_v1_startup_entrance_pre_open_delay_ticks_pc34(); ++i) {
        (void)csb_v1_startup_advance_tick_pc34(&tick, &result);
    }
    check(tick.opening_delay_ticks == 0 &&
              tick.opening_step == 1 &&
              !result.door_opening_finished,
          "startup tick consumes pre-open delay before door steps");
    (void)csb_v1_startup_advance_tick_pc34(&tick, &result);
    check(tick.opening_step == 2 && !result.door_opening_finished,
          "startup tick advances door opening steps");
    (void)csb_v1_startup_advance_tick_pc34(&tick, &result);
    (void)csb_v1_startup_advance_tick_pc34(&tick, &result);
    check(result.door_opening_finished,
          "startup tick reports door-opening completion");

    check(csb_v1_startup_entrance_credits_ticks_pc34() == 1800,
          "startup sequence owns credits timeout");

    memset(&render_state, 0, sizeof(render_state));
    render_state.entrance_active = 1;
    render_state.title_active = 1;
    render_state.title_frame = 0;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_TITLE_PC34 &&
              plan.title_source_step == 1 &&
              !plan.waiting_for_input,
          "startup render plan owns title surface");

    memset(&render_state, 0, sizeof(render_state));
    render_state.entrance_active = 1;
    render_state.entrance_source_step = 2;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34 &&
              !plan.waiting_for_input,
          "startup render plan owns entrance blackout");

    memset(&render_state, 0, sizeof(render_state));
    render_state.entrance_active = 1;
    render_state.entrance_frame = 0;
    render_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
              plan.waiting_for_input &&
              plan.blink_prompt_visible,
          "startup render plan owns closed entrance prompt");

    render_state.entrance_frame = 12;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              !plan.blink_prompt_visible,
          "startup render plan owns prompt blink cadence");

    memset(&render_state, 0, sizeof(render_state));
    render_state.entrance_active = 1;
    render_state.credits_active = 1;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34,
          "startup render plan owns credits surface");

    memset(&render_state, 0, sizeof(render_state));
    render_state.entrance_active = 1;
    render_state.opening_active = 1;
    render_state.opening_delay_ticks = 1;
    render_state.opening_step = 2;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34 &&
              plan.opening_step == 2,
          "startup render plan owns door pre-open surface");

    render_state.opening_delay_ticks = 0;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34 &&
              plan.opening_step == 2,
          "startup render plan owns door-opening frame surface");

    memset(&command_state, 0, sizeof(command_state));
    check(csb_v1_startup_init_command_state_pc34(&command_state, 0) &&
              command_state.title_active &&
              command_state.title_frame == 0 &&
              command_state.title_source_step ==
                  CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
              command_state.entrance_active &&
              !command_state.entrance_dismissed &&
              command_state.entrance_source_step == 0 &&
              !command_state.credits_active &&
              !command_state.opening_active &&
              command_state.pending_command == 0,
          "startup command state initializes new-game title");

    memset(&command_state, 0xff, sizeof(command_state));
    check(csb_v1_startup_init_command_state_pc34(&command_state, 1) &&
              !command_state.title_active &&
              command_state.title_frame == 0 &&
              command_state.title_source_step == 0 &&
              !command_state.entrance_active &&
              command_state.entrance_dismissed &&
              command_state.entrance_source_step == 0 &&
              !command_state.credits_active &&
              !command_state.opening_active &&
              command_state.pending_command == 0,
          "startup command state initializes resume runtime");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.title_active = 1;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    check(!csb_v1_startup_entrance_accepts_input_pc34(&command_state),
          "startup entrance gate blocks title phase input");

    command_state.title_active = 0;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34() - 1;
    check(!csb_v1_startup_entrance_accepts_input_pc34(&command_state),
          "startup entrance gate blocks pre-wait input");

    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    check(csb_v1_startup_entrance_accepts_input_pc34(&command_state),
          "startup entrance gate accepts wait-loop input");

    command_state.opening_active = 1;
    check(!csb_v1_startup_entrance_accepts_input_pc34(&command_state),
          "startup entrance gate blocks door-opening input");

    command_state.opening_active = 0;
    command_state.credits_active = 1;
    command_state.entrance_source_step = 0;
    check(csb_v1_startup_entrance_accepts_input_pc34(&command_state),
          "startup entrance gate accepts credits-dismiss input");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.credits_active = 1;
    command_state.credits_remaining_ticks = 99;
    check(csb_v1_startup_begin_door_opening_pc34(&command_state, 200) &&
              command_state.pending_command == 200 &&
              command_state.opening_active &&
              command_state.entrance_source_step ==
                  CSB_V1_STARTUP_STAGE_ENTRANCE_PRE_OPEN_DELAY_PC34 &&
              command_state.opening_delay_ticks ==
                  csb_v1_startup_entrance_pre_open_delay_ticks_pc34() &&
              command_state.opening_step == 1 &&
              !command_state.credits_active &&
              command_state.credits_remaining_ticks == 0,
          "startup command state begins door opening");

    check(csb_v1_startup_finish_door_opening_pc34(&command_state) &&
              !command_state.opening_active &&
              !command_state.entrance_active &&
              command_state.entrance_dismissed &&
              command_state.pending_command == 0 &&
              command_state.entrance_source_step == 0,
          "startup command state finishes door opening");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.opening_active = 1;
    command_state.opening_step = 3;
    check(csb_v1_startup_begin_credits_pc34(&command_state) &&
              command_state.credits_active &&
              command_state.credits_remaining_ticks ==
                  csb_v1_startup_entrance_credits_ticks_pc34() &&
              !command_state.opening_active &&
              command_state.opening_step == 0,
          "startup command state begins credits");

    check(csb_v1_startup_dismiss_credits_pc34(&command_state) &&
              !command_state.credits_active &&
              command_state.credits_remaining_ticks == 0,
          "startup command state dismisses credits");

    check(csb_v1_startup_entrance_command_for_action_pc34(
              CSB_V1_STARTUP_ENTRANCE_ACTION_ENTER_DUNGEON_PC34) ==
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              csb_v1_startup_entrance_command_for_action_pc34(
                  CSB_V1_STARTUP_ENTRANCE_ACTION_RESUME_PC34) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34 &&
              csb_v1_startup_entrance_command_for_action_pc34(
                  CSB_V1_STARTUP_ENTRANCE_ACTION_QUIT_PC34) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34 &&
              csb_v1_startup_entrance_command_for_action_pc34(
                  CSB_V1_STARTUP_ENTRANCE_ACTION_NONE_PC34) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
          "startup entrance actions resolve source command ids");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.credits_active = 1;
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &decision) &&
              decision ==
                  CSB_V1_STARTUP_ENTRANCE_DECISION_DISMISS_CREDITS_PC34,
          "startup command resolver dismisses credits before command dispatch");

    command_state.credits_active = 0;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34() - 1;
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &decision) &&
              decision == CSB_V1_STARTUP_ENTRANCE_DECISION_IGNORED_PC34,
          "startup command resolver blocks pre-wait commands");

    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    command_state.opening_active = 1;
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &decision) &&
              decision == CSB_V1_STARTUP_ENTRANCE_DECISION_IGNORED_PC34,
          "startup command resolver blocks opening commands");

    command_state.opening_active = 0;
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &decision) &&
              decision ==
                  CSB_V1_STARTUP_ENTRANCE_DECISION_ENTER_DUNGEON_PC34,
          "startup command resolver accepts dungeon entry command");
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_BONUS_DUNGEON_PC34,
              &decision) &&
              decision ==
                  CSB_V1_STARTUP_ENTRANCE_DECISION_ENTER_BONUS_DUNGEON_PC34,
          "startup command resolver accepts bonus dungeon command");
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34,
              &decision) &&
              decision == CSB_V1_STARTUP_ENTRANCE_DECISION_RESUME_PC34,
          "startup command resolver accepts resume command");
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_DRAW_CREDITS_PC34,
              &decision) &&
              decision ==
                  CSB_V1_STARTUP_ENTRANCE_DECISION_BEGIN_CREDITS_PC34,
          "startup command resolver accepts credits command");
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34,
              &decision) &&
              decision == CSB_V1_STARTUP_ENTRANCE_DECISION_QUIT_PC34,
          "startup command resolver accepts quit command");
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
              &decision) &&
              decision == CSB_V1_STARTUP_ENTRANCE_DECISION_IGNORED_PC34,
          "startup command resolver ignores empty command");

    command_state.title_active = 1;
    command_state.title_frame = 7;
    command_state.title_source_step = 2;
    command_state.entrance_active = 1;
    command_state.entrance_source_step = 4;
    command_state.credits_active = 1;
    command_state.credits_remaining_ticks = 1;
    command_state.opening_active = 1;
    command_state.opening_delay_ticks = 1;
    command_state.opening_step = 1;
    command_state.pending_command = 200;
    check(csb_v1_startup_quit_to_launcher_pc34(&command_state) &&
              !command_state.title_active &&
              command_state.title_frame == 0 &&
              command_state.title_source_step == 0 &&
              !command_state.entrance_active &&
              command_state.entrance_dismissed &&
              command_state.entrance_source_step == 0 &&
              !command_state.credits_active &&
              !command_state.opening_active &&
              command_state.pending_command == 0,
          "startup command state quits to launcher");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.title_active = 1;
    command_state.title_frame = 7;
    command_state.title_source_step = 2;
    check(csb_v1_startup_receipt_phase_pc34(
              &command_state, 44, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "csb-title-2") == 0 &&
              startup_active == 1 &&
              startup_frame == 7,
          "startup receipt phase reports title source step");

    command_state.title_active = 0;
    command_state.opening_active = 1;
    command_state.opening_step = 5;
    check(csb_v1_startup_receipt_phase_pc34(
              &command_state, 44, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "csb-entrance-opening-5") == 0 &&
              startup_active == 1 &&
              startup_frame == 44,
          "startup receipt phase reports door opening");

    command_state.opening_active = 0;
    command_state.credits_active = 1;
    check(csb_v1_startup_receipt_phase_pc34(
              &command_state, 44, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "csb-credits") == 0 &&
              startup_active == 1,
          "startup receipt phase reports credits");

    command_state.credits_active = 0;
    command_state.entrance_source_step = 4;
    check(csb_v1_startup_receipt_phase_pc34(
              &command_state, 44, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "csb-entrance-4") == 0 &&
              startup_active == 1,
          "startup receipt phase reports entrance source step");

    command_state.entrance_active = 0;
    check(csb_v1_startup_receipt_phase_pc34(
              &command_state, 44, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "csb-runtime") == 0 &&
              startup_active == 0,
          "startup receipt phase reports runtime");

    check(strstr(csb_v1_startup_entrance_pointer_source_evidence_pc34(),
                 "ENTRANCE.C") != NULL,
          "source evidence cites ENTRANCE.C");

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed ? 1 : 0;
}
