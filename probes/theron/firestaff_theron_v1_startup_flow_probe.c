#include "theron_v1_startup_flow.h"

#include <stdio.h>
#include <string.h>

static int g_pass;
static int g_fail;

static void check_int(const char *label, int got, int expected) {
    if (got == expected) {
        printf("PASS %s=%d\n", label, got);
        ++g_pass;
    } else {
        printf("FAIL %s got=%d expected=%d\n", label, got, expected);
        ++g_fail;
    }
}

static void check_contains(const char *label, const char *haystack, const char *needle) {
    if (haystack && needle && strstr(haystack, needle)) {
        printf("PASS %s contains %s\n", label, needle);
        ++g_pass;
    } else {
        printf("FAIL %s missing %s in %s\n",
               label,
               needle ? needle : "(null)",
               haystack ? haystack : "(null)");
        ++g_fail;
    }
}

static void check_str(const char *label, const char *got, const char *expected) {
    if (got && expected && strcmp(got, expected) == 0) {
        printf("PASS %s=%s\n", label, got);
        ++g_pass;
    } else {
        printf("FAIL %s got=%s expected=%s\n",
               label,
               got ? got : "(null)",
               expected ? expected : "(null)");
        ++g_fail;
    }
}

static int fake_runtime_level_load(Theron_V1_World *world,
                                   Theron_DungeonID dungeon_id,
                                   void *userdata,
                                   char *receipt,
                                   size_t receipt_cap) {
    int *called = (int*)userdata;

    if (called) {
        ++(*called);
    }
    if (!world || dungeon_id < 1 || dungeon_id > THERON_DUNGEON_COUNT) {
        return 0;
    }
    world->current_dungeon = dungeon_id;
    world->current_level = 0;
    world->level_loaded[dungeon_id - 1][0] = 1;
    world->party.leader_x = 5;
    world->party.leader_y = 6;
    world->party.leader_dir = 1;
    if (receipt && receipt_cap > 0u) {
        snprintf(receipt,
                 receipt_cap,
                 "fake level stage=%d start=(5,6,1)",
                 (int)dungeon_id);
    }
    return 1;
}

static int fake_runtime_level_load_fail(Theron_V1_World *world,
                                        Theron_DungeonID dungeon_id,
                                        void *userdata,
                                        char *receipt,
                                        size_t receipt_cap) {
    int *called = (int*)userdata;

    (void)world;
    (void)dungeon_id;
    if (called) {
        ++(*called);
    }
    if (receipt && receipt_cap > 0u) {
        snprintf(receipt, receipt_cap, "forced load failure");
    }
    return 0;
}

