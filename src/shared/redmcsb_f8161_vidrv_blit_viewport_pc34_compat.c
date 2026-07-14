#include "redmcsb_f8161_vidrv_blit_viewport_pc34_compat.h"

bool redmcsb_f8161_vidrv_blit_viewport_pc34_compat(
    const uint8_t *bitmap, size_t bitmap_byte_count,
    RedmcsbF0680C25VgaAperturePc34Compat *aperture,
    const RedmcsbF8151BoxPc34Compat *box)
{
    /* VIDEODRV.C:3573-3575: G8177=0x10; F8151(...); G8177=0. */
    return redmcsb_f8151_vidrv_blit_pc34_compat(
        bitmap, bitmap_byte_count, aperture, box, 0, 0,
        REDMCSB_F8161_VIEWPORT_SOURCE_WIDTH_PC34,
        REDMCSB_F8161_VIEWPORT_DESTINATION_WIDTH_PC34, -1,
        REDMCSB_F8151_FLIP_NONE_PC34,
        REDMCSB_F8161_VIEWPORT_COLOR_OFFSET_PC34);
}

const char *redmcsb_f8161_vidrv_blit_viewport_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3566-3577 C25 F8161: set G8177 to 0x10, "
           "call F8151(bitmap, NULL, box, 0, 0, 224, 320, -1, 0), then "
           "restore G8177 to zero.";
}
