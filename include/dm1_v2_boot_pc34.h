#ifndef FIRESTAFF_DM1_V2_BOOT_PC34_H
#define FIRESTAFF_DM1_V2_BOOT_PC34_H

#include "dm1_v2_phase_gate_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int is_dm1;
    int requested_presentation_mode;
    DM1_V2_PresentationModeKind resolved_mode;
    int v2_presentation_enabled;
    int v2_config_persistence_enabled;
    int modern_pack_available;
    int v22_shape_runtime_active;
    int v22_inplace_cache_attempted;
    int v22_inplace_cache_active;
    DM1_V2_PhaseGateDecision render_decision;
    DM1_V2_PhaseGateDecision input_decision;
    DM1_V2_PhaseGateDecision config_decision;
} DM1_V2_BootStartupReceipt_PC34;

void dm1_v2_boot_startup_receipt_clear_pc34(
    DM1_V2_BootStartupReceipt_PC34 *receipt);

int dm1_v2_boot_startup_prepare_pc34(
    const char *game_id,
    const char *data_dir,
    int m12_presentation_mode,
    DM1_V2_BootStartupReceipt_PC34 *out_receipt);

const char *dm1_v2_boot_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_BOOT_PC34_H */
