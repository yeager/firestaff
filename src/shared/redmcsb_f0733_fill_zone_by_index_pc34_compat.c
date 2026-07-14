#include "redmcsb_f0733_fill_zone_by_index_pc34_compat.h"

void redmcsb_f0733_fill_zone_by_index_pc34_compat(
    const redmcsb_f0733_graphics_pc34_compat *graphics,
    int16_t zone_index,
    int16_t color)
{
    int16_t zone_xyz[4];
    int16_t *resolved_zone;

    /* ReDMCSB BLITFILL.C:225-238, PC 3.4 MEDIA527_I34E_I34M. */
    resolved_zone = graphics->get_zone(graphics->context, zone_index, zone_xyz);
    graphics->fill_box(graphics->context, resolved_zone, color,
                       INT16_C(320), INT16_C(200));
}

const char *redmcsb_f0733_fill_zone_by_index_source_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:9432-9435 declares F0733 with signed int16_t "
           "zone and color parameters; BLITFILL.C:225-238 resolves the zone "
           "with F0638_GetZone and calls F0135_VIDEO_FillBox on "
           "G0348_Bitmap_Screen at G2071_C320_ScreenPixelWidth and "
           "G2072_C200_ScreenPixelHeight. COORD.C:2490-2495 defines "
           "F0638_GetZone as F0637_GetProportionalZone(..., 10000, 10000).";
}
