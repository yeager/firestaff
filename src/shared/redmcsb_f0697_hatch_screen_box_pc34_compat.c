#include "redmcsb_f0697_hatch_screen_box_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0697_hatch_screen_box_pc34_compat(
    const RedmcsbF0697VideoDriverPc34Compat *video_driver,
    const RedmcsbF0697ZonePc34Compat *zone,
    uint16_t color)
{
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;

    (void)color;

    if (video_driver == NULL || video_driver->hatch_screen_box == NULL ||
        zone == NULL) {
        return false;
    }

    /* ReDMCSB IMAGE.C:172-176, MEDIA709_I34E_I34M_P31J. */
    left = zone->left;
    right = zone->right;
    top = zone->top;
    bottom = zone->bottom;
    video_driver->hatch_screen_box(video_driver->context, left, right, top,
                                   bottom);
    return true;
}

const char *redmcsb_f0697_hatch_screen_box_source_evidence_pc34(void)
{
    return "ReDMCSB IMAGE.C:160-178 F0697_HatchScreenBox, "
           "MEDIA709_I34E_I34M_P31J: M704-M707 zone coordinates are copied "
           "to locals and passed to VIDRV_06_HatchScreenBox; P2373_ui_Color "
           "is not forwarded by the PC 3.4 branch.";
}
