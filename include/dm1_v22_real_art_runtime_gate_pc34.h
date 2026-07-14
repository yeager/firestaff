/* DM1 V2.2 runtime admission for reviewed, non-placeholder modern art. */
#ifndef FIRESTAFF_DM1_V22_REAL_ART_RUNTIME_GATE_PC34_H
#define FIRESTAFF_DM1_V22_REAL_ART_RUNTIME_GATE_PC34_H

#include "fs_portable_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int asset_root_configured;
    int asset_root_matches_receipt;
    int material_finished_real;
    int receipt_promoted;
    int admitted;
    char asset_root[FSP_PATH_MAX];
    char manifest_path[FSP_PATH_MAX];
    char receipt_path[FSP_PATH_MAX];
} DM1_V22_RealArtRuntimeGate_PC34;

/* Rebind all V2.2 asset consumers to data_dir and compute the admission
 * receipt. V2.2 is admitted only for a reviewed FINISHED_REAL pack rooted at
 * the exact same modern asset directory consumed by the renderer. */
int dm1_v22_real_art_runtime_gate_refresh_pc34(
    const char *data_dir,
    DM1_V22_RealArtRuntimeGate_PC34 *out_gate);

const char *dm1_v22_real_art_runtime_gate_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V22_REAL_ART_RUNTIME_GATE_PC34_H */
