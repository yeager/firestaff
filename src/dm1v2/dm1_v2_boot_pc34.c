#include "dm1_v2_boot_pc34.h"

#include <string.h>

#include "dm1_v22_finished_pack_receipt_pc34.h"
#include "dm1_v2_asset_pipeline_pc34.h"
#include "dm1_v2_shape_runtime_pc34.h"
#include "m11_v22_inplace_draw_pc34.h"

void dm1_v2_boot_startup_receipt_clear_pc34(
    DM1_V2_BootStartupReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

int dm1_v2_boot_startup_prepare_pc34(
    const char *game_id,
    const char *data_dir,
    int m12_presentation_mode,
    DM1_V2_BootStartupReceipt_PC34 *out_receipt)
{
    DM1_V2_PhaseGateConfig gate;
    const DM1_V2_PresentationModeState *mode_state;

    if (out_receipt) {
        dm1_v2_boot_startup_receipt_clear_pc34(out_receipt);
    }
    if (!game_id || strcmp(game_id, "dm1") != 0) {
        return 0;
    }

    if (data_dir && data_dir[0]) {
        m11_v22_set_manifest_path(data_dir);
        dm1_v22_fpr_set_receipt_path(data_dir);
    }

    dm1_v2_presentation_mode_set_m12(m12_presentation_mode);
    mode_state = dm1_v2_presentation_mode_state();

    dm1_v2_phase_gate_defaults(&gate);
    gate.v2PresentationEnabled =
        (mode_state && mode_state->v2Active) ? 1 : 0;
    gate.v2ConfigPersistenceEnabled = 1;

    if (out_receipt) {
        out_receipt->is_dm1 = 1;
        out_receipt->requested_presentation_mode = m12_presentation_mode;
        out_receipt->resolved_mode = mode_state
            ? mode_state->kind
            : DM1_V2_PM_V1_FAITHFUL;
        out_receipt->v2_presentation_enabled = gate.v2PresentationEnabled;
        out_receipt->v2_config_persistence_enabled =
            gate.v2ConfigPersistenceEnabled;
        out_receipt->modern_pack_available = mode_state
            ? mode_state->modernPackAvailable
            : 0;
        out_receipt->render_decision = dm1_v2_phase_gate_decide(
            &gate,
            DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
        out_receipt->input_decision = dm1_v2_phase_gate_decide(
            &gate,
            DM1_V2_PHASE_DOMAIN_INPUT_PRESENTATION);
        out_receipt->config_decision = dm1_v2_phase_gate_decide(
            &gate,
            DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    }

    if (dm1_v2_shape_runtime_v22_active()) {
        int cache_ready = m11_v22_inplace_draw_init();
        if (out_receipt) {
            out_receipt->v22_shape_runtime_active = 1;
            out_receipt->v22_inplace_cache_attempted = 1;
            out_receipt->v22_inplace_cache_active =
                cache_ready && m11_v22_inplace_draw_active();
        }
    }

    return 1;
}

const char *dm1_v2_boot_source_evidence_pc34(void)
{
    return
        "DM1 V2 boot startup prepare: sets DM1 V2/V22 asset roots before "
        "presentation-mode resolve, then gates V2 render/input/config through "
        "dm1_v2_phase_gate_pc34. Source-lock anchors: ReDMCSB COMMAND.C "
        "F0359 LoadGameSettings and DUNVIEW.C F0128 viewport presentation.";
}
