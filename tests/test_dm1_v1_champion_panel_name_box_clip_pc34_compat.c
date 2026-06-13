/*
 * DM1 V1 champion-panel name-box clip gate tests.
 *
 * Contract-only, no-asset fixture. Pins the *clip geometry and
 * clip behavior* of the CHAMDRAW.C F0292 name-strip / inventory
 * viewport / dead-champion paths. The F0292 -> F0354 dispatch
 * predicate is pinned separately by
 * `dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat`;
 * the portrait-box geometry is pinned by
 * `dm1_v1_champion_panel_f0354_box_variants_pc34_compat`; the
 * portrait-state redraw matrix is pinned by
 * `dm1_v1_champion_panel_portrait_state_redraw_pc34_compat`
 * and
 * `dm1_v1_champion_panel_portrait_box_redraw_states_pc34_compat`;
 * the F0292 NAME_TITLE strip x-anchor edge (slot-3 left=207,
 * right=249, printX=208, width=43) is pinned inside
 * `test_dm1_v1_champion_panel_hud_pc34_compat`. The present
 * fixture does NOT cover any of those; it covers:
 *
 *   1. The 7-row-tall, 43-pixel name box at the top of each
 *      champion's status box (M770_BOX_TOP=0, M771_BOX_BOTTOM=6,
 *      M768_BOX_LEFT=L0868, M769_BOX_RIGHT=L0868+42).
 *   2. The 1-pixel left padding (L0868 + 1 print column) for
 *      the F0053_TEXT_PrintToLogicalScreen name print.
 *   3. The 6-pixel-wide PC 3.4 font width (= 7 chars * 6 = 42,
 *      so Name[8] exactly clips to the 43-pixel box).
 *   4. The leader / non-leader color cascade (C09_COLOR_GOLD
 *      for leader, C13_COLOR_LIGHTEST_GRAY for the other three
 *      champions; C01_COLOR_DARK_GRAY background).
 *   5. The dead-champion name strip x-anchor (L0868 + 1 on PC
 *      3.4) and the C13/C01 dead color cascade.
 *   6. The inventory viewport name/title branch: F0052 at
 *      (3, 7), L0869_i_ChampionTitleX = 6*strlen(name) + 3,
 *      +6 increment when Title[0] not in {',', ';', '-'}.
 *   7. The Name[8] 7-char byte cap matching the 43-pixel name
 *      box right edge.
 *
 * Source-lock anchors: ReDMCSB CHAMDRAW.C F0292:750/818-833/
 * 843-895/845/855-871, DEFS.H:2157 (C69_CHAMPION_STATUS_BOX_
 * SPACING), DEFS.H:623 / DEFS.H:660 (CHAMPION.Name[8]),
 * DEFS.H:2079/2087/2091 (C01/C09/C13 colors), DEFS.H:3787/3791
 * (C159/C163 zones).
 *
 * The fixture does not duplicate:
 *  - the F0292 -> F0354 dispatch gate
 *    (dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat),
 *  - the F0354 portrait-box geometry
 *    (dm1_v1_champion_panel_f0354_box_variants_pc34_compat),
 *  - the portrait-state redraw matrix
 *    (dm1_v1_champion_panel_portrait_state_redraw_pc34_compat +
 *     dm1_v1_champion_panel_portrait_box_redraw_states_pc34_compat),
 *  - the F0292 NAME_TITLE strip x-anchor edge on slot 3
 *    (test_dm1_v1_champion_panel_hud_pc34_compat lines 422-440),
 *  - the mouth/eye press release gate, the food/water status-box
 *    gate, the HUD recompute gate, the action-hand slot-priority
 *    gate, the action-cell slotbox gate, the status-hand slot-
 *    pixels gate, the hand-slot-priority gate, the held-hand
 *    probe, the held-hand icon direction probe, the partial-party
 *    probe, the leader-rotation pixel slice, the shield-border
 *    pixel slice, the recompute runtime, the status-box asset
 *    slice, the status-states runtime, the status-states partial
 *    party probe, the pixels runtime, the panel pressing
 *    mouth/eye statusbox, the inventory champion switch hand
 *    carry, the spell-area overlay gate,
 *  - any chest/inventory/mirror runtime regression.
 */

#include "firestaff/dm1/v1/champion_panel/name_box_clip_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

#define CHECK(ID, GOT, WANT, ANCHOR) check_int((ID), (int)(GOT), (int)(WANT), (ANCHOR))
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

/* ----- Pure-function tests ----- */

