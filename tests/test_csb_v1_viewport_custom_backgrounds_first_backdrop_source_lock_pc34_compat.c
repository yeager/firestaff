/* ReDMCSB source-lock anchors:
 * DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542, F0098:2962-3002,
 * F0107:3502-3938, and DEFS.H:2596-2614.
 * CSB-lineage anchors: Viewport.cpp:6451-6505 ApplyBackground and
 * Viewport.cpp:6599-6619 pSkinDef[0] first-backdrop bitmap application.
 */
#include "firestaff/csb/v1/viewport/custom_backgrounds_first_backdrop_pc34_compat.h"

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

static void check_hash(const char *label, uint64_t got, uint64_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%016llx want=0x%016llx anchor=%s\n",
               label, (unsigned long long)got, (unsigned long long)want, anchor);
        return;
    }
    printf("ok %s=0x%016llx anchor=%s\n",
           label, (unsigned long long)got, anchor);
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

static void set_skin(uint8_t *skins, int width, int x, int y, uint8_t skin)
{
    skins[(size_t)y * (size_t)width + (size_t)x] = skin;
}

static void test_contract_and_evidence(void)
{
    const CSB_V1_CustomBackgroundsFirstBackdropSourceLockContractPc34 *contract =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_contract_pc34();
    const char *evidence =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_evidence_pc34();

    check_int("contract.present", contract != NULL, 1, "contract");
    if (!contract) {
        return;
    }

    check_int("contract.contract_only", contract->contract_only, 1,
              contract->source_summary);
    check_int("contract.no_game_data", contract->no_game_data_dependency, 1,
              contract->source_summary);
    check_int("contract.no_gui", contract->no_gui_dependency, 1,
              contract->source_summary);
    check_int("contract.first_room", contract->first_backdrop_room_num, 0,
              contract->csb_lineage_pskindef_anchor);
    check_int("contract.second_room", contract->second_backdrop_room_num, 2,
              contract->csb_lineage_pskindef_anchor);
    check_int("contract.room0_forward", contract->room0_rel_forward, 3,
              contract->csb_lineage_pskindef_anchor);
    check_int("contract.room0_side", contract->room0_rel_side, -2,
              contract->csb_lineage_pskindef_anchor);
    check_int("contract.skin_def_min", contract->skin_def_min_words, 7,
              contract->csb_lineage_pskindef_anchor);
    check_int("contract.pskin0", contract->pskin_first_backdrop_index, 0,
              contract->csb_lineage_pskindef_anchor);
    check_int("contract.pskin4", contract->pskin_first_backdrop_mask_index, 4,
              contract->csb_lineage_pskindef_anchor);
    check_int("contract.pskin2", contract->pskin_middle_backdrop_index, 2,
              contract->csb_lineage_pskindef_anchor);
    check_int("contract.pskin1", contract->pskin_near_backdrop_index, 1,
              contract->csb_lineage_pskindef_anchor);
    check_int("contract.before_f0098", contract->first_backdrop_before_f0098_base, 1,
              contract->redmcsb_f0098_anchor);
    check_int("contract.pair_count", contract->d0l2_d0r2_pair_count, 2,
              contract->redmcsb_defs_anchor);
    check_int("contract.keepout_preserves",
              contract->f0107_mask_0x8000_keepout_preserves_destination, 1,
              contract->redmcsb_f0107_anchor);
    check_int("contract.keepout_no_erase",
              contract->f0107_keepout_does_not_erase_first_backdrop, 1,
              contract->redmcsb_f0107_anchor);
    check_int("contract.distinct_second", contract->distinct_from_second_backdrop_gate, 1,
              contract->source_summary);
    check_int("contract.distinct_room_slot", contract->distinct_from_room_slot_gate, 1,
              contract->source_summary);
    check_int("contract.distinct_mask_after_floor",
              contract->distinct_from_mask_after_floor_ceiling_gate, 1,
              contract->source_summary);

    check_contains("evidence.f0128", evidence,
                   "F0128_DUNGEONVIEW_Draw_CPSF:8318-8542",
                   contract->redmcsb_f0128_anchor);
    check_contains("evidence.f0098", evidence,
                   "F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002",
                   contract->redmcsb_f0098_anchor);
    check_contains("evidence.f0107", evidence, "F0107:3502-3938",
                   contract->redmcsb_f0107_anchor);
    check_contains("evidence.mask", evidence, "MASK 0x8000",
                   contract->redmcsb_f0107_anchor);
    check_contains("evidence.defs", evidence, "DEFS.H:2596-2614",
                   contract->redmcsb_defs_anchor);
    check_contains("evidence.applybackground", evidence, "Viewport.cpp:6451-6505",
                   contract->csb_lineage_applybackground_anchor);
    check_contains("evidence.pskindef", evidence, "Viewport.cpp:6599-6619",
                   contract->csb_lineage_pskindef_anchor);
}

