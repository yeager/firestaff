#include "dm1_v1_f0449_f0450_floppy_media_guard_pc34_compat.h"

#include <string.h>

static int dm1_v1_f0449_f0450_has_verified_pc34_media(
    const DM1_V1_F0449F0450FloppyMediaRequestPc34 *request)
{
    return request && request->raw_media_bytes && request->raw_media_byte_count > 0u &&
        request->raw_media_fingerprint != 0u && request->original_pc34_media_verified &&
        request->no_emulated_media && request->no_synthetic_availability;
}

static void dm1_v1_f0449_f0450_set_receipt(
    const DM1_V1_F0449F0450FloppyMediaRequestPc34 *request,
    DM1_V1_F0449F0450FloppyMediaReceiptPc34 *out_receipt,
    int write_protection_probe_suppressed,
    int media_change_probe_suppressed)
{
    if (!out_receipt) return;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm1_v1_f0449_f0450_has_verified_pc34_media(request)) return;
    out_receipt->fail_closed = 1;
    out_receipt->write_protection_probe_suppressed = write_protection_probe_suppressed;
    out_receipt->media_change_probe_suppressed = media_change_probe_suppressed;
    out_receipt->suppress_synthetic_availability = 1;
    out_receipt->source_evidence =
        dm1_v1_f0449_f0450_floppy_media_guard_source_evidence_pc34();
}

int dm1_v1_f0449_is_disk_write_protected_guard_pc34(
    const DM1_V1_F0449F0450FloppyMediaRequestPc34 *request,
    DM1_V1_F0449F0450FloppyMediaReceiptPc34 *out_receipt)
{
    dm1_v1_f0449_f0450_set_receipt(request, out_receipt, 1, 0);
    return 0;
}

int dm1_v1_f0450_force_media_change_detection_guard_pc34(
    const DM1_V1_F0449F0450FloppyMediaRequestPc34 *request,
    DM1_V1_F0449F0450FloppyMediaReceiptPc34 *out_receipt)
{
    dm1_v1_f0449_f0450_set_receipt(request, out_receipt, 0, 1);
    return 0;
}

const char *dm1_v1_f0449_f0450_floppy_media_guard_source_evidence_pc34(void)
{
    return "ReDMCSB FLOPPYST.C:21-37 F0449 uses Atari ST Floprd/Flopwr; "
           "FLOPPYST.C:41-126 F0450 uses Atari ST Rwabs/GEMDOS. FLOPPY.C:606-"
           "620 shows the PC34 F0450 call before Floprd but supplies no PC34 "
           "F0449/F0450 body or media-state mutation.";
}
