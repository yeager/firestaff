#include "csb_v1_f0437_f0438_f0580_f0581_startup_runtime_coupling_pc34_compat.h"

#include <string.h>

static int csb_v1_startup_runtime_common_ready_pc34(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts)
{
    return facts && facts->valid && facts->real_startup_assets_bound &&
        facts->draw_consumes_receipt_only && facts->input_consumes_receipt_only &&
        facts->no_fallback_callbacks && facts->no_wrapper_fallback_routes &&
        facts->no_synthetic_visuals;
}

static int csb_v1_startup_runtime_title_ready_pc34(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts)
{
    return csb_v1_startup_runtime_common_ready_pc34(facts) &&
        facts->title_presents_runtime_captured &&
        facts->title_chaos_zoom_runtime_captured &&
        facts->title_chaos_hold_runtime_captured &&
        facts->title_strikes_back_runtime_captured &&
        facts->title_runtime_phase_mask ==
            CSB_V1_STARTUP_RUNTIME_TITLE_ALL_PHASES_PC34 &&
        facts->title_runtime_phase_hash_count ==
            CSB_V1_STARTUP_RUNTIME_TITLE_SAMPLE_COUNT_PC34 &&
        facts->title_runtime_unique_sample_hash_count ==
            CSB_V1_STARTUP_RUNTIME_TITLE_SAMPLE_COUNT_PC34;
}

static int csb_v1_startup_runtime_hud_ready_pc34(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts)
{
    return facts->closed_door_hud_runtime_captured &&
        facts->utility_hud_runtime_captured;
}

static int csb_v1_startup_runtime_door_ready_pc34(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts)
{
    return facts->door_opening_delay_runtime_captured &&
        facts->door_opening_frame_runtime_captured &&
        facts->source_door_step_count ==
            CSB_V1_STARTUP_RUNTIME_DOOR_STEP_COUNT_PC34 &&
        facts->door_step_index >= 0 &&
        facts->door_step_index < CSB_V1_STARTUP_RUNTIME_DOOR_STEP_COUNT_PC34 &&
        facts->no_legacy_door_fallback_route;
}

static void csb_v1_startup_runtime_mark_rejected_pc34(
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *receipt, uint32_t stage,
    const char *evidence)
{
    if (!receipt) return;
    receipt->rejected_stage_mask |= stage;
    receipt->no_synthetic_visuals = 1;
    receipt->source_evidence = evidence;
}

static void csb_v1_startup_runtime_mark_accepted_pc34(
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *receipt, uint32_t stage,
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts,
    const char *evidence)
{
    if (!receipt || !facts) return;
    receipt->valid = 1;
    receipt->accepted_stage_mask |= stage;
    receipt->real_startup_assets_bound = 1;
    receipt->title_phase_route_complete =
        csb_v1_startup_runtime_title_ready_pc34(facts) ? 1 : 0;
    receipt->hud_runtime_coupled =
        csb_v1_startup_runtime_hud_ready_pc34(facts) ? 1 : 0;
    receipt->door_runtime_coupled =
        csb_v1_startup_runtime_door_ready_pc34(facts) ? 1 : 0;
    receipt->draw_consumes_receipt_only = 1;
    receipt->input_consumes_receipt_only = 1;
    receipt->no_legacy_wrappers = 1;
    receipt->no_synthetic_visuals = 1;
    receipt->source_evidence = evidence;
}