static void test_order_and_pairs(void)
{
    CSB_V1_CustomBackgroundsFirstBackdropSourceLockStepPc34 steps[4];
    const CSB_V1_CustomBackgroundsFirstBackdropSourceLockContractPc34 *contract =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_contract_pc34();
    const size_t count =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_order_pc34(
            steps, sizeof(steps) / sizeof(steps[0]));
    const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 *left =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_for_side_pc34(1);
    const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 *right =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_for_side_pc34(2);

    check_size("order.count", count, 3u, contract->source_summary);
    check_int("order.0.pskindef0", steps[0],
              CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_ROOM0_PSKINDEF0,
              contract->csb_lineage_pskindef_anchor);
    check_int("order.1.f0098", steps[1],
              CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_F0098_D0L2_D0R2_BASE,
              contract->redmcsb_f0098_anchor);
    check_int("order.2.f0107", steps[2],
              CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_F0107_MASK_0X8000_KEEP_OUT,
              contract->redmcsb_f0107_anchor);
    check_size("pair.count",
               csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_count_pc34(),
               2u, contract->redmcsb_defs_anchor);
    check_int("pair.0.left",
              csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_pc34(0) == left,
              1, contract->redmcsb_defs_anchor);
    check_int("pair.1.right",
              csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_pc34(1) == right,
              1, contract->redmcsb_defs_anchor);
    check_int("pair.2.null",
              csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_pc34(2) == NULL,
              1, contract->redmcsb_defs_anchor);
    check_int("pair.bad_side.null",
              csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_for_side_pc34(3) == NULL,
              1, contract->redmcsb_defs_anchor);

    check_int("left.view_square", left ? left->view_square : -1, 8,
              contract->redmcsb_defs_anchor);
    check_int("right.view_square", right ? right->view_square : -1, 10,
              contract->redmcsb_defs_anchor);
    check_int("left.lane", left ? left->relative_lateral : 0, -2,
              contract->redmcsb_defs_anchor);
    check_int("right.lane", right ? right->relative_lateral : 0, 2,
              contract->redmcsb_defs_anchor);
    check_int("left.pskin0", left ? left->skin_def_bitmap_index : -1, 0,
              contract->csb_lineage_pskindef_anchor);
    check_int("left.pskin4", left ? left->skin_def_mask_index : -1, 4,
              contract->csb_lineage_pskindef_anchor);
    check_int("right.pskin0", right ? right->skin_def_bitmap_index : -1, 0,
              contract->csb_lineage_pskindef_anchor);
    check_int("right.pskin4", right ? right->skin_def_mask_index : -1, 4,
              contract->csb_lineage_pskindef_anchor);
}

static void test_room0_pskindef0_selection(void)
{
    enum { width = 10, height = 10, party_x = 4, party_y = 6, facing = 0 };
    uint8_t skins[width * height];
    const uint16_t skin_def[7] = { 101, 202, 303, 0, 401, 502, 603 };
    CSB_V1_CustomBackgroundsFirstBackdropSelectionPc34 selection;
    const CSB_V1_CustomBackgroundsFirstBackdropSourceLockContractPc34 *contract =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_contract_pc34();

    memset(skins, 0, sizeof(skins));
    set_skin(skins, width, 2, 3, 42);
    set_skin(skins, width, 3, 3, 99);

    check_int("select.room0.call",
              csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_select_pc34(
                  skins, width, height, 6, party_x, party_y, facing, 0, 0,
                  skin_def, 7u, &selection),
              1, contract->csb_lineage_pskindef_anchor);
    check_int("select.room0.target_x", selection.target_x, 2,
              contract->csb_lineage_pskindef_anchor);
    check_int("select.room0.target_y", selection.target_y, 3,
              contract->csb_lineage_pskindef_anchor);
    check_int("select.room0.skin", selection.selected_skin, 42,
              contract->csb_lineage_pskindef_anchor);
    check_int("select.room0.bitmap_index", selection.selected_pskin_bitmap_index, 0,
              contract->csb_lineage_pskindef_anchor);
    check_int("select.room0.mask_index", selection.selected_pskin_mask_index, 4,
              contract->csb_lineage_pskindef_anchor);
    check_int("select.room0.bitmap_id", selection.selected_bitmap_id, 101,
              contract->csb_lineage_pskindef_anchor);
    check_int("select.room0.mask_id", selection.selected_mask_id, 401,
              contract->csb_lineage_pskindef_anchor);
    check_int("select.room0.first", selection.selected_first_backdrop, 1,
              contract->csb_lineage_pskindef_anchor);
    check_int("select.room0.not_second_path", selection.rejected_second_backdrop_path, 1,
              contract->source_summary);

    check_int("select.room2.rejected",
              csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_select_pc34(
                  skins, width, height, 6, party_x, party_y, facing, 2, 0,
                  skin_def, 7u, &selection),
              0, contract->source_summary);
    check_int("select.room2.not_first", selection.selected_first_backdrop, 0,
              contract->source_summary);
    check_int("select.room2.second_path", selection.rejected_second_backdrop_path, 0,
              contract->source_summary);
}

