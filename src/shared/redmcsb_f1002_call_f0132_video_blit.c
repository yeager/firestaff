#include "redmcsb_f1002_call_f0132_video_blit.h"

#include <string.h>

static int16_t redmcsb_f1002_bitmap_pixel_width(const uint8_t *bitmap)
{
    int16_t width;

    memcpy(&width, bitmap - (2 * sizeof(width)), sizeof(width));
    return width;
}

void redmcsb_f1002_call_f0132_video_blit(
    uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    int16_t *xyz,
    int16_t x,
    int16_t y,
    int16_t transparent_color,
    int16_t flip,
    redmcsb_f1002_video_blit video_blit)
{
    video_blit(bitmap_source, bitmap_destination, xyz, x, y,
               redmcsb_f1002_bitmap_pixel_width(bitmap_source),
               redmcsb_f1002_bitmap_pixel_width(bitmap_destination),
               transparent_color, flip);
}

const char *redmcsb_f1002_call_f0132_video_blit_source_evidence(void)
{
    return "ReDMCSB BASE.C:1202-1212 defines "
           "F1002_Call_F0132_VIDEO_Blit under MEDIA458_P20JA_P20JB. "
           "BASE.C:1211 forwards source, destination, XYZ, X, Y, "
           "transparent color, and flip to F0132_VIDEO_Blit, with "
           "M100_PIXEL_WIDTH(source) and M100_PIXEL_WIDTH(destination). "
           "DEFS.H:3444 defines M100_PIXEL_WIDTH(block) as the int16_t "
           "two words before the bitmap data pointer.";
}
