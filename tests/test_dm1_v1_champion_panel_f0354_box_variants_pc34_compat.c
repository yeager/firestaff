/*
 * DM1 V1 champion panel F0354 box-variants gate tests.
 *
 * Contract-only, no-asset fixture. Pins the post-dispatch box
 * geometry and variant selection of PANEL.C F0354. The
 * F0292 -> F0354 dispatch predicate is pinned separately by
 * `dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat`; this
 * fixture does NOT cover the dispatch predicate, the
 * CHAMDRAW.C F0292 short-circuit, the dead champion branch, the
 * non-inventory champion fallback, the F0254 secondary dispatch,
 * the F0293 champion-index loop, or the C151..C154 status-box
 * zone stride. It covers:
 *
 *   1. The PC 3.4 byte-coordinate box formula
 *      (championIndex*69+7, 0, championIndex*69+7+31, 28)
 *      and the 32x29 portrait dimensions.
 *   2. The zone-table box variant used by Amiga/console ports
 *      (F0638_GetZone(C175 + championIndex, ...)).
 *   3. The post-blit invisibility hatch
 *      (F0136_VIDEO_HatchScreenBox on C175+championIndex with
 *      C12_COLOR_DARKEST_GRAY) and the Event71Count_Invisibility
 *      gate that arms it.
 *
 * Source-lock anchors: ReDMCSB PANEL.C F0354:2195-2242, DEFS.H:2157
 * (C69_CHAMPION_STATUS_BOX_SPACING), DEFS.H:2471 (C016_BYTE_WIDTH),
 * DEFS.H:3793 (C175_ZONE_FIRST_CHAMPION_STATUS_BOX),
 * DEFS.H:6391-6392 (G2078_C32_PortraitWidth / G2079_C29_PortraitHeight),
 * and the MEDIA720 invisibility-hatch block at PANEL.C F0354:2237-2241.
 *
 * The fixture does not duplicate:
 *  - the F0292 -> F0354 dispatch gate
 *    (dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat),
 *  - the portrait-state redraw matrix
 *    (dm1_v1_champion_panel_portrait_state_redraw_pc34_compat +
 *     dm1_v1_champion_panel_portrait_box_redraw_states_pc34_compat),
 *  - the mouth/eye press release gate
 *    (test_dm1_v1_champion_panel_mouth_eye_release_pc34_compat),
 *  - the food/water status-box gate
 *    (test_dm1_v1_champion_panel_food_water_status_box_pc34_compat),
 *  - the HUD recompute gate, the action-hand slot-priority gate,
 *    the action-cell slotbox gate, the status-hand slot-pixels
 *    gate, the hand-slot-priority gate, the held-hand probe,
 *    the held-hand icon direction probe, the partial-party probe,
 *    the leader-rotation pixel slice, the shield-border pixel
 *    slice, the recompute runtime, the status-box asset slice,
 *    the status-states runtime, the status-states partial party
 *    probe, the pixels runtime, the panel shield border,
 *    the panel pressing mouth/eye statusbox,
 *    the inventory champion switch hand carry,
 *  - any chest/inventory/mirror runtime regression.
 */

#include "firestaff/dm1/v1/champion_panel/dm1_v1_champion_panel_f0354_box_variants_pc34_compat.h"

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

