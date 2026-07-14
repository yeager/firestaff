#include "redmcsb_f0686_copy_previous_line_pc34_compat.h"

static uint8_t getp(const uint8_t *bytes, size_t pixel)
{
    return (uint8_t)(((pixel & 1U) ? bytes[pixel >> 1U] :
                      (bytes[pixel >> 1U] >> 4U)) & 15U);
}

static void setp(uint8_t *bytes, size_t pixel, uint8_t value)
{
    size_t byte_index = pixel >> 1U;

    if ((pixel & 1U) == 0U)
        bytes[byte_index] = (uint8_t)((bytes[byte_index] & 15U) | (value << 4U));
    else
        bytes[byte_index] = (uint8_t)((bytes[byte_index] & 240U) | value);
}

bool redmcsb_f0686_copy_previous_line_pc34_compat(
    uint8_t *bytes, size_t byte_count, size_t destination_pixel,
    size_t source_pixel, size_t pixel_count)
{
    size_t pixel_capacity;
    size_t pixel;

    if (bytes == NULL || byte_count > SIZE_MAX / 2U) return false;
    pixel_capacity = byte_count * 2U;
    if (destination_pixel > pixel_capacity || source_pixel > pixel_capacity ||
        pixel_count > pixel_capacity - destination_pixel ||
        pixel_count > pixel_capacity - source_pixel) return false;
    for (pixel = 0U; pixel < pixel_count; ++pixel)
        setp(bytes, destination_pixel + pixel, getp(bytes, source_pixel + pixel));
    return true;
}
const char *redmcsb_f0686_copy_previous_line_source_evidence_pc34(void){return "ReDMCSB IMAGE2.C F0686_IMG_CopyFromPreviousLine (19-37), PC I34E/I34M packed pixel copy";}
