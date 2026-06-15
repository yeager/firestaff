#include "csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return;
    }
    printf("ok %s=%d anchor=%s\n", label, got, anchor);
}

static void check_size(const char *label, size_t got, size_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%zu want=%zu anchor=%s\n", label, got, want, anchor);
        return;
    }
    printf("ok %s=%zu anchor=%s\n", label, got, anchor);
}

static void check_contains(
    const char *label,
    const char *haystack,
    const char *needle,
    const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
        return;
    }
    printf("ok %s contains=%s anchor=%s\n", label, needle, anchor);
}

static void test_contract_and_evidence(void)
{
    const CSB_V1_CustomBackgroundsMaskAfterFloorCeilingContract *contract =
        csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_contract_pc34();
    const char *evidence =
        csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_source_evidence_pc34();

    check_int("contract.present", contract != NULL, 1, "contract");
    if (!contract) {
        return;
    }

    check_int("contract.contract_only", contract->contract_only, 1,
              contract->source_summary);
    check_int("contract.step.f0098", contract->f0098_floor_ceiling_step,
              CSB_V1_MASK_AFTER_FLOOR_STEP_F0098_FLOOR_CEILING,
              contract->redmcsb_f0098_anchor);
    check_int("contract.step.reset", contract->g0297_reset_step,
              CSB_V1_MASK_AFTER_FLOOR_STEP_G0297_RESET,
              contract->redmcsb_f0098_anchor);
    check_int("contract.step.mask", contract->applybackground_mask_step,
              CSB_V1_MASK_AFTER_FLOOR_STEP_APPLYBACKGROUND_MASK,
              contract->csb_lineage_applybackground_anchor);
    check_int("contract.step.large", contract->room_bitmap_large_step,
              CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_LARGE,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.step.middle", contract->room_bitmap_middle_step,
              CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_MIDDLE,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.step.near", contract->room_bitmap_near_step,
              CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_NEAR,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.step.default", contract->state_default_step,
              CSB_V1_MASK_AFTER_FLOOR_STEP_STATE_DEFAULT,
              contract->redmcsb_f0098_anchor);
    check_int("contract.skin_def_min_words", contract->skin_def_min_words, 7,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.large_bitmap_index", contract->large_bitmap_skin_def_index, 0,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.large_mask_index", contract->large_mask_skin_def_index, 4,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.middle_bitmap_index", contract->middle_bitmap_skin_def_index, 2,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.middle_mask_index", contract->middle_mask_skin_def_index, 6,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.near_bitmap_index", contract->near_bitmap_skin_def_index, 1,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.near_mask_index", contract->near_mask_skin_def_index, 5,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.near_limit", contract->near_layer_room_num_limit, 5,
              contract->csb_lineage_bitmap_application_anchor);

    check_contains("evidence.f0128", evidence,
                   "F0128_DUNGEONVIEW_Draw_CPSF:8318-8542",
                   contract->redmcsb_f0128_anchor);
    check_contains("evidence.f0098", evidence,
                   "F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002",
                   contract->redmcsb_f0098_anchor);
    check_contains("evidence.floor", evidence, "G2108_Floor",
                   contract->redmcsb_f0098_anchor);
    check_contains("evidence.ceiling", evidence, "G2109_Ceiling",
                   contract->redmcsb_f0098_anchor);
    check_contains("evidence.g0297", evidence,
                   "G0297_B_DrawFloorAndCeilingRequested",
                   contract->redmcsb_f0098_anchor);
    check_contains("evidence.defs", evidence, "DEFS.H:2596-2614",
                   contract->redmcsb_defs_view_square_anchor);
    check_contains("evidence.applybackground", evidence, "Viewport.cpp:6451-6505",
                   contract->csb_lineage_applybackground_anchor);
    check_contains("evidence.bitmap_order", evidence, "Viewport.cpp:6599-6619",
                   contract->csb_lineage_bitmap_application_anchor);
    check_contains("evidence.url", evidence, contract->csbwin_viewport_url,
                   contract->csbwin_viewport_url);
}

