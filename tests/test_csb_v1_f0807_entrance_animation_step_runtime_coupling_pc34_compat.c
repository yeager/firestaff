#include "csb_v1_f0807_entrance_animation_step_runtime_coupling_pc34_compat.h"

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

static CSB_V1_F0579_EntranceBitplanesReceipt_PC34 make_bitplanes(void)
{
    CSB_V1_F0579_EntranceBitplanesReceipt_PC34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.source_composite_bound = 1;
    receipt.target_screen_bound = 1;
    receipt.geometry_source_locked = 1;
    receipt.runtime_coupling_consumed = 1;
    receipt.title_hud_door_runtime_ready = 1;
    receipt.no_legacy_bitplane_wrapper = 1;
    receipt.no_synthetic_visuals = 1;
    return receipt;
}

static CSB_V1_F0807_EntranceAnimationStepFacts_PC34 make_complete_facts(void)
{
    CSB_V1_F0807_EntranceAnimationStepFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.animation_step_real_asset_bound = 1;
    facts.target_screen_real_asset_bound = 1;
    facts.source_door_step_count = CSB_V1_STARTUP_RUNTIME_DOOR_STEP_COUNT_PC34;
    facts.animation_step_index = CSB_V1_F0807_ENTRANCE_DOOR_STEP_LAST_PC34;
    facts.target_screen_width = CSB_V1_F0807_TARGET_SCREEN_WIDTH_PC34;
    facts.target_screen_height = CSB_V1_F0807_TARGET_SCREEN_HEIGHT_PC34;
    facts.blit_box_source_locked = 1;
    facts.no_transparency_mode = 1;
    facts.draw_consumes_receipt_only = 1;
    facts.input_consumes_receipt_only = 1;
    facts.no_legacy_animation_wrapper = 1;
    facts.no_synthetic_visuals = 1;
    facts.bitplanes = make_bitplanes();
    facts.runtime_coupling = make_runtime_coupling();
    return facts;
}

static void test_accepts_real_f0807_animation_step(void)
{
    CSB_V1_F0807_EntranceAnimationStepFacts_PC34 facts = make_complete_facts();
    CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 receipt;

    CHECK(F0807_ENTRANCE_DrawAnimationStep(&facts, &receipt) == 1);
    CHECK(receipt.valid == 1);
    CHECK(receipt.animation_step_bound == 1);
    CHECK(receipt.target_screen_bound == 1);
    CHECK(receipt.source_step_range_locked == 1);
    CHECK(receipt.bitplanes_consumed == 1);
    CHECK(receipt.runtime_coupling_consumed == 1);
    CHECK(receipt.draw_consumes_receipt_only == 1);
    CHECK(receipt.input_consumes_receipt_only == 1);
    CHECK(receipt.no_legacy_animation_wrapper == 1);
    CHECK(receipt.no_synthetic_visuals == 1);
    CHECK(receipt.accepted_animation_step_index ==
          CSB_V1_F0807_ENTRANCE_DOOR_STEP_LAST_PC34);
}

static void test_rejects_unbound_or_wrong_step_routes(void)
{
    CSB_V1_F0807_EntranceAnimationStepFacts_PC34 facts = make_complete_facts();
    CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 receipt;

    facts.bitplanes.valid = 0;
    CHECK(F0807_ENTRANCE_DrawAnimationStep(&facts, &receipt) == 0);
    CHECK(receipt.valid == 0);
    CHECK(receipt.no_synthetic_visuals == 1);

    facts = make_complete_facts();
    facts.animation_step_index = 0;
    CHECK(F0807_ENTRANCE_DrawAnimationStep(&facts, &receipt) == 0);
    CHECK(receipt.source_step_range_locked == 0);

    facts = make_complete_facts();
    facts.animation_step_index = CSB_V1_F0807_ENTRANCE_DOOR_STEP_LAST_PC34 + 1;
    CHECK(F0807_ENTRANCE_DrawAnimationStep(&facts, &receipt) == 0);

    facts = make_complete_facts();
    facts.animation_step_real_asset_bound = 0;
    CHECK(F0807_ENTRANCE_DrawAnimationStep(&facts, &receipt) == 0);

    facts = make_complete_facts();
    facts.no_legacy_animation_wrapper = 0;
    CHECK(F0807_ENTRANCE_DrawAnimationStep(&facts, &receipt) == 0);

    facts = make_complete_facts();
    facts.runtime_coupling.door_runtime_coupled = 0;
    CHECK(F0807_ENTRANCE_DrawAnimationStep(&facts, &receipt) == 0);
}

static void test_evidence_string(void)
{
    const char *evidence =
        csb_v1_f0807_entrance_draw_animation_step_source_evidence_pc34();
    check_contains(evidence, "ENTRANCE.C:85-90");
    check_contains(evidence, "F0807_ENTRANCE_DrawAnimationStep");
    check_contains(evidence, "G2219_puc_EntranceAnimationStep");
    check_contains(evidence, "CM1_COLOR_NO_TRANSPARENCY");
    check_contains(evidence, "F0579");
}

int main(void)
{
    test_accepts_real_f0807_animation_step();
    test_rejects_unbound_or_wrong_step_routes();
    test_evidence_string();
    return 0;
}