static void test_pure_compute_title_x_baseline(void)
{
    /* 7-char name, no punctuation passthrough => 6*7+3+6 = 51. */
    int x = dm1_v1_cpnbc_compute_inventory_title_x_pc34("ABCDEFG", 'Z');
    CHECK("title_x.7char_no_punct", x, 6 * 7 + 3 + 6,
          "CHAMDRAW.C F0292:856 L0869 = 6*7+3 and F0292:866 +6 for non-punct");
}

static void test_pure_compute_title_x_punct_passthrough(void)
{
    /* 7-char name, comma passthrough => 6*7+3+0 = 45. */
    int x = dm1_v1_cpnbc_compute_inventory_title_x_pc34("ABCDEFG", ',');
    CHECK("title_x.7char_comma", x, 6 * 7 + 3,
          "CHAMDRAW.C F0292:859-866 comma is in passthrough set, no +6");
    x = dm1_v1_cpnbc_compute_inventory_title_x_pc34("ABCDEFG", ';');
    CHECK("title_x.7char_semi", x, 6 * 7 + 3,
          "CHAMDRAW.C F0292:859-866 semicolon is in passthrough set");
    x = dm1_v1_cpnbc_compute_inventory_title_x_pc34("ABCDEFG", '-');
    CHECK("title_x.7char_dash", x, 6 * 7 + 3,
          "CHAMDRAW.C F0292:859-866 dash is in passthrough set");
}

static void test_pure_compute_title_x_empty_name(void)
{
    /* Empty name + no-punct => 6*0+3+6 = 9. */
    int x = dm1_v1_cpnbc_compute_inventory_title_x_pc34("", 'Z');
    CHECK("title_x.empty_no_punct", x, 9,
          "CHAMDRAW.C F0292:856 6*0+3+6 = 9 (no-punct still adds +6)");
    x = dm1_v1_cpnbc_compute_inventory_title_x_pc34("", ',');
    CHECK("title_x.empty_comma", x, 3,
          "CHAMDRAW.C F0292:856 6*0+3=3 with comma passthrough");
}

static void test_pure_compute_title_x_null_name(void)
{
    int x = dm1_v1_cpnbc_compute_inventory_title_x_pc34(NULL, 'Z');
    CHECK("title_x.null_name", x, 9,
          "null name is treated as 0-length, no-punct still adds +6");
    x = dm1_v1_cpnbc_compute_inventory_title_x_pc34(NULL, ',');
    CHECK("title_x.null_name_comma", x, 3,
          "null name + comma passthrough = 3");
}

static void test_pure_title_passthrough(void)
{
    CHECK_BOOL("punct.comma", dm1_v1_cpnbc_title_passthrough_punctuation_pc34(','),
               1, "CHAMDRAW.C F0292:859 comma is in passthrough set");
    CHECK_BOOL("punct.semi", dm1_v1_cpnbc_title_passthrough_punctuation_pc34(';'),
               1, "CHAMDRAW.C F0292:859 semicolon is in passthrough set");
    CHECK_BOOL("punct.dash", dm1_v1_cpnbc_title_passthrough_punctuation_pc34('-'),
               1, "CHAMDRAW.C F0292:859 dash is in passthrough set");
    CHECK_BOOL("punct.letter",
               dm1_v1_cpnbc_title_passthrough_punctuation_pc34('Z'),
               0, "CHAMDRAW.C F0292:859 Z is not in passthrough set");
    CHECK_BOOL("punct.zero",
               dm1_v1_cpnbc_title_passthrough_punctuation_pc34(0),
               0, "CHAMDRAW.C F0292:859 NUL is not in passthrough set");
    CHECK_BOOL("punct.dot",
               dm1_v1_cpnbc_title_passthrough_punctuation_pc34('.'),
               0, "CHAMDRAW.C F0292:859 '.' is not in passthrough set");
}

static void test_pure_name_field_text_pixels(void)
{
    CHECK("pixels.0char", dm1_v1_cpnbc_name_field_text_pixels_pc34(""), 0,
          "empty name is 0 pixels");
    CHECK("pixels.1char", dm1_v1_cpnbc_name_field_text_pixels_pc34("A"), 6,
          "1 char * 6 px = 6");
    CHECK("pixels.7char", dm1_v1_cpnbc_name_field_text_pixels_pc34("ABCDEFG"),
          42, "7 chars * 6 px = 42 (clip to 43-pixel name box)");
    CHECK("pixels.8char_clipped", dm1_v1_cpnbc_name_field_text_pixels_pc34("ABCDEFGH"),
          42, "Name[8] is clipped to 7 visible chars * 6 = 42");
    CHECK("pixels.null", dm1_v1_cpnbc_name_field_text_pixels_pc34(NULL), 0,
          "null name is 0 pixels");
}

