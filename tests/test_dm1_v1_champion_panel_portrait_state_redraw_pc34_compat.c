#include "dm1_v1_champion_panel_portrait_state_redraw_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

#define CHECK_INT(ID, GOT, WANT, ANCHOR) check_int((ID), (GOT), (WANT), (ANCHOR))
#define CHECK_BOOL(ID, GOT, WANT, ANCHOR) \
    check_int((ID), (GOT) ? 1 : 0, (WANT) ? 1 : 0, (ANCHOR))

static void check_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void check_contains(const char *id, const char *haystack,
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

static void test_champion_zero_state_order(void)
{
    CHECK_INT("champ0.ok",
              dm1_v1_champion_panel_state_redraw_order(
                  0, DM1_V1_CHAMPION_PANEL_STATE_OK_PC34),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C033_FRESH_BLIT_PC34,
              "CHAMDRAW.C F0292:898-935 and DEFS.H:2188-2195 C033");
    CHECK_INT("champ0.wounded",
              dm1_v1_champion_panel_state_redraw_order(
                  0, DM1_V1_CHAMPION_PANEL_STATE_WOUNDED_PC34),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
              "CHAMDRAW.C F0291:595-655 and F0292:898-935 C034");
    CHECK_INT("champ0.poisoned",
              dm1_v1_champion_panel_state_redraw_order(
                  0, DM1_V1_CHAMPION_PANEL_STATE_POISONED_PC34),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
              "CHAMDRAW.C F0292:908-914 poison selects wounded mouth border");
    CHECK_INT("champ0.hungry",
              dm1_v1_champion_panel_state_redraw_order(
                  0, DM1_V1_CHAMPION_PANEL_STATE_HUNGRY_PC34),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
              "CHAMDRAW.C F0292:908-914 food warning selects C034");
    CHECK_INT("champ0.asleep",
              dm1_v1_champion_panel_state_redraw_order(
                  0, DM1_V1_CHAMPION_PANEL_STATE_ASLEEP_PC34),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
              "CHAMDRAW.C F0292:771-839 status-box redraw before live details");
    CHECK_INT("champ0.confused",
              dm1_v1_champion_panel_state_redraw_order(
                  0, DM1_V1_CHAMPION_PANEL_STATE_CONFUSED_PC34),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
              "CHAMDRAW.C F0292:771-839 status-box redraw before live details");
    CHECK_INT("champ0.paralyzed",
              dm1_v1_champion_panel_state_redraw_order(
                  0, DM1_V1_CHAMPION_PANEL_STATE_PARALYZED_PC34),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
              "CHAMDRAW.C F0292:771-839 status-box redraw before live details");
    CHECK_INT("champ0.dead",
              dm1_v1_champion_panel_state_redraw_order(
                  0, DM1_V1_CHAMPION_PANEL_STATE_DEAD_PC34),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C035_DEAD_POLYGON_PC34,
              "CHAMDRAW.C F0292:816-839 dead branch; DEFS.H:2188-2195 C035");
}

static void test_all_champions_same_order(void)
{
    int champion;
    int state;
    const int expected[DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATE_COUNT_PC34] = {
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C033_FRESH_BLIT_PC34,
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C035_DEAD_POLYGON_PC34
    };

    for (champion = 1;
         champion < DM1_V1_CHAMPION_PANEL_STATE_REDRAW_CHAMPION_COUNT_PC34;
         ++champion) {
        for (state = 0; state < DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATE_COUNT_PC34;
             ++state) {
            char id[64];

            snprintf(id, sizeof(id), "champ%d.state%d.same_order", champion, state);
            CHECK_INT(id,
                      dm1_v1_champion_panel_state_redraw_order(
                          champion,
                          (dm1_v1_champion_panel_state_redraw_state_pc34_compat_t)
                              state),
                      expected[state],
                      "CHAMDRAW.C F0293:1134-1138 calls F0292 by champion index");
        }
    }
}

static void test_terminal_order_and_null_safety(void)
{
    const dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t *entry =
        (const dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t *)1;

    CHECK_INT("ok.first",
              dm1_v1_champion_panel_state_redraw_order(
                  0, DM1_V1_CHAMPION_PANEL_STATE_OK_PC34),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C033_FRESH_BLIT_PC34,
              "CHAMDRAW.C F0292:898-935 C033 normal before C034/C035");
    CHECK_BOOL("dead.last.gt.wounded",
               DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C035_DEAD_POLYGON_PC34 >
                   DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34,
               true,
               "DEFS.H:2188-2195 C033/C034/C035 cascade");
    CHECK_INT("dead.last",
              dm1_v1_champion_panel_state_redraw_order(
                  3, DM1_V1_CHAMPION_PANEL_STATE_DEAD_PC34),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C035_DEAD_POLYGON_PC34,
              "CHAMDRAW.C F0292:816-839 terminal dead branch");
    CHECK_BOOL("lookup.null_out_ok",
               dm1_v1_champion_panel_state_redraw_entry(
                   0, DM1_V1_CHAMPION_PANEL_STATE_OK_PC34, NULL),
               true,
               "pure helper does not require caller storage");
    CHECK_BOOL("lookup.invalid_champion",
               dm1_v1_champion_panel_state_redraw_entry(
                   -1, DM1_V1_CHAMPION_PANEL_STATE_OK_PC34, &entry),
               false,
               "CHAMDRAW.C F0292:755 indexes bounded M516_CHAMPIONS");
    CHECK_BOOL("lookup.invalid_champion_clears",
               entry == NULL,
               true,
               "NULL-safety: rejected lookup clears output pointer");
    CHECK_INT("order.invalid_state",
              dm1_v1_champion_panel_state_redraw_order(
                  0,
                  (dm1_v1_champion_panel_state_redraw_state_pc34_compat_t)99),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_INVALID_PC34,
              "CHAMDRAW.C F0293:1134-1138 bounded synthetic state fixture");
}

static void test_table_cartesian_product(void)
{
    int seen[DM1_V1_CHAMPION_PANEL_STATE_REDRAW_CHAMPION_COUNT_PC34]
            [DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATE_COUNT_PC34] = {{0}};
    int i;
    int champion;
    int state;

    CHECK_INT("table.count",
              (int)(sizeof(dm1_v1_champion_panel_state_redraw_table) /
                    sizeof(dm1_v1_champion_panel_state_redraw_table[0])),
              DM1_V1_CHAMPION_PANEL_STATE_REDRAW_TABLE_COUNT_PC34,
              "CHAMDRAW.C F0293:1117-1143 4 champion dispatch fixture");

    for (i = 0; i < DM1_V1_CHAMPION_PANEL_STATE_REDRAW_TABLE_COUNT_PC34; ++i) {
        const dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t *entry =
            &dm1_v1_champion_panel_state_redraw_table[i];

        CHECK_BOOL("table.entry.champion_in_range",
                   entry->champion_index >= 0 &&
                       entry->champion_index <
                           DM1_V1_CHAMPION_PANEL_STATE_REDRAW_CHAMPION_COUNT_PC34,
                   true,
                   "CHAMDRAW.C F0293:1134 active champion index loop");
        CHECK_BOOL("table.entry.state_in_range",
                   entry->state >= DM1_V1_CHAMPION_PANEL_STATE_OK_PC34 &&
                       entry->state <= DM1_V1_CHAMPION_PANEL_STATE_DEAD_PC34,
                   true,
                   "synthetic 8-state source-lock fixture");
        ++seen[entry->champion_index][entry->state];
    }

    for (champion = 0;
         champion < DM1_V1_CHAMPION_PANEL_STATE_REDRAW_CHAMPION_COUNT_PC34;
         ++champion) {
        for (state = 0; state < DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATE_COUNT_PC34;
             ++state) {
            char id[64];

            snprintf(id, sizeof(id), "table.cartesian.%d.%d", champion, state);
            CHECK_INT(id, seen[champion][state], 1,
                      "CHAMDRAW.C F0293:1134-1138 champion x state coverage");
        }
    }
}

static void test_geometry(void)
{
    int champion;
    const dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t *champ0_ok;
    const dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t *champ1_ok;

    CHECK_BOOL("lookup.champ0_ok",
               dm1_v1_champion_panel_state_redraw_entry(
                   0, DM1_V1_CHAMPION_PANEL_STATE_OK_PC34, &champ0_ok),
               true,
               "CHAMDRAW.C F0293:1134 first champion");
    CHECK_BOOL("lookup.champ1_ok",
               dm1_v1_champion_panel_state_redraw_entry(
                   1, DM1_V1_CHAMPION_PANEL_STATE_OK_PC34, &champ1_ok),
               true,
               "CHAMDRAW.C F0293:1134 second champion");
    CHECK_BOOL("status.origins.disjoint",
               champ0_ok->status_box_left != champ1_ok->status_box_left ||
                   champ0_ok->status_box_top != champ1_ok->status_box_top,
               true,
               "CHAMDRAW.C F0292 status boxes stride by champion index");
    CHECK_INT("slot.champ0.ready_x", champ0_ok->ready_hand_x, 4,
              "DATA.C:264-272 champion 0 ready hand origin");
    CHECK_INT("slot.champ0.action_x", champ0_ok->action_hand_x, 24,
              "DATA.C:264-272 champion 0 action hand origin");
    CHECK_INT("slot.champ1.ready_x", champ1_ok->ready_hand_x, 73,
              "DATA.C:264-272 champion 1 ready hand origin");
    CHECK_INT("slot.champ1.action_x", champ1_ok->action_hand_x, 93,
              "DATA.C:264-272 champion 1 action hand origin");

    for (champion = 0;
         champion < DM1_V1_CHAMPION_PANEL_STATE_REDRAW_CHAMPION_COUNT_PC34;
         ++champion) {
        const dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t *entry;
        const int left =
            champion *
            DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_STRIDE_X_PC34;
        char id[64];

        CHECK_BOOL("geometry.lookup",
                   dm1_v1_champion_panel_state_redraw_entry(
                       champion, DM1_V1_CHAMPION_PANEL_STATE_OK_PC34, &entry),
                   true,
                   "CHAMDRAW.C F0293:1134-1138 champion-index lookup");
        snprintf(id, sizeof(id), "status.%d.zone", champion);
        CHECK_INT(id, entry->status_box_zone,
                  DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_ZONE_BASE_PC34 +
                      champion,
                  "DEFS.H:3783-3786 C151..C154 status-box zones");
        snprintf(id, sizeof(id), "status.%d.left", champion);
        CHECK_INT(id, entry->status_box_left, left,
                  "CHAMDRAW.C F0292 status-box x = championIndex*69");
        snprintf(id, sizeof(id), "status.%d.right", champion);
        CHECK_INT(id, entry->status_box_right, left + 66,
                  "CHAMDRAW.C F0292 status-box right = left+66");
        snprintf(id, sizeof(id), "status.%d.bottom", champion);
        CHECK_INT(id, entry->status_box_bottom, 28,
                  "CHAMDRAW.C F0292 status-box bottom = 28");
        snprintf(id, sizeof(id), "zone.%d.name", champion);
        CHECK_INT(id, entry->name_zone,
                  DM1_V1_CHAMPION_PANEL_STATE_REDRAW_NAME_ZONE_BASE_PC34 +
                      champion,
                  "DEFS.H:3787-3790 C159..C162 name zones");
        snprintf(id, sizeof(id), "zone.%d.text", champion);
        CHECK_INT(id, entry->text_zone,
                  DM1_V1_CHAMPION_PANEL_STATE_REDRAW_TEXT_ZONE_BASE_PC34 +
                      champion,
                  "DEFS.H:3791 C163 first champion name zone");
        snprintf(id, sizeof(id), "zone.%d.portrait", champion);
        CHECK_INT(id, entry->portrait_zone,
                  DM1_V1_CHAMPION_PANEL_STATE_REDRAW_PORTRAIT_ZONE_BASE_PC34 +
                      champion,
                  "DEFS.H:3793 C175 first champion status-box portrait zone");
        snprintf(id, sizeof(id), "zone.%d.bar", champion);
        CHECK_INT(id, entry->bar_graph_zone,
                  DM1_V1_CHAMPION_PANEL_STATE_REDRAW_BAR_ZONE_BASE_PC34 +
                      champion,
                  "DEFS.H:3795-3798 C187..C190 bar-graph zones");
        snprintf(id, sizeof(id), "zone.%d.icon", champion);
        CHECK_INT(id, entry->champion_icon_zone,
                  DM1_V1_CHAMPION_PANEL_STATE_REDRAW_ICON_ZONE_BASE_PC34 +
                      champion,
                  "DEFS.H:3779-3782 C113..C116 champion-icon zones");
    }
}

static void test_source_evidence(void)
{
    const char *evidence =
        dm1_v1_champion_panel_state_redraw_source_evidence();

    check_contains("evidence.f0297", evidence, "CHAMPION.C F0297:243-298",
                   "leader-hand redraw anchor");
    check_contains("evidence.f0291", evidence, "CHAMDRAW.C F0291:551-552",
                   "slot read anchor");
    check_contains("evidence.f0292", evidence, "CHAMDRAW.C F0292:771-839",
                   "state redraw anchor");
    check_contains("evidence.f0293", evidence, "CHAMDRAW.C F0293:1117-1143",
                   "all-state dispatch anchor");
    check_contains("evidence.f0296", evidence, "CHAMDRAW.C F0296:1249-1257",
                   "changed-object icon anchor");
    check_contains("evidence.zones", evidence, "DEFS.H:3783-3795",
                   "status-box zone anchors");
    check_contains("evidence.defs", evidence, "DEFS.H:779-781",
                   "slot constant anchor");
    check_contains("evidence.data", evidence, "DATA.C:264-272",
                   "slot box origin anchor");
}

int main(void)
{
    test_champion_zero_state_order();
    test_all_champions_same_order();
    test_terminal_order_and_null_safety();
    test_table_cartesian_product();
    test_geometry();
    test_source_evidence();

    if (g_failures) {
        printf("FAIL test_dm1_v1_champion_panel_portrait_state_redraw_pc34_compat "
               "failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }

    printf("PASS test_dm1_v1_champion_panel_portrait_state_redraw_pc34_compat "
           "failures=0 assertions=%d\n",
           g_assertions);
    return 0;
}
