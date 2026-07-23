#include "dm1_v1_s0080_s0081_media_platform_boundary_pc34_compat.h"

#include <string.h>

static int dm1_v1_s0080_s0081_request_valid_pc34(
    const DM1_V1_S0080S0081MediaPlatformRequestPc34 *request)
{
    return request && request->raw_media_fingerprint != 0u &&
        request->original_pc34_media_verified && request->no_dma_emulation &&
        request->no_floppy_emulation;
}

static void dm1_v1_s0080_s0081_set_receipt_pc34(
    const DM1_V1_S0080S0081MediaPlatformRequestPc34 *request,
    DM1_V1_S0080S0081MediaPlatformReceiptPc34 *out_receipt,
    int dma_completion_suppressed,
    int floppy_power_suppressed)
{
    if (!out_receipt) return;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm1_v1_s0080_s0081_request_valid_pc34(request)) return;
    out_receipt->fail_closed = 1;
    out_receipt->dma_completion_suppressed = dma_completion_suppressed;
    out_receipt->floppy_power_suppressed = floppy_power_suppressed;
    out_receipt->source_evidence =
        dm1_v1_s0080_s0081_media_platform_source_evidence_pc34();
}

int dm1_v1_s0080_check_dma_transfer_completion_boundary_pc34(
    const DM1_V1_S0080S0081MediaPlatformRequestPc34 *request,
    DM1_V1_S0080S0081MediaPlatformReceiptPc34 *out_receipt)
{
    dm1_v1_s0080_s0081_set_receipt_pc34(request, out_receipt, 1, 0);
    return 0;
}

int dm1_v1_s0081_turn_off_floppy_drive_boundary_pc34(
    const DM1_V1_S0080S0081MediaPlatformRequestPc34 *request,
    DM1_V1_S0080S0081MediaPlatformReceiptPc34 *out_receipt)
{
    dm1_v1_s0080_s0081_set_receipt_pc34(request, out_receipt, 0, 1);
    return 0;
}

const char *dm1_v1_s0080_s0081_media_platform_source_evidence_pc34(void)
{
    return "ReDMCSB ATARIST.H:250-251 binds S0080/S0081 to Atari CPSDF "
           "DMA-completion and floppy-power routines. No PC34 source body is "
           "available, so Firestaff neither emulates DMA nor changes drive power.";
}