void csb_v1_startup_runtime_coupling_receipt_init_pc34(
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int F0437_STARTEND_DrawTitle(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts,
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0437_startend_draw_title_source_evidence_pc34();
    csb_v1_startup_runtime_coupling_receipt_init_pc34(out_receipt);
    if (!csb_v1_startup_runtime_title_ready_pc34(facts)) {
        csb_v1_startup_runtime_mark_rejected_pc34(
            out_receipt, CSB_V1_STARTUP_RUNTIME_F0437_DRAW_TITLE_PC34,
            evidence);
        return 0;
    }
    csb_v1_startup_runtime_mark_accepted_pc34(
        out_receipt, CSB_V1_STARTUP_RUNTIME_F0437_DRAW_TITLE_PC34, facts,
        evidence);
    return 1;
}

int F0438_STARTEND_OpenEntranceDoors(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts,
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0438_startend_open_entrance_doors_source_evidence_pc34();
    csb_v1_startup_runtime_coupling_receipt_init_pc34(out_receipt);
    if (!csb_v1_startup_runtime_common_ready_pc34(facts) ||
        !csb_v1_startup_runtime_hud_ready_pc34(facts) ||
        !csb_v1_startup_runtime_door_ready_pc34(facts)) {
        csb_v1_startup_runtime_mark_rejected_pc34(
            out_receipt,
            CSB_V1_STARTUP_RUNTIME_F0438_OPEN_ENTRANCE_DOORS_PC34,
            evidence);
        return 0;
    }
    csb_v1_startup_runtime_mark_accepted_pc34(
        out_receipt,
        CSB_V1_STARTUP_RUNTIME_F0438_OPEN_ENTRANCE_DOORS_PC34, facts,
        evidence);
    return 1;
}

int F0580_ENTRANCE_DrawDoorAnimationStep(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts,
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0580_entrance_draw_door_step_source_evidence_pc34();
    csb_v1_startup_runtime_coupling_receipt_init_pc34(out_receipt);
    if (!csb_v1_startup_runtime_common_ready_pc34(facts) ||
        !csb_v1_startup_runtime_door_ready_pc34(facts)) {
        csb_v1_startup_runtime_mark_rejected_pc34(
            out_receipt, CSB_V1_STARTUP_RUNTIME_F0580_DRAW_DOOR_STEP_PC34,
            evidence);
        return 0;
    }
    csb_v1_startup_runtime_mark_accepted_pc34(
        out_receipt, CSB_V1_STARTUP_RUNTIME_F0580_DRAW_DOOR_STEP_PC34, facts,
        evidence);
    return 1;
}

int F0581_ENTRANCE_BlitDoors(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts,
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0581_entrance_blit_doors_source_evidence_pc34();
    csb_v1_startup_runtime_coupling_receipt_init_pc34(out_receipt);
    if (!csb_v1_startup_runtime_common_ready_pc34(facts) ||
        !facts->closed_door_hud_runtime_captured ||
        !facts->door_opening_frame_runtime_captured ||
        !facts->no_legacy_door_fallback_route) {
        csb_v1_startup_runtime_mark_rejected_pc34(
            out_receipt, CSB_V1_STARTUP_RUNTIME_F0581_BLIT_DOORS_PC34,
            evidence);
        return 0;
    }
    csb_v1_startup_runtime_mark_accepted_pc34(
        out_receipt, CSB_V1_STARTUP_RUNTIME_F0581_BLIT_DOORS_PC34, facts,
        evidence);
    return 1;
}

const char *csb_v1_f0437_startend_draw_title_source_evidence_pc34(void)
{
    return "ReDMCSB TITLE.C:424-463 F0437_STARTEND_DrawTitle presents C001 "
           "PRESENTS, CHAOS zoom/hold, and STRIKES BACK; CSB PC34 requires "
           "all four runtime phase captures from real startup assets";
}

const char *csb_v1_f0438_startend_open_entrance_doors_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C F0438/F0807 opens C002/C003 doors over the "
           "entrance surface; CSB PC34 requires the 31-step door route, HUD "
           "handoff, receipt-only draw/input, and no legacy door fallback";
}

const char *csb_v1_f0580_entrance_draw_door_step_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C F0580 draws one door animation step; CSB PC34 "
           "accepts only source-bounded step indexes from the real startup "
           "door-opening runtime capture";
}

const char *csb_v1_f0581_entrance_blit_doors_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C F0581 blits closed/opening doors; CSB PC34 "
           "requires the same real closed-door HUD and opening-frame capture "
           "route, with fallback door wrappers rejected";
}
