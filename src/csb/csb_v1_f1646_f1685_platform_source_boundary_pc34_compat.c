#include "csb_v1_f1646_f1685_platform_source_boundary_pc34_compat.h"

#include <string.h>

static const char *anchor_for(unsigned int number)
{
    if (number >= 1650u && number <= 1654u)
        return "SWITCH.C/SWITCHMM.C: allocation and coordinate platform route";
    if (number == 1658u)
        return "VDEO2.C:137 IsShiftKeyPressed";
    if (number >= 1662u && number <= 1663u)
        return "VDEO1.C:1251-1284 vertical-blank platform route";
    if (number >= 1665u && number <= 1683u)
        return "INT1.C: interrupt-manager platform route";
    if (number == 1684u)
        return "USIO1.C:80 GetMouseStatus; existing DM1-only PC34 owner";
    if (number == 1685u)
        return "USIO1.C:180 InstallMouseInterruptHandler";
    return "no numbered callable body in ReDMCSB corpus";
}

static const char *boundary_for(unsigned int number)
{
    if (number == 1684u) return "DM1-only input owner; no CSB package route";
    if (number >= 1650u && number <= 1654u)
        return "DM1 Switch owner or unproved platform route";
    return "fail_closed: non-PC34 platform or absent source";
}

int csb_v1_f1646_f1685_platform_source_boundary_admit_pc34(
    unsigned int number,
    CSB_V1_F1646F1685PlatformSourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F1646F1685PlatformSourceBoundaryReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 1646u || number > 1685u) return 0;

    receipt.function_number = number;
    receipt.redmcsb_anchor = anchor_for(number);
    receipt.existing_owner_or_boundary = boundary_for(number);
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f1646_f1685_platform_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB SWITCH.C, SWITCHMM.C, VDEO1.C, VDEO2.C, INT1.C, and "
           "USIO1.C are the authority for F1646-F1685. Existing DM1 owners, "
           "including F1684 mouse status, remain exclusive. No authenticated "
           "CSB PC34 package consumer is proven, so every CSB route fails closed "
           "without synthetic input, UI, graphics, audio, or timing.";
}
