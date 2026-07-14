#include "redmcsb_f0698_invert_box_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0698_invert_box_pc34_compat(
    const RedmcsbF0698VideoDriverPc34Compat *video_driver,
    const RedmcsbF0698ZonePc34Compat *zone)
{
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;

    if (video_driver == NULL || video_driver->invert_box == NULL ||
        zone == NULL) {
        return false;
    }

    /* ReDMCSB IMAGE.C:282-293, MEDIA709_I34E_I34M_P31J. */
    left = zone->left;
    right = zone->right;
    top = zone->top;
    bottom = zone->bottom;
    video_driver->invert_box(video_driver->context, left, right, top, bottom);
    return true;
}

const char *redmcsb_f0698_invert_box_source_evidence_pc34(void)
{
    return "ReDMCSB IMAGE.C:231-294 F0698_InvertBox, "
           "MEDIA709_I34E_I34M_P31J: M704-M707 zone coordinates are copied "
           "to locals and passed to VIDRV_05_InvertBox.";
}
