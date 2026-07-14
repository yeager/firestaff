#include "dm1_v22_real_art_runtime_gate_pc34.h"

#include "dm1_v22_finished_art_material_gate_pc34.h"
#include "dm1_v22_finished_pack_receipt_pc34.h"
#include "dm1_v2_asset_pipeline_pc34.h"

#include <stdio.h>
#include <string.h>

int dm1_v22_real_art_runtime_gate_refresh_pc34(
    const char *data_dir,
    DM1_V22_RealArtRuntimeGate_PC34 *out_gate)
{
    DM1_V22_RealArtRuntimeGate_PC34 gate;
    const char *asset_root;
    const char *manifest_path;
    const char *receipt_path;
    char expected_manifest[FSP_PATH_MAX];

    memset(&gate, 0, sizeof(gate));
    m11_v22_set_manifest_path(data_dir);
    dm1_v22_famg_set_manifest_path(data_dir);
    dm1_v22_fpr_set_receipt_path(data_dir);
    dm1_v22_fpr_reset_state();

    asset_root = m11_v22_get_modern_asset_root();
    manifest_path = dm1_v22_fpr_get_manifest_path();
    receipt_path = dm1_v22_fpr_get_receipt_path();
    if (asset_root && asset_root[0]) {
        gate.asset_root_configured = 1;
        snprintf(gate.asset_root, sizeof(gate.asset_root), "%s", asset_root);
        snprintf(expected_manifest, sizeof(expected_manifest), "%s/modern_asset_manifest.json",
                 asset_root);
    } else {
        expected_manifest[0] = '\0';
    }
    if (manifest_path) {
        snprintf(gate.manifest_path, sizeof(gate.manifest_path), "%s", manifest_path);
    }
    if (receipt_path) {
        snprintf(gate.receipt_path, sizeof(gate.receipt_path), "%s", receipt_path);
    }

    gate.asset_root_matches_receipt =
        gate.asset_root_configured && manifest_path && manifest_path[0] &&
        strcmp(expected_manifest, manifest_path) == 0;
    gate.material_finished_real = dm1_v22_famg_is_finished_real();
    gate.receipt_promoted = dm1_v22_fpr_is_promoted();
    gate.admitted = gate.asset_root_matches_receipt &&
        gate.material_finished_real && gate.receipt_promoted;

    if (out_gate) {
        *out_gate = gate;
    }
    return gate.admitted;
}

const char *dm1_v22_real_art_runtime_gate_source_evidence_pc34(void)
{
    return
        "DM1 V2.2 reviewed real-art runtime admission\n"
        "Source: ReDMCSB DUNVIEW.C:6697-6816 (viewport composition)\n"
        "Source: dm1_v22_finished_art_material_gate_pc34 (non-placeholder PNG gate)\n"
        "Source: dm1_v22_finished_pack_receipt_pc34 (reviewed manifest receipt)\n"
        "Runtime rule: V2.2 requires the renderer asset root to match the receipt "
        "manifest root, FINISHED_REAL material, and MATCH_FINISHED_REAL receipt.\n"
        "All other states fall back to V2.1; placeholder art is never admitted.\n";
}