int main(void) {
    Theron_DungeonProgression progression;
    Theron_StartupFlow flow;
    Theron_V1_Party party;
    Theron_V1_World world;
    Theron_StartupResult result;
    char phase_label[64];
    int startup_active = -1;

    printf("=== Theron V1 startup flow probe ===\n");
    printf("source: %s\n", theron_v1_startup_flow_source_evidence());

    theron_v1_dungeon_progression_init(&progression);
    theron_v1_startup_flow_init(&flow);
    check_int("init phase", flow.phase, THERON_STARTUP_PHASE_TITLE);
    check_int("init selected dungeon", flow.selected_dungeon, THERON_DUNGEON_INVALID);
    check_int("init companion count", flow.companion_count, 0);
    check_int("Firestaff none input maps to Theron idle",
              theron_v1_startup_input_from_firestaff_menu_code(0),
              THERON_STARTUP_INPUT_NONE);
    check_int("Firestaff up input maps to Theron up",
              theron_v1_startup_input_from_firestaff_menu_code(1),
              THERON_STARTUP_INPUT_UP);
    check_int("Firestaff down input maps to Theron down",
              theron_v1_startup_input_from_firestaff_menu_code(2),
              THERON_STARTUP_INPUT_DOWN);
    check_int("Firestaff left input maps to Theron left",
              theron_v1_startup_input_from_firestaff_menu_code(3),
              THERON_STARTUP_INPUT_LEFT);
    check_int("Firestaff right input maps to Theron right",
              theron_v1_startup_input_from_firestaff_menu_code(4),
              THERON_STARTUP_INPUT_RIGHT);
    check_int("Firestaff turn-left input maps to Theron left",
              theron_v1_startup_input_from_firestaff_menu_code(7),
              THERON_STARTUP_INPUT_LEFT);
    check_int("Firestaff turn-right input maps to Theron right",
              theron_v1_startup_input_from_firestaff_menu_code(8),
              THERON_STARTUP_INPUT_RIGHT);
    check_int("Firestaff accept input maps to Theron accept",
              theron_v1_startup_input_from_firestaff_menu_code(9),
              THERON_STARTUP_INPUT_ACCEPT);
    check_int("Firestaff back input maps to Theron back",
              theron_v1_startup_input_from_firestaff_menu_code(10),
              THERON_STARTUP_INPUT_BACK);
    check_int("Firestaff action input maps to Theron action",
              theron_v1_startup_input_from_firestaff_menu_code(11),
              THERON_STARTUP_INPUT_ACTION);
    check_int("unknown Firestaff input maps to Theron idle",
              theron_v1_startup_input_from_firestaff_menu_code(999),
              THERON_STARTUP_INPUT_NONE);
    check_int("startup availability stage 1",
              theron_v1_startup_stage_available(
                  &progression,
                  THERON_DUNGEON_1_HALL_OF_RECORDS),
              1);
    check_int("startup availability locked stage 2",
              theron_v1_startup_stage_available(
                  &progression,
                  THERON_DUNGEON_2_CRYPT_OF_SHADOWS),
              0);
    check_int("startup availability invalid stage",
              theron_v1_startup_stage_available(
                  &progression,
                  THERON_DUNGEON_INVALID),
              0);

    {
        int selected = -1;
        result = theron_v1_startup_show_stage_select(
            &flow,
            THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
        check_int("show stage-select rc", result, THERON_STARTUP_OK);
        check_int("show stage-select phase",
                  flow.phase,
                  THERON_STARTUP_PHASE_STAGE_SELECT);
        check_int("show stage-select selected dungeon",
                  flow.selected_dungeon,
                  THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
        result = theron_v1_startup_toggle_mirror(&flow, 0, &selected);
        check_int("toggle mirror before choose rejected",
                  result,
                  THERON_STARTUP_ERR_NO_STAGE);
        result = theron_v1_startup_choose_stage(
            &flow,
            &progression,
            THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("toggle fixture choose rc", result, THERON_STARTUP_OK);
        result = theron_v1_startup_toggle_mirror(&flow, 2, &selected);
        check_int("toggle mirror selects rc", result, THERON_STARTUP_OK);
        check_int("toggle mirror selected flag", selected, 1);
        check_int("toggle mirror selected mask",
                  flow.selected_mirrors_mask,
                  1 << 2);
        result = theron_v1_startup_toggle_mirror(&flow, 2, &selected);
        check_int("toggle mirror deselects rc", result, THERON_STARTUP_OK);
        check_int("toggle mirror deselected flag", selected, 0);
        check_int("toggle mirror deselected mask",
                  flow.selected_mirrors_mask,
                  0);
    }

    theron_v1_startup_flow_init(&flow);
    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("mirror before stage rejected", result, THERON_STARTUP_ERR_NO_STAGE);

    {
        Theron_StartupFlow rebuilt;
        Theron_StartupFlowSnapshot snapshot;
        theron_v1_startup_flow_init(&flow);
        result = theron_v1_startup_choose_stage(
            &flow,
            &progression,
            THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("snapshot stage choose rc", result, THERON_STARTUP_OK);
        check_int("snapshot select mirror 6 rc",
                  theron_v1_startup_select_mirror(&flow, 6),
                  THERON_STARTUP_OK);
        check_int("snapshot select mirror 2 rc",
                  theron_v1_startup_select_mirror(&flow, 2),
                  THERON_STARTUP_OK);
        check_int("snapshot select mirror 0 rc",
                  theron_v1_startup_select_mirror(&flow, 0),
                  THERON_STARTUP_OK);
        theron_v1_startup_flow_capture_snapshot(&flow, &snapshot);
        check_int("snapshot phase", snapshot.phase, THERON_STARTUP_PHASE_READY);
        check_int("snapshot companion count", snapshot.companion_count, 3);
        check_int("snapshot order 0", snapshot.selected_mirror_order[0], 6);
        check_int("snapshot order 1", snapshot.selected_mirror_order[1], 2);
        check_int("snapshot order 2", snapshot.selected_mirror_order[2], 0);
        snapshot.selected_mirror_order[1] = 6;
        result = theron_v1_startup_flow_rebuild_from_snapshot(
            &snapshot,
            &progression,
            &rebuilt);
        check_int("snapshot rebuild rc", result, THERON_STARTUP_OK);
        check_int("snapshot rebuild phase", rebuilt.phase, THERON_STARTUP_PHASE_READY);
        check_int("snapshot rebuild companion count", rebuilt.companion_count, 3);
        check_int("snapshot rebuild order 0", rebuilt.selected_mirror_order[0], 6);
        check_int("snapshot rebuild order 1 skips duplicate",
                  rebuilt.selected_mirror_order[1],
                  0);
        check_int("snapshot rebuild order 2 fills masked mirror",
                  rebuilt.selected_mirror_order[2],
                  2);
    }

    {
        Theron_StartupAction action;
        Theron_StartupHit hit;
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_TITLE,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("title accept action rc", result, THERON_STARTUP_OK);
        check_int("title accept shows stage select",
                  action.kind,
                  THERON_STARTUP_ACTION_SHOW_STAGE_SELECT);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            1,
            THERON_STARTUP_INPUT_UP,
            &action);
        check_int("stage up wraps to continue rc", result, THERON_STARTUP_OK);
        check_int("stage up action moves cursor",
                  action.kind,
                  THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR);
        check_int("stage up focuses continue", action.continue_focus, 1);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_3_ABYSS_OF_FLAMES,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_DOWN,
            &action);
        check_int("stage down rc", result, THERON_STARTUP_OK);
        check_int("stage down selected dungeon",
                  action.selected_dungeon,
                  THERON_DUNGEON_4_TOMB_OF_WOE);
        result = theron_v1_startup_handle_input_with_progression(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            &progression,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_DOWN,
            &action);
        check_int("initial locked stages skipped rc", result, THERON_STARTUP_OK);
        check_int("initial locked stages keep stage 1",
                  action.selected_dungeon,
                  THERON_DUNGEON_1_HALL_OF_RECORDS);
        {
            Theron_DungeonProgression progressed;
            theron_v1_dungeon_progression_init(&progressed);
            (void)theron_v1_dungeon_advance(&progressed);
            result = theron_v1_startup_handle_input_with_progression(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                THERON_DUNGEON_1_HALL_OF_RECORDS,
                &progressed,
                0,
                0,
                0,
                THERON_STARTUP_INPUT_DOWN,
                &action);
            check_int("post-first stage down rc", result, THERON_STARTUP_OK);
            check_int("post-first stage down skips complete stage",
                      action.selected_dungeon,
                      THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
            result = theron_v1_startup_handle_input_with_progression(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                THERON_DUNGEON_6_CASTLE_OF_FATE,
                &progressed,
                0,
                0,
                0,
                THERON_STARTUP_INPUT_DOWN,
                &action);
            check_int("locked final stage skipped rc", result, THERON_STARTUP_OK);
            check_int("locked final stage skipped",
                      action.selected_dungeon,
                      THERON_DUNGEON_6_CASTLE_OF_FATE);
            result = theron_v1_startup_handle_input_with_progression(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                &progressed,
                0,
                0,
                1,
                THERON_STARTUP_INPUT_UP,
                &action);
            check_int("post-first stage up rc", result, THERON_STARTUP_OK);
            check_int("post-first stage up focuses continue",
                      action.continue_focus,
                      1);
            check_int("post-first stage up keeps first available",
                      action.selected_dungeon,
                      THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
        }
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_4_TOMB_OF_WOE,
            0,
            1,
            1,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("stage continue accept rc", result, THERON_STARTUP_OK);
        check_int("stage continue accept action",
                  action.kind,
                  THERON_STARTUP_ACTION_CONTINUE_SAVE);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_4_TOMB_OF_WOE,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_ACTION,
            &action);
        check_int("stage choose action rc", result, THERON_STARTUP_OK);
        check_int("stage choose action kind",
                  action.kind,
                  THERON_STARTUP_ACTION_CHOOSE_STAGE);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_SOUL_ROOM,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_LEFT,
            &action);
        check_int("soul left rc", result, THERON_STARTUP_OK);
        check_int("soul left wraps to forcefield",
                  action.cursor,
                  THERON_STARTUP_HERO_MIRROR_COUNT);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            THERON_STARTUP_HERO_MIRROR_COUNT,
            0,
            0,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("ready forcefield accept rc", result, THERON_STARTUP_OK);
        check_int("ready forcefield accept action",
                  action.kind,
                  THERON_STARTUP_ACTION_ENTER_FORCEFIELD);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            3,
            0,
            0,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("ready mirror accept rc", result, THERON_STARTUP_OK);
        check_int("ready mirror accept action",
                  action.kind,
                  THERON_STARTUP_ACTION_TOGGLE_MIRROR);
        check_int("ready mirror accept index", action.mirror_index, 3);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            3,
            0,
            0,
            THERON_STARTUP_INPUT_BACK,
            &action);
        check_int("ready back rc", result, THERON_STARTUP_OK);
        check_int("ready back action",
                  action.kind,
                  THERON_STARTUP_ACTION_SHOW_STAGE_SELECT);
        check_contains("action label",
                       theron_v1_startup_action_name(
                           THERON_STARTUP_ACTION_ENTER_FORCEFIELD),
                       "forcefield");
        {
            Theron_StartupActionPlan plan;
            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_RETURN_TO_LAUNCHER;
            result = theron_v1_startup_plan_for_action(&action, &plan)
                         ? THERON_STARTUP_OK
                         : THERON_STARTUP_ERR_BAD_STAGE;
            check_int("plan return rc", result, THERON_STARTUP_OK);
            check_int("plan return kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_RETURN_TO_LAUNCHER);
            check_str("plan return scope", plan.status_scope, "RETURN");
            check_str("plan return status", plan.status, "BACK TO LAUNCHER");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_SHOW_STAGE_SELECT;
            action.selected_dungeon = THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
            action.cursor = 1;
            action.continue_focus = 0;
            check_int("plan stage-select rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan stage-select kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_SHOW_STAGE_SELECT);
            check_int("plan stage-select dungeon",
                      plan.selected_dungeon,
                      THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
            check_str("plan stage-select status", plan.status, "STAGE SELECT");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR;
            action.selected_dungeon = THERON_DUNGEON_4_TOMB_OF_WOE;
            action.continue_focus = 1;
            check_int("plan stage-cursor rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan stage-cursor kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_MOVE_STAGE_CURSOR);
            check_int("plan stage-cursor continue",
                      plan.continue_focus,
                      1);
            check_str("plan stage-cursor status", plan.status, "STAGE CURSOR");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_CONTINUE_SAVE;
            check_int("plan continue rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan continue kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_CONTINUE_SAVE);
            check_str("plan continue status", plan.status, "CONTINUE LOADED");
            check_str("plan continue failure",
                      plan.failure_status,
                      "CONTINUE FAILED");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_CHOOSE_STAGE;
            action.selected_dungeon = THERON_DUNGEON_5_VAULT_OF_SECRETS;
            check_int("plan choose-stage rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan choose-stage kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_CHOOSE_STAGE);
            check_str("plan choose-stage status", plan.status, "SOUL ROOM");
            check_str("plan choose-stage failure",
                      plan.failure_status,
                      "STAGE LOCKED");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_MOVE_SOUL_CURSOR;
            action.cursor = 4;
            check_int("plan soul-cursor rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan soul-cursor kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_MOVE_SOUL_CURSOR);
            check_int("plan soul-cursor cursor", plan.cursor, 4);
            check_str("plan soul-cursor status",
                      plan.status,
                      "SOUL ROOM CURSOR");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_TOGGLE_MIRROR;
            action.mirror_index = 6;
            check_int("plan mirror-toggle rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan mirror-toggle kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_TOGGLE_MIRROR);
            check_int("plan mirror-toggle index", plan.mirror_index, 6);
            check_str("plan mirror-toggle status",
                      plan.status,
                      "HERO RESURRECTED");
            check_str("plan mirror-toggle alternate",
                      plan.alternate_status,
                      "HERO RELEASED");

            theron_v1_startup_action_init(&action);
            action.kind = THERON_STARTUP_ACTION_ENTER_FORCEFIELD;
            check_int("plan forcefield rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan forcefield kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_ENTER_FORCEFIELD);
            check_str("plan forcefield scope", plan.status_scope, "BOOT");
            check_str("plan forcefield status", plan.status, "THERON READY");

            theron_v1_startup_action_init(&action);
            check_int("plan ignore rc",
                      theron_v1_startup_plan_for_action(&action, &plan),
                      1);
            check_int("plan ignore kind",
                      plan.kind,
                      THERON_STARTUP_PLAN_IGNORE);
        }

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_TITLE;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_TITLE,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            0,
            &hit,
            &action);
        check_int("title hit rc", result, THERON_STARTUP_OK);
        check_int("title hit shows stage select",
                  action.kind,
                  THERON_STARTUP_ACTION_SHOW_STAGE_SELECT);

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_PANEL;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            1,
            &hit,
            &action);
        check_int("panel hit rc", result, THERON_STARTUP_OK);
        check_int("panel hit consumes without action",
                  action.kind,
                  THERON_STARTUP_ACTION_NONE);

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_CONTINUE;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            1,
            &hit,
            &action);
        check_int("continue hit rc", result, THERON_STARTUP_OK);
        check_int("continue hit action",
                  action.kind,
                  THERON_STARTUP_ACTION_CONTINUE_SAVE);
        check_int("continue hit focus", action.continue_focus, 1);

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_STAGE;
        hit.selected_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            1,
            1,
            &hit,
            &action);
        check_int("stage hit rc", result, THERON_STARTUP_OK);
        check_int("stage hit action",
                  action.kind,
                  THERON_STARTUP_ACTION_CHOOSE_STAGE);
        check_int("stage hit selected dungeon",
                  action.selected_dungeon,
                  THERON_DUNGEON_3_ABYSS_OF_FLAMES);

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_STAGE;
        hit.selected_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
        result = theron_v1_startup_handle_hit_with_progression(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            &progression,
            0,
            0,
            0,
            &hit,
            &action);
        check_int("locked stage hit rc", result, THERON_STARTUP_OK);
        check_int("locked stage hit consumed",
                  action.kind,
                  THERON_STARTUP_ACTION_NONE);
        check_int("locked stage hit keeps target for status",
                  action.selected_dungeon,
                  THERON_DUNGEON_3_ABYSS_OF_FLAMES);
        {
            Theron_DungeonProgression progressed_hit;
            theron_v1_dungeon_progression_init(&progressed_hit);
            (void)theron_v1_dungeon_advance(&progressed_hit);
            result = theron_v1_startup_handle_hit_with_progression(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                THERON_DUNGEON_1_HALL_OF_RECORDS,
                &progressed_hit,
                0,
                0,
                0,
                &hit,
                &action);
            check_int("unlocked stage hit rc", result, THERON_STARTUP_OK);
            check_int("unlocked stage hit chooses",
                      action.kind,
                      THERON_STARTUP_ACTION_CHOOSE_STAGE);
            check_int("unlocked stage hit selected dungeon",
                      action.selected_dungeon,
                      THERON_DUNGEON_3_ABYSS_OF_FLAMES);
        }

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_MIRROR;
        hit.mirror_index = 4;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_SOUL_ROOM,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            0,
            &hit,
            &action);
        check_int("mirror hit rc", result, THERON_STARTUP_OK);
        check_int("mirror hit action",
                  action.kind,
                  THERON_STARTUP_ACTION_TOGGLE_MIRROR);
        check_int("mirror hit index", action.mirror_index, 4);
        check_int("mirror hit cursor", action.cursor, 4);

        theron_v1_startup_hit_init(&hit);
        hit.kind = THERON_STARTUP_HIT_FORCEFIELD;
        result = theron_v1_startup_handle_hit(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            2,
            0,
            0,
            &hit,
            &action);
        check_int("forcefield hit rc", result, THERON_STARTUP_OK);
        check_int("forcefield hit action",
                  action.kind,
                  THERON_STARTUP_ACTION_ENTER_FORCEFIELD);
        check_int("forcefield hit cursor",
                  action.cursor,
                  THERON_STARTUP_HERO_MIRROR_COUNT);

        {
            Theron_StartupLayoutState layout_state;
            Theron_StartupLayoutElement elements[16];
            Theron_StartupRenderPlan render_plan;
            char rows[16][THERON_STARTUP_RENDER_ROW_CAPACITY];
            int order[THERON_STARTUP_MAX_COMPANIONS] = {6, 2, -1};
            int layout_count;
            int row_count;

            theron_v1_startup_layout_state_init(&layout_state);
            layout_state.phase = THERON_STARTUP_PHASE_STAGE_SELECT;
            layout_state.selected_dungeon =
                THERON_DUNGEON_1_HALL_OF_RECORDS;
            layout_state.progression = &progression;
            layout_state.has_tqsv_continue = 1;
            layout_state.tqsv_slot = 2;
            snprintf(layout_state.chapter_label,
                     sizeof(layout_state.chapter_label),
                     "Chapter 1: Hall of Records");
            layout_count = theron_v1_startup_layout_build(
                &layout_state,
                elements,
                (int)(sizeof(elements) / sizeof(elements[0])));
            check_int("layout stage count", layout_count, 10);
            check_int("layout title kind",
                      elements[0].kind,
                      THERON_STARTUP_LAYOUT_ELEMENT_TITLE);
            check_int("layout continue kind",
                      elements[2].kind,
                      THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE);
            check_int("layout continue slot", elements[2].save_slot, 2);
            check_int("layout stage1 kind",
                      elements[3].kind,
                      THERON_STARTUP_LAYOUT_ELEMENT_STAGE);
            check_int("layout stage1 enabled", elements[3].enabled, 1);
            check_int("layout stage2 locked", elements[4].enabled, 0);
            result = theron_v1_startup_layout_hit_at(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                elements,
                layout_count,
                42,
                66,
                &hit) ? THERON_STARTUP_OK : THERON_STARTUP_ERR_NULL;
            check_int("layout continue hit rc", result, THERON_STARTUP_OK);
            check_int("layout continue hit kind",
                      hit.kind,
                      THERON_STARTUP_HIT_CONTINUE);
            result = theron_v1_startup_layout_hit_at(
                THERON_STARTUP_PHASE_STAGE_SELECT,
                elements,
                layout_count,
                42,
                91,
                &hit) ? THERON_STARTUP_OK : THERON_STARTUP_ERR_NULL;
            check_int("layout locked stage hit rc", result, THERON_STARTUP_OK);
            check_int("layout locked stage hit panel",
                      hit.kind,
                      THERON_STARTUP_HIT_PANEL);
            row_count = theron_v1_startup_render_rows_build(
                &layout_state,
                elements,
                layout_count,
                rows,
                (int)(sizeof(rows) / sizeof(rows[0])));
            check_int("layout stage rows count", row_count, 12);
            check_contains("layout stage rows chapter",
                           rows[1],
                           "Chapter 1: Hall of Records");
            check_contains("layout stage rows continue",
                           rows[3],
                           "CONTINUE  TQSV SLOT 2");
            check_contains("layout stage rows cursor",
                           rows[4],
                           "> 1  Hall of Records");
            check_int("layout stage render plan rc",
                      theron_v1_startup_render_plan_build(
                          &layout_state,
                          elements,
                          layout_count,
                          &render_plan),
                      1);
            check_int("layout stage render plan phase",
                      render_plan.phase,
                      THERON_STARTUP_PHASE_STAGE_SELECT);
            check_int("layout stage render plan border x",
                      render_plan.border_x,
                      12);
            check_int("layout stage render plan border color",
                      render_plan.border_color,
                      11);
            check_int("layout stage render plan text count",
                      render_plan.text_count,
                      12);
            check_str("layout stage render plan title",
                      render_plan.text[0].text,
                      "THERON'S QUEST");
            check_int("layout stage render plan title style",
                      render_plan.text[0].style,
                      THERON_STARTUP_RENDER_TEXT_TITLE);
            check_str("layout stage render plan prompt",
                      render_plan.text[2].text,
                      "CHOOSE A STAGE");
            check_int("layout stage render plan continue y",
                      render_plan.text[3].y,
                      66);
            check_contains("layout stage render plan continue",
                           render_plan.text[3].text,
                           "CONTINUE  TQSV SLOT 2");
            check_int("layout stage render plan selected style",
                      render_plan.text[4].style,
                      THERON_STARTUP_RENDER_TEXT_ACTIVE);
            check_int("layout stage render plan locked style",
                      render_plan.text[5].style,
                      THERON_STARTUP_RENDER_TEXT_LOCKED);
            check_contains("layout stage render plan footer",
                           render_plan.text[11].text,
                           "ENTER CONTINUE/STAGE");

            theron_v1_startup_layout_state_init(&layout_state);
            layout_state.phase = THERON_STARTUP_PHASE_READY;
            layout_state.selected_dungeon =
                THERON_DUNGEON_1_HALL_OF_RECORDS;
            layout_state.soul_cursor = THERON_STARTUP_HERO_MIRROR_COUNT;
            snprintf(layout_state.chapter_label,
                     sizeof(layout_state.chapter_label),
                     "Chapter 1: Hall of Records");
            snprintf(layout_state.startup_text_prompt,
                     sizeof(layout_state.startup_text_prompt),
                     "GO AWAY AND RESURRECT THERON");
            layout_state.startup_roster_name_count = 8;
            layout_state.startup_roster_names[4] = "HAKAR";
            layout_state.startup_roster_names[5] = "TIRAN";
            layout_state.startup_roster_names[7] = "PENTAI";
            layout_state.startup_roster_titles[4] = "THE BRAVE";
            layout_state.startup_roster_titles[7] = "THE SURVIVOR";
            layout_state.selected_mirrors_mask = (1 << 6) | (1 << 2);
            layout_state.selected_mirror_order = order;
            layout_state.selected_mirror_order_count =
                THERON_STARTUP_MAX_COMPANIONS;
            layout_count = theron_v1_startup_layout_build(
                &layout_state,
                elements,
                (int)(sizeof(elements) / sizeof(elements[0])));
            check_int("layout ready count", layout_count, 10);
            check_int("layout mirror6 order",
                      elements[8].selected_order,
                      1);
            check_int("layout forcefield enabled",
                      elements[9].enabled,
                      1);
            check_int("layout forcefield cursor",
                      elements[9].cursor,
                      1);
            result = theron_v1_startup_layout_hit_at(
                THERON_STARTUP_PHASE_READY,
                elements,
                layout_count,
                48,
                144,
                &hit) ? THERON_STARTUP_OK : THERON_STARTUP_ERR_NULL;
            check_int("layout mirror hit rc", result, THERON_STARTUP_OK);
            check_int("layout mirror hit kind",
                      hit.kind,
                      THERON_STARTUP_HIT_MIRROR);
            check_int("layout mirror hit index", hit.mirror_index, 6);
            result = theron_v1_startup_layout_hit_at(
                THERON_STARTUP_PHASE_READY,
                elements,
                layout_count,
                48,
                162,
                &hit) ? THERON_STARTUP_OK : THERON_STARTUP_ERR_NULL;
            check_int("layout forcefield hit rc", result, THERON_STARTUP_OK);
            check_int("layout forcefield hit kind",
                      hit.kind,
                      THERON_STARTUP_HIT_FORCEFIELD);
            row_count = theron_v1_startup_render_rows_build(
                &layout_state,
                elements,
                layout_count,
                rows,
                (int)(sizeof(rows) / sizeof(rows[0])));
            check_int("layout ready rows count", row_count, 13);
            check_contains("layout ready rows prompt",
                           rows[3],
                           "GO AWAY AND RESURRECT THERON");
            check_contains("layout ready rows decoded mirror",
                           rows[4],
                           "HAKAR");
            check_contains("layout ready rows selected order",
                           rows[10],
                           "RESURRECTED #1");
            check_int("layout ready render plan rc",
                      theron_v1_startup_render_plan_build(
                          &layout_state,
                          elements,
                          layout_count,
                          &render_plan),
                      1);
            check_int("layout ready render plan phase",
                      render_plan.phase,
                      THERON_STARTUP_PHASE_READY);
            check_int("layout ready render plan text count",
                      render_plan.text_count,
                      13);
            check_str("layout ready render plan title",
                      render_plan.text[0].text,
                      "THERON'S QUEST");
            check_str("layout ready render plan room",
                      render_plan.text[2].text,
                      "SOUL ROOM");
            check_contains("layout ready render plan prompt",
                           render_plan.text[3].text,
                           "GO AWAY AND RESURRECT THERON");
            check_contains("layout ready render plan decoded mirror",
                           render_plan.text[4].text,
                           "HAKAR");
            check_int("layout ready render plan selected style",
                      render_plan.text[10].style,
                      THERON_STARTUP_RENDER_TEXT_PICKED);
            check_int("layout ready render plan forcefield style",
                      render_plan.text[11].style,
                      THERON_STARTUP_RENDER_TEXT_ACTIVE);
            check_contains("layout ready render plan footer",
                           render_plan.text[12].text,
                           "ACTION ENTERS");
        }
    }

    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
    check_int("locked stage rejected", result, THERON_STARTUP_ERR_STAGE_LOCKED);

    theron_v1_dungeon_advance(&progression);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_5_VAULT_OF_SECRETS);
    check_int("middle stage accepted after first completion", result, THERON_STARTUP_OK);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_7_TOWER_OF_EPILOGUE);
    check_int("final stage still locked after first completion",
              result,
              THERON_STARTUP_ERR_STAGE_LOCKED);

    theron_v1_dungeon_progression_init(&progression);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_1_HALL_OF_RECORDS);
    check_int("stage 1 accepted", result, THERON_STARTUP_OK);
    check_int("stage phase soul room", flow.phase, THERON_STARTUP_PHASE_SOUL_ROOM);
    check_int("stage theron present", flow.theron_present, 1);

    result = theron_v1_startup_select_mirror(&flow, 6);
    check_int("mirror 6 accepted first", result, THERON_STARTUP_OK);
    result = theron_v1_startup_select_mirror(&flow, 6);
    check_int("duplicate mirror rejected", result, THERON_STARTUP_ERR_DUPLICATE_MIRROR);
    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("mirror 0 accepted second", result, THERON_STARTUP_OK);
    result = theron_v1_startup_select_mirror(&flow, 2);
    check_int("mirror 2 accepted third", result, THERON_STARTUP_OK);
    result = theron_v1_startup_deselect_mirror(&flow, 0);
    check_int("mirror 0 deselected", result, THERON_STARTUP_OK);
    check_int("companion count after middle deselect", flow.companion_count, 2);
    check_int("first order after middle deselect", flow.selected_mirror_order[0], 6);
    check_int("second order after middle deselect", flow.selected_mirror_order[1], 2);
    check_int("third order cleared after middle deselect",
              flow.selected_mirror_order[2],
              0xff);
    result = theron_v1_startup_deselect_mirror(&flow, 0);
    check_int("unselected mirror deselect rejected",
              result,
              THERON_STARTUP_ERR_MIRROR_NOT_SELECTED);
    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("mirror 0 reaccepted third", result, THERON_STARTUP_OK);
    check_int("three companions selected", flow.companion_count, 3);
    check_int("first selected mirror order", flow.selected_mirror_order[0], 6);
    check_int("second selected mirror order", flow.selected_mirror_order[1], 2);
    check_int("third selected mirror order", flow.selected_mirror_order[2], 0);
    check_int("ready phase", flow.phase, THERON_STARTUP_PHASE_READY);

    result = theron_v1_startup_select_mirror(&flow, 4);
    check_int("fourth companion rejected", result, THERON_STARTUP_ERR_PARTY_FULL);

    memset(&party, 0, sizeof(party));
    result = theron_v1_startup_enter_forcefield(&flow, &party);
    check_int("forcefield accepted", result, THERON_STARTUP_OK);
    check_int("forcefield flag", flow.forcefield_entered, 1);
    check_int("in dungeon phase", flow.phase, THERON_STARTUP_PHASE_IN_DUNGEON);
    check_int("party count theron plus three", party.champion_count, 4);
    check_int("leader slot is Theron", party.active_slot, THERON_CHAMPION_SLOT_THERON);
    check_contains("slot 0 name", party.champions[0].name, "Theron");
    check_contains("slot 1 mirror name", party.champions[1].name, "Pental");
    check_contains("slot 2 mirror name", party.champions[2].name, "Tiran");
    check_contains("slot 3 mirror name", party.champions[3].name, "Hakar");
    check_int("slot 1 portrait", party.champions[1].portrait_index, 7);
    check_int("slot 2 portrait", party.champions[2].portrait_index, 3);
    check_int("slot 3 portrait", party.champions[3].portrait_index, 1);
    check_int("slot 1 class", party.champions[1].primary_class, THERON_CLASS_FIGHTER);
    check_int("slot 2 class", party.champions[2].primary_class, THERON_CLASS_FIGHTER);
    check_int("slot 3 class", party.champions[3].primary_class, THERON_CLASS_FIGHTER);

    {
        const char *roster_names[8] = {
            "THERON-JP",
            "MARA-JP",
            "LINOS-JP",
            "HEXA-JP",
            "HAKAR-JP",
            "TIRAN-JP",
            "DOTAN-JP",
            "PENTAI-JP"
        };
        theron_v1_startup_flow_init(&flow);
        result = theron_v1_startup_choose_stage(
            &flow,
            &progression,
            THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("roster forcefield choose rc", result, THERON_STARTUP_OK);
        check_int("roster forcefield mirror 6 rc",
                  theron_v1_startup_select_mirror(&flow, 6),
                  THERON_STARTUP_OK);
        check_int("roster forcefield mirror 2 rc",
                  theron_v1_startup_select_mirror(&flow, 2),
                  THERON_STARTUP_OK);
        memset(&party, 0, sizeof(party));
        snprintf(party.champions[THERON_CHAMPION_SLOT_THERON].name,
                 sizeof(party.champions[THERON_CHAMPION_SLOT_THERON].name),
                 "THERON SAVED");
        result = theron_v1_startup_enter_forcefield_with_roster(
            &flow,
            &party,
            roster_names,
            8);
        check_int("roster forcefield rc", result, THERON_STARTUP_OK);
        check_contains("roster preserves Theron",
                       party.champions[0].name,
                       "THERON SAVED");
        check_contains("roster applies mirror 6 raw name",
                       party.champions[1].name,
                       "PENTAI-JP");
        check_contains("roster applies mirror 2 raw name",
                       party.champions[2].name,
                       "TIRAN-JP");
        theron_v1_world_init(&world);
        world.level_loaded[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0] = 1;
        world.object_count = 7;
        world.timer_count = 3;
        result = theron_v1_startup_enter_world_from_forcefield(
            &flow,
            &world);
        check_int("world entry after forcefield rc",
                  result,
                  THERON_STARTUP_OK);
        check_int("world entry current dungeon",
                  world.current_dungeon,
                  THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("world entry current level", world.current_level, 0);
        check_int("world entry progression level",
                  world.progression.current_level,
                  1);
        check_int("world entry clears level-loaded row",
                  world.level_loaded[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0],
                  0);
        check_int("world entry clears objects", world.object_count, 0);
        check_int("world entry clears timers", world.timer_count, 0);
        theron_v1_startup_flow_init(&flow);
        result = theron_v1_startup_enter_world_from_forcefield(
            &flow,
            &world);
        check_int("world entry before forcefield rejected",
                  result,
                  THERON_STARTUP_ERR_NOT_READY);
        theron_v1_startup_flow_init(&flow);
        result = theron_v1_startup_choose_stage(
            &flow,
            &progression,
            THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("runtime entry choose rc", result, THERON_STARTUP_OK);
        memset(&party, 0, sizeof(party));
        result = theron_v1_startup_enter_forcefield(&flow, &party);
        check_int("runtime entry forcefield rc", result, THERON_STARTUP_OK);
        theron_v1_world_init(&world);
        world.party = party;
        {
            int called = 0;
            char runtime_receipt[192];
            runtime_receipt[0] = '\0';
            result = theron_v1_startup_enter_runtime_from_forcefield(
                &flow,
                &world,
                fake_runtime_level_load,
                &called,
                runtime_receipt,
                sizeof(runtime_receipt));
            check_int("runtime entry rc", result, THERON_STARTUP_OK);
            check_int("runtime entry callback called", called, 1);
            check_int("runtime entry level loaded",
                      world.level_loaded[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0],
                      1);
            check_int("runtime entry party x", world.party.leader_x, 5);
            check_contains("runtime entry receipt",
                           runtime_receipt,
                           "fake level stage=1");
        }
        theron_v1_startup_flow_init(&flow);
        result = theron_v1_startup_choose_stage(
            &flow,
            &progression,
            THERON_DUNGEON_1_HALL_OF_RECORDS);
        check_int("runtime fail choose rc", result, THERON_STARTUP_OK);
        memset(&party, 0, sizeof(party));
        result = theron_v1_startup_enter_forcefield(&flow, &party);
        check_int("runtime fail forcefield rc", result, THERON_STARTUP_OK);
        theron_v1_world_init(&world);
        world.party = party;
        {
            int called = 0;
            char runtime_receipt[192];
            runtime_receipt[0] = '\0';
            result = theron_v1_startup_enter_runtime_from_forcefield(
                &flow,
                &world,
                fake_runtime_level_load_fail,
                &called,
                runtime_receipt,
                sizeof(runtime_receipt));
            check_int("runtime entry load failure rc",
                      result,
                      THERON_STARTUP_ERR_LEVEL_LOAD);
            check_int("runtime entry failing callback called", called, 1);
            check_contains("runtime entry load failure receipt",
                           runtime_receipt,
                           "level load failed");
        }
    }

    check_contains("mirror 0 meta", theron_v1_startup_mirror_meta(0)->name, "Hakar");
    check_contains("mirror 1 meta", theron_v1_startup_mirror_meta(1)->name, "Mara");
    check_contains("mirror 3 meta", theron_v1_startup_mirror_meta(3)->name, "Linos");
    check_contains("mirror 6 meta", theron_v1_startup_mirror_meta(6)->name, "Pental");
    check_int("mirror 0 roster index", theron_v1_startup_roster_index_for_mirror(0), 4);
    check_int("mirror 1 roster index", theron_v1_startup_roster_index_for_mirror(1), 1);
    check_int("mirror 2 roster index", theron_v1_startup_roster_index_for_mirror(2), 5);
    check_int("mirror 3 roster index", theron_v1_startup_roster_index_for_mirror(3), 2);
    check_int("mirror 4 roster index", theron_v1_startup_roster_index_for_mirror(4), 6);
    check_int("mirror 5 roster index", theron_v1_startup_roster_index_for_mirror(5), 3);
    check_int("mirror 6 roster index", theron_v1_startup_roster_index_for_mirror(6), 7);
    check_int("bad mirror roster index", theron_v1_startup_roster_index_for_mirror(99), -1);
    check_int("roster index 4 mirror", theron_v1_startup_mirror_index_for_roster(4), 0);
    check_int("roster index 1 mirror", theron_v1_startup_mirror_index_for_roster(1), 1);
    check_int("roster index 5 mirror", theron_v1_startup_mirror_index_for_roster(5), 2);
    check_int("roster index 2 mirror", theron_v1_startup_mirror_index_for_roster(2), 3);
    check_int("roster index 6 mirror", theron_v1_startup_mirror_index_for_roster(6), 4);
    check_int("roster index 3 mirror", theron_v1_startup_mirror_index_for_roster(3), 5);
    check_int("roster index 7 mirror", theron_v1_startup_mirror_index_for_roster(7), 6);
    check_int("Theron roster index has no mirror", theron_v1_startup_mirror_index_for_roster(0), -1);
    check_contains("class label", theron_v1_startup_class_name(THERON_CLASS_PRIEST), "PRIEST");

    theron_v1_startup_flow_init(&flow);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_1_HALL_OF_RECORDS);
    check_int("stage accepted for Theron-only run", result, THERON_STARTUP_OK);
    memset(&party, 0, sizeof(party));
    result = theron_v1_startup_enter_forcefield(&flow, &party);
    check_int("Theron-only forcefield accepted", result, THERON_STARTUP_OK);
    check_int("Theron-only party count", party.champion_count, 1);

    check_contains("title phase name",
                   theron_v1_startup_phase_name(THERON_STARTUP_PHASE_TITLE),
                   "title");
    check_contains("phase name",
                   theron_v1_startup_phase_name(THERON_STARTUP_PHASE_SOUL_ROOM),
                   "soul");
    check_contains("result name", theron_v1_startup_result_name(THERON_STARTUP_ERR_PARTY_FULL), "party");
    check_contains("deselect result name",
                   theron_v1_startup_result_name(THERON_STARTUP_ERR_MIRROR_NOT_SELECTED),
                   "mirror");
    check_contains("source evidence", theron_v1_startup_flow_source_evidence(), "dmweb");
    check_int("receipt title rc",
              theron_v1_startup_receipt_phase(
                  THERON_STARTUP_PHASE_TITLE,
                  phase_label,
                  sizeof(phase_label),
                  &startup_active),
              1);
    check_contains("receipt title phase", phase_label, "theron-startup-0");
    check_int("receipt title active", startup_active, 1);
    check_int("receipt stage rc",
              theron_v1_startup_receipt_phase(
                  THERON_STARTUP_PHASE_STAGE_SELECT,
                  phase_label,
                  sizeof(phase_label),
                  &startup_active),
              1);
    check_contains("receipt stage phase", phase_label, "theron-startup-1");
    check_int("receipt stage active", startup_active, 1);
    check_int("receipt soul rc",
              theron_v1_startup_receipt_phase(
                  THERON_STARTUP_PHASE_SOUL_ROOM,
                  phase_label,
                  sizeof(phase_label),
                  &startup_active),
              1);
    check_contains("receipt soul phase", phase_label, "theron-startup-2");
    check_int("receipt soul active", startup_active, 1);
    check_int("receipt ready rc",
              theron_v1_startup_receipt_phase(
                  THERON_STARTUP_PHASE_READY,
                  phase_label,
                  sizeof(phase_label),
                  &startup_active),
              1);
    check_contains("receipt ready phase", phase_label, "theron-startup-3");
    check_int("receipt ready active", startup_active, 1);
    check_int("receipt runtime rc",
              theron_v1_startup_receipt_phase(
                  THERON_STARTUP_PHASE_IN_DUNGEON,
                  phase_label,
                  sizeof(phase_label),
                  &startup_active),
              1);
    check_contains("receipt runtime phase", phase_label, "theron-runtime");
    check_int("receipt runtime active", startup_active, 0);

    printf("# total=%d passed=%d failed=%d\n", g_pass + g_fail, g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
