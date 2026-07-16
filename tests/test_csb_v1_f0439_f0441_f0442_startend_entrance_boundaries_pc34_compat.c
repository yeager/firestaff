#include "csb_v1_f0439_f0441_f0442_startend_entrance_boundaries_pc34_compat.h"

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

static CSB_V1_StartEndEntranceHostViewFacts_PC34 make_closed_door_host_view(void)
{
    CSB_V1_StartEndEntranceHostViewFacts_PC34 view;
    memset(&view, 0, sizeof(view));
    view.valid = 1;
    view.capture_proof_valid = 1;
    view.capture_real_asset_matched = 1;
    view.closed_door_menu_route = 1;
    view.render_draw_valid = 1;
    view.closed_door_asset_commands_ready = 1;
    view.render_draw_real_asset_matched = 1;
    view.hud_menu_draw_valid = 1;
    view.draw_closed_doors = 1;
    view.host_draw_package_ready = 1;
    view.host_draw_uses_receipt_package = 1;
    view.no_legacy_render_wrapper_ready = 1;
    return view;
}

static CSB_V1_StartEndEntranceHostViewFacts_PC34 make_credits_host_view(void)
{
    CSB_V1_StartEndEntranceHostViewFacts_PC34 view;
    memset(&view, 0, sizeof(view));
    view.valid = 1;
    view.capture_proof_valid = 1;
    view.capture_real_asset_matched = 1;
    view.credits_route = 1;
    view.render_draw_valid = 1;
    view.primitive_commands_ready = 1;
    view.render_draw_real_asset_matched = 1;
    view.host_draw_package_ready = 1;
    view.host_draw_uses_receipt_package = 1;
    view.no_legacy_render_wrapper_ready = 1;
    return view;
}

static CSB_V1_StartEndEntranceHostOwnershipFacts_PC34 make_process_ownership(void)
{
    CSB_V1_StartEndEntranceHostOwnershipFacts_PC34 ownership;
    memset(&ownership, 0, sizeof(ownership));
    ownership.valid = 1;
    ownership.snapshot_capture_valid = 1;
    ownership.host_view_valid = 1;
    ownership.host_draw_valid = 1;
    ownership.host_input_dispatch_valid = 1;
    ownership.capture_proof_valid = 1;
    ownership.packaged_visual_capture_ready = 1;
    ownership.real_asset_matched = 1;
    ownership.draw_consumes_receipt_only = 1;
    ownership.input_consumes_receipt_only = 1;
    ownership.host_input_blocked = 1;
    ownership.startup_input_ready = 1;
    ownership.host_route_wrappers_retired = 1;
    ownership.no_loose_render_plan_exports = 1;
    return ownership;
}

static void test_accepts_real_receipt_routes(void)
{
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 receipt;
    CSB_V1_StartEndEntranceHostViewFacts_PC34 closed =
        make_closed_door_host_view();
    CSB_V1_StartEndEntranceHostViewFacts_PC34 credits =
        make_credits_host_view();
    CSB_V1_StartEndEntranceHostOwnershipFacts_PC34 ownership =
        make_process_ownership();

    CHECK(F0439_STARTEND_DrawEntrance(&closed, &receipt) == 1);
    CHECK(receipt.valid == 1);
    CHECK((receipt.accepted_stage_mask &
           CSB_V1_STARTEND_F0439_DRAW_ENTRANCE_PC34) != 0u);
    CHECK(receipt.real_asset_matched == 1);
    CHECK(receipt.no_synthetic_payloads == 1);
    CHECK(receipt.no_fallback_graphics == 1);

    CHECK(F0442_STARTEND_ProcessCommand202_EntranceDrawCredits(
              &credits, &receipt) == 1);
    CHECK((receipt.accepted_stage_mask &
           CSB_V1_STARTEND_F0442_DRAW_CREDITS_PC34) != 0u);
    CHECK(receipt.host_draw_consumed == 1);

    CHECK(F0441_STARTEND_ProcessEntrance(&ownership, &receipt) == 1);
    CHECK((receipt.accepted_stage_mask &
           CSB_V1_STARTEND_F0441_PROCESS_ENTRANCE_PC34) != 0u);
    CHECK(receipt.host_input_consumed == 1);
    CHECK(receipt.input_consumes_receipt_only == 1);
}

static void test_rejects_missing_real_data_or_fallback_graphics(void)
{
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 receipt;
    CSB_V1_StartEndEntranceHostViewFacts_PC34 closed =
        make_closed_door_host_view();
    CSB_V1_StartEndEntranceHostViewFacts_PC34 credits =
        make_credits_host_view();
    CSB_V1_StartEndEntranceHostOwnershipFacts_PC34 ownership =
        make_process_ownership();

    closed.capture_real_asset_matched = 0;
    CHECK(F0439_STARTEND_DrawEntrance(&closed, &receipt) == 0);
    CHECK((receipt.rejected_stage_mask &
           CSB_V1_STARTEND_F0439_DRAW_ENTRANCE_PC34) != 0u);
    CHECK(receipt.no_synthetic_payloads == 1);

    closed = make_closed_door_host_view();
    closed.hud_draw_fallback_text = 1;
    CHECK(F0439_STARTEND_DrawEntrance(&closed, &receipt) == 0);
    CHECK(receipt.no_fallback_graphics == 1);

    credits.draw_fallback_text = 1;
    CHECK(F0442_STARTEND_ProcessCommand202_EntranceDrawCredits(
              &credits, &receipt) == 0);
    CHECK((receipt.rejected_stage_mask &
           CSB_V1_STARTEND_F0442_DRAW_CREDITS_PC34) != 0u);

    ownership.host_input_dispatch_valid = 0;
    CHECK(F0441_STARTEND_ProcessEntrance(&ownership, &receipt) == 0);
    CHECK((receipt.rejected_stage_mask &
           CSB_V1_STARTEND_F0441_PROCESS_ENTRANCE_PC34) != 0u);
}

static void test_evidence_strings(void)
{
    check_contains(csb_v1_f0439_startend_draw_entrance_source_evidence_pc34(),
                   "ENTRANCE.C:373-619");
    check_contains(csb_v1_f0439_startend_draw_entrance_source_evidence_pc34(),
                   "closed-door route proof");

    check_contains(csb_v1_f0441_startend_process_entrance_source_evidence_pc34(),
                   "ENTRANCE.C:645-930");
    check_contains(csb_v1_f0441_startend_process_entrance_source_evidence_pc34(),
                   "same real startup package");

    check_contains(csb_v1_f0442_startend_draw_credits_source_evidence_pc34(),
                   "ENTRANCE.C:951-1018");
    check_contains(csb_v1_f0442_startend_draw_credits_source_evidence_pc34(),
                   "synthetic credit pixels");
}

int main(void)
{
    test_accepts_real_receipt_routes();
    test_rejects_missing_real_data_or_fallback_graphics();
    test_evidence_strings();
    return 0;
}
