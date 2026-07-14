#include "redmcsb_f0687_f0688_img3_pc34_compat.h"

#include <stdint.h>

int main(void)
{
    const uint8_t bytes[] = {0x3F, 0x2A, 0xFF, 0xF1, 0x23, 0x40};
    redmcsb_f0687_img3_stream_pc34_compat stream = {bytes, 6U, 0U};
    uint16_t count;
    uint8_t nibble;

    if (!redmcsb_f0688_img3_get_pixel_count_pc34_compat(&stream, &count) || count != 5U) return 1;
    if (!redmcsb_f0688_img3_get_pixel_count_pc34_compat(&stream, &count) || count != 59U) return 2;
    stream.pixel_index = 4U;
    if (!redmcsb_f0688_img3_get_pixel_count_pc34_compat(&stream, &count) || count != 0x1234U) return 3;
    stream.pixel_index = 12U;
    if (redmcsb_f0687_img3_get_nibble_pc34_compat(&stream, &nibble)) return 4;
    stream.byte_count = SIZE_MAX / 2U + 1U;
    stream.pixel_index = 0U;
    if (redmcsb_f0687_img3_get_nibble_pc34_compat(&stream, &nibble)) return 5;
    return 0;
}
