#include "redmcsb_f8163_copy_pixels_to_screen_c25_pc34_compat.h"

bool redmcsb_f8163_copy_pixels_to_screen_c25_pc34_compat(
    const uint8_t *bitmap, size_t bitmap_byte_count,
    uint16_t source_pixel_index, uint16_t destination_pixel_index,
    uint16_t pixel_count,
    RedmcsbF0680C25VgaAperturePc34Compat *aperture,
    uint8_t viewport_color_index_offset)
{
    /* VIDEODRV.C:3607-3646: G2159 assignment followed by C25 F0680 call. */
    return redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
        bitmap, bitmap_byte_count, (size_t)source_pixel_index, aperture,
        (size_t)destination_pixel_index, (size_t)pixel_count,
        viewport_color_index_offset);
}

const char *redmcsb_f8163_copy_pixels_to_screen_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3607-3646 C25 F8163: set G2159 to the caller "
           "bitmap and call F0680 with the unchanged source index, destination "
           "index and pixel count.";
}
