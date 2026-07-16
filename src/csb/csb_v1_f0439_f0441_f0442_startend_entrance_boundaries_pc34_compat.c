#include "csb_v1_f0439_f0441_f0442_startend_entrance_boundaries_pc34_compat.h"

#include <string.h>

static void csb_v1_startend_entrance_mark_rejected_pc34(
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *receipt, uint32_t stage,
    const char *source_evidence)
{
    if (!receipt) return;
    receipt->rejected_stage_mask |= stage;
    receipt->no_synthetic_payloads = 1;
    receipt->no_fallback_graphics = 1;
    receipt->source_evidence = source_evidence;
}

static void csb_v1_startend_entrance_mark_accepted_pc34(
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *receipt, uint32_t stage,
    const char *source_evidence)
{
    if (!receipt) return;
    receipt->valid = 1;
    receipt->accepted_stage_mask |= stage;
    receipt->real_asset_matched = 1;
    receipt->draw_consumes_receipt_only = 1;
    receipt->no_synthetic_payloads = 1;
    receipt->no_fallback_graphics = 1;
    receipt->source_evidence = source_evidence;
}

void csb_v1_startend_entrance_boundary_receipt_init_pc34(
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int F0439_STARTEND_DrawEntrance(
    const CSB_V1_StartEndEntranceHostViewFacts_PC34 *host_view,
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0439_startend_draw_entrance_source_evidence_pc34();

    csb_v1_startend_entrance_boundary_receipt_init_pc34(out_receipt);
    if (!host_view || !host_view->valid || !host_view->capture_proof_valid ||
        !host_view->capture_real_asset_matched ||
        !host_view->closed_door_menu_route ||
        host_view->draw_fallback_text ||
        !host_view->render_draw_valid ||
        !host_view->closed_door_asset_commands_ready ||
        !host_view->render_draw_real_asset_matched ||
        !host_view->hud_menu_draw_valid ||
        !host_view->draw_closed_doors ||
        host_view->hud_draw_fallback_text ||
        !host_view->host_draw_package_ready ||
        !host_view->host_draw_uses_receipt_package ||
        !host_view->no_legacy_render_wrapper_ready) {
        csb_v1_startend_entrance_mark_rejected_pc34(
            out_receipt, CSB_V1_STARTEND_F0439_DRAW_ENTRANCE_PC34, evidence);
        return 0;
    }

    csb_v1_startend_entrance_mark_accepted_pc34(
        out_receipt, CSB_V1_STARTEND_F0439_DRAW_ENTRANCE_PC34, evidence);
    out_receipt->host_view_consumed = 1;
    out_receipt->host_draw_consumed = 1;
    out_receipt->route_wrappers_retired = 1;
    return 1;
}

int F0441_STARTEND_ProcessEntrance(
    const CSB_V1_StartEndEntranceHostOwnershipFacts_PC34 *ownership,
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0441_startend_process_entrance_source_evidence_pc34();

    csb_v1_startend_entrance_boundary_receipt_init_pc34(out_receipt);
    if (!ownership || !ownership->valid || !ownership->snapshot_capture_valid ||
        !ownership->host_view_valid || !ownership->host_draw_valid ||
        !ownership->host_input_dispatch_valid || !ownership->capture_proof_valid ||
        !ownership->packaged_visual_capture_ready ||
        !ownership->real_asset_matched ||
        !ownership->draw_consumes_receipt_only ||
        !ownership->input_consumes_receipt_only ||
        !ownership->host_input_blocked || !ownership->startup_input_ready ||
        !ownership->host_route_wrappers_retired ||
        !ownership->no_loose_render_plan_exports ||
        ownership->draw_fallback_text) {
        csb_v1_startend_entrance_mark_rejected_pc34(
            out_receipt, CSB_V1_STARTEND_F0441_PROCESS_ENTRANCE_PC34,
            evidence);
        return 0;
    }

    csb_v1_startend_entrance_mark_accepted_pc34(
        out_receipt, CSB_V1_STARTEND_F0441_PROCESS_ENTRANCE_PC34, evidence);
    out_receipt->host_view_consumed = 1;
    out_receipt->host_draw_consumed = 1;
    out_receipt->host_input_consumed = 1;
    out_receipt->input_consumes_receipt_only = 1;
    out_receipt->route_wrappers_retired = 1;
    return 1;
}

int F0442_STARTEND_ProcessCommand202_EntranceDrawCredits(
    const CSB_V1_StartEndEntranceHostViewFacts_PC34 *host_view,
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0442_startend_draw_credits_source_evidence_pc34();

    csb_v1_startend_entrance_boundary_receipt_init_pc34(out_receipt);
    if (!host_view || !host_view->valid || !host_view->capture_proof_valid ||
        !host_view->capture_real_asset_matched ||
        !host_view->credits_route ||
        host_view->draw_fallback_text ||
        !host_view->render_draw_valid ||
        !host_view->primitive_commands_ready ||
        !host_view->render_draw_real_asset_matched ||
        !host_view->host_draw_package_ready ||
        !host_view->host_draw_uses_receipt_package ||
        !host_view->no_legacy_render_wrapper_ready) {
        csb_v1_startend_entrance_mark_rejected_pc34(
            out_receipt, CSB_V1_STARTEND_F0442_DRAW_CREDITS_PC34, evidence);
        return 0;
    }

    csb_v1_startend_entrance_mark_accepted_pc34(
        out_receipt, CSB_V1_STARTEND_F0442_DRAW_CREDITS_PC34, evidence);
    out_receipt->host_view_consumed = 1;
    out_receipt->host_draw_consumed = 1;
    out_receipt->route_wrappers_retired = 1;
    return 1;
}

const char *csb_v1_f0439_startend_draw_entrance_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:373-619 F0439_STARTEND_DrawEntrance draws the "
           "entrance screen and closed door graphics; CSB PC34 accepts it only "
           "through a real-asset host-view receipt with closed-door route proof "
           "and no fallback graphics";
}

const char *csb_v1_f0441_startend_process_entrance_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:645-930 F0441_STARTEND_ProcessEntrance owns "
           "the entrance wait/input loop around F0439/F0442/F0438; CSB PC34 "
           "accepts it only when host view, draw, and input receipts all "
           "consume the same real startup package";
}

const char *csb_v1_f0442_startend_draw_credits_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:951-1018 F0442_STARTEND_ProcessCommand202_"
           "EntranceDrawCredits presents C005 credits before returning to the "
           "entrance loop; CSB PC34 requires a real credits route proof and "
           "rejects fallback text or synthetic credit pixels";
}