static void test_declared_order(void)
{
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingStep steps[8];
    const CSB_V1_CustomBackgroundsMaskAfterFloorCeilingContract *contract =
        csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_contract_pc34();
    const size_t count =
        csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_order_pc34(
            steps, sizeof(steps) / sizeof(steps[0]));

    check_size("order.count", count, 6u, contract->source_summary);
    check_int("order.0.floor_ceiling", steps[0],
              CSB_V1_MASK_AFTER_FLOOR_STEP_F0098_FLOOR_CEILING,
              contract->redmcsb_f0098_anchor);
    check_int("order.1.reset", steps[1],
              CSB_V1_MASK_AFTER_FLOOR_STEP_G0297_RESET,
              contract->redmcsb_f0098_anchor);
    check_int("order.2.mask", steps[2],
              CSB_V1_MASK_AFTER_FLOOR_STEP_APPLYBACKGROUND_MASK,
              contract->csb_lineage_applybackground_anchor);
    check_int("order.3.large", steps[3],
              CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_LARGE,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("order.4.middle", steps[4],
              CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_MIDDLE,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("order.5.near", steps[5],
              CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_NEAR,
              contract->csb_lineage_bitmap_application_anchor);
}

static void test_room_bitmap_application_order(void)
{
    static const uint16_t skin_def[7] = {
        101, 202, 303, 0, 401, 502, 603
    };
    const CSB_V1_CustomBackgroundsMaskAfterFloorCeilingContract *contract =
        csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_contract_pc34();
    int room_num;

    for (room_num = 0; room_num < 16; ++room_num) {
        char label[96];
        const int near_expected = room_num < contract->near_layer_room_num_limit;
        CSB_V1_CustomBackgroundsMaskAfterFloorCeilingResult result;

        snprintf(label, sizeof(label), "room%02d.run", room_num);
        check_int(label,
                  csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_run_pc34(
                      room_num, 1, 1, skin_def, 7u, &result),
                  1, contract->source_summary);
        snprintf(label, sizeof(label), "room%02d.floor", room_num);
        check_int(label, result.floor_drawn, 1, contract->redmcsb_f0098_anchor);
        snprintf(label, sizeof(label), "room%02d.ceiling", room_num);
        check_int(label, result.ceiling_drawn, 1, contract->redmcsb_f0098_anchor);
        snprintf(label, sizeof(label), "room%02d.g0297_initial", room_num);
        check_int(label, result.g0297_initial_requested, 1,
                  contract->redmcsb_f0098_anchor);
        snprintf(label, sizeof(label), "room%02d.g0297_after_f0098", room_num);
        check_int(label, result.g0297_after_floor_ceiling, 0,
                  contract->redmcsb_f0098_anchor);
        snprintf(label, sizeof(label), "room%02d.g0297_before_mask", room_num);
        check_int(label, result.g0297_before_applybackground_mask, 0,
                  contract->csb_lineage_applybackground_anchor);
        snprintf(label, sizeof(label), "room%02d.g0297_after_bitmap", room_num);
        check_int(label, result.g0297_after_room_bitmap, 0,
                  contract->csb_lineage_bitmap_application_anchor);
        snprintf(label, sizeof(label), "room%02d.floor_before_mask", room_num);
        check_int(label, result.floor_ceiling_before_mask, 1,
                  contract->csb_lineage_applybackground_anchor);
        snprintf(label, sizeof(label), "room%02d.mask_before_bitmap", room_num);
        check_int(label, result.mask_before_room_bitmap, 1,
                  contract->csb_lineage_bitmap_application_anchor);
        snprintf(label, sizeof(label), "room%02d.reset_before_mask", room_num);
        check_int(label, result.reset_before_mask, 1,
                  contract->redmcsb_f0098_anchor);
        snprintf(label, sizeof(label), "room%02d.mask", room_num);
        check_int(label, result.applybackground_mask_applied, 1,
                  contract->csb_lineage_applybackground_anchor);
        snprintf(label, sizeof(label), "room%02d.apply_count", room_num);
        check_int(label, result.room_bitmap_apply_count, near_expected ? 3 : 2,
                  contract->csb_lineage_bitmap_application_anchor);

        snprintf(label, sizeof(label), "room%02d.order_count", room_num);
        check_size(label, result.order_count, near_expected ? 6u : 5u,
                   contract->csb_lineage_bitmap_application_anchor);
        snprintf(label, sizeof(label), "room%02d.order0", room_num);
        check_int(label, result.order[0],
                  CSB_V1_MASK_AFTER_FLOOR_STEP_F0098_FLOOR_CEILING,
                  contract->redmcsb_f0098_anchor);
        snprintf(label, sizeof(label), "room%02d.order1", room_num);
        check_int(label, result.order[1],
                  CSB_V1_MASK_AFTER_FLOOR_STEP_G0297_RESET,
                  contract->redmcsb_f0098_anchor);
        snprintf(label, sizeof(label), "room%02d.order2", room_num);
        check_int(label, result.order[2],
                  CSB_V1_MASK_AFTER_FLOOR_STEP_APPLYBACKGROUND_MASK,
                  contract->csb_lineage_applybackground_anchor);
        snprintf(label, sizeof(label), "room%02d.order3", room_num);
        check_int(label, result.order[3],
                  CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_LARGE,
                  contract->csb_lineage_bitmap_application_anchor);
        snprintf(label, sizeof(label), "room%02d.order4", room_num);
        check_int(label, result.order[4],
                  CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_MIDDLE,
                  contract->csb_lineage_bitmap_application_anchor);
        if (near_expected) {
            snprintf(label, sizeof(label), "room%02d.order5", room_num);
            check_int(label, result.order[5],
                      CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_NEAR,
                      contract->csb_lineage_bitmap_application_anchor);
        }

        snprintf(label, sizeof(label), "room%02d.large_bitmap_index", room_num);
        check_int(label, result.layers[0].bitmap_skin_def_index, 0,
                  result.layers[0].source_lines);
        snprintf(label, sizeof(label), "room%02d.large_mask_index", room_num);
        check_int(label, result.layers[0].mask_skin_def_index, 4,
                  result.layers[0].source_lines);
        snprintf(label, sizeof(label), "room%02d.large_bitmap_id", room_num);
        check_int(label, result.layers[0].bitmap_graphic_id, 101,
                  result.layers[0].source_lines);
        snprintf(label, sizeof(label), "room%02d.large_mask_id", room_num);
        check_int(label, result.layers[0].mask_graphic_id, 401,
                  result.layers[0].source_lines);
        snprintf(label, sizeof(label), "room%02d.middle_bitmap_index", room_num);
        check_int(label, result.layers[1].bitmap_skin_def_index, 2,
                  result.layers[1].source_lines);
        snprintf(label, sizeof(label), "room%02d.middle_mask_index", room_num);
        check_int(label, result.layers[1].mask_skin_def_index, 6,
                  result.layers[1].source_lines);
        snprintf(label, sizeof(label), "room%02d.middle_bitmap_id", room_num);
        check_int(label, result.layers[1].bitmap_graphic_id, 303,
                  result.layers[1].source_lines);
        snprintf(label, sizeof(label), "room%02d.middle_mask_id", room_num);
        check_int(label, result.layers[1].mask_graphic_id, 603,
                  result.layers[1].source_lines);
        snprintf(label, sizeof(label), "room%02d.near_bitmap_index", room_num);
        check_int(label, result.layers[2].bitmap_skin_def_index, 1,
                  result.layers[2].source_lines);
        snprintf(label, sizeof(label), "room%02d.near_mask_index", room_num);
        check_int(label, result.layers[2].mask_skin_def_index, 5,
                  result.layers[2].source_lines);
        snprintf(label, sizeof(label), "room%02d.near_bitmap_id", room_num);
        check_int(label, result.layers[2].bitmap_graphic_id, 202,
                  result.layers[2].source_lines);
        snprintf(label, sizeof(label), "room%02d.near_mask_id", room_num);
        check_int(label, result.layers[2].mask_graphic_id, 502,
                  result.layers[2].source_lines);
        snprintf(label, sizeof(label), "room%02d.near_applies", room_num);
        check_int(label, result.layers[2].applies_for_room_num, near_expected,
                  contract->csb_lineage_bitmap_application_anchor);
        snprintf(label, sizeof(label), "room%02d.near_applied", room_num);
        check_int(label, result.room_bitmap_near_applied, near_expected,
                  contract->csb_lineage_bitmap_application_anchor);
    }
}

static void test_mask_skipped_state_default(void)
{
    static const uint16_t skin_def[7] = {
        101, 202, 303, 0, 401, 502, 603
    };
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingResult result;
    const CSB_V1_CustomBackgroundsMaskAfterFloorCeilingContract *contract =
        csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_contract_pc34();

    check_int("default.run",
              csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_run_pc34(
                  0, 1, 0, skin_def, 7u, &result),
              1, contract->source_summary);
    check_int("default.floor", result.floor_drawn, 1,
              contract->redmcsb_f0098_anchor);
    check_int("default.ceiling", result.ceiling_drawn, 1,
              contract->redmcsb_f0098_anchor);
    check_int("default.state_default", result.state_default, 1,
              contract->redmcsb_f0098_anchor);
    check_int("default.mask_skipped", result.applybackground_mask_applied, 0,
              contract->csb_lineage_applybackground_anchor);
    check_int("default.no_room_bitmap", result.room_bitmap_apply_count, 0,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("default.g0297_after_f0098", result.g0297_after_floor_ceiling, 0,
              contract->redmcsb_f0098_anchor);
    check_int("default.g0297_before_mask", result.g0297_before_applybackground_mask, 0,
              contract->csb_lineage_applybackground_anchor);
    check_int("default.g0297_after_bitmap", result.g0297_after_room_bitmap, 0,
              contract->csb_lineage_bitmap_application_anchor);
    check_size("default.order_count", result.order_count, 3u,
               contract->source_summary);
    check_int("default.order0", result.order[0],
              CSB_V1_MASK_AFTER_FLOOR_STEP_F0098_FLOOR_CEILING,
              contract->redmcsb_f0098_anchor);
    check_int("default.order1", result.order[1],
              CSB_V1_MASK_AFTER_FLOOR_STEP_G0297_RESET,
              contract->redmcsb_f0098_anchor);
    check_int("default.order2", result.order[2],
              CSB_V1_MASK_AFTER_FLOOR_STEP_STATE_DEFAULT,
              contract->redmcsb_f0098_anchor);
}

int main(void)
{
    printf("probe=csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_source_evidence_pc34());

    test_contract_and_evidence();
    test_declared_order();
    test_room_bitmap_application_order();
    test_mask_skipped_state_default();

    printf("PASS assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
