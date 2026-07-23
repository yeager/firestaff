#include "csb_v1_f1186_f1205_anim_source_boundary_pc34_compat.h"

#include <string.h>

typedef struct {
    const char *symbol;
    const char *anchor;
} Spec;

static const Spec k_specs[] = {
    { "F1186_Process_SO", "ANIM.C:652" },
    { "F1187_Process_MI", "ANIM.C:664" },
    { "F1188_Process_SF", "ANIM.C:672" },
    { "F1189_Process_MF", "ANIM.C:705" },
    { "F1190_Process_FO", "ANIM.C:738" },
    { "F1191_Process_NE", "ANIM.C:747" },
    { "F1192_Process_BN", "ANIM.C:759" },
    { "F1193_Process_DO", "ANIM.C:766" },
    { "F1194_Process_WA", "ANIM.C:775" },
    { "F1195_Process_TR", "ANIM.C:783" },
    { "F1196_ANIM_dispatch", "ANIM.C:792" },
    { "F1197_no_numbered_body", "ReDMCSB corpus: no numbered F1197 body" },
    { "F1198_ANIM_error_exit", "ANIM.C:936" },
    { "F1199_ANIM_playback", "ANIM.C:942" },
    { "F1200_ANIM_usio_queue_drain", "ANIM.C:973" },
    { "F1201_ANIM_breakable_input", "ANIM.C:982" },
    { "F1202_ANIM_allocation", "ANIM.C:997" },
    { "F1203_ANIM_release_music_stop", "ANIM.C:1008" },
    { "F1204_ANIM_expand_palette", "ANIM.C:1019" },
    { "F1205_ANIM_double_buffer", "ANIM.C:1070" }
};

int csb_v1_f1186_f1205_anim_source_boundary_admit_pc34(
    unsigned int number,
    CSB_V1_F1186F1205AnimSourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F1186F1205AnimSourceBoundaryReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 1186u || number > 1205u) return 0;

    receipt.function_number = number;
    receipt.symbol = k_specs[number - 1186u].symbol;
    receipt.redmcsb_anchor = k_specs[number - 1186u].anchor;
    receipt.authentic_pc34_material_required = 1;
    receipt.runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f1186_f1205_anim_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB ANIM.C F1186-F1205 is owned by the DM1 animation path. "
           "No authenticated CSB PC34 ANIM stream and runtime consumer are "
           "proven, so every CSB route fails closed and this receipt does not "
           "render, synthesize UI, or create timing.";
}