static void test_run_for_pair(
    const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 *pair,
    uint64_t expected_hash)
{
    const uint16_t skin_def[7] = { 101, 202, 303, 0, 401, 502, 603 };
    uint8_t skins[100];
    CSB_V1_CustomBackgroundsFirstBackdropSelectionPc34 selection;
    CSB_V1_CustomBackgroundsFirstBackdropRunPc34 run;
    const CSB_V1_CustomBackgroundsFirstBackdropSourceLockContractPc34 *contract =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_contract_pc34();

    memset(skins, 0, sizeof(skins));
    set_skin(skins, 10, 2, 3, 42);
    (void)csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_select_pc34(
        skins, 10, 10, 6, 4, 6, 0, 0, 0, skin_def, 7u, &selection);

    check_int("run.call",
              csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_run_pc34(
                  pair, &selection, &run),
              1, contract->source_summary);
    check_int("run.ok", run.ok, 1, contract->source_summary);
    check_int("run.side", run.side, pair ? pair->side : -1,
              contract->redmcsb_defs_anchor);
    check_int("run.step_count", run.step_count, 3,
              contract->source_summary);
    check_int("run.step0", run.steps[0],
              CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_ROOM0_PSKINDEF0,
              contract->csb_lineage_pskindef_anchor);
    check_int("run.step1", run.steps[1],
              CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_F0098_D0L2_D0R2_BASE,
              contract->redmcsb_f0098_anchor);
    check_int("run.step2", run.steps[2],
              CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_F0107_MASK_0X8000_KEEP_OUT,
              contract->redmcsb_f0107_anchor);
    check_int("run.pskindef0_applied", run.room0_pskindef0_applied, 1,
              contract->csb_lineage_pskindef_anchor);
    check_int("run.pskindef0_before_f0098", run.room0_pskindef0_before_f0098, 1,
              contract->redmcsb_f0098_anchor);
    check_int("run.not_second", run.room2_second_backdrop_not_used, 1,
              contract->source_summary);
    check_int("run.first_before_f0098_pixel", run.first_backdrop_pixel_before_f0098, 31,
              contract->csb_lineage_applybackground_anchor);
    check_int("run.base_outside_keepout", run.f0098_pixel_outside_keepout,
              pair ? pair->f0098_base_color : -1,
              contract->redmcsb_f0098_anchor);
    check_int("run.before_keepout", run.pixel_before_f0107_keepout, 31,
              contract->redmcsb_f0107_anchor);
    check_int("run.after_keepout", run.pixel_after_f0107_keepout, 31,
              contract->redmcsb_f0107_anchor);
    check_int("run.final_first", run.final_first_backdrop_pixel, 31,
              contract->redmcsb_f0107_anchor);
    check_int("run.final_base", run.final_f0098_base_pixel,
              pair ? pair->f0098_base_color : -1,
              contract->redmcsb_f0098_anchor);
    check_int("run.final_f0107", run.final_f0107_opaque_pixel, 47,
              contract->redmcsb_f0107_anchor);
    check_int("run.base_after_first", run.f0098_base_after_first_backdrop, 1,
              contract->redmcsb_f0098_anchor);
    check_int("run.keepout_after_base", run.f0107_keepout_after_f0098, 1,
              contract->redmcsb_f0107_anchor);
    check_int("run.keepout_preserved", run.f0107_keepout_preserved_first_backdrop, 1,
              contract->redmcsb_f0107_anchor);
    check_hash("run.content_hash", run.content_hash, expected_hash,
               contract->source_summary);
    check_hash("run.rehash",
               csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_hash_pc34(&run),
               expected_hash, contract->source_summary);
}

int main(void)
{
    const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 *left =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_for_side_pc34(1);
    const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 *right =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_for_side_pc34(2);

    printf("probe=csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_evidence_pc34());

    test_contract_and_evidence();
    test_order_and_pairs();
    test_room0_pskindef0_selection();
    test_run_for_pair(left, UINT64_C(0x8ef45632d4c5fdbc));
    test_run_for_pair(right, UINT64_C(0x1bd24687c985294f));

    printf("PASS assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
