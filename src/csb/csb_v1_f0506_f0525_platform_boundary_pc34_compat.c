#include "csb_v1_f0506_f0525_platform_boundary_pc34_compat.h"

#include <string.h>

int csb_v1_f0506_f0525_platform_boundary_receipt_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0506F0525PlatformBoundaryReceiptPc34 *out)
{
    CSB_V1_F0506F0525PlatformBoundaryReceiptPc34 receipt;

    if (!out) return CSB_V1_F0506_F0525_PLATFORM_REJECT_ARGUMENT;
    memset(&receipt, 0, sizeof(receipt));
    receipt.source_evidence =
        "ReDMCSB AMIGA.H F0506-F0525 declarations only; "
        "no PC34 callable body or authenticated Amiga binary corpus";
    receipt.amiga_owner_required = 1;
    receipt.floppy_owner_required = 1;
    *out = receipt;

    /* A real PC34 graphics cache does not prove Amiga copper, planar,
       trackdisk, filesystem, or portrait-layout semantics. */
    (void)cache;
    (void)package;
    return CSB_V1_F0506_F0525_PLATFORM_REJECT_UNPROVEN_AMIGA_OWNER;
}
