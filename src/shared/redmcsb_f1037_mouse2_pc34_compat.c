#include "redmcsb_f1037_mouse2_pc34_compat.h"

bool redmcsb_f1037_mouse2_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f1037_mouse2_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:1861-2091 encloses F1036_Mouse1 and "
           "F1037_Mouse2 in MEDIA613_X30J_A36M_A31E_A31M_A33M_A35E_A35M_X31J. "
           "IO.C:2017-2090 defines F1037_Mouse2: it allocates a temporary "
           "graphic, copies with a selected palette map, composites with "
           "F0654_Call_F0132_VIDEO_Blit through X68000/Amiga-specific zones, "
           "and applies G3101_auc_PaletteChanges_1. No PC 3.4 branch or "
           "portable host adapter is supplied.";
}
