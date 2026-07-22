#include "redmcsb_f0692_fillbox_f0135_pc34_compat.h"

#include "dm1_f0135_video_fillbox_planar_20260714_pc34_compat.h"

int redmcsb_f0692_fillbox_f0135_pc34_compat(
    uint8_t *bitmap,
    size_t bitmap_size,
    size_t row_bytes,
    size_t pixel_height,
    const int16_t box[4],
    uint16_t color)
{
    return dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
        bitmap, bitmap_size, row_bytes, pixel_height, box, color);
}

const char *redmcsb_f0692_fillbox_f0135_source_evidence_pc34(void)
{
    return "ReDMCSB FILLBOX.C F0692_FillBox forwards the caller-owned "
           "bitmap, inclusive box, and color through F0135_VIDEO_FillBox; "
           "the PC34 bridge uses the existing planar F0135 contract. "
           "IMAGE3.C F0692 packed-raster routes are separate.";
}
