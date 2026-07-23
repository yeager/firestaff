#include "csb_v1_f1726_f1765_source_boundary_pc34_compat.h"

#include <string.h>

static const char *anchor_for(unsigned int number)
{
    if (number >= 1726u && number <= 1733u)
        return "UTIO.C/FLOPPY.C/MEMORY.C function-local storage";
    if (number >= 1734u && number <= 1741u)
        return "INPUT.C function-local storage";
    if (number >= 1742u && number <= 1747u)
        return "IO.C mouse function-local storage";
    if (number >= 1748u && number <= 1749u)
        return "BLITFILL.C HatchBox function-local storage";
    if (number >= 1750u && number <= 1751u)
        return "BASE.C error-display function-local storage";
    if (number >= 1752u && number <= 1756u)
        return "SCRLMGMT.C/SCRLTASK.C local state; F1756 UTSTGRAP.C debug helper";
    if (number == 1757u || number == 1758u ||
        (number >= 1760u && number <= 1761u) ||
        (number >= 1763u && number <= 1765u))
        return "UTDEBUG.C unreferenced debug helper";
    return "no numbered callable body in ReDMCSB corpus";
}

int csb_v1_f1726_f1765_source_boundary_admit_pc34(
    unsigned int number,
    CSB_V1_F1726F1765SourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F1726F1765SourceBoundaryReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 1726u || number > 1765u) return 0;

    receipt.function_number = number;
    receipt.redmcsb_anchor = anchor_for(number);
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f1726_f1765_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB UTIO.C, FLOPPY.C, MEMORY.C, INPUT.C, IO.C, BLITFILL.C, "
           "BASE.C, SCRLMGMT.C, SCRLTASK.C, and UTDEBUG.C classify F1726-F1765 "
           "as local, platform, or debug code. No authenticated CSB PC34 package "
           "consumer is proven, so every CSB route fails closed without synthetic "
           "input, UI, graphics, audio, or timing.";
}