static void test_byte_coord_default(void)
{
    DM1_V1_CPFBV_ChampionBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpfbv_build_byte_coord_pc34(0, 0, &box);

    CHECK("byte_default.reached_f0354", box.reached_f0354, 1,
          "PANEL.C F0354:2195 champion_index 0 is in range");
    CHECK("byte_default.byte_left", box.byte_left, 7,
          "PANEL.C F0354:2208 (0*69)+7");
    CHECK("byte_default.byte_top", box.byte_top, 0,
          "PANEL.C F0354:2208 M770_BOX_TOP=0");
    CHECK("byte_default.byte_right", box.byte_right, 38,
          "PANEL.C F0354:2209 7+31");
    CHECK("byte_default.byte_bottom", box.byte_bottom, 28,
          "PANEL.C F0354:2208 M771_BOX_BOTTOM=28");
    CHECK("byte_default.byte_width", box.byte_width, 32,
          "DEFS.H:6391 G2078_C32_PortraitWidth");
    CHECK("byte_default.byte_height", box.byte_height, 29,
          "DEFS.H:6392 G2079_C29_PortraitHeight");
    CHECK("byte_default.portrait_zone", box.portrait_zone, 175,
          "DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX for champ 0");
    CHECK("byte_default.variant", (int)box.variant,
          (int)DM1_V1_CPFBV_VARIANT_PC34_BYTE_COORD_PC34,
          "PC 3.4 byte-coord variant");
    CHECK("byte_default.event71_zero", box.event71_count_invisibility, 0,
          "no invisibility spell active");
    CHECK("byte_default.hatch_applies", box.hatch_applies, 0,
          "hatch is gated by Event71>0");
    CHECK("byte_default.hatch_color", box.hatch_color, 12,
          "PANEL.C F0354:2240 C12_COLOR_DARKEST_GRAY");
    CHECK("byte_default.byte_box_in_range", box.byte_box_in_range, 1,
          "byte box is 32x29 and on the canonical 69-stride");
    CHECK("byte_default.portrait_zone_in_range", box.portrait_zone_in_range,
          1, "C175+0 is in range");
    CHECK("byte_default.hatch_zone_in_range", box.hatch_zone_in_range, 1,
          "C175+0 hatch zone is in range");
    CHECK("byte_default.zone_left_zero", box.zone_left, 0,
          "byte-coord variant does not consume F0638_GetZone");
}

static void test_byte_coord_per_champion(void)
{
    int champion;
    int expected_left;

    for (champion = 0; champion < DM1_V1_CPFBV_CHAMPION_COUNT_PC34;
         ++champion) {
        DM1_V1_CPFBV_ChampionBoxPc34Compat box;

        memset(&box, 0, sizeof(box));
        dm1_v1_cpfbv_build_byte_coord_pc34(0, champion, &box);
        expected_left = (champion * 69) + 7;

        char id_left[64];
        char id_right[64];
        char id_zone[64];
        snprintf(id_left, sizeof(id_left),
                 "byte_per_champ%d.byte_left", champion);
        snprintf(id_right, sizeof(id_right),
                 "byte_per_champ%d.byte_right", champion);
        snprintf(id_zone, sizeof(id_zone),
                 "byte_per_champ%d.portrait_zone", champion);
        CHECK(id_left, box.byte_left, expected_left,
              "PANEL.C F0354:2208 (champ*69)+7");
        CHECK(id_right, box.byte_right, expected_left + 31,
              "PANEL.C F0354:2209 left+31");
        CHECK(id_zone, box.portrait_zone, 175 + champion,
              "DEFS.H:3793 C175+championIndex");
    }
}

static void test_byte_coord_hatch_off_when_event71_zero(void)
{
    DM1_V1_CPFBV_ChampionBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpfbv_build_byte_coord_pc34(0, 2, &box);

    CHECK("hatch_off.reached_f0354", box.reached_f0354, 1,
          "PANEL.C F0354:2195 champion_index 2 in range");
    CHECK("hatch_off.hatch_applies", box.hatch_applies, 0,
          "PANEL.C F0354:2238 Event71==0 -> no hatch");
    CHECK("hatch_off.variant", (int)box.variant,
          (int)DM1_V1_CPFBV_VARIANT_PC34_BYTE_COORD_PC34,
          "byte-coord variant is the selected variant when hatch is off");
}

static void test_byte_coord_hatch_on_when_event71_nonzero(void)
{
    DM1_V1_CPFBV_ChampionBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpfbv_build_byte_coord_pc34(1, 1, &box);

    CHECK("hatch_on.reached_f0354", box.reached_f0354, 1,
          "PANEL.C F0354:2195 champion_index 1 in range");
    CHECK("hatch_on.hatch_applies", box.hatch_applies, 1,
          "PANEL.C F0354:2238 Event71>0 -> hatch arms");
    CHECK("hatch_on.event71_count_invisibility",
          box.event71_count_invisibility, 1,
          "MENU.C F0404_invisibility_count records the spell");
    CHECK("hatch_on.hatch_zone", box.hatch_zone, 176,
          "PANEL.C F0354:2240 C175+championIndex=176 for champ 1");
    CHECK("hatch_on.hatch_color", box.hatch_color, 12,
          "PANEL.C F0354:2240 C12_COLOR_DARKEST_GRAY");
    /* The byte-coord variant is still the geometry; the hatch
     * variant is a state tag, not a replacement for the blit. */
    CHECK("hatch_on.variant_byte_coord", (int)box.variant,
          (int)DM1_V1_CPFBV_VARIANT_PC34_BYTE_COORD_PC34,
          "byte-coord variant still owns the geometry; hatch is additive");
}