/* ----- Status box live ----- */

static void test_status_box_live_default(void)
{
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpnbc_build_status_box_live_pc34(0, 0, "ABCDEFG", &box);

    CHECK("live0.reached_f0292_name_box", box.reached_f0292_name_box, 1,
          "champion_index 0 is in range");
    CHECK("live0.is_inventory_champion", box.is_inventory_champion, 0,
          "live non-inventory branch");
    CHECK("live0.is_dead", box.is_dead, 0,
          "live non-dead branch");
    CHECK("live0.leader_index_match", box.leader_index_match, 1,
          "champion 0 matches leader 0");
    CHECK("live0.status_box_x_anchor", box.status_box_x_anchor, 0,
          "CHAMDRAW.C F0292:750 0*69=0");
    CHECK("live0.name_box_left", box.name_box_left, 0,
          "CHAMDRAW.C F0292:879 M768_BOX_LEFT=L0868=0");
    CHECK("live0.name_box_top", box.name_box_top, 0,
          "CHAMDRAW.C F0292:879 M770_BOX_TOP=0");
    CHECK("live0.name_box_right", box.name_box_right, 42,
          "CHAMDRAW.C F0292:881 M769_BOX_RIGHT=L0868+42=42");
    CHECK("live0.name_box_bottom", box.name_box_bottom, 6,
          "CHAMDRAW.C F0292:880 M771_BOX_BOTTOM=6");
    CHECK("live0.name_box_width", box.name_box_width, 43,
          "right-left+1=42-0+1=43");
    CHECK("live0.name_box_height", box.name_box_height, 7,
          "bottom-top+1=6-0+1=7");
    CHECK("live0.name_print_x", box.name_print_x, 1,
          "CHAMDRAW.C F0292:884 L0868+1");
    CHECK("live0.name_print_y", box.name_print_y, 5,
          "CHAMDRAW.C F0292:884 y=5");
    CHECK("live0.name_zone_index", box.name_zone_index, 159,
          "DEFS.H:3787 C159 + champion_index");
    CHECK("live0.color_fg", box.color_fg, 9,
          "CHAMDRAW.C F0292:845 MEDIA049 PC leader = C09_COLOR_GOLD");
    CHECK("live0.color_bg", box.color_bg, 1,
          "CHAMDRAW.C F0292:881 C01_COLOR_DARK_GRAY fill");
    CHECK("live0.box_in_range", box.box_in_range, 1,
          "43x7 box with 1-px left pad and 7 chars * 6 px/char");
    CHECK("live0.zone_in_range", box.zone_in_range, 1,
          "C159 + 0 = 159");
    CHECK("live0.color_cascade_correct", box.color_cascade_correct, 1,
          "leader C09 / C01 cascade");
    CHECK("live0.glyphs_that_fit", box.glyphs_that_fit_in_name_box, 7,
          "Name[8] field caps visible chars at 7");
    CHECK("live0.name_clip_text_pixels", box.name_clip_text_pixels, 42,
          "7 chars * 6 px = 42");
    CHECK("live0.name_field_clip_holds", box.name_field_clip_holds, 1,
          "42 <= 43 name-box width");
    CHECK("live0.variant",
          (int)box.variant,
          (int)DM1_V1_CPNBC_VARIANT_PC34_STATUS_BOX_LIVE_PC34,
          "live status box variant");
}

