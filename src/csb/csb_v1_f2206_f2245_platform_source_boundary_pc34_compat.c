#include "csb_v1_f2206_f2245_platform_source_boundary_pc34_compat.h"

#include <string.h>

static const char *anchor_for(unsigned int number)
{
    if (number == 2206u) return "COMMAND.C:2204 CPSX";
    if (number == 2207u) return "DEFS.H:9735 CPSX";
    if (number == 2210u || number == 2211u) return "SWSH.C event/QuickDraw initialization";
    if (number >= 2214u && number <= 2225u) return "FILE.C Apple IIgs ProDOS vector";
    if (number == 2226u) return "STARTUP1.C:221 F2173 subroutine";
    if (number == 2227u) return "FILE.C:15 F2174 subroutine";
    if (number == 2228u) return "FILE.C:29 Apple IIgs ProDOS FORMAT";
    if (number == 2232u) return "IIGS.H:1710 DisplayErrorAndStop";
    if (number == 2234u) return "ANIMTOWN.C:659 ClearBothScreenBuffers";
    if (number == 2235u) return "DEFS.H:9806 MUSIC_StopCD";
    if (number >= 2236u && number <= 2241u) return "CEDT026.C FM-Towns mouse route";
    if (number >= 2242u && number <= 2244u) return "CEDT028.C memory platform route";
    if (number == 2245u) return "CEDT027.C INIT_TOWNS";
    return "no numbered callable body in ReDMCSB corpus";
}

static const char *boundary_for(unsigned int number)
{
    if (number == 2206u || number == 2207u || number == 2226u || number == 2227u ||
        number == 2234u || number == 2235u || (number >= 2236u && number <= 2241u))
        return "DM1-only runtime owner; no CSB package route";
    return "fail_closed: non-PC34 platform or absent source";
}

int csb_v1_f2206_f2245_platform_source_boundary_admit_pc34(
    unsigned int number,
    CSB_V1_F2206F2245PlatformSourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F2206F2245PlatformSourceBoundaryReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 2206u || number > 2245u) return 0;

    receipt.function_number = number;
    receipt.redmcsb_anchor = anchor_for(number);
    receipt.existing_owner_or_boundary = boundary_for(number);
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f2206_f2245_platform_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB COMMAND.C, SWSH.C, FILE.C, STARTUP1.C, IIGS.H, ANIMTOWN.C, "
           "CEDT026.C, CEDT027.C, and CEDT028.C are the authority for F2206-F2245. "
           "Existing DM1 owners remain exclusive. No authenticated CSB PC34 package "
           "consumer is proven, so every CSB route fails closed without synthetic "
           "input, UI, graphics, audio, timing, or file behavior.";
}