static void test_byte_coord_stress_event71_count(void)
{
    int event71;
    int champion;
    int saw_hatch_on = 0;
    int saw_hatch_off = 0;
    DM1_V1_CPFBV_ChampionBoxPc34Compat box;

    for (event71 = 0; event71 <= 5; ++event71) {
        for (champion = 0; champion < DM1_V1_CPFBV_CHAMPION_COUNT_PC34;
             ++champion) {
            memset(&box, 0, sizeof(box));
            dm1_v1_cpfbv_build_byte_coord_pc34(event71, champion, &box);
            if (box.hatch_applies) {
                ++saw_hatch_on;
            } else {
                ++saw_hatch_off;
            }
        }
    }
    CHECK("byte_stress.hatch_on_count", saw_hatch_on,
          5 * DM1_V1_CPFBV_CHAMPION_COUNT_PC34,
          "PANEL.C F0354:2238 Event71 in 1..5 arms hatch for all 4 champs");
    CHECK("byte_stress.hatch_off_count", saw_hatch_off,
          1 * DM1_V1_CPFBV_CHAMPION_COUNT_PC34,
          "PANEL.C F0354:2238 Event71==0 leaves hatch off for all 4 champs");
}

static void test_zone_variant_uses_f0638_box(void)
{
    DM1_V1_CPFBV_ChampionBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpfbv_build_zone_table_pc34(0, 2, 100, 200, 130, 230, &box);

    CHECK("zone.reached_f0354", box.reached_f0354, 1,
          "PANEL.C F0354:2195 champion_index 2 in range");
    CHECK("zone.zone_left", box.zone_left, 100,
          "F0638_GetZone first element");
    CHECK("zone.zone_top", box.zone_top, 200,
          "F0638_GetZone second element");
    CHECK("zone.zone_right", box.zone_right, 130,
          "F0638_GetZone third element");
    CHECK("zone.zone_bottom", box.zone_bottom, 230,
          "F0638_GetZone fourth element");
    CHECK("zone.variant", (int)box.variant,
          (int)DM1_V1_CPFBV_VARIANT_PC34_ZONE_TABLE_PC34,
          "Amiga/console port zone-table variant");
    CHECK("zone.portrait_zone", box.portrait_zone, 177,
          "DEFS.H:3793 C175+2");
    CHECK("zone.byte_left_anchored", box.byte_left, 145,
          "PC 3.4 byte-coord left is still (2*69)+7");
    CHECK("zone.hatch_applies", box.hatch_applies, 0,
          "Event71=0 leaves hatch off on the zone path too");
}

static void test_zone_variant_with_event71(void)
{
    DM1_V1_CPFBV_ChampionBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpfbv_build_zone_table_pc34(3, 3, 200, 0, 231, 28, &box);

    CHECK("zone_with_hatch.reached_f0354", box.reached_f0354, 1,
          "PANEL.C F0354:2195 champion_index 3 in range");
    CHECK("zone_with_hatch.hatch_applies", box.hatch_applies, 1,
          "PANEL.C F0354:2238 Event71>0 -> hatch on the zone path");
    CHECK("zone_with_hatch.variant", (int)box.variant,
          (int)DM1_V1_CPFBV_VARIANT_PC34_ZONE_TABLE_PC34,
          "zone variant still owns the geometry; hatch is additive");
    CHECK("zone_with_hatch.hatch_zone", box.hatch_zone, 178,
          "PANEL.C F0354:2240 C175+3");
}

static void test_hatch_variant_explicit(void)
{
    DM1_V1_CPFBV_ChampionBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpfbv_build_hatch_pc34(2, 1, &box);

    CHECK("hatch_var.reached_f0354", box.reached_f0354, 1,
          "PANEL.C F0354:2195 champion_index 1 in range");
    CHECK("hatch_var.hatch_applies", box.hatch_applies, 1,
          "PANEL.C F0354:2238 Event71>0 -> hatch arms");
    CHECK("hatch_var.variant", (int)box.variant,
          (int)DM1_V1_CPFBV_VARIANT_PC34_HATCH_PC34,
          "explicit hatch variant tag");
    CHECK("hatch_var.hatch_color", box.hatch_color, 12,
          "PANEL.C F0354:2240 C12_COLOR_DARKEST_GRAY");
    CHECK("hatch_var.hatch_zone", box.hatch_zone, 176,
          "PANEL.C F0354:2240 C175+1");
    CHECK("hatch_var.byte_left_anchored", box.byte_left, 76,
          "PANEL.C F0354:2208 (1*69)+7");
}

