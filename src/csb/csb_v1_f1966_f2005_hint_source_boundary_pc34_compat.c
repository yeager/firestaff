#include "csb_v1_f1966_f2005_hint_source_boundary_pc34_compat.h"

#include <string.h>

static const char *anchor_for(unsigned int number)
{
    if (number >= 1966u && number <= 1970u)
        return "HINT002.C/HINTDBG.C debug route";
    if (number >= 1982u && number <= 1985u)
        return "HINTCASE.C/HINT008.C case and CPSX route";
    if (number == 1987u || number == 1988u)
        return "CEDTINCI.C checksum route";
    if (number == 1989u) return "CEDT002.C InitializeRandomNumber";
    if (number == 1992u) return "CEDTINC8.C RequestUtilityDiskInDrive";
    if (number == 1998u) return "CEDTINCK.C PrintSpacePaddedText";
    if (number == 1999u || number == 2000u) return "CEDTINC6.C FILE read/write";
    return "no numbered callable body in ReDMCSB corpus";
}

static const char *boundary_for(unsigned int number)
{
    if (number == 1984u) return "DM1-only case helper owner; no CSB route";
    return "fail_closed: no authenticated CSB PC34 package owner";
}

int csb_v1_f1966_f2005_hint_source_boundary_admit_pc34(
    unsigned int number,
    CSB_V1_F1966F2005HintSourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F1966F2005HintSourceBoundaryReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 1966u || number > 2005u) return 0;

    receipt.function_number = number;
    receipt.redmcsb_anchor = anchor_for(number);
    receipt.existing_owner_or_boundary = boundary_for(number);
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f1966_f2005_hint_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB HINT002.C, HINTDBG.C, HINTCASE.C, HINT008.C, CEDTINCI.C, "
           "CEDT002.C, CEDTINC6.C, CEDTINC8.C, and CEDTINCK.C are the authority "
           "for F1966-F2005. No authenticated CSB PC34 package owner is proven; "
           "F1984 remains DM1-only. Every CSB route fails closed without synthetic "
           "input, UI, graphics, audio, timing, or file behavior.";
}
