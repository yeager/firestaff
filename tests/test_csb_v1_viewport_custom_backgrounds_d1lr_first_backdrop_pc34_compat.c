#include "firestaff/csb/v1/viewport/custom_backgrounds_d1lr_first_backdrop_pc34_compat.h"

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

static void check_u32(const char *label, uint32_t got, uint32_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%08x want=0x%08x anchor=%s\n",
               label, (unsigned int)got, (unsigned int)want, anchor);
        return;
    }
    printf("ok %s=0x%08x anchor=%s\n", label, (unsigned int)got, anchor);
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

static void set_skin(uint8_t *skins, int width, int x, int y, uint8_t skin)
{
    skins[(size_t)y * (size_t)width + (size_t)x] = skin;
}

static void test_contract_and_evidence(void)
{
    const CSB_V1_D1LRFirstBackdropContractPc34 *contract =
        csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_contract_pc34();
    const char *evidence =
        csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_source_evidence_pc34();

    check_int("contract.present", contract != NULL, 1, "contract");
    if (!contract) {
        return;
    }

    check_int("contract.contract_only", contract->contract_only, 1,
              contract->source_summary);
    check_int("contract.no_game_data", contract->no_game_data_dependency, 1,
              contract->source_summary);
    check_int("contract.no_real_assets", contract->no_real_asset_pixels, 1,
              contract->source_summary);
    check_int("contract.pair_count", contract->pair_count, 2,
              contract->csb_lineage_d1lr_dispatch_anchor);
    check_int("contract.skin_def_min", contract->skin_def_min_words, 7,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("contract.first_bitmap_index", contract->first_backdrop_bitmap_index, 0,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("contract.first_mask_index", contract->first_backdrop_mask_index, 4,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("contract.middle_bitmap_index", contract->middle_backdrop_bitmap_index, 2,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("contract.middle_mask_index", contract->middle_backdrop_mask_index, 6,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("contract.near_bitmap_index", contract->near_backdrop_bitmap_index, 1,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("contract.near_mask_index", contract->near_backdrop_mask_index, 5,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("contract.near_room_limit", contract->near_layer_room_limit, 5,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("contract.first_order", contract->first_backdrop_apply_order, 0,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("contract.middle_order", contract->middle_backdrop_apply_order, 1,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("contract.room_d1l", contract->d1l_room_num, 10,
              contract->csb_lineage_d1lr_dispatch_anchor);
    check_int("contract.room_d1r", contract->d1r_room_num, 11,
              contract->csb_lineage_d1lr_dispatch_anchor);
    check_int("contract.distinct_d0l2_d0r2",
              contract->distinct_from_d0l2_d0r2_first_backdrop, 1,
              contract->source_summary);
    check_int("contract.distinct_d0c",
              contract->distinct_from_d0c_first_backdrop, 1,
              contract->source_summary);
    check_int("contract.distinct_backdrop1",
              contract->distinct_from_room_slot_backdrop1, 1,
              contract->source_summary);

    check_contains("evidence.f0128", evidence, "F0128_DUNGEONVIEW_Draw_CPSF:8318-8542",
                   contract->redmcsb_f0128_anchor);
    check_contains("evidence.defs", evidence, "DEFS.H:2596-2614",
                   contract->redmcsb_defs_anchor);
    check_contains("evidence.relpos", evidence, "Viewport.cpp:5324-5337",
                   contract->csb_lineage_relpos_anchor);
    check_contains("evidence.applybackground", evidence, "Viewport.cpp:6451-6505",
                   contract->csb_lineage_applybackground_anchor);
    check_contains("evidence.custombackgrounds", evidence, "Viewport.cpp:6574-6622",
                   contract->csb_lineage_custom_backgrounds_anchor);
    check_contains("evidence.d1lr_dispatch", evidence, "Viewport.cpp:7050-7070",
                   contract->csb_lineage_d1lr_dispatch_anchor);
    check_contains("evidence.disjoint", evidence, "D0L2/D0R2 first-backdrop",
                   contract->source_summary);
}

static void test_pairs(void)
{
    const CSB_V1_D1LRFirstBackdropContractPc34 *contract =
        csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_contract_pc34();
    const CSB_V1_D1LRFirstBackdropPairPc34 *d1l =
        csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_for_room_pc34(10);
    const CSB_V1_D1LRFirstBackdropPairPc34 *d1r =
        csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_for_room_pc34(11);

    check_size("pair.count",
               csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_count_pc34(),
               2u, contract->csb_lineage_d1lr_dispatch_anchor);
    check_int("pair.0.d1l",
              csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_at_pc34(0) == d1l,
              1, contract->csb_lineage_d1lr_dispatch_anchor);
    check_int("pair.1.d1r",
              csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_at_pc34(1) == d1r,
              1, contract->csb_lineage_d1lr_dispatch_anchor);
    check_int("pair.2.null",
              csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_at_pc34(2) == NULL,
              1, contract->source_summary);
    check_int("pair.room9.null",
              csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_for_room_pc34(9) == NULL,
              1, "room 9 is D2C, not this D1L/D1R slice");
    check_int("d1l.side", d1l ? d1l->side : -1,
              CSB_V1_D1LR_FIRST_BACKDROP_SIDE_D1L,
              contract->csb_lineage_d1lr_dispatch_anchor);
    check_int("d1r.side", d1r ? d1r->side : -1,
              CSB_V1_D1LR_FIRST_BACKDROP_SIDE_D1R,
              contract->csb_lineage_d1lr_dispatch_anchor);
    check_int("d1l.room", d1l ? d1l->room_num : -1, 10,
              contract->csb_lineage_d1lr_dispatch_anchor);
    check_int("d1r.room", d1r ? d1r->room_num : -1, 11,
              contract->csb_lineage_d1lr_dispatch_anchor);
    check_int("d1l.view_square", d1l ? d1l->redmcsb_view_square : -1, 4,
              contract->redmcsb_defs_anchor);
    check_int("d1r.view_square", d1r ? d1r->redmcsb_view_square : -1, 5,
              contract->redmcsb_defs_anchor);
    check_int("d1l.forward", d1l ? d1l->relative_forward : -1, 1,
              contract->csb_lineage_relpos_anchor);
    check_int("d1r.forward", d1r ? d1r->relative_forward : -1, 1,
              contract->csb_lineage_relpos_anchor);
    check_int("d1l.side_rel", d1l ? d1l->relative_side : 0, -1,
              contract->csb_lineage_relpos_anchor);
    check_int("d1r.side_rel", d1r ? d1r->relative_side : 0, 1,
              contract->csb_lineage_relpos_anchor);
    check_int("d1l.f0128_depth", d1l ? d1l->f0128_depth : -1, 1,
              contract->redmcsb_f0128_anchor);
    check_int("d1r.f0128_lateral", d1r ? d1r->f0128_lateral : 0, 1,
              contract->redmcsb_f0128_anchor);
    check_int("d1l.before_body", d1l ? d1l->custom_backgrounds_before_square_body : 0,
              1, contract->csb_lineage_d1lr_dispatch_anchor);
    check_int("d1r.no_near", d1r ? d1r->near_layer_rejected_by_room_limit : 0,
              1, contract->csb_lineage_custom_backgrounds_anchor);
}

static void test_selection_and_composite(void)
{
    enum { width = 10, height = 10, party_x = 4, party_y = 6, facing = 0 };
    uint8_t skins[width * height];
    uint16_t skin_def[7] = { 101, 202, 303, 0, 401, 502, 603 };
    CSB_V1_D1LRFirstBackdropTracePc34 trace;
    const CSB_V1_D1LRFirstBackdropContractPc34 *contract =
        csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_contract_pc34();
    const CSB_V1_D1LRFirstBackdropPairPc34 *d1l =
        csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_for_room_pc34(10);
    const CSB_V1_D1LRFirstBackdropPairPc34 *d1r =
        csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_for_room_pc34(11);

    memset(skins, 0, sizeof(skins));
    set_skin(skins, width, 3, 5, 42);
    set_skin(skins, width, 5, 5, 43);

    check_int("select.d1l.call",
              csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_select_pc34(
                  d1l, skins, width, height, party_x, party_y, facing, 0,
                  skin_def, 7u, &trace),
              1, contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.x", trace.target_x, 3, contract->csb_lineage_relpos_anchor);
    check_int("select.d1l.y", trace.target_y, 5, contract->csb_lineage_relpos_anchor);
    check_int("select.d1l.skin", trace.selected_skin, 42,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.entry", trace.has_custom_background_entry, 1,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.first_bitmap", trace.first_bitmap_id, 101,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.first_mask", trace.first_mask_id, 401,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.middle_bitmap", trace.middle_bitmap_id, 303,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.middle_mask", trace.middle_mask_id, 603,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.near_bitmap", trace.near_bitmap_id, 202,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.near_mask", trace.near_mask_id, 502,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.first_applied", trace.first_backdrop_applied, 1,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.middle_after_first",
              trace.middle_backdrop_applied_after_first, 1,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.near_rejected", trace.near_backdrop_rejected, 1,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.first_order", trace.first_backdrop_apply_order, 0,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.middle_order", trace.middle_backdrop_apply_order, 1,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1l.near_order", trace.near_backdrop_apply_order, -1,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_u32("select.d1l.masked_sample", trace.masked_sample_after, 0x11bb33ddu,
              contract->csb_lineage_applybackground_anchor);

    check_int("select.d1r.call",
              csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_select_pc34(
                  d1r, skins, width, height, party_x, party_y, facing, 0,
                  skin_def, 7u, &trace),
              1, contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1r.x", trace.target_x, 5, contract->csb_lineage_relpos_anchor);
    check_int("select.d1r.y", trace.target_y, 5, contract->csb_lineage_relpos_anchor);
    check_int("select.d1r.skin", trace.selected_skin, 43,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.d1r.near_rejected", trace.near_backdrop_rejected, 1,
              contract->csb_lineage_custom_backgrounds_anchor);

    memset(skins, 0, sizeof(skins));
    check_int("select.default.call",
              csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_select_pc34(
                  d1l, skins, width, height, party_x, party_y, facing, 77,
                  skin_def, 7u, &trace),
              1, contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.default.used", trace.used_default_skin, 1,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.default.skin", trace.selected_skin, 77,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("select.null_pair",
              csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_select_pc34(
                  NULL, skins, width, height, party_x, party_y, facing, 0,
                  skin_def, 7u, &trace),
              0, contract->source_summary);
    check_u32("apply.word.direct",
              csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_apply_word_pc34(
                  0x11223344u, 0xaabbccddu, 0x00ffu),
              0x11bb33ddu, contract->csb_lineage_applybackground_anchor);
}

int main(void)
{
    test_contract_and_evidence();
    test_pairs();
    test_selection_and_composite();

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
