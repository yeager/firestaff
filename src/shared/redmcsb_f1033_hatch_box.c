#include "redmcsb_f1033_hatch_box.h"

void redmcsb_f1033_hatch_box(
    redmcsb_f1033_hatch_box_primitive_fn hatch_box_primitive,
    uint8_t *screen_bitmap,
    int16_t *xyz,
    int16_t color,
    int16_t screen_pixel_width)
{
    hatch_box_primitive(screen_bitmap, xyz, color, screen_pixel_width);
}

const char *redmcsb_f1033_hatch_box_source_evidence(void)
{
    return "ReDMCSB BLITFILL.C:428-433 defines "
           "F1033_HatchBox_Unreferenced and calls "
           "F1032_GRF1_12_HatchBox with G0348_Bitmap_Screen, the original "
           "P2723_pi_XYZ, P2764_i_Color, and G2071_C320_ScreenPixelWidth; "
           "FILLBOX.C:625-640 gives the X30J F1032 four-argument ABI.";
}
