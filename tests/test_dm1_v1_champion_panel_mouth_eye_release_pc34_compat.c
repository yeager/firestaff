#include "dm1_v1_champion_panel_mouth_eye_release_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_str_eq(const char *id, const char *got, const char *want,
                          const char *anchor)
{
    ++g_assertions;
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=\"%s\" want=\"%s\" at %s\n",
               id, got ? got : "(null)", want ? want : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s == \"%s\" (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void expect_op(
    const char *prefix,
    const DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat *result,
    int index,
    DM1_V1_ChampionPanelMouthEyeReleaseOpPc34Compat op,
    const char *anchor)
{
    char id[96];

    snprintf(id, sizeof(id), "%s.op%d", prefix, index);
    expect_int(id, result->operations[index], op, anchor);
}

static void test_evidence(void)
{
    const DM1_V1_ChampionPanelMouthEyeReleaseEvidencePc34Compat *evidence =
        DM1_V1_ChampionPanelMouthEyeRelease_EvidencePc34Compat();
    const char *source =
        DM1_V1_ChampionPanelMouthEyeRelease_SourceEvidencePc34Compat();

    expect_bool("evidence.contract_only", evidence->contract_only, true,
                "PANEL.C F0349/F0350/F0352/F0353 contract-only slice");
    expect_str_eq("evidence.mouth_press", evidence->mouth_press_anchor,
                  "PANEL.C F0349_INVENTORY_ProcessCommand70_ClickOnMouth:1788-1818",
                  "PANEL.C F0349:1788-1818");
    expect_str_eq("evidence.mouth_release", evidence->mouth_release_anchor,
                  "PANEL.C F0350_INVENTORY_DrawStopPressingMouth:1953-1963",
                  "PANEL.C F0350:1953-1963");
    expect_str_eq("evidence.eye_press", evidence->eye_press_anchor,
                  "PANEL.C F0352_INVENTORY_ProcessCommand71_ClickOnEye:2111-2160",
                  "PANEL.C F0352:2111-2160");
    expect_str_eq("evidence.eye_release", evidence->eye_release_anchor,
                  "PANEL.C F0353_INVENTORY_DrawStopPressingEye:2162-2193",
                  "PANEL.C F0353:2162-2193");
    expect_contains("evidence.slotbox", evidence->slotbox_anchor,
                    "CHAMPION.C F0302", "CHAMPION.C F0302:677-711");
    expect_contains("evidence.chest", evidence->chest_anchor,
                    "CHEST.C F0333", "CHEST.C F0333/F0334");
    expect_contains("evidence.command_queue", evidence->command_queue_anchor,
                    "COMMAND.C F0359/F0380", "COMMAND.C F0359/F0380");
    expect_contains("evidence.icons", evidence->icon_anchor,
                    "C202/C203/C205", "DEFS.H:1953-1956");
    expect_contains("evidence.zones", evidence->zone_anchor,
                    "C545/C546", "DEFS.H:3914-3915");
    expect_contains("evidence.viewport", evidence->viewport_anchor,
                    "viewport redraw mode", "DEFS.H:2564");
    expect_contains("evidence.scope", evidence->scope_note,
                    "contract-only", "bounded synthetic model");
    expect_contains("evidence.no_asset", evidence->no_real_asset_claim,
                    "without claiming real-asset parity",
                    "contract-only no original DOS parity claim");
    expect_contains("source.contract", source, "contract_only=1",
                    "source evidence string");
    expect_contains("source.f0349", source, "F0349:1788-1818",
                    "PANEL.C F0349 source evidence");
    expect_contains("source.f0350", source, "F0350:1959-1962",
                    "PANEL.C F0350 source evidence");
    expect_contains("source.f0352", source, "F0352:2123-2159",
                    "PANEL.C F0352 source evidence");
    expect_contains("source.f0353", source, "F0353:2174-2192",
                    "PANEL.C F0353 source evidence");
    expect_contains("source.f0302", source, "CHAMPION.C F0302:677-711",
                    "CHAMPION.C source evidence");
    expect_contains("source.f0333", source, "CHEST.C F0333:30-38",
                    "CHEST.C source evidence");
    expect_contains("source.command_queue", source,
                    "COMMAND.C F0359/F0380:1982-1990",
                    "COMMAND.C source evidence");
    expect_contains("source.icons", source, "C202/C203/C205",
                    "DEFS.H icon ordinals");
    expect_contains("source.zones", source, "C545/C546",
                    "DEFS.H mouth/eye zones");
    expect_contains("source.viewport", source, "C0_VIEWPORT_NOT_DUNGEON_VIEW",
                    "DEFS.H viewport mode");
    expect_contains("source.no_asset", source, "without claiming real-asset parity",
                    "contract-only marker");
}

