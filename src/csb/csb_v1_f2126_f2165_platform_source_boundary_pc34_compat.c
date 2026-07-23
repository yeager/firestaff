#include "csb_v1_f2126_f2165_platform_source_boundary_pc34_compat.h"

#include <string.h>

static const char *anchor_for(unsigned int number)
{
    if (number == 2136u) return "CEDT025.C:161 GetAsciiCode";
    if (number == 2143u || number == 2144u) return "CEDT102.C Initialize/Free";
    if (number == 2145u || number == 2146u) return "COPYBYTE.C sub_F0007";
    if (number == 2147u || number == 2148u) return "BASE.C SuperHiRes graphics";
    if (number >= 2150u && number <= 2152u) return "COMMAND.C CPSX helper";
    if (number == 2153u) return "BLITFILL.C subF0136";
    if (number >= 2158u && number <= 2161u) return "ENTRANCE.C/IIGS.H mouse platform route";
    if (number == 2162u || number == 2163u) return "ENDGAME.C platform route";
    if (number == 2164u) return "DRAWMSGA.C PatchAddresses";
    if (number == 2165u) return "BASE.C PrintErrorMessage";
    return "no numbered callable body in ReDMCSB corpus";
}

static const char *boundary_for(unsigned int number)
{
    if (number == 2136u || (number >= 2150u && number <= 2152u) ||
        (number >= 2158u && number <= 2164u))
        return "DM1-only runtime owner; no CSB package route";
    return "fail_closed: non-PC34 platform or absent source";
}

int csb_v1_f2126_f2165_platform_source_boundary_admit_pc34(
    unsigned int number,
    CSB_V1_F2126F2165PlatformSourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F2126F2165PlatformSourceBoundaryReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 2126u || number > 2165u) return 0;

    receipt.function_number = number;
    receipt.redmcsb_anchor = anchor_for(number);
    receipt.existing_owner_or_boundary = boundary_for(number);
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f2126_f2165_platform_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB CEDT025.C, CEDT102.C, COPYBYTE.C, BASE.C, COMMAND.C, "
           "BLITFILL.C, ENTRANCE.C, IIGS.H, ENDGAME.C, and DRAWMSGA.C are the "
           "authority for F2126-F2165. Existing DM1 owners remain exclusive. No "
           "authenticated CSB PC34 package consumer is proven, so every CSB route "
           "fails closed without synthetic input, UI, graphics, audio, or timing.";
}
