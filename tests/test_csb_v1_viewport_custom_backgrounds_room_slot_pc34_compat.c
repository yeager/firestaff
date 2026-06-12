#include "csb_v1_viewport_custom_backgrounds_room_slot_pc34_compat.h"

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

static void set_skin(uint8_t *skins, int width, int x, int y, uint8_t skin)
{
    skins[(size_t)y * (size_t)width + (size_t)x] = skin;
}

static int expected_x(int party_x, int facing, int forward, int side)
{
    static const int dx_fwd[4] = { 0, 1, 0, -1 };
    static const int dx_side[4] = { 1, 0, -1, 0 };

    return party_x + dx_side[facing] * side + dx_fwd[facing] * forward;
}

static int expected_y(int party_y, int facing, int forward, int side)
{
    static const int dy_fwd[4] = { -1, 0, 1, 0 };
    static const int dy_side[4] = { 0, 1, 0, -1 };

    return party_y + dy_side[facing] * side + dy_fwd[facing] * forward;
}

static void test_contract_and_evidence(void)
{
    const CSB_V1_CustomBackgroundsRoomSlotContract *contract =
        csb_v1_viewport_custom_backgrounds_room_slot_contract_pc34();
    const char *evidence =
        csb_v1_viewport_custom_backgrounds_room_slot_source_evidence_pc34();

    check_int("contract.present", contract != NULL, 1, "contract");
    if (!contract) {
        return;
    }
    check_int("contract.contract_only", contract->contract_only, 1,
              contract->redmcsb_viewport_anchor);
    check_int("contract.room_slot_count", contract->room_slot_count, 16,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("contract.first_backdrop_room", contract->first_backdrop_room_num, 0,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("contract.second_backdrop_room", contract->second_backdrop_room_num, 2,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("contract.skin_def_min_words", contract->skin_def_min_words, 7,
              contract->csb_lineage_custom_backgrounds_anchor);
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
    check_int("contract.near_room_limit", contract->near_layer_room_num_limit, 5,
              contract->csb_lineage_custom_backgrounds_anchor);

    check_contains("evidence.f0098", evidence,
                   "F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002",
                   contract->redmcsb_floor_ceiling_anchor);
    check_contains("evidence.f0128", evidence,
                   "F0128_DUNGEONVIEW_Draw_CPSF:8318-8542",
                   contract->redmcsb_viewport_anchor);
    check_contains("evidence.defs", evidence, "DEFS.H:2596-2614",
                   contract->redmcsb_defs_room_slot_anchor);
    check_contains("evidence.relpos", evidence, "Viewport.cpp:5324-5337",
                   contract->csb_lineage_relpos_anchor);
    check_contains("evidence.apply", evidence, "Viewport.cpp:6451-6505",
                   contract->csb_lineage_bitmap_application_anchor);
    check_contains("evidence.custombackgrounds", evidence, "Viewport.cpp:6574-6622",
                   contract->csb_lineage_custom_backgrounds_anchor);
    check_contains("evidence.dispatch", evidence, "Viewport.cpp:6926-7147",
                   contract->csb_lineage_room_dispatch_anchor);
}

static void test_room_slot_selection(void)
{
    enum { width = 10, height = 10, party_x = 4, party_y = 6, facing = 0 };
    uint8_t skins[width * height];
    size_t i;
    const CSB_V1_CustomBackgroundsRoomSlotContract *contract =
        csb_v1_viewport_custom_backgrounds_room_slot_contract_pc34();

    memset(skins, 0, sizeof(skins));
    for (i = 0; i < csb_v1_viewport_custom_backgrounds_room_slot_count_pc34(); ++i) {
        const CSB_V1_CustomBackgroundsRoomSlotSpec *slot =
            csb_v1_viewport_custom_backgrounds_room_slot_spec_pc34(i);
        if (slot) {
            const int x = expected_x(party_x, facing,
                                     slot->relative_forward,
                                     slot->relative_side);
            const int y = expected_y(party_y, facing,
                                     slot->relative_forward,
                                     slot->relative_side);
            set_skin(skins, width, x, y, (uint8_t)(70 + slot->room_num));
        }
    }

    check_size("slots.count",
               csb_v1_viewport_custom_backgrounds_room_slot_count_pc34(),
               16u, contract->csb_lineage_room_dispatch_anchor);

    for (i = 0; i < csb_v1_viewport_custom_backgrounds_room_slot_count_pc34(); ++i) {
        char label[96];
        CSB_V1_CustomBackgroundsRoomSlotSelection selection;
        const CSB_V1_CustomBackgroundsRoomSlotSpec *slot =
            csb_v1_viewport_custom_backgrounds_room_slot_spec_pc34(i);
        const int want_x = expected_x(party_x, facing,
                                      slot->relative_forward,
                                      slot->relative_side);
        const int want_y = expected_y(party_y, facing,
                                      slot->relative_forward,
                                      slot->relative_side);
        const int selected =
            csb_v1_viewport_custom_backgrounds_room_slot_select_pc34(
                skins, width, height, 6, party_x, party_y, facing,
                slot->room_num, 0, &selection);

        snprintf(label, sizeof(label), "slot%02d.select", slot->room_num);
        check_int(label, selected, 1, contract->csb_lineage_custom_backgrounds_anchor);
        snprintf(label, sizeof(label), "slot%02d.room", slot->room_num);
        check_int(label, selection.room_num, slot->room_num, slot->source_lines);
        snprintf(label, sizeof(label), "slot%02d.ordinal", slot->room_num);
        check_int(label, selection.room_slot_ordinal, slot->room_slot_ordinal,
                  contract->csb_lineage_room_dispatch_anchor);
        snprintf(label, sizeof(label), "slot%02d.defs_view_square", slot->room_num);
        check_int(label, selection.redmcsb_view_square_ordinal,
                  slot->redmcsb_view_square_ordinal,
                  contract->redmcsb_defs_room_slot_anchor);
        snprintf(label, sizeof(label), "slot%02d.forward", slot->room_num);
        check_int(label, selection.relative_forward, slot->relative_forward,
                  contract->csb_lineage_relpos_anchor);
        snprintf(label, sizeof(label), "slot%02d.side", slot->room_num);
        check_int(label, selection.relative_side, slot->relative_side,
                  contract->csb_lineage_relpos_anchor);
        snprintf(label, sizeof(label), "slot%02d.target_x", slot->room_num);
        check_int(label, selection.target_x, want_x,
                  contract->csb_lineage_relpos_anchor);
        snprintf(label, sizeof(label), "slot%02d.target_y", slot->room_num);
        check_int(label, selection.target_y, want_y,
                  contract->csb_lineage_relpos_anchor);
        snprintf(label, sizeof(label), "slot%02d.skin", slot->room_num);
        check_int(label, selection.selected_skin, 70 + slot->room_num,
                  contract->csb_lineage_custom_backgrounds_anchor);
        snprintf(label, sizeof(label), "slot%02d.entry", slot->room_num);
        check_int(label, selection.has_custom_background_entry, 1,
                  contract->csb_lineage_custom_backgrounds_anchor);
        snprintf(label, sizeof(label), "slot%02d.no_default", slot->room_num);
        check_int(label, selection.default_backdrop_selected, 0,
                  contract->redmcsb_floor_ceiling_anchor);
    }
}

static void test_bitmap_application_and_backdrop_mutation(void)
{
    uint8_t skins[100];
    uint16_t skin_def[7] = {
        101, 202, 303, 0, 401, 502, 603
    };
    CSB_V1_CustomBackgroundsRoomSlotSelection selection;
    CSB_V1_CustomBackgroundsViewportState state;
    CSB_V1_CustomBackgroundsBitmapApplication application;
    const CSB_V1_CustomBackgroundsRoomSlotContract *contract =
        csb_v1_viewport_custom_backgrounds_room_slot_contract_pc34();

    memset(skins, 0, sizeof(skins));
    memset(&state, 0, sizeof(state));
    state.first_backdrop_room_num = -1;
    state.second_backdrop_room_num = -1;
    set_skin(skins, 10, 2, 3, 11);
    set_skin(skins, 10, 3, 3, 22);

    check_int("first.select",
              csb_v1_viewport_custom_backgrounds_room_slot_select_pc34(
                  skins, 10, 10, 4, 4, 6, 0, 0, 0, &selection),
              1, contract->csb_lineage_custom_backgrounds_anchor);
    check_int("first.apply",
              csb_v1_viewport_custom_backgrounds_room_slot_apply_bitmap_pc34(
                  &selection, skin_def, 7u, &state, &application),
              1, contract->csb_lineage_bitmap_application_anchor);
    check_int("first.large_bitmap", application.large_bitmap_graphic_id, 101,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("first.large_mask", application.large_mask_graphic_id, 401,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("first.middle_bitmap", application.middle_bitmap_graphic_id, 303,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("first.middle_mask", application.middle_mask_graphic_id, 603,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("first.near_bitmap", application.near_bitmap_graphic_id, 202,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("first.near_mask", application.near_mask_graphic_id, 502,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("first.apply_calls", application.applybackground_call_count, 3,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("first.mutates_first", application.mutates_first_backdrop_selection, 1,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("first.mutates_second", application.mutates_second_backdrop_selection, 0,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("first.disjoint", application.keeps_both_backdrops_gate_disjoint, 1,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("state.first_room", state.first_backdrop_room_num, 0,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("state.first_skin", state.first_backdrop_skin, 11,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("state.second_untouched", state.second_backdrop_room_num, -1,
              contract->csb_lineage_room_dispatch_anchor);

    check_int("second.select",
              csb_v1_viewport_custom_backgrounds_room_slot_select_pc34(
                  skins, 10, 10, 4, 4, 6, 0, 2, 0, &selection),
              1, contract->csb_lineage_custom_backgrounds_anchor);
    check_int("second.apply",
              csb_v1_viewport_custom_backgrounds_room_slot_apply_bitmap_pc34(
                  &selection, skin_def, 7u, &state, &application),
              1, contract->csb_lineage_bitmap_application_anchor);
    check_int("second.apply_calls", application.applybackground_call_count, 3,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("second.mutates_first", application.mutates_first_backdrop_selection, 0,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("second.mutates_second", application.mutates_second_backdrop_selection, 1,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("second.disjoint", application.keeps_both_backdrops_gate_disjoint, 1,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("state.first_still", state.first_backdrop_room_num, 0,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("state.second_room", state.second_backdrop_room_num, 2,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("state.second_skin", state.second_backdrop_skin, 22,
              contract->csb_lineage_custom_backgrounds_anchor);

    check_int("far.select",
              csb_v1_viewport_custom_backgrounds_room_slot_select_pc34(
                  skins, 10, 10, 4, 4, 6, 0, 5, 99, &selection),
              1, contract->csb_lineage_custom_backgrounds_anchor);
    check_int("far.uses_default_skin", selection.used_default_skin, 1,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("far.apply",
              csb_v1_viewport_custom_backgrounds_room_slot_apply_bitmap_pc34(
                  &selection, skin_def, 7u, &state, &application),
              1, contract->csb_lineage_bitmap_application_anchor);
    check_int("far.no_near", application.near_applied, 0,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("far.apply_calls", application.applybackground_call_count, 2,
              contract->csb_lineage_bitmap_application_anchor);
}

static void test_negative_path(void)
{
    uint8_t skins[16];
    uint16_t skin_def[7] = {
        101, 202, 303, 0, 401, 502, 603
    };
    CSB_V1_CustomBackgroundsRoomSlotSelection selection;
    CSB_V1_CustomBackgroundsViewportState state;
    CSB_V1_CustomBackgroundsBitmapApplication application;
    const CSB_V1_CustomBackgroundsRoomSlotContract *contract =
        csb_v1_viewport_custom_backgrounds_room_slot_contract_pc34();

    memset(skins, 0, sizeof(skins));
    memset(&state, 0, sizeof(state));

    check_int("negative.select",
              csb_v1_viewport_custom_backgrounds_room_slot_select_pc34(
                  skins, 4, 4, 1, 1, 1, 0, 0, 0, &selection),
              1, contract->csb_lineage_custom_backgrounds_anchor);
    check_int("negative.no_entry", selection.has_custom_background_entry, 0,
              contract->csb_lineage_custom_backgrounds_anchor);
    check_int("negative.default_backdrop", selection.default_backdrop_selected, 1,
              contract->redmcsb_floor_ceiling_anchor);
    check_int("negative.apply",
              csb_v1_viewport_custom_backgrounds_room_slot_apply_bitmap_pc34(
                  &selection, skin_def, 7u, &state, &application),
              1, contract->csb_lineage_bitmap_application_anchor);
    check_int("negative.app_default_backdrop", application.default_backdrop_selected, 1,
              contract->redmcsb_floor_ceiling_anchor);
    check_int("negative.no_apply_calls", application.applybackground_call_count, 0,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("negative.state_default", state.default_backdrop_selected, 1,
              contract->redmcsb_floor_ceiling_anchor);
}

int main(void)
{
    printf("probe=csb_v1_viewport_custom_backgrounds_room_slot_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_custom_backgrounds_room_slot_source_evidence_pc34());

    test_contract_and_evidence();
    test_room_slot_selection();
    test_bitmap_application_and_backdrop_mutation();
    test_negative_path();

    printf("PASS assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