static void test_hatch_variant_off_when_event71_zero(void)
{
    DM1_V1_CPFBV_ChampionBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpfbv_build_hatch_pc34(0, 1, &box);

    CHECK("hatch_var_off.hatch_applies", box.hatch_applies, 0,
          "PANEL.C F0354:2238 Event71==0 -> hatch does not arm");
    CHECK("hatch_var_off.variant", (int)box.variant,
          (int)DM1_V1_CPFBV_VARIANT_PC34_BYTE_COORD_PC34,
          "fall-through to byte-coord variant when hatch is off");
}

static void test_invalid_champion_index(void)
{
    DM1_V1_CPFBV_ChampionBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpfbv_build_byte_coord_pc34(0, -1, &box);
    CHECK("invalid.reached_f0354", box.reached_f0354, 0,
          "out-of-range champion_index does not reach F0354");
    CHECK("invalid.variant", (int)box.variant,
          (int)DM1_V1_CPFBV_VARIANT_PC34_NOT_REACHED_PC34,
          "NOT_REACHED variant tag for invalid index");
    CHECK("invalid.byte_box_in_range", box.byte_box_in_range, 0,
          "byte-coord box is not in range for invalid index");
    CHECK("invalid.portrait_zone_in_range", box.portrait_zone_in_range, 0,
          "portrait zone is not in range for invalid index");
    CHECK("invalid.hatch_zone_in_range", box.hatch_zone_in_range, 0,
          "hatch zone is not in range for invalid index");

    memset(&box, 0, sizeof(box));
    dm1_v1_cpfbv_build_byte_coord_pc34(0, 4, &box);
    CHECK("oor.reached_f0354", box.reached_f0354, 0,
          "champion_index==4 is out of range");
    CHECK("oor.variant", (int)box.variant,
          (int)DM1_V1_CPFBV_VARIANT_PC34_NOT_REACHED_PC34,
          "NOT_REACHED variant tag for out-of-range index");
}

static void test_byte_coord_stress_event71_negative(void)
{
    DM1_V1_CPFBV_ChampionBoxPc34Compat box;

    memset(&box, 0, sizeof(box));
    dm1_v1_cpfbv_build_byte_coord_pc34(-1, 0, &box);

    CHECK("byte_neg_event71.hatch_applies", box.hatch_applies, 0,
          "negative Event71 leaves the hatch off (only >0 arms it)");
    CHECK("byte_neg_event71.variant", (int)box.variant,
          (int)DM1_V1_CPFBV_VARIANT_PC34_BYTE_COORD_PC34,
          "byte-coord variant owns the geometry; negative Event71 keeps the "
          "byte-coord branch");
}

