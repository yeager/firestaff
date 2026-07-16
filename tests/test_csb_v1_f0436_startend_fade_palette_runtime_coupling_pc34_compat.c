#include "csb_v1_f0436_startend_fade_palette_runtime_coupling_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static CSB_V1_StartupRuntimeCouplingReceipt_PC34 make_runtime_coupling(void)
{
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.real_startup_assets_bound = 1;
    receipt.title_phase_route_complete = 1;
    receipt.hud_runtime_coupled = 1;
    receipt.door_runtime_coupled = 1;
    receipt.draw_consumes_receipt_only = 1;
    receipt.input_consumes_receipt_only = 1;
    receipt.no_legacy_wrappers = 1;
    receipt.no_synthetic_visuals = 1;
    return receipt;
}

static CSB_V1_StartEndEntranceBoundaryReceipt_PC34 make_entrance_boundary(
    uint32_t stage)
{
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.accepted_stage_mask = stage;
    receipt.real_asset_matched = 1;
    receipt.host_view_consumed = 1;
    receipt.host_draw_consumed = 1;
    receipt.host_input_consumed = 1;
    receipt.draw_consumes_receipt_only = 1;
    receipt.input_consumes_receipt_only = 1;
    receipt.no_synthetic_payloads = 1;
    receipt.no_fallback_graphics = 1;
    receipt.route_wrappers_retired = 1;
    return receipt;
}

static CSB_V1_F0436_FadePaletteFacts_PC34 make_common_facts(void)
{
    CSB_V1_F0436_FadePaletteFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.target_palette_real_asset_bound = 1;
    facts.palette_entry_count = CSB_V1_F0436_PALETTE_ENTRY_COUNT_PC34;
    facts.fade_step_count = CSB_V1_F0436_AMIGA_FADE_STEP_COUNT_PC34;
    facts.component_masks_source_locked = 1;
    facts.vertical_blank_synchronized = 1;
    facts.no_renderer_palette_substitute = 1;
    facts.no_legacy_palette_wrapper = 1;
    facts.no_synthetic_palette = 1;
    facts.runtime_coupling = make_runtime_coupling();
    facts.entrance_boundary =
        make_entrance_boundary(CSB_V1_STARTEND_F0439_DRAW_ENTRANCE_PC34);
    return facts;
}

static void test_accepts_title_palette_route(void)
{
    CSB_V1_F0436_FadePaletteFacts_PC34 facts = make_common_facts();
    CSB_V1_F0436_FadePaletteReceipt_PC34 receipt;

    facts.route_mask = CSB_V1_F0436_ROUTE_TITLE_PRESENTS_PC34 |
        CSB_V1_F0436_ROUTE_TITLE_CHAOS_PC34 |
        CSB_V1_F0436_ROUTE_TITLE_STRIKES_BACK_PC34;
    facts.title_palette_route = 1;
    facts.entrance_palette_route = 0;

    CHECK(F0436_STARTEND_FadeToPalette(&facts, &receipt) == 1);
    CHECK(receipt.valid == 1);
    CHECK(receipt.accepted_route_mask == facts.route_mask);
    CHECK(receipt.title_runtime_consumed == 1);
    CHECK(receipt.entrance_boundary_consumed == 0);
    CHECK(receipt.palette_entry_count_source_locked == 1);
    CHECK(receipt.fade_step_count_source_locked == 1);
    CHECK(receipt.no_renderer_palette_substitute == 1);
    CHECK(receipt.no_synthetic_palette == 1);
}

static void test_accepts_entrance_and_credits_palette_routes(void)
{
    CSB_V1_F0436_FadePaletteFacts_PC34 facts = make_common_facts();
    CSB_V1_F0436_FadePaletteReceipt_PC34 receipt;

    facts.route_mask = CSB_V1_F0436_ROUTE_ENTRANCE_SCREEN_PC34;
    facts.title_palette_route = 0;
    facts.entrance_palette_route = 1;
    facts.fade_step_count = CSB_V1_F0436_GENERIC_FADE_STEP_COUNT_PC34;
    CHECK(F0436_STARTEND_FadeToPalette(&facts, &receipt) == 1);
    CHECK(receipt.entrance_boundary_consumed == 1);
    CHECK(receipt.title_runtime_consumed == 0);

    facts = make_common_facts();
    facts.route_mask = CSB_V1_F0436_ROUTE_ENTRANCE_CREDITS_PC34;
    facts.entrance_palette_route = 1;
    facts.credits_palette_route = 1;
    facts.entrance_boundary =
        make_entrance_boundary(CSB_V1_STARTEND_F0442_DRAW_CREDITS_PC34);
    CHECK(F0436_STARTEND_FadeToPalette(&facts, &receipt) == 1);
    CHECK(receipt.accepted_route_mask == CSB_V1_F0436_ROUTE_ENTRANCE_CREDITS_PC34);
}

static void test_rejects_missing_real_route_or_synthetic_palette(void)
{
    CSB_V1_F0436_FadePaletteFacts_PC34 facts = make_common_facts();
    CSB_V1_F0436_FadePaletteReceipt_PC34 receipt;

    facts.route_mask = CSB_V1_F0436_ROUTE_TITLE_PRESENTS_PC34;
    facts.title_palette_route = 1;
    facts.runtime_coupling.title_phase_route_complete = 0;
    CHECK(F0436_STARTEND_FadeToPalette(&facts, &receipt) == 0);
    CHECK(receipt.valid == 0);
    CHECK(receipt.no_synthetic_palette == 1);

    facts = make_common_facts();
    facts.route_mask = CSB_V1_F0436_ROUTE_ENTRANCE_SCREEN_PC34;
    facts.entrance_palette_route = 1;
    facts.entrance_boundary.real_asset_matched = 0;
    CHECK(F0436_STARTEND_FadeToPalette(&facts, &receipt) == 0);

    facts = make_common_facts();
    facts.route_mask = CSB_V1_F0436_ROUTE_TITLE_CHAOS_PC34;
    facts.title_palette_route = 1;
    facts.palette_entry_count = 15;
    CHECK(F0436_STARTEND_FadeToPalette(&facts, &receipt) == 0);

    facts = make_common_facts();
    facts.route_mask = CSB_V1_F0436_ROUTE_TITLE_CHAOS_PC34;
    facts.title_palette_route = 1;
    facts.no_renderer_palette_substitute = 0;
    CHECK(F0436_STARTEND_FadeToPalette(&facts, &receipt) == 0);

    facts = make_common_facts();
    facts.route_mask = CSB_V1_F0436_ROUTE_TITLE_CHAOS_PC34;
    facts.title_palette_route = 1;
    facts.no_synthetic_palette = 0;
    CHECK(F0436_STARTEND_FadeToPalette(&facts, &receipt) == 0);
}

static void test_evidence_string(void)
{
    const char *evidence =
        csb_v1_f0436_startend_fade_to_palette_source_evidence_pc34();
    check_contains(evidence, "PALETTE.C:209-395");
    check_contains(evidence, "F0436_STARTEND_FadeToPalette");
    check_contains(evidence, "PRESENTS/CHAOS/STRIKES");
    check_contains(evidence, "ENTRANCE.C");
    check_contains(evidence, "16 source palette entries");
}

int main(void)
{
    test_accepts_title_palette_route();
    test_accepts_entrance_and_credits_palette_routes();
    test_rejects_missing_real_route_or_synthetic_palette();
    test_evidence_string();
    return 0;
}
