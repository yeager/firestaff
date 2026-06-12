#include "firestaff/csb/v1/viewport/csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_pc34_compat.h"

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

static void check_u32(const char *label, uint32_t got, uint32_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%08x want=0x%08x anchor=%s\n",
               label, got, want, anchor);
        return;
    }
    printf("ok %s=0x%08x anchor=%s\n", label, got, anchor);
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
    const CSB_V1_CustomBackgroundsRoomSlotBackdrop1Contract *contract =
        csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_contract_pc34();
    const char *evidence =
        csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_source_evidence_pc34();

    check_int("contract.present", contract != NULL, 1, "contract");
    if (!contract) {
        return;
    }

    check_int("contract.contract_only", contract->contract_only, 1,
              contract->source_summary);
    check_int("contract.room_slot_count", contract->room_slot_count, 16,
              contract->redmcsb_defs_anchor);
    check_int("contract.skin_def_min_words", contract->skin_def_min_words, 7,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.backdrop0_bitmap_index",
              contract->backdrop0_bitmap_skin_def_index, 0,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.backdrop0_mask_index",
              contract->backdrop0_mask_skin_def_index, 4,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.backdrop1_bitmap_index",
              contract->backdrop1_bitmap_skin_def_index, 1,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.backdrop1_mask_index",
              contract->backdrop1_mask_skin_def_index, 5,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.backdrop1_bitmap_size",
              contract->backdrop1_bitmap_size_words, 4144,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.backdrop1_mask_height",
              contract->backdrop1_mask_height, 20,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.backdrop1_room_limit",
              contract->backdrop1_room_num_limit, 5,
              contract->csb_lineage_bitmap_application_anchor);
    check_int("contract.backdrop1_after_backdrop0",
              contract->backdrop1_after_backdrop0, 1,
              contract->csb_lineage_applybackground_anchor);
    check_int("contract.backdrop1_after_f0107",
              contract->backdrop1_after_f0107_keepout, 1,
              contract->redmcsb_f0107_anchor);
    check_int("contract.reuses_room_slot_selector",
              contract->reuses_room_slot_selector, 1,
              contract->redmcsb_defs_anchor);

    check_contains("evidence.f0128", evidence,
                   "F0128_DUNGEONVIEW_Draw_CPSF:8318-8542",
                   contract->redmcsb_f0128_anchor);
    check_contains("evidence.f0098", evidence,
                   "F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002",
                   contract->redmcsb_f0098_anchor);
    check_contains("evidence.f0107", evidence,
                   "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF:3502-3938",
                   contract->redmcsb_f0107_anchor);
    check_contains("evidence.defs", evidence, "DEFS.H:2596-2614",
                   contract->redmcsb_defs_anchor);
    check_contains("evidence.dungeon_f0163", evidence, "F0163:1769-1838",
                   contract->redmcsb_dungeon_anchor);
    check_contains("evidence.dungeon_f0164", evidence, "F0164:1840-1905",
                   contract->redmcsb_dungeon_anchor);
    check_contains("evidence.dungeon_f0172", evidence, "F0172:2466-2523",
                   contract->redmcsb_dungeon_anchor);
    check_contains("evidence.applybackground", evidence, "Viewport.cpp:6451-6505",
                   contract->csb_lineage_applybackground_anchor);
    check_contains("evidence.pskindef1", evidence, "pSkinDef[1]/[5]",
                   contract->csb_lineage_bitmap_application_anchor);
    check_contains("disjoint.room_slot_backdrop1", contract->disjointness_note,
                   "pSkinDef[1]/[5] backdrop1",
                   contract->disjointness_note);
}

