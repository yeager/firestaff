#include "csb_v1_f1526_f1565_platform_source_boundary_pc34_compat.h"

#include <string.h>

static const char *anchor_for(unsigned int number)
{
    switch (number) {
    case 1526u: return "ANIM.C:110 CloseWorkstationAndExitApplication";
    case 1527u: return "HINT001.C:50 MOUSE_EnableScreenUpdate_CPSE";
    case 1528u: return "HINT001.C:44 MOUSE_DisableScreenUpdate";
    case 1529u: return "no numbered callable body in ReDMCSB corpus";
    case 1530u: return "CEDTINCI.C:304 ShowMousePointerAsArrow";
    case 1531u: return "CEDTINCI.C:303 MouseHandler";
    case 1532u:
    case 1533u: return "no numbered callable body in ReDMCSB corpus";
    case 1534u: return "ATARIST.H:376 CallAES";
    default: return "UTSTAES.C:169-431 AES platform vector";
    }
}

int csb_v1_f1526_f1565_platform_source_boundary_admit_pc34(
    unsigned int number,
    CSB_V1_F1526F1565PlatformSourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F1526F1565PlatformSourceBoundaryReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 1526u || number > 1565u) return 0;

    receipt.function_number = number;
    receipt.redmcsb_anchor = anchor_for(number);
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f1526_f1565_platform_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB ANIM.C, HINT001.C, CEDTINCI.C, ATARIST.H, and UTSTAES.C "
           "show F1526-F1565 are workstation, mouse, and AES platform routes. "
           "No authenticated CSB PC34 package consumer is proven, so every CSB "
           "route fails closed and this receipt does not synthesize UI, graphics, "
           "input, audio, or timing.";
}
