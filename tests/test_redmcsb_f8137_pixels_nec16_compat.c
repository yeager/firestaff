#include <stdint.h>
#include <string.h>

#include "redmcsb_f8137_pixels_nec16_compat.h"

int main(void)
{
    uint8_t video[3] = {0xaaU, 0xaaU, 0xaaU};
    uint8_t expected[3] = {0xa3U, 0x33U, 0x3aU};

    redmcsb_f8137_set_multiple_pixels_nec16_compat(video, 3U, 1U, 3U, 4U);
    if (memcmp(video, expected, sizeof(video)) != 0) return 1;
    redmcsb_f8137_set_multiple_pixels_nec16_compat(video, 3U, 0U, 0x1fU, 1U);
    if (video[0] != 0xf3U) return 1;
    redmcsb_f8137_set_multiple_pixels_nec16_compat(video, 3U, 5U, 4U, 2U);
    if (video[2] != 0xa4U) return 1;
    redmcsb_f8137_set_multiple_pixels_nec16_compat(video, 3U, 6U, 2U, 1U);
    return video[2] != 0xa4U ||
           strstr(redmcsb_f8137_pixels_nec16_source_evidence(), "NEC816.C:1804-1853") == 0;
}
