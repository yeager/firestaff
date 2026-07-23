#include "csb_v1_f0566_f0585_platform_boundary_pc34_compat.h"

#include <string.h>

int csb_v1_f0566_f0585_platform_boundary_receipt_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0566F0585PlatformBoundaryReceiptPc34 *out)
{
    CSB_V1_F0566F0585PlatformBoundaryReceiptPc34 receipt;

    if (!out) return CSB_V1_F0566_F0585_PLATFORM_REJECT_ARGUMENT;
    memset(&receipt, 0, sizeof(receipt));
    receipt.source_evidence =
        "ReDMCSB AMIGA.H F0566-F0581 declarations; F0582-F0585 absent "
        "from callable inventory; no PC34 body or Amiga interrupt/bitplane corpus";
    receipt.viewport_owner_required = 1;
    receipt.interrupt_owner_required = 1;
    receipt.entrance_owner_required = 1;
    *out = receipt;

    /* An authentic PC34 graphics cache cannot prove Amiga vblank, floppy
       interrupt, display blit, or bitplane door-animation semantics. */
    (void)cache;
    (void)package;
    return CSB_V1_F0566_F0585_PLATFORM_REJECT_UNPROVEN_AMIGA_OWNER;
}