static void test_mouth_press_and_release(void)
{
    DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat input;
    DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat result;

    DM1_V1_ChampionPanelMouthEyeRelease_DefaultInputPc34Compat(&input);
    input.action = DM1_V1_CPMER_ACTION_MOUTH_PRESS_PC34;
    input.leader_empty_handed = true;
    input.left_button_down = true;
    input.panel_already_food_water_poisoned = false;

    expect_int("mouth_press.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0349:1788-1818");
    expect_bool("mouth_press.valid", result.valid, true,
                "PANEL.C F0349:1788-1818");
    expect_bool("mouth_press.contract", result.contract_only, true,
                "contract-only synthetic route");
    expect_bool("mouth_press.ignore", result.ignore_mouse_movements, true,
                "PANEL.C F0349:1792 G0597 true");
    expect_bool("mouth_press.pressing", result.pressing_mouth, true,
                "PANEL.C F0349:1793 G0333 true");
    expect_bool("mouth_press.pointer_hidden", result.pointer_hidden, true,
                "PANEL.C F0349:1811 hide pointer");
    expect_int("mouth_press.hide_request", result.hide_mouse_pointer_request_count,
               1, "PANEL.C F0349:1815 G0587=1");
    expect_int("mouth_press.panel_route", result.panel_route,
               DM1_V1_CPMER_PANEL_ROUTE_FOOD_WATER_POISON_PC34,
               "PANEL.C F0349:1816 calls F0345");
    expect_int("mouth_press.viewport_mode", result.viewport_mode,
               DM1_V1_CPMER_VIEWPORT_NOT_DUNGEON_VIEW_PC34,
               "PANEL.C F0349:1817 F0097(0)");
    expect_int("mouth_press.viewport_count", result.viewport_draw_count, 1,
               "PANEL.C F0349:1817 one viewport redraw");
    expect_int("mouth_press.op_count", result.operation_count, 6,
               "PANEL.C F0349:1792-1817 operation order");
    expect_op("mouth_press", &result, 0, DM1_V1_CPMER_OP_SET_IGNORE_MOUSE_PC34,
              "PANEL.C F0349:1792");
    expect_op("mouth_press", &result, 1, DM1_V1_CPMER_OP_SET_PRESSING_MOUTH_PC34,
              "PANEL.C F0349:1793");
    expect_op("mouth_press", &result, 2, DM1_V1_CPMER_OP_HIDE_POINTER_PC34,
              "PANEL.C F0349:1811");
    expect_op("mouth_press", &result, 3, DM1_V1_CPMER_OP_SET_HIDE_REQUEST_PC34,
              "PANEL.C F0349:1815");
    expect_op("mouth_press", &result, 4,
              DM1_V1_CPMER_OP_DRAW_FOOD_WATER_POISON_PANEL_PC34,
              "PANEL.C F0349:1816");
    expect_op("mouth_press", &result, 5, DM1_V1_CPMER_OP_DRAW_VIEWPORT_PC34,
              "PANEL.C F0349:1817");

    input.panel_already_food_water_poisoned = true;
    expect_int("mouth_press_repeat.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0349:1788-1790 existing food panel returns");
    expect_bool("mouth_press_repeat.early", result.early_return, true,
                "PANEL.C F0349:1789-1790");
    expect_int("mouth_press_repeat.op_count", result.operation_count, 0,
               "PANEL.C F0349:1789 returns before side effects");

    input.panel_already_food_water_poisoned = false;
    input.left_button_down = false;
    expect_int("mouth_press_button_up.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0349:1798-1809 button-up reset");
    expect_bool("mouth_press_button_up.early", result.early_return, true,
                "PANEL.C F0349:1798-1809");
    expect_bool("mouth_press_button_up.ignore", result.ignore_mouse_movements, false,
                "PANEL.C F0349:1803");
    expect_bool("mouth_press_button_up.pressing", result.pressing_mouth, false,
                "PANEL.C F0349:1805");
    expect_int("mouth_press_button_up.op_count", result.operation_count, 4,
               "PANEL.C F0349:1792-1805 reset order");
    expect_op("mouth_press_button_up", &result, 2,
              DM1_V1_CPMER_OP_RESET_IGNORE_MOUSE_PC34,
              "PANEL.C F0349:1803");
    expect_op("mouth_press_button_up", &result, 3,
              DM1_V1_CPMER_OP_RESET_PRESSING_MOUTH_PC34,
              "PANEL.C F0349:1805");

    input.action = DM1_V1_CPMER_ACTION_MOUTH_RELEASE_PC34;
    input.left_button_down = true;
    expect_int("mouth_release.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0350:1953-1963");
    expect_int("mouth_release.panel_route", result.panel_route,
               DM1_V1_CPMER_PANEL_ROUTE_INVENTORY_PANEL_PC34,
               "PANEL.C F0350:1959 F0347");
    expect_int("mouth_release.viewport_mode", result.viewport_mode,
               DM1_V1_CPMER_VIEWPORT_NOT_DUNGEON_VIEW_PC34,
               "PANEL.C F0350:1960 F0097(0)");
    expect_int("mouth_release.viewport_count", result.viewport_draw_count, 1,
               "PANEL.C F0350:1960 one viewport redraw");
    expect_int("mouth_release.hide_request", result.hide_mouse_pointer_request_count,
               1, "PANEL.C F0350:1961 G0587=1");
    expect_bool("mouth_release.pointer_shown", result.pointer_shown, true,
                "PANEL.C F0350:1962 M523");
    expect_int("mouth_release.op_count", result.operation_count, 4,
               "PANEL.C F0350:1959-1962 operation order");
    expect_op("mouth_release", &result, 0,
              DM1_V1_CPMER_OP_DRAW_INVENTORY_PANEL_PC34,
              "PANEL.C F0350:1959");
    expect_op("mouth_release", &result, 1, DM1_V1_CPMER_OP_DRAW_VIEWPORT_PC34,
              "PANEL.C F0350:1960");
    expect_op("mouth_release", &result, 2,
              DM1_V1_CPMER_OP_SET_HIDE_REQUEST_PC34,
              "PANEL.C F0350:1961");
    expect_op("mouth_release", &result, 3, DM1_V1_CPMER_OP_SHOW_POINTER_PC34,
              "PANEL.C F0350:1962");
}

