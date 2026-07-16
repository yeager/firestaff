#include "csb_v1_startup_img3_decode_pc34_compat.h"

#include <string.h>

int main(void)
{
    const uint8_t solid_and_copy[] = {
        0x00, 0x10, 0x00, 0x02,
        0x8f, 0x0f,
        0xb0, 0x0f
    };
    const uint8_t literals[] = {
        0x00, 0x10, 0x00, 0x01,
        0x90, 0x0f, 0x12, 0x34, 0x56, 0x78,
        0x9a, 0xbc, 0xde, 0xf0
    };
    const uint8_t package_blank_tail[] = {
        0x00, 0x10, 0x00, 0x01,
        0x75, 0x65
    };
    uint8_t pixels[32] = {0};
    uint8_t expected_literals[16] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 0
    };
    uint8_t rejected[32];

    memset(rejected, 0xaa, sizeof(rejected));
    if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
            solid_and_copy, sizeof(solid_and_copy), 16, 2, pixels,
            sizeof(pixels)) ||
        pixels[0] != 15 || pixels[15] != 15 || pixels[16] != 15 ||
        pixels[31] != 15) return 1;
    if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
            literals, sizeof(literals), 16, 1, pixels, sizeof(pixels)) ||
        memcmp(pixels, expected_literals, sizeof(expected_literals)) != 0) return 1;
    memset(pixels, 0, sizeof(pixels));
    if (!csb_v1_startup_img3_decode_to_indexed_pc34_compat(
            package_blank_tail, sizeof(package_blank_tail), 16, 1, pixels,
            sizeof(pixels)) ||
        pixels[0] != 5 || pixels[14] != 5 || pixels[15] != 0) return 1;
    if (csb_v1_startup_img3_decode_to_indexed_pc34_compat(
            solid_and_copy, sizeof(solid_and_copy) - 1U, 16, 2, rejected,
            sizeof(rejected)) || rejected[0] != 0xaa || rejected[31] != 0xaa)
        return 1;
    return 0;
}