static void test_trace(void)
{
    CSB_V1_CustomBackgroundsRoomSlotBackdrop1Trace trace[16];
    const CSB_V1_CustomBackgroundsRoomSlotBackdrop1Contract *contract =
        csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_contract_pc34();
    const size_t count =
        csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_trace_pc34(
            trace, sizeof(trace) / sizeof(trace[0]));
    const uint32_t hash =
        csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_hash_pc34(trace, count);
    size_t i;
    int applied_count = 0;

    check_size("trace.count", count, 16u, contract->redmcsb_defs_anchor);
    check_u32("trace.hash", hash, contract->expected_trace_hash,
              contract->source_summary);

    for (i = 0; i < count; ++i) {
        char label[128];
        const CSB_V1_CustomBackgroundsRoomSlotBackdrop1Trace *row = &trace[i];
        const int near_expected = row->room_num < contract->backdrop1_room_num_limit;

        snprintf(label, sizeof(label), "room%02d.lookup_same_slot", row->room_num);
        check_int(label, row->room_lookup_used_same_slot_table, 1,
                  contract->redmcsb_defs_anchor);
        snprintf(label, sizeof(label), "room%02d.selected_skin", row->room_num);
        check_int(label, row->selected_skin, 120 + row->room_num,
                  contract->csb_lineage_bitmap_application_anchor);
        snprintf(label, sizeof(label), "room%02d.backdrop0_bitmap", row->room_num);
        check_int(label, row->backdrop0_bitmap_graphic_id, 101,
                  row->source_lines);
        snprintf(label, sizeof(label), "room%02d.backdrop0_mask", row->room_num);
        check_int(label, row->backdrop0_mask_graphic_id, 401,
                  row->source_lines);
        snprintf(label, sizeof(label), "room%02d.backdrop1_bitmap", row->room_num);
        check_int(label, row->backdrop1_bitmap_graphic_id, 202,
                  row->source_lines);
        snprintf(label, sizeof(label), "room%02d.backdrop1_mask", row->room_num);
        check_int(label, row->backdrop1_mask_graphic_id, 502,
                  row->source_lines);
        snprintf(label, sizeof(label), "room%02d.backdrop0_apply_ordinal", row->room_num);
        check_int(label, row->backdrop0_applybackground_ordinal, 0,
                  contract->csb_lineage_applybackground_anchor);
        snprintf(label, sizeof(label), "room%02d.middle_apply_ordinal", row->room_num);
        check_int(label, row->middle_applybackground_ordinal, 1,
                  contract->csb_lineage_bitmap_application_anchor);
        snprintf(label, sizeof(label), "room%02d.backdrop1_apply_ordinal", row->room_num);
        check_int(label, row->backdrop1_applybackground_ordinal, 2,
                  contract->csb_lineage_bitmap_application_anchor);
        snprintf(label, sizeof(label), "room%02d.f0107_order", row->room_num);
        check_int(label, row->f0107_keepout_order, 2,
                  contract->redmcsb_f0107_anchor);
        snprintf(label, sizeof(label), "room%02d.backdrop0_composite_order", row->room_num);
        check_int(label, row->backdrop0_composite_order, 3,
                  contract->csb_lineage_applybackground_anchor);
        snprintf(label, sizeof(label), "room%02d.backdrop1_composite_order", row->room_num);
        check_int(label, row->backdrop1_composite_order, 5,
                  contract->csb_lineage_applybackground_anchor);
        snprintf(label, sizeof(label), "room%02d.backdrop1_after_backdrop0", row->room_num);
        check_int(label, row->backdrop1_after_backdrop0, 1,
                  contract->csb_lineage_applybackground_anchor);
        snprintf(label, sizeof(label), "room%02d.backdrop1_after_f0107", row->room_num);
        check_int(label, row->backdrop1_after_f0107_keepout, 1,
                  contract->redmcsb_f0107_anchor);
        snprintf(label, sizeof(label), "room%02d.room_gate", row->room_num);
        check_int(label, row->backdrop1_room_gate_allows_apply, near_expected,
                  contract->csb_lineage_bitmap_application_anchor);
        snprintf(label, sizeof(label), "room%02d.backdrop1_applied", row->room_num);
        check_int(label, row->backdrop1_applied, near_expected,
                  contract->csb_lineage_bitmap_application_anchor);
        snprintf(label, sizeof(label), "room%02d.bitmap_size", row->room_num);
        check_int(label, row->backdrop1_bitmap_size_words, 4144,
                  contract->csb_lineage_bitmap_application_anchor);
        snprintf(label, sizeof(label), "room%02d.mask_height", row->room_num);
        check_int(label, row->backdrop1_mask_height, 20,
                  contract->csb_lineage_bitmap_application_anchor);

        applied_count += row->backdrop1_applied;
    }

    check_int("trace.backdrop1_applied_room_count", applied_count, 5,
              contract->csb_lineage_bitmap_application_anchor);
}

int main(void)
{
    const CSB_V1_CustomBackgroundsRoomSlotBackdrop1Contract *contract =
        csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_contract_pc34();
    CSB_V1_CustomBackgroundsRoomSlotBackdrop1Trace trace[16];
    const size_t count =
        csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_trace_pc34(
            trace, sizeof(trace) / sizeof(trace[0]));
    const uint32_t hash =
        csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_hash_pc34(trace, count);

    printf("probe=csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_source_evidence_pc34());

    test_contract_and_evidence();
    test_trace();

    if (g_failures == 0) {
        printf("CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_ROOM_SLOT_BACKDROP1_PC34_COMPAT_OK "
               "assertions=%d failures=0 deterministic_hash=0x%08x\n",
               g_assertions, hash);
    } else {
        printf("CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_ROOM_SLOT_BACKDROP1_PC34_COMPAT_FAIL "
               "assertions=%d failures=%d deterministic_hash=0x%08x expected=0x%08x\n",
               g_assertions,
               g_failures,
               hash,
               contract ? contract->expected_trace_hash : 0u);
    }

    return g_failures == 0 ? 0 : 1;
}