static void test_status_box_live_per_champion(void)
{
    int champion;
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box;

    for (champion = 0; champion < DM1_V1_CPNBC_CHAMPION_COUNT_PC34;
         ++champion) {
        memset(&box, 0, sizeof(box));
        /* leader is always 0; non-leader slots should pick up
         * C13_COLOR_LIGHTEST_GRAY, leader slot picks up C09. */
        dm1_v1_cpnbc_build_status_box_live_pc34(champion, 0, "ABCDEFG", &box);

        {
            char id_left[64];
            char id_right[64];
            char id_printx[64];
            char id_zone[64];
            char id_color[64];
            snprintf(id_left, sizeof(id_left),
                     "live_per.champion%d.name_box_left", champion);
            snprintf(id_right, sizeof(id_right),
                     "live_per.champion%d.name_box_right", champion);
            snprintf(id_printx, sizeof(id_printx),
                     "live_per.champion%d.name_print_x", champion);
            snprintf(id_zone, sizeof(id_zone),
                     "live_per.champion%d.name_zone_index", champion);
            snprintf(id_color, sizeof(id_color),
                     "live_per.champion%d.color_fg", champion);
            CHECK(id_left, box.name_box_left, champion * 69,
                  "CHAMDRAW.C F0292:750/879 champion*69");
            CHECK(id_right, box.name_box_right, champion * 69 + 42,
                  "CHAMDRAW.C F0292:881 champion*69+42");
            CHECK(id_printx, box.name_print_x, champion * 69 + 1,
                  "CHAMDRAW.C F0292:884 champion*69+1");
            CHECK(id_zone, box.name_zone_index, 159 + champion,
                  "DEFS.H:3787 C159+championIndex");
            CHECK(id_color, box.color_fg,
                  champion == 0 ? 9 : 13,
                  "CHAMDRAW.C F0292:845 leader C09 vs non-leader C13");
        }
    }
}

static void test_status_box_live_color_cascade(void)
{
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box;
    int champion;
    int leader_index = 2; /* slot 2 is the leader; slot 0/1/3 are not. */

    for (champion = 0; champion < DM1_V1_CPNBC_CHAMPION_COUNT_PC34;
         ++champion) {
        memset(&box, 0, sizeof(box));
        dm1_v1_cpnbc_build_status_box_live_pc34(
            champion, leader_index, "ABCDEFG", &box);
        {
            char id[64];
            snprintf(id, sizeof(id), "cascade.champion%d", champion);
            CHECK(id, box.color_fg,
                  champion == leader_index ? 9 : 13,
                  "CHAMDRAW.C F0292:845 leader gold vs non-leader gray");
        }
    }
}

/* ----- Status box dead ----- */

static void test_status_box_dead_default(void)
{
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpnbc_build_status_box_dead_pc34(1, "ABCDEFG", &box);

    CHECK("dead1.is_dead", box.is_dead, 1,
          "dead branch selected");
    CHECK("dead1.is_inventory_champion", box.is_inventory_champion, 0,
          "dead branch is non-inventory");
    CHECK("dead1.status_box_x_anchor", box.status_box_x_anchor, 69,
          "CHAMDRAW.C F0292:750 1*69=69");
    CHECK("dead1.name_print_x", box.name_print_x, 70,
          "CHAMDRAW.C F0292:818/827 L0868+1 = 1*69+1 = 70");
    CHECK("dead1.name_print_y", box.name_print_y, 5,
          "CHAMDRAW.C F0292:818/827 y=5");
    CHECK("dead1.name_zone_index", box.name_zone_index, 164,
          "DEFS.H:3791 C163 + champion_index (zone-centered name)");
    CHECK("dead1.color_fg", box.color_fg, 13,
          "CHAMDRAW.C F0292:818/827 C13_COLOR_LIGHTEST_GRAY");
    CHECK("dead1.color_bg", box.color_bg, 1,
          "CHAMDRAW.C F0292:818/827 C01_COLOR_DARK_GRAY");
    CHECK("dead1.color_cascade_correct", box.color_cascade_correct, 1,
          "dead C13/C01 cascade");
    CHECK("dead1.box_in_range", box.box_in_range, 1,
          "dead name strip fits 43x7 box with 1-px left pad");
    CHECK("dead1.variant",
          (int)box.variant,
          (int)DM1_V1_CPNBC_VARIANT_PC34_STATUS_BOX_DEAD_PC34,
          "dead status box variant");
}

static void test_status_box_dead_per_champion(void)
{
    int champion;
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box;

    for (champion = 0; champion < DM1_V1_CPNBC_CHAMPION_COUNT_PC34;
         ++champion) {
        memset(&box, 0, sizeof(box));
        dm1_v1_cpnbc_build_status_box_dead_pc34(champion, "ABCDEFG", &box);
        {
            char id_printx[64];
            char id_zone[64];
            snprintf(id_printx, sizeof(id_printx),
                     "dead_per.champion%d.name_print_x", champion);
            snprintf(id_zone, sizeof(id_zone),
                     "dead_per.champion%d.name_zone_index", champion);
            CHECK(id_printx, box.name_print_x, champion * 69 + 1,
                  "CHAMDRAW.C F0292:818/827 champion*69+1");
            CHECK(id_zone, box.name_zone_index, 163 + champion,
                  "DEFS.H:3791 C163+championIndex");
        }
    }
}

