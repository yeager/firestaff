#include "dm1_v1_f0447_f0448_platform_boundary_pc34_compat.h"

#include <string.h>

static int dm1_v1_f0447_f0448_request_is_explicit_pc34_boundary(
    const DM1_V1_F0447F0448PlatformRequestPc34 *request)
{
    return request &&
        request->requested_platform == DM1_V1_F0447_F0448_SOURCE_PLATFORM_PC34 &&
        request->original_source_branch_verified &&
        request->no_platform_emulation && request->no_synthetic_memory_manager;
}

static void dm1_v1_f0447_f0448_set_receipt(
    const DM1_V1_F0447F0448PlatformRequestPc34 *request,
    DM1_V1_F0447F0448PlatformReceiptPc34 *out_receipt,
    int hang_suppressed,
    int memory_manager_suppressed)
{
    if (!out_receipt) return;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm1_v1_f0447_f0448_request_is_explicit_pc34_boundary(request)) return;
    out_receipt->fail_closed = 1;
    out_receipt->hang_suppressed = hang_suppressed;
    out_receipt->memory_manager_suppressed = memory_manager_suppressed;
    out_receipt->suppress_platform_emulation = 1;
    out_receipt->source_evidence =
        dm1_v1_f0447_f0448_platform_boundary_source_evidence_pc34();
}

int dm1_v1_f0447_hang_if_false_boundary_pc34(
    const DM1_V1_F0447F0448PlatformRequestPc34 *request,
    DM1_V1_F0447F0448PlatformReceiptPc34 *out_receipt)
{
    dm1_v1_f0447_f0448_set_receipt(request, out_receipt, 1, 0);
    return 0;
}

int dm1_v1_f0448_initialize_memory_manager_boundary_pc34(
    const DM1_V1_F0447F0448PlatformRequestPc34 *request,
    DM1_V1_F0447F0448PlatformReceiptPc34 *out_receipt)
{
    dm1_v1_f0447_f0448_set_receipt(request, out_receipt, 0, 1);
    return 0;
}

const char *dm1_v1_f0447_f0448_platform_boundary_source_evidence_pc34(void)
{
    return "ReDMCSB ENDGAME.C:970-980 F0447 is an Atari ST MEDIA007 "
           "copy-protection infinite loop; STARTUP1.C:76-142 F0448 probes "
           "Atari ST supervisor/GEM memory. Neither source body is PC34.";
}
