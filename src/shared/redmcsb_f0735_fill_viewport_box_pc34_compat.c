#include "redmcsb_f0735_fill_viewport_box_pc34_compat.h"

void redmcsb_f0735_fill_viewport_box_pc34_compat(
    const redmcsb_f0735_graphics_pc34_compat *graphics,
    int16_t *xyz,
    int16_t color)
{
    /* ReDMCSB BLITFILL.C F0735_FillViewportBox, PC 3.4 MEDIA527_I34E_I34M. */
    graphics->fill_box(graphics->context, xyz, color, INT16_C(224));
}

const char *redmcsb_f0735_fill_viewport_box_source_evidence_pc34(void)
{
    return "ReDMCSB BLITFILL.C F0735_FillViewportBox calls "
           "F0135_VIDEO_FillBox(G0296_puc_Bitmap_Viewport, P2229_pi_XYZ, "
           "P2230_i_Color, G2073_C224_ViewportPixelWidth); PC 3.4 defines "
           "G2073_C224_ViewportPixelWidth as 224 and supplies no height "
           "argument to F0135.";
}