/* ----- Inventory viewport ----- */

static void test_inventory_viewport_default(void)
{
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    /* 5-char name, Z title (no passthrough) => 6*5+3+6 = 39. */
    dm1_v1_cpnbc_build_inventory_viewport_pc34(0, 0, "ABCDE", 'Z', &box);

    CHECK("inv0.is_inventory_champion", box.is_inventory_champion, 1,
          "inventory viewport branch");
    CHECK("inv0.is_dead", box.is_dead, 0,
          "inventory branch is non-dead");
    CHECK("inv0.inventory_name_print_x", box.inventory_name_print_x, 3,
          "CHAMDRAW.C F0292:855 F0052 x=3");
    CHECK("inv0.inventory_name_print_y", box.inventory_name_print_y, 7,
          "CHAMDRAW.C F0292:855 F0052 y=7");
    CHECK("inv0.inventory_title_x", box.inventory_title_x, 6 * 5 + 3 + 6,
          "CHAMDRAW.C F0292:856/866 6*5+3+6=39 (no passthrough)");
    CHECK("inv0.inventory_title_x_after_punct", box.inventory_title_x_after_punct,
          6 * 5 + 3,
          "before-punctuation baseline = 6*5+3 = 33");
    CHECK("inv0.title_passthrough_no_increment",
          box.title_passthrough_no_increment, 0,
          "Z is not in passthrough set");
    CHECK("inv0.title_first_char", box.title_first_char, 'Z',
          "captured title_first_char");
    CHECK("inv0.color_fg", box.color_fg, 9,
          "CHAMDRAW.C F0292:845 leader C09_COLOR_GOLD (leader=0)");
    CHECK("inv0.variant",
          (int)box.variant,
          (int)DM1_V1_CPNBC_VARIANT_PC34_INVENTORY_VIEWPORT_PC34,
          "inventory viewport variant");
}

static void test_inventory_viewport_punctuation_passthrough(void)
{
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    /* 4-char name, comma title (passthrough) => 6*4+3+0 = 27. */
    dm1_v1_cpnbc_build_inventory_viewport_pc34(1, 0, "ABCD", ',', &box);
    CHECK("inv_punct.title_passthrough_no_increment",
          box.title_passthrough_no_increment, 1,
          "comma is in passthrough set");
    CHECK("inv_punct.inventory_title_x", box.inventory_title_x,
          6 * 4 + 3,
          "CHAMDRAW.C F0292:856 6*4+3=27 (comma passthrough skips +6)");
}

static void test_inventory_viewport_non_leader_color(void)
{
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    /* leader=0, champion=2 => non-leader => C13. */
    dm1_v1_cpnbc_build_inventory_viewport_pc34(2, 0, "ABCDE", 'Z', &box);
    CHECK("inv_nonleader.color_fg", box.color_fg, 13,
          "CHAMDRAW.C F0292:845 non-leader C13_COLOR_LIGHTEST_GRAY");
}

/* ----- Model ----- */

