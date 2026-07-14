#include "redmcsb_f0690_copy_pixel_line_to_screen_pc34_compat.h"

void redmcsb_f0690_copy_pixel_line_to_screen_pc34_compat(
    const RedmcsbF0690VideoDriverPc34Compat *video_driver,
    const uint8_t *bitmap_pixel_line,
    uint16_t destination_pixel_index,
    int16_t pixel_count)
{
    if (video_driver == 0 || video_driver->copy_pixels_to_screen == 0 ||
        bitmap_pixel_line == 0 || pixel_count <= 0) {
        return;
    }
    video_driver->copy_pixels_to_screen(
        video_driver->context, bitmap_pixel_line,
        (uint16_t)(destination_pixel_index % 320U),
        destination_pixel_index, pixel_count);
}

const char *redmcsb_f0690_copy_pixel_line_to_screen_source_evidence_pc34(void)
{
    return "ReDMCSB IMAGE3.C F0690_CopyPixelLineToScreenWithoutTransparency (932-937), PC I34E/I34M";
}