static void test_eye_press_routes(void)
{
    DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat input;
    DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat result;

    DM1_V1_ChampionPanelMouthEyeRelease_DefaultInputPc34Compat(&input);
    input.action = DM1_V1_CPMER_ACTION_EYE_PRESS_PC34;
    input.leader_empty_handed = true;
    input.left_button_down = true;

    expect_int("eye_press_empty.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0352:2123-2159 empty hand");
    expect_bool("eye_press_empty.ignore", result.ignore_mouse_movements, true,
                "PANEL.C F0352:2123 G0597 true");
    expect_bool("eye_press_empty.pressing", result.pressing_eye, true,
                "PANEL.C F0352:2124 G0331 true");
    expect_bool("eye_press_empty.pointer_hidden", result.pointer_hidden, true,
                "PANEL.C F0352:2142 hide pointer");
    expect_int("eye_press_empty.delay", result.delay_ticks,
               DM1_V1_CPMER_DELAY_TICKS_PC34,
               "PANEL.C F0352:2146 delay 8");
    expect_int("eye_press_empty.icon", result.icon_index,
               DM1_V1_CPMER_ICON_EYE_LOOKING_PC34,
               "PANEL.C F0352:2151 C203");
    expect_int("eye_press_empty.zone", result.icon_zone,
               DM1_V1_CPMER_ZONE_EYE_PC34,
               "PANEL.C F0352:2151 C546");
    expect_int("eye_press_empty.panel_route", result.panel_route,
               DM1_V1_CPMER_PANEL_ROUTE_SKILLS_STATISTICS_PC34,
               "PANEL.C F0352:2153-2154 F0351");
    expect_int("eye_press_empty.viewport_mode", result.viewport_mode,
               DM1_V1_CPMER_VIEWPORT_NOT_DUNGEON_VIEW_PC34,
               "PANEL.C F0352:2159 F0097(0)");
    expect_int("eye_press_empty.op_count", result.operation_count, 8,
               "PANEL.C F0352:2123-2159 operation order");
    expect_op("eye_press_empty", &result, 0,
              DM1_V1_CPMER_OP_SET_IGNORE_MOUSE_PC34,
              "PANEL.C F0352:2123");
    expect_op("eye_press_empty", &result, 1,
              DM1_V1_CPMER_OP_SET_PRESSING_EYE_PC34,
              "PANEL.C F0352:2124");
    expect_op("eye_press_empty", &result, 2,
              DM1_V1_CPMER_OP_DISCARD_INPUT_PC34,
              "PANEL.C F0352:2141");
    expect_op("eye_press_empty", &result, 3,
              DM1_V1_CPMER_OP_HIDE_POINTER_PC34,
              "PANEL.C F0352:2142");
    expect_op("eye_press_empty", &result, 4,
              DM1_V1_CPMER_OP_DELAY_PC34,
              "PANEL.C F0352:2146");
    expect_op("eye_press_empty", &result, 5,
              DM1_V1_CPMER_OP_DRAW_EYE_LOOKING_PC34,
              "PANEL.C F0352:2151");
    expect_op("eye_press_empty", &result, 6,
              DM1_V1_CPMER_OP_DRAW_SKILLS_STATISTICS_PC34,
              "PANEL.C F0352:2153-2154");
    expect_op("eye_press_empty", &result, 7,
              DM1_V1_CPMER_OP_DRAW_VIEWPORT_PC34,
              "PANEL.C F0352:2159");

    input.leader_empty_handed = false;
    input.leader_hand_has_object = true;
    expect_int("eye_press_object.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0352:2155-2158 object hand");
    expect_int("eye_press_object.panel_route", result.panel_route,
               DM1_V1_CPMER_PANEL_ROUTE_OBJECT_DESCRIPTION_PC34,
               "PANEL.C F0352:2155-2158 F0342");
    expect_bool("eye_press_object.name_cleared", result.object_name_cleared, true,
                "PANEL.C F0352:2156 F0035");
    expect_bool("eye_press_object.inspect", result.object_panel_inspect, true,
                "PANEL.C F0352:2157 C1_TRUE inspect");
    expect_int("eye_press_object.op_count", result.operation_count, 9,
               "PANEL.C F0352:2155-2159 object operation order");
    expect_op("eye_press_object", &result, 6,
              DM1_V1_CPMER_OP_CLEAR_LEADER_HAND_NAME_PC34,
              "PANEL.C F0352:2156");
    expect_op("eye_press_object", &result, 7,
              DM1_V1_CPMER_OP_DRAW_OBJECT_PANEL_PC34,
              "PANEL.C F0352:2157");
    expect_op("eye_press_object", &result, 8,
              DM1_V1_CPMER_OP_DRAW_VIEWPORT_PC34,
              "PANEL.C F0352:2159");

    input.left_button_down = false;
    expect_int("eye_press_button_up.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0352:2129-2139 button-up reset");
    expect_bool("eye_press_button_up.early", result.early_return, true,
                "PANEL.C F0352:2129-2139");
    expect_bool("eye_press_button_up.ignore", result.ignore_mouse_movements, false,
                "PANEL.C F0352:2134");
    expect_bool("eye_press_button_up.pressing", result.pressing_eye, false,
                "PANEL.C F0352:2135");
    expect_int("eye_press_button_up.op_count", result.operation_count, 4,
               "PANEL.C F0352:2123-2135 reset order");
    expect_op("eye_press_button_up", &result, 2,
              DM1_V1_CPMER_OP_RESET_IGNORE_MOUSE_PC34,
              "PANEL.C F0352:2134");
    expect_op("eye_press_button_up", &result, 3,
              DM1_V1_CPMER_OP_RESET_PRESSING_EYE_PC34,
              "PANEL.C F0352:2135");
}