static void test_model_default(void)
{
    DM1_V1_CPNBC_ModelPc34Compat model;
    int champion;

    memset(&model, 0, sizeof(model));
    dm1_v1_cpnbc_build_model_pc34(0, "ABCDEFG", &model);

    CHECK("model.contract_only", model.contract_only, 1,
          "contract-only marker");
    CHECK("model.disjoint_from_f0354_box_variants_pc34_compat",
          model.disjoint_from_f0354_box_variants_pc34_compat, 1,
          "disjoint from F0354 box variants");
    CHECK("model.disjoint_from_portrait_box_blit_dispatch_pc34_compat",
          model.disjoint_from_portrait_box_blit_dispatch_pc34_compat, 1,
          "disjoint from portrait-box blit dispatch");
    CHECK("model.disjoint_from_portrait_box_redraw_states_pc34_compat",
          model.disjoint_from_portrait_box_redraw_states_pc34_compat, 1,
          "disjoint from portrait-box redraw states");
    CHECK("model.disjoint_from_portrait_state_redraw_pc34_compat",
          model.disjoint_from_portrait_state_redraw_pc34_compat, 1,
          "disjoint from portrait-state redraw");
    CHECK("model.disjoint_from_mouth_eye_release_pc34_compat",
          model.disjoint_from_mouth_eye_release_pc34_compat, 1,
          "disjoint from mouth/eye release");
    CHECK("model.disjoint_from_food_water_status_box_pc34_compat",
          model.disjoint_from_food_water_status_box_pc34_compat, 1,
          "disjoint from food/water status box");
    CHECK("model.disjoint_from_hud_food_water_recompute_pc34_compat",
          model.disjoint_from_hud_food_water_recompute_pc34_compat, 1,
          "disjoint from HUD food/water recompute");
    CHECK("model.disjoint_from_action_hand_slot_priority_pc34_compat",
          model.disjoint_from_action_hand_slot_priority_pc34_compat, 1,
          "disjoint from action-hand slot priority");
    CHECK("model.disjoint_from_champion_panel_pixels_runtime_probe",
          model.disjoint_from_champion_panel_pixels_runtime_probe, 1,
          "disjoint from pixels runtime probe");
    CHECK("model.disjoint_from_champion_panel_status_states_runtime_probe",
          model.disjoint_from_champion_panel_status_states_runtime_probe, 1,
          "disjoint from status-states runtime");
    CHECK("model.disjoint_from_champion_panel_status_hand_slot_pixels_source_"
          "lock",
          model
              .disjoint_from_champion_panel_status_hand_slot_pixels_source_lock,
          1, "disjoint from status-hand slot pixels source lock");
    CHECK("model.disjoint_from_champion_panel_partial_party_pixel_probe",
          model.disjoint_from_champion_panel_partial_party_pixel_probe, 1,
          "disjoint from partial-party pixel probe");
    CHECK("model.disjoint_from_champion_panel_status_box_asset_slice_probe",
          model.disjoint_from_champion_panel_status_box_asset_slice_probe, 1,
          "disjoint from status-box asset slice");
    CHECK("model.disjoint_from_champion_panel_recompute_runtime",
          model.disjoint_from_champion_panel_recompute_runtime, 1,
          "disjoint from recompute runtime");
    CHECK("model.disjoint_from_champion_panel_leader_rotation_pixel_slice",
          model.disjoint_from_champion_panel_leader_rotation_pixel_slice, 1,
          "disjoint from leader-rotation pixel slice");
    CHECK("model.disjoint_from_champion_panel_action_cell_slotbox_runtime",
          model.disjoint_from_champion_panel_action_cell_slotbox_runtime, 1,
          "disjoint from action-cell slotbox runtime");
    CHECK("model.disjoint_from_champion_panel_hand_slot_priority_source_"
          "lock",
          model.disjoint_from_champion_panel_hand_slot_priority_source_lock, 1,
          "disjoint from hand-slot priority source lock");
    CHECK("model.disjoint_from_champion_panel_shield_border_pixel",
          model.disjoint_from_champion_panel_shield_border_pixel, 1,
          "disjoint from shield-border pixel");
    CHECK("model.disjoint_from_champion_panel_pressing_mouth_eye_statusbox_"
          "pc34_compat",
          model
              .disjoint_from_champion_panel_pressing_mouth_eye_statusbox_pc34_compat,
          1, "disjoint from pressing mouth/eye statusbox");
    CHECK("model.disjoint_from_inventory_champion_switch_hand_carry_"
          "pc34_compat",
          model.disjoint_from_inventory_champion_switch_hand_carry_pc34_compat,
          1, "disjoint from inventory champion switch hand carry");
    CHECK("model.disjoint_from_spell_area_overlay_pc34_compat",
          model.disjoint_from_spell_area_overlay_pc34_compat, 1,
          "disjoint from spell-area overlay");
    CHECK("model.disjoint_from_hud_pc34_compat_name_strip_x_anchor_only",
          model.disjoint_from_hud_pc34_compat_name_strip_x_anchor_only, 1,
          "disjoint from HUD name-strip x-anchor edge");

    for (champion = 0; champion < DM1_V1_CPNBC_CHAMPION_COUNT_PC34;
         ++champion) {
        char id_variant[64];
        char id_left[64];
        char id_zone[64];
        char id_color[64];
        snprintf(id_variant, sizeof(id_variant),
                 "model.champion%d.variant", champion);
        snprintf(id_left, sizeof(id_left),
                 "model.champion%d.name_box_left", champion);
        snprintf(id_zone, sizeof(id_zone),
                 "model.champion%d.name_zone_index", champion);
        snprintf(id_color, sizeof(id_color),
                 "model.champion%d.color_fg", champion);
        CHECK(id_variant,
              (int)model.champions[champion].variant,
              (int)DM1_V1_CPNBC_VARIANT_PC34_STATUS_BOX_LIVE_PC34,
              "all four champions default to live status-box variant");
        CHECK(id_left, model.champions[champion].name_box_left,
              champion * 69,
              "CHAMDRAW.C F0292:879 champion*69");
        CHECK(id_zone, model.champions[champion].name_zone_index,
              159 + champion,
              "DEFS.H:3787 C159+championIndex");
        CHECK(id_color, model.champions[champion].color_fg,
              champion == 0 ? 9 : 13,
              "CHAMDRAW.C F0292:845 leader C09 vs non-leader C13");
    }
    CHECK("model.deterministic_hash_nonzero", model.deterministic_hash != 0, 1,
          "deterministic hash is non-zero (FNV-1a seed OR-d with per-champ)");
    printf("HASH_MODEL 0x%08X\n", model.deterministic_hash);
    for (champion = 0; champion < DM1_V1_CPNBC_CHAMPION_COUNT_PC34;
         ++champion) {
        printf("HASH_BOX_%d 0x%08X\n", champion,
               model.champions[champion].hash);
    }
}