static void test_model_default(void)
{
    DM1_V1_CPFBV_ModelPc34Compat model;
    int champion;

    memset(&model, 0, sizeof(model));
    dm1_v1_cpfbv_build_model_pc34(0, &model);

    CHECK("model.contract_only", model.contract_only, 1,
          "this fixture is contract-only");
    CHECK("model.disjoint_from_portrait_box_blit_dispatch_gate",
          model.disjoint_from_portrait_box_blit_dispatch_gate, 1,
          "this fixture is disjoint from the dispatch gate");
    CHECK("model.disjoint_from_portrait_state_redraw_pc34_compat",
          model.disjoint_from_portrait_state_redraw_pc34_compat, 1,
          "disjoint from portrait state redraw");
    CHECK("model.disjoint_from_mouth_eye_release_pc34_compat",
          model.disjoint_from_mouth_eye_release_pc34_compat, 1,
          "disjoint from mouth/eye release");
    CHECK("model.disjoint_from_food_water_status_box_pc34_compat",
          model.disjoint_from_food_water_status_box_pc34_compat, 1,
          "disjoint from food/water status box");
    CHECK("model.disjoint_from_hud_recompute_pc34_compat",
          model.disjoint_from_hud_recompute_pc34_compat, 1,
          "disjoint from HUD recompute");
    CHECK("model.disjoint_from_action_hand_slot_priority_pc34_compat",
          model.disjoint_from_action_hand_slot_priority_pc34_compat, 1,
          "disjoint from action-hand slot priority");
    CHECK(
        "model.disjoint_from_champion_panel_pixels_runtime_probe",
        model.disjoint_from_champion_panel_pixels_runtime_probe, 1,
        "disjoint from pixels runtime");
    CHECK("model.disjoint_from_champion_panel_status_states_runtime_probe",
          model.disjoint_from_champion_panel_status_states_runtime_probe, 1,
          "disjoint from status-states runtime");
    CHECK(
        "model.disjoint_from_champion_panel_status_hand_slot_pixels_source_"
        "lock",
        model.disjoint_from_champion_panel_status_hand_slot_pixels_source_lock,
        1, "disjoint from status hand-slot pixels source lock");
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
          model.disjoint_from_champion_panel_hand_slot_priority_source_lock,
          1, "disjoint from hand-slot priority source lock");
    CHECK("model.disjoint_from_champion_panel_shield_border_pixel",
          model.disjoint_from_champion_panel_shield_border_pixel, 1,
          "disjoint from shield-border pixel");
    CHECK("model.disjoint_from_champion_panel_pressing_mouth_eye_statusbox_"
          "pc34_compat",
          model.disjoint_from_champion_panel_pressing_mouth_eye_statusbox_pc34_compat,
          1, "disjoint from pressing mouth/eye statusbox");
    CHECK("model.disjoint_from_inventory_champion_switch_hand_carry_"
          "pc34_compat",
          model.disjoint_from_inventory_champion_switch_hand_carry_pc34_compat,
          1, "disjoint from inventory champion switch hand carry");

    for (champion = 0; champion < DM1_V1_CPFBV_CHAMPION_COUNT_PC34;
         ++champion) {
        char id_variant[64];
        char id_left[64];
        char id_zone[64];
        snprintf(id_variant, sizeof(id_variant),
                 "model.champion%d.variant", champion);
        snprintf(id_left, sizeof(id_left),
                 "model.champion%d.byte_left", champion);
        snprintf(id_zone, sizeof(id_zone),
                 "model.champion%d.portrait_zone", champion);
        CHECK(id_variant,
              (int)model.champions[champion].variant,
              (int)DM1_V1_CPFBV_VARIANT_PC34_BYTE_COORD_PC34,
              "all four champions default to byte-coord variant");
        CHECK(id_left, model.champions[champion].byte_left,
              (champion * 69) + 7,
              "PANEL.C F0354:2208 (champ*69)+7");
        CHECK(id_zone, model.champions[champion].portrait_zone, 175 + champion,
              "DEFS.H:3793 C175+championIndex");
    }
    CHECK("model.deterministic_hash_nonzero", model.deterministic_hash != 0, 1,
          "deterministic hash is non-zero (FNV-1a seed OR-d with per-champ)");
}

static void test_model_event71_hatch_uniform_across_party(void)
{
    DM1_V1_CPFBV_ModelPc34Compat model;
    int champion;
    int hatch_on_count = 0;

    memset(&model, 0, sizeof(model));
    dm1_v1_cpfbv_build_model_pc34(1, &model);

    for (champion = 0; champion < DM1_V1_CPFBV_CHAMPION_COUNT_PC34;
         ++champion) {
        if (model.champions[champion].hatch_applies) {
            ++hatch_on_count;
        }
    }
    CHECK("model_event71.hatch_on_count", hatch_on_count,
          DM1_V1_CPFBV_CHAMPION_COUNT_PC34,
          "PANEL.C F0354:2238 Event71 is party-wide so all 4 champs get the "
          "hatch");
}

static void test_byte_coord_stress_hash_stable(void)
{
    DM1_V1_CPFBV_ChampionBoxPc34Compat box1;
    DM1_V1_CPFBV_ChampionBoxPc34Compat box2;
    uint32_t hash1;
    uint32_t hash2;

    memset(&box1, 0, sizeof(box1));
    memset(&box2, 0, sizeof(box2));
    dm1_v1_cpfbv_build_byte_coord_pc34(2, 2, &box1);
    dm1_v1_cpfbv_build_byte_coord_pc34(2, 2, &box2);
    hash1 = box1.hash;
    hash2 = box2.hash;
    CHECK("byte_hash.stable", hash1 == hash2, 1,
          "FNV-1a hash is deterministic for identical inputs");
    CHECK("byte_hash.nonzero", hash1 != 0, 1,
          "FNV-1a hash is non-zero after the byte-coord fields are mixed in");
}

