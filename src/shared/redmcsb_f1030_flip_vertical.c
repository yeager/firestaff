#include "redmcsb_f1030_flip_vertical.h"

#include <string.h>

void redmcsb_f1030_flip_vertical(
    redmcsb_f1030_flip_vertical_primitive_fn flip_vertical_primitive,
    uint8_t *bitmap)
{
    int16_t pixel_width;
    int16_t pixel_height;

    memcpy(&pixel_width, bitmap - (2 * sizeof(pixel_width)),
           sizeof(pixel_width));
    memcpy(&pixel_height, bitmap - sizeof(pixel_height), sizeof(pixel_height));
    flip_vertical_primitive(bitmap, pixel_width, pixel_height);
}

const char *redmcsb_f1030_flip_vertical_source_evidence(void)
{
    return "ReDMCSB BASE.C:1577-1580 F1030_ calls "
           "F0131_VIDEO_FlipVertical with P2758_puc_Bitmap and "
           "M100_PIXEL_WIDTH/M101_PIXEL_HEIGHT; DEFS.H:3444-3445 defines "
           "those macros as the two preceding int16_t bitmap-header values; "
           "FLIPVERT.C:12-20 declares F0131_VIDEO_FlipVertical with the "
           "bitmap, width, and height parameters.";
}
