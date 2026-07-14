#include "redmcsb_f1032_hatch_box_pc34_compat.h"

bool redmcsb_f1032_hatch_box_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f1032_hatch_box_source_evidence_pc34(void)
{
    return "ReDMCSB FILLBOX.C:624-718 encloses F1032_GRF1_12_HatchBox in "
           "MEDIA611_X30J_A36M_A31E_A31M_A33M_A35E_A35M. "
           "FILLBOX.C:642-695 is the X68000 route, which writes directly "
           "to G3090_X68000VideoMemoryAddress while in supervisor mode. "
           "FILLBOX.C:697-715 is the Amiga route, which binds the bitmap "
           "through F1129_, applies G3209_ as AreaPtrn with AreaPtSz 1, "
           "uses JAM1 RectFill, clears the pattern state, and WaitBlit "
           "synchronizes. GRF1.C:29 defines G3209_ as AA AA 55 55. "
           "No PC 3.4 branch or portable host adapter is supplied.";
}