static void test_eye_release_routes_and_validation(void)
{
    DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat input;
    DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat result;

    DM1_V1_ChampionPanelMouthEyeRelease_DefaultInputPc34Compat(&input);
    input.action = DM1_V1_CPMER_ACTION_EYE_RELEASE_PC34;
    input.leader_empty_handed = true;
    input.leader_hand_has_object = false;
    input.inventory_champion_ordinal = 3;

    expect_int("eye_release_empty.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0353:2162-2193 empty hand");
    expect_int("eye_release_empty.icon", result.icon_index,
               DM1_V1_CPMER_ICON_EYE_NOT_LOOKING_PC34,
               "PANEL.C F0353:2178 C202");
    expect_int("eye_release_empty.zone", result.icon_zone,
               DM1_V1_CPMER_ZONE_EYE_PC34,
               "PANEL.C F0353:2178 C546");
    expect_int("eye_release_empty.panel_route", result.panel_route,
               DM1_V1_CPMER_PANEL_ROUTE_INVENTORY_PANEL_PC34,
               "PANEL.C F0353:2180 F0347");
    expect_int("eye_release_empty.viewport_mode", result.viewport_mode,
               DM1_V1_CPMER_VIEWPORT_NOT_DUNGEON_VIEW_PC34,
               "PANEL.C F0353:2181 F0097(0)");
    expect_int("eye_release_empty.clear_count",
               result.skill_recently_upgraded_clear_count,
               DM1_V1_CPMER_SKILL_GROUP_COUNT_PC34,
               "PANEL.C F0353:2186-2188 four skill flags");
    expect_bool("eye_release_empty.pointer_shown", result.pointer_shown, true,
                "PANEL.C F0353:2192 M523");
    expect_int("eye_release_empty.op_count", result.operation_count, 5,
               "PANEL.C F0353:2174-2192 operation order");
    expect_op("eye_release_empty", &result, 0,
              DM1_V1_CPMER_OP_DRAW_EYE_NOT_LOOKING_PC34,
              "PANEL.C F0353:2178");
    expect_op("eye_release_empty", &result, 1,
              DM1_V1_CPMER_OP_DRAW_INVENTORY_PANEL_PC34,
              "PANEL.C F0353:2180");
    expect_op("eye_release_empty", &result, 2,
              DM1_V1_CPMER_OP_DRAW_VIEWPORT_PC34,
              "PANEL.C F0353:2181");
    expect_op("eye_release_empty", &result, 3,
              DM1_V1_CPMER_OP_CLEAR_SKILL_RECENTLY_UPGRADED_PC34,
              "PANEL.C F0353:2186-2188");
    expect_op("eye_release_empty", &result, 4,
              DM1_V1_CPMER_OP_SHOW_POINTER_PC34,
              "PANEL.C F0353:2192");

    input.leader_hand_has_object = true;
    expect_int("eye_release_object.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0353:2186-2192 object hand");
    expect_bool("eye_release_object.name_drawn", result.object_name_drawn, true,
                "PANEL.C F0353:2190 F0034");
    expect_int("eye_release_object.clear_count",
               result.skill_recently_upgraded_clear_count, 0,
               "PANEL.C F0353:2186-2190 object hand skips clear");
    expect_op("eye_release_object", &result, 3,
              DM1_V1_CPMER_OP_DRAW_LEADER_HAND_NAME_PC34,
              "PANEL.C F0353:2190");

    expect_int("default_null.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(NULL, &result),
               1, "default input uses F0353 eye release");
    expect_int("default_null.action_icon", result.icon_index,
               DM1_V1_CPMER_ICON_EYE_NOT_LOOKING_PC34,
               "PANEL.C F0353:2178 default route");

    input.action = (DM1_V1_ChampionPanelMouthEyeReleaseActionPc34Compat)99;
    expect_int("invalid_action.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               0, "synthetic contract rejects unknown action");
    expect_bool("invalid_action.rejected", result.rejected_action, true,
                "synthetic action domain");

    DM1_V1_ChampionPanelMouthEyeRelease_DefaultInputPc34Compat(&input);
    input.inventory_champion_ordinal = 5;
    expect_int("invalid_ordinal.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               0, "DEFS.H M000 ordinals are none or 1..4");
    expect_bool("invalid_ordinal.rejected", result.rejected_inventory_ordinal, true,
                "synthetic inventory ordinal domain");

    expect_int("null_output.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, NULL),
               0, "synthetic null output guard");
}

static void test_pending_hand_overlap_no_consume_no_stale_panel(void)
{
    DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat input;
    DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat result;

    DM1_V1_ChampionPanelMouthEyeRelease_DefaultInputPc34Compat(&input);
    input.action = DM1_V1_CPMER_ACTION_EYE_PRESS_PC34;
    input.leader_empty_handed = false;
    input.leader_hand_has_object = true;
    input.leader_hand_thing_before = 0x4a11;
    input.pending_hand_queue_count = 2;
    input.pending_hand_thing_before = 0x4b22;

    expect_int("overlap_eye_live_hand.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0352:2155-2159 live G4055 object panel");
    expect_int("overlap_eye_live_hand.route", result.panel_route,
               DM1_V1_CPMER_PANEL_ROUTE_OBJECT_DESCRIPTION_PC34,
               "PANEL.C F0352:2155-2158 uses live leader hand");
    expect_int("overlap_eye_live_hand.leader_before",
               result.leader_hand_thing_before, 0x4a11,
               "CHAMPION.C F0302 live hand remains separate");
    expect_int("overlap_eye_live_hand.leader_after",
               result.leader_hand_thing_after, 0x4a11,
               "PANEL.C F0352 inspects without consuming");
    expect_bool("overlap_eye_live_hand.leader_consumed",
                result.leader_hand_consumed, false,
                "PANEL.C F0352 no F0298 leader-hand remove");
    expect_int("overlap_eye_live_hand.pending_count_before",
               result.pending_hand_queue_count_before, 2,
               "COMMAND.C F0380 pending queue snapshot");
    expect_int("overlap_eye_live_hand.pending_count_after",
               result.pending_hand_queue_count_after, 2,
               "COMMAND.C F0380 not drained by F0352");
    expect_int("overlap_eye_live_hand.pending_before",
               result.pending_hand_thing_before, 0x4b22,
               "COMMAND.C pending click is not G4055");
    expect_int("overlap_eye_live_hand.pending_after",
               result.pending_hand_thing_after, 0x4b22,
               "COMMAND.C pending click preserved");
    expect_bool("overlap_eye_live_hand.pending_consumed",
                result.pending_hand_consumed, false,
                "PANEL.C F0352 no pending-hand consume");
    expect_bool("overlap_eye_live_hand.pending_preserved",
                result.pending_queue_preserved, true,
                "COMMAND.C F0380 pending hand survives eye route");
    expect_bool("overlap_eye_live_hand.stale_panel",
                result.stale_panel_after, false,
                "object panel came from live G4055, not pending hand");

    DM1_V1_ChampionPanelMouthEyeRelease_DefaultInputPc34Compat(&input);
    input.action = DM1_V1_CPMER_ACTION_EYE_PRESS_PC34;
    input.leader_empty_handed = true;
    input.leader_hand_has_object = false;
    input.leader_hand_thing_before = -1;
    input.pending_hand_queue_count = 1;
    input.pending_hand_thing_before = 0x4c33;

    expect_int("overlap_eye_pending_only.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0352:2153-2154 empty live hand stats route");
    expect_int("overlap_eye_pending_only.route", result.panel_route,
               DM1_V1_CPMER_PANEL_ROUTE_SKILLS_STATISTICS_PC34,
               "PANEL.C F0352 must not inspect pending hand as G4055");
    expect_bool("overlap_eye_pending_only.pending_preserved",
                result.pending_queue_preserved, true,
                "COMMAND.C F0380 pending hand remains queued");
    expect_bool("overlap_eye_pending_only.stale_panel",
                result.stale_panel_after, false,
                "pending hand must not leave stale object panel");

    input.action = DM1_V1_CPMER_ACTION_MOUTH_PRESS_PC34;
    input.left_button_down = true;
    expect_int("overlap_mouth_pending_only.build",
               DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0349:1788-1818 empty live hand mouth route");
    expect_int("overlap_mouth_pending_only.route", result.panel_route,
               DM1_V1_CPMER_PANEL_ROUTE_FOOD_WATER_POISON_PC34,
               "PANEL.C F0349 uses G0415, not pending hand queue");
    expect_bool("overlap_mouth_pending_only.pending_consumed",
                result.pending_hand_consumed, false,
                "PANEL.C F0349 no pending-hand consume");
    expect_bool("overlap_mouth_pending_only.stale_panel",
                result.stale_panel_after, false,
                "mouth route cannot leave object-description panel stale");
}

int main(void)
{
    test_evidence();
    test_mouth_press_and_release();
    test_eye_press_routes();
    test_eye_release_routes_and_validation();
    test_pending_hand_overlap_no_consume_no_stale_panel();

    if (g_assertions < 85) {
        printf("FAIL assertion_count got=%d want>=85\n", g_assertions);
        return 1;
    }
    if (g_failures) {
        printf("FAILURES %d / %d assertions\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS test_dm1_v1_champion_panel_mouth_eye_release_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
