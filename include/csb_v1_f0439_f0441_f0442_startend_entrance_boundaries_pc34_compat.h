#ifndef FIRESTAFF_CSB_V1_F0439_F0441_F0442_STARTEND_ENTRANCE_BOUNDARIES_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0439_F0441_F0442_STARTEND_ENTRANCE_BOUNDARIES_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CSB_V1_StartEndEntranceBoundaryStage_PC34 {
    CSB_V1_STARTEND_F0439_DRAW_ENTRANCE_PC34 = 1u << 0,
    CSB_V1_STARTEND_F0441_PROCESS_ENTRANCE_PC34 = 1u << 1,
    CSB_V1_STARTEND_F0442_DRAW_CREDITS_PC34 = 1u << 2
} CSB_V1_StartEndEntranceBoundaryStage_PC34;

#define CSB_V1_STARTEND_ENTRANCE_ALL_STAGES_PC34 \
    (CSB_V1_STARTEND_F0439_DRAW_ENTRANCE_PC34 | \
     CSB_V1_STARTEND_F0441_PROCESS_ENTRANCE_PC34 | \
     CSB_V1_STARTEND_F0442_DRAW_CREDITS_PC34)

typedef struct CSB_V1_StartEndEntranceBoundaryReceipt_PC34 {
    int valid;
    uint32_t accepted_stage_mask;
    uint32_t rejected_stage_mask;
    int real_asset_matched;
    int host_view_consumed;
    int host_draw_consumed;
    int host_input_consumed;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    int no_synthetic_payloads;
    int no_fallback_graphics;
    int route_wrappers_retired;
    const char *source_evidence;
} CSB_V1_StartEndEntranceBoundaryReceipt_PC34;

typedef struct CSB_V1_StartEndEntranceHostViewFacts_PC34 {
    int valid;
    int capture_proof_valid;
    int capture_real_asset_matched;
    int closed_door_menu_route;
    int credits_route;
    int draw_fallback_text;
    int render_draw_valid;
    int closed_door_asset_commands_ready;
    int primitive_commands_ready;
    int render_draw_real_asset_matched;
    int hud_menu_draw_valid;
    int draw_closed_doors;
    int hud_draw_fallback_text;
    int host_draw_package_ready;
    int host_draw_uses_receipt_package;
    int no_legacy_render_wrapper_ready;
} CSB_V1_StartEndEntranceHostViewFacts_PC34;

typedef struct CSB_V1_StartEndEntranceHostOwnershipFacts_PC34 {
    int valid;
    int snapshot_capture_valid;
    int host_view_valid;
    int host_draw_valid;
    int host_input_dispatch_valid;
    int capture_proof_valid;
    int packaged_visual_capture_ready;
    int real_asset_matched;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    int host_input_blocked;
    int startup_input_ready;
    int host_route_wrappers_retired;
    int no_loose_render_plan_exports;
    int draw_fallback_text;
} CSB_V1_StartEndEntranceHostOwnershipFacts_PC34;

void csb_v1_startend_entrance_boundary_receipt_init_pc34(
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *receipt);

int F0439_STARTEND_DrawEntrance(
    const CSB_V1_StartEndEntranceHostViewFacts_PC34 *host_view,
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *out_receipt);
int F0441_STARTEND_ProcessEntrance(
    const CSB_V1_StartEndEntranceHostOwnershipFacts_PC34 *ownership,
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *out_receipt);
int F0442_STARTEND_ProcessCommand202_EntranceDrawCredits(
    const CSB_V1_StartEndEntranceHostViewFacts_PC34 *host_view,
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *out_receipt);

const char *csb_v1_f0439_startend_draw_entrance_source_evidence_pc34(void);
const char *csb_v1_f0441_startend_process_entrance_source_evidence_pc34(void);
const char *csb_v1_f0442_startend_draw_credits_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0439_F0441_F0442_STARTEND_ENTRANCE_BOUNDARIES_PC34_COMPAT_H */
