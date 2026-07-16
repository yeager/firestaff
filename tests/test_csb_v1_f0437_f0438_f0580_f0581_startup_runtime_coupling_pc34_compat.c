#include "csb_v1_f0437_f0438_f0580_f0581_startup_runtime_coupling_pc34_compat.h"

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

static CSB_V1_StartupRuntimeCouplingFacts_PC34 make_complete_facts(void)
{
    CSB_V1_StartupRuntimeCouplingFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.real_startup_assets_bound = 1;
    facts.title_presents_runtime_captured = 1;
    facts.title_chaos_zoom_runtime_captured = 1;
    facts.title_chaos_hold_runtime_captured = 1;
    facts.title_strikes_back_runtime_captured = 1;
    facts.title_runtime_phase_mask =
        CSB_V1_STARTUP_RUNTIME_TITLE_ALL_PHASES_PC34;
    facts.title_runtime_phase_hash_count =
        CSB_V1_STARTUP_RUNTIME_TITLE_SAMPLE_COUNT_PC34;
    facts.title_runtime_unique_sample_hash_count =
        CSB_V1_STARTUP_RUNTIME_TITLE_SAMPLE_COUNT_PC34;
    facts.closed_door_hud_runtime_captured = 1;
    facts.utility_hud_runtime_captured = 1;
    facts.door_opening_delay_runtime_captured = 1;
    facts.door_opening_frame_runtime_captured = 1;
    facts.source_door_step_count = CSB_V1_STARTUP_RUNTIME_DOOR_STEP_COUNT_PC34;
    facts.door_step_index = 30;
    facts.draw_consumes_receipt_only = 1;
    facts.input_consumes_receipt_only = 1;
    facts.no_fallback_callbacks = 1;
    facts.no_wrapper_fallback_routes = 1;
    facts.no_legacy_door_fallback_route = 1;
    facts.no_synthetic_visuals = 1;
    return facts;
}

static void test_accepts_complete_runtime_coupling(void)
{
    CSB_V1_StartupRuntimeCouplingFacts_PC34 facts = make_complete_facts();
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 receipt;

    CHECK(F0437_STARTEND_DrawTitle(&facts, &receipt) == 1);
    CHECK((receipt.accepted_stage_mask &
           CSB_V1_STARTUP_RUNTIME_F0437_DRAW_TITLE_PC34) != 0u);
    CHECK(receipt.title_phase_route_complete == 1);
    CHECK(receipt.no_synthetic_visuals == 1);

    CHECK(F0438_STARTEND_OpenEntranceDoors(&facts, &receipt) == 1);
    CHECK((receipt.accepted_stage_mask &
           CSB_V1_STARTUP_RUNTIME_F0438_OPEN_ENTRANCE_DOORS_PC34) != 0u);
    CHECK(receipt.hud_runtime_coupled == 1);
    CHECK(receipt.door_runtime_coupled == 1);

    CHECK(F0580_ENTRANCE_DrawDoorAnimationStep(&facts, &receipt) == 1);
    CHECK((receipt.accepted_stage_mask &
           CSB_V1_STARTUP_RUNTIME_F0580_DRAW_DOOR_STEP_PC34) != 0u);
    CHECK(receipt.door_runtime_coupled == 1);

    CHECK(F0581_ENTRANCE_BlitDoors(&facts, &receipt) == 1);
    CHECK((receipt.accepted_stage_mask &
           CSB_V1_STARTUP_RUNTIME_F0581_BLIT_DOORS_PC34) != 0u);
    CHECK(receipt.draw_consumes_receipt_only == 1);
    CHECK(receipt.input_consumes_receipt_only == 1);
    CHECK(receipt.no_legacy_wrappers == 1);
}

static void test_rejects_incomplete_or_fallback_runtime_coupling(void)
{
    CSB_V1_StartupRuntimeCouplingFacts_PC34 facts = make_complete_facts();
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 receipt;

    facts.title_chaos_hold_runtime_captured = 0;
    CHECK(F0437_STARTEND_DrawTitle(&facts, &receipt) == 0);
    CHECK((receipt.rejected_stage_mask &
           CSB_V1_STARTUP_RUNTIME_F0437_DRAW_TITLE_PC34) != 0u);
    CHECK(receipt.no_synthetic_visuals == 1);

    facts = make_complete_facts();
    facts.no_wrapper_fallback_routes = 0;
    CHECK(F0438_STARTEND_OpenEntranceDoors(&facts, &receipt) == 0);
    CHECK((receipt.rejected_stage_mask &
           CSB_V1_STARTUP_RUNTIME_F0438_OPEN_ENTRANCE_DOORS_PC34) != 0u);

    facts = make_complete_facts();
    facts.door_step_index = CSB_V1_STARTUP_RUNTIME_DOOR_STEP_COUNT_PC34;
    CHECK(F0580_ENTRANCE_DrawDoorAnimationStep(&facts, &receipt) == 0);
    CHECK((receipt.rejected_stage_mask &
           CSB_V1_STARTUP_RUNTIME_F0580_DRAW_DOOR_STEP_PC34) != 0u);

    facts = make_complete_facts();
    facts.no_legacy_door_fallback_route = 0;
    CHECK(F0581_ENTRANCE_BlitDoors(&facts, &receipt) == 0);
    CHECK((receipt.rejected_stage_mask &
           CSB_V1_STARTUP_RUNTIME_F0581_BLIT_DOORS_PC34) != 0u);
}

static void test_evidence_strings(void)
{
    check_contains(csb_v1_f0437_startend_draw_title_source_evidence_pc34(),
                   "TITLE.C:424-463");
    check_contains(csb_v1_f0437_startend_draw_title_source_evidence_pc34(),
                   "PRESENTS, CHAOS");

    check_contains(csb_v1_f0438_startend_open_entrance_doors_source_evidence_pc34(),
                   "F0438/F0807");
    check_contains(csb_v1_f0438_startend_open_entrance_doors_source_evidence_pc34(),
                   "31-step door route");

    check_contains(csb_v1_f0580_entrance_draw_door_step_source_evidence_pc34(),
                   "F0580");
    check_contains(csb_v1_f0580_entrance_draw_door_step_source_evidence_pc34(),
                   "source-bounded step indexes");

    check_contains(csb_v1_f0581_entrance_blit_doors_source_evidence_pc34(),
                   "F0581");
    check_contains(csb_v1_f0581_entrance_blit_doors_source_evidence_pc34(),
                   "fallback door wrappers rejected");
}

int main(void)
{
    test_accepts_complete_runtime_coupling();
    test_rejects_incomplete_or_fallback_runtime_coupling();
    test_evidence_strings();
    return 0;
}