/* ----- Stress ----- */

static void test_status_box_live_stress_hash_stable(void)
{
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box1;
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box2;
    uint32_t hash1;
    uint32_t hash2;

    memset(&box1, 0, sizeof(box1));
    memset(&box2, 0, sizeof(box2));
    dm1_v1_cpnbc_build_status_box_live_pc34(2, 2, "ABCDEFG", &box1);
    dm1_v1_cpnbc_build_status_box_live_pc34(2, 2, "ABCDEFG", &box2);
    hash1 = box1.hash;
    hash2 = box2.hash;
    CHECK("live_hash.stable", hash1 == hash2, 1,
          "FNV-1a hash is deterministic for identical inputs");
    CHECK("live_hash.nonzero", hash1 != 0, 1,
          "FNV-1a hash is non-zero after the live fields are mixed in");
}

static void test_status_box_live_stress_box_width(void)
{
    int champion;
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box;

    for (champion = 0; champion < DM1_V1_CPNBC_CHAMPION_COUNT_PC34;
         ++champion) {
        memset(&box, 0, sizeof(box));
        dm1_v1_cpnbc_build_status_box_live_pc34(champion, 0, "ABCDEFG", &box);
        {
            char id[64];
            snprintf(id, sizeof(id),
                     "live_stress_width.champion%d.glyphs_fit", champion);
            CHECK(id, box.glyphs_that_fit_in_name_box, 7,
                  "Name[8] field caps at 7 visible glyphs");
            snprintf(id, sizeof(id),
                     "live_stress_width.champion%d.width", champion);
            CHECK(id, box.name_box_width, 43,
                  "7 glyphs * 6 px + 1 left pad = 43 px");
        }
    }
}

static void test_invalid_champion_index(void)
{
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpnbc_build_status_box_live_pc34(-1, 0, "ABCDEFG", &box);
    CHECK("invalid.live.reached", box.reached_f0292_name_box, 0,
          "negative champion_index is out of range");
    CHECK("invalid.live.variant",
          (int)box.variant,
          (int)DM1_V1_CPNBC_VARIANT_PC34_NAME_BOX_NOT_REACHED_PC34,
          "invalid index => not-reached variant");

    memset(&box, 0, sizeof(box));
    dm1_v1_cpnbc_build_status_box_live_pc34(4, 0, "ABCDEFG", &box);
    CHECK("invalid.live.4.reached", box.reached_f0292_name_box, 0,
          "champion_index 4 is out of range");

    memset(&box, 0, sizeof(box));
    dm1_v1_cpnbc_build_status_box_dead_pc34(-1, "ABCDEFG", &box);
    CHECK("invalid.dead.reached", box.reached_f0292_name_box, 0,
          "negative champion_index is out of range (dead)");

    memset(&box, 0, sizeof(box));
    dm1_v1_cpnbc_build_inventory_viewport_pc34(4, 0, "ABCDEFG", 'Z', &box);
    CHECK("invalid.inv.reached", box.reached_f0292_name_box, 0,
          "champion_index 4 is out of range (inventory)");
}

static void test_clip_name_field_overflow(void)
{
    /* A 20-char name (the Title[20] size) would not clip on the
     * inventory viewport column 3 path; the Name[8] field is the
     * only clip that matters for the F0053 status box print. The
     * name_field_text_pixels contract caps at 42 (7 chars * 6 px)
     * which always fits the 43-pixel name box. */
    int pixels = dm1_v1_cpnbc_name_field_text_pixels_pc34("ABCDEFGHIJKLMNOPQRST");
    CHECK("clip.20char_capped_at_42", pixels, 42,
          "Name[8] caps at 7 visible chars * 6 px = 42");
}

