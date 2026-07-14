#include "redmcsb_f0732_fill_screen_area_pc34_compat.h"

#include <stddef.h>

void redmcsb_f0732_fill_screen_area_pc34_compat(
    const redmcsb_f0732_video_driver_pc34_compat *video_driver,
    int16_t *zone,
    uint16_t color)
{
    int16_t box[4];

    /* FILLBOX.C PC 3.4 MEDIA709_I34E_I34M_P31J conversion from ZONE to BOX. */
    box[0] = zone[0];
    box[1] = (int16_t)((uint16_t)zone[0] + (uint16_t)zone[2] - 1U);
    box[2] = zone[1];
    box[3] = (int16_t)((uint16_t)zone[1] + (uint16_t)zone[3] - 1U);

    /* BLITFILL.C:199-209 F0732 -> F0135_VIDEO_FillBox(NULL, ..., 320). */
    video_driver->fill_box(video_driver->context, NULL, box, (int16_t)color,
                           REDMCSB_F0732_SCREEN_PIXEL_WIDTH_PC34);
}

const char *redmcsb_f0732_fill_screen_area_source_evidence_pc34(void)
{
    return "ReDMCSB BLITFILL.C:199-209 defines F0732 as "
           "F0135_VIDEO_FillBox(NULL, zone, color, "
           "G2071_C320_ScreenPixelWidth); FILLBOX.C:425-440 (PC 3.4 "
           "MEDIA709_I34E_I34M_P31J) expands {left, top, width, height} "
           "to the inclusive VIDRV_01 box.";
}
