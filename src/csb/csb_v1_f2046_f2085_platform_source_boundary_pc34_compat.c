#include "csb_v1_f2046_f2085_platform_source_boundary_pc34_compat.h"

#include <string.h>

static const char *anchor_for(unsigned int number)
{
    if (number == 2047u) return "FILLBOX.C:837 GetMouseX";
    if (number == 2048u) return "FILLBOX.C:843 GetMouseY";
    if (number == 2060u) return "CEDT018.C:822 ShowPointer";
    if (number == 2065u) return "CEDT018.C:6 Checksum";
    if (number == 2072u) return "CEDT018.C:955 HidePointer";
    if (number == 2078u) return "CEDT023.C:1261 ReadSector";
    if (number == 2079u) return "CEDT023.C:485 FormatFloppyDisk";
    if (number == 2084u) return "CEDT023.C:541 CheckX68000OriginalDisk_CPSX";
    if (number == 2085u) return "CEDT023.C:12 EjectFloppyDisk";
    return "no numbered callable body in ReDMCSB corpus";
}

static const char *boundary_for(unsigned int number)
{
    if (number == 2047u || number == 2048u)
        return "DM1-only mouse accessor; no CSB package route";
    return "fail_closed: non-PC34 platform or absent source";
}

int csb_v1_f2046_f2085_platform_source_boundary_admit_pc34(
    unsigned int number,
    CSB_V1_F2046F2085PlatformSourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F2046F2085PlatformSourceBoundaryReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 2046u || number > 2085u) return 0;

    receipt.function_number = number;
    receipt.redmcsb_anchor = anchor_for(number);
    receipt.existing_owner_or_boundary = boundary_for(number);
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f2046_f2085_platform_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB FILLBOX.C, CEDT018.C, and CEDT023.C are the authority for "
           "F2046-F2085. F2047/F2048 remain DM1-only mouse owners. No authenticated "
           "CSB PC34 package consumer is proven, so every CSB route fails closed "
           "without synthetic input, UI, graphics, audio, timing, or floppy behavior.";
}