/* ----- Source evidence ----- */

static void test_source_evidence(void)
{
    const char *evidence = dm1_v1_cpnbc_source_evidence_pc34();

    check_contains("evidence.l0868", evidence,
                   "CHAMDRAW.C F0292:750",
                   "L0868 = championIndex * 69");
    check_contains("evidence.dead_strip", evidence,
                   "CHAMDRAW.C F0292:818-833",
                   "dead-champion name strip");
    check_contains("evidence.live_name_box", evidence,
                   "CHAMDRAW.C F0292:843-895",
                   "live non-inventory NAME_TITLE branch");
    check_contains("evidence.color_cascade", evidence, "CHAMDRAW.C F0292:845",
                   "color cascade");
    check_contains("evidence.inventory_branch", evidence,
                   "CHAMDRAW.C F0292:855-871",
                   "inventory viewport name/title");
    check_contains("evidence.title_x_formula", evidence,
                   "6 * strlen(Name) + 3",
                   "L0869 = 6*strlen+3");
    check_contains("evidence.title_punct", evidence,
                   "Title[0] is not one of {',', ';', '-'}",
                   "punctuation passthrough +6");
    check_contains("evidence.c69", evidence,
                   "C69_CHAMPION_STATUS_BOX_SPACING=69",
                   "C69 anchor");
    check_contains("evidence.c01", evidence, "C01_COLOR_DARK_GRAY=1",
                   "C01 anchor");
    check_contains("evidence.c09", evidence, "C09_COLOR_GOLD=9",
                   "C09 anchor");
    check_contains("evidence.c13", evidence, "C13_COLOR_LIGHTEST_GRAY=13",
                   "C13 anchor");
    check_contains("evidence.c159", evidence,
                   "C159_ZONE_CHAMPION_0_STATUS_BOX_NAME=159",
                   "C159 anchor");
    check_contains("evidence.c163", evidence,
                   "C163_ZONE_FIRST_CHAMPION_NAME=163",
                   "C163 anchor");
    check_contains("evidence.name_field", evidence, "Name[8]",
                   "Name[8] field cap");
    check_contains("evidence.no_real_asset_claim", evidence,
                   "no real-asset bitmap parity claim",
                   "contract-only no-claim marker");
    check_contains("evidence.disjoint_marker", evidence,
                   "name-box clip contract",
                   "disjoint slice marker");
    check_contains("evidence.glyph_width", evidence,
                   "6-px/char PC 3.4 font",
                   "6-px/char font");
    check_contains("evidence.byte_coord_box", evidence, "M770_BOX_TOP=0",
                   "byte-coord box top");
    check_contains("evidence.byte_coord_right", evidence,
                   "L0868_i_ChampionStatusBoxX) + 42",
                   "byte-coord box right = L0868+42");
    check_contains("evidence.fill_color", evidence,
                   "C01_COLOR_DARK_GRAY",
                   "fill color");
    check_contains("evidence.print_xy", evidence, "L0868 + 1, 5",
                   "print x=L0868+1, y=5");
}

int main(void)
{
    test_pure_compute_title_x_baseline();
    test_pure_compute_title_x_punct_passthrough();
    test_pure_compute_title_x_empty_name();
    test_pure_compute_title_x_null_name();
    test_pure_title_passthrough();
    test_pure_name_field_text_pixels();
    test_status_box_live_default();
    test_status_box_live_per_champion();
    test_status_box_live_color_cascade();
    test_status_box_dead_default();
    test_status_box_dead_per_champion();
    test_inventory_viewport_default();
    test_inventory_viewport_punctuation_passthrough();
    test_inventory_viewport_non_leader_color();
    test_model_default();
    test_status_box_live_stress_hash_stable();
    test_status_box_live_stress_box_width();
    test_invalid_champion_index();
    test_clip_name_field_overflow();
    test_source_evidence();

    if (g_failures) {
        printf("FAIL test_dm1_v1_champion_panel_name_box_clip_pc34_compat"
               " failures=%d assertions=%d\n",
               g_failures, g_assertions);
        printf("Assertions: %d\n", g_assertions);
        printf("Failures: %d\n", g_failures);
        return 1;
    }

    printf("PASS test_dm1_v1_champion_panel_name_box_clip_pc34_compat"
           " failures=0 assertions=%d\n",
           g_assertions);
    printf("Assertions: %d\n", g_assertions);
    printf("Failures: %d\n", g_failures);
    return 0;
}