static void test_byte_coord_stress_byte_width_32(void)
{
    int champion;
    DM1_V1_CPFBV_ChampionBoxPc34Compat box;

    for (champion = 0; champion < DM1_V1_CPFBV_CHAMPION_COUNT_PC34;
         ++champion) {
        memset(&box, 0, sizeof(box));
        dm1_v1_cpfbv_build_byte_coord_pc34(0, champion, &box);
        char id[64];
        snprintf(id, sizeof(id), "byte_stress_width.champion%d.width",
                 champion);
        CHECK(id, box.byte_width, 32,
              "DEFS.H:6391 G2078_C32_PortraitWidth (byte-coord)");
        snprintf(id, sizeof(id), "byte_stress_width.champion%d.right_minus_"
                                  "left_plus_one",
                 champion);
        CHECK(id, box.byte_right - box.byte_left + 1, 32,
              "PANEL.C F0354:2209 left+31 implies width=32");
    }
}

static void test_source_evidence(void)
{
    const char *evidence = dm1_v1_cpfbv_source_evidence_pc34();

    check_contains("evidence.f0354_call", evidence, "PANEL.C F0354:2195-2242",
                   "F0354 function body");
    check_contains("evidence.byte_coord_box", evidence,
                   "PANEL.C F0354:2208-2213", "byte-coord box formula");
    check_contains("evidence.byte_coord_left", evidence, "M770_BOX_TOP=0",
                   "byte-coord top = 0");
    check_contains("evidence.byte_coord_right", evidence, "+31",
                   "byte-coord right = left+31");
    check_contains("evidence.zone_table_box", evidence,
                   "PANEL.C F0354:2222-2226", "zone-table box formula");
    check_contains("evidence.f0638_get_zone", evidence, "F0638_GetZone",
                   "F0638_GetZone call");
    check_contains("evidence.hatch", evidence, "PANEL.C F0354:2237-2241",
                   "invisibility hatch");
    check_contains("evidence.hatch_color", evidence, "C12_COLOR_DARKEST_GRAY",
                   "C12 hatch color");
    check_contains("evidence.event71", evidence, "Event71Count_Invisibility",
                   "Event71 invisibility gate");
    check_contains("evidence.c016_byte_width", evidence, "C016_BYTE_WIDTH=16",
                   "PC 3.4 byte width");
    check_contains("evidence.c69_spacing", evidence,
                   "C69_CHAMPION_STATUS_BOX_SPACING=69",
                   "status box spacing");
    check_contains("evidence.c175_portrait_zone", evidence,
                   "C175_ZONE_FIRST_CHAMPION_STATUS_BOX=175",
                   "portrait zone base");
    check_contains("evidence.g2078_g2079", evidence,
                   "G2078_C32_PortraitWidth", "portrait width global");
    check_contains("evidence.media720", evidence, "MEDIA720",
                   "MEDIA720 gate for the hatch");
    check_contains("evidence.no_real_asset_claim", evidence,
                   "no real-asset bitmap parity claim",
                   "contract-only no-claim marker");
    check_contains("evidence.disjoint_marker", evidence,
                   "post-dispatch geometry / variant contract",
                   "disjoint slice marker");
}

int main(void)
{
    test_byte_coord_default();
    test_byte_coord_per_champion();
    test_byte_coord_hatch_off_when_event71_zero();
    test_byte_coord_hatch_on_when_event71_nonzero();
    test_byte_coord_stress_event71_count();
    test_zone_variant_uses_f0638_box();
    test_zone_variant_with_event71();
    test_hatch_variant_explicit();
    test_hatch_variant_off_when_event71_zero();
    test_invalid_champion_index();
    test_byte_coord_stress_event71_negative();
    test_model_default();
    test_model_event71_hatch_uniform_across_party();
    test_byte_coord_stress_hash_stable();
    test_byte_coord_stress_byte_width_32();
    test_source_evidence();

    if (g_failures) {
        printf("FAIL test_dm1_v1_champion_panel_f0354_box_variants_pc34_compat"
               " failures=%d assertions=%d\n",
               g_failures, g_assertions);
        printf("Assertions: %d\n", g_assertions);
        printf("Failures: %d\n", g_failures);
        return 1;
    }

    printf("PASS test_dm1_v1_champion_panel_f0354_box_variants_pc34_compat"
           " failures=0 assertions=%d\n",
           g_assertions);
    printf("Assertions: %d\n", g_assertions);
    printf("Failures: %d\n", g_failures);
    return 0;
}
