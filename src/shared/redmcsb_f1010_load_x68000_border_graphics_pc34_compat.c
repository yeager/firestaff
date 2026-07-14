#include "redmcsb_f1010_load_x68000_border_graphics_pc34_compat.h"

bool redmcsb_f1010_load_x68000_border_graphics_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f1010_load_x68000_border_graphics_source_evidence_pc34(
    void)
{
    return "ReDMCSB IMAGE.C:58-138 encloses "
           "F1010_LoadX68000BorderGraphics in MEDIA607_X30J_X31J. "
           "IMAGE.C:108-136 temporarily loads four graphics beginning at "
           "M723_X68000_BORDER_TOP, expands each into the viewport bitmap, "
           "copies its four-byte header, blits it to zones 420-423, restores "
           "the X68000 video-memory address, and frees the accumulated "
           "temporary allocation. MEDIA692_X31J returns first when G3076_B_ "
           "is false. No PC 3.4 branch or portable host adapter is supplied.";
}
