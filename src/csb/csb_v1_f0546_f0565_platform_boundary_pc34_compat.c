#include "csb_v1_f0546_f0565_platform_boundary_pc34_compat.h"

#include <string.h>

int csb_v1_f0546_f0565_platform_boundary_receipt_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0546F0565PlatformBoundaryReceiptPc34 *out)
{
    CSB_V1_F0546F0565PlatformBoundaryReceiptPc34 receipt;

    if (!out) return CSB_V1_F0546_F0565_PLATFORM_REJECT_ARGUMENT;
    memset(&receipt, 0, sizeof(receipt));
    receipt.source_evidence =
        "ReDMCSB AMIGA.H F0546-F0565 declarations only; "
        "no PC34 callable body or authenticated Amiga display/input corpus";
    receipt.mouse_owner_required = 1;
    receipt.video_owner_required = 1;
    receipt.text_scroller_owner_required = 1;
    *out = receipt;

    /* PC34 graphics provenance cannot establish Amiga sprite, copper,
       bitplane, text-scroller, or viewport palette ownership. */
    (void)cache;
    (void)package;
    return CSB_V1_F0546_F0565_PLATFORM_REJECT_UNPROVEN_AMIGA_OWNER;
}
